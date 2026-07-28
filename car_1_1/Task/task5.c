#include "task5.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

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
#define TASK5_DISPLAY_DIVIDER  (5U)

static uint8_t detectedFrames;
static bool tracking;
static VisionUart_Frame target;
static uint8_t displayDivider;

static uint8_t Task5_ClampDuty(int16_t duty)
{
    if (duty <= 0) return 0U;
    if (duty >= 100) return 100U;
    return (uint8_t) duty;
}

static void Task5_WritePadded(const char *text)
{
    uint8_t length = 0U;

    while ((text[length] != '\0') && (length < 16U)) length++;
    TFT_WriteString(text);
    while (length++ < 16U) TFT_WriteChar(' ');
}

static void Task5_ShowStatus(bool fullRefresh)
{
    const Motor_Status motor = Motor_GetStatus();
    const int16_t center = (int16_t) (target.x + (int16_t) (target.width / 2U));
    char text[17];

    if (fullRefresh) TFT_Clear(TFT_COLOR_BLACK);
    TFT_SetCursor(0U, 0U);
    Task5_WritePadded(tracking ? "BALL RUN" : "BALL WAIT");
    TFT_SetCursor(0U, 16U);
    (void) snprintf(text, sizeof(text), "X:%d E:%d", center,
        (int) (center - TASK5_CENTER_X));
    Task5_WritePadded(text);
    TFT_SetCursor(0U, 32U);
    (void) snprintf(text, sizeof(text), "L:%u R:%u", motor.leftDuty, motor.rightDuty);
    Task5_WritePadded(text);
    TFT_SetCursor(0U, 48U);
    (void) snprintf(text, sizeof(text), "F:%lu", (unsigned long) VisionUart_GetFrameCount());
    Task5_WritePadded(text);
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
    displayDivider = 0U;
    Task5_ShowStatus(true);
}

void Task5_Update(void)
{
    VisionUart_Frame frame;
    bool displayChanged = false;

    VisionUart_Poll();
    while (VisionUart_GetFrame(&frame)) {
        if (++displayDivider >= TASK5_DISPLAY_DIVIDER) {
            displayDivider = 0U;
            displayChanged = true;
        }
        if (frame.type == VISION_FRAME_BALL) {
            target = frame;
            if (detectedFrames < TASK5_CONFIRM_FRAMES) detectedFrames++;
            if (detectedFrames >= TASK5_CONFIRM_FRAMES) {
                if (!tracking) displayChanged = true;
                tracking = true;
            }
        } else {
            if (tracking || (detectedFrames != 0U)) displayChanged = true;
            detectedFrames = 0U;
            tracking = false;
            Motor_Stop();
        }
    }
    if (VisionUart_GetFrameAgeMs() >= TASK5_FRAME_TIMEOUT_MS) {
        if (tracking || (detectedFrames != 0U)) displayChanged = true;
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
