/**
 * @file display.h
 * @brief 电子墨水屏显示初始化和管理模块
 * @details 提供电子墨水屏的基础初始化和全屏刷新功能
 * @author ESP12S E-Paper Weather Display System
 * @version 2023.2.23
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <GxEPD2_BW.h>
#include <U8g2_for_Adafruit_GFX.h>
#include "Ticker.h"

// ==================== 电子墨水屏设备配置 ====================

/**
 * @brief 电子墨水屏显示对象声明
 * @details 2.9寸黑白电子墨水屏，分辨率296x128
 *          使用SSD1608控制器（IL3820兼容）
 *          引脚连接：CS=15, DC=4, RST=2, BUSY=5
 */
extern GxEPD2_BW<GxEPD2_290, GxEPD2_290::HEIGHT> display;

/**
 * @brief U8g2中文字体显示对象声明
 * @details 用于在Adafruit GFX库上显示中文字符
 */
extern U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

// ==================== 外部变量声明 ====================

/**
 * @brief 外部定时器标志位声明
 * @details 这些变量在其他头文件中定义，这里进行外部声明以便使用
 */
extern int timer_weather;  // 天气更新标志位（定义在getWeather.h）
extern int timer_1;        // 时间更新标志位（定义在getTime.h）
extern int timer_gushi;    // 古诗更新标志位（定义在getGushi.h）
extern int timer_driver;   // 限行更新标志位（定义在getDriver.h）

// ==================== 函数声明 ====================

/**
 * @brief 电子墨水屏初始化函数
 * @details 初始化显示屏硬件、设置中文字体、配置显示参数
 */
void ePaper_init();

/**
 * @brief 全屏刷新函数
 * @details 执行黑-白-黑的全屏刷新序列，清除残留影像
 */
void ePaper_flash();

/**
 * @brief 全屏刷新定时器初始化
 * @details 启动定时器，每60分钟执行一次全屏刷新
 */
void ePaper_flash_init();

/**
 * @brief 全屏刷新定时器对象声明
 * @details 每60分钟触发一次全屏刷新，防止电子墨水屏出现残留影像
 */
extern Ticker ePaper_flash_ticker;

/**
 * @brief 全屏刷新标志位声明
 */
extern int timer_ePaper_flash;

#endif // DISPLAY_H