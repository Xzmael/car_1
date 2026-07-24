#include "led.h"

#include "ti_msp_dl_config.h"

void LED_Init(void)
{
    LED_Off();
}

void LED_On(void)
{
    /* PA23 LED is active low. */
    DL_GPIO_clearPins(LED_PORT, LED_LED_PIN_PIN);
}

void LED_Off(void)
{
    DL_GPIO_setPins(LED_PORT, LED_LED_PIN_PIN);
}

void LED_Toggle(void)
{
    DL_GPIO_togglePins(LED_PORT, LED_LED_PIN_PIN);
}
