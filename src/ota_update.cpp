#include "ota_update.h"

#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecureBearSSL.h>

#include "display.h"
#include "firmware_version.h"

namespace
{
const unsigned long otaCheckIntervalMs = 6UL * 60UL * 60UL * 1000UL;
unsigned long lastOtaCheck = 0;
unsigned long otaProgressStartedAt = 0;
unsigned long otaLastProgressLogAt = 0;
unsigned long otaLastProgressDrawAt = 0;
int otaLastLoggedPercent = -1;
int otaLastDrawnPercent = -1;
bool nonReleaseSkipLogged = false;

bool isReleaseVersion()
{
    return APP_VERSION[0] == 'v';
}

void configureOtaTlsClient(BearSSL::WiFiClientSecure &client)
{
    client.setInsecure();
    client.setBufferSizes(4096, 512);
}

void drawOtaPage(const char *title, const char *line1 = "", const char *line2 = "")
{
    display.setFullWindow();
    display.firstPage();
    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.drawRect(0, 0, 296, 128, GxEPD_BLACK);
        display.fillRect(0, 0, 296, 24, GxEPD_BLACK);

        u8g2Fonts.setForegroundColor(GxEPD_WHITE);
        u8g2Fonts.setBackgroundColor(GxEPD_BLACK);
        u8g2Fonts.setFont(u8g2_font_wqy12_t_gb2312b);
        u8g2Fonts.setCursor(10, 17);
        u8g2Fonts.print(title);

        u8g2Fonts.setForegroundColor(GxEPD_BLACK);
        u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
        u8g2Fonts.setCursor(10, 50);
        u8g2Fonts.print(line1);
        u8g2Fonts.setCursor(10, 74);
        u8g2Fonts.print(line2);
    } while (display.nextPage());
}

void finishOtaCheckWithRefresh(const char *title, const char *line1, const char *line2 = "")
{
    drawOtaPage(title, line1, line2);
    delay(1000);
    ePaper_flash();
}

void drawOtaProgress(int percent, int current, int total)
{
    const unsigned long now = millis();
    const unsigned long elapsed = now - otaProgressStartedAt;
    const float speedKbps = elapsed > 0 ? (current / 1024.0f) / (elapsed / 1000.0f) : 0.0f;

    char versionLine[48];
    char progressLine[48];
    char speedLine[48];
    snprintf(versionLine, sizeof(versionLine), "%s -> OTA", APP_VERSION);
    snprintf(progressLine, sizeof(progressLine), "%d%%  %d/%d KB", percent, current / 1024, total / 1024);
    snprintf(speedLine, sizeof(speedLine), "%.1f KB/s", speedKbps);

    display.setPartialWindow(0, 24, 296, 104);
    display.firstPage();
    do
    {
        display.fillRect(0, 24, 296, 104, GxEPD_WHITE);
        display.drawRect(0, 24, 296, 104, GxEPD_BLACK);
        u8g2Fonts.setForegroundColor(GxEPD_BLACK);
        u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
        u8g2Fonts.setFont(u8g2_font_wqy12_t_gb2312b);
        u8g2Fonts.setCursor(10, 48);
        u8g2Fonts.print(versionLine);

        display.drawRect(10, 62, 276, 18, GxEPD_BLACK);
        const int barWidth = constrain((274 * percent) / 100, 0, 274);
        display.fillRect(11, 63, barWidth, 16, GxEPD_BLACK);

        u8g2Fonts.setCursor(10, 100);
        u8g2Fonts.print(progressLine);
        u8g2Fonts.setCursor(196, 100);
        u8g2Fonts.print(speedLine);
    } while (display.nextPage());
}

void maybeDrawUpdateProgress(int percent, int current, int total)
{
    const unsigned long now = millis();
    const bool firstDraw = otaLastDrawnPercent < 0;
    const bool reachedNextStep = percent >= otaLastDrawnPercent + 10;
    const bool finished = percent >= 100;

    if (!firstDraw && !finished && !reachedNextStep)
    {
        return;
    }
    if (!firstDraw && !finished && now - otaLastProgressDrawAt < 5000)
    {
        return;
    }

    otaLastDrawnPercent = percent;
    otaLastProgressDrawAt = now;
    drawOtaProgress(percent, current, total);
}

void handleUpdateProgress(int current, int total)
{
    if (total <= 0)
    {
        return;
    }

    const int percent = constrain((current * 100) / total, 0, 100);
    const unsigned long now = millis();
    if (otaLastLoggedPercent >= 0 &&
        percent < otaLastLoggedPercent + 5 &&
        now - otaLastProgressLogAt < 2000 &&
        percent < 100)
    {
        return;
    }

    otaLastLoggedPercent = percent;
    otaLastProgressLogAt = now;

    const unsigned long elapsed = now - otaProgressStartedAt;
    const float speedKbps = elapsed > 0 ? (current / 1024.0f) / (elapsed / 1000.0f) : 0.0f;
    Serial.printf("OTA update progress: %d%%, %d/%d bytes, %.1f KB/s, heap=%u, maxBlock=%u, frag=%u%%\n",
                  percent,
                  current,
                  total,
                  speedKbps,
                  ESP.getFreeHeap(),
                  ESP.getMaxFreeBlockSize(),
                  ESP.getHeapFragmentation());

    maybeDrawUpdateProgress(percent, current, total);
}

