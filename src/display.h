

GxEPD2_BW<GxEPD2_290, GxEPD2_290::HEIGHT> display(GxEPD2_290(/*CS=15*/ SS, /*DC=4*/ 4, /*RST=2*/ 2, /*BUSY=5*/ 5)); // GDEH029A1 128x296, SSD1608 (IL3820)
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

/*墨水屏 汉字显示 初始化*/
void ePaper_init()
{
    Serial.begin(115200);
    display.init(115200);
    display.setRotation(1);
    u8g2Fonts.begin(display);                  // 将u8g2过程连接到Adafruit GFX
    u8g2Fonts.setFontDirection(0);             // 字体的绘制方向（从左到右）
    u8g2Fonts.setForegroundColor(GxEPD_BLACK); // 设置前景色
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE); // 设置背景色
    display.setTextColor(GxEPD_BLACK);         // 字体颜色为黑色
    u8g2Fonts.setFont(u8g2_font_wqy16_t_gb2312a);
}