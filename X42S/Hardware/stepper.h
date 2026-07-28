#ifndef HARDWARE_STEPPER_H
#define HARDWARE_STEPPER_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    STEPPER_DIRECTION_UP = 0,
    STEPPER_DIRECTION_DOWN = 1
} Stepper_Direction;

void Stepper_Init(void);
bool Stepper_MoveBoth(Stepper_Direction direction, uint32_t pulses);
void Stepper_Stop(void);
bool Stepper_IsBusy(void);
void Stepper_TimerIRQHandler(void);

#endif
