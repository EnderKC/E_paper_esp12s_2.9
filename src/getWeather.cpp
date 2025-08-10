/**
 * @file getWeather.cpp
 * @brief 天气信息获取和显示模块实现
 * @details 通过心知天气API获取实时天气数据，并在电子墨水屏上显示天气图标、温度、位置和日期信息
 * @author ESP12S E-Paper Weather Display System
 * @version 2023.2.23
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "Ticker.h"
#include "getWeather.h"
#include "display.h"
#include "weather_img.h"
#include "My_information.h"

// ==================== API密钥定义 ====================

/**
 * @brief API密钥定义
 * @details 这些密钥在My_information.h中声明，在这里定义
 */
String weather_APIkey = "S6HxLK_QBsQ8jY3Rv"; // 知心天气APIkey
String driver_api_key = "SuBkUBbhV4zhLGD8R"; // 车辆限行APIkey

/**
 * @brief 限行API URL定义
 * @details 天津和北京的限行查询接口
 */
String driver_api_tianjin = "http://api.seniverse.com/v3/life/driving_restriction.json?key=" + driver_api_key + "&location=WWGQDCW6TBW1";
String driver_api_beijing = "http://api.seniverse.com/v3/life/driving_restriction.json?key=" + driver_api_key + "&location=WX4FBXXFKE4F";

// ==================== API配置与网络连接 ====================

/**
 * @brief 心知天气API请求URL定义
 * @details 使用IP定位获取当前位置的实时天气信息
 * - API提供商：心知天气 (Seniverse)
 * - 语言：简体中文
 * - 温度单位：摄氏度
 * - 定位方式：根据请求IP自动定位
 */
String weather_api = "http://api.seniverse.com/v3/weather/now.json?key="+weather_APIkey+"&location=ip&language=zh-Hans&unit=c";

/**
 * @brief HTTP客户端定义，用于发送API请求
 */
HTTPClient httpClient;

/**
 * @brief WiFi TCP连接客户端定义
 */
WiFiClient tcpClient;

// ==================== 定时器系统 ====================

/**
 * @brief 天气更新定时器标志位定义
 * @details 当值为1时表示需要更新天气信息
 *          初始值为1确保首次运行时立即获取天气数据
 */
int timer_weather = 1;

/**
 * @brief 天气信息定时器对象定义
 * @details 每20分钟触发一次天气信息更新
 */
Ticker weather_ticker;

/**
 * @brief 当前显示的月份记录定义
 * @details 用于检测日期变化，当检测到月份变化时触发天气和日期显示更新
 */
int display_day = 0;

// ==================== 外部变量声明 ====================

// 这些变量在其他源文件中定义，这里进行声明以便使用
extern struct tm timeinfo;  // 定义在getTime.cpp
extern int now_day;         // 定义在getTime.cpp

// ==================== 函数实现 ====================

/**
 * @brief 解析心知天气API返回的JSON数据
 * @param input 从API获取的原始JSON字符串
 * @param data 输出数组，用于存储解析后的天气信息
 * @details JSON数据结构解析：
 *          - results[0].location.name: 地理位置名称
 *          - results[0].now.text: 天气状况文字描述（如"晴"、"多云"等）
 *          - results[0].now.code: 天气状况代码（用于选择对应图标）
 *          - results[0].now.temperature: 当前温度（摄氏度）
 * 
 * @note 输出数组格式：data[0]=位置, data[1]=天气描述, data[2]=天气代码, data[3]=温度
 */
void analyze_weather_json(String input, String (&data)[4])
{
    // 分配512字节内存用于JSON文档解析
    StaticJsonDocument<512> doc;

    // 尝试解析JSON数据
    DeserializationError error = deserializeJson(doc, input);

    // 检查JSON解析是否成功
    if (error)
    {
        Serial.print(F("JSON解析失败: "));
        Serial.println(error.f_str());
        return;
    }

    // 获取第一个结果对象（通常只有一个结果）
    JsonObject results_0 = doc["results"][0];

    // 解析地理位置信息
    JsonObject results_0_location = results_0["location"];
    String location_id = results_0_location["id"];                           // 位置ID: "WW92M43YCQG0"
    String location_name = results_0_location["name"];                       // 位置名称: "邯郸"
    String location_country = results_0_location["country"];                 // 国家代码: "CN"
    String location_path = results_0_location["path"];                       // 完整路径: "邯郸,邯郸,河北,中国"
    String location_timezone = results_0_location["timezone"];               // 时区: "Asia/Shanghai"
    String location_timezone_offset = results_0_location["timezone_offset"]; // 时区偏移: "+08:00"

    // 解析当前天气信息
    JsonObject results_0_now = results_0["now"];
    String now_text = results_0_now["text"];               // 天气描述: "阴"
    String now_code = results_0_now["code"];               // 天气代码: "9"
    String now_temperature = results_0_now["temperature"]; // 当前温度: "6"

    // 最后更新时间（示例："2023-02-08T16:00:09+08:00"）
    String last_update = results_0["last_update"];

    // 将解析结果存储到输出数组
    data[0] = location_name;    // 位置名称
    data[1] = now_text;         // 天气状况
    data[2] = now_code;         // 天气代码
    data[3] = now_temperature;  // 当前温度
}

