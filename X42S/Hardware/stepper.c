#include "stepper.h"

#include "ti_msp_dl_config.h"

static volatile uint32_t remainingPulses;
static volatile bool pulseLow;
static volatile bool stepperBusy;
static volatile int32_t stepperPosition;
static volatile Stepper_Direction activeDirection;

static void Stepper_SetPulHigh(void)
{
    DL_GPIO_setPins(STEP_A_PUL_PORT, STEP_A_PUL_PIN);
}

static void Stepper_SetPulLow(void)
{
    DL_GPIO_clearPins(STEP_A_PUL_PORT, STEP_A_PUL_PIN);
}

static void Stepper_Disable(void)
{
    DL_GPIO_setPins(STEP_A_EN_PORT, STEP_A_EN_PIN);
    DL_GPIO_setPins(STEP_B_EN_PORT, STEP_B_EN_PIN); /* B is permanently unused. */
}

static void Stepper_Enable(void)
{
    DL_GPIO_clearPins(STEP_A_EN_PORT, STEP_A_EN_PIN);
}

void Stepper_Init(void)
{
    remainingPulses = 0U;
    pulseLow = false;
    stepperBusy = false;
    stepperPosition = 0;
    activeDirection = STEPPER_DIRECTION_UP;
    DL_TimerG_stopCounter(STEPPER_TIMER_INST);
    Stepper_SetPulHigh();
    Stepper_Disable();
    DL_GPIO_clearPins(STEP_A_DIR_PORT, STEP_A_DIR_PIN | STEP_B_DIR_PIN);
}

bool Stepper_MoveA(Stepper_Direction direction, uint32_t pulses)
{
    if (stepperBusy || (pulses == 0U)) return false;

    if (direction == STEPPER_DIRECTION_DOWN) {
        DL_GPIO_setPins(STEP_A_DIR_PORT, STEP_A_DIR_PIN);
    } else {
        DL_GPIO_clearPins(STEP_A_DIR_PORT, STEP_A_DIR_PIN);
    }
    activeDirection = direction;
    remainingPulses = pulses;
    pulseLow = false;
    Stepper_SetPulHigh();
    Stepper_Enable();
    stepperBusy = true;
    DL_TimerG_startCounter(STEPPER_TIMER_INST);
    return true;
}

void Stepper_Stop(void)
{
    DL_TimerG_stopCounter(STEPPER_TIMER_INST);
    Stepper_SetPulHigh();
    Stepper_Disable();
    remainingPulses = 0U;
    pulseLow = false;
    stepperBusy = false;
}

bool Stepper_IsBusy(void)
{
    return stepperBusy;
}

int32_t Stepper_GetPosition(void)
{
    return stepperPosition;
}

void Stepper_SetPosition(int32_t position)
{
    if (!stepperBusy) stepperPosition = position;
}

void Stepper_TimerIRQHandler(void)
{
    if (!stepperBusy) return;

    if (!pulseLow) {
        Stepper_SetPulLow();
        pulseLow = true;
    } else {
        Stepper_SetPulHigh();
        pulseLow = false;
        if (activeDirection == STEPPER_DIRECTION_UP) stepperPosition++;
        else stepperPosition--;
        if (--remainingPulses == 0U) Stepper_Stop();
    }
}
