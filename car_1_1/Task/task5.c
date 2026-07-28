#include "task5.h"

#include <stdbool.h>
#include <stdint.h>

#include "buzzer.h"
#include "led.h"
#include "motor.h"
#include "tft.h"
#include "vision_uart.h"

#define TASK5_BASE_DUTY       (15U)
#define TASK5_CENTER_X        (224)
#define TASK5_CENTER_DEADBAND (20)
#define TASK5_MAX_CORRECTION  (10)
#define TASK5_CONFIRM_FRAMES  (2U)
#define TASK5_FRAME_TIMEOUT_MS (300U)

static uint8_t detectedFrames;
static bool tracking;
static VisionUart_Frame target;

static uint8_t Task5_ClampDuty(int16_t duty)
{
    if (duty <= 0) return 0U;
    if (duty >= 100) return 100U;
    return (uint8_t) duty;
}

static void Task5_ShowStatus(bool fullRefresh)
{
    const Motor_Status motor = Motor_GetStatus();
    const int16_t center = (int16_t) (target.x + (int16_t) (target.width / 2U));

    if (fullRefresh) TFT_Clear(TFT_COLOR_BLACK);
    else {
        TFT_ClearLine(0U, TFT_COLOR_BLACK);
        TFT_ClearLine(16U, TFT_COLOR_BLACK);
        TFT_ClearLine(32U, TFT_COLOR_BLACK);
        TFT_ClearLine(48U, TFT_COLOR_BLACK);
    }
    TFT_SetCursor(0U, 0U);
    TFT_WriteString(tracking ? "BALL RUN" : "BALL WAIT");
    TFT_SetCursor(0U, 16U);
    TFT_WriteString("X:");
    TFT_WriteInt(center);
    TFT_WriteString(" E:");
    TFT_WriteInt(center - TASK5_CENTER_X);
    TFT_SetCursor(0U, 32U);
    TFT_WriteString("L:");
    TFT_WriteUInt(motor.leftDuty);
    TFT_WriteString(" R:");
    TFT_WriteUInt(motor.rightDuty);
    TFT_SetCursor(0U, 48U);
    TFT_WriteString("F:");
    TFT_WriteUInt(VisionUart_GetFrameCount());
    TFT_SetCursor(0U, 64U);
    TFT_WriteString("SW4: MENU");
}

static void Task5_Track(void)
{
    const int16_t center = (int16_t) (target.x + (int16_t) (target.width / 2U));
    int16_t correction = (int16_t) ((center - TASK5_CENTER_X) / 12);

    if ((center - TASK5_CENTER_X) < TASK5_CENTER_DEADBAND &&
        (center - TASK5_CENTER_X) > -TASK5_CENTER_DEADBAND) correction = 0;
    if (correction > TASK5_MAX_CORRECTION) correction = TASK5_MAX_CORRECTION;
    if (correction < -TASK5_MAX_CORRECTION) correction = -TASK5_MAX_CORRECTION;
    /* A left-side target makes the left wheel slower and the right wheel faster. */
    Motor_SetForwardDuty(Task5_ClampDuty((int16_t) TASK5_BASE_DUTY + correction),
        Task5_ClampDuty((int16_t) TASK5_BASE_DUTY - correction));
}

void Task5_Start(void)
{
    Motor_Stop();
    LED_Off();
    Buzzer_Off();
    VisionUart_Init();
    detectedFrames = 0U;
    tracking = false;
    target.type = VISION_FRAME_NONE;
    target.x = 0;
    target.width = 0U;
    target.height = 0U;
    Task5_ShowStatus(true);
}

void Task5_Update(void)
{
    VisionUart_Frame frame;
    bool displayChanged = false;

    VisionUart_Poll();
    while (VisionUart_GetFrame(&frame)) {
        displayChanged = true;
        if (frame.type == VISION_FRAME_BALL) {
            target = frame;
            if (detectedFrames < TASK5_CONFIRM_FRAMES) detectedFrames++;
            if (detectedFrames >= TASK5_CONFIRM_FRAMES) tracking = true;
        } else {
            detectedFrames = 0U;
            tracking = false;
            Motor_Stop();
        }
    }
    if (VisionUart_GetFrameAgeMs() >= TASK5_FRAME_TIMEOUT_MS) {
        detectedFrames = 0U;
        tracking = false;
        Motor_Stop();
        displayChanged = true;
    }
    if (tracking) Task5_Track();
    if (displayChanged) Task5_ShowStatus(false);
}

void Task5_Stop(void)
{
    tracking = false;
    Motor_Stop();
    LED_Off();
    Buzzer_Off();
}
