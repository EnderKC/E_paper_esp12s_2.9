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
unsigned long otaLastProgressDrawAt = 0;
int otaLastDrawnPercent = -1;
bool nonReleaseSkipLogged = false;

bool isReleaseVersion()
{
    return APP_VERSION[0] == 'v';
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

void handleUpdateProgress(int current, int total)
{
    if (total <= 0)
    {
        return;
    }

    const int percent = constrain((current * 100) / total, 0, 100);
    const unsigned long now = millis();
    if (otaLastDrawnPercent >= 0 &&
        percent < otaLastDrawnPercent + 5 &&
        now - otaLastProgressDrawAt < 2000 &&
        percent < 100)
    {
        return;
    }

    otaLastDrawnPercent = percent;
    otaLastProgressDrawAt = now;
    drawOtaProgress(percent, current, total);
}

bool fetchManifest(String &payload)
{
    payload = "";

    BearSSL::WiFiClientSecure otaClient;
    otaClient.setInsecure();

    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    if (!http.begin(otaClient, OTA_MANIFEST_URL))
    {
        Serial.println("OTA manifest HTTP begin failed");
        return false;
    }

    const int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK)
    {
        Serial.printf("OTA manifest HTTP failed, code=%d\n", httpCode);
        http.end();
        return false;
    }

    payload = http.getString();
    http.end();
    return payload.length() > 0;
}

bool parseManifest(const String &payload, String &version, String &binUrl, String &md5)
{
    StaticJsonDocument<512> doc;
    const DeserializationError error = deserializeJson(doc, payload);
    if (error)
    {
        Serial.printf("OTA manifest JSON failed: %s\n", error.c_str());
        return false;
    }

    version = doc["version"] | "";
    binUrl = doc["bin"] | "";
    md5 = doc["md5"] | "";

    if (!version.length() || !binUrl.length())
    {
        Serial.println("OTA manifest missing version or bin URL");
        return false;
    }

    return true;
}

bool runHttpUpdate(const String &binUrl, const String &md5)
{
    BearSSL::WiFiClientSecure otaClient;
    otaClient.setInsecure();

    otaProgressStartedAt = millis();
    otaLastProgressDrawAt = 0;
    otaLastDrawnPercent = -1;

    ESPhttpUpdate.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    ESPhttpUpdate.setClientTimeout(15000);
    ESPhttpUpdate.onProgress(handleUpdateProgress);
    ESPhttpUpdate.onStart([]() {
        drawOtaPage("正在更新固件", "准备下载固件...", "请勿断电");
    });
    ESPhttpUpdate.onError([](int error) {
        char errorLine[32];
        snprintf(errorLine, sizeof(errorLine), "错误码: %d", error);
        drawOtaPage("固件更新失败", errorLine, "稍后会再次检查");
    });
    ESPhttpUpdate.onEnd([]() {
        drawOtaPage("固件更新完成", "设备即将重启", "");
    });

    if (md5.length())
    {
        ESPhttpUpdate.setMD5sum(md5);
    }

    Serial.printf("OTA downloading %s\n", binUrl.c_str());
    const t_httpUpdate_return result = ESPhttpUpdate.update(otaClient, binUrl, APP_VERSION);

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

    Serial.printf("OTA current version: %s\n", APP_VERSION);
    drawOtaPage("检查固件更新", APP_VERSION, "正在连接更新服务器...");

    String payload;
    if (!fetchManifest(payload))
    {
        drawOtaPage("检查更新失败", "无法获取版本信息", "稍后会再次检查");
        return;
    }

    String remoteVersion;
    String binUrl;
    String md5;
    if (!parseManifest(payload, remoteVersion, binUrl, md5))
    {
        drawOtaPage("检查更新失败", "版本信息无效", "稍后会再次检查");
        return;
    }

    Serial.printf("OTA remote version: %s\n", remoteVersion.c_str());
    if (!isNewFirmwareVersion(APP_VERSION, remoteVersion))
    {
        Serial.println("OTA already up to date");
        drawOtaPage("固件已是最新", APP_VERSION, remoteVersion.c_str());
        delay(1000);
        return;
    }

    drawOtaPage("发现新版本", remoteVersion.c_str(), "开始下载固件...");
    runHttpUpdate(binUrl, md5);
}
