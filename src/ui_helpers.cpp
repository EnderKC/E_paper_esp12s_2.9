#include <Arduino.h>
#include "display.h"
#include "ui_helpers.h"

void drawStatusPanel(int x, int y, int w, int h, const String &text)
{
    display.setPartialWindow(x, y, w, h);
    display.firstPage();
    do
    {
        display.fillRect(x, y, w, h, GxEPD_WHITE);

        u8g2Fonts.setForegroundColor(GxEPD_BLACK);
        u8g2Fonts.setBackgroundColor(GxEPD_WHITE);

        const int padding = 4;
        const int maxWidth = w - padding * 2;
        const int lineHeight = 18;
        const int maxLines = h / lineHeight;

        int lineStart = 0;
        int yBase = y + 12;

        for (int line = 0; line < maxLines && lineStart < (int)text.length(); ++line)
        {
            int bestEnd = lineStart;
            int lastBreak = lineStart;

            for (int i = lineStart; i < (int)text.length();)
            {
                int next = i + 1;
                uint8_t c = (uint8_t)text.charAt(i);
                if ((c & 0x80) == 0)
                    next = i + 1;
                else if ((c & 0xE0) == 0xC0)
                    next = i + 2;
                else if ((c & 0xF0) == 0xE0)
                    next = i + 3;
                else if ((c & 0xF8) == 0xF0)
                    next = i + 4;

                if (next > (int)text.length())
                    break;

                String slice = text.substring(lineStart, next);
                if (u8g2Fonts.getUTF8Width(slice.c_str()) > maxWidth)
                    break;

                bestEnd = next;
                if (text.charAt(i) == ' ' || text.charAt(i) == ',')
                    lastBreak = next;
                i = next;
            }

            if (bestEnd == lineStart)
                break;

            int cut = lastBreak > lineStart ? lastBreak : bestEnd;
            String lineText = text.substring(lineStart, cut);
            lineText.trim();
            u8g2Fonts.setCursor(x + padding, yBase + line * lineHeight);
            u8g2Fonts.print(lineText);
            lineStart = cut;
            while (lineStart < (int)text.length() && text.charAt(lineStart) == ' ')
                ++lineStart;
        }
    } while (display.nextPage());
}

String formatWeekday(int weekday)
{
    switch (weekday)
    {
    case 0: return "周日";
    case 1: return "周一";
    case 2: return "周二";
    case 3: return "周三";
    case 4: return "周四";
    case 5: return "周五";
    case 6: return "周六";
    default: return "未知";
    }
}
