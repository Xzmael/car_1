#include "imu660rb.h"
#include "lsm6dsr_reg.h"

#include "ti_msp_dl_config.h"

#define BOOT_TIME         (10)
#define I2C_ADDRESS        (0x6AU)
#define I2C_TIMEOUT        (100000U)
#define CAL_TIMEOUT        (5000000U)
#define I2C_START_DELAY    (1000U)

#define ODR_COEFF_12Hz5   (512)
#define ODR_COEFF_26Hz    (256)
#define ODR_COEFF_52Hz    (128)
#define ODR_COEFF_104Hz   (64)
#define ODR_COEFF_208Hz   (32)
#define ODR_COEFF_416Hz   (16)
#define ODR_COEFF_833Hz   (8)
#define ODR_COEFF_1667Hz  (4)
#define ODR_COEFF_3333Hz  (2)
#define ODR_COEFF_6667Hz  (1)

static stmdev_ctx_t dev_ctx;

float acceleration_mg[3];
float angular_rate_mdps[3];

static int16_t data_raw_acceleration[3];
static int16_t data_raw_angular_rate[3];
static uint8_t whoamI, rst;
static uint8_t imuStage;
static float samplePeriod, sampleRate;
static uint32_t lastTick;
static IMU660RB_Status imuStatus = IMU660RB_STATUS_I2C_ERROR;
static volatile bool dataReady;

static FusionMatrix gyroscopeMisalignment = {{{1.0f, 0.0f, 0.0f},
                                                {0.0f, 1.0f, 0.0f},
                                                {0.0f, 0.0f, 1.0f}}};
static FusionVector gyroscopeSensitivity = {{1.0f, 1.0f, 1.0f}};
FusionAhrs ahrs;
FusionEuler euler;
static FusionOffset offset;

static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len);
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);
static void platform_delay(uint32_t ms);
static bool i2c_wait_idle(void);
static bool i2c_wait_complete(void);

IMU660RB_Status IMU660RB_Init(void)
{
    int8_t freq_fine;
    uint32_t calibrationTimeout;

    /* Initialize mems driver interface */
    dev_ctx.write_reg = platform_write;
    dev_ctx.read_reg = platform_read;
    dev_ctx.mdelay = platform_delay;
    imuStatus = IMU660RB_STATUS_I2C_ERROR;
    dataReady = false;

    /* Wait sensor boot time */
    platform_delay(BOOT_TIME);
    /* Check device ID */
    imuStage = 1U;
    if (lsm6dsr_device_id_get(&dev_ctx, &whoamI) != 0) {
        return imuStatus;
    }

    if (whoamI != LSM6DSR_ID) {
        imuStatus = IMU660RB_STATUS_DEVICE_ID_ERROR;
        return imuStatus;
    }

    /* Restore default configuration */
    imuStage = 2U;
    if (lsm6dsr_reset_set(&dev_ctx, PROPERTY_ENABLE) != 0) return imuStatus;

    imuStage = 3U;
    calibrationTimeout = 100U;
    do {
        if (lsm6dsr_reset_get(&dev_ctx, &rst) != 0) return imuStatus;
        platform_delay(1U);
    } while (rst != 0U && --calibrationTimeout != 0U);
    if (rst != 0U) { imuStatus = IMU660RB_STATUS_TIMEOUT; return imuStatus; }

    /* Enable Block Data Update */
    imuStage = 4U;
    if (lsm6dsr_block_data_update_set(&dev_ctx, PROPERTY_ENABLE) != 0) return imuStatus;

    /* Set Output Data Rate */
    imuStage = 5U;
    if ((lsm6dsr_xl_data_rate_set(&dev_ctx, LSM6DSR_XL_ODR_52Hz) != 0) ||
        (lsm6dsr_gy_data_rate_set(&dev_ctx, LSM6DSR_GY_ODR_52Hz) != 0)) return imuStatus;

    /* Set full scale */
    imuStage = 6U;
    if ((lsm6dsr_xl_full_scale_set(&dev_ctx, LSM6DSR_2g) != 0) ||
        (lsm6dsr_gy_full_scale_set(&dev_ctx, LSM6DSR_2000dps) != 0) ||
        (lsm6dsr_gy_filter_lp1_set(&dev_ctx, 1) != 0)) return imuStatus;

    {
        lsm6dsr_pin_int1_route_t int1Route;
        imuStage = 7U;
        if (lsm6dsr_pin_int1_route_get(&dev_ctx, &int1Route) != 0) return imuStatus;
        int1Route.int1_ctrl.int1_drdy_g = PROPERTY_ENABLE;
        if (lsm6dsr_pin_int1_route_set(&dev_ctx, &int1Route) != 0) return imuStatus;
        if (lsm6dsr_data_ready_mode_set(&dev_ctx, LSM6DSR_DRDY_PULSED) != 0) return imuStatus;
    }

    imuStage = 8U;
    if (lsm6dsr_odr_cal_reg_get(&dev_ctx, &freq_fine) != 0) return imuStatus;
    sampleRate = (6667 + ((0.0015 * freq_fine) * 6667)) / ODR_COEFF_52Hz;
    samplePeriod = 1.0 / sampleRate;

    FusionAhrsInitialise(&ahrs);
    FusionOffsetInitialise(&offset, sampleRate);

    /* Timestamp each fusion update.  OLED writes can delay the main loop, so
     * the configured ODR alone is not a valid integration interval. */
    SysTick->LOAD = 0x00FFFFFFU;
    SysTick->VAL = 0U;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
    lastTick = SysTick->VAL;

    imuStatus = IMU660RB_STATUS_OK;
    imuStage = 0U;
    return imuStatus;
}

