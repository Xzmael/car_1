#include "vision_uart.h"

#include "ti_msp_dl_config.h"

#define VISION_UART_LINE_SIZE (64U)

static uint8_t lineLength;
static bool discardingLine;
static bool frameReady;
static uint32_t frameCount;
static uint32_t byteCount;

void VisionUart_Init(void)
{
    lineLength = 0U;
    discardingLine = false;
    frameReady = false;
    frameCount = 0U;
    byteCount = 0U;
    while (!DL_UART_Main_isRXFIFOEmpty(VISION_UART_INST)) {
        (void) DL_UART_Main_receiveData(VISION_UART_INST);
    }
}

void VisionUart_Poll(void)
{
    while (!DL_UART_Main_isRXFIFOEmpty(VISION_UART_INST)) {
        const uint8_t byte = DL_UART_Main_receiveData(VISION_UART_INST);
        byteCount++;

        if (byte == '\n') {
            if (!discardingLine && lineLength != 0U) {
                frameReady = true;
                frameCount++;
            }
            lineLength = 0U;
            discardingLine = false;
        } else if (byte != '\r' && !discardingLine) {
            if (lineLength < (VISION_UART_LINE_SIZE - 1U)) {
                lineLength++;
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

uint32_t VisionUart_GetFrameCount(void)
{
    return frameCount;
}

uint32_t VisionUart_GetByteCount(void)
{
    return byteCount;
}
