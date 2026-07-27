#ifndef HARDWARE_VISION_UART_H
#define HARDWARE_VISION_UART_H

#include <stdbool.h>
#include <stdint.h>

void VisionUart_Init(void);
void VisionUart_Poll(void);
bool VisionUart_HasFrame(void);
uint32_t VisionUart_GetFrameCount(void);
uint32_t VisionUart_GetByteCount(void);

#endif
