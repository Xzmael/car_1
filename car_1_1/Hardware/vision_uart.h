#ifndef HARDWARE_VISION_UART_H
#define HARDWARE_VISION_UART_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    VISION_FRAME_NONE = 0,
    VISION_FRAME_BALL,
    VISION_FRAME_NO_BALL
} VisionUart_FrameType;

typedef struct {
    VisionUart_FrameType type;
    int16_t x;
    uint16_t width;
    uint16_t height;
} VisionUart_Frame;

void VisionUart_Init(void);
void VisionUart_Poll(void);
bool VisionUart_HasFrame(void);
bool VisionUart_GetFrame(VisionUart_Frame *frame);
uint32_t VisionUart_GetFrameCount(void);
uint32_t VisionUart_GetByteCount(void);

#endif
