/**
 ****************************************************************************************
 *
 * @file arch_main.c
 *
 * @brief Main loop of the application.
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


/*
 * INCLUDES
 ****************************************************************************************
 */

#include "arch.h"
#include "arch_sleep.h"
#include <stdlib.h>
#include <stddef.h>     // standard definitions
#include <stdint.h>     // standard integer definition
#include <stdbool.h>    // boolean definition
#include "boot.h"       // boot definition
#include "rwip.h"       // BLE initialization
#include "syscntl.h"    // System control initialization
#include "emi.h"        // EMI initialization
#include "intc.h"       // Interrupt initialization
#include "timer.h"      // TIMER initialization
#include "em_map_ble.h"
#include "ke_mem.h"
#include "ke_event.h"
#include "smpc.h"
#include "llc.h"
#include "pll_vcocal_lut.h"
#include "periph_setup.h"

#if(ROLE_MASTER_YCOM)
bool existence_flag=false;
#endif

#if PLF_UART
#include "uart.h"       // UART initialization
#endif //PLF_UART

#include "nvds.h"       // NVDS initialization

#if (BLE_EMB_PRESENT)
#include "rf.h"         // RF initialization
#endif // BLE_EMB_PRESENT

#if (BLE_HOST_PRESENT)
#include "rwble_hl.h"   // BLE HL definitions
#include "gapc.h"
#include "smpc.h"
#include "gattc.h"
#include "attc.h"
#include "atts.h"
#include "l2cc.h"
#endif //BLE_HOST_PRESENT

#if (BLE_APP_PRESENT)
#include "app.h"       // application functions
#include "app_sleep.h"
#endif // BLE_APP_PRESENT

#include "gtl_env.h"
#include "gtl_task.h"

#if PLF_DEBUG
#include "dbg.h"       // For dbg_warning function
#endif //PLF_DEBUG

#include "global_io.h"

#include "datasheet.h"

#include "em_map_ble_user.h"
#include "em_map_ble.h"

#include "lld_sleep.h"
#include "rwble.h"
#include "rf_580.h"
#include "gpio.h"
#include "gtl_task.h"
/**
 * @addtogroup DRIVERS
 * @{
 */
extern void app_connect(unsigned char indx);
/*
 * DEFINES
 ****************************************************************************************
 */
/// NVDS location in FLASH : 0x000E0000 (896KB (1Mo - 128KB))
#define NVDS_FLASH_ADDRESS          (0x00000270)

/// NVDS size in RAM : 0x00010000 (128KB)
#define NVDS_FLASH_SIZE             (0x00000200)

#if DEVELOPMENT__NO_OTP
    #warning "==============================================================> DEVELOPMENT__NO_OTP is set!"
#endif		


extern int l2cc_pdu_recv_ind_handler(ke_msg_id_t const msgid, struct l2cc_pdu_recv_ind *param,
                                        ke_task_id_t const dest_id, ke_task_id_t const src_id);

extern int smpc_pairing_cfm_handler(ke_msg_id_t const msgid,
                                    struct smpc_pairing_cfm *param,
                                    ke_task_id_t const dest_id, ke_task_id_t const src_id);

/*
 * STRUCTURE DEFINITIONS
 ****************************************************************************************
 */

/// Description of unloaded RAM area content
struct unloaded_area_tag
{
    uint32_t error;
};

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

#if 0   //PLF_DEBUG
volatile int dbg_assert_block = 1;  /// Variable to enable infinite loop on assert
#endif //PLF_DEBUG

/// Pointer to access unloaded RAM area
extern struct unloaded_area_tag* unloaded_area;

extern uint32_t error;              /// Variable storing the reason of platform reset

extern uint32_t last_temp_time;     /// time of last temperature count measurement
extern uint16_t last_temp_count;    /// temperature counter
extern uint32_t lp_clk_sel;

/// Reserve space for Exchange Memory, this section is linked first in the section "exchange_mem_case"
extern volatile uint8 dummy[DUMMY_SIZE];
extern uint8_t func_check_mem_flag;
extern struct lld_sleep_env_tag lld_sleep_env;
extern struct arch_sleep_env_tag sleep_env;
extern struct gtl_env_tag gtl_env;

uint32_t lld_sleep_lpcycles_2_us_func(uint32_t);