bool findAssetUrlByName(JsonArrayConst assets, const char *assetName, String &assetUrl)
{
    assetUrl = "";

    for (JsonObjectConst asset : assets)
    {
        const char *name = asset["name"] | "";
        if (strcmp(name, assetName) == 0)
        {
            assetUrl = asset["browser_download_url"] | "";
            return assetUrl.length() > 0;
        }
    }

    return false;
}

bool parseLatestRelease(const String &payload, String &releaseVersion, String &manifestUrl)
{
    JsonDocument doc;
    const DeserializationError error = deserializeJson(doc, payload);
    if (error)
    {
        Serial.printf("OTA release JSON failed: %s\n", error.c_str());
        return false;
    }

    releaseVersion = doc["tag_name"] | "";
    Serial.printf("OTA latest release version: %s\n", releaseVersion.c_str());

    JsonArrayConst assets = doc["assets"].as<JsonArrayConst>();
    if (assets.isNull())
    {
        Serial.println("OTA latest release missing assets");
        return false;
    }

    if (!findAssetUrlByName(assets, "manifest.json", manifestUrl))
    {
        Serial.println("OTA latest release missing manifest.json asset");
        return false;
    }

    Serial.printf("OTA latest manifest asset: %s\n", manifestUrl.c_str());
    return releaseVersion.length() > 0;
}

bool fetchUrlPayload(const String &url, const char *label, String &payload)
{
    payload = "";
    Serial.printf("OTA %s URL: %s\n", label, url.c_str());

    BearSSL::WiFiClientSecure otaClient;
    configureOtaTlsClient(otaClient);

    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setRedirectLimit(5);
    http.setTimeout(15000);
    if (!http.begin(otaClient, url))
    {
        Serial.printf("OTA %s HTTP begin failed\n", label);
        return false;
    }

    const int httpCode = http.GET();
    Serial.printf("OTA %s HTTP code: %d\n", label, httpCode);
    if (httpCode != HTTP_CODE_OK)
    {
        Serial.printf("OTA %s HTTP failed, code=%d, error=%s\n",
                      label,
                      httpCode,
                      http.errorToString(httpCode).c_str());
        const String errorPayload = http.getString();
        if (errorPayload.length())
        {
            Serial.printf("OTA %s error body length: %u\n", label, errorPayload.length());
            Serial.printf("OTA %s error body head: %.160s\n", label, errorPayload.c_str());
        }
        http.end();
        return false;
    }

    payload = http.getString();
    Serial.printf("OTA %s payload length: %u\n", label, payload.length());
    if (payload.length())
    {
        Serial.printf("OTA %s payload head: %.160s\n", label, payload.c_str());
    }
    http.end();
    return payload.length() > 0;
}

bool parseManifest(const String &payload, String &version, String &binUrl, String &md5, uint32_t &binSize)
{
    JsonDocument doc;
    const DeserializationError error = deserializeJson(doc, payload);
    if (error)
    {
        Serial.printf("OTA manifest JSON failed: %s\n", error.c_str());
        return false;
    }

    version = doc["version"] | "";
    binUrl = doc["bin"] | "";
    md5 = doc["md5"] | "";
    binSize = doc["size"] | 0UL;

    Serial.printf("OTA manifest version: %s\n", version.c_str());
    Serial.printf("OTA manifest bin: %s\n", binUrl.c_str());
    Serial.printf("OTA manifest size: %u\n", binSize);
    Serial.printf("OTA manifest md5: %s\n", md5.length() ? md5.c_str() : "<none>");

    if (!version.length() || !binUrl.length())
    {
        Serial.println("OTA manifest missing version or bin URL");
        return false;
    }

    return true;
}

void logOtaRuntimeInfo(uint32_t expectedSize)
{
    Serial.printf("OTA runtime flash size: sketch=%u, freeSketch=%u, expectedBin=%u\n",
                  ESP.getSketchSize(),
                  ESP.getFreeSketchSpace(),
                  expectedSize);
    Serial.printf("OTA runtime heap: free=%u, maxBlock=%u, fragmentation=%u%%\n",
                  ESP.getFreeHeap(),
                  ESP.getMaxFreeBlockSize(),
                  ESP.getHeapFragmentation());
}

