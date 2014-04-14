/**
 ****************************************************************************************
 *
 * @file rwble.c
 *
 * @brief Entry points the BLE software
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup ROOT
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <string.h>
#include "co_version.h"
#include "rwble.h"
#if BLE_HCIC_ITF
#include "hcic.h"
#endif //BLE_HCIC_ITF
#include "ke_event.h"
#include "ke_timer.h"
#include "co_buf.h"
#include "lld.h"
#include "llc.h"
#include "llm.h"
#include "dbg.h"
#include "lld_evt.h"
#include "reg_blecore.h"

#if (NVDS_SUPPORT)
#include "nvds.h"         // NVDS definitions
#endif // NVDS_SUPPORT

#include "gtl_eif.h"
#include "uart.h"
#include "reg_ble_em_rx.h"
#include "ke_mem.h"
#include "pll_vcocal_lut.h"
#include "gpio.h"
#include "rf_580.h"
#include "periph_setup.h"


extern uint32_t lp_clk_sel;
/*
 * FORWARD DECLARATION OF GLOBAL FUNCTIONS
 ****************************************************************************************
 */
void ble_regs_pop (void);


/*
 * GLOBAL VARIABLES DECLARATION
 ****************************************************************************************
 */
extern uint8_t func_check_mem_flag;

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
 
 
// ============================================================================================
// ==================== DEEP SLEEP PATCH - THIS CODE MUST STAY IN RAM =========================
// ============================================================================================
extern void rf_workaround_init(void);
extern void rf_reinit(void);

// /*********************************************************************************
//  *** WAKEUP_LP_INT ISR
//  ***/
void BLE_WAKEUP_LP_Handler(void)
{    
	volatile long t=0;

#if !(USE_WDOG)
    SetWord16(SET_FREEZE_REG, FRZ_WDOG); //Prepare WDOG, i.e. stop
#endif
     
//    // Gives 1dB higher sensitivity - UNTESTED
//    if (GetBits16(ANA_STATUS_REG, BOOST_SELECTED) == 0x1) 
//    { 
//        // Boost-mode
//        SetBits16(DCDC_CTRL2_REG, DCDC_CUR_LIM, 0x8); // 80mA
//    }
//    else 
//    { 
//        // Buck-mode
//        SetBits16(DCDC_CTRL2_REG, DCDC_CUR_LIM, 0x4); // 40mA
//    }
    
	/*
	* Wait and Switch to XTAL 16MHz
	* (default after each wake-up is RC 16MHz, but XTAL initialization sequence has been already initiated by PMU)
	* NOTE: 
	*       1. If app does not need XTAL16MHz but RC16MHz is enough then skip this section!
	*       2. Wait-loop BEFORE activating PERIPH_PD in order to save some power...
	*/
    // It will save some power if you lower the clock while waiting for XTAL16 to settle.
	// Could also switch to 32KHz, but then processing power is dramatically reduced (e.g. patching() routine may be too slow).
    SetBits16(CLK_AMBA_REG, PCLK_DIV, 3);  // lowest is 2MHz (div 8, source is RC @16MHz)
    SetBits16(CLK_AMBA_REG, HCLK_DIV, 3);

    while ( !GetBits16(SYS_STAT_REG, XTAL16_SETTLED) )  // this takes some mili seconds
        __NOP(), __NOP(), __NOP();  // reduce some APB activity

    SetBits16(CLK_CTRL_REG, SYS_CLK_SEL, 0); // select XTAL 16MHz
    SetBits16(CLK_16M_REG, RC16M_ENABLE, 0); // save power from RC 16MHz
    
    // and restore clock rates (refer to a couple of lines above)
    SetBits16(CLK_AMBA_REG, PCLK_DIV, 0);
    SetBits16(CLK_AMBA_REG, HCLK_DIV, 0);
    /*
	* Init System Power Domain blocks: GPIO, WD Timer, Sys Timer, etc.
	* Power up and init Peripheral Power Domain blocks,
	* and finally release the pad latches.
	*/
	
    periph_init();

    
	/*
	* Since XTAL 16MHz is activated, power-up the Radio Subsystem (including BLE)
	*
	* Note that BLE core clock is masked in order to handle the case where RADIO_PD does not get into power down state.
	* The BLE clock should be active only as long as system is running at XTAL 16MHz (not at RC16 or 32KHz).
	* Also BLE clock should be enabled before powering up the RADIO Power Domain !
	*/
	SetBits16(CLK_RADIO_REG, BLE_ENABLE, 1); // BLE clock enable
	SetBits16(PMU_CTRL_REG, RADIO_SLEEP, 0); // Power up! Note: System must run at 16MHz when powering up RADIO_PD.
	while (!(GetWord16(SYS_STAT_REG) & RAD_IS_UP)) ; // this may take up to 1/2 of the 32KHz clock period


	/* 
	* Wait for at least one Low Power clock edge after the power up of the Radio Power Domain *e.g. with ble_wait_lp_clk_posedge() )
	* or even better check the BLE_CNTL2_REG[WAKEUPLPSTAT] !
	* Thus you assure that BLE_WAKEUP_LP_IRQ is deasserted and BLE_SLP_IRQ is asserted.
	* After this check exit this ISE in order to proceed with BLE_SLP_Handler().
	*/
	while ( GetBits32(BLE_CNTL2_REG, WAKEUPLPSTAT) || !GetBits32(BLE_INTSTAT_REG, SLPINTSTAT))
		if (t) break;

	// Now BLE_WAKEUP_LP_IRQ is deasserted and BLE_SLP_IRQ is asserted, so exit in order to proceed with BLE_SLP_Handler().
	// NOTE: If returning from BLE_WAKEUP_LP_Handler() will not cause BLE_SLP_Handler() to start, 
	//       but the code after __WFI() is executed, then THERE WAS A SW SETUP PROBLEM !!! 
	//			 so it is recommended to place a check after __WFI().
	
	/*
	* Radio Subsystem initialization. Execute here after making sure that BLE core is awake.
	*/
	rf_workaround_init();
	rf_reinit();	
}

