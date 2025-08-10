/**
 * @file getDriver.cpp
 * @brief 车辆限行信息获取和显示模块实现
 * @details 通过心知天气API获取车辆限行信息，并在电子墨水屏上显示
 * @author ESP12S E-Paper Weather Display System
 * @version 2023.2.23
 */

#include "getDriver.h"
#include "My_information.h"
#include "display.h"
#include <ArduinoJson.h>

// ==================== 外部变量声明 ====================

/**
 * @brief 外部变量声明
 * @details 这些变量在getWeather.cpp中定义，这里进行外部声明
 */
extern String driver_api_tianjin; // 天津限行API URL（定义在getWeather.cpp）
extern String driver_api_beijing; // 北京限行API URL（定义在getWeather.cpp）

/**
 * @brief HTTP客户端对象（限行专用）
 * @details 专门用于限行API请求的HTTP客户端实例
 */
HTTPClient httpClient_driver;

/**
 * @brief WiFi TCP连接客户端（限行专用）
 * @details 专门用于限行API请求的TCP连接客户端
 */
WiFiClient tcpClient_driver;

// ==================== 定时器系统实现 ====================

/**
 * @brief 限行更新定时器对象
 * @details 每120分钟触发一次限行信息更新
 */
Ticker driver_ticker;

/**
 * @brief 限行更新标志位
 * @details 当值为1时表示需要更新限行信息，初始值为1以便首次运行时立即获取数据
 */
int timer_driver = 1;

// ==================== 函数实现 ====================

/**
 * @brief 解析限行API返回的JSON数据
 * @param input 从API获取的原始JSON字符串
 * @param data 输出数组，用于存储解析后的限行信息
 *             data[0]: 第一个限行尾号
 *             data[1]: 第二个限行尾号  
 *             data[2]: 限行备注信息（如"尾号限行"）
 *             data[3]: 城市名称
 * @details 使用ArduinoJson库解析心知天气API返回的限行信息JSON数据
 */
void analyze_driver_json(String input, String (&data)[4])
{
    // 创建JSON文档对象，分配1536字节内存
    StaticJsonDocument<1536> doc;

    // 反序列化JSON字符串
    DeserializationError error = deserializeJson(doc, input);

    // 检查JSON解析是否成功
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }

    // 解析位置信息
    JsonObject location = doc["results"][0]["location"];
    String location_id = location["id"];                           // "WWGQDCW6TBW1"
    String location_name = location["name"];                       // "天津"
    String location_country = location["country"];                 // "CN"
    String location_path = location["path"];                       // "天津,天津,中国"
    String location_timezone = location["timezone"];               // "Asia/Shanghai"
    String location_timezone_offset = location["timezone_offset"]; // "+08:00"

    // 解析限行基本信息
    JsonObject restriction = doc["results"][0]["restriction"];
    String restriction_penalty = restriction["penalty"];      // 违规处罚信息
    String restriction_region = restriction["region"];        // "外环线(不含)以内道路"
    String restriction_time = restriction["time"];            // "每日7时至19时"
    String restriction_remarks = restriction["remarks"];      // 备注信息

    // 解析限行详细规则数组
    JsonArray restriction_limits = restriction["limits"];

    // 解析今日限行信息（第一个元素）
    JsonObject restriction_limits_0 = restriction_limits[0];
    String restriction_limits_0_date = restriction_limits_0["date"];
    String restriction_limits_0_plates_0 = restriction_limits_0["plates"][0]; // 第一个限行尾号
    String restriction_limits_0_plates_1 = restriction_limits_0["plates"][1]; // 第二个限行尾号
    String restriction_limits_0_memo = restriction_limits_0["memo"];          // "尾号限行"

    // 解析明日限行信息（第二个元素）
    JsonObject restriction_limits_1 = restriction_limits[1];
    String restriction_limits_1_date = restriction_limits_1["date"];
    String restriction_limits_1_plates_0 = restriction_limits_1["plates"][0];
    String restriction_limits_1_plates_1 = restriction_limits_1["plates"][1];
    String restriction_limits_1_memo = restriction_limits_1["memo"]; // "尾号限行"

    // 解析第三天限行信息（第三个元素，通常为周末不限行）
    JsonObject restriction_limits_2 = restriction_limits[2];
    String restriction_limits_2_date = restriction_limits_2["date"];
    String restriction_limits_2_memo = restriction_limits_2["memo"]; // "周末不限行"

    // 将解析结果存储到输出数组
    data[0] = restriction_limits_0_plates_0; // 第一个限行尾号
    data[1] = restriction_limits_0_plates_1; // 第二个限行尾号
    data[2] = restriction_limits_0_memo;     // 限行类型备注
    data[3] = location_name;                 // 城市名称

    // 直接调用显示更新函数
    update_driver(data);
}