bool runHttpUpdate(const String &binUrl, const String &md5, uint32_t expectedSize)
{
    BearSSL::WiFiClientSecure otaClient;
    configureOtaTlsClient(otaClient);

    otaProgressStartedAt = millis();
    otaLastProgressLogAt = 0;
    otaLastProgressDrawAt = 0;
    otaLastLoggedPercent = -1;
    otaLastDrawnPercent = -1;

    logOtaRuntimeInfo(expectedSize);
    if (expectedSize > 0 && expectedSize > ESP.getFreeSketchSpace())
    {
        Serial.printf("OTA skipped, not enough sketch space: expected=%u, free=%u\n",
                      expectedSize,
                      ESP.getFreeSketchSpace());
        return false;
    }

    ESPhttpUpdate.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    ESPhttpUpdate.setClientTimeout(15000);
    ESPhttpUpdate.onProgress(handleUpdateProgress);
    ESPhttpUpdate.onStart([]() {
        Serial.println("OTA update started");
    });
    ESPhttpUpdate.onError([](int error) {
        Serial.printf("OTA update error callback: %d\n", error);
    });
    ESPhttpUpdate.onEnd([]() {
        Serial.println("OTA update finished, restarting");
    });

    if (md5.length())
    {
        ESPhttpUpdate.setMD5sum(md5);
        Serial.printf("OTA expected MD5: %s\n", md5.c_str());
    }
    else
    {
        Serial.println("OTA expected MD5: <none>");
    }

    Serial.printf("OTA downloading %s\n", binUrl.c_str());
    const t_httpUpdate_return result = ESPhttpUpdate.update(otaClient, binUrl, APP_VERSION);
    Serial.printf("OTA update result: %d\n", result);

    switch (result)
    {
    case HTTP_UPDATE_FAILED:
        Serial.printf("OTA update failed, error=%d, %s\n",
                      ESPhttpUpdate.getLastError(),
                      ESPhttpUpdate.getLastErrorString().c_str());
        return false;
    case HTTP_UPDATE_NO_UPDATES:
        Serial.println("OTA no updates");
        return true;
    case HTTP_UPDATE_OK:
        Serial.println("OTA update OK");
        return true;
    }

    return false;
}
}

bool isNewFirmwareVersion(const String &currentVersion, const String &remoteVersion)
{
    return currentVersion.startsWith("v") &&
           remoteVersion.startsWith("v") &&
           remoteVersion != currentVersion;
}

void checkForFirmwareUpdate(bool force)
{
    if (!WiFi.isConnected())
    {
        if (force)
        {
            Serial.println("OTA check requested, force=true, wifi=disconnected");
        }
        return;
    }

    if (!isReleaseVersion())
    {
        if (force || !nonReleaseSkipLogged)
        {
            Serial.printf("OTA skipped for non-release version: %s\n", APP_VERSION);
            nonReleaseSkipLogged = true;
        }
        return;
    }

    const unsigned long now = millis();
    if (!force && lastOtaCheck != 0 && now - lastOtaCheck < otaCheckIntervalMs)
    {
        return;
    }
    lastOtaCheck = now;

    Serial.printf("OTA check requested, force=%s, wifi=connected\n",
                  force ? "true" : "false");
    Serial.printf("OTA current version: %s\n", APP_VERSION);
    drawOtaPage("检查固件更新", APP_VERSION, "正在连接更新服务器...");

    String latestReleaseVersion;
    String manifestUrl;
    {
        String releasePayload;
        if (!fetchUrlPayload(OTA_RELEASE_API_URL, "release", releasePayload))
        {
            finishOtaCheckWithRefresh("检查更新失败", "无法获取版本信息", "稍后会再次检查");
            return;
        }

        if (!parseLatestRelease(releasePayload, latestReleaseVersion, manifestUrl))
        {
            finishOtaCheckWithRefresh("检查更新失败", "版本接口无效", "稍后会再次检查");
            return;
        }
    }

    String remoteVersion;
    String binUrl;
    String md5;
    uint32_t binSize = 0;
    {
        String payload;
        if (!fetchUrlPayload(manifestUrl, "manifest", payload))
        {
            finishOtaCheckWithRefresh("检查更新失败", "无法获取固件清单", "稍后会再次检查");
            return;
        }

        if (!parseManifest(payload, remoteVersion, binUrl, md5, binSize))
        {
            finishOtaCheckWithRefresh("检查更新失败", "版本信息无效", "稍后会再次检查");
            return;
        }
    }

    Serial.printf("OTA remote version: %s\n", remoteVersion.c_str());
    Serial.printf("OTA version comparison: current=%s, remote=%s, shouldUpdate=%s\n",
                  APP_VERSION,
                  remoteVersion.c_str(),
                  isNewFirmwareVersion(APP_VERSION, remoteVersion) ? "true" : "false");
    if (!isNewFirmwareVersion(APP_VERSION, remoteVersion))
    {
        Serial.println("OTA already up to date");
        finishOtaCheckWithRefresh("固件已是最新", APP_VERSION, remoteVersion.c_str());
        return;
    }

    drawOtaPage("发现新版本", remoteVersion.c_str(), "开始下载固件...");
    if (!runHttpUpdate(binUrl, md5, binSize))
    {
        finishOtaCheckWithRefresh("固件更新失败", "稍后会再次检查", "");
    }
}