#if 0
// /*********************************************************************************
//  *** CSCNT ISR
//  ***/
void BLE_CSCNT_Handler(void)
{
    SetBits32(BLE_INTACK_REG, CSCNTINTACK, 1);
#if DEEP_SLEEP //Needed only for compilation. Remove when ROM code is ready.
#if RW_BLE_SUPPORT
    // Handle end of wake-up
	rwip_wakeup_end();
	/*
 	 *	Init TX and RX buffers, they are in EM but not in the retainable part
 	 *  so the pointers have to be programmed again	*
 	 */
	if(func_check_mem_flag)
	{
		if (func_check_mem_flag==2)
		{
	  //init TX/RX buffers after DEEPSLEEP	
		co_buf_init_deep_sleep();
		// Set the first RX descriptor pointer into the HW
        ble_currentrxdescptr_set(REG_BLE_EM_RX_ADDR_GET(co_buf_rx_current_get()));
		}   	
       //INIT NONE RET. HEAP after DEEPSLEEP	
       	ke_mem_init(KE_MEM_NON_RETENTION, (uint8_t*)(jump_table_struct[rwip_heap_non_ret_pos]), jump_table_struct[rwip_heap_non_ret_size]);

		func_check_mem_flag = 0;//false;
		
	}
 #endif //RW_BLE_SUPPORT
#endif //DEEP_SLEEP
}

#endif //0

