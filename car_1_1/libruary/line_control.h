#ifndef LIBRUARY_LINE_CONTROL_H
#define LIBRUARY_LINE_CONTROL_H

#include <stdint.h>

#include "gray.h"

typedef enum {
    LINE_CONTROL_TRACKING = 0,
    LINE_CONTROL_LOST,
    LINE_CONTROL_ALL_BLACK,
    LINE_CONTROL_TURN_FORWARD,
    LINE_CONTROL_HARD_LEFT,
    LINE_CONTROL_HARD_RIGHT
} LineControl_Mode;

typedef struct {
    uint8_t leftDuty;
    uint8_t rightDuty;
    int16_t position;
    LineControl_Mode mode;
} LineControl_Output;

void LineControl_Init(void);
LineControl_Output LineControl_Update(Gray_Result gray);
void LineControl_Start(void);
void LineControl_Run(void);
void LineControl_Stop(void);

#endif
