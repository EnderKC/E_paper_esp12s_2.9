#include <Arduino.h>
#include <ESP8266WiFi.h> // 本程序使用ESP8266WiFi库
#include <Adafruit_GFX.h>

const char* ssid     = "请用C++输出wifi";     // WIFI账户
const char* password = "cout<<\"wifi\";"; // WIFI密码
void setup()
{
    Serial.begin(9600);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(100);
    }
    Serial.println(WiFi.localIP());
}
void loop()
{
}