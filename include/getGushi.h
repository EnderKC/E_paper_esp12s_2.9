void gushi_timer();

String gushi_api = "http://v1.jinrishici.com/all.json";

HTTPClient httpClient_gushi;
WiFiClient tcpClient_gushi;

Ticker gushi_ticker;

int timer_gushi = 1;

void update_gushi(String data[4]);

void analyze_gushi_json(String input, String (&data)[4])
{
    // String input;

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

void getGushi()
{
    httpClient_gushi.begin(tcpClient_gushi, gushi_api);
    int httpCode = httpClient_gushi.GET();

    if (httpCode == HTTP_CODE_OK)
    {
        String Payload = httpClient_gushi.getString();// 使用getString函数获取服务器响应体内容

        Serial.print("\r\nServer Respose Code: ");
        Serial.println(httpCode);
        Serial.println("Server Response Payload: ");
        Serial.println(Payload);
        /*分析数据*/
        String data[4];
        analyze_gushi_json(Payload, data);
        Serial.println(data[0]);
        Serial.println(data[1]);
        Serial.println(data[2]);
        Serial.println(data[3]);
        if (Payload.length() == 0)
        {
            Serial.println("未获取到,重新获取String");
        }
        update_gushi(data);
    }
    else
    {
        Serial.print("\r\nServer Respose Code: ");
        Serial.println(httpCode);
    }
    /* 关闭ESP8266与服务器的连接 */
    httpClient_gushi.end();
}

void gushi_init()
{
    gushi_ticker.attach(60 * 10, gushi_timer);
}

void update_gushi(String data[4])
{
    display.setPartialWindow(121, 97, 175, 31);
    display.fillRect(121, 96, 175, 32, GxEPD_WHITE);
    u8g2Fonts.setForegroundColor(GxEPD_BLACK); // 设置前景色
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE); // 设置背景色
    u8g2Fonts.setCursor(121, 108 + 15);
    u8g2Fonts.print(data[0]);
    display.nextPage();
}

void gushi_timer()
{
    timer_gushi = 1;
}