IMU660RB_Status IMU660RB_GetStatus(void)
{
    return imuStatus;
}

uint8_t IMU660RB_GetDeviceId(void)
{
    return whoamI;
}

uint8_t IMU660RB_GetStage(void)
{
    return imuStage;
}

IMU660RB_Status Read_IMU660RB(void)
{
    uint32_t currentTick;
    uint32_t elapsedTicks;
    if (imuStatus != IMU660RB_STATUS_OK) return imuStatus;
    if (lsm6dsr_acceleration_raw_get(&dev_ctx, data_raw_acceleration) != 0) return imuStatus;
    acceleration_mg[0] = lsm6dsr_from_fs2g_to_mg(data_raw_acceleration[0]);
    acceleration_mg[1] = lsm6dsr_from_fs2g_to_mg(data_raw_acceleration[1]);
    acceleration_mg[2] = lsm6dsr_from_fs2g_to_mg(data_raw_acceleration[2]);

    if (lsm6dsr_angular_rate_raw_get(&dev_ctx, data_raw_angular_rate) != 0) return imuStatus;
    angular_rate_mdps[0] = lsm6dsr_from_fs2000dps_to_mdps(data_raw_angular_rate[0]);
    angular_rate_mdps[1] = lsm6dsr_from_fs2000dps_to_mdps(data_raw_angular_rate[1]);
    angular_rate_mdps[2] = lsm6dsr_from_fs2000dps_to_mdps(data_raw_angular_rate[2]);

    /* Sensor is face-down: car forward/left/up maps to -Y/-X/-Z. */
    FusionVector accelerometer = {{-acceleration_mg[1] / 1000.0f,
                                    -acceleration_mg[0] / 1000.0f,
                                    -acceleration_mg[2] / 1000.0f}};
    FusionVector gyroscope = {{-angular_rate_mdps[1] / 1000.0f,
                                -angular_rate_mdps[0] / 1000.0f,
                                -angular_rate_mdps[2] / 1000.0f}};

    currentTick = SysTick->VAL;
    elapsedTicks = (lastTick - currentTick) & 0x00FFFFFFU;
    lastTick = currentTick;
    samplePeriod = (float) elapsedTicks / (float) CPUCLK_FREQ;
    if ((samplePeriod < 0.001f) || (samplePeriod > 0.2f)) {
        samplePeriod = 1.0f / sampleRate;
    }

    gyroscope = FusionCalibrationInertial(gyroscope, gyroscopeMisalignment,
        gyroscopeSensitivity, FUSION_VECTOR_ZERO);
    gyroscope = FusionOffsetUpdate(&offset, gyroscope);
    FusionAhrsUpdateNoMagnetometer(&ahrs, gyroscope, accelerometer, samplePeriod);
    euler = FusionQuaternionToEuler(FusionAhrsGetQuaternion(&ahrs));
    return imuStatus;
}

void IMU660RB_DataReadyNotify(void)
{
    dataReady = true;
}

bool IMU660RB_HasNewData(void)
{
    if (!dataReady) return false;
    dataReady = false;
    return true;
}

