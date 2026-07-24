#include "ti_msp_dl_config.h"
#include "Hardware/gray.h"
#include "Hardware/key.h"
#include "Hardware/motor.h"
#include "Hardware/oled.h"
#include "Hardware/task_manager.h"

int main(void)
{
    SYSCFG_DL_init();
    Motor_Init();
    Gray_Init();
    Key_Init();
    delay_cycles(6400000U);

    if (OLED_Init() != OLED_STATUS_OK) {
        while (1) { Motor_Stop(); }
    }

    TaskManager_Init();
    TaskManager_Run();
}
