#include "task3.h"

#include <stdint.h>

#include "gray.h"
#include "motor.h"
#include "oled.h"
#include "IMU660RB/imu660rb.h"
#include "line_control.h"

#define TASK3_START_DELAY_SAMPLES (208U)
#define TASK3_DISPLAY_SAMPLES     (26U)

static bool taskFaulted;
static bool motorStarted;
static uint16_t startDelaySamples;
static uint8_t displayDivider;

static void Task3_Refresh(void)
{
    if (OLED_Refresh() != OLED_STATUS_OK) {
        (void) OLED_Init();
    }
}

static void Task3_ShowFault(IMU660RB_Status status)
{
    OLED_Clear();
    OLED_SetCursor(0U, 16U);
    OLED_WriteString((status == IMU660RB_STATUS_TIMEOUT) ? "TIMEOUT" : "IMU ERR");
    OLED_SetCursor(0U, 40U);
    OLED_WriteString("SW4: MENU");
    Task3_Refresh();
}

static void Task3_ShowStatus(void)
{
    const Gray_Result gray = Gray_GetResult();
    const Motor_Status motor = Motor_GetStatus();

    OLED_Clear();
    OLED_SetCursor(0U, 0U);
    OLED_WriteString("Y:");
    OLED_WriteFloat2(euler.angle.yaw);
    OLED_SetCursor(0U, 16U);
    OLED_WriteString(motorStarted ?
        ((gray.status == GRAY_STATUS_LOST) ? "LOST GO" : "LINE") : "WAIT");
    OLED_SetCursor(0U, 32U);
    OLED_WriteString("P:");
    OLED_WriteInt(gray.position);
    OLED_WriteString(" N:");
    OLED_WriteUInt(gray.blackCount);
    OLED_SetCursor(0U, 48U);
    OLED_WriteString("L:");
    OLED_WriteUInt(motor.leftDuty);
    OLED_WriteString(" R:");
    OLED_WriteUInt(motor.rightDuty);
    Task3_Refresh();
}

void Task3_Start(void)
{
    const IMU660RB_Status status = IMU660RB_Init();

    Motor_Stop();
    LineControl_Stop();
    taskFaulted = false;
    motorStarted = false;
    startDelaySamples = TASK3_START_DELAY_SAMPLES;
    displayDivider = 0U;
    if (status != IMU660RB_STATUS_OK) {
        taskFaulted = true;
        Task3_ShowFault(status);
        return;
    }
    Gray_Read();
    Task3_ShowStatus();
}

void Task3_Update(void)
{
    IMU660RB_Status status;
    bool imuUpdated = false;

    Gray_Read();
    if (IMU660RB_HasNewData()) {
        imuUpdated = true;
        status = Read_IMU660RB();
        if (status != IMU660RB_STATUS_OK) {
            LineControl_Stop();
            taskFaulted = true;
            Task3_ShowFault(status);
            return;
        }
        if (!motorStarted && --startDelaySamples == 0U) {
            LineControl_Start();
            motorStarted = true;
        }
        if (motorStarted) {
            /* The shared controller drives straight when every sensor is white. */
            LineControl_Run();
        }
    }
    if (imuUpdated && (++displayDivider >= TASK3_DISPLAY_SAMPLES)) {
        displayDivider = 0U;
        Task3_ShowStatus();
    }
}

void Task3_Stop(void)
{
    LineControl_Stop();
}

bool Task3_HasFault(void)
{
    return taskFaulted;
}
