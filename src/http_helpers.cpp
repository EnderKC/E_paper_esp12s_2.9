#include "http_helpers.h"

bool fetchHttpPayload(HTTPClient &httpClient, WiFiClient &tcpClient, const String &url, String &payload, const char *label)
{
    payload = "";

    if (!httpClient.begin(tcpClient, url))
    {
        Serial.printf("%s HTTP begin failed\n", label);
        return false;
    }

    const int httpCode = httpClient.GET();
    if (httpCode != HTTP_CODE_OK)
    {
        Serial.printf("%s HTTP failed, code=%d\n", label, httpCode);
        httpClient.end();
        return false;
    }

    payload = httpClient.getString();
    httpClient.end();

    Serial.printf("%s HTTP OK, payload length=%u\n", label, payload.length());
    return payload.length() > 0;
}
