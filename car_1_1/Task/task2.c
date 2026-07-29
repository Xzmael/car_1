#include "task2.h"

#include <stdint.h>

#include "buzzer.h"
#include "gray.h"
#include "led.h"
#include "motor.h"
#include "tft.h"
#include "IMU660RB/imu660rb.h"
#include "line_control.h"

#define TASK2_STRAIGHT_DUTY       (42U)
#define TASK2_START_DELAY_SAMPLES (208U)
#define TASK2_EDGE_SAMPLES        (1U)
#define TASK2_ALARM_SAMPLES       (11U)
#define TASK2_DISPLAY_SAMPLES     (26U)

typedef enum {
    TASK2_WAIT = 0,
    TASK2_A_TO_B,
    TASK2_B_TO_C,
    TASK2_C_TO_D,
    TASK2_D_TO_A,
    TASK2_DONE
} Task2_State;

static Task2_State taskState;
static bool taskFaulted;
static float yawAB;
static uint16_t startDelaySamples;
static uint8_t edgeSamples;
static uint8_t alarmSamples;
static uint8_t displayDivider;



static void Task2_ClearAlarm(void)
{
    alarmSamples = 0U;
    LED_Off();
    Buzzer_Off();
}

static void Task2_StartAlarm(void)
{
    alarmSamples = TASK2_ALARM_SAMPLES;
    LED_On();
    Buzzer_On();
}

static void Task2_UpdateAlarm(void)
{
    if (alarmSamples == 0U) return;
    if (--alarmSamples == 0U) {
        LED_Off();
        Buzzer_Off();
    }
}

static bool Task2_BlackConfirmed(void)
{
    if (Gray_GetRaw() != 0U) {
        if (edgeSamples < TASK2_EDGE_SAMPLES) edgeSamples++;
    } else {
        edgeSamples = 0U;
    }
    return edgeSamples >= TASK2_EDGE_SAMPLES;
}

static bool Task2_WhiteConfirmed(void)
{
    if (Gray_GetRaw() == 0U) {
        if (edgeSamples < TASK2_EDGE_SAMPLES) edgeSamples++;
    } else {
        edgeSamples = 0U;
    }
    return edgeSamples >= TASK2_EDGE_SAMPLES;
}

static const char *Task2_StateText(void)
{
    switch (taskState) {
    case TASK2_A_TO_B: return "A-B";
    case TASK2_B_TO_C: return "B-C";
    case TASK2_C_TO_D: return "C-D";
    case TASK2_D_TO_A: return "D-A";
    case TASK2_DONE: return "DONE";
    default: return "WAIT";
    }
}

static void Task2_ShowFault(IMU660RB_Status status)
{
    TFT_Clear(TFT_COLOR_BLACK);
    TFT_SetCursor(0U, 16U);
    TFT_WriteString((status == IMU660RB_STATUS_TIMEOUT) ? "TIMEOUT" : "IMU ERR");
    TFT_SetCursor(0U, 40U);
    TFT_WriteString("SW4: MENU");

}

static void Task2_ShowStatus(void)
{
    const Motor_Status motor = Motor_GetStatus();
    const Gray_Result gray = Gray_GetResult();

    TFT_Clear(TFT_COLOR_BLACK);
    TFT_SetCursor(0U, 0U);
    TFT_WriteString("Y:");
    TFT_WriteFloat2(euler.angle.yaw);
    TFT_SetCursor(0U, 16U);
    TFT_WriteString(Task2_StateText());
    TFT_WriteString(" N:");
    TFT_WriteUInt(gray.blackCount);
    TFT_SetCursor(0U, 32U);
    TFT_WriteString("RAW:");
    TFT_WriteUInt(gray.raw);
    TFT_SetCursor(0U, 48U);
    TFT_WriteString("L:");
    TFT_WriteUInt(motor.leftDuty);
    TFT_WriteString(" R:");
    TFT_WriteUInt(motor.rightDuty);

}

static void Task2_EnterFollowing(Task2_State nextState)
{
    Motor_Stop();
    LineControl_Start();
    taskState = nextState;
    edgeSamples = 0U;
    Task2_StartAlarm();
}

void Task2_Start(void)
{
    const IMU660RB_Status status = IMU660RB_Init();

    Motor_Stop();
    LineControl_Stop();
    taskFaulted = false;
    taskState = TASK2_WAIT;
    startDelaySamples = TASK2_START_DELAY_SAMPLES;
    edgeSamples = 0U;
    displayDivider = 0U;
    yawAB = 0.0f;
    Task2_ClearAlarm();
    if (status != IMU660RB_STATUS_OK) {
        taskFaulted = true;
        Task2_ShowFault(status);
        return;
    }
    Gray_Read();
    Task2_ShowStatus();
}

void Task2_Update(void)
{
    IMU660RB_Status status;
    bool imuUpdated = false;

    Gray_Read();
    if (IMU660RB_HasNewData()) {
        imuUpdated = true;
        status = Read_IMU660RB();
        if (status != IMU660RB_STATUS_OK) {
            LineControl_Stop();
            Task2_ClearAlarm();
            taskFaulted = true;
            Task2_ShowFault(status);
            return;
        }

        if (taskState == TASK2_WAIT && --startDelaySamples == 0U) {
            yawAB = euler.angle.yaw;
            Motor_HoldYawTargetStart(TASK2_STRAIGHT_DUTY, yawAB);
            taskState = TASK2_A_TO_B;
            edgeSamples = 0U;
        }

        if (taskState == TASK2_A_TO_B) {
            Motor_HoldYawUpdate(euler.angle.yaw);
            if (Task2_BlackConfirmed()) {
                Task2_EnterFollowing(TASK2_B_TO_C);
            }
        } else if (taskState == TASK2_B_TO_C) {
            if (Task2_WhiteConfirmed()) {
                LineControl_Stop();
                Motor_HoldYawTargetStart(TASK2_STRAIGHT_DUTY, yawAB + 180.0f);
                taskState = TASK2_C_TO_D;
                edgeSamples = 0U;
                Task2_StartAlarm();
            }
        } else if (taskState == TASK2_C_TO_D) {
            Motor_HoldYawUpdate(euler.angle.yaw);
            if (Task2_BlackConfirmed()) {
                Task2_EnterFollowing(TASK2_D_TO_A);
            }
        } else if (taskState == TASK2_D_TO_A) {
            if (Task2_WhiteConfirmed()) {
                LineControl_Stop();
                taskState = TASK2_DONE;
                edgeSamples = 0U;
                Task2_StartAlarm();
            }
        }
        Task2_UpdateAlarm();
    }

    /* Run line correction on every main-loop pass instead of at the IMU rate. */
    if ((taskState == TASK2_B_TO_C) || (taskState == TASK2_D_TO_A)) {
        LineControl_Run();
    }

    if (imuUpdated && (++displayDivider >= TASK2_DISPLAY_SAMPLES)) {
        displayDivider = 0U;
        Task2_ShowStatus();
    }
}

void Task2_Stop(void)
{
    LineControl_Stop();
    Task2_ClearAlarm();
    taskState = TASK2_DONE;
}

bool Task2_HasFault(void)
{
    return taskFaulted;
}
