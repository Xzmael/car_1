#include "task1.h"

#include "buzzer.h"
#include "key.h"
#include "oled.h"
#include "stepper.h"

#define TASK1_TEST_PULSES (100U)

static bool wasBusy;

static void Task1_ShowStatus(const char *status)
{
    OLED_Clear();
    OLED_SetCursor(0U, 0U);
    OLED_WriteString("X42S READY");
    OLED_SetCursor(0U, 16U);
    OLED_WriteString("SW1:UP");
    OLED_SetCursor(0U, 28U);
    OLED_WriteString("SW2:DOWN");
    OLED_SetCursor(0U, 44U);
    OLED_WriteString(status);
    (void) OLED_Refresh();
}

void Task1_Init(void)
{
    wasBusy = false;
    Buzzer_Off();
    Task1_ShowStatus("READY");
}

void Task1_Update(void)
{
    const uint8_t pressed = Key_GetPressed();
    const bool busy = Stepper_IsBusy();

    if (!busy && (pressed & KEY_SW1) != 0U) {
        if (Stepper_MoveBoth(STEPPER_DIRECTION_UP, TASK1_TEST_PULSES)) {
            wasBusy = true;
            Task1_ShowStatus("UP RUN");
        }
    } else if (!busy && (pressed & KEY_SW2) != 0U) {
        if (Stepper_MoveBoth(STEPPER_DIRECTION_DOWN, TASK1_TEST_PULSES)) {
            wasBusy = true;
            Task1_ShowStatus("DOWN RUN");
        }
    }
    if (wasBusy && !Stepper_IsBusy()) {
        wasBusy = false;
        Task1_ShowStatus("READY");
    }
}
