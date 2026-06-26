/**
 * @file main.cpp
 * @brief ESP8266电子墨水屏天气显示系统主程序
 * @details 基于ESP8266微控制器的电子墨水屏信息显示设备，支持天气、时间、诗词/限行信息显示
 * @author ESP12S E-Paper Weather Display System
 * @version 2023.2.23
 */

#include <Arduino.h>
#include <WiFiUdp.h>                // UDP网络通信库
#include "Ticker.h"                  // 定时器任务调度库
#include <ESP8266WiFi.h>            // ESP8266 WiFi连接库
#include <ESP8266HTTPClient.h>      // HTTP客户端，用于API请求
#include <WiFiManager.h>            // WiFi连接管理库，提供Web配置界面
#include <ArduinoJson.h>            // JSON数据解析库
#include <NTPClient.h>              // 网络时间协议客户端
#include <Adafruit_GFX.h>           // Adafruit图形绘制库
#include <GxEPD2_BW.h>             // 电子墨水屏显示驱动库
#include <U8g2_for_Adafruit_GFX.h> // 中文字体显示库

// 项目自定义头文件
#include "My_information.h"         // 个人配置信息（API密钥等）
#include "display.h"               // 显示屏初始化和基础显示函数
#include "connWIFI.h"              // WiFi连接配置
#include "getTime.h"               // 时间获取和显示模块
#include "weather_img.h"           // 天气图标数据
#include "getWeather.h"            // 天气信息获取模块
#include "getGushi.h"              // 古诗词获取显示模块
#include "getDriver.h"             // 车辆限行信息获取模块
#include "mqtt.h"                  // MQTT通信模块
#include "app_config.h"
#include "ota_update.h"

/**
 * @brief 系统版本更新日志
 * @details 记录主要功能更新和问题修复历史
 * 
 * <2023.2.16>
 * 1. 完善古诗显示有时会显示不出 原因：解析JSON时分配空间太小
 * 2. 更改天气/日期字体显示效果
 * 3. 增加字库显示
 * 4. 改进天气显示方案
 * 
 * <2023.2.17>
 * 5. 更好的WiFi配置界面
 * 6. 优化部分代码
 * 
 * <2023.2.22>
 * 7. 更改文件目录结构
 * 8. 添加天津限行显示
 * 
 * <2023.2.23>
 * 9. 更好的日期刷新（实时检测显示日期与网络日期是否匹配）
 * 10. 更改车辆限行的部分逻辑代码
 * 11. main文件添加临时的切换显示功能
 */

/**
 * @brief 系统初始化函数
 * @details 按顺序初始化各个功能模块，建立完整的信息显示系统
 * 
 * 初始化流程：
 * 1. 电子墨水屏硬件初始化
 * 2. WiFi连接管理器初始化（支持Web配置页面）
 * 3. 时间同步服务初始化
 * 4. 天气信息定时器初始化（20分钟更新周期）
 * 5. 古诗词定时器初始化（10分钟更新周期）
 * 6. 车辆限行信息定时器初始化（2小时更新周期）
 */
void setup()
{
    ePaper_init();    // 初始化电子墨水屏显示模块
    initWifiManager(); // 初始化WiFi管理器，提供连接配置界面
    checkForFirmwareUpdate(true); // 联网后主动检查固件更新
    time_init();      // 初始化NTP时间同步，每10秒检查一次
    weather_init();   // 初始化天气信息获取，20分钟刷新一次
    ePaper_flash_init();// 初始化墨水屏全局刷新，30分钟刷新一次
#if APP_MODE == 0
    gushi_init();     // 初始化古诗词获取，10分钟刷新一次
#endif
#if APP_MODE == 1
    driver_init();    // 初始化限行信息获取，120分钟刷新一次
#endif
#if APP_MODE == 2
    initMQTT();       // 初始化MQTT连接
#endif
}

/**
 * @brief 主循环函数 - 基于事件的任务调度器
 * @details 采用非阻塞的轮询方式检查定时器标志位，实现多任务协作调度
 *          这种设计避免了使用delay()造成的阻塞，提高了系统响应性
 * 
 * 调度逻辑：
 * - timer_1: 每10秒检查时间显示更新
 * - timer_weather: 每20分钟更新天气信息，或检测到日期变化时立即更新
 * - timer_gushi: 每10分钟更新古诗词（仅在古诗模式时）
 * - timer_driver: 每2小时更新限行信息（仅在限行模式时）
 * 
 * @note 所有定时器使用Ticker库在中断中设置标志位，主循环轮询检查
 */
void loop()
{
    checkForFirmwareUpdate();

    // 时间显示更新检查（频率：每10秒）
    if (timer_1 == 1)
    {
        timer_1 = 0;          // 重置定时器标志
        compare_time();       // 检查并更新时间显示
    }
    // 强制更新时间
    if (timer_1 == 2) 
    {
        timer_1 = 0;
        force_update_time();
    }
    // 天气信息更新检查（频率：每20分钟或日期变化时）
    if (timer_weather == 1 || (now_mon * 100 + now_day) != display_day) 
    {
        timer_weather = 0;    // 重置定时器标志
        getWeather();         // 获取并显示天气信息
    }
    
    // 墨水屏全局刷新（频率：每30分钟）
    if (timer_ePaper_flash == 1)
    {
        timer_ePaper_flash = 0;
        ePaper_flash();
    }
    
#if APP_MODE == 0
    // 古诗词更新检查（仅古诗模式，频率：每10分钟）
    if (timer_gushi == 1)
    {
        timer_gushi = 0;      // 重置定时器标志  
        getGushi();           // 获取并显示古诗词
    }
#endif

#if APP_MODE == 1
    // 车辆限行信息更新检查（仅限行模式，频率：每2小时）
    if (timer_driver == 1)
    {
        timer_driver = 0;     // 重置定时器标志
        getDriver();          // 获取并显示限行信息
    }
#endif

#if APP_MODE == 2
    mqttLoop();
#endif
}
