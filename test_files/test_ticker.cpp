
/**********************************************************************
项目名称/Project          : 零基础入门学用物联网
程序名称/Program name     : a_basic_tinker
团队/Team                : 太极创客团队 / Taichi-Maker (www.taichi-maker.com)
作者/Author              : CYNO朔
日期/Date（YYYYMMDD）     : 20200703
程序目的/Purpose          :
本程序旨在演示如何使用Ticker库来定时执行操作。
如需了解本程序的详细说明，请参考以下函数：
http://www.taichi-maker.com/homepage/esp8266-nodemcu-iot/iot-c/esp8266-tips/ticker/
-----------------------------------------------------------------------
其它说明 / Other Description：
本程序为太极创客团队制作的免费视频教程《零基础入门学用物联网 》中一部分。该教程系统的
向您讲述ESP8266的物联网应用相关的软件和硬件知识。以下是该教程目录页：
http://www.taichi-maker.com/homepage/esp8266-nodemcu-iot/
***********************************************************************/
#include <Arduino.h>
#include <Ticker.h>
#include <Adafruit_GFX.h>

Ticker ticker; // 建立Ticker用于实现定时功能
int count;     // 计数用变量
void sayHi();

void setup()
{
    Serial.begin(9600);
    pinMode(LED_BUILTIN, OUTPUT);

    // 每隔一秒钟调用sayHi函数一次，attach函数的第一个参数
    // 是控制定时间隔的变量。该参数的单位为秒。第二个参数是
    // 定时执行的函数名称。
    ticker.attach(1, sayHi);
}

void loop()
{
    // 用LED呼吸灯效果来演示在Tinker对象控制下，ESP8266可以定时
    // 执行其它任务
    for (int fadeValue = 0; fadeValue <= 1023; fadeValue += 5)
    {
        analogWrite(LED_BUILTIN, fadeValue);
        delay(10);
    }

    for (int fadeValue = 1023; fadeValue >= 0; fadeValue -= 5)
    {
        analogWrite(LED_BUILTIN, fadeValue);
        delay(10);
    }
    delay(3000);
}

// 在Tinker对象控制下，此函数将会定时执行。
void sayHi()
{
    count++;
    Serial.print("Hi ");
    Serial.println(count);
}