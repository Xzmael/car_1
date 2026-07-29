#ifndef HARDWARE_STEPPER_H
#define HARDWARE_STEPPER_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    STEPPER_DIRECTION_UP = 0,
    STEPPER_DIRECTION_DOWN = 1
} Stepper_Direction;

void Stepper_Init(void);
bool Stepper_MoveA(Stepper_Direction direction, uint32_t pulses);
void Stepper_Stop(void);
bool Stepper_IsBusy(void);
int32_t Stepper_GetPosition(void);
void Stepper_SetPosition(int32_t position);
void Stepper_TimerIRQHandler(void);

#endif
