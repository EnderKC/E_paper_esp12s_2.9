#ifndef NETWORK_CONFIG_H
#define NETWORK_CONFIG_H

#include <Arduino.h>

// MQTT
extern const char *mqtt_broker;
extern const char *mqtt_state_topic;
extern const char *mqtt_command_topic;
extern const char *mqtt_username;
extern const char *mqtt_password;

// Weather / driver API
extern String weather_APIkey;
extern String driver_api_key;
extern String weather_api;
extern String driver_api_tianjin;
extern String driver_api_beijing;

void applyNetworkConfig();

#endif // NETWORK_CONFIG_H
