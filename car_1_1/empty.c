/*
 * Copyright (c) 2021, Texas Instruments Incorporated
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

#include "ti_msp_dl_config.h"
#include "Hardware/buzzer.h"
#include "Hardware/led.h"
#include "Hardware/oled.h"

int main(void)
{
    uint32_t count = 0U;

    SYSCFG_DL_init();
    LED_Init();
    Buzzer_Init();

    if (OLED_Init() == OLED_STATUS_OK) {
        OLED_Clear();
        OLED_DrawRect(0U, 0U, OLED_WIDTH, OLED_HEIGHT, true);
        OLED_SetCursor(18U, 14U);
        OLED_WriteString("LINE FOLLOWER");
        OLED_SetCursor(28U, 30U);
        OLED_WriteString("OLED READY");
        OLED_DrawLine(15U, 48U, 112U, 48U, true);
        (void) OLED_Refresh();
    }

    while (1) {
        LED_Toggle();
        Buzzer_On();
        delay_cycles(800000U);
        Buzzer_Off();

        OLED_SetCursor(42U, 54U);
        OLED_WriteString("COUNT:");
        OLED_WriteUInt(count++);
        (void) OLED_Refresh();
        delay_cycles(15200000U);
    }
}
