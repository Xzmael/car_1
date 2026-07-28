#include "key.h"

#include "ti_msp_dl_config.h"

static uint8_t keyState;
static uint8_t keyPressed;
static uint8_t lastSample;

static uint8_t Key_ReadRaw(void)
{
    const uint32_t pins = DL_GPIO_readPins(KEY_PORT, KEY_SW1_PIN | KEY_SW2_PIN);
    uint8_t result = 0U;

    if ((pins & KEY_SW1_PIN) == 0U) result |= KEY_SW1;
    if ((pins & KEY_SW2_PIN) == 0U) result |= KEY_SW2;
    return result;
}

void Key_Init(void)
{
    keyState = 0U;
    keyPressed = 0U;
    lastSample = Key_ReadRaw();
}

void Key_Scan(void)
{
    const uint8_t sample = Key_ReadRaw();

    if (sample == lastSample) {
        keyPressed |= (uint8_t) (sample & (uint8_t) ~keyState);
        keyState = sample;
    }
    lastSample = sample;
}

uint8_t Key_GetPressed(void)
{
    const uint8_t pressed = keyPressed;
    keyPressed = 0U;
    return pressed;
}
