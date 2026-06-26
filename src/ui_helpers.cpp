#include <Arduino.h>
#include "display.h"
#include "ui_helpers.h"

namespace
{
const uint8_t *const adaptiveFonts[] = {
    u8g2_font_wqy12_t_gb2312b,
};

struct TextLayout
{
    const uint8_t *font = u8g2_font_wqy12_t_gb2312b;
    String lines[2];
    uint8_t lineCount = 0;
    uint8_t lineHeight = 16;
    int8_t ascent = 12;
    int8_t descent = 0;
};

int utf8NextIndex(const String &text, int index)
{
    const uint8_t c = static_cast<uint8_t>(text.charAt(index));
    if ((c & 0x80) == 0)
        return index + 1;
    if ((c & 0xE0) == 0xC0)
        return index + 2;
    if ((c & 0xF0) == 0xE0)
        return index + 3;
    if ((c & 0xF8) == 0xF0)
        return index + 4;
    return index + 1;
}

int findLineEnd(const String &text, int start, int maxWidth)
{
    int bestEnd = start;
    int lastBreak = start;

    for (int i = start; i < static_cast<int>(text.length());)
    {
        const int next = utf8NextIndex(text, i);
        if (next > static_cast<int>(text.length()))
            break;

        const String slice = text.substring(start, next);
        if (u8g2Fonts.getUTF8Width(slice.c_str()) > maxWidth)
            break;

        bestEnd = next;
        const char ch = text.charAt(i);
        if (ch == ' ' || ch == ',' || ch == ';' || ch == ':' || ch == '.')
            lastBreak = next;
        i = next;
    }

    if (bestEnd == start)
        return start;
    return lastBreak > start ? lastBreak : bestEnd;
}

String fitWithEllipsis(const String &text, int maxWidth)
{
    const char *ellipsis = "...";
    if (u8g2Fonts.getUTF8Width(text.c_str()) <= maxWidth)
        return text;

    const int ellipsisWidth = u8g2Fonts.getUTF8Width(ellipsis);
    String result;
    for (int i = 0; i < static_cast<int>(text.length());)
    {
        const int next = utf8NextIndex(text, i);
        if (next > static_cast<int>(text.length()))
            break;

        const String candidate = result + text.substring(i, next);
        if (u8g2Fonts.getUTF8Width(candidate.c_str()) + ellipsisWidth > maxWidth)
            break;

        result = candidate;
        i = next;
    }

    result.trim();
    return result + ellipsis;
}

bool buildLayoutForFont(const String &text, const uint8_t *font, int maxWidth, int maxHeight, TextLayout &layout)
{
    u8g2Fonts.setFont(font);

    layout.font = font;
    layout.ascent = u8g2Fonts.getFontAscent();
    layout.descent = u8g2Fonts.getFontDescent();
    layout.lineHeight = static_cast<uint8_t>((layout.ascent - layout.descent) + 2);
    layout.lineCount = 0;
    layout.lines[0] = "";
    layout.lines[1] = "";

    if (layout.lineHeight > maxHeight)
        return false;

    String source = text;
    source.trim();
    if (!source.length())
        source = " ";

    if (u8g2Fonts.getUTF8Width(source.c_str()) <= maxWidth)
    {
        layout.lines[0] = source;
        layout.lineCount = 1;
        return true;
    }

    if (layout.lineHeight * 2 > maxHeight)
        return false;

    int start = 0;
    for (uint8_t line = 0; line < 2 && start < static_cast<int>(source.length()); ++line)
    {
        int end = findLineEnd(source, start, maxWidth);
        if (end == start)
            end = utf8NextIndex(source, start);

        String lineText = source.substring(start, end);
        lineText.trim();

        start = end;
        while (start < static_cast<int>(source.length()) && source.charAt(start) == ' ')
            ++start;

        if (line == 1 && start < static_cast<int>(source.length()))
            lineText = fitWithEllipsis(lineText + source.substring(start), maxWidth);

        layout.lines[line] = lineText;
        layout.lineCount = line + 1;
    }

    return layout.lineCount > 0;
}
}

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

void drawAdaptiveTextPanel(int x, int y, int w, int h, const String &text)
{
    const int padding = 4;
    const int verticalPadding = 2;
    const int maxWidth = w - padding * 2;
    const int maxHeight = h - verticalPadding * 2;
    TextLayout layout;
    bool hasLayout = false;

    for (const uint8_t *font : adaptiveFonts)
    {
        if (buildLayoutForFont(text, font, maxWidth, maxHeight, layout))
        {
            hasLayout = true;
            break;
        }
    }

    if (!hasLayout)
    {
        u8g2Fonts.setFont(u8g2_font_wqy12_t_gb2312b);
        layout.font = u8g2_font_wqy12_t_gb2312b;
        layout.ascent = u8g2Fonts.getFontAscent();
        layout.descent = u8g2Fonts.getFontDescent();
        layout.lineHeight = static_cast<uint8_t>((layout.ascent - layout.descent) + 2);
        layout.lines[0] = fitWithEllipsis(text, maxWidth);
        layout.lineCount = 1;
    }

    display.setPartialWindow(x, y, w, h);
    display.firstPage();
    do
    {
        display.fillRect(x, y, w, h, GxEPD_WHITE);

        u8g2Fonts.setFont(layout.font);
        u8g2Fonts.setForegroundColor(GxEPD_BLACK);
        u8g2Fonts.setBackgroundColor(GxEPD_WHITE);

        const int totalTextHeight = layout.lineCount * layout.lineHeight;
        const int firstBaseline = y + (h - totalTextHeight) / 2 + layout.ascent;

        for (uint8_t i = 0; i < layout.lineCount; ++i)
        {
            const int lineWidth = u8g2Fonts.getUTF8Width(layout.lines[i].c_str());
            const int cursorX = x + (w - lineWidth) / 2;
            const int cursorY = firstBaseline + i * layout.lineHeight;
            u8g2Fonts.setCursor(cursorX, cursorY);
            u8g2Fonts.print(layout.lines[i]);
        }
    } while (display.nextPage());

    u8g2Fonts.setFont(u8g2_font_wqy12_t_gb2312b);
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
