#include <Arduino.h>
#include <WiFiUdp.h>
#include "Ticker.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h> // get请求
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <Adafruit_GFX.h>          //绘制图形
#include <GxEPD2_BW.h>             //e-paper显示库
#include <U8g2_for_Adafruit_GFX.h> //输出汉字
#include <vector>
#include "display.h"
#include "connWIFI.h"
#include "getTime.h"
#include "weather_img.h"
#include "getWeather.h"
#include "getGushi.h"

/*
=========更新日志==========
<2023.2.16> 
1. 完善古诗显示有时会显示不出 原因：解析json时分配空间太小
2. 更改 天气\日期 字体显示效果
3. 增加字库显示
4. 改进天气显示方案
*/

// Ticker ticker;// 建立Ticker用于实现定时功能

/*初始化设置*/
void setup()
{
    ePaper_init();
    connectNetwork();
    time_init();
    weather_init();
    gushi_init();
}

/*循环执行*/
void loop()
{
    /*优雅的控制程序运行*/
    if (timer_1 == 1)
    {
        timer_1 = 0;
        compare_time(); // 时间更新
    }
    if (timer_weather == 1)
    {
        timer_weather = 0;
        // display_weather();
        getWeather(); // 天气更新
    }
    if (timer_gushi == 1)
    {
        timer_gushi = 0;
        getGushi(); // 古诗更新
    }
}
