---
title: 初认识墨水屏-GxEPD
date: 2023-2-2 20:40:00
tags: [esp8266]
sticky: 10
---

# 关于 GxEPD2 库的使用测试
>网络上实在是没有关于 GxEPD 库的说明，好像所有人都会用（除了我），这篇文章旨在帮助第一次接触的小白踩坑 GxEPD
## 1. api测试

### 1.1 `display.nextPage()`
从源码中看，`display.nextPage()` 返回的是一个`bool`类型的数据，应该是显示下一页面，并判断屏幕是否显示完成，例程中的:

```C++
  do
  {
    display.fillScreen(GxEPD_WHITE); // 设置背景颜色
    display.setCursor(x, y);         // 设置光标位置 （显示图像的起始位置）
    display.print("hellowprld");     // 要显示的文本
  } while (display.nextPage());
```
从上述代码可以看到，在显示未完成的时候会执行一个循环，将图像显示出来。在实际测试中确实如此。

下面这样做也可以将画面正常显示出来：
```C++
    /*不带 do While 循环*/
    display.setCursor(x + 10, y + 10);
    display.print("helloWorld");
    display.nextPage();
```
### 1.2 `display.firstPage()`
源码如下：
```cpp
    void firstPage()
    {
      fillScreen(GxEPD_WHITE);
      _current_page = 0;
      _second_phase = false;
    }
```

GxEPD2 库的 `firstPage()` 函数是用来初始化显示内容的起始页面的。在使用 GxEPD2 库绘制图形或文字前，必须先调用 `firstPage()` 函数。

该函数会返回布尔值，表示是否还有更多的页面需要绘制。如果返回 `true`，说明还有更多页面需要绘制，可以继续调用其他绘图函数；如果返回 `false`，说明所有的页面都已经绘制完毕，可以调用 `display()` 函数更新显示。

```cpp
void loop() {
  // 初始化页面
  if (epd.firstPage()) {
    do {
      // 绘制图形或文字
      // ...
    } while (epd.nextPage());
  }
  // 更新显示
  epd.display();
}
```

### 1.3 `display.fillScreen()`
刷新屏幕颜色，让屏幕完全变成某个颜色。
```cpp
    /*如果是bw两色的墨水屏，颜色如下，如果需要不同灰度，请详见源码*/
#define GxEPD_BLACK     0x0000
#define GxEPD_WHITE     0xFFFF
```

### 1.4 `display.setCursor()`
设置文字，图像的显示位置，根据X,Y确认位置。

### 1.5 `isplay.setFullWindow()`
设置墨水屏全局刷新

### 1.6 `display.setPartialWindow()`
设置墨水屏局部刷新 参数为 xy 刷新坐标 wh 刷新窗口大小

1. void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation = true)：更新矩形窗口，其中 x 和 y 分别代表矩形窗口的左上角坐标，w 和 h 分别代表矩形窗口的宽度和高度。
2. void updateToWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation = true)：像素内存更新到矩形窗口，其中 x 和 y 分别代表矩形窗口的左上角坐标，w 和 h 分别代表矩形窗口的宽度和高度。

您可以在绘制图像之后调用 updateWindow() 函数，仅更新该图像区域，从而实现局部刷新。

```cpp
#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>

// 创建一个 GxEPD2_BW 或 GxEPD2_3C 对象（取决于您所使用的显示器）
GxEPD2_BW<GxEPD2_154c, GxEPD2_154c::HEIGHT> display;

void setup() {
  // 初始化显示器
  display.init();
}

void loop() {
  // 在屏幕上绘制一个黑色矩形
  display.fillRect(10, 10, 100, 100, GxEPD_BLACK);
  
  // 将更改显示在局部刷新区域内
  display.updateWindow(10, 10, 100, 100, true);
  
  // 执行其他任务
  // ...
}
```
在上面的代码中，我们创建了一个 GxEPD2_BW 或 GxEPD2_3C 对象（取决于您所使用的显示器），并使用 fillRect 方法在屏幕上绘制了一个黑色矩形。然后，我们使用 updateWindow 方法来将更改显示在局部刷新区域内。

## 2. 让墨水屏显示出中文
>这个的教程比较多，这里简单说一下

显示中文需要 **U8g2_for_Adafruit_GFX** 这个库 ，这里我们用到 甘草酸不酸大神修改好的库 下载链接 ： https://wwvd.lanzoul.com/iddKx0mgar8j

下载好以后，将 **U8g2_for_Adafruit_GFX** 库放在 `./Project/.pio/libdeps/esp12e` 中。有的教程说需要改一下头文件，实测改不改都可以正常显示。

