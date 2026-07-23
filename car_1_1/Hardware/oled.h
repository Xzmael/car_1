#ifndef HARDWARE_OLED_H
#define HARDWARE_OLED_H

#include <stdbool.h>
#include <stdint.h>

#define OLED_WIDTH  (128U)
#define OLED_HEIGHT (64U)

typedef enum {
    OLED_STATUS_OK = 0,
    OLED_STATUS_NACK,
    OLED_STATUS_TIMEOUT
} OLED_Status;

OLED_Status OLED_Init(void);
OLED_Status OLED_Refresh(void);
void OLED_Clear(void);
void OLED_SetCursor(uint8_t x, uint8_t y);
void OLED_WriteChar(char character);
void OLED_WriteString(const char *text);
void OLED_WriteUInt(uint32_t value);
void OLED_DrawPixel(uint8_t x, uint8_t y, bool on);
void OLED_DrawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, bool on);
void OLED_DrawRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool on);

#endif
