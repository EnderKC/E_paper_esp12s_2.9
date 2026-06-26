#ifndef UI_HELPERS_H
#define UI_HELPERS_H

#include <Arduino.h>

void drawStatusPanel(int x, int y, int w, int h, const String &text);
String formatWeekday(int weekday);
void renderStatusLine(const String &text);

#endif // UI_HELPERS_H