/**
 * @brief 主天气获取函数
 * @details 发送HTTP GET请求到心知天气API，获取当前地理位置的实时天气信息
 * 
 * 执行流程：
 * 1. 初始化HTTP客户端连接
 * 2. 发送GET请求到心知天气API
 * 3. 检查HTTP响应状态码
 * 4. 解析JSON响应数据
 * 5. 调用update_weather()更新显示界面
 * 6. 关闭连接释放资源
 * 
 * @note 函数包含Serial调试输出，方便开发者监控API请求状态
 */
void getWeather()
{
    // 初始化HTTP客户端，设置目标URL
    httpClient.begin(tcpClient, weather_api);
    
    // 发送GET请求
    int httpCode = httpClient.GET();

    // 检查HTTP响应状态
    if (httpCode == HTTP_CODE_OK)
    {
        // 获取服务器响应内容
        String Payload = httpClient.getString();
        
        // 输出Serial调试信息
        Serial.print("\r\n服务器响应状态码: ");
        Serial.println(httpCode);
        Serial.println("服务器响应内容: ");
        Serial.println(Payload);
        
        // 解析JSON数据
        String data[4];
        analyze_weather_json(Payload, data);
        
        // 输出解析结果用于调试
        Serial.println("解析结果:");
        Serial.println("位置: " + data[0]);
        Serial.println("天气: " + data[1]);
        Serial.println("代码: " + data[2]);
        Serial.println("温度: " + data[3] + "℃");
        
        // 更新显示界面
        update_weather(data);
    }
    else
    {
        // HTTP请求失败，输出错误信息
        Serial.print("\r\nHTTP请求失败，状态码: ");
        Serial.println(httpCode);
    }
    
    // 关闭ESP8266与服务器的连接，释放资源
    httpClient.end();
}

/**
 * @brief 天气模块初始化函数
 * @details 初始化天气显示区域并启动定时器
 * 
 * 初始化步骤：
 * 1. 设置电子墨水屏部分更新窗口（120x128像素）
 * 2. 清除天气显示区域（填充黑色背景）
 * 3. 启动天气更新定时器（20分钟间隔）
 * 
 * @note 显示区域坐标：(0,0) 至 (119,127)，位于屏幕左侧
 */
void weather_init()
{
    // 设置部分更新窗口，只更新天气区域以节省电力
    display.setPartialWindow(0, 0, 120, 128);
    
    // 填充黑色背景，清空天气显示区域
    display.fillRect(0, 0, 120, 128, GxEPD_BLACK);
    
    // 刷新显示缓存
    display.nextPage();
    
    // 启动天气更新定时器：60秒 * 20 = 20分钟间隔
    weather_ticker.attach(60*20, timer_weather_con);
}

/**
 * @brief 绘制天气显示区域的基础框架
 * @details 清空天气显示区域并绘制分割线，为天气内容显示做准备
 * 
 * 显示区域布局：
 * - 整体尺寸：120x128像素
 * - 水平分割线：y=60，分隔天气图标与信息显示区域
 * - 水平分割线：y=95，分隔天气信息与日期显示区域
 * 
 * @note 设置屏幕旋转为横屏模式（rotation=1）
 */
void display_weather()
{
    // 初始化显示库，准备绘制
    display.firstPage();
    
    // 设置部分更新窗口，只更新天气区域
    display.setPartialWindow(0, 0, 120, 128);
    
    // 填充黑色背景，清空显示区域
    display.fillRect(0, 0, 120, 128, GxEPD_BLACK);
    
    // 设置屏幕旋转：1=顺时针旋转90度（横屏模式）
    display.setRotation(1);
    
    // 绘制水平分割线：在y=60位置从左到右绘制一条线
    display.drawLine(0, 60, 119, 60, 1);
    
    // 绘制第二条水平分割线：在y=95位置从左到右
    display.drawFastHLine(0, 95, 119, 1);
}

/**
 * @brief 更新天气显示界面的主函数
 * @param data 天气数据数组 [0]=位置, [1]=天气描述, [2]=天气代码, [3]=温度
 * @details 根据天气代码选择对应的天气图标，并显示天气信息和日期
 * 
 * 显示布局：
 * - 天气图标：位于(30,0)，尺寸60x60像素
 * - 天气信息：位于(10,90)，包含位置+天气+温度
 * - 日期信息：位于(10,123)，包含月日+星期
 * 
 * 天气代码对应表：
 * - 0,2,38: 晴天（白天）
 * - 1,3: 晴天（夜晚）  
 * - 4-8: 多云
 * - 9: 阴天
 * - 10,13-18: 阵雨
 * - 11,12: 雷阵雨
 * - 20-25,37: 小雪
 * - 26,27: 浮尘
 * - 28,29,32,33,36: 大风
 * - 31,34,35: 雾
 * - 99: 未知天气
 */