// /*********************************************************************************
//  *** SLP_INT ISR
//  ***/
void BLE_SLP_Handler(void)
//void BLE_SLP_Handler_func(void)
{
	ble_regs_pop();
	//smpc_regs_pop();
    
//#if !DEEP_SLEEP_ENABLED
//# if  DEVELOPMENT__NO_OTP    
//	SetBits16(SYS_CTRL_REG, DEBUGGER_ENABLE, 1);
//# else
//	SetBits16(SYS_CTRL_REG, DEBUGGER_ENABLE, 0);   
//# endif	// DEVELOPMENT__NO_OTP
//#endif // !DEEP_SLEEP_ENABLED

	SetBits16(GP_CONTROL_REG, BLE_WAKEUP_REQ, 0);   //just to be sure    

    if(jump_table_struct[0] == TASK_GTL)	
	{
		// UART and pads have already been activated by periph_init() which is called
		// at initialization by main_func() and during wakeup by BLE_WAKEUP_LP_Handler().
		
		gtl_eif_init();
	}

	SetBits32(BLE_INTACK_REG, SLPINTACK, 1);

#if DEEP_SLEEP //Needed only for compilation. Remove when ROM code is ready.
#if RW_BLE_SUPPORT
	rwip_wakeup();
#endif //RW_BLE_SUPPORT
#endif //DEEP_SLEEP
    
    if (lp_clk_sel == LP_CLK_RCX20)
        calibrate_rcx20(20);
}

#if 0

/*********************************************************************************
 *** ERROR_INT ISR
 ***/
void BLE_ERROR_Handler(void)
{
#if DEVELOPMENT__NO_OTP
	// Break into the debugger if one is attached otherwise a hardfault int will be triggerd
	__asm("BKPT #0\n"); 
#endif	
	
	// Handle exception accordingly
	
	// Acknowledge interrupt
    SetBits32(BLE_INTACK_REG, ERRORINTACK, 1);
	SetBits32(BLE_CNTL2_REG, EMACCERRACK, 1);
}

/*********************************************************************************
 *** EVENT_INT ISR
 ***/
void BLE_EVENT_Handler(void)
{
    // Check BLE interrupt status and call the appropriate handlers
    //uint32_t irq_stat = ble_intstat_get();
    uint32_t irq_stat = GetWord16(BLE_INTSTAT_REG);

    // End of event interrupt
    if (irq_stat & BLE_EVENTINTSTAT_BIT)
    {
        lld_evt_end_isr();
    }
}

/*********************************************************************************
 *** 
 ***/

#endif //if 0

void BLE_RF_DIAG_Handler(void)
{
  uint32 irq_scr;
  uint16_t cn;    
  cn = GetWord16(RF_BMCW_REG) & 0x003F;    

  irq_scr = GetWord32(BLE_RF_DIAGIRQ_REG);  // read BLE_RF_DIAGIRQ_REG so that you clear DIAGIRQ_STAT_0 (otherwise interrupt is activated again and again!)
    
#if LUT_PATCH_ENABLED 
  const volatile struct LUT_CFG_struct *pLUT_CFG;	// = (const volatile struct LUT_CFG_struct *)(jump_table_struct[lut_cfg_pos]);
  pLUT_CFG = (const volatile struct LUT_CFG_struct *)(jump_table_struct[lut_cfg_pos]);
  if(!pLUT_CFG->HW_LUT_MODE)
  { 
    set_rf_cal_cap(cn); 
  }
#endif
  
#if MGCKMODA_PATCH_ENABLED 
  if(GetBits16(RF_MGAIN_CTRL_REG, GAUSS_GAIN_SEL) && (irq_scr & DIAGIRQ_STAT_0))  // TODO: If GAUSS_GAIN_SEL==0x1 AND it is an TX_EN interrupt (for RX_EN int it is not necessary to run)
  { 
    set_gauss_modgain(cn); 
  }
#endif
  
#ifdef PRODUCTION_TEST  
	if( irq_scr & DIAGIRQ_STAT_0)//check TXEN posedge 
	{	
		test_tx_packet_nr++;
	}
	if( irq_scr & DIAGIRQ_STAT_1)//check RXEN posedge 
	{	
		test_rx_irq_cnt++;
	}  
#endif    
}

/// @} RWBLE
