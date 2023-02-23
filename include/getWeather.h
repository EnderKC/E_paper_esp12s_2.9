String weather_api = "http://api.seniverse.com/v3/weather/now.json?key="+weather_APIkey+"&location=ip&language=zh-Hans&unit=c";

HTTPClient httpClient;
WiFiClient tcpClient;

void update_weather(String data[4]);
void display_weather();
void timer_weather_con();

int timer_weather = 1;

Ticker weather_ticker;

int display_day = 0;

void analyze_weather_json(String input, String (&data)[4])
{
    StaticJsonDocument<512> doc;

    DeserializationError error = deserializeJson(doc, input);

    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }

    JsonObject results_0 = doc["results"][0];

    JsonObject results_0_location = results_0["location"];
    String location_id = results_0_location["id"];                           // "WW92M43YCQG0"
    String location_name = results_0_location["name"];                       // "邯郸"
    String location_country = results_0_location["country"];                 // "CN"
    String location_path = results_0_location["path"];                       // "邯郸,邯郸,河北,中国"
    String location_timezone = results_0_location["timezone"];               // "Asia/Shanghai"
    String location_timezone_offset = results_0_location["timezone_offset"]; // "+08:00"

    JsonObject results_0_now = results_0["now"];
    String now_text = results_0_now["text"];               // "阴"
    String now_code = results_0_now["code"];               // "9"
    String now_temperature = results_0_now["temperature"]; // "6"

    String last_update = results_0["last_update"]; // "2023-02-08T16:00:09+08:00"

    data[0] = location_name;
    data[1] = now_text;
    data[2] = now_code;
    data[3] = now_temperature;
}

void getWeather()
{
    httpClient.begin(tcpClient, weather_api);
    int httpCode = httpClient.GET();

    if (httpCode == HTTP_CODE_OK)
    {
        String Payload = httpClient.getString(); // 使用getString函数获取服务器响应体内容
        Serial.print("\r\nServer Respose Code: ");
        Serial.println(httpCode);
        Serial.println("Server Response Payload: ");
        Serial.println(Payload);
        /*分析数据*/
        String data[4];
        analyze_weather_json(Payload, data);
        Serial.println(data[0]);
        Serial.println(data[1]);
        Serial.println(data[2]);
        Serial.println(data[3]);
        update_weather(data);
    }
    else
    {
        Serial.print("\r\nServer Respose Code: ");
        Serial.println(httpCode);
    }
    /* 关闭ESP8266与服务器的连接 */
    httpClient.end();
}

void weather_init()
{
    display.setPartialWindow(0, 0, 120, 128);
    display.fillRect(0, 0, 120, 128, GxEPD_BLACK);
    display.nextPage();
    // display_weather();
    weather_ticker.attach(60*20, timer_weather_con);
}

void display_weather()
{
    display.firstPage();
    display.setPartialWindow(0, 0, 120, 128);
    display.fillRect(0, 0, 120, 128, GxEPD_BLACK);
    display.setRotation(1);
    display.drawLine(0, 60, 119, 60, 1);
    display.drawFastHLine(0, 95, 119, 1);
    // display.nextPage();
}

void update_weather(String data[4])
{
    display_weather();
    // display.setPartialWindow(0, 0, 120, 128);
    display.setRotation(1);
    display.fillRect(30, 0, 60, 60, GxEPD_WHITE);
    // display.fillRect(0, 75, 20, 20, GxEPD_WHITE);
    // display.fillRect(0, 108, 20, 100, GxEPD_WHITE);
    if (data[2] == "0" || data[2] == "2" || data[2] == "38")
    {
        display.drawInvertedBitmap(30, 0, qing_d, 60, 60, GxEPD_BLACK);
    }
    else if (data[2] == "1" || data[2] == "3")
    {
        display.drawInvertedBitmap(30, 0, qing_n, 60, 60, GxEPD_BLACK);
    }
    else if (data[2] == "4" || data[2] == "5" || data[2] == "6" || data[2] == "7" || data[2] == "8")
    {
        display.drawInvertedBitmap(30, 0, duoyun, 60, 60, GxEPD_BLACK);
    }
    else if (data[2] == "9")
    {
        display.drawInvertedBitmap(30, 0, yintian, 60, 60, GxEPD_BLACK);
    }
    else if (data[2] == "10" || data[2] == "13" || data[2] == "14" || data[2] == "15" || data[2] == "16" || data[2] == "17" || data[2] == "18")
    {
        display.drawInvertedBitmap(30, 0, zhenyu, 60, 60, GxEPD_BLACK);
    }
    else if (data[2] == "11" || data[2] == "12")
    {
        display.drawInvertedBitmap(30, 0, leizhenyu, 60, 60, GxEPD_BLACK);
    }
    else if (data[2] == "20" || data[2] == "21" || data[2] == "22" || data[2] == "23" || data[2] == "24" || data[2] == "25" || data[2] == "37")
    {
        display.drawInvertedBitmap(30, 0, xiaoxue, 60, 60, GxEPD_BLACK);
    }
    else if (data[2] == "26" || data[2] == "27")
    {
        display.drawInvertedBitmap(30, 0, fuchen, 60, 60, GxEPD_BLACK);
    }
    else if (data[2] == "28" || data[2] == "29" || data[2] == "32" || data[2] == "33" || data[2] == "36")
    {
        display.drawInvertedBitmap(30, 0, feng, 60, 60, GxEPD_BLACK);
    }
    else if (data[2] == "31" || data[2] == "34" || data[2] == "35")
    {
        display.drawInvertedBitmap(30, 0, wu, 60, 60, GxEPD_BLACK);
    }
    else if (data[2] == "99")
    {
        display.drawInvertedBitmap(30, 0, weizhi, 60, 60, GxEPD_BLACK);
    }

    u8g2Fonts.setForegroundColor(GxEPD_WHITE); // 设置前景色
    u8g2Fonts.setBackgroundColor(GxEPD_BLACK); // 设置背景色
    u8g2Fonts.setCursor(10, 90);
    u8g2Fonts.print(data[0]+" "+data[1]+" "+data[3]+"℃");
    // u8g2Fonts.setCursor(20, 108 + 15);

    /*更新日期*/
    if (timeinfo.tm_mday || now_day)
    {
        display_day = timeinfo.tm_mon;
        u8g2Fonts.setForegroundColor(GxEPD_WHITE); // 设置前景色
        u8g2Fonts.setBackgroundColor(GxEPD_BLACK); // 设置背景色
        u8g2Fonts.setCursor(10, 108 + 15);
        String week_day;
        switch (timeinfo.tm_wday)
        {
        case 0:
            week_day = "周日";
            break;
        case 1:
            week_day = "周一";
            break;
        case 2:
            week_day = "周二";
            break;
        case 3:
            week_day = "周三";
            break;
        case 4:
            week_day = "周四";
            break;
        case 5:
            week_day = "周五";
            break;
        case 6:
            week_day = "周六";
            break;
        default:
            week_day = "未知";
            break;
        }
        u8g2Fonts.print(String(timeinfo.tm_mon+1)+"月 "+String(timeinfo.tm_mday)+"日 "+week_day);
    }

    display.nextPage();
}

void timer_weather_con()
{
    Serial.println("update weather !");
    timer_weather = 1;
}