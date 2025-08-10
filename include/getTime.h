
/**
 * @file getTime.h
 * @brief 时间同步和数字时钟显示模块
 * @details 通过NTP服务器获取网络时间，在电子墨水屏上显示数字时钟
 *          采用七段显示器风格的自绘数字，提供清晰的时间显示效果
 * @author ESP12S E-Paper Weather Display System
 * @version 2023.2.23
 */

#ifndef GET_TIME_H
#define GET_TIME_H

#include <Arduino.h>
#include "Ticker.h"

// ==================== NTP服务器配置 ====================

/**
 * @brief 阿里云NTP服务器地址配置
 * @details 使用多个NTP服务器确保时间同步的可靠性
 *          - NTP1: 主服务器
 *          - NTP2: 备用服务器1
 *          - NTP3: 备用服务器2
 */
#define NTP1 "ntp1.aliyun.com"
#define NTP2 "ntp2.aliyun.com" 
#define NTP3 "ntp3.aliyun.com"

// ==================== 函数声明 ====================

/**
 * @brief 绘制单个数字的七段显示器样式
 * @param x 数字显示的X坐标起始位置
 * @param y 数字显示的Y坐标起始位置  
 * @param num 要显示的数字(0-9)
 */
void num_time(int x, int y, int num);

/**
 * @brief 比较网络时间与当前显示时间
 * @details 只有在时间发生变化时才更新显示，降低功耗
 */
void compare_time();

/**
 * @brief 更新时间显示界面
 * @details 在指定区域绘制HH:MM格式的数字时钟
 */
void time_update();

/**
 * @brief 系统问候函数
 * @details 预留的问候功能接口
 */
void sayHi();

/**
 * @brief 时间定时器回调函数
 * @details 每10秒触发一次时间检查
 */
void time_timer();

// ==================== NTP服务器配置 ====================

/**
 * @brief 阿里云NTP服务器地址配置
 * @details 使用多个NTP服务器确保时间同步的可靠性
 *          - NTP1: 主服务器
 *          - NTP2: 备用服务器1
 *          - NTP3: 备用服务器2
 */
#define NTP1 "ntp1.aliyun.com"
#define NTP2 "ntp2.aliyun.com" 
#define NTP3 "ntp3.aliyun.com"

// ==================== 时间数据结构和变量声明 ====================

/**
 * @brief 系统时间信息结构体声明
 * @details 存储从NTP服务器获取的时间信息
 */
extern struct tm timeinfo;

/**
 * @brief 时间更新定时器对象声明
 * @details 每10秒触发一次时间检查
 */
extern Ticker time_ticker;

/**
 * @brief 时间更新标志位声明
 * @details 值为1时表示需要检查时间更新
 *          初始值为1确保首次运行时立即显示时间
 */
extern int timer_1;

/**
 * @brief 当前显示的时间缓存变量声明
 * @details 用于与网络时间比较，避免不必要的屏幕刷新
 * - now_hour: 当前显示的小时
 * - now_min: 当前显示的分钟  
 * - now_week: 当前显示的星期
 * - now_mon: 当前显示的月份
 * - now_day: 当前显示的日期
 */
extern int now_hour;
extern int now_min;
extern int now_week;
extern int now_mon;
extern int now_day;

/**
 * @brief 时间初始化函数声明
 * @details 初始化NTP时间同步和时间显示区域
 */
void time_init();

/**
 * @brief 强制更新时间显示函数声明
 * @details 强制刷新时间显示，不检查时间变化
 */
void force_update_time();

#endif // GET_TIME_H