#include "task2.h"

#include <stdint.h>

#include "gray.h"
#include "motor.h"
#include "oled.h"
#include "IMU660RB/imu660rb.h"
#include "line_control.h"

#define TASK2_DISPLAY_SAMPLES (26U)

static bool taskFaulted;
static uint8_t displayDivider;
static LineControl_Output lineOutput;

static void Task2_Refresh(void)
{
    if (OLED_Refresh() != OLED_STATUS_OK) {
        (void) OLED_Init();
    }
}

static void Task2_ShowFault(IMU660RB_Status status)
{
    OLED_Clear();
    OLED_SetCursor(0U, 16U);
    OLED_WriteString((status == IMU660RB_STATUS_TIMEOUT) ? "TIMEOUT" : "IMU ERR");
    OLED_SetCursor(0U, 40U);
    OLED_WriteString("SW4: MENU");
    Task2_Refresh();
}

static void Task2_ShowStatus(void)
{
    const Gray_Result gray = Gray_GetResult();
    const Motor_Status motor = Motor_GetStatus();

    OLED_Clear();
    OLED_SetCursor(0U, 0U);
    OLED_WriteString("Y:");
    OLED_WriteFloat2(euler.angle.yaw);
    OLED_SetCursor(0U, 16U);
    if (lineOutput.mode == LINE_CONTROL_LOST) OLED_WriteString("LOST");
    else if (lineOutput.mode == LINE_CONTROL_ALL_BLACK) OLED_WriteString("ALL");
    else if (lineOutput.mode == LINE_CONTROL_HARD_LEFT) OLED_WriteString("HARD L");
    else if (lineOutput.mode == LINE_CONTROL_HARD_RIGHT) OLED_WriteString("HARD R");
    else OLED_WriteString("LINE");
    OLED_SetCursor(0U, 32U);
    OLED_WriteString("P:");
    OLED_WriteInt(lineOutput.position);
    OLED_WriteString(" N:");
    OLED_WriteUInt(gray.blackCount);
    OLED_SetCursor(0U, 48U);
    OLED_WriteString("L:");
    OLED_WriteUInt(motor.leftDuty);
    OLED_WriteString(" R:");
    OLED_WriteUInt(motor.rightDuty);
    Task2_Refresh();
}

void Task2_Start(void)
{
    const IMU660RB_Status status = IMU660RB_Init();

    Motor_Stop();
    taskFaulted = false;
    displayDivider = 0U;
    LineControl_Init();
    lineOutput.leftDuty = 0U;
    lineOutput.rightDuty = 0U;
    lineOutput.position = 0;
    lineOutput.mode = LINE_CONTROL_LOST;
    if (status != IMU660RB_STATUS_OK) {
        taskFaulted = true;
        Task2_ShowFault(status);
        return;
    }
    Gray_Read();
    lineOutput = LineControl_Update(Gray_GetResult());
    Motor_SetForwardDuty(lineOutput.leftDuty, lineOutput.rightDuty);
    Task2_ShowStatus();
}

void Task2_Update(void)
{
    IMU660RB_Status status;
    bool imuUpdated = false;

    Gray_Read();
    lineOutput = LineControl_Update(Gray_GetResult());
    Motor_SetForwardDuty(lineOutput.leftDuty, lineOutput.rightDuty);
    if (IMU660RB_HasNewData()) {
        imuUpdated = true;
        status = Read_IMU660RB();
        if (status != IMU660RB_STATUS_OK) {
            Motor_Stop();
            taskFaulted = true;
            Task2_ShowFault(status);
            return;
        }
    }
    if (imuUpdated && (++displayDivider >= TASK2_DISPLAY_SAMPLES)) {
        displayDivider = 0U;
        Task2_ShowStatus();
    }
}

void Task2_Stop(void)
{
    Motor_Stop();
}

bool Task2_HasFault(void)
{
    return taskFaulted;
}
