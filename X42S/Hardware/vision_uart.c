#include "vision_uart.h"

#include "ti_msp_dl_config.h"

#define VISION_LINE_SIZE (48U)

static char line[VISION_LINE_SIZE];
static uint8_t length;
static bool discard;
static bool ready;
static VisionUart_Frame latest;
static volatile uint32_t ageMs;
static uint32_t frameCount;

static bool ParseUnsigned(const char **text, uint16_t *value)
{
    uint32_t result = 0U;
    bool digit = false;
    while ((**text >= '0') && (**text <= '9')) {
        result = result * 10U + (uint32_t) (**text - '0');
        if (result > 65535U) return false;
        (*text)++;
        digit = true;
    }
    *value = (uint16_t) result;
    return digit;
}

static void ProcessLine(void)
{
    const char *text;
    uint16_t x, width, height;

    line[length] = '\0';
    if ((length == 4U) && (line[0] == 'N') && (line[1] == 'O') &&
        (line[2] == 'N') && (line[3] == 'E')) {
        latest.type = VISION_UART_NONE;
        latest.centerX = 0;
    } else if ((length > 5U) && (line[0] == 'B') && (line[1] == 'A') &&
               (line[2] == 'L') && (line[3] == 'L') && (line[4] == ',')) {
        text = &line[5];
        if (!ParseUnsigned(&text, &x) || (*text++ != ',') ||
            !ParseUnsigned(&text, &width) || (*text++ != ',') ||
            !ParseUnsigned(&text, &height) || (*text != '\0')) return;
        latest.type = VISION_UART_BALL;
        latest.centerX = (int16_t) (x + width / 2U);
    } else {
        return;
    }
    ready = true;
    frameCount++;
    ageMs = 0U;
}

void VisionUart_Init(void)
{
    length = 0U; discard = false; ready = false; ageMs = 0U; frameCount = 0U;
    latest.type = VISION_UART_NO_FRAME; latest.centerX = 0;
    while (!DL_UART_Main_isRXFIFOEmpty(VISION_UART_INST)) {
        (void) DL_UART_Main_receiveData(VISION_UART_INST);
    }
}

void VisionUart_Tick1ms(void)
{
    if (ageMs != UINT32_MAX) ageMs++;
}

void VisionUart_Poll(void)
{
    while (!DL_UART_Main_isRXFIFOEmpty(VISION_UART_INST)) {
        const uint8_t byte = DL_UART_Main_receiveData(VISION_UART_INST);
        if (byte == '\n') {
            if (!discard && (length != 0U)) ProcessLine();
            length = 0U; discard = false;
        } else if ((byte != '\r') && !discard) {
            if (length < (VISION_LINE_SIZE - 1U)) line[length++] = (char) byte;
            else { length = 0U; discard = true; }
        }
    }
}

bool VisionUart_GetFrame(VisionUart_Frame *frame)
{
    if (!ready || (frame == 0)) return false;
    *frame = latest;
    ready = false;
    return true;
}

uint32_t VisionUart_GetFrameAgeMs(void) { return ageMs; }
uint32_t VisionUart_GetFrameCount(void) { return frameCount; }
