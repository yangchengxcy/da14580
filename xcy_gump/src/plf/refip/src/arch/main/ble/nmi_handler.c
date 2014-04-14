/**
 ****************************************************************************************
 *
 * @file nmi_handler.c
 *
 * @brief NMI hundler. 
 *
 * Copyright (C) 2012. Dialog Semiconductor Ltd, unpublished work. This computer 
 * program includes Confidential, Proprietary Information and is a Trade Secret of 
 * Dialog Semiconductor Ltd.  All use, disclosure, and/or reproduction is prohibited 
 * unless authorized in writing. All Rights Reserved.
 *
 * <bluetooth.support@diasemi.com> and contributors.
 *
 ****************************************************************************************
 */

#include "rwip_config.h"        // RW SW configuration
#include "global_io.h"
#include "ARMCM0.h"


void NMI_Handler(void)
{
    // Reached this point due to a WDOG timeout
    
    SetBits16(PMU_CTRL_REG, RADIO_SLEEP, 1);        // turn off radio PD
    SetBits16(PMU_CTRL_REG, PERIPH_SLEEP, 1);       // turn off peripheral power domain
    SetBits16(CLK_RADIO_REG, BLE_LP_RESET, 1);      // reset the BLE LP timer
    NVIC_ClearPendingIRQ(BLE_WAKEUP_LP_IRQn);       // clear any pending LP IRQs
#if DEVELOPMENT__NO_OTP
    SetWord16(SET_FREEZE_REG, FRZ_WDOG);            // Stop WDOG
    SetBits16(SYS_CTRL_REG, DEBUGGER_ENABLE, 1);    // enable debugger to be able to re-attach
    if ((GetWord16(SYS_STAT_REG) & DBG_IS_UP) == DBG_IS_UP)
        __asm("BKPT #0\n");
#else
    
    // Remap addres 0x00 to ROM and force execution
    SetWord16(SYS_CTRL_REG, (GetWord16(SYS_CTRL_REG) & ~REMAP_ADR0) | SW_RESET );
#endif

    // Will reach this point only in Development when using any sleep mode!
    while(1);
}
