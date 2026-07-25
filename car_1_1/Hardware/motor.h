#ifndef HARDWARE_MOTOR_H
#define HARDWARE_MOTOR_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    float targetYaw;
    float yawError;
    uint8_t leftDuty;
    uint8_t rightDuty;
    bool running;
} Motor_Status;

void Motor_Init(void);
void Motor_SetForward(uint8_t duty);
void Motor_SetForwardDuty(uint8_t leftDuty, uint8_t rightDuty);
void Motor_Stop(void);
void Motor_HoldYawStart(uint8_t duty);
void Motor_HoldYawUpdate(float yaw);
Motor_Status Motor_GetStatus(void);

#endif
