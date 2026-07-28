#include "vision_uart.h"

#include "ti_msp_dl_config.h"

#define VISION_UART_LINE_SIZE (64U)

static uint8_t lineLength;
static char lineBuffer[VISION_UART_LINE_SIZE];
static bool discardingLine;
static bool frameReady;
static uint32_t frameCount;
static uint32_t byteCount;
static volatile uint32_t frameAgeMs;
static VisionUart_Frame latestFrame;

static bool VisionUart_ParseUInt(const char **text, uint16_t *value)
{
    uint32_t result = 0U;
    bool hasDigit = false;

    while ((**text >= '0') && (**text <= '9')) {
        result = result * 10U + (uint32_t) (**text - '0');
        if (result > 65535U) return false;
        (*text)++;
        hasDigit = true;
    }
    *value = (uint16_t) result;
    return hasDigit;
}

static bool VisionUart_ParseBall(VisionUart_Frame *frame)
{
    const char *text = &lineBuffer[5];
    uint16_t x;

    if (!VisionUart_ParseUInt(&text, &x) || (*text++ != ',') ||
        !VisionUart_ParseUInt(&text, &frame->width) || (*text++ != ',') ||
        !VisionUart_ParseUInt(&text, &frame->height) || (*text != '\0')) return false;
    frame->type = VISION_FRAME_BALL;
    frame->x = (int16_t) x;
    return true;
}

static void VisionUart_ProcessLine(void)
{
    VisionUart_Frame frame;

    lineBuffer[lineLength] = '\0';
    frame.type = VISION_FRAME_NONE;
    frame.x = 0;
    frame.width = 0U;
    frame.height = 0U;
    if ((lineLength == 4U) && (lineBuffer[0] == 'N') && (lineBuffer[1] == 'O') &&
        (lineBuffer[2] == 'N') && (lineBuffer[3] == 'E')) {
        frame.type = VISION_FRAME_NO_BALL;
    } else if ((lineLength > 5U) && (lineBuffer[0] == 'B') && (lineBuffer[1] == 'A') &&
               (lineBuffer[2] == 'L') && (lineBuffer[3] == 'L') && (lineBuffer[4] == ',') &&
               !VisionUart_ParseBall(&frame)) {
        return;
    }
    if (frame.type != VISION_FRAME_NONE) {
        latestFrame = frame;
        frameReady = true;
        frameCount++;
        frameAgeMs = 0U;
    }
}

void VisionUart_Init(void)
{
    lineLength = 0U;
    discardingLine = false;
    frameReady = false;
    frameCount = 0U;
    byteCount = 0U;
    frameAgeMs = 0U;
    latestFrame.type = VISION_FRAME_NONE;
    while (!DL_UART_Main_isRXFIFOEmpty(VISION_UART_INST)) {
        (void) DL_UART_Main_receiveData(VISION_UART_INST);
    }
}

void VisionUart_Tick1ms(void)
{
    if (frameAgeMs != UINT32_MAX) frameAgeMs++;
}

void VisionUart_Poll(void)
{
    while (!DL_UART_Main_isRXFIFOEmpty(VISION_UART_INST)) {
        const uint8_t byte = DL_UART_Main_receiveData(VISION_UART_INST);
        byteCount++;

        if (byte == '\n') {
            if (!discardingLine && lineLength != 0U) VisionUart_ProcessLine();
            lineLength = 0U;
            discardingLine = false;
        } else if (byte != '\r' && !discardingLine) {
            if (lineLength < (VISION_UART_LINE_SIZE - 1U)) {
                lineBuffer[lineLength++] = (char) byte;
            } else {
                /* A too-long line is discarded until its terminating newline. */
                lineLength = 0U;
                discardingLine = true;
            }
        }
    }
}

bool VisionUart_HasFrame(void)
{
    return frameReady;
}

bool VisionUart_GetFrame(VisionUart_Frame *frame)
{
    if (!frameReady || (frame == 0)) return false;
    *frame = latestFrame;
    frameReady = false;
    return true;
}

uint32_t VisionUart_GetFrameCount(void)
{
    return frameCount;
}

uint32_t VisionUart_GetByteCount(void)
{
    return byteCount;
}

uint32_t VisionUart_GetFrameAgeMs(void)
{
    return frameAgeMs;
}
