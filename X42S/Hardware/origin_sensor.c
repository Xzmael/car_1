#include "origin_sensor.h"

#include "ti_msp_dl_config.h"

void OriginSensor_Init(void)
{
}

bool OriginSensor_IsActive(void)
{
    return (DL_GPIO_readPins(ORIGIN_PORT, ORIGIN_SENSOR_PIN) == 0U);
}
