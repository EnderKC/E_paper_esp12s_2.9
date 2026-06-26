/**
 * @file getGushi.cpp
 * @brief 古诗词获取和显示模块实现
 * @details 通过今日诗词API获取随机古诗词，并在电子墨水屏上显示
 * @author ESP12S E-Paper Weather Display System
 * @version 2023.2.23
 */

#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include "Ticker.h"
#include "getGushi.h"
#include "display.h"
#include "ui_helpers.h"
#include "http_helpers.h"

// ==================== API配置定义 ====================

/**
 * @brief 今日诗词API请求URL定义
 */
String gushi_api = "http://v1.jinrishici.com/all.json";

/**
 * @brief HTTP客户端定义（古诗词专用）
 */
HTTPClient httpClient_gushi;

/**
 * @brief WiFi TCP连接客户端定义（古诗词专用）
 */
WiFiClient tcpClient_gushi;

// ==================== 定时器系统 ====================

/**
 * @brief 古诗词更新定时器对象定义
 * @details 每10分钟触发一次古诗词更新
 */
Ticker gushi_ticker;

/**
 * @brief 古诗词更新标志位定义
 * @details 当值为1时表示需要更新古诗词
 */
int timer_gushi = 1;

// ==================== 函数实现 ====================

/**
 * @brief 解析今日诗词API返回的JSON数据
 * @param input 从API获取的原始JSON字符串
 * @param data 输出数组，用于存储解析后的古诗词信息
 */
void analyze_gushi_json(String input, String (&data)[4])
{
    StaticJsonDocument<256> doc;

    DeserializationError error = deserializeJson(doc, input);

    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }

    String content = doc["content"];   // "泰山不要欺毫末，颜子无心羡老彭。"
    String origin = doc["origin"];     // "放言五首·其五"
    String author = doc["author"];     // "白居易"
    String category = doc["category"]; // "古诗文-山水-泰山"

    data[0] = content;
    data[1] = origin;
    data[2] = author;
    data[3] = category;
}

/**
 * @brief 主古诗词获取函数
 * @details 发送HTTP GET请求到今日诗词API，获取随机古诗词
 */
void getGushi()
{
    String payload;
    if (fetchHttpPayload(httpClient_gushi, tcpClient_gushi, gushi_api, payload, "gushi"))
    {
        String data[4];
        analyze_gushi_json(payload, data);
        update_gushi(data);
    }
}

/**
 * @brief 古诗词模块初始化函数
 * @details 启动古诗词获取定时器
 */
void gushi_init()
{
    gushi_ticker.attach(60 * 10, gushi_timer);
}

/**
 * @brief 更新古诗词显示界面
 * @param data 包含古诗词信息的字符串数组 [诗句, 标题, 作者, 分类]
 */
void update_gushi(String data[4])
{
    renderStatusLine(data[0]);
}

/**
 * @brief 古诗词定时器回调函数
 * @details 由Ticker定时器调用，设置古诗词更新标志位
 */
void gushi_timer()
{
    timer_gushi = 1;
}
