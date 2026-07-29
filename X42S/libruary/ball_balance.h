#ifndef LIBRUARY_BALL_BALANCE_H
#define LIBRUARY_BALL_BALANCE_H

#include <stdbool.h>
#include <stdint.h>

void BallBalance_Init(void);
void BallBalance_SetTargetTenths(int16_t targetTenthsCm);
void BallBalance_UpdatePixel(int16_t pixel);
void BallBalance_Stop(void);
bool BallBalance_IsStable(void);
int16_t BallBalance_GetPositionTenths(void);
int16_t BallBalance_GetTargetTenths(void);
int16_t BallBalance_GetPixel(void);

#endif
