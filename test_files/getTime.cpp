#include <Arduino.h>
#include <GxEPD2_BW.h>               //e-paper显示库
#include <Fonts/FreeMonoBold9pt7b.h> //字体库
#include <U8g2_for_Adafruit_GFX.h>
#include <Adafruit_GFX.h>
#include <ESP8266WiFi.h>

#define NTP1  "ntp1.aliyun.com"
#define NTP2  "ntp2.aliyun.com"
#define NTP3  "ntp3.aliyun.com"

const char* ssid     = "请用C++输出wifi";     // WIFI账户
const char* password = "cout<<\"wifi\";"; // WIFI密码


GxEPD2_BW<GxEPD2_290, GxEPD2_290::HEIGHT> display(GxEPD2_290(/*CS=15*/ SS, /*DC=4*/ 4, /*RST=2*/ 2, /*BUSY=5*/ 5)); // GDEH029A1 128x296, SSD1608 (IL3820)

extern const uint8_t chinese_gb2312[253023] U8G2_FONT_SECTION("chinese_city_gb2312");
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

void connectNetwork();
void connectNetwork_page1();
void connectNetwork_page2();
void helloFullScreenPartialMode();

void setup()
{
  Serial.begin(115200);
  // display.init(115200, true, 2, false);
  /*中文字体初始化*/
  display.init(115200);
  display.setRotation(1);

  u8g2Fonts.begin(display); // 将u8g2过程连接到Adafruit GFX
  u8g2Fonts.setFontDirection(0);
  u8g2Fonts.setForegroundColor(GxEPD_BLACK); // 设置前景色
  u8g2Fonts.setBackgroundColor(GxEPD_WHITE); // 设置背景色
  display.setTextColor(GxEPD_BLACK);
  u8g2Fonts.setFont(u8g2_font_wqy16_t_gb2312a);

  connectNetwork();
  helloFullScreenPartialMode();
}

void loop()
{
  // put your main code here, to run repeatedly:
}

/* 链接WIFI */
void connectNetwork(){
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    connectNetwork_page1();
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
    }
    connectNetwork_page2();
    Serial.println("WiFi connected!");
}

/*显示 正在联网*/
void connectNetwork_page1(){
  display.setFullWindow();
  display.fillScreen(GxEPD_WHITE);
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.setFont(&FreeMonoBold9pt7b);
  display.getTextBounds("connecting....", 0, 0, &tbx, &tby, &tbw, &tbh);
  // center bounding box by transposition of origin:
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;
  u8g2Fonts.setCursor(x,y);
  u8g2Fonts.print("正在连接网络...");
  display.nextPage();
}

/*显示 联网成功*/
void connectNetwork_page2(){
  display.fillScreen(GxEPD_WHITE);
  display.setFont(&FreeMonoBold9pt7b);
  int width = u8g2Fonts.getUTF8Width("已连接至网络！");
  // center bounding box by transposition of origin:
  uint16_t x = ((display.width() - width) / 2);
  uint16_t y = (display.height() / 2);
  u8g2Fonts.setCursor(x,y);
  // display.setPartialWindow(x,y,width,20);
  u8g2Fonts.print("已连接至网络！");
  Serial.println(width);
  display.nextPage();
}

void helloFullScreenPartialMode()
{
  //Serial.println("helloFullScreenPartialMode");
  const char fullscreen[] = "full screen update";
  const char fpm[] = "fast partial mode";
  const char spm[] = "slow partial mode";
  const char npm[] = "no partial mode";
  const char HelloWorld[] = "Hello World!";
  display.setPartialWindow(0, 0, display.width(), display.height());
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  if (display.epd2.WIDTH < 104) display.setFont(0);
  display.setTextColor(GxEPD_BLACK);
  const char* updatemode;
  if (display.epd2.hasFastPartialUpdate)
  {
    updatemode = fpm;
  }
  else if (display.epd2.hasPartialUpdate)
  {
    updatemode = spm;
  }
  else
  {
    updatemode = npm;
  }
  // do this outside of the loop
  int16_t tbx, tby; uint16_t tbw, tbh;
  // center update text
  display.getTextBounds(fullscreen, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t utx = ((display.width() - tbw) / 2) - tbx;
  uint16_t uty = ((display.height() / 4) - tbh / 2) - tby;
  // center update mode
  display.getTextBounds(updatemode, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t umx = ((display.width() - tbw) / 2) - tbx;
  uint16_t umy = ((display.height() * 3 / 4) - tbh / 2) - tby;
  // center HelloWorld
  display.getTextBounds(HelloWorld, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t hwx = ((display.width() - tbw) / 2) - tbx;
  uint16_t hwy = ((display.height() - tbh) / 2) - tby;
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(hwx, hwy);
    display.print(HelloWorld);
    display.setCursor(utx, uty);
    display.print(fullscreen);
    display.setCursor(umx, umy);
    display.print(updatemode);
  }
  while (display.nextPage());
  //Serial.println("helloFullScreenPartialMode done");
}
