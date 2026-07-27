#include "task4.h"

#include <stdbool.h>

#include "buzzer.h"
#include "led.h"
#include "motor.h"
#include "tft.h"
#include "vision_uart.h"

static bool visionDetected;



static void Task4_ShowStatus(void)
{
    TFT_Clear(TFT_COLOR_BLACK);
    TFT_SetCursor(0U, 8U);
    TFT_WriteString(visionDetected ? "VISION OK" : "VISION WAIT");
    TFT_SetCursor(0U, 32U);
    TFT_WriteString("B:");
    TFT_WriteUInt(VisionUart_GetByteCount());
    TFT_WriteString(" F:");
    TFT_WriteUInt(VisionUart_GetFrameCount());
    TFT_SetCursor(0U, 52U);
    TFT_WriteString("SW4: MENU");

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
