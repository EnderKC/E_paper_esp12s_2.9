/**
 * @file getTime.cpp
 * @brief 时间同步和数字时钟显示模块实现
 * @details 通过NTP服务器获取网络时间，在电子墨水屏上显示数字时钟
 *          采用七段显示器风格的自绘数字，提供清晰的时间显示效果
 * @author ESP12S E-Paper Weather Display System
 * @version 2023.2.23
 */

#include <Arduino.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <time.h>
#include "Ticker.h"
#include "getTime.h"
#include "display.h"

// ==================== 时间数据结构和变量定义 ====================

/**
 * @brief 系统时间信息结构体定义
 * @details 存储从NTP服务器获取的时间信息
 */
struct tm timeinfo;

/**
 * @brief 时间更新定时器对象定义
 * @details 每10秒触发一次时间检查
 */
Ticker time_ticker;

/**
 * @brief 时间更新标志位定义
 * @details 值为1时表示需要检查时间更新
 *          初始值为1确保首次运行时立即显示时间
 */
int timer_1 = 1;

/**
 * @brief 当前显示的时间缓存变量定义
 * @details 用于与网络时间比较，避免不必要的屏幕刷新
 * - now_hour: 当前显示的小时
 * - now_min: 当前显示的分钟  
 * - now_week: 当前显示的星期
 * - now_mon: 当前显示的月份
 * - now_day: 当前显示的日期
 */
int now_hour = 0;
int now_min = 0;
int now_week = 0;
int now_mon = 0;
int now_day = 0;

// ==================== 函数实现 ====================

/**
 * @brief 时间初始化函数
 * @details 初始化NTP时间同步和时间显示区域
 */
void time_init()
{
    // timeClient.begin();
    // timeClient.update();
    configTime(8 * 3600, 0, NTP1, NTP2, NTP3);
    getLocalTime(&timeinfo);
    /*区域初始化*/
    display.fillScreen(GxEPD_WHITE);
    display.setPartialWindow(119, 0, 177, 90);
    display.fillRect(119, 0, 177, 90, GxEPD_BLACK);
    // display.updateWindow(119, 0, 177, 90, true);
    display.nextPage();
    delay(500); // 程序暂停2秒
    display.fillRect(119, 0, 177, 90, GxEPD_WHITE);
    display.nextPage();
    delay(500); // 程序暂停2秒
    display.drawRect(119, 0, 177, 90, GxEPD_BLACK);
    display.nextPage();
    time_ticker.attach(10, time_timer); // 每隔10s更新一次时间
}

/**
 * @brief 时间定时器回调函数
 * @details 每10秒触发一次时间检查
 */
void time_timer()
{
    Serial.println("update time !");
    timer_1 = 1;
}

/**
 * @brief 比较网络时间与当前显示时间
 * @details 只有在时间发生变化时才更新显示，降低功耗
 */
void compare_time()
{
    /*更新时间*/
    getLocalTime(&timeinfo);
    if (timeinfo.tm_hour != now_hour || timeinfo.tm_min != now_min)
    {
        time_update();
        now_hour = timeinfo.tm_hour;
        now_min = timeinfo.tm_min;
        now_mon = timeinfo.tm_mon;
    }
}

/**
 * @brief 强制更新时间显示函数
 * @details 强制刷新时间显示，不检查时间变化
 */
void force_update_time()
{
    time_update();
    now_hour = timeinfo.tm_hour;
    now_min = timeinfo.tm_min;
    now_mon = timeinfo.tm_mon;
}

/**
 * @brief 更新时间显示界面
 * @details 在指定区域绘制HH:MM格式的数字时钟
 */
void time_update()
{
    display.setPartialWindow(119, 0, 177, 96);
    display.firstPage();
    display.drawRect(119, 0, 177, 96, GxEPD_BLACK);
    int hours = timeinfo.tm_hour;
    int min = timeinfo.tm_min;
    do
    {
        /*前两位数字*/
        int hours_f, hours_b;
        if (hours < 10)
        {
            hours_f = 0;
            hours_b = hours;
        }
        else
        {
            hours_f = hours / 10;
            hours_b = hours % 10;
        }
        num_time(124, 17, hours_f);
        num_time(159, 17, hours_b);
        /*中间点点*/
        display.fillCircle(207, 37, 3, GxEPD_BLACK);
        display.fillCircle(207, 56, 3, GxEPD_BLACK);
        /*后两位数字*/
        int min_f, min_b;
        if (min < 10)
        {
            min_f = 0;
            min_b = min;
        }
        else
        {
            min_f = min / 10;
            min_b = min % 10;
        }
        num_time(225, 17, min_f);
        num_time(260, 17, min_b);
    } while (display.nextPage());

    Serial.println(hours);
    Serial.println(min);
}

/**
 * @brief 绘制单个数字的七段显示器样式
 * @param x 数字显示的X坐标起始位置
 * @param y 数字显示的Y坐标起始位置  
 * @param num 要显示的数字(0-9)
 */
