#ifndef U8G2_ESP8266_FLASH_H
#define U8G2_ESP8266_FLASH_H

#if defined(ESP8266) && !defined(U8X8_FONT_SECTION)
#define U8X8_FONT_SECTION(name) __attribute__((section(".irom.text." name)))
#endif

#if defined(ESP8266) && !defined(u8x8_pgm_read)
#include <pgmspace.h>
#define u8x8_pgm_read(adr) pgm_read_byte(adr)
#endif

#endif // U8G2_ESP8266_FLASH_H
