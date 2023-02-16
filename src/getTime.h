
void num_time(int x, int y, int num);
void compare_time();
void time_update();
void sayHi();
void time_timer();

#define NTP1 "ntp1.aliyun.com"
#define NTP2 "ntp2.aliyun.com"
#define NTP3 "ntp3.aliyun.com"

struct tm timeinfo;

Ticker time_ticker;
int timer_1 = 1;

int now_hour = 0;
int now_min = 0;
int now_week = 0;
int now_mon = 0;
int now_day = 0;

void time_init()
{
    // timeClient.begin();
    // timeClient.update();
    configTime(8 * 3600, 0, NTP1, NTP2, NTP3);
    getLocalTime(&timeinfo);
    /*区域初始化*/
    display.fillScreen(GxEPD_WHITE);
    display.setPartialWindow(119, 0, 177, 90);
    display.fillRect(119, 0, 177, 90, GxEPD_BLACK);
    // display.updateWindow(119, 0, 177, 90, true);
    display.nextPage();
    delay(500); // 程序暂停2秒
    display.fillRect(119, 0, 177, 90, GxEPD_WHITE);
    display.nextPage();
    delay(500); // 程序暂停2秒
    display.drawRect(119, 0, 177, 90, GxEPD_BLACK);
    display.nextPage();
    time_ticker.attach(10, time_timer); // 每隔10s更新一次时间
}

// 告诉程序，时间该更新了
void time_timer()
{
    Serial.println("update time !");
    timer_1 = 1;
}

/*对比网络时间与当前显示时间是否一致，降低功耗*/
void compare_time()
{
    /*更新时间*/
    getLocalTime(&timeinfo);
    if (timeinfo.tm_hour != now_hour || timeinfo.tm_min != now_min)
    {
        time_update();
        now_hour = timeinfo.tm_hour;
        now_min = timeinfo.tm_min;
    }
    
}

void time_update()
{
    display.setPartialWindow(119, 0, 177, 96);
    display.firstPage();
    display.drawRect(119, 0, 177, 96, GxEPD_BLACK);
    int hours = timeinfo.tm_hour;
    int min = timeinfo.tm_min;
    do
    {
        /*前两位数字*/
        int hours_f, hours_b;
        if (hours < 10)
        {
            hours_f = 0;
            hours_b = hours;
        }
        else
        {
            hours_f = hours / 10;
            hours_b = hours % 10;
        }
        num_time(124, 17, hours_f);
        num_time(159, 17, hours_b);
        /*中间点点*/
        display.fillCircle(207, 37, 3, GxEPD_BLACK);
        display.fillCircle(207, 56, 3, GxEPD_BLACK);
        /*后两位数字*/
        int min_f, min_b;
        if (min < 10)
        {
            min_f = 0;
            min_b = min;
        }
        else
        {
            min_f = min / 10;
            min_b = min % 10;
        }
        num_time(225, 17, min_f);
        num_time(260, 17, min_b);
    } while (display.nextPage());

    Serial.println(hours);
    Serial.println(min);
}