volatile uint8 descript[EM_SYSMEM_SIZE] __attribute__((at(EM_SYSMEM_START))); //CASE_15_OFFSET
bool sys_startup_flag __attribute__((section("retention_mem_area0"), zero_init));
#if (BLE_CONNECTION_MAX_USER > 4)
volatile uint8_t cs_table[EM_BLE_CS_COUNT_USER * REG_BLE_EM_CS_SIZE] __attribute__((section("cs_area"), zero_init));
#endif

/*
 * LOCAL FUNCTION DECLARATIONS
 ****************************************************************************************
 */

static bool cmp_abs_time(struct co_list_hdr const * timerA, struct co_list_hdr const * timerB)
{
    uint32_t timeA = ((struct ke_timer*)timerA)->time;
    uint32_t timeB = ((struct ke_timer*)timerB)->time;

    return (((uint32_t)( (timeA - timeB) & 0xFFFF) ) > KE_TIMER_DELAY_MAX);
}


// h/w 
const uint32_t * const patch_table[]={
	(const uint32_t *) cmp_abs_time,
	(const uint32_t *) l2cc_pdu_recv_ind_handler,
	(const uint32_t *) smpc_send_pairing_req_ind,
	(const uint32_t *) smpc_check_pairing_feat,
	(const uint32_t *) smpc_pairing_cfm_handler,
	
};

// Coarse Calibration configuration
const struct LUT_CFG_struct LUT_CFG =
    { 
        /*.HW_LUT_MODE               =*/ 1,                     // 1: HW LUT, 0: SW LUT    
        /*.RX_HSI_ENABLED 			 =*/ 1, 
		/*.PA_PULLING_OFFSET 		 =*/ 0, 
		/*.NR_CCUPD_1ST 		     =*/ 10, 
		/*.NR_CCUPD_REST 		     =*/ 4, 
		/*.NR_CCUPD_OL               =*/ 40, 
        /*.BLE_BAND_MARGIN           =*/ 10,                    // in MHz
		/*.EST_HALF_OVERLAP 		 =*/ 4,                     // in MHz
        /*.REQUIRED_CHAN_OVERLAP     =*/ 2,                     // At least 2 channels should be synthesizable by two adjacent calcaps (both of them)
        /*.PLL_LOCK_VTUNE_NUMAVGPOW  =*/ 3,                     // So 2^3=8 samples averaging
        /*.PLL_LOCK_VTUNE_LIMIT_LO   =*/ (1024*200/1200),       // Min acceptable Vtune = 0200mV
        /*.PLL_LOCK_VTUNE_LIMIT_HI   =*/ (1024*1000/1200),      // Max acceptable Vtune = 1000mV
        /*.PLL_LOCK_VTUNE_P2PVAR     =*/ (1024*50/1200),        // Vtune has to be stable within 50mV
        /*.PLL_LOCK_TIMING			 =*/ 416,    				// 416*62.5nsec=26usec
		/*.VCO_CALCNT_STARTVAL	 	 =*/ 0xFFFF,    			// Just in case it is modified by the Metal Fixes
		/*.VCO_CALCNT_TIMEOUT	 	 =*/ 300,    				// Just in case the while loops lock in meas_precharge_freq()
};


/**
 ****************************************************************************************
 * @brief SVC_Handler. Handles h/w patching mechanism IRQ
 *
 * @return void 
 ****************************************************************************************
 */

void SVC_Handler_c(unsigned int * svc_args)

{
// Stack frame contains:
// r0, r1, r2, r3, r12, r14, the return address and xPSR
// - Stacked R0 = svc_args[0]
// - Stacked R1 = svc_args[1]
// - Stacked R2 = svc_args[2]
// - Stacked R3 = svc_args[3]
// - Stacked R12 = svc_args[4]
// - Stacked LR = svc_args[5]
// - Stacked PC = svc_args[6]
// - Stacked xPSR= svc_args[7]

	unsigned int svc_number;
    
	svc_number = ((char *)svc_args[6])[-2];
    
	if (svc_number < (sizeof patch_table)/4)
		svc_args[6] = (uint32_t)patch_table[svc_number];
	else
		while(1);

	return;

}

/**
 ****************************************************************************************
 * @brief Set and enable h/w Patch functions
 *
 * @return void
 ****************************************************************************************
 */

