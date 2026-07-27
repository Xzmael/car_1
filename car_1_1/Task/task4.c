#include "task4.h"

#include <stdbool.h>

#include "buzzer.h"
#include "led.h"
#include "motor.h"
#include "oled.h"
#include "vision_uart.h"

static bool visionDetected;

static void Task4_Refresh(void)
{
    if (OLED_Refresh() != OLED_STATUS_OK) {
        (void) OLED_Init();
    }
}

static void Task4_ShowStatus(void)
{
    OLED_Clear();
    OLED_SetCursor(0U, 8U);
    OLED_WriteString(visionDetected ? "VISION OK" : "VISION WAIT");
    OLED_SetCursor(0U, 32U);
    OLED_WriteString("B:");
    OLED_WriteUInt(VisionUart_GetByteCount());
    OLED_WriteString(" F:");
    OLED_WriteUInt(VisionUart_GetFrameCount());
    OLED_SetCursor(0U, 52U);
    OLED_WriteString("SW4: MENU");
    Task4_Refresh();
}

void Task4_Start(void)
{
    Motor_Stop();
    LED_Off();
    Buzzer_Off();
    VisionUart_Init();
    visionDetected = false;
    Task4_ShowStatus();
}

void Task4_Update(void)
{
    VisionUart_Poll();
    if (!visionDetected && VisionUart_HasFrame()) {
        visionDetected = true;
        Task4_ShowStatus();
    } else if (!visionDetected && VisionUart_GetByteCount() != 0U) {
        Task4_ShowStatus();
    }
}

void Task4_Stop(void)
{
    Motor_Stop();
    LED_Off();
    Buzzer_Off();
}