// 单位数字的输出
void num_time(int x, int y, int num)
{
    /*完整测试*/
    /*
    display.fillRoundRect(x + 5, y, 20, 5, 2, GxEPD_BLACK);
    display.fillRoundRect(x + 25, y + 5, 5, 20, 2, GxEPD_BLACK);
    display.fillRoundRect(x + 25, y + 30, 5, 20, 2, GxEPD_BLACK);
    display.fillRoundRect(x + 5, y + 50, 20, 5, 2, GxEPD_BLACK);
    display.fillRoundRect(x, y + 30, 5, 20, 2, GxEPD_BLACK);
    display.fillRoundRect(x, y + 5, 5, 20, 2, GxEPD_BLACK);
    display.fillRoundRect(x + 5, y + 25, 20, 5, 2, GxEPD_BLACK);
    */
    switch (num)
    {
    case 0:
        display.fillRoundRect(x + 5, y, 20, 5, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 25, y + 5, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 25, y + 30, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 5, y + 50, 20, 5, 2, GxEPD_BLACK);
        display.fillRoundRect(x, y + 30, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x, y + 5, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 5, y + 25, 20, 5, 2, GxEPD_WHITE);
        break;
    case 1:
        display.fillRoundRect(x + 5, y, 20, 5, 2, GxEPD_WHITE);
        display.fillRoundRect(x + 25, y + 5, 5, 20, 2, GxEPD_WHITE);
        display.fillRoundRect(x + 25, y + 30, 5, 20, 2, GxEPD_WHITE);
        display.fillRoundRect(x + 5, y + 50, 20, 5, 2, GxEPD_WHITE);
        display.fillRoundRect(x, y + 30, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x, y + 5, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 5, y + 25, 20, 5, 2, GxEPD_WHITE);
        break;
    case 2:
        display.fillRoundRect(x + 5, y, 20, 5, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 25, y + 5, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 25, y + 30, 5, 20, 2, GxEPD_WHITE);
        display.fillRoundRect(x + 5, y + 50, 20, 5, 2, GxEPD_BLACK);
        display.fillRoundRect(x, y + 30, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x, y + 5, 5, 20, 2, GxEPD_WHITE);
        display.fillRoundRect(x + 5, y + 25, 20, 5, 2, GxEPD_BLACK);
        break;
    case 3:
        display.fillRoundRect(x + 5, y, 20, 5, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 25, y + 5, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 25, y + 30, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 5, y + 50, 20, 5, 2, GxEPD_BLACK);
        display.fillRoundRect(x, y + 30, 5, 20, 2, GxEPD_WHITE);
        display.fillRoundRect(x, y + 5, 5, 20, 2, GxEPD_WHITE);
        display.fillRoundRect(x + 5, y + 25, 20, 5, 2, GxEPD_BLACK);
        break;
    case 4:
        display.fillRoundRect(x + 5, y, 20, 5, 2, GxEPD_WHITE);
        display.fillRoundRect(x + 25, y + 5, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 25, y + 30, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 5, y + 50, 20, 5, 2, GxEPD_WHITE);
        display.fillRoundRect(x, y + 30, 5, 20, 2, GxEPD_WHITE);
        display.fillRoundRect(x, y + 5, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 5, y + 25, 20, 5, 2, GxEPD_BLACK);
        break;
    case 5:
        display.fillRoundRect(x + 5, y, 20, 5, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 25, y + 5, 5, 20, 2, GxEPD_WHITE);
        display.fillRoundRect(x + 25, y + 30, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 5, y + 50, 20, 5, 2, GxEPD_BLACK);
        display.fillRoundRect(x, y + 30, 5, 20, 2, GxEPD_WHITE);
        display.fillRoundRect(x, y + 5, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 5, y + 25, 20, 5, 2, GxEPD_BLACK);
        break;
    case 6:
        display.fillRoundRect(x + 5, y, 20, 5, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 25, y + 5, 5, 20, 2, GxEPD_WHITE);
        display.fillRoundRect(x + 25, y + 30, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 5, y + 50, 20, 5, 2, GxEPD_BLACK);
        display.fillRoundRect(x, y + 30, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x, y + 5, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 5, y + 25, 20, 5, 2, GxEPD_BLACK);
        break;
    case 7:
        display.fillRoundRect(x + 5, y, 20, 5, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 25, y + 5, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 25, y + 30, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 5, y + 50, 20, 5, 2, GxEPD_WHITE);
        display.fillRoundRect(x, y + 30, 5, 20, 2, GxEPD_WHITE);
        display.fillRoundRect(x, y + 5, 5, 20, 2, GxEPD_WHITE);
        display.fillRoundRect(x + 5, y + 25, 20, 5, 2, GxEPD_WHITE);
        break;
    case 8:
        display.fillRoundRect(x + 5, y, 20, 5, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 25, y + 5, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 25, y + 30, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 5, y + 50, 20, 5, 2, GxEPD_BLACK);
        display.fillRoundRect(x, y + 30, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x, y + 5, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 5, y + 25, 20, 5, 2, GxEPD_BLACK);
        break;
    case 9:
        display.fillRoundRect(x + 5, y, 20, 5, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 25, y + 5, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 25, y + 30, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 5, y + 50, 20, 5, 2, GxEPD_BLACK);
        display.fillRoundRect(x, y + 30, 5, 20, 2, GxEPD_WHITE);
        display.fillRoundRect(x, y + 5, 5, 20, 2, GxEPD_BLACK);
        display.fillRoundRect(x + 5, y + 25, 20, 5, 2, GxEPD_BLACK);
        break;
    default:
        break;
    }
}