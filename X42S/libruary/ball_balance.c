#include "ball_balance.h"

#include "stepper.h"

/* Measure these three values with the ball at the printed -5/0/+5 cm marks. */
#define PIXEL_AT_NEG5      (100)
#define PIXEL_AT_CENTER    (160)
#define PIXEL_AT_POS5      (220)
#define BALL_DEADBAND_TENTHS (3)
#define BACKLASH_STEPS     (8U)
#define SOFT_MIN_STEPS     (0)
#define SOFT_MAX_STEPS     (1800)

/* Change to 0 if an action that should move the ball right moves it left. */
#define POSITIVE_BALL_NEEDS_UP (1)

static int16_t positionTenths;
static int16_t targetTenths;
static int16_t lastError;
static int16_t lastPixel;
static bool stable;
static Stepper_Direction lastDirection;
static bool directionValid;

static int16_t PixelToTenths(int16_t pixel)
{
    int32_t value;
    if (pixel <= PIXEL_AT_CENTER) {
        value = ((int32_t) (pixel - PIXEL_AT_CENTER) * 50) /
                (PIXEL_AT_CENTER - PIXEL_AT_NEG5);
    } else {
        value = ((int32_t) (pixel - PIXEL_AT_CENTER) * 50) /
                (PIXEL_AT_POS5 - PIXEL_AT_CENTER);
    }
    if (value > 32767) value = 32767;
    if (value < -32768) value = -32768;
    return (int16_t) value;
}

static uint16_t ErrorToSteps(int16_t error, int16_t delta)
{
    int16_t magnitude = (error < 0) ? (int16_t) -error : error;
    int16_t derivative = (delta < 0) ? (int16_t) -delta : delta;
    uint16_t steps = 4U + (uint16_t) magnitude * 2U + (uint16_t) derivative;
    if (steps > 80U) steps = 80U;
    return steps;
}

void BallBalance_Init(void)
{
    positionTenths = 0;
    targetTenths = 0;
    lastError = 0;
    lastPixel = 0;
    stable = false;
    directionValid = false;
}

void BallBalance_SetTargetTenths(int16_t target)
{
    targetTenths = target;
    stable = false;
    lastError = 0;
}

void BallBalance_UpdatePixel(int16_t pixel)
{
    int16_t error;
    int16_t delta;
    Stepper_Direction direction;
    uint16_t steps;
    int32_t nextPosition;

    lastPixel = pixel;
    positionTenths = PixelToTenths(pixel);
    error = (int16_t) (targetTenths - positionTenths);
    if ((error >= -BALL_DEADBAND_TENTHS) && (error <= BALL_DEADBAND_TENTHS)) {
        stable = true;
        lastError = error;
        return;
    }
    stable = false;
    if (Stepper_IsBusy()) return;

    delta = (int16_t) (error - lastError);
    steps = ErrorToSteps(error, delta);
    if (error > 0) {
        direction = POSITIVE_BALL_NEEDS_UP ? STEPPER_DIRECTION_UP : STEPPER_DIRECTION_DOWN;
    } else {
        direction = POSITIVE_BALL_NEEDS_UP ? STEPPER_DIRECTION_DOWN : STEPPER_DIRECTION_UP;
    }
    nextPosition = (int32_t) Stepper_GetPosition() +
        ((direction == STEPPER_DIRECTION_UP) ? (int32_t) steps : -(int32_t) steps);
    if ((nextPosition < SOFT_MIN_STEPS) || (nextPosition > SOFT_MAX_STEPS)) return;
    if (directionValid && (direction != lastDirection)) steps = (uint16_t) (steps + BACKLASH_STEPS);
    if (Stepper_MoveA(direction, steps)) {
        lastDirection = direction;
        directionValid = true;
    }
    lastError = error;
}

void BallBalance_Stop(void)
{
    Stepper_Stop();
    stable = false;
    directionValid = false;
}

bool BallBalance_IsStable(void) { return stable; }
int16_t BallBalance_GetPositionTenths(void) { return positionTenths; }
int16_t BallBalance_GetTargetTenths(void) { return targetTenths; }
int16_t BallBalance_GetPixel(void) { return lastPixel; }
