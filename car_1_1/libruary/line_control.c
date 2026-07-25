#include "line_control.h"

#define LINE_BASE_DUTY       (20)
#define LINE_ALL_BLACK_DUTY  (12)
#define LINE_HARD_INNER_DUTY (2)
#define LINE_HARD_OUTER_DUTY (32)
#define LINE_MAX_CORRECTION  (10)
#define LINE_P_GAIN          (0.75f)
#define LINE_D_GAIN          (0.90f)
#define LINE_HARD_POSITION   (9)
#define LINE_TURN_STOP_SAMPLES (13U)
#define LINE_CENTER_MASK      (0x00F0U)

static int16_t previousPosition;
static LineControl_Mode turnMode;
static uint8_t turnStopSamples;
static uint8_t centerSamples;

static uint8_t LineControl_ClampDuty(int16_t duty)
{
    if (duty <= 0) return 0U;
    if (duty >= 100) return 100U;
    return (uint8_t) duty;
}

void LineControl_Init(void)
{
    previousPosition = 0;
    turnMode = LINE_CONTROL_TRACKING;
    turnStopSamples = 0U;
    centerSamples = 0U;
}

LineControl_Output LineControl_Update(Gray_Result gray)
{
    LineControl_Output output;
    int16_t correction;
    int16_t delta;

    output.position = gray.position;

    if (turnMode == LINE_CONTROL_TURN_STOP) {
        output.leftDuty = 0U;
        output.rightDuty = 0U;
        output.mode = LINE_CONTROL_TURN_STOP;
        if (--turnStopSamples == 0U) {
            turnMode = (previousPosition < 0) ? LINE_CONTROL_HARD_LEFT :
                                                 LINE_CONTROL_HARD_RIGHT;
        }
        return output;
    }
    if ((turnMode == LINE_CONTROL_HARD_LEFT) ||
        (turnMode == LINE_CONTROL_HARD_RIGHT)) {
        if ((gray.status == GRAY_STATUS_NORMAL) &&
            ((gray.raw & LINE_CENTER_MASK) != 0U)) {
            if (++centerSamples >= 2U) {
                turnMode = LINE_CONTROL_TRACKING;
                previousPosition = gray.position;
            }
        } else {
            centerSamples = 0U;
        }
        if (turnMode == LINE_CONTROL_HARD_LEFT) {
            output.leftDuty = 0U;
            output.rightDuty = LINE_HARD_OUTER_DUTY;
            output.mode = LINE_CONTROL_HARD_LEFT;
            return output;
        }
        if (turnMode == LINE_CONTROL_HARD_RIGHT) {
            output.leftDuty = LINE_HARD_OUTER_DUTY;
            output.rightDuty = 0U;
            output.mode = LINE_CONTROL_HARD_RIGHT;
            return output;
        }
    }

    if (gray.status == GRAY_STATUS_LOST) {
        output.leftDuty = LINE_BASE_DUTY;
        output.rightDuty = LINE_BASE_DUTY;
        output.mode = LINE_CONTROL_LOST;
        return output;
    }
    if (gray.status == GRAY_STATUS_ALL_BLACK) {
        output.leftDuty = LINE_ALL_BLACK_DUTY;
        output.rightDuty = LINE_ALL_BLACK_DUTY;
        output.mode = LINE_CONTROL_ALL_BLACK;
        previousPosition = 0;
        return output;
    }

    if (gray.position <= -LINE_HARD_POSITION) {
        previousPosition = gray.position;
        turnMode = LINE_CONTROL_TURN_STOP;
        turnStopSamples = LINE_TURN_STOP_SAMPLES;
        centerSamples = 0U;
        output.leftDuty = 0U;
        output.rightDuty = 0U;
        output.mode = LINE_CONTROL_TURN_STOP;
        return output;
    }
    if (gray.position >= LINE_HARD_POSITION) {
        previousPosition = gray.position;
        turnMode = LINE_CONTROL_TURN_STOP;
        turnStopSamples = LINE_TURN_STOP_SAMPLES;
        centerSamples = 0U;
        output.leftDuty = 0U;
        output.rightDuty = 0U;
        output.mode = LINE_CONTROL_TURN_STOP;
        return output;
    }

    delta = gray.position - previousPosition;
    correction = (int16_t) ((LINE_P_GAIN * (float) gray.position) +
                            (LINE_D_GAIN * (float) delta));
    if (correction > LINE_MAX_CORRECTION) correction = LINE_MAX_CORRECTION;
    if (correction < -LINE_MAX_CORRECTION) correction = -LINE_MAX_CORRECTION;
    output.leftDuty = LineControl_ClampDuty(LINE_BASE_DUTY + correction);
    output.rightDuty = LineControl_ClampDuty(LINE_BASE_DUTY - correction);
    output.mode = LINE_CONTROL_TRACKING;
    previousPosition = gray.position;
    return output;
}
