/**
 * @file getWeather.h
 * @brief 天气信息获取和显示模块
 * @details 通过心知天气API获取实时天气数据，并在电子墨水屏上显示天气图标、温度、位置和日期信息
 * @author ESP12S E-Paper Weather Display System
 * @version 2023.2.23
 */

#ifndef GET_WEATHER_H
#define GET_WEATHER_H

#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include "Ticker.h"
#include <time.h>

// ==================== HTTP客户端声明 ====================

/**
 * @brief 心知天气API请求URL声明
 * @details 使用IP定位获取当前位置的实时天气信息
 */
extern String weather_api;

/**
 * @brief HTTP客户端声明
 */
extern HTTPClient httpClient;

/**
 * @brief WiFi TCP连接客户端声明
 */
extern WiFiClient tcpClient;

// ==================== 定时器系统声明 ====================

/**
 * @brief 天气更新定时器标志位声明
 * @details 当值为1时表示需要更新天气信息
 *          初始值为1确保首次运行时立即获取天气数据
 */
extern int timer_weather;

/**
 * @brief 天气信息定时器对象声明
 * @details 每20分钟触发一次天气信息更新
 */
extern Ticker weather_ticker;

/**
 * @brief 当前显示的月份记录声明
 * @details 用于检测日期变化，当检测到月份变化时触发天气和日期显示更新
 */
extern int display_day;

// ==================== 外部变量声明 ====================

/**
 * @brief 外部时间信息声明
 */
extern struct tm timeinfo;
extern int now_day;

// ==================== 函数声明 ====================

/**
 * @brief 天气模块初始化函数声明
 * @details 初始化天气显示区域并启动定时器
 */
void weather_init();

/**
 * @brief 主天气获取函数声明
 * @details 发送HTTP GET请求到心知天气API，获取当前地理位置的实时天气信息
 */
void getWeather();

/**
 * @brief 解析心知天气API返回的JSON数据函数声明
 * @param input 从API获取的原始JSON字符串
 * @param data 输出数组，用于存储解析后的天气信息
 */
void analyze_weather_json(String input, String (&data)[4]);

/**
 * @brief 更新天气显示界面函数声明
 * @param data 包含天气信息的字符串数组 [位置, 天气描述, 天气代码, 温度]
 */
void update_weather(String data[4]);

/**
 * @brief 绘制天气显示区域的基础框架函数声明
 * @details 清空天气显示区域并绘制分隔线
 */
void display_weather();

/**
 * @brief 天气定时器回调函数声明
 * @details 由Ticker定时器调用，设置天气更新标志位
 */
void timer_weather_con();

#endif // GET_WEATHER_H