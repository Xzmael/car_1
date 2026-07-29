#include "task3.h"

#include <stdint.h>

#include "gray.h"
#include "motor.h"
#include "tft.h"
#include "IMU660RB/imu660rb.h"
#include "line_control.h"

#define TASK3_START_DELAY_SAMPLES (208U)
#define TASK3_DISPLAY_SAMPLES     (26U)

static bool taskFaulted;
static bool motorStarted;
static uint16_t startDelaySamples;
static uint8_t displayDivider;



static void Task3_ShowFault(IMU660RB_Status status)
{
    TFT_Clear(TFT_COLOR_BLACK);
    TFT_SetCursor(0U, 16U);
    TFT_WriteString((status == IMU660RB_STATUS_TIMEOUT) ? "TIMEOUT" : "IMU ERR");
    TFT_SetCursor(0U, 40U);
    TFT_WriteString("SW4: MENU");

}

static void Task3_ShowStatus(void)
{
    const Gray_Result gray = Gray_GetResult();
    const Motor_Status motor = Motor_GetStatus();

    TFT_Clear(TFT_COLOR_BLACK);
    TFT_SetCursor(0U, 0U);
    TFT_WriteString("Y:");
    TFT_WriteFloat2(euler.angle.yaw);
    TFT_SetCursor(0U, 16U);
    TFT_WriteString(motorStarted ?
        ((gray.status == GRAY_STATUS_LOST) ? "LOST GO" : "LINE") : "WAIT");
    TFT_SetCursor(0U, 32U);
    TFT_WriteString("P:");
    TFT_WriteInt(gray.position);
    TFT_WriteString(" N:");
    TFT_WriteUInt(gray.blackCount);
    TFT_SetCursor(0U, 48U);
    TFT_WriteString("L:");
    TFT_WriteUInt(motor.leftDuty);
    TFT_WriteString(" R:");
    TFT_WriteUInt(motor.rightDuty);

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
    }
    /* Gray control must not wait for the slower IMU data-ready cadence. */
    if (motorStarted) LineControl_Run();
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