static bool i2c_wait_idle(void)
{
    uint32_t timeout = I2C_TIMEOUT;
    while ((DL_I2C_getControllerStatus(IMU_I2C_INST) & DL_I2C_CONTROLLER_STATUS_IDLE) == 0U) {
        if (--timeout == 0U) { imuStatus = IMU660RB_STATUS_TIMEOUT; return false; }
    }
    return true;
}

/*
 * @brief  Write generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to write
 * @param  bufp      pointer to data to write in register reg
 * @param  len       number of consecutive register to write
 *
 */
static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp,
                              uint16_t len)
{
    uint8_t packet[8];
    uint16_t i;
    if ((len > (sizeof(packet) - 1U)) || !i2c_wait_idle()) return -1;
    packet[0] = reg;
    for (i = 0U; i < len; i++) packet[i + 1U] = bufp[i];
    DL_I2C_fillControllerTXFIFO(IMU_I2C_INST, packet, (uint16_t)(len + 1U));
    DL_I2C_startControllerTransfer(IMU_I2C_INST, I2C_ADDRESS, DL_I2C_CONTROLLER_DIRECTION_TX, (uint16_t)(len + 1U));
    /* I2C_ERR_13: wait at least three I2C functional clocks after START. */
    delay_cycles(I2C_START_DELAY);
    return i2c_wait_complete() ? 0 : -1;
}

/*
 * @brief  Read generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to read
 * @param  bufp      pointer to buffer that store the data read
 * @param  len       number of consecutive register to read
 *
 */
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len)
{
    uint32_t timeout;
    uint16_t i;
    if (!i2c_wait_idle()) return -1;
    DL_I2C_fillControllerTXFIFO(IMU_I2C_INST, &reg, 1U);
    /* LSM6DSR accepts a STOP between register selection and the read phase. */
    DL_I2C_startControllerTransfer(IMU_I2C_INST, I2C_ADDRESS,
        DL_I2C_CONTROLLER_DIRECTION_TX, 1U);
    /* I2C_ERR_13: wait at least three I2C functional clocks after START. */
    delay_cycles(I2C_START_DELAY);
    if (!i2c_wait_complete()) return -1;
    /* Wait for STOP to complete before the separate read transaction. */
    if (!i2c_wait_idle()) return -1;
    delay_cycles(I2C_START_DELAY);
    DL_I2C_startControllerTransfer(IMU_I2C_INST, I2C_ADDRESS,
        DL_I2C_CONTROLLER_DIRECTION_RX, len);
    /* I2C_ERR_13: wait at least three I2C functional clocks after START. */
    delay_cycles(I2C_START_DELAY);
    for (i = 0U; i < len; i++) {
        timeout = I2C_TIMEOUT;
        while (DL_I2C_isControllerRXFIFOEmpty(IMU_I2C_INST)) {
            if ((DL_I2C_getControllerStatus(IMU_I2C_INST) & DL_I2C_CONTROLLER_STATUS_ERROR) != 0U) {
                imuStatus = IMU660RB_STATUS_I2C_ERROR;
                return -1;
            }
            if (--timeout == 0U) { imuStatus = IMU660RB_STATUS_TIMEOUT; return -1; }
            /* Give a 100 kHz I2C transfer time to place the byte in RX FIFO. */
            delay_cycles(32U);
        }
        bufp[i] = DL_I2C_receiveControllerData(IMU_I2C_INST);
    }
    return i2c_wait_complete() ? 0 : -1;
}

static bool i2c_wait_complete(void)
{
    uint32_t timeout = I2C_TIMEOUT;
    while ((DL_I2C_getControllerStatus(IMU_I2C_INST) & DL_I2C_CONTROLLER_STATUS_BUSY) != 0U) {
        if (--timeout == 0U) { imuStatus = IMU660RB_STATUS_TIMEOUT; return false; }
    }
    if ((DL_I2C_getControllerStatus(IMU_I2C_INST) & DL_I2C_CONTROLLER_STATUS_ERROR) != 0U) {
        imuStatus = IMU660RB_STATUS_I2C_ERROR;
        return false;
    }
    return true;
}

/*
 * @brief  platform specific delay (platform dependent)
 *
 * @param  ms        delay in ms
 *
 */
static void platform_delay(uint32_t ms)
{
    while (ms-- != 0U) delay_cycles(32000U);
}