void update_weather(String data[4])
{
    // 绘制天气显示区域的基础框架
    display_weather();
    
    // 设置屏幕旋转为横屏模式
    display.setRotation(1);
    
    // 清除天气图标显示区域（填充白色背景）
    display.fillRect(30, 0, 60, 60, GxEPD_WHITE);
    
    // 根据天气代码选择对应的天气图标
    if (data[2] == "0" || data[2] == "2" || data[2] == "38")
    {
        // 晴天（白天）图标
        display.drawInvertedBitmap(30, 0, qing_d, 60, 60, GxEPD_BLACK);
    }
    else if (data[2] == "1" || data[2] == "3")
    {
        // 晴天（夜晚）图标
        display.drawInvertedBitmap(30, 0, qing_n, 60, 60, GxEPD_BLACK);
    }
    else if (data[2] == "4" || data[2] == "5" || data[2] == "6" || data[2] == "7" || data[2] == "8")
    {
        // 多云天气图标
        display.drawInvertedBitmap(30, 0, duoyun, 60, 60, GxEPD_BLACK);
    }
    else if (data[2] == "9")
    {
        // 阴天图标
        display.drawInvertedBitmap(30, 0, yintian, 60, 60, GxEPD_BLACK);
    }
    else if (data[2] == "10" || data[2] == "13" || data[2] == "14" || data[2] == "15" || data[2] == "16" || data[2] == "17" || data[2] == "18")
    {
        // 阵雨天气图标
        display.drawInvertedBitmap(30, 0, zhenyu, 60, 60, GxEPD_BLACK);
    }
    else if (data[2] == "11" || data[2] == "12")
    {
        // 雷阵雨天气图标
        display.drawInvertedBitmap(30, 0, leizhenyu, 60, 60, GxEPD_BLACK);
    }
    else if (data[2] == "20" || data[2] == "21" || data[2] == "22" || data[2] == "23" || data[2] == "24" || data[2] == "25" || data[2] == "37")
    {
        // 小雪天气图标
        display.drawInvertedBitmap(30, 0, xiaoxue, 60, 60, GxEPD_BLACK);
    }
    else if (data[2] == "26" || data[2] == "27")
    {
        // 浮尘天气图标
        display.drawInvertedBitmap(30, 0, fuchen, 60, 60, GxEPD_BLACK);
    }
    else if (data[2] == "28" || data[2] == "29" || data[2] == "32" || data[2] == "33" || data[2] == "36")
    {
        // 大风天气图标
        display.drawInvertedBitmap(30, 0, feng, 60, 60, GxEPD_BLACK);
    }
    else if (data[2] == "31" || data[2] == "34" || data[2] == "35")
    {
        // 雾天图标
        display.drawInvertedBitmap(30, 0, wu, 60, 60, GxEPD_BLACK);
    }
    else if (data[2] == "99")
    {
        // 未知天气图标
        display.drawInvertedBitmap(30, 0, weizhi, 60, 60, GxEPD_BLACK);
    }

    // 设置字体颜色：前景色为白色，背景色为黑色
    u8g2Fonts.setForegroundColor(GxEPD_WHITE);
    u8g2Fonts.setBackgroundColor(GxEPD_BLACK);
    
    // 设置游标位置并显示天气信息："位置 天气描述 温度℃"
    u8g2Fonts.setCursor(10, 90);
    u8g2Fonts.print(data[0]+" "+data[1]+" "+data[3]+"℃");

    // 更新日期显示（仅在有有效时间信息时）
    if (timeinfo.tm_mday || now_day)
    {
        // 记录当前显示的月份，用于日期变化检测
        display_day = timeinfo.tm_mon;
        
        // 设置字体颜色
        u8g2Fonts.setForegroundColor(GxEPD_WHITE);
        u8g2Fonts.setBackgroundColor(GxEPD_BLACK);
        
        // 设置日期显示位置
        u8g2Fonts.setCursor(10, 108 + 15);
        
        // 根据星期值转换为中文显示
        String week_day;
        switch (timeinfo.tm_wday)
        {
        case 0: week_day = "周日"; break;
        case 1: week_day = "周一"; break;
        case 2: week_day = "周二"; break;
        case 3: week_day = "周三"; break;
        case 4: week_day = "周四"; break;
        case 5: week_day = "周五"; break;
        case 6: week_day = "周六"; break;
        default: week_day = "未知"; break;
        }
        
        // 显示日期信息："月份月 日期日 星期"
        u8g2Fonts.print(String(timeinfo.tm_mon+1)+"月 "+String(timeinfo.tm_mday)+"日 "+week_day);
    }

    // 刷新显示缓存，显示更新的内容
    display.nextPage();
}

/**
 * @brief 天气定时器回调函数
 * @details 由Ticker定时器在中断中调用，设置天气更新标志位
 *          每20分钟触发一次，通知主循环执行天气信息更新
 * 
 * @note 这是一个中断服务函数，应保持简洁快速
 *       只负责设置标志位，具体的天气获取工作由主循环中的getWeather()完成
 */
void timer_weather_con()
{
    Serial.println("天气定时器触发 - 准备更新天气信息！");
    timer_weather = 1;  // 设置天气更新标志位
}