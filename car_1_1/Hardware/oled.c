#include "oled.h"

#include <string.h>

#include "ti_msp_dl_config.h"

#define OLED_ADDRESS       (0x3CU)
#define OLED_BUFFER_SIZE   ((OLED_WIDTH * OLED_HEIGHT) / 8U)
/* Use a conservative software-I2C rate to tolerate motor and sensor noise. */
#define OLED_I2C_DELAY     (256U)
#define OLED_PAGE_RETRIES  (3U)

static uint8_t oledBuffer[OLED_BUFFER_SIZE];
static uint8_t cursorX;
static uint8_t cursorY;

static void OLED_I2C_Delay(void)
{
    delay_cycles(OLED_I2C_DELAY);
}

/* Releasing a line lets the OLED module pull it high through its I2C pull-up. */
static void OLED_SCL_High(void)
{
    DL_GPIO_setPins(OLED_SCL_PORT, OLED_SCL_SCL_PIN_PIN);
    DL_GPIO_disableOutput(OLED_SCL_PORT, OLED_SCL_SCL_PIN_PIN);
}

static void OLED_SCL_Low(void)
{
    DL_GPIO_clearPins(OLED_SCL_PORT, OLED_SCL_SCL_PIN_PIN);
    DL_GPIO_enableOutput(OLED_SCL_PORT, OLED_SCL_SCL_PIN_PIN);
}

static void OLED_SDA_High(void)
{
    DL_GPIO_setPins(OLED_SDA_PORT, OLED_SDA_SDA_PIN_PIN);
    DL_GPIO_disableOutput(OLED_SDA_PORT, OLED_SDA_SDA_PIN_PIN);
}

static void OLED_SDA_Low(void)
{
    DL_GPIO_clearPins(OLED_SDA_PORT, OLED_SDA_SDA_PIN_PIN);
    DL_GPIO_enableOutput(OLED_SDA_PORT, OLED_SDA_SDA_PIN_PIN);
}

static bool OLED_SDA_Read(void)
{
    return (DL_GPIO_readPins(OLED_SDA_PORT, OLED_SDA_SDA_PIN_PIN) != 0U);
}

static void OLED_I2C_Start(void)
{
    OLED_SDA_High();
    OLED_SCL_High();
    OLED_I2C_Delay();
    OLED_SDA_Low();
    OLED_I2C_Delay();
    OLED_SCL_Low();
}

static void OLED_I2C_Stop(void)
{
    OLED_SDA_Low();
    OLED_I2C_Delay();
    OLED_SCL_High();
    OLED_I2C_Delay();
    OLED_SDA_High();
    OLED_I2C_Delay();
}

static bool OLED_I2C_WriteByte(uint8_t value)
{
    uint8_t bit;

    for (bit = 0U; bit < 8U; bit++) {
        if ((value & 0x80U) != 0U) {
            OLED_SDA_High();
        } else {
            OLED_SDA_Low();
        }
        OLED_I2C_Delay();
        OLED_SCL_High();
        OLED_I2C_Delay();
        OLED_SCL_Low();
        value <<= 1U;
    }

    OLED_SDA_High();
    OLED_I2C_Delay();
    OLED_SCL_High();
    OLED_I2C_Delay();
    bit = OLED_SDA_Read() ? 0U : 1U;
    OLED_SCL_Low();
    return (bit != 0U);
}

static OLED_Status OLED_Send(uint8_t control, const uint8_t *data, uint16_t length)
{
    uint16_t i;

    OLED_I2C_Start();
    if (!OLED_I2C_WriteByte((uint8_t) (OLED_ADDRESS << 1U)) ||
        !OLED_I2C_WriteByte(control)) {
        OLED_I2C_Stop();
        return OLED_STATUS_NACK;
    }
    for (i = 0U; i < length; i++) {
        if (!OLED_I2C_WriteByte(data[i])) {
            OLED_I2C_Stop();
            return OLED_STATUS_NACK;
        }
    }
    OLED_I2C_Stop();
    return OLED_STATUS_OK;
}

static OLED_Status OLED_Command(uint8_t command)
{
    return OLED_Send(0x00U, &command, 1U);
}

