#include "buzzer.h"

#include "ti_msp_dl_config.h"

void Buzzer_Init(void)
{
    Buzzer_Off();
}

void Buzzer_On(void)
{
    /* PB23 buzzer is active low. */
    DL_GPIO_clearPins(BUZZER_PORT, BUZZER_BUZZER_PIN_PIN);
}

void Buzzer_Off(void)
{
    DL_GPIO_setPins(BUZZER_PORT, BUZZER_BUZZER_PIN_PIN);
}
