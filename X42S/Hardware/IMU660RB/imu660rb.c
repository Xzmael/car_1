#include "imu660rb.h"

#include <math.h>

#include "ti_msp_dl_config.h"

#define IMU_ADDRESS             (0x6AU)
#define IMU_EXPECTED_ID         (0x6BU)
#define REG_WHO_AM_I            (0x0FU)
#define REG_CTRL1_XL            (0x10U)
#define REG_CTRL2_G             (0x11U)
#define REG_CTRL3_C             (0x12U)
#define REG_INT1_CTRL           (0x0DU)
#define REG_STATUS              (0x1EU)
#define REG_OUTX_L_G            (0x22U)
#define STATUS_XLDA             (1U << 0)
#define STATUS_GDA              (1U << 1)
#define I2C_DELAY_CYCLES        (48U)
#define SAMPLE_PERIOD_MS        (10U)
#define CALIBRATION_SAMPLES     (100U)
#define COMPLEMENTARY_TAU_S     (0.40f)
#define RAD_TO_DEG              (57.2957795f)

static volatile uint32_t tickMs;
static uint32_t lastUpdateMs;
static IMU660RB_Status status = IMU660RB_STATUS_I2C_ERROR;
static IMU660RB_Data data;
static float gyroOffset[3];
static float rollZero;
static float pitchZero;
static IMU660RB_RodAxis rodAxis = IMU660RB_ROD_AXIS_PITCH;
static bool rodInverted;
static bool newData;
static uint8_t deviceId;

static void BusDelay(void)
{
    delay_cycles(I2C_DELAY_CYCLES);
}

static void SclHigh(void)
{
    DL_GPIO_setPins(IMU_BUS_PORT, IMU_BUS_SCL_PIN);
    DL_GPIO_disableOutput(IMU_BUS_PORT, IMU_BUS_SCL_PIN);
    BusDelay();
}

static void SclLow(void)
{
    DL_GPIO_clearPins(IMU_BUS_PORT, IMU_BUS_SCL_PIN);
    DL_GPIO_enableOutput(IMU_BUS_PORT, IMU_BUS_SCL_PIN);
    BusDelay();
}

static void SdaHigh(void)
{
    DL_GPIO_setPins(IMU_BUS_PORT, IMU_BUS_SDA_PIN);
    DL_GPIO_disableOutput(IMU_BUS_PORT, IMU_BUS_SDA_PIN);
    BusDelay();
}

static void SdaLow(void)
{
    DL_GPIO_clearPins(IMU_BUS_PORT, IMU_BUS_SDA_PIN);
    DL_GPIO_enableOutput(IMU_BUS_PORT, IMU_BUS_SDA_PIN);
    BusDelay();
}

static bool SdaRead(void)
{
    return DL_GPIO_readPins(IMU_BUS_PORT, IMU_BUS_SDA_PIN) != 0U;
}

static void Start(void)
{
    SdaHigh();
    SclHigh();
    SdaLow();
    SclLow();
}

static void Stop(void)
{
    SdaLow();
    SclHigh();
    SdaHigh();
}

static bool WriteByte(uint8_t value)
{
    uint8_t bit;
    for (bit = 0U; bit < 8U; bit++) {
        if ((value & 0x80U) != 0U) SdaHigh(); else SdaLow();
        SclHigh();
        SclLow();
        value <<= 1U;
    }
    SdaHigh();
    SclHigh();
    bit = SdaRead() ? 0U : 1U;
    SclLow();
    return bit != 0U;
}

static uint8_t ReadByte(bool acknowledge)
{
    uint8_t bit;
    uint8_t value = 0U;
    SdaHigh();
    for (bit = 0U; bit < 8U; bit++) {
        value <<= 1U;
        SclHigh();
        if (SdaRead()) value |= 1U;
        SclLow();
    }
    if (acknowledge) SdaLow(); else SdaHigh();
    SclHigh();
    SclLow();
    SdaHigh();
    return value;
}

