#include "key.h"

#include "ti_msp_dl_config.h"

static uint8_t keyState;
static uint8_t keyPressed;
static uint8_t lastSample;

static uint8_t Key_ReadRaw(void)
{
    const uint32_t pins = DL_GPIO_readPins(GPIOB, KEY_SW1_PIN | KEY_SW2_PIN |
        KEY_SW3_PIN | KEY_SW4_PIN);
    uint8_t result = 0U;

    /* Keys use internal pull-ups, so a low input means pressed. */
    if ((pins & KEY_SW1_PIN) == 0U) result |= KEY_SW1;
    if ((pins & KEY_SW2_PIN) == 0U) result |= KEY_SW2;
    if ((pins & KEY_SW3_PIN) == 0U) result |= KEY_SW3;
    if ((pins & KEY_SW4_PIN) == 0U) result |= KEY_SW4;
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

    /* Accept a state only after two consecutive identical samples. */
    if (sample == lastSample) {
        keyPressed |= (uint8_t) (sample & (uint8_t) ~keyState);
        keyState = sample;
    }
    lastSample = sample;
}

uint8_t Key_GetState(void)
{
    return keyState;
}

uint8_t Key_GetPressed(void)
{
    const uint8_t pressed = keyPressed;
    keyPressed = 0U;
    return pressed;
}
