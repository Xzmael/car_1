#ifndef HARDWARE_GRAY_H
#define HARDWARE_GRAY_H

#include <stdint.h>

#define GRAY_SENSOR_COUNT (12U)

typedef enum {
    GRAY_STATUS_NORMAL = 0,
    GRAY_STATUS_LOST,
    GRAY_STATUS_ALL_BLACK
} Gray_Status;

typedef struct {
    uint16_t raw;
    uint8_t blackCount;
    int16_t position;
    Gray_Status status;
} Gray_Result;

void Gray_Init(void);
void Gray_Read(void);
uint16_t Gray_GetRaw(void);
Gray_Result Gray_GetResult(void);

#endif
