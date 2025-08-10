/**
 * @file display.cpp
 * @brief 电子墨水屏显示初始化和管理模块实现
 * @details 提供电子墨水屏的基础初始化和全屏刷新功能
 * @author ESP12S E-Paper Weather Display System
 * @version 2023.2.23
 */

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <GxEPD2_BW.h>
#include <U8g2_for_Adafruit_GFX.h>
#include "Ticker.h"
#include "display.h"

// ==================== 电子墨水屏设备定义 ====================

/**
 * @brief 电子墨水屏显示对象定义
 * @details 2.9寸黑白电子墨水屏，分辨率296x128
 *          使用SSD1608控制器（IL3820兼容）
 *          引脚连接：CS=15, DC=4, RST=2, BUSY=5
 */
GxEPD2_BW<GxEPD2_290, GxEPD2_290::HEIGHT> display(GxEPD2_290(/*CS=15*/ SS, /*DC=4*/ 4, /*RST=2*/ 2, /*BUSY=5*/ 5));

/**
 * @brief U8g2中文字体显示对象定义
 * @details 用于在Adafruit GFX库上显示中文字符
 */
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

/**
 * @brief 全屏刷新定时器对象定义
 * @details 每60分钟触发一次全屏刷新，防止电子墨水屏出现残留影像
 */
Ticker ePaper_flash_ticker;

/**
 * @brief 全屏刷新标志位定义
 */
int timer_ePaper_flash = 0;

// ==================== 外部变量声明 ====================

// 这些变量在其他源文件中定义，这里进行声明以便使用
extern int timer_weather;  // 天气更新标志位（定义在getWeather.cpp）
extern int timer_1;        // 时间更新标志位（定义在getTime.cpp）
extern int timer_gushi;    // 古诗更新标志位（定义在getGushi.cpp）
extern int timer_driver;   // 限行更新标志位（定义在getDriver.cpp）

// ==================== 宏定义 ====================
#ifndef switch_WD
#define switch_WD 0  // 默认古诗模式，这个宏应该从main.cpp传入
#endif

// ==================== 函数实现 ====================

/**
 * @brief 电子墨水屏初始化函数
 * @details 初始化电子墨水屏的硬件和软件设置
 * 
 * 初始化步骤：
 * 1. 初始化串口通信（115200波特率）
 * 2. 初始化电子墨水屏硬件
 * 3. 设置屏幕旋转角度（横屏模式）
 * 4. 初始化U8g2中文字体显示
 * 5. 设置字体参数（方向、颜色、字体类型）
 * 
 * @note 使用wqy12字体支持GB2312中文显示
 */
void ePaper_init()
{
    // 初始化串口通信，用于调试输出
    Serial.begin(115200);
    
    // 初始化电子墨水屏硬件，设置通信波特率
    display.init(115200);
    
    // 设置屏幕旋转角度：1=顺时针旋转90度（横屏显示）
    display.setRotation(1);
    
    // 初始化U8g2中文字体显示系统，关联到Adafruit GFX库
    u8g2Fonts.begin(display);
    
    // 设置字体绘制方向：0=从左到右水平显示
    u8g2Fonts.setFontDirection(0);
    
    // 设置前景色为黑色（字体颜色）
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);
    
    // 设置背景色为白色
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
    
    // 设置显示屏文字颜色为黑色
    display.setTextColor(GxEPD_BLACK);
    
    // 设置中文字体：12像素高度的文泉驿正黑字体，支持GB2312编码
    u8g2Fonts.setFont(u8g2_font_wqy12_t_gb2312b);
}

/**
 * @brief 全屏刷新函数
 * @details 执行完整的全屏刷新序列，清除电子墨水屏的残留影像，并重新绘制所有内容
 * 
 * 刷新过程：
 * 1. 填充整个屏幕为黑色
 * 2. 等待500毫秒让像素稳定
 * 3. 填充整个屏幕为白色
 * 4. 重新绘制所有显示内容（天气、时间、诗词/限行信息）
 * 
 * @warning 此函数会清除屏幕上的所有内容，但会自动重新绘制
 */
void ePaper_flash()
{
    display.setFullWindow();
    // 填充整个屏幕为黑色
    // display.fillScreen(GxEPD_BLACK);
    // display.nextPage();
    
    // 等待500毫秒，让电子墨水像素充分反应
    // delay(500);
    
    // 填充整个屏幕为白色，完成刷新
    display.fillScreen(GxEPD_WHITE);
    display.nextPage();
    
    // 全屏刷新完成后，重新绘制所有内容
    Serial.println("全屏刷新完成，正在重新绘制所有内容...");
    
    // 强制重新绘制天气信息（设置标志位）
    timer_weather = 1;
    
    // 强制重新绘制时间显示（设置标志位）
    timer_1 = 2;
    
    // 根据当前模式强制重新绘制右侧内容
#if switch_WD == 0
    // 古诗模式：重新绘制古诗
    timer_gushi = 1;
#else
    // 限行模式：重新绘制限行信息
    timer_driver = 1;
#endif
    
    Serial.println("所有内容标志位已设置，将在主循环中重新绘制");
}

/**
 * @brief 全屏刷新定时器回调函数
 * @details 定时器中断中调用，设置全屏刷新标志位
 */
void ePaper_flash_timer()
{
    timer_ePaper_flash = 1;
}

/**
 * @brief 全屏刷新定时器初始化函数
 * @details 启动定时器，每60分钟执行一次全屏刷新
 * 
 * 定时器设置：
 * - 间隔时间：30分钟 = 1800秒
 * - 回调函数：ePaper_flash_timer()
 * 
 * @note 全屏刷新后需要重新绘制所有内容（天气、时间、诗词等）
 * @warning 过于频繁的全屏刷新会损害电子墨水屏寿命
 */
void ePaper_flash_init()
{
    // 启动全屏刷新定时器：30分钟 = 30 * 60 = 1800秒
    ePaper_flash_ticker.attach(1800, ePaper_flash_timer);
}