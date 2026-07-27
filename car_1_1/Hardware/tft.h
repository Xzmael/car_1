#ifndef HARDWARE_TFT_H
#define HARDWARE_TFT_H

#include <stdint.h>

#define TFT_WIDTH   (128U)
#define TFT_HEIGHT  (160U)

#define TFT_COLOR_WHITE   (0xFFFFU)
#define TFT_COLOR_BLACK   (0x0000U)
#define TFT_COLOR_RED     (0xF800U)
#define TFT_COLOR_GREEN   (0x07E0U)
#define TFT_COLOR_BLUE    (0x001FU)
#define TFT_COLOR_YELLOW  (0xFFE0U)
#define TFT_COLOR_CYAN    (0x07FFU)

void TFT_Init(void);
void TFT_Clear(uint16_t color);
void TFT_ClearLine(uint8_t y, uint16_t color);
void TFT_SetCursor(uint8_t x, uint8_t y);
void TFT_WriteChar(char character);
void TFT_WriteString(const char *text);
void TFT_WriteUInt(uint32_t value);
void TFT_WriteInt(int32_t value);
void TFT_WriteFloat2(float value);

#endif
