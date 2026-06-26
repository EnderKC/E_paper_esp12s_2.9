#ifndef HTTP_HELPERS_H
#define HTTP_HELPERS_H

#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

bool fetchHttpPayload(HTTPClient &httpClient, WiFiClient &tcpClient, const String &url, String &payload, const char *label);

#endif // HTTP_HELPERS_H
