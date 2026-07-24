#ifndef _IMU660RB_H_
#define _IMU660RB_H_

#include "Fusion/Fusion.h"

typedef enum {
    IMU660RB_STATUS_OK = 0,
    IMU660RB_STATUS_I2C_ERROR,
    IMU660RB_STATUS_TIMEOUT,
    IMU660RB_STATUS_DEVICE_ID_ERROR
} IMU660RB_Status;

extern float acceleration_mg[3];
extern float angular_rate_mdps[3];

extern FusionAhrs ahrs;
extern FusionEuler euler;

IMU660RB_Status IMU660RB_Init(void);
IMU660RB_Status IMU660RB_GetStatus(void);
uint8_t IMU660RB_GetDeviceId(void);
uint8_t IMU660RB_GetStage(void);
IMU660RB_Status Read_IMU660RB(void);

#endif  /* #ifndef _IMU660RB_H_ */
