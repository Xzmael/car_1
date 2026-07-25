#include "motor.h"

#include "ti_msp_dl_config.h"
#include "IMU660RB/imu660rb.h"

#define MOTOR_PWM_PERIOD       (1600U)
#define MOTOR_MAX_DUTY         (100U)
#define MOTOR_MAX_DIFFERENTIAL (3.0f)
#define MOTOR_YAW_P_GAIN       (0.45f)
#define MOTOR_YAW_DEADBAND     (4.0f)
#define MOTOR_YAW_JUMP_LIMIT   (15.0f)

static Motor_Status motorStatus;
static uint8_t motorBaseDuty;
static float motorLastYaw;

static uint8_t Motor_ClampDuty(float duty)
{
    if (duty <= 0.0f) return 0U;
    if (duty >= (float) MOTOR_MAX_DUTY) return MOTOR_MAX_DUTY;
    return (uint8_t) duty;
}

static float Motor_NormalizeAngle(float angle)
{
    while (angle > 180.0f) angle -= 360.0f;
    while (angle <= -180.0f) angle += 360.0f;
    return angle;
}

static void Motor_SetDuty(uint8_t leftDuty, uint8_t rightDuty)
{
    const uint32_t leftCompare = ((uint32_t) leftDuty * MOTOR_PWM_PERIOD) / MOTOR_MAX_DUTY;
    const uint32_t rightCompare = ((uint32_t) rightDuty * MOTOR_PWM_PERIOD) / MOTOR_MAX_DUTY;

    /* TB6612 B channel drives the left wheel; A channel drives the right wheel. */
    DL_TimerG_setCaptureCompareValue(MOTOR_PWMB_INST, leftCompare,
        GPIO_MOTOR_PWMB_C1_IDX);
    DL_TimerG_setCaptureCompareValue(MOTOR_PWMA_INST, rightCompare,
        GPIO_MOTOR_PWMA_C0_IDX);
    motorStatus.leftDuty = leftDuty;
    motorStatus.rightDuty = rightDuty;
}

void Motor_Init(void)
{
    motorStatus.targetYaw = 0.0f;
    motorStatus.yawError = 0.0f;
    motorStatus.leftDuty = 0U;
    motorStatus.rightDuty = 0U;
    motorStatus.running = false;
    motorBaseDuty = 0U;
    DL_TimerG_startCounter(MOTOR_PWMA_INST);
    DL_TimerG_startCounter(MOTOR_PWMB_INST);
    Motor_Stop();
}

void Motor_SetForward(uint8_t duty)
{
    Motor_SetForwardDuty(duty, duty);
}

void Motor_SetForwardDuty(uint8_t leftDuty, uint8_t rightDuty)
{
    if (leftDuty > MOTOR_MAX_DUTY) leftDuty = MOTOR_MAX_DUTY;
    if (rightDuty > MOTOR_MAX_DUTY) rightDuty = MOTOR_MAX_DUTY;
    /* Motor wiring requires IN1 low and IN2 high for vehicle-forward motion. */
    DL_GPIO_clearPins(MOTOR_DIR_AIN1_PORT, MOTOR_DIR_AIN1_PIN);
    DL_GPIO_setPins(MOTOR_DIR_AIN2_PORT, MOTOR_DIR_AIN2_PIN);
    DL_GPIO_clearPins(MOTOR_DIR_BIN1_PORT, MOTOR_DIR_BIN1_PIN);
    DL_GPIO_setPins(MOTOR_DIR_BIN2_PORT, MOTOR_DIR_BIN2_PIN);
    Motor_SetDuty(leftDuty, rightDuty);
    motorStatus.running = (leftDuty != 0U) || (rightDuty != 0U);
}

void Motor_Stop(void)
{
    Motor_SetDuty(0U, 0U);
    DL_GPIO_clearPins(MOTOR_DIR_AIN1_PORT, MOTOR_DIR_AIN1_PIN | MOTOR_DIR_AIN2_PIN);
    DL_GPIO_clearPins(MOTOR_DIR_BIN1_PORT, MOTOR_DIR_BIN1_PIN | MOTOR_DIR_BIN2_PIN);
    motorStatus.yawError = 0.0f;
    motorStatus.running = false;
}

void Motor_HoldYawStart(uint8_t duty)
{
    Motor_HoldYawTargetStart(duty, euler.angle.yaw);
}

void Motor_HoldYawTargetStart(uint8_t duty, float targetYaw)
{
    if (duty > MOTOR_MAX_DUTY) duty = MOTOR_MAX_DUTY;
    motorBaseDuty = duty;
    motorStatus.targetYaw = Motor_NormalizeAngle(targetYaw);
    motorLastYaw = euler.angle.yaw;
    motorStatus.yawError = 0.0f;
    Motor_SetForward(duty);
}

void Motor_HoldYawUpdate(float yaw)
{
    float correction;
    if (!motorStatus.running) return;

    /* Ignore one implausible IMU frame without changing the locked heading. */
    if (Motor_NormalizeAngle(yaw - motorLastYaw) > MOTOR_YAW_JUMP_LIMIT ||
        Motor_NormalizeAngle(yaw - motorLastYaw) < -MOTOR_YAW_JUMP_LIMIT) {
        return;
    }
    motorLastYaw = yaw;
    motorStatus.yawError = Motor_NormalizeAngle(motorStatus.targetYaw - yaw);
    correction = motorStatus.yawError * MOTOR_YAW_P_GAIN;
    if ((correction < MOTOR_YAW_DEADBAND) && (correction > -MOTOR_YAW_DEADBAND)) {
        correction = 0.0f;
    }
    if (correction > MOTOR_MAX_DIFFERENTIAL) correction = MOTOR_MAX_DIFFERENTIAL;
    else if (correction < -MOTOR_MAX_DIFFERENTIAL) correction = -MOTOR_MAX_DIFFERENTIAL;
    /* Vehicle yaw convention is opposite the motor-side duty convention. */
    Motor_SetDuty(Motor_ClampDuty((float) motorBaseDuty - correction),
        Motor_ClampDuty((float) motorBaseDuty + correction));
}

Motor_Status Motor_GetStatus(void)
{
    return motorStatus;
}
