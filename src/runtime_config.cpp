#include "runtime_config.h"

#include <ArduinoJson.h>
#include <LittleFS.h>

#include "app_config.h"
#include "secrets.h"

namespace
{
const char *configPath = "/config.json";
RuntimeConfig runtimeConfig;
bool fsReady = false;

String readJsonString(JsonVariantConst value, const char *fallback)
{
    const char *text = value | fallback;
    return String(text ? text : "");
}

RuntimeConfig defaultRuntimeConfig()
{
    RuntimeConfig config;
    config.weatherApiKey = WEATHER_API_KEY;
    config.driverApiKey = DRIVER_API_KEY;
    config.weatherLocation = "ip";
    config.mqttBroker = MQTT_BROKER;
    config.mqttUsername = MQTT_USERNAME;
    config.mqttPassword = MQTT_PASSWORD;
    return config;
}

bool loadRuntimeConfig()
{
    if (!LittleFS.exists(configPath))
    {
        Serial.println("Runtime config not found, using build defaults");
        return false;
    }

    File file = LittleFS.open(configPath, "r");
    if (!file)
    {
        Serial.println("Runtime config open failed");
        return false;
    }

    JsonDocument doc;
    const DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error)
    {
        Serial.printf("Runtime config JSON failed: %s\n", error.c_str());
        return false;
    }

    runtimeConfig.weatherApiKey = readJsonString(doc["weather_api_key"], WEATHER_API_KEY);
    runtimeConfig.driverApiKey = readJsonString(doc["driver_api_key"], DRIVER_API_KEY);
    runtimeConfig.weatherLocation = readJsonString(doc["weather_location"], "ip");
    runtimeConfig.mqttBroker = readJsonString(doc["mqtt_broker"], MQTT_BROKER);
    runtimeConfig.mqttUsername = readJsonString(doc["mqtt_username"], MQTT_USERNAME);
    runtimeConfig.mqttPassword = readJsonString(doc["mqtt_password"], MQTT_PASSWORD);

    if (!runtimeConfig.weatherLocation.length())
    {
        runtimeConfig.weatherLocation = "ip";
    }

    Serial.println("Runtime config loaded");
    return true;
}
}

bool initRuntimeConfig()
{
    runtimeConfig = defaultRuntimeConfig();

    LittleFSConfig fsConfig;
    fsConfig.setAutoFormat(true);
    LittleFS.setConfig(fsConfig);
    fsReady = LittleFS.begin();
    if (!fsReady)
    {
        Serial.println("LittleFS mount failed, using build defaults");
        return false;
    }

    loadRuntimeConfig();
    return true;
}

const RuntimeConfig &getRuntimeConfig()
{
    return runtimeConfig;
}

bool isRuntimeConfigComplete()
{
    if (!runtimeConfig.weatherApiKey.length())
    {
        return false;
    }

#if APP_MODE == 1
    if (!runtimeConfig.driverApiKey.length())
    {
        return false;
    }
#endif

#if APP_MODE == 2
    if (!runtimeConfig.mqttBroker.length() ||
        !runtimeConfig.mqttUsername.length() ||
        !runtimeConfig.mqttPassword.length())
    {
        return false;
    }
#endif

    return true;
}

bool saveRuntimeConfig(const RuntimeConfig &config)
{
    if (!fsReady && !initRuntimeConfig())
    {
        return false;
    }

    JsonDocument doc;
    doc["weather_api_key"] = config.weatherApiKey;
    doc["driver_api_key"] = config.driverApiKey;
    doc["weather_location"] = config.weatherLocation.length() ? config.weatherLocation : "ip";
    doc["mqtt_broker"] = config.mqttBroker;
    doc["mqtt_username"] = config.mqttUsername;
    doc["mqtt_password"] = config.mqttPassword;

    File file = LittleFS.open(configPath, "w");
    if (!file)
    {
        Serial.println("Runtime config write open failed");
        return false;
    }

    const size_t written = serializeJsonPretty(doc, file);
    file.close();

    if (!written)
    {
        Serial.println("Runtime config write failed");
        return false;
    }

    runtimeConfig = config;
    if (!runtimeConfig.weatherLocation.length())
    {
        runtimeConfig.weatherLocation = "ip";
    }

    Serial.println("Runtime config saved");
    return true;
}

void setRuntimeConfig(const RuntimeConfig &config)
{
    runtimeConfig = config;
    if (!runtimeConfig.weatherLocation.length())
    {
        runtimeConfig.weatherLocation = "ip";
    }
}
