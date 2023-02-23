#include <Arduino.h>
#include <WiFiUdp.h>
#include "Ticker.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h> // get请求
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <Adafruit_GFX.h>          //绘制图形
#include <GxEPD2_BW.h>             //e-paper显示库
#include <U8g2_for_Adafruit_GFX.h> //输出汉字
#include "My_information.h"
#include "display.h"
#include "connWIFI.h"
#include "getTime.h"
#include "weather_img.h"
#include "getWeather.h"
#include "getGushi.h"
#include "getDriver.h"

#define switch_WD 1  // 切换 古诗 车辆限行显示 0 为古诗 1 为车辆限行（天津）

/*
=========更新日志==========
<2023.2.16>
1. 完善古诗显示有时会显示不出 原因：解析json时分配空间太小
2. 更改 天气\日期 字体显示效果
3. 增加字库显示
4. 改进天气显示方案
<2023.2.17>
5. 更好的WIFI配置界面
6. 优化部分代码
<2023.2.22>
7. 更改文件目录结构
8. 添加天津限行显示
<2023.2.23>
9. 更好的日期刷新（实时检测显示日期与网络日期是否匹配）
10. 更改车辆限行的部分逻辑代码
11. main文件添加临时的切换显示功能
*/

/*初始化设置*/
void setup()
{
    ePaper_init();
    initWifiManager();
    time_init();
    weather_init();  // 20分钟刷新显示
    gushi_init();  // 10分钟刷新显示
    driver_init(); // 120分钟刷新显示
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
    if (timer_weather == 1 || now_mon != display_day) // 修正时间显示
    {
        timer_weather = 0;
        // display_weather();
        getWeather(); // 天气更新
    }
#if switch_WD == 0
    if (timer_gushi == 1)
    {
        timer_gushi = 0;
        getGushi(); // 古诗更新
    }
#endif
#if switch_WD == 1
    if (timer_driver == 1)
    {
        timer_driver = 0;
        getDriver(); // 车辆更新
    }
#endif
}