static const uint8_t *OLED_Glyph(char character)
{
    static const uint8_t blank[5] = {0U, 0U, 0U, 0U, 0U};
    static const uint8_t question[5] = {0x02U, 0x01U, 0x51U, 0x09U, 0x06U};
    static const uint8_t minus[5] = {0x08U, 0x08U, 0x08U, 0x08U, 0x08U};
    static const uint8_t decimalPoint[5] = {0x00U, 0x60U, 0x60U, 0x00U, 0x00U};
    static const uint8_t digits[10][5] = {
        {0x3EU, 0x51U, 0x49U, 0x45U, 0x3EU}, {0x00U, 0x42U, 0x7FU, 0x40U, 0x00U},
        {0x42U, 0x61U, 0x51U, 0x49U, 0x46U}, {0x21U, 0x41U, 0x45U, 0x4BU, 0x31U},
        {0x18U, 0x14U, 0x12U, 0x7FU, 0x10U}, {0x27U, 0x45U, 0x45U, 0x45U, 0x39U},
        {0x3CU, 0x4AU, 0x49U, 0x49U, 0x30U}, {0x01U, 0x71U, 0x09U, 0x05U, 0x03U},
        {0x36U, 0x49U, 0x49U, 0x49U, 0x36U}, {0x06U, 0x49U, 0x49U, 0x29U, 0x1EU}
    };
    static const uint8_t letters[26][5] = {
        {0x7EU,0x11U,0x11U,0x11U,0x7EU},{0x7FU,0x49U,0x49U,0x49U,0x36U},
        {0x3EU,0x41U,0x41U,0x41U,0x22U},{0x7FU,0x41U,0x41U,0x22U,0x1CU},
        {0x7FU,0x49U,0x49U,0x49U,0x41U},{0x7FU,0x09U,0x09U,0x09U,0x01U},
        {0x3EU,0x41U,0x49U,0x49U,0x7AU},{0x7FU,0x08U,0x08U,0x08U,0x7FU},
        {0x00U,0x41U,0x7FU,0x41U,0x00U},{0x20U,0x40U,0x41U,0x3FU,0x01U},
        {0x7FU,0x08U,0x14U,0x22U,0x41U},{0x7FU,0x40U,0x40U,0x40U,0x40U},
        {0x7FU,0x02U,0x0CU,0x02U,0x7FU},{0x7FU,0x04U,0x08U,0x10U,0x7FU},
        {0x3EU,0x41U,0x41U,0x41U,0x3EU},{0x7FU,0x09U,0x09U,0x09U,0x06U},
        {0x3EU,0x41U,0x51U,0x21U,0x5EU},{0x7FU,0x09U,0x19U,0x29U,0x46U},
        {0x46U,0x49U,0x49U,0x49U,0x31U},{0x01U,0x01U,0x7FU,0x01U,0x01U},
        {0x3FU,0x40U,0x40U,0x40U,0x3FU},{0x1FU,0x20U,0x40U,0x20U,0x1FU},
        {0x3FU,0x40U,0x38U,0x40U,0x3FU},{0x63U,0x14U,0x08U,0x14U,0x63U},
        {0x07U,0x08U,0x70U,0x08U,0x07U},{0x61U,0x51U,0x49U,0x45U,0x43U}
    };

    if ((character >= '0') && (character <= '9')) return digits[character - '0'];
    if ((character >= 'A') && (character <= 'Z')) return letters[character - 'A'];
    if (character == ' ') return blank;
    if (character == '-') return minus;
    if (character == '.') return decimalPoint;
    return question;
}

OLED_Status OLED_Init(void)
{
    static const uint8_t sequence[] = {0xAEU,0xD5U,0x80U,0xA8U,0x3FU,0xD3U,0x00U,
        0x40U,0x8DU,0x14U,0x20U,0x00U,0xA1U,0xC8U,0xDAU,0x12U,0x81U,0xCFU,
        0xD9U,0xF1U,0xDBU,0x40U,0xA4U,0xA6U,0xAFU};
    uint8_t i;

    OLED_SCL_High();
    OLED_SDA_High();
    /* The controller requires a delay after its supply reaches operating voltage. */
    delay_cycles(3200000U);
    OLED_Clear();
    for (i = 0U; i < sizeof(sequence); i++) {
        if (OLED_Command(sequence[i]) != OLED_STATUS_OK) return OLED_STATUS_NACK;
    }
    return OLED_Refresh();
}

OLED_Status OLED_Refresh(void)
{
    uint8_t page;
    uint8_t attempt;
    OLED_Status status;

    for (page = 0U; page < 8U; page++) {
        status = OLED_STATUS_NACK;
        for (attempt = 0U; attempt < OLED_PAGE_RETRIES; attempt++) {
            if ((OLED_Command((uint8_t) (0xB0U + page)) == OLED_STATUS_OK) &&
                (OLED_Command(0x00U) == OLED_STATUS_OK) &&
                (OLED_Command(0x10U) == OLED_STATUS_OK)) {
                status = OLED_Send(0x40U,
                    &oledBuffer[(uint16_t) page * OLED_WIDTH], OLED_WIDTH);
            }
            if (status == OLED_STATUS_OK) break;
            /* A stop has been sent by OLED_Send; let the bus return high before retrying. */
            delay_cycles(OLED_I2C_DELAY * 8U);
        }
        if (status != OLED_STATUS_OK) return status;
    }
    return OLED_STATUS_OK;
}

