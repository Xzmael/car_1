#include "task1.h"

#include <stdbool.h>
#include <stdint.h>

#include "buzzer.h"
#include "gray.h"
#include "led.h"
#include "motor.h"
#include "oled.h"
#include "IMU660RB/imu660rb.h"

#define TASK1_DUTY (20U)
#define TASK1_LED_FLASH_TICKS (160U)
#define TASK1_BUZZER_TICKS (60U)
#define TASK1_GRAY_STARTUP_TICKS (20U)
#define TASK1_BLACK_SAMPLES_TO_STOP (2U)
#define TASK1_DISPLAY_SAMPLES (26U)

static uint8_t displayDivider;
static bool lineStopped;
static bool alarmActive;
static uint16_t ledFlashTicks;
static uint16_t buzzerTicks;
static uint8_t grayStartupTicks;
static uint8_t blackSampleCount;
static bool grayStopArmed;
static bool taskFaulted;

static void Task1_Refresh(void)
{
    if (OLED_Refresh() != OLED_STATUS_OK) {
        (void) OLED_Init();
    }
}

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
    OLED_Clear();
    OLED_SetCursor(0U, 16U);
    if (status == IMU660RB_STATUS_DEVICE_ID_ERROR) {
        OLED_WriteString("ID ERR");
    } else if (status == IMU660RB_STATUS_TIMEOUT) {
        OLED_WriteString("TIMEOUT");
    } else {
        OLED_WriteString("IMU ERR");
    }
    OLED_SetCursor(0U, 40U);
    OLED_WriteString("SW4: MENU");
    Task1_Refresh();
}

static void Task1_ShowStatus(void)
{
    const Motor_Status motor = Motor_GetStatus();
    const Gray_Result gray = Gray_GetResult();

    OLED_Clear();
    OLED_SetCursor(0U, 0U);
    OLED_WriteString("Y:");
    OLED_WriteFloat2(euler.angle.yaw);
    OLED_SetCursor(0U, 16U);
    OLED_WriteString("L:");
    OLED_WriteUInt(motor.leftDuty);
    OLED_WriteString(" R:");
    OLED_WriteUInt(motor.rightDuty);
    OLED_SetCursor(0U, 32U);
    OLED_WriteString("RAW:");
    OLED_WriteUInt(gray.raw);
    OLED_SetCursor(0U, 48U);
    OLED_WriteString(lineStopped ? "STOP N:" : "GO N:");
    OLED_WriteUInt(gray.blackCount);
    Task1_Refresh();
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
    Task1_ClearAlarm();
    if (status != IMU660RB_STATUS_OK) {
        taskFaulted = true;
        Task1_ShowFault(status);
        return;
    }

    Motor_HoldYawStart(TASK1_DUTY);
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
        if (!lineStopped) {
            Motor_HoldYawUpdate(euler.angle.yaw);
        }
    }
    if (!lineStopped && Task1_GrayStopDetected()) {
        lineStopped = true;
        Motor_Stop();
        Task1_StartAlarm();
        Task1_ShowStatus();
        displayDivider = 0U;
    }
    if (lineStopped) {
        Task1_UpdateAlarm();
    }
    /* Full OLED refresh is slow; only refresh after roughly half a second of IMU frames. */
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
