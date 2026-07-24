#include "gray.h"

#include "ti_msp_dl_config.h"

static Gray_Result grayResult;
static int16_t lastPosition;

void Gray_Init(void)
{
    grayResult.raw = 0U;
    grayResult.blackCount = 0U;
    grayResult.position = 0;
    grayResult.status = GRAY_STATUS_LOST;
    lastPosition = 0;
}

void Gray_Read(void)
{
    static const int8_t weights[GRAY_SENSOR_COUNT] = {
        -11, -9, -7, -5, -3, -1, 1, 3, 5, 7, 9, 11
    };
    const uint32_t portAPins = DL_GPIO_readPins(GPIOA,
        GRAY_S1_PIN | GRAY_S2_PIN | GRAY_S3_PIN | GRAY_S4_PIN |
        GRAY_S7_PIN | GRAY_S8_PIN | GRAY_S10_PIN | GRAY_S12_PIN);
    const uint32_t portBPins = DL_GPIO_readPins(GPIOB,
        GRAY_S5_PIN | GRAY_S6_PIN | GRAY_S9_PIN | GRAY_S11_PIN);
    const uint32_t inputs[GRAY_SENSOR_COUNT] = {
        portAPins & GRAY_S1_PIN, portAPins & GRAY_S2_PIN,
        portAPins & GRAY_S3_PIN, portAPins & GRAY_S4_PIN,
        portBPins & GRAY_S5_PIN, portBPins & GRAY_S6_PIN,
        portAPins & GRAY_S7_PIN, portAPins & GRAY_S8_PIN,
        portBPins & GRAY_S9_PIN, portAPins & GRAY_S10_PIN,
        portBPins & GRAY_S11_PIN, portAPins & GRAY_S12_PIN
    };
    uint8_t index;
    int16_t sum = 0;

    grayResult.raw = 0U;
    grayResult.blackCount = 0U;
    for (index = 0U; index < GRAY_SENSOR_COUNT; index++) {
        if (inputs[index] != 0U) {
            grayResult.raw |= (uint16_t) (1U << index);
            grayResult.blackCount++;
            sum += weights[index];
        }
    }

    if (grayResult.blackCount == 0U) {
        grayResult.status = GRAY_STATUS_LOST;
        grayResult.position = lastPosition;
    } else if (grayResult.blackCount == GRAY_SENSOR_COUNT) {
        grayResult.status = GRAY_STATUS_ALL_BLACK;
        grayResult.position = 0;
        lastPosition = 0;
    } else {
        grayResult.status = GRAY_STATUS_NORMAL;
        grayResult.position = (int16_t) (sum / (int16_t) grayResult.blackCount);
        lastPosition = grayResult.position;
    }
}

uint16_t Gray_GetRaw(void)
{
    return grayResult.raw;
}

Gray_Result Gray_GetResult(void)
{
    return grayResult;
}
