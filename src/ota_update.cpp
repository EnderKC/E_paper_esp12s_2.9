#include "ota_update.h"

#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecureBearSSL.h>

#include "firmware_version.h"

namespace
{
const unsigned long otaCheckIntervalMs = 6UL * 60UL * 60UL * 1000UL;
unsigned long lastOtaCheck = 0;
BearSSL::WiFiClientSecure otaClient;

bool fetchManifest(String &payload)
{
    payload = "";

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
    otaClient.setInsecure();

    ESPhttpUpdate.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    ESPhttpUpdate.setClientTimeout(15000);
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

    const unsigned long now = millis();
    if (!force && lastOtaCheck != 0 && now - lastOtaCheck < otaCheckIntervalMs)
    {
        return;
    }
    lastOtaCheck = now;

    Serial.printf("OTA current version: %s\n", APP_VERSION);

    String payload;
    if (!fetchManifest(payload))
    {
        return;
    }

    String remoteVersion;
    String binUrl;
    String md5;
    if (!parseManifest(payload, remoteVersion, binUrl, md5))
    {
        return;
    }

    Serial.printf("OTA remote version: %s\n", remoteVersion.c_str());
    if (!isNewFirmwareVersion(APP_VERSION, remoteVersion))
    {
        Serial.println("OTA already up to date");
        return;
    }

    runHttpUpdate(binUrl, md5);
}