void patch_func (void)
{
    //0x00032795 cmp_abs_time
    SetWord32(PATCH_ADDR0_REG, 0x00032794);
    SetWord32(PATCH_DATA0_REG, 0x6889df00); //cmp_abs_time svc 0

    //0x0002a32b l2cc_pdu_recv_ind_handler (atts)
    SetWord32(PATCH_ADDR1_REG, 0x0002a328);
    SetWord32(PATCH_DATA1_REG, 0xdf014770); //l2cc_pdu_recv_ind_handler svc 1
	
    //0x0002ca1f  smpc_send_pairing_req_ind
    SetWord32(PATCH_ADDR2_REG, 0x0002ca1c);
    SetWord32(PATCH_DATA2_REG, 0xdf02bdf8); //smpc_send_pairing_req_ind svc 2

    //0x0002cb43  smpc_check_pairing_feat
    SetWord32(PATCH_ADDR3_REG, 0x0002cb40);
    SetWord32(PATCH_DATA3_REG, 0xdf03e7f5); //smpc_check_pairing_feat svc 3
	
    //0x0002d485  smpc_pairing_cfm_handler
    SetWord32(PATCH_ADDR4_REG, 0x0002d484);
    SetWord32(PATCH_DATA4_REG, 0xb089df04); //smpc_pairing_cfm_handler svc 4


    NVIC_DisableIRQ(SVCall_IRQn);
    NVIC_SetPriority(SVCall_IRQn, 0);
    NVIC_EnableIRQ(SVCall_IRQn);
}


void lld_sleep_init_func(void)
{
    uint16_t delay;
    uint32_t twirq_set_value;
    
    if (lp_clk_sel == LP_CLK_RCX20)
        twirq_set_value = XTAL_TRIMMING_TIME_RCX;
    else // if (lp_clk_sel == LP_CLK_XTAL32)
        twirq_set_value = XTAL_TRIMMING_TIME;
    
    // Clear the environment
    memset(&lld_sleep_env, 0, sizeof(lld_sleep_env));

    // Actual "delay" is application specific and is the execution time of the BLE_WAKEUP_LP_Handler(), which depends on XTAL trimming delay,
    // plus the time taken for the OTP copy. Time unit of delay is usec.
    delay = lld_sleep_lpcycles_2_us_func(twirq_set_value);
    // Icrease time taking in to account the time from the setting of BLE_DEEP_SLEEP_ON_BIT until the execution of WFI.    
    delay += 200;     
    rwip_wakeup_delay_set(delay);

    // Enable external wake-up by default
    ble_extwkupdsb_setf(0);
}

/**
 ****************************************************************************************
 * @brief otp_prepare()
 *
 * About: Prepare OTP Controller in order to be able to reload SysRAM at the next power-up
 ****************************************************************************************
 */
static __inline void  otp_prepare(uint32 code_size)
{
    // Enable OPTC clock in order to have access
    SetBits16 (CLK_AMBA_REG, OTP_ENABLE, 1);

    // Wait a little bit to start the OTP clock...
    for(uint8 i=0;i<10;i++); //change this later to a defined time  

    SetBits16(SYS_CTRL_REG, OTP_COPY, 1);

    // Copy the size of software from the first word of the retaintion mem.
    SetWord32 (OTPC_NWORDS_REG, code_size - 1);

    // And close the OPTC clock to save power
    SetBits16 (CLK_AMBA_REG, OTP_ENABLE, 0);
}


/*
 * EXPORTED FUNCTION DEFINITIONS
 ****************************************************************************************
 */


/*
 * MAIN FUNCTION
 ****************************************************************************************
 */
extern void set_system_clocks(void);
extern void init_pwr_and_clk_ble(void);
extern void rf_workaround_init(void);

/**
 ****************************************************************************************
 * @brief BLE main function.
 *
 * This function is called right after the booting process has completed.
 ****************************************************************************************
 */
int main_func(void) __attribute__((noreturn));