static bool WriteRegister(uint8_t reg, uint8_t value)
{
    Start();
    if (!WriteByte((uint8_t) (IMU_ADDRESS << 1U)) || !WriteByte(reg) ||
        !WriteByte(value)) {
        Stop();
        return false;
    }
    Stop();
    return true;
}

static bool ReadRegisters(uint8_t reg, uint8_t *buffer, uint8_t length)
{
    uint8_t index;
    Start();
    if (!WriteByte((uint8_t) (IMU_ADDRESS << 1U)) || !WriteByte(reg)) {
        Stop();
        return false;
    }
    Start();
    if (!WriteByte((uint8_t) ((IMU_ADDRESS << 1U) | 1U))) {
        Stop();
        return false;
    }
    for (index = 0U; index < length; index++) {
        buffer[index] = ReadByte(index != (uint8_t) (length - 1U));
    }
    Stop();
    return true;
}

static int16_t MakeInt16(const uint8_t *bytes)
{
    return (int16_t) ((uint16_t) bytes[0] | ((uint16_t) bytes[1] << 8U));
}

static bool ReadRaw(float acceleration[3], float angularRate[3])
{
    uint8_t raw[12];
    uint8_t axis;
    if (!ReadRegisters(REG_OUTX_L_G, raw, sizeof(raw))) return false;
    for (axis = 0U; axis < 3U; axis++) {
        angularRate[axis] = (float) MakeInt16(&raw[axis * 2U]) * 0.070f;
        acceleration[axis] = (float) MakeInt16(&raw[6U + axis * 2U]) * 0.000061f;
    }
    return true;
}

static void DelayMs(uint32_t duration)
{
    while (duration-- != 0U) delay_cycles(32000U);
}

static void UpdateRodOutput(void)
{
    if (rodAxis == IMU660RB_ROD_AXIS_ROLL) {
        data.rodAngleDeg = data.rollDeg - rollZero;
        data.rodRateDps = data.angularRateDps[0];
    } else {
        data.rodAngleDeg = data.pitchDeg - pitchZero;
        data.rodRateDps = data.angularRateDps[1];
    }
    if (rodInverted) {
        data.rodAngleDeg = -data.rodAngleDeg;
        data.rodRateDps = -data.rodRateDps;
    }
}

IMU660RB_Status IMU660RB_Init(void)
{
    float acceleration[3];
    float angularRate[3];
    float rollSum = 0.0f;
    float pitchSum = 0.0f;
    uint16_t sample;
    uint8_t axis;

    DL_GPIO_clearPins(IMU_CTRL_SA0_PORT, IMU_CTRL_SA0_PIN);
    DL_GPIO_setPins(IMU_CTRL_CS_PORT, IMU_CTRL_CS_PIN);
    SclHigh();
    SdaHigh();
    DelayMs(20U);
    if (!ReadRegisters(REG_WHO_AM_I, &deviceId, 1U)) {
        status = IMU660RB_STATUS_I2C_ERROR;
        return status;
    }
    if (deviceId != IMU_EXPECTED_ID) {
        status = IMU660RB_STATUS_DEVICE_ID_ERROR;
        return status;
    }
    /* 104 Hz, 2 g accelerometer; 104 Hz, 2000 dps gyroscope; BDU + auto increment. */
    if (!WriteRegister(REG_CTRL3_C, 0x44U) || !WriteRegister(REG_CTRL1_XL, 0x40U) ||
        !WriteRegister(REG_CTRL2_G, 0x4CU) || !WriteRegister(REG_INT1_CTRL, 0x03U)) {
        status = IMU660RB_STATUS_I2C_ERROR;
        return status;
    }
    DelayMs(100U);
    for (axis = 0U; axis < 3U; axis++) gyroOffset[axis] = 0.0f;
    for (sample = 0U; sample < CALIBRATION_SAMPLES; sample++) {
        if (!ReadRaw(acceleration, angularRate)) {
            status = IMU660RB_STATUS_I2C_ERROR;
            return status;
        }
        for (axis = 0U; axis < 3U; axis++) gyroOffset[axis] += angularRate[axis];
        rollSum += atan2f(acceleration[1], acceleration[2]) * RAD_TO_DEG;
        pitchSum += atan2f(-acceleration[0],
            sqrtf(acceleration[1] * acceleration[1] + acceleration[2] * acceleration[2])) * RAD_TO_DEG;
        DelayMs(SAMPLE_PERIOD_MS);
    }
    for (axis = 0U; axis < 3U; axis++) gyroOffset[axis] /= (float) CALIBRATION_SAMPLES;
    data.rollDeg = rollSum / (float) CALIBRATION_SAMPLES;
    data.pitchDeg = pitchSum / (float) CALIBRATION_SAMPLES;
    rollZero = data.rollDeg;
    pitchZero = data.pitchDeg;
    lastUpdateMs = tickMs;
    newData = false;
    status = IMU660RB_STATUS_OK;
    UpdateRodOutput();
    return status;
}

