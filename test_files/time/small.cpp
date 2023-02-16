#include <Arduino.h>
#include <GxEPD2_GFX.h>
#include <GxEPD2_BW.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "1.h"

GxEPD2_BW<GxEPD2_290, GxEPD2_290::HEIGHT> display(GxEPD2_290(/*CS=15*/ SS, /*DC=4*/ 4, /*RST=2*/ 2, /*BUSY=5*/ 5)); // GDEH029A1 128x296, SSD1608 (IL3820)

// GxEPD2_BW<GxEPD2_154, GxEPD2_154::HEIGHT> display(GxEPD2_154(/*CS=15*/ SS, /*DC=4*/ 4, /*RST=2*/ 2, /*BUSY=5*/ 5)); // GDEP015OC1 200x200, IL3829, no longer available
// 局刷函数
// int tick = 0;
// void nextPage(int n = 5) /*屏幕刷新（每10次局刷进行一次全局刷新）*/
//{
//   if (tick < n) {
//     display.setPartialWindow(0, 0, 200, 200);  //设置局部刷新窗口
//     display.nextPage();
//     tick += 1;
//   } else {
//     display.setFullWindow();
//     display.nextPage();

//     tick = 0;
//   }
// }

int randomNumber;

void setup()
{
  Serial.begin(115200);
  Serial.println("GxEPD2 Test!");

  display.init(115200);   // 设置串口波特率115200
  display.setRotation(1); // 设置屏幕旋转1和3是横向  0和2是纵向
  display.fillScreen(GxEPD_WHITE);
  display.firstPage();
  // display.drawInvertedBitmap(0,0,pics[8], 296,128,GxEPD_BLACK);
  display.drawInvertedBitmap(0, 0, gImage_3, 296, 128, GxEPD_BLACK);
  display.nextPage();
}

void loop()
{
  // randomNumber = random(0, 8);
  Serial.print("Random number: ");
  Serial.println(randomNumber);

  // //显示图片
  // display.fillScreen(GxEPD_WHITE);
  // display.drawInvertedBitmap(0,0,pics[7], 296,128,GxEPD_BLACK);
  // display.nextPage();

  delay(2000);
}
/*
display.fillScreen(GxEPD_WHITE);
display.drawInvertedBitmap(0, 0,gImage_9, 200,200,GxEPD_BLACK);
display.nextPage();
delay(10000);

display.fillScreen(GxEPD_WHITE);
display.drawInvertedBitmap(0, 0,gImage_8, 200,200,GxEPD_BLACK);
display.nextPage();
delay(10000);

display.fillScreen(GxEPD_WHITE);   //全屏幕填白色
display.drawInvertedBitmap(0, 0,gImage_1, 200,200,GxEPD_BLACK);
display.nextPage();
delay(10000);



display.fillScreen(GxEPD_WHITE);
display.drawInvertedBitmap(0, 0,gImage_3, 200,200,GxEPD_BLACK);
display.nextPage();
delay(10000);

display.fillScreen(GxEPD_WHITE);
display.drawInvertedBitmap(0, 0,gImage_4, 200,200,GxEPD_BLACK);
display.nextPage();
delay(10000);

display.fillScreen(GxEPD_WHITE);
display.drawInvertedBitmap(0, 0,gImage_5, 200,200,GxEPD_BLACK);
display.nextPage();
delay(10000);

display.fillScreen(GxEPD_WHITE);
display.drawInvertedBitmap(0, 0,gImage_6, 200,200,GxEPD_BLACK);
display.nextPage();
delay(10000);

display.fillScreen(GxEPD_WHITE);
display.drawInvertedBitmap(0, 0,gImage_2, 200,200,GxEPD_BLACK);
display.nextPage();
delay(10000);

display.fillScreen(GxEPD_WHITE);
display.drawInvertedBitmap(0, 0,gImage_7, 200,200,GxEPD_BLACK);
display.nextPage();
delay(10000);
*/
