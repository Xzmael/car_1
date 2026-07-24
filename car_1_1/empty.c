/* TB6612 straight-drive verification with IMU yaw hold and line-stop latch. */
#include "ti_msp_dl_config.h"
#include "Hardware/gray.h"
#include "Hardware/IMU660RB/imu660rb.h"
#include "Hardware/motor.h"
#include "Hardware/oled.h"

static void OLED_ShowFault(IMU660RB_Status status)
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
    OLED_WriteString("MOTOR STOP");
    if (OLED_Refresh() != OLED_STATUS_OK) {
        (void) OLED_Init();
    }
}

static void OLED_ShowDrive(bool lineStopped, bool imuOnline)
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
    OLED_WriteString(imuOnline ? "E:" : "IMU ERR");
    if (imuOnline) {
        OLED_WriteFloat2(motor.yawError);
    }
    OLED_SetCursor(0U, 48U);
    if (lineStopped) {
        OLED_WriteString("STOP N:");
        OLED_WriteUInt(gray.blackCount);
    } else {
        OLED_WriteString("GO N:");
        OLED_WriteUInt(gray.blackCount);
    }
    /* Recover the software I2C display if motor noise interrupted a transfer. */
    if (OLED_Refresh() != OLED_STATUS_OK) {
        (void) OLED_Init();
    }
}

int main(void)
{
    IMU660RB_Status imuStatus;
    bool lineStopped = false;
    bool imuOnline = true;
    uint8_t displayDivider = 0U;

    SYSCFG_DL_init();
    Motor_Init();
    Gray_Init();
    delay_cycles(6400000U);

    if (OLED_Init() != OLED_STATUS_OK) {
        while (1) { Motor_Stop(); }
    }

    OLED_Clear();
    OLED_SetCursor(0U, 24U);
    OLED_WriteString("IMU INIT");
    (void) OLED_Refresh();

    imuStatus = IMU660RB_Init();
    if (imuStatus != IMU660RB_STATUS_OK) {
        Motor_Stop();
        OLED_ShowFault(imuStatus);
        while (1) { }
    }

    /* Do not start moving after a reset while the stop line is still present. */
    Gray_Read();
    if (Gray_GetResult().blackCount != 0U) {
        lineStopped = true;
        Motor_Stop();
    } else {
        Motor_HoldYawStart(30U);
    }
    OLED_ShowDrive(lineStopped, true);
    while (1) {
        if (Read_IMU660RB() != IMU660RB_STATUS_OK) {
            imuOnline = false;
        } else {
            imuOnline = true;
        }

        Gray_Read();
        if (!lineStopped && (Gray_GetResult().blackCount != 0U)) {
            lineStopped = true;
            Motor_Stop();
            /* Refresh the stop state once immediately, then use the slow refresh rate. */
            OLED_ShowDrive(lineStopped, imuOnline);
            displayDivider = 0U;
        }
        if (!lineStopped && imuOnline) {
            Motor_HoldYawUpdate(euler.angle.yaw);
        }
        /* Full-screen software I2C updates are intentionally slower than control. */
        if (++displayDivider >= 4U) {
            displayDivider = 0U;
            OLED_ShowDrive(lineStopped, imuOnline);
        }
    }
}
