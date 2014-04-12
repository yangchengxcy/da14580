/**
 ****************************************************************************************
 *
 * @file wucpt_handler.c
 *
 * @brief Wakeup IRQ Handler.
 *
 * Copyright (C) 2013. Dialog Semiconductor Ltd, unpublished work. This computer 
 * program includes Confidential, Proprietary Information and is a Trade Secret of 
 * Dialog Semiconductor Ltd.  All use, disclosure, and/or reproduction is prohibited 
 * unless authorized in writing. All Rights Reserved.
 *
 * <bluetooth.support@diasemi.com> and contributors.
 *
 ****************************************************************************************
 */

#include "global_io.h"
#include "ARMCM0.h"

#include "app.h"
#include "app_keyboard.h"
#include "app_task.h"

#if BLE_HID_DEVICE

extern void periph_init(void);

/**
 * WUPCT_Handler:
 */
void WKUP_QUADEC_Handler(void)
{
	/*
	* The system operates with RC16 clk
	*/
	
	/*
	* Prepare WDOG, i.e. stop
	*/
#if !(USE_WDOG)    
	SetWord16(SET_FREEZE_REG, FRZ_WDOG);
#endif    

#if !(ES4_CODE)    
    if (GetBits16(ANA_STATUS_REG, BOOST_SELECTED) == 0x1) { // Boost-mode
		SetBits16(DCDC_CTRL2_REG, DCDC_CUR_LIM, 0xA); // 100mA (default = preferred setting)
		SetBits16(DCDC_CTRL2_REG, DCDC_AUTO_CAL, 0x6); // default = preferred setting
	}
	else { // Buck-mode
		SetBits16(DCDC_CTRL2_REG, DCDC_AUTO_CAL, 0x1); // default = preferred setting
    }
#endif    
    
	/*
	* Restore clock 
	*/
	SetBits16(CLK_AMBA_REG, PCLK_DIV, 0); 
	SetBits16(CLK_AMBA_REG, HCLK_DIV, 0);
	
	SetBits16(CLK_PER_REG, WAKEUPCT_ENABLE, 1);  // enable clock of Wakeup Controller
	SetWord16(WKUP_RESET_IRQ_REG, 1); //Acknowledge it
	
	//No more interrupts of this kind
	SetBits16(WKUP_CTRL_REG, WKUP_ENABLE_IRQ, 0);
	NVIC_DisableIRQ(WKUP_QUADEC_IRQn);
	
	/*
	* Init System Power Domain blocks: GPIO, WD Timer, Sys Timer, etc.
	* Power up and init Peripheral Power Domain blocks,
	* and finally release the pad latches.
	*/
	app_env.app_flags &= ~ENABLE_WKUP_INT; //clear flag before entering periph_init()

    if(GetBits16(SYS_STAT_REG, PER_IS_DOWN))
		periph_init();

	/*
	* Notify HID Application to start scanning
	*/
	app_env.app_flags |= KBD_START_SCAN;

	return;
}

#endif
