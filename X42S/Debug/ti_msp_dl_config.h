/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
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
 *  ============ ti_msp_dl_config.h =============
 *  Configured MSPM0 DriverLib module declarations
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */
#ifndef ti_msp_dl_config_h
#define ti_msp_dl_config_h

#define CONFIG_MSPM0G350X
#define CONFIG_MSPM0G3507

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform all required MSP DL initialization
 *
 *  This function should be called once at a point before any use of
 *  MSP DL.
 */


/* clang-format off */

#define POWER_STARTUP_DELAY                                                (16)


#define CPUCLK_FREQ                                                     32000000



/* Defines for STEPPER_TIMER */
#define STEPPER_TIMER_INST                                               (TIMG0)
#define STEPPER_TIMER_INST_IRQHandler                           TIMG0_IRQHandler
#define STEPPER_TIMER_INST_INT_IRQN                             (TIMG0_INT_IRQn)
#define STEPPER_TIMER_INST_LOAD_VALUE                                   (31999U)




/* Port definition for Pin Group BUZZER */
#define BUZZER_PORT                                                      (GPIOB)

/* Defines for BUZZER_PIN: GPIOB.23 with pinCMx 51 on package pin 22 */
#define BUZZER_BUZZER_PIN_PIN                                   (DL_GPIO_PIN_23)
#define BUZZER_BUZZER_PIN_IOMUX                                  (IOMUX_PINCM51)
/* Port definition for Pin Group OLED_SCL */
#define OLED_SCL_PORT                                                    (GPIOA)

/* Defines for SCL_PIN: GPIOA.31 with pinCMx 6 on package pin 39 */
#define OLED_SCL_SCL_PIN_PIN                                    (DL_GPIO_PIN_31)
#define OLED_SCL_SCL_PIN_IOMUX                                    (IOMUX_PINCM6)
/* Port definition for Pin Group OLED_SDA */
#define OLED_SDA_PORT                                                    (GPIOA)

/* Defines for SDA_PIN: GPIOA.28 with pinCMx 3 on package pin 35 */
#define OLED_SDA_SDA_PIN_PIN                                    (DL_GPIO_PIN_28)
#define OLED_SDA_SDA_PIN_IOMUX                                    (IOMUX_PINCM3)
/* Port definition for Pin Group KEY */
#define KEY_PORT                                                         (GPIOB)

/* Defines for SW1: GPIOB.20 with pinCMx 48 on package pin 19 */
#define KEY_SW1_PIN                                             (DL_GPIO_PIN_20)
#define KEY_SW1_IOMUX                                            (IOMUX_PINCM48)
/* Defines for SW2: GPIOB.21 with pinCMx 49 on package pin 20 */
#define KEY_SW2_PIN                                             (DL_GPIO_PIN_21)
#define KEY_SW2_IOMUX                                            (IOMUX_PINCM49)
/* Defines for A_EN: GPIOA.8 with pinCMx 19 on package pin 54 */
#define STEP_A_EN_PORT                                                   (GPIOA)
#define STEP_A_EN_PIN                                            (DL_GPIO_PIN_8)
#define STEP_A_EN_IOMUX                                          (IOMUX_PINCM19)
/* Defines for A_PUL: GPIOA.0 with pinCMx 1 on package pin 33 */
#define STEP_A_PUL_PORT                                                  (GPIOA)
#define STEP_A_PUL_PIN                                           (DL_GPIO_PIN_0)
#define STEP_A_PUL_IOMUX                                          (IOMUX_PINCM1)
/* Defines for A_DIR: GPIOA.1 with pinCMx 2 on package pin 34 */
#define STEP_A_DIR_PORT                                                  (GPIOA)
#define STEP_A_DIR_PIN                                           (DL_GPIO_PIN_1)
#define STEP_A_DIR_IOMUX                                          (IOMUX_PINCM2)
/* Defines for B_EN: GPIOB.27 with pinCMx 58 on package pin 29 */
#define STEP_B_EN_PORT                                                   (GPIOB)
#define STEP_B_EN_PIN                                           (DL_GPIO_PIN_27)
#define STEP_B_EN_IOMUX                                          (IOMUX_PINCM58)
/* Defines for B_PUL: GPIOA.27 with pinCMx 60 on package pin 31 */
#define STEP_B_PUL_PORT                                                  (GPIOA)
#define STEP_B_PUL_PIN                                          (DL_GPIO_PIN_27)
#define STEP_B_PUL_IOMUX                                         (IOMUX_PINCM60)
/* Defines for B_DIR: GPIOA.26 with pinCMx 59 on package pin 30 */
#define STEP_B_DIR_PORT                                                  (GPIOA)
#define STEP_B_DIR_PIN                                          (DL_GPIO_PIN_26)
#define STEP_B_DIR_IOMUX                                         (IOMUX_PINCM59)


/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);
void SYSCFG_DL_STEPPER_TIMER_init(void);


bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
