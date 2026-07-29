#ifndef HARDWARE_IMU660RB_H
#define HARDWARE_IMU660RB_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    IMU660RB_STATUS_OK = 0,
    IMU660RB_STATUS_I2C_ERROR,
    IMU660RB_STATUS_TIMEOUT,
    IMU660RB_STATUS_DEVICE_ID_ERROR
} IMU660RB_Status;

typedef enum {
    IMU660RB_ROD_AXIS_ROLL = 0,
    IMU660RB_ROD_AXIS_PITCH
} IMU660RB_RodAxis;

typedef struct {
    float accelerationG[3];
    float angularRateDps[3];
    float rollDeg;
    float pitchDeg;
    float rodAngleDeg;
    float rodRateDps;
} IMU660RB_Data;

IMU660RB_Status IMU660RB_Init(void);
IMU660RB_Status IMU660RB_Update(void);
void IMU660RB_Tick1ms(void);
void IMU660RB_SetRodAxis(IMU660RB_RodAxis axis, bool inverted);
void IMU660RB_SetRodZero(void);
const IMU660RB_Data *IMU660RB_GetData(void);
IMU660RB_Status IMU660RB_GetStatus(void);
uint8_t IMU660RB_GetDeviceId(void);
bool IMU660RB_HasNewData(void);

#endif
