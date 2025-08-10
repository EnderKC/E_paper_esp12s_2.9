/**
 * @file getGushi.h
 * @brief 古诗词获取和显示模块
 * @details 通过今日诗词API获取随机古诗词，并在电子墨水屏上显示
 * @author ESP12S E-Paper Weather Display System
 * @version 2023.2.23
 */

#ifndef GET_GUSHI_H
#define GET_GUSHI_H

#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include "Ticker.h"

// ==================== API配置声明 ====================

/**
 * @brief 今日诗词API请求URL声明
 */
extern String gushi_api;

/**
 * @brief HTTP客户端声明（古诗词专用）
 */
extern HTTPClient httpClient_gushi;

/**
 * @brief WiFi TCP连接客户端声明（古诗词专用）
 */
extern WiFiClient tcpClient_gushi;

// ==================== 定时器系统声明 ====================

/**
 * @brief 古诗词更新定时器对象声明
 * @details 每10分钟触发一次古诗词更新
 */
extern Ticker gushi_ticker;

/**
 * @brief 古诗词更新标志位声明
 * @details 当值为1时表示需要更新古诗词
 */
extern int timer_gushi;

// ==================== 函数声明 ====================

/**
 * @brief 古诗词模块初始化函数声明
 * @details 启动古诗词获取定时器
 */
void gushi_init();

/**
 * @brief 主古诗词获取函数声明
 * @details 发送HTTP GET请求到今日诗词API，获取随机古诗词
 */
void getGushi();

/**
 * @brief 解析今日诗词API返回的JSON数据函数声明
 * @param input 从API获取的原始JSON字符串
 * @param data 输出数组，用于存储解析后的古诗词信息
 */
void analyze_gushi_json(String input, String (&data)[4]);

/**
 * @brief 更新古诗词显示界面函数声明
 * @param data 包含古诗词信息的字符串数组 [诗句, 标题, 作者, 分类]
 */
void update_gushi(String data[4]);

/**
 * @brief 古诗词定时器回调函数声明
 * @details 由Ticker定时器调用，设置古诗词更新标志位
 */
void gushi_timer();

#endif // GET_GUSHI_H