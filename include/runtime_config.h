#ifndef RUNTIME_CONFIG_H
#define RUNTIME_CONFIG_H

#include <Arduino.h>

struct RuntimeConfig
{
    String weatherApiKey;
    String driverApiKey;
    String weatherLocation;
    String mqttBroker;
    String mqttUsername;
    String mqttPassword;
};

bool initRuntimeConfig();
const RuntimeConfig &getRuntimeConfig();
bool isRuntimeConfigComplete();
bool saveRuntimeConfig(const RuntimeConfig &config);
void setRuntimeConfig(const RuntimeConfig &config);

#endif // RUNTIME_CONFIG_H