void OLED_Clear(void)
{
    (void) memset(oledBuffer, 0, sizeof(oledBuffer));
    cursorX = 0U;
    cursorY = 0U;
}

void OLED_SetCursor(uint8_t x, uint8_t y)
{
    cursorX = (x < OLED_WIDTH) ? x : 0U;
    cursorY = y;
}

void OLED_DrawPixel(uint8_t x, uint8_t y, bool on)
{
    uint16_t index;
    uint8_t mask;
    if ((x >= OLED_WIDTH) || (y >= OLED_HEIGHT)) return;
    index = (uint16_t) x + ((uint16_t) (y >> 3U) * OLED_WIDTH);
    mask = (uint8_t) (1U << (y & 7U));
    if (on) oledBuffer[index] |= mask; else oledBuffer[index] &= (uint8_t) ~mask;
}

void OLED_WriteChar(char character)
{
    const uint8_t *glyph = OLED_Glyph(character);
    uint8_t column, row;
    if ((cursorX + 5U) >= OLED_WIDTH) {
        cursorX = 0U;
        cursorY = (uint8_t) (cursorY + 8U);
    }
    /* Do not write past the eight display pages when a string wraps. */
    if (cursorY > (OLED_HEIGHT - 8U)) return;
    for (column = 0U; column < 5U; column++) {
        for (row = 0U; row < 7U; row++) OLED_DrawPixel((uint8_t) (cursorX + column), (uint8_t) (cursorY + row), ((glyph[column] >> row) & 1U) != 0U);
    }
    cursorX = (uint8_t) (cursorX + 6U);
}

void OLED_WriteString(const char *text)
{
    while ((text != 0) && (*text != '\0')) OLED_WriteChar(*text++);
}

void OLED_WriteUInt(uint32_t value)
{
    char text[11];
    uint8_t length = 0U;
    do { text[length++] = (char) ('0' + (value % 10U)); value /= 10U; } while (value != 0U);
    while (length > 0U) OLED_WriteChar(text[--length]);
}

void OLED_WriteInt(int32_t value)
{
    if (value < 0) {
        OLED_WriteChar('-');
        OLED_WriteUInt((uint32_t) (-(value + 1)) + 1U);
    } else {
        OLED_WriteUInt((uint32_t) value);
    }
}

void OLED_WriteFloat2(float value)
{
    int32_t scaled;
    uint32_t magnitude;

    /* Round to the displayed hundredth before splitting integer and fraction. */
    scaled = (int32_t) ((value >= 0.0f) ? (value * 100.0f + 0.5f) :
                                           (value * 100.0f - 0.5f));
    if (scaled < 0) {
        OLED_WriteChar('-');
        magnitude = (uint32_t) (-(scaled + 1)) + 1U;
    } else {
        magnitude = (uint32_t) scaled;
    }

    OLED_WriteUInt(magnitude / 100U);
    OLED_WriteChar('.');
    OLED_WriteChar((char) ('0' + ((magnitude / 10U) % 10U)));
    OLED_WriteChar((char) ('0' + (magnitude % 10U)));
}

void OLED_DrawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, bool on)
{
    int16_t dx = (x0 < x1) ? (int16_t) x1 - x0 : (int16_t) x0 - x1;
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t dy = (y0 < y1) ? (int16_t) y0 - y1 : (int16_t) y1 - y0;
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t error = dx + dy;
    while (true) {
        OLED_DrawPixel(x0, y0, on);
        if ((x0 == x1) && (y0 == y1)) break;
        if ((2 * error) >= dy) { error += dy; x0 = (uint8_t) ((int16_t) x0 + sx); }
        if ((2 * error) <= dx) { error += dx; y0 = (uint8_t) ((int16_t) y0 + sy); }
    }
}

void OLED_DrawRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool on)
{
    if ((width == 0U) || (height == 0U)) return;
    OLED_DrawLine(x, y, (uint8_t) (x + width - 1U), y, on);
    OLED_DrawLine(x, (uint8_t) (y + height - 1U), (uint8_t) (x + width - 1U), (uint8_t) (y + height - 1U), on);
    OLED_DrawLine(x, y, x, (uint8_t) (y + height - 1U), on);
    OLED_DrawLine((uint8_t) (x + width - 1U), y, (uint8_t) (x + width - 1U), (uint8_t) (y + height - 1U), on);
}