/**
 * @brief 主限行信息获取函数
 * @details 发送HTTP GET请求到心知天气API，获取天津地区限行信息
 *          成功获取后调用analyze_driver_json函数解析数据并显示
 */
void getDriver()
{
    // 初始化HTTP客户端连接
    httpClient_driver.begin(tcpClient_driver, driver_api_tianjin);
    
    // 发送GET请求
    int httpCode = httpClient_driver.GET();

    // 检查HTTP响应状态
    if (httpCode == HTTP_CODE_OK)
    {
        // 获取服务器响应内容
        String Payload = httpClient_driver.getString();
        
        // 打印调试信息
        Serial.print("\r\nServer Respose Code: ");
        Serial.println(httpCode);
        Serial.println("Server Response Payload: ");
        Serial.println(Payload);
        
        // 解析JSON数据
        String data[4];
        analyze_driver_json(Payload, data);
        
        // 调试输出解析结果
        Serial.println(data[0]);
        Serial.println(data[1]);
        
        // 判断今日是否限行
        if (data[0].length() > 0)
        {
            Serial.println("今日限行");
            update_driver(data);
        }
        else
        {
            Serial.println("今日不限行");
        }
    }
    else
    {
        // 打印HTTP错误码
        Serial.print("\r\nServer Respose Code: ");
        Serial.println(httpCode);
    }
    
    // 关闭HTTP连接
    httpClient_driver.end();
}

/**
 * @brief 更新限行显示界面
 * @param data 包含限行信息的字符串数组
 *             data[0]: 第一个限行尾号
 *             data[1]: 第二个限行尾号
 *             data[2]: 限行备注信息
 *             data[3]: 城市名称
 * @details 在电子墨水屏右下角区域显示限行信息，使用部分刷新提高响应速度
 */
void update_driver(String data[4])
{
    // 设置部分刷新窗口（位置：x=121, y=97, 宽度=175, 高度=31）
    display.setPartialWindow(121, 97, 175, 31);
    
    // 清除显示区域（填充白色背景）
    display.fillRect(121, 96, 175, 32, GxEPD_WHITE);
    
    // 设置字体颜色
    u8g2Fonts.setForegroundColor(GxEPD_BLACK); // 前景色：黑色
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE); // 背景色：白色
    
    // 设置文本显示位置
    u8g2Fonts.setCursor(121, 108 + 15);
    
    // 根据限行数据显示相应内容
    if (data[0].length() > 0)
    {
        // 有限行信息时显示：城市名+限行类型+限行尾号
        u8g2Fonts.print(data[3] + data[2] + ": " + data[0] + "  " + data[1]);
    }
    else
    {
        // 无限行信息时显示不限行提示
        u8g2Fonts.print(data[3] + "不限行,或获取失败");
    }

    // 刷新显示内容
    display.nextPage();
}

/**
 * @brief 限行模块初始化函数
 * @details 启动限行信息获取定时器，每120分钟（2小时）触发一次更新
 *          初始化后会立即获取一次限行信息（timer_driver初始值为1）
 */
void driver_init()
{
    // 设置定时器：每2小时（60*60*2秒）触发一次driver_timer回调
    driver_ticker.attach(60*60*2, driver_timer);
}

/**
 * @brief 限行定时器回调函数
 * @details 由Ticker定时器调用，设置限行更新标志位
 *          此函数在定时器中断中执行，仅设置标志位，实际更新在主循环中进行
 */
void driver_timer()
{
    timer_driver = 1; // 设置更新标志，主循环检测到此标志后会调用getDriver()
}