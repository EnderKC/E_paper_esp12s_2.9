#include <Arduino.h>
#include <time.h>
#include <WiFiClientSecureBearSSL.h>
#include "display.h"
#include "mqtt.h"
#include "ui_helpers.h"

namespace
{
const uint16_t mqtt_port = 8883;

static const char mqtt_ca_cert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH
MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI
2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx
1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQ
q2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5Wz
tCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQ
vIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQABo0IwQDAP
BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQUTiJUIBiV
5uNu5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAGBnKJRvDkhj6zHd6mcY
1Yl9PMWLSn/pvtsrF9+wX3N3KjITOYFnQoQj8kVnNeyIv/iPsGEMNKSuIEyExtv4
NeF22d+mQrvHRAiGfzZ0JFrabA0UWTW98kndth/Jsw1HKj2ZL7tcu7XUIOGZX1NG
Fdtom/DzMNU+MeKNhJ7jitralj41E6Vf8PlwUHBHQRFXGU7Aj64GxJUTFy8bJZ91
8rGOmaFvE7FBcf6IKshPECBV1/MUReXgRPTqh5Uykw7+U0b6LJ3/iyK5S9kJRaTe
pLiaWN0bfVKfjllDiIGknibVb63dDcY3fe0Dkhvld1927jyNxF1WW6LZZm6zNTfl
MrY=
-----END CERTIFICATE-----
)EOF";

BearSSL::WiFiClientSecure secureClient;
PubSubClient mqttClient(secureClient);
BearSSL::X509List trustAnchors(mqtt_ca_cert);

char mqttClientId[24];
unsigned long lastConnectAttempt = 0;
bool mqttConfigured = false;

bool isTimeValid()
{
    return time(nullptr) > 1700000000;
}

void buildClientId()
{
    snprintf(mqttClientId, sizeof(mqttClientId), "esp8266-%06x", ESP.getChipId());
}

void handleMqttMessage(char *topic, byte *payload, unsigned int length)
{
    String topicText = topic ? String(topic) : String();
    String message;
    message.reserve(length);
    for (unsigned int i = 0; i < length; ++i)
    {
        message += static_cast<char>(payload[i]);
    }

    Serial.printf("MQTT message on %s: %s\n", topicText.c_str(), message.c_str());

    if (topicText == mqtt_command_topic || topicText == mqtt_state_topic)
    {
        drawStatusPanel(121, 96, 175, 32, message);
        return;
    }

    drawStatusPanel(121, 96, 175, 32, topicText.length() ? topicText + ": " + message : message);
}

bool connectToMQTTBroker()
{
    if (!WiFi.isConnected() || !isTimeValid())
    {
        return false;
    }

    secureClient.setTrustAnchors(&trustAnchors);
    secureClient.setX509Time(time(nullptr));
    secureClient.setBufferSizes(512, 512);

    if (!mqttClient.connected())
    {
        const bool ok = mqttClient.connect(
            mqttClientId,
            mqtt_username,
            mqtt_password,
            mqtt_state_topic,
            1,
            true,
            "offline");

        if (!ok)
        {
            Serial.printf("MQTT connect failed, state=%d\n", mqttClient.state());
            return false;
        }
    }

    mqttClient.subscribe(mqtt_command_topic, 1);
    mqttClient.publish(mqtt_state_topic, "online", true);
    Serial.println("MQTT connected");
    return true;
}
}

void initMQTT()
{
    if (mqttConfigured)
    {
        return;
    }

    buildClientId();
    mqttClient.setServer(mqtt_broker, mqtt_port);
    mqttClient.setCallback(handleMqttMessage);
    mqttClient.setKeepAlive(30);
    mqttClient.setSocketTimeout(8);
    mqttClient.setBufferSize(512);
    mqttConfigured = true;
}

void mqttLoop()
{
    if (!mqttConfigured)
    {
        initMQTT();
    }

    if (mqttClient.connected())
    {
        mqttClient.loop();
        return;
    }

    if (millis() - lastConnectAttempt < 5000)
    {
        return;
    }

    lastConnectAttempt = millis();
    connectToMQTTBroker();
}
