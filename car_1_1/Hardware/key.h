#ifndef HARDWARE_KEY_H
#define HARDWARE_KEY_H

#include <stdint.h>

#define KEY_SW1 (1U << 0)
#define KEY_SW2 (1U << 1)
#define KEY_SW4 (1U << 2)

void Key_Init(void);
void Key_Scan(void);
uint8_t Key_GetState(void);
uint8_t Key_GetPressed(void);

#endif
