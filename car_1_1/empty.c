#include "ti_msp_dl_config.h"
#include "Hardware/buzzer.h"
#include "Hardware/gray.h"
#include "Hardware/IMU660RB/imu660rb.h"
#include "Hardware/key.h"
#include "Hardware/led.h"
#include "Hardware/motor.h"
#include "Hardware/oled.h"
#include "libruary/task_manager.h"

void GROUP1_IRQHandler(void)
{
    if (DL_Interrupt_getPendingGroup(DL_INTERRUPT_GROUP_1) ==
        IMU_INT1_INT_IIDX) {
        const uint32_t pending = DL_GPIO_getPendingInterrupt(GPIOB);
        if (pending == IMU_INT1_INT1_IIDX) {
            DL_GPIO_clearInterruptStatus(GPIOB, IMU_INT1_INT1_PIN);
            IMU660RB_DataReadyNotify();
        }
    }
}

int main(void)
{
    SYSCFG_DL_init();
    NVIC_EnableIRQ(IMU_INT1_INT_IRQN);
    Motor_Init();
    Gray_Init();
    Key_Init();
    LED_Init();
    Buzzer_Init();
    delay_cycles(6400000U);

    if (OLED_Init() != OLED_STATUS_OK) {
        while (1) { Motor_Stop(); }
    }

    TaskManager_Init();
    TaskManager_Run();
}