IMU660RB_Status IMU660RB_Update(void)
{
    uint8_t sensorStatus;
    uint8_t axis;
    float acceleration[3];
    float angularRate[3];
    float accelRoll;
    float accelPitch;
    float dt;
    float alpha;
    uint32_t now = tickMs;

    if (status != IMU660RB_STATUS_OK) return status;
    if ((uint32_t) (now - lastUpdateMs) < SAMPLE_PERIOD_MS) return status;
    dt = (float) (uint32_t) (now - lastUpdateMs) * 0.001f;
    lastUpdateMs = now;
    if (!ReadRegisters(REG_STATUS, &sensorStatus, 1U) ||
        ((sensorStatus & (STATUS_XLDA | STATUS_GDA)) != (STATUS_XLDA | STATUS_GDA))) return status;
    if (!ReadRaw(acceleration, angularRate)) {
        status = IMU660RB_STATUS_I2C_ERROR;
        return status;
    }
    for (axis = 0U; axis < 3U; axis++) {
        data.accelerationG[axis] = acceleration[axis];
        data.angularRateDps[axis] = angularRate[axis] - gyroOffset[axis];
    }
    accelRoll = atan2f(acceleration[1], acceleration[2]) * RAD_TO_DEG;
    accelPitch = atan2f(-acceleration[0],
        sqrtf(acceleration[1] * acceleration[1] + acceleration[2] * acceleration[2])) * RAD_TO_DEG;
    alpha = COMPLEMENTARY_TAU_S / (COMPLEMENTARY_TAU_S + dt);
    data.rollDeg = alpha * (data.rollDeg + data.angularRateDps[0] * dt) +
                   (1.0f - alpha) * accelRoll;
    data.pitchDeg = alpha * (data.pitchDeg + data.angularRateDps[1] * dt) +
                    (1.0f - alpha) * accelPitch;
    UpdateRodOutput();
    newData = true;
    return status;
}

void IMU660RB_Tick1ms(void)
{
    if (tickMs != UINT32_MAX) tickMs++;
}

void IMU660RB_SetRodAxis(IMU660RB_RodAxis axis, bool inverted)
{
    rodAxis = axis;
    rodInverted = inverted;
    UpdateRodOutput();
}

void IMU660RB_SetRodZero(void)
{
    rollZero = data.rollDeg;
    pitchZero = data.pitchDeg;
    UpdateRodOutput();
}

const IMU660RB_Data *IMU660RB_GetData(void) { return &data; }
IMU660RB_Status IMU660RB_GetStatus(void) { return status; }
uint8_t IMU660RB_GetDeviceId(void) { return deviceId; }

bool IMU660RB_HasNewData(void)
{
    const bool result = newData;
    newData = false;
    return result;
}
