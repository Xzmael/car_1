#ifndef HARDWARE_VISION_UART_H
#define HARDWARE_VISION_UART_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    VISION_UART_NO_FRAME = 0,
    VISION_UART_BALL,
    VISION_UART_NONE
} VisionUart_Type;

typedef struct {
    VisionUart_Type type;
    int16_t centerX;
} VisionUart_Frame;

void VisionUart_Init(void);
void VisionUart_Poll(void);
void VisionUart_Tick1ms(void);
bool VisionUart_GetFrame(VisionUart_Frame *frame);
uint32_t VisionUart_GetFrameAgeMs(void);
uint32_t VisionUart_GetFrameCount(void);

#endif