在项目的src下创建 `main.cpp` 复制以下代码 ：
```cpp
#include <Arduino.h>
#include <GxEPD2_BW.h>               //e-paper显示库
#include <Fonts/FreeMonoBold9pt7b.h> //字体库
#include <U8g2_for_Adafruit_GFX.h>
#include "gb2312.c"
void helloWorld();

GxEPD2_BW<GxEPD2_290, GxEPD2_290::HEIGHT> display(GxEPD2_290(/*CS=15*/ SS, /*DC=4*/ 4, /*RST=2*/ 2, /*BUSY=5*/ 5)); // GDEH029A1 128x296, SSD1608 (IL3820)

U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

void setup()
{
  Serial.begin(115200);
  display.init(115200, true, 2, false);
  /*中文字体初始化*/
  display.init(115200);
  display.setRotation(1);

  u8g2Fonts.begin(display); // 将u8g2过程连接到Adafruit GFX
  u8g2Fonts.setFontDirection(0);
  u8g2Fonts.setForegroundColor(GxEPD_BLACK); // 设置前景色
  u8g2Fonts.setBackgroundColor(GxEPD_WHITE); // 设置背景色

  display.setTextColor(GxEPD_BLACK);
  u8g2Fonts.setFont(u8g2_font_wqy16_t_gb2312a); 

  helloWorld();
  delay(100);
  Serial.println("over");
}

void loop()
{
  // put your main code here, to run repeatedly:
}

void helloWorld()
{
  display.fillScreen(GxEPD_WHITE);
  do
  {
    u8g2Fonts.setCursor(5, 25);
    u8g2Fonts.println("不要温顺的走进那个良夜，");
    u8g2Fonts.println("激情不能被消沉的暮色淹没，");
    u8g2Fonts.println("abc . ! = - ");
  } while (display.nextPage());
}
```
直接复制运行，可以看到，墨水屏幕已经正常显示出内容（本次显示为 2.9 寸双色）

### 1. `u8g2Fonts.setFontDirection()`
setFontDirection 函数用于设置字体的方向。此函数控制字体如何从左到右、从右到左、从上到下或从下到上呈现。其中，direction 参数是一个 8 位无符号整数，用于指定字体的方向，比如 0、90、180、270 等。
```cpp
#include <U8g2lib.h>

U8G2_FOR_ADAFRUIT_GFX u8g2;

void setup() {
  u8g2.begin();
  u8g2.setFontDirection(90);
}

void loop() {
  // 绘制文本等
}
```


## 3. 画图
1. 导入 `Adafruit_GFX.h` 
2. 初始化屏幕:
   ```cpp
   void setup() {
    Serial.begin(115200);
    display.init();
    u8g2Fonts.begin(display);  //将u8g2连接到display
    display.firstPage();
    display.display(1);
    display.init(115200);  
    display.setRotation(1);
    }
   ```
3. 画图
   ```cpp
   display.drawLine(0, 0, 20,0,0);//（初始x,初始y，最终x,最终y,颜色）(0黑1白）
   ```
4. 刷新屏幕
   ```cpp
   display.nextPage()
   ```

- 绘制像素（点）

  首先是最基本的像素推送器。你可以调用它，并指定参数：X Y坐标和一种颜色，它会在屏幕上生成一个点:
  ```cpp
  void drawPixel(uint16_t x, uint16_t y, uint16_t color);
  ```

- 绘制线
  
  你也可以画一条线，指定起点和终点和颜色:
  ```cpp
  void drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
  ```
  对于水平或垂直的线，有优化的线绘制功能，避免角计算:
  ```cpp
  void drawFastVLine(uint16_t x0, uint16_t y0, uint16_t length, uint16_t color);
  void drawFastHLine(uint8_t x0, uint8_t y0, uint8_t length, uint16_t color);
  ```

- 绘制矩形
  
  接下来，使用以下程序可以绘制和填充矩形和正方形。每个都可以输入矩形左上角的X、Y对、宽度和高度(以像素为单位)以及颜色。drawRect()只渲染矩形的框架(轮廓)-内部不受影响-而fillRect()用给定的颜色填充整个区域:
  ```cpp
  void drawRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color);
  void fillRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color);
  ```
  要创建具有对比轮廓的实线矩形，首先使用fillRect()，然后在其上使用drawRect()。

- 绘制圆
  
  同样，对于圆圈，你可以画边框和填充。每个函数输入一个X, Y对作为中心点，半径（以像素为单位）和颜色:
  ```cpp
  void drawCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);
  void fillCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);
  ```

- 绘制圆角矩形
  
  对于圆角矩形，绘制和填充功能都是可用的。每个矩形都以X、Y、宽度和高度(就像普通矩形一样)开始，然后是角半径(以像素为单位)，最后是颜色值:
  ```cpp
  void drawRoundRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t radius, uint16_t color);
  void fillRoundRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t radius, uint16_t color);
  ```
  这里还有一个额外的小技巧:因为画圆函数总是相对于一个中心点像素绘制的，所以最终的圆直径将总是一个奇数像素。如果需要一个均匀大小的圆(这将在像素之间放置中心点)，可以使用一个圆角矩形函数来实现:传递相同的宽度和高度(均为偶数)，以及刚好为该值一半的角半径。

- 绘制三角形
  
  对于三角形，同样有绘制和填充函数。每个函数都需要七个参数:定义三角形的三个角点的X、Y坐标，然后是颜色:
  ```cpp
  void drawTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
  void fillTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
  ```


