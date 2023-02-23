String driver_api_tianjin = "http://api.seniverse.com/v3/life/driving_restriction.json?key=" + driver_api_key + "&location=WWGQDCW6TBW1";
String driver_api_beijing = "http://api.seniverse.com/v3/life/driving_restriction.json?key=" + driver_api_key + "&location=WX4FBXXFKE4F";
int timer_driver = 1;

Ticker driver_ticker;

HTTPClient httpClient_driver;
WiFiClient tcpClient_driver;

void update_driver(String data[4]);
void getDriver();
void driver_timer();

void analyze_driver_json(String input, String (&data)[4])
{
    // String input;

    StaticJsonDocument<1536> doc;

    DeserializationError error = deserializeJson(doc, input);

    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }

    JsonObject location = doc["results"][0]["location"];
    String location_id = location["id"];                           // "WWGQDCW6TBW1"
    String location_name = location["name"];                       // "天津"
    String location_country = location["country"];                 // "CN"
    String location_path = location["path"];                       // "天津,天津,中国"
    String location_timezone = location["timezone"];               // "Asia/Shanghai"
    String location_timezone_offset = location["timezone_offset"]; // "+08:00"

    JsonObject restriction = doc["results"][0]["restriction"];
    String restriction_penalty = restriction["penalty"];
    String restriction_region = restriction["region"]; // "外环线(不含)以内道路"
    String restriction_time = restriction["time"];     // "每日7时至19时"
    String restriction_remarks = restriction["remarks"];

    JsonArray restriction_limits = restriction["limits"];

    JsonObject restriction_limits_0 = restriction_limits[0];
    String restriction_limits_0_date = restriction_limits_0["date"];

    String restriction_limits_0_plates_0 = restriction_limits_0["plates"][0];
    String restriction_limits_0_plates_1 = restriction_limits_0["plates"][1];

    String restriction_limits_0_memo = restriction_limits_0["memo"]; // "尾号限行"

    JsonObject restriction_limits_1 = restriction_limits[1];
    String restriction_limits_1_date = restriction_limits_1["date"];

    String restriction_limits_1_plates_0 = restriction_limits_1["plates"][0];
    String restriction_limits_1_plates_1 = restriction_limits_1["plates"][1];

    String restriction_limits_1_memo = restriction_limits_1["memo"]; // "尾号限行"

    JsonObject restriction_limits_2 = restriction_limits[2];
    String restriction_limits_2_date = restriction_limits_2["date"];

    String restriction_limits_2_memo = restriction_limits_2["memo"]; // "周末不限行"

    data[0] = restriction_limits_0_plates_0;
    data[1] = restriction_limits_0_plates_1;
    data[2] = restriction_limits_0_memo;
    data[3] = location_name;

    update_driver(data);
}

void getDriver()
{
    httpClient_driver.begin(tcpClient_driver, driver_api_tianjin);
    int httpCode = httpClient_driver.GET();

    if (httpCode == HTTP_CODE_OK)
    {
        String Payload = httpClient_driver.getString(); // 使用getString函数获取服务器响应体内容
        Serial.print("\r\nServer Respose Code: ");
        Serial.println(httpCode);
        Serial.println("Server Response Payload: ");
        Serial.println(Payload);
        /*分析数据*/
        String data[4];
        analyze_driver_json(Payload, data);
        Serial.println(data[0]);
        Serial.println(data[1]);
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
        Serial.print("\r\nServer Respose Code: ");
        Serial.println(httpCode);
    }
    /* 关闭ESP8266与服务器的连接 */
    httpClient.end();
}

void update_driver(String data[4])
{
    display.setPartialWindow(121, 97, 175, 31);
    display.fillRect(121, 96, 175, 32, GxEPD_WHITE);
    u8g2Fonts.setForegroundColor(GxEPD_BLACK); // 设置前景色
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE); // 设置背景色
    u8g2Fonts.setCursor(121, 108 + 15);
    if (data[0].length() > 0)
    {
        u8g2Fonts.print(data[3]+data[2] +": "+ data[0] + "  " + data[1]);
    }
    else
    {
        u8g2Fonts.print(data[3] + "不限行,或获取失败");
    }

    display.nextPage();
}

void driver_init()
{
    driver_ticker.attach(60*60*2, driver_timer);
}

void driver_timer()
{
    timer_driver = 1;
}