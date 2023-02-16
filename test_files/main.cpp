#include <Arduino.h>
#include <GxEPD2_BW.h>               //e-paper显示库
#include <Fonts/FreeMonoBold9pt7b.h> //字体库
#include <U8g2_for_Adafruit_GFX.h>
#include <Adafruit_GFX.h>

GxEPD2_BW<GxEPD2_290, GxEPD2_290::HEIGHT> display(GxEPD2_290(/*CS=15*/ SS, /*DC=4*/ 4, /*RST=2*/ 2, /*BUSY=5*/ 5)); // GDEH029A1 128x296, SSD1608 (IL3820)

extern const uint8_t chinese_gb2312[253023] U8G2_FONT_SECTION("chinese_city_gb2312");
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

void setup()
{
  Serial.begin(115200);
  display.init(115200, true, 2, false);
  /*中文字体初始化*/
  display.init(115200);
  display.setRotation(1);

  u8g2Fonts.begin(display); // 将u8g2过程连接到Adafruit GFX
  // u8g2Fonts.setFontDirection(0);
  u8g2Fonts.setForegroundColor(GxEPD_BLACK); // 设置前景色
  u8g2Fonts.setBackgroundColor(GxEPD_WHITE); // 设置背景色
  // display.setTextColor(GxEPD_BLACK);
  u8g2Fonts.setFont(u8g2_font_wqy16_t_gb2312a); 
  delay(100);
}

void loop()
{
  // put your main code here, to run repeatedly:
}



