


/* 链接WIFI */
void connectNetwork()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    display.fillScreen(GxEPD_WHITE);
    u8g2Fonts.setCursor(50, 50);
    u8g2Fonts.print("正在连接网络...");
    display.nextPage();

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
    }
    display.fillScreen(GxEPD_WHITE);
    u8g2Fonts.setCursor(50, 50);
    u8g2Fonts.print("网络链接成功！");
    display.nextPage();
    delay(1000);
    display.fillScreen(GxEPD_WHITE);
    display.nextPage();
    Serial.println("WiFi connected!");
}
