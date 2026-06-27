#ifndef MQTT_H
#define MQTT_H

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "network_config.h"

void initMQTT();
void resetMQTT();
void mqttLoop();

#endif
