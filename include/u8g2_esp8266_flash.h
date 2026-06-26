#ifndef U8G2_ESP8266_FLASH_H
#define U8G2_ESP8266_FLASH_H

#if defined(ESP8266) && !defined(U8X8_FONT_SECTION)
#define U8X8_FONT_SECTION(name) __attribute__((section(".irom.text." name)))
#endif

#endif // U8G2_ESP8266_FLASH_H
