#include "task1.h"

#include <stdbool.h>
#include <stdint.h>

#include "buzzer.h"
#include "gray.h"
#include "led.h"
#include "motor.h"
#include "tft.h"
#include "IMU660RB/imu660rb.h"

#define TASK1_DUTY (30U)
#define TASK1_LED_FLASH_TICKS (160U)
#define TASK1_BUZZER_TICKS (60U)
#define TASK1_GRAY_STARTUP_TICKS (20U)
#define TASK1_BLACK_SAMPLES_TO_STOP (2U)
#define TASK1_DISPLAY_SAMPLES (26U)
#define TASK_START_DELAY_SAMPLES (208U)

static uint8_t displayDivider;
static bool lineStopped;
static bool alarmActive;
static uint16_t ledFlashTicks;
static uint16_t buzzerTicks;
static uint8_t grayStartupTicks;
static uint8_t blackSampleCount;
static bool grayStopArmed;
static bool taskFaulted;
static bool motorStarted;
static uint16_t startDelaySamples;



static void Task1_ClearAlarm(void)
{
    alarmActive = false;
    ledFlashTicks = 0U;
    buzzerTicks = 0U;
    LED_Off();
    Buzzer_Off();
}

static void Task1_StartAlarm(void)
{
    alarmActive = true;
    ledFlashTicks = 0U;
    buzzerTicks = 0U;
    LED_On();
    Buzzer_On();
}

static void Task1_UpdateAlarm(void)
{
    if (!alarmActive) return;

    if (++ledFlashTicks >= TASK1_LED_FLASH_TICKS) {
        ledFlashTicks = 0U;
        LED_Toggle();
    }
    if (++buzzerTicks >= TASK1_BUZZER_TICKS) {
        Buzzer_Off();
    }
}

static bool Task1_GrayStopDetected(void)
{
    if (!grayStopArmed) {
        if (++grayStartupTicks >= TASK1_GRAY_STARTUP_TICKS) {
            grayStopArmed = true;
        }
        return false;
    }
    if (Gray_GetRaw() != 0U) {
        if (++blackSampleCount >= TASK1_BLACK_SAMPLES_TO_STOP) {
            return true;
        }
    } else {
        blackSampleCount = 0U;
    }
    return false;
}

static void Task1_ShowFault(IMU660RB_Status status)
{
    TFT_Clear(TFT_COLOR_BLACK);
    TFT_SetCursor(0U, 16U);
    if (status == IMU660RB_STATUS_DEVICE_ID_ERROR) {
        TFT_WriteString("ID ERR");
    } else if (status == IMU660RB_STATUS_TIMEOUT) {
        TFT_WriteString("TIMEOUT");
    } else {
        TFT_WriteString("IMU ERR");
    }
    TFT_SetCursor(0U, 40U);
    TFT_WriteString("SW4: MENU");

}

static void Task1_ShowStatus(void)
{
    const Motor_Status motor = Motor_GetStatus();
    const Gray_Result gray = Gray_GetResult();

    TFT_Clear(TFT_COLOR_BLACK);
    TFT_SetCursor(0U, 0U);
    TFT_WriteString("Y:");
    TFT_WriteFloat2(euler.angle.yaw);
    TFT_SetCursor(0U, 16U);
    TFT_WriteString("L:");
    TFT_WriteUInt(motor.leftDuty);
    TFT_WriteString(" R:");
    TFT_WriteUInt(motor.rightDuty);
    TFT_SetCursor(0U, 32U);
    TFT_WriteString("RAW:");
    TFT_WriteUInt(gray.raw);
    TFT_SetCursor(0U, 48U);
    TFT_WriteString(lineStopped ? "STOP N:" :
                     (motorStarted ? "GO N:" : "WAIT N:"));
    TFT_WriteUInt(gray.blackCount);

}

void Task1_Start(void)
{
    const IMU660RB_Status status = IMU660RB_Init();

    Motor_Stop();
    lineStopped = false;
    displayDivider = 0U;
    grayStartupTicks = 0U;
    blackSampleCount = 0U;
    grayStopArmed = false;
    taskFaulted = false;
    motorStarted = false;
    startDelaySamples = TASK_START_DELAY_SAMPLES;
    Task1_ClearAlarm();
    if (status != IMU660RB_STATUS_OK) {
        taskFaulted = true;
        Task1_ShowFault(status);
        return;
    }

    /* IMU frames keep running during the start delay; only PWM is held off. */
    Task1_ShowStatus();
}

void Task1_Update(void)
{
    IMU660RB_Status status = IMU660RB_STATUS_OK;
    bool imuUpdated = false;

    Gray_Read();
    if (IMU660RB_HasNewData()) {
        imuUpdated = true;
        status = Read_IMU660RB();
        if (status != IMU660RB_STATUS_OK) {
            Motor_Stop();
            Task1_ClearAlarm();
            taskFaulted = true;
            Task1_ShowFault(status);
            return;
        }
        if (!motorStarted && --startDelaySamples == 0U) {
            Motor_HoldYawStart(TASK1_DUTY);
            motorStarted = true;
        }
        if (motorStarted && !lineStopped) {
            Motor_HoldYawUpdate(euler.angle.yaw);
        }
    }
    if (motorStarted && !lineStopped && Task1_GrayStopDetected()) {
        lineStopped = true;
        Motor_Stop();
        Task1_StartAlarm();
        Task1_ShowStatus();
        displayDivider = 0U;
    }
    if (lineStopped) {
        Task1_UpdateAlarm();
    }
    /* Limit full-screen TFT updates while IMU control remains at full rate. */
    if (imuUpdated && (++displayDivider >= TASK1_DISPLAY_SAMPLES)) {
        displayDivider = 0U;
        Task1_ShowStatus();
    }
}

void Task1_Stop(void)
{
    Motor_Stop();
    Task1_ClearAlarm();
}

bool Task1_HasFault(void)
{
    return taskFaulted;
}