int main_func(void)
{
	volatile unsigned i;
    sleep_mode_t sleep_mode; // keep at system RAM. On each while loop it will get a new value. 
    
    sys_startup_flag = true;
 
    /*
     ************************************************************************************
     * Platform initialization
     ************************************************************************************
     */
#if (USE_WDOG)
    SetWord16(WATCHDOG_REG, 0xC8);          // 200 * 10.24ms = ~2sec active time!
    SetWord16(WATCHDOG_CTRL_REG, 0);        // Generate an NMI when counter reaches 0 and a WDOG (SYS) Reset when it reaches -16!
                                            // WDOG can be frozen by SW!
    SetWord16(RESET_FREEZE_REG, FRZ_WDOG);  // Start WDOG
#else
    SetWord16(SET_FREEZE_REG, FRZ_WDOG);
#endif
    
    set_system_clocks();
    GPIO_init();
    periph_init();
    
          
    /* Don't remove next line otherwhise dummy[0] could be optimized away
     * The dummy array is intended to reserve the needed Exch.Memory space in retention memory
     */
    dummy[0] = dummy[0];
    descript[0] = descript[0];
#if (BLE_CONNECTION_MAX_USER > 4)
    cs_table[0] = cs_table[0];
#endif
                                         
    /* Don't remove next line otherwhise data__1 is optimized away.
     * The address 0x9010 is used by the ROM code (rand.o) and cannot be used by the 
     * application code!
     */
    //GZ data__1 = 0;
                                            
    // Initialize unloaded RAM area
    //unloaded_area_init();

    // Initialize random process
    srand(1);

    // Initialize the exchange memory interface, emi in RAM for the time being, so no init necessary
#if 0
    emi_init();
#endif

    // Initialize NVDS module
    nvds_init((uint8_t *)NVDS_FLASH_ADDRESS, NVDS_FLASH_SIZE);

    //check and read BDADDR from OTP
    nvds_read_bdaddr_from_otp();

#ifdef RADIO_580
    iq_trim_from_otp();
#endif

    /*
     ************************************************************************************
     * BLE initialization
     ************************************************************************************
     */
     
    init_pwr_and_clk_ble(); 
    //diagnostic();

  //  rf_init(&rwip_rf);
  //  SetBits32(BLE_RADIOCNTL1_REG, XRFSEL, 3);

#if UNCALIBRATED_AT_FAB
    SetBits16(BANDGAP_REG, BGR_TRIM, 0x0);  // trim RET Bandgap
    SetBits16(BANDGAP_REG, LDO_RET_TRIM, 0xA);  // trim RET LDO
    SetWord16(RF_LNA_CTRL1_REG, 0x24E);
    SetWord16(RF_LNA_CTRL2_REG, 0x26);
    SetWord16(RF_LNA_CTRL3_REG, 0x7);
    SetWord16(RF_RSSI_COMP_CTRL_REG, 0x7777);
    SetWord16(RF_VCO_CTRL_REG, 0x1);
    SetBits16(CLK_16M_REG,  RC16M_TRIM, 0xA);
#endif

    // Initialize BLE stack 
    NVIC_ClearPendingIRQ(BLE_SLP_IRQn);     
    NVIC_ClearPendingIRQ(BLE_EVENT_IRQn); 
    NVIC_ClearPendingIRQ(BLE_RF_DIAG_IRQn);
    NVIC_ClearPendingIRQ(BLE_RX_IRQn);
    NVIC_ClearPendingIRQ(BLE_CRYPT_IRQn);
    NVIC_ClearPendingIRQ(BLE_FINETGTIM_IRQn);	
    NVIC_ClearPendingIRQ(BLE_GROSSTGTIM_IRQn);	
    NVIC_ClearPendingIRQ(BLE_WAKEUP_LP_IRQn);     	
    rwip_init(error);
    
    /* Set spi to HW (Ble)
     * Necessary: So from this point the BLE HW can generate spi burst iso SW
     * SPI BURSTS are necessary for the radio TX and RX burst, done by hardware
     * beause of the accurate desired timing 
     */
    //FPGA
#ifdef FPGA_USED    
    SetBits32(BLE_CNTL2_REG,SW_RPL_SPI ,1);
#endif

    //Enable BLE core    
    SetBits32(BLE_RWBTLECNTL_REG,RWBLE_EN ,1); 

    
#if RW_BLE_SUPPORT && HCIC_ITF

    // If FW initializes due to FW reset, send the message to Host
    if(error != RESET_NO_ERROR)
    {
        rwble_send_message(error);
    }
#endif

    /*
     ************************************************************************************
     * Sleep mode initializations (especially for full embedded)
     ************************************************************************************
     */
#if (EXT_SLEEP_ENABLED)
     app_set_extended_sleep();
#elif (DEEP_SLEEP_ENABLED)
     app_set_deep_sleep();
#else
     app_disable_sleep();
#endif    
   
    if (lp_clk_sel == LP_CLK_RCX20)
    {    
        calibrate_rcx20(20);
        read_rcx_freq(20);  
    }
    
    /*
     ************************************************************************************
     * Application initializations
     ************************************************************************************
     */
     
#if (BLE_APP_PRESENT)    
    {
        app_init();         // Initialize APP
    }
#endif /* #if (BLE_APP_PRESENT) */

    
    /*
    ************************************************************************************
    * Main loop
    ************************************************************************************
    */
    lld_sleep_init_func();
    
    SetWord16(TRIM_CTRL_REG, 0xA2);
    SetBits16(CLK_16M_REG, XTAL16_CUR_SET, 0x5);
    
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
    
// Now enable the TX_EN/RX_EN interrupts, depending on the RF mode of operation (PLL-LUT and MGC_KMODALPHA combinations)
#if LUT_PATCH_ENABLED
    const volatile struct LUT_CFG_struct *pLUT_CFG;
    pLUT_CFG = (const volatile struct LUT_CFG_struct *)(jump_table_struct[lut_cfg_pos]);
    if (!pLUT_CFG->HW_LUT_MODE)
    {
        enable_rf_diag_irq(RF_DIAG_IRQ_MODE_RXTX); 
    }
    else
    {
#if MGCKMODA_PATCH_ENABLED
        enable_rf_diag_irq(RF_DIAG_IRQ_MODE_TXONLY);                           // This just enables the TX_EN int. RX_EN int enable status remains as it was
#endif //MGCKMODA_PATCH_ENABLED
    }
#else //LUT_PATCH_ENABLED
#if MGCKMODA_PATCH_ENABLED
    enable_rf_diag_irq(RF_DIAG_IRQ_MODE_TXONLY);                               // This just enables the TX_EN int. RX_EN int enable status remains as it was
#endif //MGCKMODA_PATCH_ENABLED
#endif //LUT_PATCH_ENABLED

#if BLE_APP_SPOTAR
    //app_spotar_exec_patch();
#endif

    if ( (app_get_sleep_mode() == 2) || (app_get_sleep_mode() == 1) )
    {
         SetWord16(SET_FREEZE_REG, FRZ_WDOG);            // Stop WDOG until debugger is removed
         if((GetWord16(SYS_STAT_REG) & DBG_IS_UP) == DBG_IS_UP) 
         SetBits16(SYS_CTRL_REG, DEBUGGER_ENABLE, 1);    // close debugger
    }	
	
    /*
     ************************************************************************************
     * Watchdog
     ************************************************************************************
     */
#if (USE_WDOG)
    SetWord16(WATCHDOG_REG, 0xC8);          // 200 * 10.24ms active time for initialization!
    SetWord16(RESET_FREEZE_REG, FRZ_WDOG);  // Start WDOG
#endif

    /*
     ************************************************************************************
     * Main loop
     ************************************************************************************
     */
    while(1)
    {   
		// schedule all pending events
		if(GetBits16(CLK_RADIO_REG, BLE_ENABLE) == 1) { // BLE clock is enabled
			if(GetBits32(BLE_DEEPSLCNTL_REG, DEEP_SLEEP_STAT) == 0 && !(rwip_prevent_sleep_get() & RW_WAKE_UP_ONGOING)) { // BLE is running
#ifndef FPGA_USED            
                uint8_t ble_evt_end_set = ke_event_get(KE_EVENT_BLE_EVT_END); // BLE event end is set. conditional RF calibration can run.
#endif                
                rwip_schedule();	
#if (ROLE_MASTER_YCOM)
				         
				         if(existence_flag==1)
								 {
									//GPIO_ConfigurePin( GPIO_PORT_1, GPIO_PIN_1, OUTPUT, PID_GPIO, true ); //Alert LED
								 }
								 else
								 {
									 ;
								 }
				        // app_connect(0);
								//app_connect(9);
				
				
#endif
#ifndef FPGA_USED            
   
                if (ble_evt_end_set)
                {
                    uint32_t sleep_duration = 0;
                    if (lp_clk_sel == LP_CLK_RCX20)
                        read_rcx_freq(20);
                    if (lld_sleep_check(&sleep_duration, 4)) //6 slots -> 3.750 ms
                        conditionally_run_radio_cals(); // check time and temperature to run radio calibrations. 
                }

#endif
                
#if (BLE_APP_PRESENT)
				if ( app_asynch_trm() )
					continue; // so that rwip_schedule() is called again
#endif
                
			}	
		} 
        
#if (BLE_APP_PRESENT)
		// asynchronous events processing
		if (app_asynch_proc())
			continue; // so that rwip_schedule() is called again
#endif

		GLOBAL_INT_STOP();

#if (BLE_APP_PRESENT)
        app_asynch_sleep_proc();
#endif        
        
		// if app has turned sleep off, rwip_sleep() will act accordingly
		// time from rwip_sleep() to WFI() must be kept as short as possible!
		sleep_mode = rwip_sleep();

		// BLE is sleeping ==> app defines the mode
		if (sleep_mode == mode_sleeping) {
			if (sleep_env.slp_state == ARCH_EXT_SLEEP_ON) {
                sleep_mode = mode_ext_sleep;
            } else {
                sleep_mode = mode_deep_sleep;
            }
        }

		if (sleep_mode == mode_ext_sleep || sleep_mode == mode_deep_sleep) 
        {
			SetBits16(PMU_CTRL_REG, RADIO_SLEEP, 1); // turn off radio
            
            if (jump_table_struct[nb_links_user] > 1)
            {
                if( (sleep_mode == mode_deep_sleep) && func_check_mem() && test_rxdone() && ke_mem_is_empty(KE_MEM_NON_RETENTION) )
                {	
                    func_check_mem_flag = 2;//true;
                }
                else
                    sleep_mode = mode_ext_sleep;
            }	
            else
            {
                if( (sleep_mode == mode_deep_sleep) && ke_mem_is_empty(KE_MEM_NON_RETENTION) )
                {	
                    func_check_mem_flag = 1;//true;
                }
                else
                    sleep_mode = mode_ext_sleep;
            }	
            
#if (BLE_APP_PRESENT)
			// hook for app specific tasks when preparing sleeping
			app_sleep_prepare_proc(&sleep_mode);
#endif
            
			
            if (sleep_mode == mode_ext_sleep || sleep_mode == mode_deep_sleep)
            {
                SCB->SCR |= 1<<2; // enable sleepdeep mode bit in System Control Register (SCR[2]=SLEEPDEEP)
                
                SetBits16(SYS_CTRL_REG, PAD_LATCH_EN, 0);           // activate PAD latches
                SetBits16(PMU_CTRL_REG, PERIPH_SLEEP, 1);           // turn off peripheral power domain
                if (sleep_mode == mode_ext_sleep)	{
                    SetBits16(SYS_CTRL_REG, RET_SYSRAM, 1);         // retain System RAM
                    SetBits16(SYS_CTRL_REG, OTP_COPY, 0);           // disable OTP copy	  
                } else { // mode_deep_sleep
#if DEVELOPMENT__NO_OTP
                    SetBits16(SYS_CTRL_REG, RET_SYSRAM, 1);         // retain System RAM		
#else
                    SetBits16(SYS_CTRL_REG, RET_SYSRAM, 0);         // turn System RAM off => all data will be lost!
#endif
                    otp_prepare(0x1FC0);                            // this is 0x1FC0 32 bits words, so 0x7F00 bytes 
                }
            }

            SetBits16(CLK_16M_REG, XTAL16_BIAS_SH_DISABLE, 0);

            
#if (BLE_APP_PRESENT)
            // hook for app specific tasks just before sleeping
            app_sleep_entry_proc(&sleep_mode);
#endif
            
			WFI();

#if (BLE_APP_PRESENT)            
			// hook for app specific tasks just after waking up
			app_sleep_exit_proc(sleep_mode);
#endif

			// reset SCR[2]=SLEEPDEEP bit else the mode=idle WFI will cause a deep sleep 
			// instead of a processor halt
			SCB->SCR &= ~(1<<2);				
		}
		else if (sleep_mode == mode_idle) {

#if (!BLE_APP_PRESENT)              
            if (check_gtl_state())
            {
#endif
                WFI();
                
#if (!BLE_APP_PRESENT)                              
            }
#endif
               
		}
		
		// restore interrupts
		GLOBAL_INT_START();
  
#if (USE_WDOG)        
        SetWord16(WATCHDOG_REG, 0xC8);          // Reset WDOG! 200 * 10.24ms active time for normal mode!
#endif
    }
}

/// @} DRIVERS
