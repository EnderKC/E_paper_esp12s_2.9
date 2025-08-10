/**
 * @file getDriver.h
 * @brief 车辆限行信息获取和显示模块头文件
 * @details 通过心知天气API获取车辆限行信息，并在电子墨水屏上显示
 * @author ESP12S E-Paper Weather Display System
 * @version 2023.2.23
 */

#ifndef GET_DRIVER_H
#define GET_DRIVER_H

#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include "Ticker.h"

// ==================== API配置声明 ====================

/**
 * @brief 天津限行API请求URL声明
 * @details 使用心知天气API获取天津地区车辆限行信息
 */
extern String driver_api_tianjin;

/**
 * @brief 北京限行API请求URL声明
 * @details 使用心知天气API获取北京地区车辆限行信息  
 */
extern String driver_api_beijing;

/**
 * @brief HTTP客户端声明（限行专用）
 * @details 专门用于限行API请求的HTTP客户端实例
 */
extern HTTPClient httpClient_driver;

/**
 * @brief WiFi TCP连接客户端声明（限行专用）
 * @details 专门用于限行API请求的TCP连接客户端
 */
extern WiFiClient tcpClient_driver;

// ==================== 定时器系统声明 ====================

/**
 * @brief 限行更新定时器对象声明
 * @details 每120分钟触发一次限行信息更新
 */
extern Ticker driver_ticker;

/**
 * @brief 限行更新标志位声明
 * @details 当值为1时表示需要更新限行信息，初始值为1以便首次运行时立即获取数据
 */
extern int timer_driver;

// ==================== 函数声明 ====================

/**
 * @brief 限行模块初始化函数
 * @details 启动限行信息获取定时器，每120分钟（2小时）触发一次更新
 */
void driver_init();

/**
 * @brief 主限行信息获取函数
 * @details 发送HTTP GET请求到心知天气API，获取天津地区限行信息
 */
void getDriver();

/**
 * @brief 解析限行API返回的JSON数据函数
 * @param input 从API获取的原始JSON字符串
 * @param data 输出数组，用于存储解析后的限行信息
 *             data[0]: 第一个限行尾号
 *             data[1]: 第二个限行尾号
 *             data[2]: 限行备注信息（如"尾号限行"）
 *             data[3]: 城市名称
 */
void analyze_driver_json(String input, String (&data)[4]);

/**
 * @brief 更新限行显示界面函数
 * @param data 包含限行信息的字符串数组
 * @details 在电子墨水屏右下角区域显示限行信息，使用部分刷新提高响应速度
 */
void update_driver(String data[4]);

/**
 * @brief 限行定时器回调函数
 * @details 由Ticker定时器调用，设置限行更新标志位
 *          此函数在定时器中断中执行，仅设置标志位，实际更新在主循环中进行
 */
void driver_timer();

#endif // GET_DRIVER_H