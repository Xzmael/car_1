/*
 * Copyright (c) 2023, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ============ ti_msp_dl_config.c =============
 *  Configured MSPM0 DriverLib module definitions
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */

#include "ti_msp_dl_config.h"

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform any initialization needed before using any board APIs
 */
SYSCONFIG_WEAK void SYSCFG_DL_init(void)
{
    SYSCFG_DL_initPower();
    SYSCFG_DL_GPIO_init();
    /* Module-Specific Initializations*/
    SYSCFG_DL_SYSCTL_init();
    SYSCFG_DL_STEPPER_TIMER_init();
}



SYSCONFIG_WEAK void SYSCFG_DL_initPower(void)
{
    DL_GPIO_reset(GPIOA);
    DL_GPIO_reset(GPIOB);
    DL_TimerG_reset(STEPPER_TIMER_INST);

    DL_GPIO_enablePower(GPIOA);
    DL_GPIO_enablePower(GPIOB);
    DL_TimerG_enablePower(STEPPER_TIMER_INST);
    delay_cycles(POWER_STARTUP_DELAY);
}

SYSCONFIG_WEAK void SYSCFG_DL_GPIO_init(void)
{

    DL_GPIO_initDigitalOutput(BUZZER_BUZZER_PIN_IOMUX);

    DL_GPIO_initDigitalOutput(OLED_SCL_SCL_PIN_IOMUX);

    DL_GPIO_initDigitalOutput(OLED_SDA_SDA_PIN_IOMUX);

    DL_GPIO_initDigitalInputFeatures(KEY_SW1_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(KEY_SW2_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalOutput(STEP_A_EN_IOMUX);

    DL_GPIO_initDigitalOutput(STEP_A_PUL_IOMUX);

    DL_GPIO_initDigitalOutput(STEP_A_DIR_IOMUX);

    DL_GPIO_initDigitalOutput(STEP_B_EN_IOMUX);

    DL_GPIO_initDigitalOutput(STEP_B_PUL_IOMUX);

    DL_GPIO_initDigitalOutput(STEP_B_DIR_IOMUX);

    DL_GPIO_clearPins(GPIOA, STEP_A_DIR_PIN |
		STEP_B_DIR_PIN);
    DL_GPIO_setPins(GPIOA, OLED_SCL_SCL_PIN_PIN |
		OLED_SDA_SDA_PIN_PIN |
		STEP_A_EN_PIN |
		STEP_A_PUL_PIN |
		STEP_B_PUL_PIN);
    DL_GPIO_enableOutput(GPIOA, OLED_SCL_SCL_PIN_PIN |
		OLED_SDA_SDA_PIN_PIN |
		STEP_A_EN_PIN |
		STEP_A_PUL_PIN |
		STEP_A_DIR_PIN |
		STEP_B_PUL_PIN |
		STEP_B_DIR_PIN);
    DL_GPIO_setPins(GPIOB, BUZZER_BUZZER_PIN_PIN |
		STEP_B_EN_PIN);
    DL_GPIO_enableOutput(GPIOB, BUZZER_BUZZER_PIN_PIN |
		STEP_B_EN_PIN);

}


SYSCONFIG_WEAK void SYSCFG_DL_SYSCTL_init(void)
{

	//Low Power Mode is configured to be SLEEP0
    DL_SYSCTL_setBORThreshold(DL_SYSCTL_BOR_THRESHOLD_LEVEL_0);

    DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);
    /* Set default configuration */
    DL_SYSCTL_disableHFXT();
    DL_SYSCTL_disableSYSPLL();
    DL_SYSCTL_setULPCLKDivider(DL_SYSCTL_ULPCLK_DIV_1);
    DL_SYSCTL_setMCLKDivider(DL_SYSCTL_MCLK_DIVIDER_DISABLE);

}



/*
 * Timer clock configuration to be sourced by BUSCLK /  (32000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   32000000 Hz = 32000000 Hz / (1 * (0 + 1))
 */
static const DL_TimerG_ClockConfig gSTEPPER_TIMERClockConfig = {
    .clockSel    = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale    = 0U,
};

/*
 * Timer load value (where the counter starts from) is calculated as (timerPeriod * timerClockFreq) - 1
 * STEPPER_TIMER_INST_LOAD_VALUE = (1 ms * 32000000 Hz) - 1
 */
static const DL_TimerG_TimerConfig gSTEPPER_TIMERTimerConfig = {
    .period     = STEPPER_TIMER_INST_LOAD_VALUE,
    .timerMode  = DL_TIMER_TIMER_MODE_PERIODIC,
    .startTimer = DL_TIMER_STOP,
};

SYSCONFIG_WEAK void SYSCFG_DL_STEPPER_TIMER_init(void) {

    DL_TimerG_setClockConfig(STEPPER_TIMER_INST,
        (DL_TimerG_ClockConfig *) &gSTEPPER_TIMERClockConfig);

    DL_TimerG_initTimerMode(STEPPER_TIMER_INST,
        (DL_TimerG_TimerConfig *) &gSTEPPER_TIMERTimerConfig);
    DL_TimerG_enableInterrupt(STEPPER_TIMER_INST , DL_TIMERG_INTERRUPT_ZERO_EVENT);
	NVIC_SetPriority(STEPPER_TIMER_INST_INT_IRQN, 1);
    DL_TimerG_enableClock(STEPPER_TIMER_INST);





}


