#include "task_manager.h"

#include <stdbool.h>
#include <stdint.h>

#include "buzzer.h"
#include "gray.h"
#include "key.h"
#include "led.h"
#include "motor.h"
#include "oled.h"
#include "IMU660RB/imu660rb.h"

#define TASK_COUNT (4U)
#define TASK1_DUTY (20U)
/* These loop counts give a visible blink and one short beep without blocking control. */
#define TASK1_LED_FLASH_TICKS (160U)
#define TASK1_BUZZER_TICKS (60U)
/* Briefly ignore sensor power-up transients, then always enable black-line stop. */
#define TASK1_GRAY_STARTUP_TICKS (20U)
#define TASK1_BLACK_SAMPLES_TO_STOP (2U)

typedef enum {
    TASK_MANAGER_MENU = 0,
    TASK_MANAGER_TASK1,
    TASK_MANAGER_PLACEHOLDER,
    TASK_MANAGER_IMU_FAULT
} TaskManager_State;

static TaskManager_State taskState;
static uint8_t selectedTask;
static uint8_t displayDivider;
static bool lineStopped;
static bool alarmActive;
static uint16_t ledFlashTicks;
static uint16_t buzzerTicks;
static uint8_t grayStartupTicks;
static uint8_t blackSampleCount;
static bool grayStopArmed;

static void TaskManager_ClearAlarm(void)
{
    alarmActive = false;
    ledFlashTicks = 0U;
    buzzerTicks = 0U;
    LED_Off();
    Buzzer_Off();
}

static bool TaskManager_GrayStopDetected(void)
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

static void TaskManager_StartAlarm(void)
{
    alarmActive = true;
    ledFlashTicks = 0U;
    buzzerTicks = 0U;
    LED_On();
    Buzzer_On();
}

static void TaskManager_UpdateAlarm(void)
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

static void TaskManager_Refresh(void)
{
    if (OLED_Refresh() != OLED_STATUS_OK) {
        (void) OLED_Init();
    }
}

static void TaskManager_ShowMenu(void)
{
    uint8_t task;

    OLED_Clear();
    OLED_SetCursor(0U, 0U);
    OLED_WriteString("SELECT TASK");
    for (task = 1U; task <= TASK_COUNT; task++) {
        OLED_SetCursor(0U, (uint8_t) (task * 12U + 4U));
        OLED_WriteString((task == selectedTask) ? ">T" : " T");
        OLED_WriteUInt(task);
    }
    TaskManager_Refresh();
}

static void TaskManager_ShowPlaceholder(void)
{
    OLED_Clear();
    OLED_SetCursor(0U, 16U);
    OLED_WriteString("TASK ");
    OLED_WriteUInt(selectedTask);
    OLED_SetCursor(0U, 40U);
    OLED_WriteString("NOT READY");
    TaskManager_Refresh();
}

static void TaskManager_ShowImuFault(IMU660RB_Status status)
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
    TaskManager_Refresh();
}

static void TaskManager_ShowTask1(void)
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
    TaskManager_Refresh();
}

static void TaskManager_ReturnToMenu(void)
{
    Motor_Stop();
    TaskManager_ClearAlarm();
    taskState = TASK_MANAGER_MENU;
    lineStopped = false;
    displayDivider = 0U;
    grayStartupTicks = 0U;
    blackSampleCount = 0U;
    grayStopArmed = false;
    TaskManager_ShowMenu();
}

static void TaskManager_StartTask1(void)
{
    const IMU660RB_Status status = IMU660RB_Init();

    Motor_Stop();
    lineStopped = false;
    displayDivider = 0U;
    grayStartupTicks = 0U;
    blackSampleCount = 0U;
    grayStopArmed = false;
    TaskManager_ClearAlarm();
    if (status != IMU660RB_STATUS_OK) {
        taskState = TASK_MANAGER_IMU_FAULT;
        TaskManager_ShowImuFault(status);
        return;
    }

    /* Start immediately; enable stop detection after the sensor power-up settles. */
    Motor_HoldYawStart(TASK1_DUTY);
    taskState = TASK_MANAGER_TASK1;
    TaskManager_ShowTask1();
}

void TaskManager_Init(void)
{
    taskState = TASK_MANAGER_MENU;
    selectedTask = 1U;
    displayDivider = 0U;
    lineStopped = false;
    grayStartupTicks = 0U;
    blackSampleCount = 0U;
    grayStopArmed = false;
    Motor_Stop();
    TaskManager_ClearAlarm();
    TaskManager_ShowMenu();
}

void TaskManager_Run(void)
{
    while (true) {
        uint8_t pressed;

        Key_Scan();
        pressed = Key_GetPressed();

        if ((pressed & KEY_SW4) != 0U && taskState != TASK_MANAGER_MENU) {
            TaskManager_ReturnToMenu();
            continue;
        }

        if (taskState == TASK_MANAGER_MENU) {
            if ((pressed & KEY_SW1) != 0U) {
                selectedTask = (selectedTask == 1U) ? TASK_COUNT : selectedTask - 1U;
                TaskManager_ShowMenu();
            } else if ((pressed & KEY_SW2) != 0U) {
                selectedTask = (selectedTask == TASK_COUNT) ? 1U : selectedTask + 1U;
                TaskManager_ShowMenu();
            } else if ((pressed & KEY_SW3) != 0U) {
                if (selectedTask == 1U) {
                    TaskManager_StartTask1();
                } else {
                    Motor_Stop();
                    TaskManager_ClearAlarm();
                    taskState = TASK_MANAGER_PLACEHOLDER;
                    TaskManager_ShowPlaceholder();
                }
            }
            continue;
        }

        if (taskState == TASK_MANAGER_TASK1) {
            const IMU660RB_Status status = Read_IMU660RB();

            Gray_Read();
            if (status != IMU660RB_STATUS_OK) {
                Motor_Stop();
                TaskManager_ClearAlarm();
                taskState = TASK_MANAGER_IMU_FAULT;
                TaskManager_ShowImuFault(status);
                continue;
            }
            if (!lineStopped && TaskManager_GrayStopDetected()) {
                lineStopped = true;
                Motor_Stop();
                TaskManager_StartAlarm();
                TaskManager_ShowTask1();
                displayDivider = 0U;
            }
            if (!lineStopped) {
                Motor_HoldYawUpdate(euler.angle.yaw);
            } else {
                TaskManager_UpdateAlarm();
            }
            if (++displayDivider >= 4U) {
                displayDivider = 0U;
                TaskManager_ShowTask1();
            }
        }
    }
}