void num_time(int x, int y, int num)
{
    /*完整测试*/
    /*
    display.fillRoundRect(x + 5, y, 20, 5, 2, GxEPD_BLACK);
    display.fillRoundRect(x + 25, y + 5, 5, 20, 2, GxEPD_BLACK);
    display.fillRoundRect(x + 25, y + 30, 5, 20, 2, GxEPD_BLACK);
    display.fillRoundRect(x + 5, y + 50, 20, 5, 2, GxEPD_BLACK);
    display.fillRoundRect(x, y + 30, 5, 20, 2, GxEPD_BLACK);
    display.fillRoundRect(x, y + 5, 5, 20, 2, GxEPD_BLACK);
    display.fillRoundRect(x + 5, y + 25, 20, 5, 2, GxEPD_BLACK);
    */
    switch (num)
    {
    case 0:
        display.fillRoundRect(x + 5, y, 20, 5, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 25, y + 5, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 25, y + 30, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 5, y + 50, 20, 5, 2, GxEPD_BLACK);
        display.fillRoundRect(x, y + 30, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x, y + 5, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 5, y + 25, 20, 5, 2, GxEPD_WHITE);
        break;
    case 1:
        display.fillRoundRect(x + 5, y, 20, 5, 2, GxEPD_WHITE);
        display.fillRoundRect(x + 25, y + 5, 5, 20, 2, GxEPD_WHITE);
        display.fillRoundRect(x + 25, y + 30, 5, 20, 2, GxEPD_WHITE);
        display.fillRoundRect(x + 5, y + 50, 20, 5, 2, GxEPD_WHITE);
        display.fillRoundRect(x, y + 30, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x, y + 5, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 5, y + 25, 20, 5, 2, GxEPD_WHITE);
        break;
    case 2:
        display.fillRoundRect(x + 5, y, 20, 5, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 25, y + 5, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 25, y + 30, 5, 20, 2, GxEPD_WHITE);
        display.fillRoundRect(x + 5, y + 50, 20, 5, 2, GxEPD_BLACK);
        display.fillRoundRect(x, y + 30, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x, y + 5, 5, 20, 2, GxEPD_WHITE);
        display.fillRoundRect(x + 5, y + 25, 20, 5, 2, GxEPD_BLACK);
        break;
    case 3:
        display.fillRoundRect(x + 5, y, 20, 5, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 25, y + 5, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 25, y + 30, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 5, y + 50, 20, 5, 2, GxEPD_BLACK);
        display.fillRoundRect(x, y + 30, 5, 20, 2, GxEPD_WHITE);
        display.fillRoundRect(x, y + 5, 5, 20, 2, GxEPD_WHITE);
        display.fillRoundRect(x + 5, y + 25, 20, 5, 2, GxEPD_BLACK);
        break;
    case 4:
        display.fillRoundRect(x + 5, y, 20, 5, 2, GxEPD_WHITE);
        display.fillRoundRect(x + 25, y + 5, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 25, y + 30, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 5, y + 50, 20, 5, 2, GxEPD_WHITE);
        display.fillRoundRect(x, y + 30, 5, 20, 2, GxEPD_WHITE);
        display.fillRoundRect(x, y + 5, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 5, y + 25, 20, 5, 2, GxEPD_BLACK);
        break;
    case 5:
        display.fillRoundRect(x + 5, y, 20, 5, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 25, y + 5, 5, 20, 2, GxEPD_WHITE);
        display.fillRoundRect(x + 25, y + 30, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 5, y + 50, 20, 5, 2, GxEPD_BLACK);
        display.fillRoundRect(x, y + 30, 5, 20, 2, GxEPD_WHITE);
        display.fillRoundRect(x, y + 5, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 5, y + 25, 20, 5, 2, GxEPD_BLACK);
        break;
    case 6:
        display.fillRoundRect(x + 5, y, 20, 5, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 25, y + 5, 5, 20, 2, GxEPD_WHITE);
        display.fillRoundRect(x + 25, y + 30, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 5, y + 50, 20, 5, 2, GxEPD_BLACK);
        display.fillRoundRect(x, y + 30, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x, y + 5, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 5, y + 25, 20, 5, 2, GxEPD_BLACK);
        break;
    case 7:
        display.fillRoundRect(x + 5, y, 20, 5, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 25, y + 5, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 25, y + 30, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 5, y + 50, 20, 5, 2, GxEPD_WHITE);
        display.fillRoundRect(x, y + 30, 5, 20, 2, GxEPD_WHITE);
        display.fillRoundRect(x, y + 5, 5, 20, 2, GxEPD_WHITE);
        display.fillRoundRect(x + 5, y + 25, 20, 5, 2, GxEPD_WHITE);
        break;
    case 8:
        display.fillRoundRect(x + 5, y, 20, 5, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 25, y + 5, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 25, y + 30, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 5, y + 50, 20, 5, 2, GxEPD_BLACK);
        display.fillRoundRect(x, y + 30, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x, y + 5, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 5, y + 25, 20, 5, 2, GxEPD_BLACK);
        break;
    case 9:
        display.fillRoundRect(x + 5, y, 20, 5, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 25, y + 5, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 25, y + 30, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 5, y + 50, 20, 5, 2, GxEPD_BLACK);
        display.fillRoundRect(x, y + 30, 5, 20, 2, GxEPD_WHITE);
        display.fillRoundRect(x, y + 5, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 5, y + 25, 20, 5, 2, GxEPD_BLACK);
        break;
    default:
        break;
    }
}

/**
 * @brief 系统问候函数
 * @details 预留的问候功能接口
 */
void sayHi()
{
    // 预留函数，暂无实现
}