#include "ti_msp_dl_config.h"

#include "Hardware/buzzer.h"
#include "Hardware/key.h"
#include "Hardware/oled.h"
#include "Hardware/stepper.h"
#include "libruary/task_manager.h"

void STEPPER_TIMER_INST_IRQHandler(void)
{
    if ((DL_TimerG_getPendingInterrupt(STEPPER_TIMER_INST) & DL_TIMER_INTERRUPT_ZERO_EVENT) != 0U) {
        DL_TimerG_clearInterruptStatus(STEPPER_TIMER_INST, DL_TIMER_INTERRUPT_ZERO_EVENT);
        Stepper_TimerIRQHandler();
    }
}

int main(void)
{
    SYSCFG_DL_init();
    Buzzer_Init();
    Key_Init();
    Stepper_Init();
    (void) OLED_Init();
    TaskManager_Init();

    NVIC_EnableIRQ(STEPPER_TIMER_INST_INT_IRQN);
    while (1) {
        Key_Scan();
        TaskManager_Update();
    }
}
