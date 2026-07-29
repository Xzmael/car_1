#include "task1.h"

#include "buzzer.h"
#include "ball_balance.h"
#include "key.h"
#include "oled.h"
#include "origin_sensor.h"
#include "stepper.h"
#include "vision_uart.h"

#define HOME_CHUNK_STEPS      (20U)
#define HOME_TO_LEVEL_STEPS   (800U)
#define VISION_TIMEOUT_MS     (250U)
#define REQUIRED_STABLE_FRAMES (5U)

typedef enum {
    TASK1_READY = 0,
    TASK1_HOME_SEEK,
    TASK1_HOME_LEVEL,
    TASK1_WAIT_VISION,
    TASK1_HOLD_CENTER,
    TASK1_HOLD_POS5,
    TASK1_HOLD_NEG5,
    TASK1_DONE,
    TASK1_STOPPED,
    TASK1_VISION_LOST
} Task1_State;

static Task1_State state;
static uint8_t validFrames;
static uint8_t stableFrames;
static bool runSequence;
static bool screenDirty;
static uint16_t screenPeriod;

static void Task1_ShowStatus(void)
{
    OLED_Clear();
    OLED_SetCursor(0U, 0U);
    OLED_WriteString("BALL CTRL");
    OLED_SetCursor(0U, 16U);
    OLED_WriteString("PX:");
    OLED_WriteInt(BallBalance_GetPixel());
    OLED_WriteString(" CM:");
    OLED_WriteFloat2((float) BallBalance_GetPositionTenths() / 10.0f);
    OLED_SetCursor(0U, 28U);
    OLED_WriteString("TG:");
    OLED_WriteFloat2((float) BallBalance_GetTargetTenths() / 10.0f);
    OLED_WriteString(" ST:");
    OLED_WriteInt(Stepper_GetPosition());
    OLED_SetCursor(0U, 44U);
    if (state == TASK1_READY) OLED_WriteString("SW1 RUN SW2 HOME");
    else if (state == TASK1_HOME_SEEK) OLED_WriteString("HOME SEEK");
    else if (state == TASK1_HOME_LEVEL) OLED_WriteString("HOME LEVEL");
    else if (state == TASK1_WAIT_VISION) OLED_WriteString("VISION WAIT");
    else if (state == TASK1_HOLD_CENTER) OLED_WriteString("HOLD CENTER");
    else if (state == TASK1_HOLD_POS5) OLED_WriteString("HOLD POS5");
    else if (state == TASK1_HOLD_NEG5) OLED_WriteString("HOLD NEG5");
    else if (state == TASK1_DONE) OLED_WriteString("TEST DONE");
    else if (state == TASK1_VISION_LOST) OLED_WriteString("VISION LOST");
    else OLED_WriteString("STOPPED");
    (void) OLED_Refresh();
}

static void Task1_StartHome(bool runTest)
{
    BallBalance_Stop();
    Buzzer_Off();
    validFrames = 0U;
    stableFrames = 0U;
    runSequence = runTest;
    state = TASK1_HOME_SEEK;
    screenDirty = true;
}

static void Task1_SetTarget(Task1_State next, int16_t target)
{
    BallBalance_SetTargetTenths(target);
    stableFrames = 0U;
    runSequence = false;
    state = next;
}

void Task1_Init(void)
{
    state = TASK1_READY;
    validFrames = 0U;
    stableFrames = 0U;
    screenDirty = true;
    screenPeriod = 0U;
    Buzzer_Off();
    BallBalance_Init();
    OriginSensor_Init();
    VisionUart_Init();
    Task1_ShowStatus();
}

void Task1_Update(void)
{
    const uint8_t pressed = Key_GetPressed();
    VisionUart_Frame frame;

    if ((pressed & KEY_SW2) != 0U) {
        if ((state == TASK1_READY) || (state == TASK1_STOPPED) ||
            (state == TASK1_DONE) || (state == TASK1_VISION_LOST)) Task1_StartHome(false);
        else {
            BallBalance_Stop();
            Buzzer_Off();
            state = TASK1_STOPPED;
            screenDirty = true;
        }
    } else if ((pressed & KEY_SW1) != 0U && (state == TASK1_READY)) {
        Task1_StartHome(true);
    }

    if (state == TASK1_HOME_SEEK) {
        if (OriginSensor_IsActive()) {
            Stepper_Stop();
            Stepper_SetPosition(0);
            (void) Stepper_MoveA(STEPPER_DIRECTION_UP, HOME_TO_LEVEL_STEPS);
            state = TASK1_HOME_LEVEL;
            screenDirty = true;
        } else if (!Stepper_IsBusy()) {
            (void) Stepper_MoveA(STEPPER_DIRECTION_DOWN, HOME_CHUNK_STEPS);
        }
    } else if ((state == TASK1_HOME_LEVEL) && !Stepper_IsBusy()) {
        if (runSequence) Task1_SetTarget(TASK1_WAIT_VISION, 0);
        else state = TASK1_READY;
        screenDirty = true;
    }

    if ((state >= TASK1_WAIT_VISION) && (state <= TASK1_HOLD_NEG5) &&
        (VisionUart_GetFrameAgeMs() > VISION_TIMEOUT_MS)) {
        BallBalance_Stop();
        Buzzer_Off();
        state = TASK1_VISION_LOST;
        screenDirty = true;
    }
    if (VisionUart_GetFrame(&frame)) {
        if (frame.type == VISION_UART_NONE) {
            if ((state >= TASK1_WAIT_VISION) && (state <= TASK1_HOLD_NEG5)) {
                BallBalance_Stop(); Buzzer_Off(); state = TASK1_VISION_LOST; screenDirty = true;
            }
        } else if (frame.type == VISION_UART_BALL) {
            if (state == TASK1_WAIT_VISION) {
                if (++validFrames >= 3U) Task1_SetTarget(TASK1_HOLD_CENTER, 0);
            }
            if ((state == TASK1_HOLD_CENTER) || (state == TASK1_HOLD_POS5) ||
                (state == TASK1_HOLD_NEG5)) {
                BallBalance_UpdatePixel(frame.centerX);
                if (BallBalance_IsStable()) {
                    if (stableFrames < 255U) stableFrames++;
                    if (stableFrames >= REQUIRED_STABLE_FRAMES) {
                        if (state == TASK1_HOLD_CENTER) Task1_SetTarget(TASK1_HOLD_POS5, 50);
                        else if (state == TASK1_HOLD_POS5) Task1_SetTarget(TASK1_HOLD_NEG5, -50);
                        else { BallBalance_Stop(); state = TASK1_DONE; Buzzer_On(); }
                        screenDirty = true;
                    }
                } else stableFrames = 0U;
            }
        }
    }
    if (screenDirty && (screenPeriod == 0U)) {
        screenDirty = false;
        screenPeriod = 100U;
        Task1_ShowStatus();
    }
}

void Task1_Tick1ms(void)
{
    VisionUart_Tick1ms();
    if (screenPeriod != 0U) screenPeriod--;
    screenDirty = true;
}
