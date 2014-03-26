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
#include <stddef.h>    // standard definitions
#include <stdint.h>    // standard integer definition
#include <stdbool.h>   // boolean definition
#include "boot.h"      // boot definition
#include "rwip.h"     // BLE initialization
#include "syscntl.h"   // System control initialization
#include "emi.h"       // EMI initialization
#include "intc.h"      // Interrupt initialization
#include "timer.h"     // TIMER initialization
#include "em_map_ble.h"
#include "ke_mem.h"
#include "ke_event.h"
#include "smpc.h"
#include "llc.h"
#include "pll_vcocal_lut.h"

#if PLF_UART
#include "uart.h"      // UART initialization
#endif //PLF_UART
#include "nvds.h"      // NVDS initialization
#include "flash.h"     // Flash initialization
#include "led.h"       // Led initialization
#if (BLE_EMB_PRESENT)
#include "rf.h"        // RF initialization
#endif // BLE_EMB_PRESENT

#if (BLE_HOST_PRESENT)
#include "rwble_hl.h"        // BLE HL definitions
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
#include "app_utils.h"
# if (BLE_HID_DEVICE)
# include "app_keyboard.h"
# endif // BLE_HID_DEVICE
#endif // BLE_APP_PRESENT

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

#if BLE_PROX_REPORTER
 #include "app_proxr.h"
#endif

#if BLE_SPOTA_RECEIVER
#include "app_spotar.h"
#endif
#if BLE_BATT_SERVER
#include "app_batt.h"
#endif

/**
 * @addtogroup DRIVERS
 * @{
 */

/*
 * DEFINES
 ****************************************************************************************
 */
/// NVDS location in FLASH : 0x000E0000 (896KB (1Mo - 128KB))
#define NVDS_FLASH_ADDRESS          (0x00000270)

/// NVDS size in RAM : 0x00010000 (128KB)
#define NVDS_FLASH_SIZE             (0x00000200)

#if BLE_APP_PRESENT
# if !defined(CFG_PRINTF)
#  undef PROGRAM_ENABLE_UART
# else
#  define PROGRAM_ENABLE_UART
# endif
#endif // BLE_APP_PRESENT

#ifdef PROGRAM_ENABLE_UART
#include "uart.h"      // UART initialization
#endif

#if DEVELOPMENT__NO_OTP
    #warning "==============================================================> DEVELOPMENT__NO_OTP is set!"
#endif		


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

uint32_t last_temp_time __attribute__((section("exchange_mem_case1"))) = 0 ; /// time of last temperature count measurement
extern uint16_t last_temp_count; /// temperature counter

#if PLF_DEBUG
/// Variable to enable infinite loop on assert
volatile int dbg_assert_block = 1;
#endif //PLF_DEBUG

/// Pointer to access unloaded RAM area
extern struct unloaded_area_tag* unloaded_area;

/// Variable storing the reason of platform reset
extern uint32_t error;// = RESET_NO_ERROR;

/// Reserve space for Exchange Memory, this section is linked first in the section "exchange_mem_case"
extern volatile uint8 dummy[DUMMY_SIZE]     __attribute__((section("exchange_mem_case1")));
volatile uint8 descript[EM_SYSMEM_SIZE]  __attribute__((at(EM_SYSMEM_START))); //CASE_15_OFFSET

extern bool func_check_mem_flag     __attribute__((section("exchange_mem_case1")));
extern struct lld_sleep_env_tag  lld_sleep_env;

extern struct arch_sleep_env_tag sleep_env;
extern uint32_t lld_sleep_lpcycles_2_us_func(uint32_t);
bool sys_startup_flag      __attribute__((section("exchange_mem_case1")));
/*
 * LOCAL FUNCTION DECLARATIONS
 ****************************************************************************************
 */
extern void lld_con_update_ind(struct lld_evt_tag *old_evt,
                        struct llcp_con_up_req const *param_pdu);

extern void lld_ch_map_ind(struct lld_evt_tag *evt, uint16_t instant);


struct lld_evt_tag *lld_con_start(struct llm_le_create_con_cmd const *con_par,
                                  struct co_buf_tx_node *con_req_pdu,
                                  uint16_t conhdl);

extern void lld_data_tx_push(struct lld_evt_tag *evt, struct co_buf_tx_node *txnode);

extern void prf_cleanup(uint8_t conidx, uint16_t conhdl, uint8_t reason);

extern void llm_con_req_ind(struct co_buf_rx_desc *rxdesc);

extern void lld_evt_insert(struct lld_evt_tag *evt);

#if (BLE_APP_SLAVE)
extern void rom_smpc_handle_enc_change_evt(uint8_t conidx, uint8_t role, uint8_t status);

void smpc_handle_enc_change_evt(uint8_t conidx, uint8_t role, uint8_t status)
{
    if(status == CO_ERROR_NO_ERROR)
    {

        // Encrypted link
        GAPC_SET_FIELD(conidx, ENCRYPTED, true);

        if(smpc_env[conidx]->pair_info != NULL)
        {
            // set authentication information
            GAPC_SET_FIELD(conidx, AUTH, smpc_env[conidx]->pair_info->auth);
        }
    }
        
    //remove patch of smpc_handle_enc_change_evt
    SetWord32(PATCH_ADDR4_REG, 0x0);
    SetWord32(PATCH_DATA4_REG, 0x8600); //do nothing
    rom_smpc_handle_enc_change_evt(conidx, role, status);
    //0x0002b464 smpc_handle_enc_change_evt
    SetWord32(PATCH_ADDR4_REG, 0x0002b464);
    SetWord32(PATCH_DATA4_REG, 0x4607df04); //smpc_handle_enc_change_evt svc 2
}
#endif



const uint32_t * const patch_table[]={

	(const uint32_t *) lld_evt_restart,
	(const uint32_t *) lld_data_tx_push,
	(const uint32_t *) lld_con_update_ind,
	(const uint32_t *) lld_ch_map_ind,
#if (BLE_APP_SLAVE)
    (const uint32_t *) smpc_handle_enc_change_evt,
    (const uint32_t *) lld_evt_insert,
#else
	(const uint32_t *) lld_evt_move_to_master,
    (const uint32_t *) lld_con_start,
#endif	
	(const uint32_t *) prf_cleanup,
#if (BLE_APP_SLAVE)    
	(const uint32_t *) llm_con_req_ind,
#else
    (const uint32_t *) lld_evt_insert,
#endif    

};


const volatile struct LUT_CFG_struct LUT_CFG =
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



// SVC handler - main code to handle processing
// Input parameter is stack frame starting address
// obtained from assembly wrapper.

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


void patch_func (void)
{
    //0x000252a8 lld_evt_restart
    SetWord32(PATCH_ADDR0_REG, 0x000252a8);
    SetWord32(PATCH_DATA0_REG, 0xdf00b662); //lld_evt_restart svc 0

    //0x00024bea	lld_data_tx_push
    SetWord32(PATCH_ADDR1_REG, 0x00024be8);
    SetWord32(PATCH_DATA1_REG, 0xdf014770); //lld_data_tx_push svc 1

    //0x000247ec lld_con_update_ind
    SetWord32(PATCH_ADDR2_REG, 0x000247ec);
    SetWord32(PATCH_DATA2_REG, 0xb510df02); //lld_con_update_ind svc 2

    //0x000247e4 lld_ch_map_ind
    SetWord32(PATCH_ADDR3_REG, 0x000247e4);
    SetWord32(PATCH_DATA3_REG, 0xf001df03); //lld_ch_map_ind svc 3

#if (BLE_APP_SLAVE)
    //0x0002b464 smpc_handle_enc_change_evt
    SetWord32(PATCH_ADDR4_REG, 0x0002b464);
    SetWord32(PATCH_DATA4_REG, 0x4607df04); //smpc_handle_enc_change_evt svc 4
    
    //0x00024f04 lld_evt_insert
    SetWord32(PATCH_ADDR5_REG, 0x00024f04);
    SetWord32(PATCH_DATA5_REG, 0x4fcfdf05); //lld_evt_insert svc 5
#else
    //0x00025884 lld_evt_move_to_master
    SetWord32(PATCH_ADDR4_REG, 0x00025884);
    SetWord32(PATCH_DATA4_REG, 0x4602df04); //lld_evt_move_to_master svc 4
    
    //0x000244a6 lld_con_start
    SetWord32(PATCH_ADDR5_REG, 0x000244a4);
    SetWord32(PATCH_DATA5_REG, 0xdf05bd10); //lld_con_start svc 5
#endif    

    //0x0003188a prf_cleanup
    SetWord32(PATCH_ADDR6_REG, 0x00031888);
    SetWord32(PATCH_DATA6_REG, 0xdf064770); //prf_cleanup svc 6
#if (BLE_APP_SLAVE)
    //0x000260a0 llm_con_req_ind
    SetWord32(PATCH_ADDR7_REG, 0x000260a0);
    SetWord32(PATCH_DATA7_REG, 0xb087df07); //llm_con_req_ind svc 7
#else
    //0x00024f04 lld_evt_insert
    SetWord32(PATCH_ADDR7_REG, 0x00024f04);
    SetWord32(PATCH_DATA7_REG, 0x4fcfdf07); //lld_evt_insert svc 7

#endif    

    NVIC_DisableIRQ(SVCall_IRQn);
    NVIC_SetPriority(SVCall_IRQn, 0);
    NVIC_EnableIRQ(SVCall_IRQn);
}


/**
 ****************************************************************************************
 * @brief Initialisation of ble core, pwr and clk
 *
 * The Hclk and Pclk are set
 ****************************************************************************************
 */
void init_pwr_and_clk_ble(void) 
{
    SetBits16(CLK_RADIO_REG, BLE_DIV, 0);
    SetBits16(CLK_RADIO_REG, BLE_ENABLE, 1); 
    SetBits16(CLK_RADIO_REG, RFCU_DIV, 1);
    SetBits16(CLK_RADIO_REG, RFCU_ENABLE, 1);

    SetBits16(CLK_32K_REG,  XTAL32K_ENABLE, 1);	// Enable XTAL32KHz 
    SetBits16(SYS_CTRL_REG, CLK32_SOURCE,   1);	// Select XTAL32K (not RC32K) as LP clock
#if ES4_CODE
    SetBits16(CLK_32K_REG,  XTAL32K_DISABLE_AMPREG,   1); 
    SetBits16(CLK_32K_REG,  XTAL32K_CUR,    1);
#endif
    SetBits16(CLK_32K_REG,  RC32K_ENABLE,   0);	// and disable RC32KHz

    /* 
     * Power up BLE core & reset BLE Timers
     */
    SetBits16(CLK_RADIO_REG, BLE_LP_RESET, 1);       // Apply HW reset to BLE_Timers
    SetBits16(PMU_CTRL_REG, RADIO_SLEEP,0) ;
    while (!(GetWord16(SYS_STAT_REG) & RAD_IS_UP)) ; // Just wait for radio to truely wake up

    SetBits16(CLK_RADIO_REG, BLE_LP_RESET, 0);

    /* 
     * Just make sure that BLE core is stopped (if already running)
     */
    SetBits32(BLE_RWBTLECNTL_REG, RWBLE_EN, 0); 


    /* 
     * Since BLE is stopped (and powered), set CLK_SEL
     */    
    SetBits32(BLE_CNTL2_REG, BLE_CLK_SEL, 16);
    SetBits32(BLE_CNTL2_REG, BLE_RSSI_SEL, 1);    

    /* 
     * Set spi interface to software
     */   
    // the following 2 lines are for FPGA implementation
    //SetBits32(BLE_CNTL2_REG, SW_RPL_SPI ,0);
    //SetBits32(BLE_CNTL2_REG, BB_ONLY,1);     
}


/**
 ****************************************************************************************
 * @brief Map port pins
 *
 * The Uart and SPI port pins and GPIO ports(for debugging) are mapped
 ****************************************************************************************
 */
void set_pad_functions(void)        // set gpio port function mode
{
#ifdef PROGRAM_ENABLE_UART
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_4, OUTPUT, PID_UART1_TX, false );
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_5, INPUT, PID_UART1_RX, false );    
# ifdef PROGRAM_ALTERNATE_UART_PINS
#if !(BLE_APP_PRESENT) 
#if DISABLE_UART_RTS_CTS    
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_7, OUTPUT, PID_UART1_RTSN, false ); //CD SOS
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_6, INPUT_PULLDOWN, PID_UART1_CTSN, false ); //CD SOS
#else
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_7, OUTPUT, PID_UART1_RTSN, false );//CD SOS
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_6, INPUT, PID_UART1_CTSN, false );//CD SOS
#endif // DISABLE_UART_RTS_CTS
#endif // !BLE_APP_PRESENT  
# else //PROGRAM_ALTERNATE_UART_PINS
#if !(BLE_APP_PRESENT) 
#if DISABLE_UART_RTS_CTS    
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_3, OUTPUT, PID_UART1_RTSN, false );
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_2, INPUT_PULLDOWN, PID_UART1_CTSN, false );
#else
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_3, OUTPUT, PID_UART1_RTSN, false );
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_2, INPUT, PID_UART1_CTSN, false );
#endif // DISABLE_UART_RTS_CTS
#endif // !BLE_APP_PRESENT
# endif // PROGRAM_ALTERNATE_UART_PINS

#endif // PROGRAM_ENABLE_UART

#if BLE_ACCEL
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_6, OUTPUT, PID_SPI_EN, true );
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_0, OUTPUT, PID_SPI_CLK, false );
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_3, OUTPUT, PID_SPI_DO, false );	
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_5, INPUT, PID_SPI_DI, false );
#if DEEP_SLEEP_ENABLED 	//GZ int
    //Set P1_5 to ACCEL's INT1
    GPIO_ConfigurePin( GPIO_PORT_1, GPIO_PIN_5, INPUT, PID_GPIO, false );
#else
	//Set P0_7 to ACCEL's INT1
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_7, INPUT, PID_GPIO, false );
#endif
#endif // BLE_ACCEL
}


/**
 ****************************************************************************************
 * @brief Enable pad's and peripheral clocks assuming that peripherals' power domain is down.
 *
 * The Uart and SPi clocks are set. 
 ****************************************************************************************
 */
void periph_init(void)  // set i2c, spi, uart, uart2 serial clks
{
	// Power up peripherals' power domain
    SetBits16(PMU_CTRL_REG, PERIPH_SLEEP, 0);
    while (!(GetWord16(SYS_STAT_REG) & PER_IS_UP)) ; 
    
#if ES4_CODE
    SetBits16(CLK_16M_REG,XTAL16_BIAS_SH_DISABLE, 1);
#endif    
	
	// TODO: Application specific - Modify accordingly!
	// Example: Activate UART and SPI.
	
    // Initialize UART component
#ifdef PROGRAM_ENABLE_UART
    SetBits16(CLK_PER_REG, UART1_ENABLE, 1);    // enable clock - always @16MHz
	
    // baudr=9-> 115k2
    // mode=3-> no parity, 1 stop bit 8 data length
#ifdef UART_MEGABIT
    uart_init(UART_BAUDRATE_1M, 3);
#else
    uart_init(UART_BAUDRATE_115K2, 3);
#endif // UART_MEGABIT
    NVIC_SetPriority(UART_IRQn, 1);             // remove if the bug in uart.c is fixed in ES4(b)!
#endif // PROGRAM_ENABLE_UART

    //FPGA  
//    SetBits16(CLK_PER_REG, SPI_ENABLE, 1);    // enable  clock
//    SetBits16(CLK_PER_REG, SPI_DIV, 1);	    // set divider to 1	

#if BLE_ACCEL
    SetBits16(CLK_PER_REG, SPI_ENABLE, 1);      // enable  clock
    SetBits16(CLK_PER_REG, SPI_DIV, 1);	        // set divider to 1	
	SetBits16(CLK_PER_REG, WAKEUPCT_ENABLE, 1); // enable clock of Wakeup Controller
#endif

#if 0
	//Example: Do something with the timer if need be...
    SetWord16(TIMER0_CTRL_REG, 0); 
    SetWord16(TIMER0_RELOAD_M_REG, 0);
    SetWord16(TIMER0_RELOAD_N_REG, 0);
    SetWord16(TIMER0_ON_REG, 0);
#endif
	
	//rom patch
	patch_func();
	
	//Init pads
	set_pad_functions();

#if (BLE_APP_PRESENT)
#if BLE_HID_DEVICE
	SetBits16(CLK_PER_REG, WAKEUPCT_ENABLE, 1); // enable clock of Wakeup Controller
//	if (app_env.app_flags & KBD_START_SCAN) // reinit kbd only if needed
	if ( !(app_env.app_flags & KBD_START_SCAN) && !(app_env.app_flags & KBD_SCANNING) )
		app_kbd_reinit_matrix();
#endif  // BLE_HID_DEVICE
    
#if BLE_PROX_REPORTER
    app_proxr_port_reinit();
#elif BLE_FINDME_LOCATOR
    app_button_enable();
#endif //BLE_PROX_REPORTER

#if BLE_SPOTA_RECEIVER
    app_spotar_init();
#endif // BLE_SPOTA_RECEIVER
#if BLE_STREAMDATA_DEVICE
    stream_start_button_init();
#endif  //BLE_STREAMDATA_DEVICE
#if BLE_BATTERY_SERVER
    app_batt_port_reinit();
#endif //BLE_BATTERY_SERVER
#endif 
    // Enable the pads
	SetBits16(SYS_CTRL_REG, PAD_LATCH_EN, 1);
}

#define NO_MORE_RF_WORKAROUND
void rf_workaround_init(void)
{
}


void lld_sleep_init_func(void)
{
    uint16_t delay;
    
    // Clear the environment
    memset(&lld_sleep_env, 0, sizeof(lld_sleep_env));

    // Actual "delay" is application specific and is the execution time of the BLE_WAKEUP_LP_Handler(), which depends on XTAL trimming delay,
    // plus the time taken for the OTP copy. Time unit of delay is usec.
    delay = lld_sleep_lpcycles_2_us_func(TWIRQ_SET_VALUE);
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
    //SetBits16 (CLK_AMBA_REG, OTP_ENABLE, 0);
}


/*
 * EXPORTED FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void set_pxact_gpio(void)
{
 uint32_t i;
    
 SetWord16(P13_MODE_REG, PID_GPIO|OUTPUT);
 SetWord16(P1_SET_DATA_REG, 0x8);
 for ( i=0;i<150;i++); //20 is almost 7.6usec of time.
 SetWord16(P1_RESET_DATA_REG, 0x8);

}

void conditionally_run_radio_cals(void) {
    
    
    uint16_t count, count_diff;
    uint8_t force_rf_cal = 0;
    bool rf_cal_stat;
    
    uint32_t current_time = lld_evt_time_get();    
    
    if (current_time < last_temp_time)
    { 
        last_temp_time = 0;
    }

    if (force_rf_cal)
    {
        
        force_rf_cal = 0;
        
        last_temp_time = current_time;
        last_temp_count = get_rc16m_count();
#if LUT_PATCH_ENABLED
        pll_vcocal_LUT_InitUpdate(LUT_UPDATE);    //Update pll look up table
#endif          
        rf_cal_stat = rf_calibration();
        if ( rf_cal_stat== false)
            force_rf_cal = 1;        

        return;
    }
    
    if ( (current_time - last_temp_time) >= 3200) //2 sec
    {    
        
        
        last_temp_time = current_time;
        count = get_rc16m_count();                  // Estimate the RC16M frequency
        
        if (count > last_temp_count)
            count_diff = count - last_temp_count;
        else
            count_diff = last_temp_count - count ;
        
        if (count_diff >= 24)// If corresponds to 5 C degrees difference
        { 

            // Update the value of last_count
            last_temp_count = count;
#if LUT_PATCH_ENABLED
             pll_vcocal_LUT_InitUpdate(LUT_UPDATE);    //Update pll look up table
#endif            
            rf_cal_stat = rf_calibration();
            if ( rf_cal_stat== false)
                force_rf_cal = 1;         // Perform the readio calibrations

        }
        
    }
    
}

/*
 * MAIN FUNCTION
 ****************************************************************************************
 */
extern void set_system_clocks(void);
extern void periph_init(void);
extern void init_pwr_and_clk_ble(void);
extern void rf_workaround_init(void);
extern void ke_task_init_ram(void);

/**
 ****************************************************************************************
 * @brief BLE main function.
 *
 * This function is called right after the booting process has completed.
 ****************************************************************************************
 */
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
    SetWord16(SET_FREEZE_REG,FRZ_WDOG);
#endif
    
    set_system_clocks();
    GPIO_init();
    periph_init();
    

#if !(ES4_CODE)
    // spike filter bypassing workaround
    SetBits16(CLK_16M_REG, RC16M_ENABLE, 1);   // not sure if I have the bitfield name OK.
    __nop();                                   // wait for RC16 to come up, is really fast.
    SetBits16(CLK_CTRL_REG, SYS_CLK_SEL, 0x1); // select RC16M
    __nop();                                   // Wait until switched, takes some time
    __nop();                                   //2 nops seem to do the trick. Double them to be sure (depends on PCLK_DIV setting)
    __nop();
    __nop();
    SetBits16(CLK_CTRL_REG, XTAL16M_SPIKE_FLT_DISABLE, 0x1); //Bypass the spike filter
    for (i=0; i<100; i++) {};          // have to wait some time for XTAL to be stable again, don't know the time...
    SetBits16(CLK_CTRL_REG, SYS_CLK_SEL, 0x0);  // Switch back to the 16MHz xtal 
    SetBits16(CLK_16M_REG, RC16M_ENABLE, 0);    // Switch off RC16M
#endif
    
    
        
    /* Don't remove next line otherwhise dummy[0] could be optimized away
     * The dummy array is intended to reserve the needed Exch.Memory space in retention memory
     */
    dummy[0]=dummy[0];
    descript[0] = descript[0];
                                                                                                                      
    // Initialize unloaded RAM area
    //unloaded_area_init();

    // Initialize random process
    srand(1);

    // Initialize the exchange memory interface->@WIK, emi in RAM for the time being, so no init necessary
#if 0
    emi_init();
#endif

    // Initialize NVDS module
    //@WIK, empty definition needed rwble init
    nvds_init((uint8_t *)NVDS_FLASH_ADDRESS, NVDS_FLASH_SIZE);

    //check and read BDADDR from OTP
    nvds_read_bdaddr_from_otp();

    /*
     ************************************************************************************
     * BLE initialization
     ************************************************************************************
     */
     
    init_pwr_and_clk_ble(); 
    //diagnostic();

  //  rf_init(&rwip_rf);
  //  SetBits32(BLE_RADIOCNTL1_REG, XRFSEL, 3);

#ifdef UNCALIBRATED_AT_FAB
//    SetWord32(BANDGAP_REG, 0x3C00);
    SetBits16(BANDGAP_REG, BGR_TRIM, 0x0);  // trim RET Bandgap
#if ES4_CODE
    SetBits16(BANDGAP_REG, LDO_RET_TRIM, 0xF);  // trim RET LDO
#else
    SetBits16(BANDGAP_REG, LDO_RET_TRIM, 0x7);  // trim RET LDO
#endif
    SetBits16(PMU_CTRL_REG, RETENTION_MODE, 0xf);
//    SetWord16(BANDGAP_REG, 0x0F);
//SetBits16(BANDGAP_REG,LDO_RET_TRIM,0xA);  // trim RET LDO
    SetWord16(RF_LNA_CTRL1_REG, 0x24E);
    SetWord16(RF_LNA_CTRL2_REG, 0x26);
    SetWord16(RF_LNA_CTRL3_REG, 0x7);
    SetWord16(RF_REF_OSC_REG, 0x29AC); 
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
//  SetBits32(BLE_CNTL2_REG,SW_RPL_SPI ,1);
     
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

    
    /*
     ************************************************************************************
     * Application initializations
     ************************************************************************************
     */
     
    ke_task_init_ram();     // Initialize Kernel RAM environment
    
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

// Now enable the TX_EN/RX_EN interrupts, depending on the RF mode of operation (PLL-LUT and MGC_KMODALPHA combinations)
#if LUT_PATCH_ENABLED
    const volatile struct LUT_CFG_struct *pLUT_CFG;
    pLUT_CFG= (const volatile struct LUT_CFG_struct *)(jump_table_struct[lut_cfg_pos]);
    if (!pLUT_CFG->HW_LUT_MODE)
    {
        enable_rf_diag_irq(RF_DIAG_IRQ_MODE_RXTX); 
    }
    else
    {
#if MGCKMODA_PATCH_ENABLED
        enable_rf_diag_irq(RF_DIAG_IRQ_MODE_TXONLY);                               // This just enables the TX_EN int. RX_EN int enable status remains as it was
#endif //MGCKMODA_PATCH_ENABLED
    }
#else //LUT_PATCH_ENABLED
#if MGCKMODA_PATCH_ENABLED
    enable_rf_diag_irq(RF_DIAG_IRQ_MODE_TXONLY);                               // This just enables the TX_EN int. RX_EN int enable status remains as it was
#endif //MGCKMODA_PATCH_ENABLED
#endif //LUT_PATCH_ENABLED

#if BLE_SPOTA_RECEIVER
    app_spotar_exec_patch();
#endif

#if 1 
    if ( (app_get_sleep_mode() == 2) || (app_get_sleep_mode() == 1) )
    {
        SetWord16(SET_FREEZE_REG, FRZ_WDOG);            // Stop WDOG until debugger is removed
        while ((GetWord16(SYS_STAT_REG) & DBG_IS_UP) == DBG_IS_UP) {}; 
        SetBits16(SYS_CTRL_REG, DEBUGGER_ENABLE, 0);  // close debugger
    }
#endif	
	
#if (BLE_APP_KEYBOARD_TESTER)
    arch_puts("\r\n*** Keyboard Tester application ***\r\n");
#endif
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
#if ES4_CODE
                uint8_t ble_evt_end_set = ke_event_get(KE_EVENT_BLE_EVT_END); // BLE event end is set. conditional RF calibration can run.
#endif                
                rwip_schedule();  
            
#if ES4_CODE   
                if (ble_evt_end_set)
                {
                    uint32_t sleep_duration = 0;
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

            SCB->SCR |= 1<<2; // enable sleepdeep mode bit in System Control Register (SCR[2]=SLEEPDEEP)

#if (BLE_APP_PRESENT)
			// hook for app specific tasks when preparing sleeping
			app_sleep_prepare_proc(&sleep_mode);
#endif
			
            if (sleep_mode == mode_ext_sleep || sleep_mode == mode_deep_sleep)
            {
                if (sleep_mode == mode_ext_sleep)	{
                    SetBits16(SYS_CTRL_REG, RET_SYSRAM, 1);         // retain System RAM
                    SetBits16(SYS_CTRL_REG, OTP_COPY, 0);           // disable OTP copy	  
                    SetBits16(SYS_CTRL_REG, PAD_LATCH_EN, 0);       // activate PAD latches
                    SetBits16(PMU_CTRL_REG, PERIPH_SLEEP, 1);       // turn off peripheral power domain
                } else { // mode_deep_sleep
                    SetBits16(SYS_CTRL_REG, PAD_LATCH_EN, 0);       // activate PAD latches
                    SetBits16(PMU_CTRL_REG, PERIPH_SLEEP, 1);       // turn off peripheral power domain 
#if  DEVELOPMENT__NO_OTP
                    SetBits16(SYS_CTRL_REG, RET_SYSRAM, 1);         // retain System RAM		
#else
                    SetBits16(SYS_CTRL_REG, RET_SYSRAM, 0);         // turn System RAM off => all data will be lost!
#endif
                    otp_prepare(0x1FC0);                            // this is 0x1FC0 32 bits words, so 0x7F00 bytes 
                }

#if !(ES4_CODE)                
                if (GetBits16(ANA_STATUS_REG, BOOST_SELECTED) == 0x1) { // Boost-mode
                    SetBits16(DCDC_CTRL2_REG, DCDC_CUR_LIM, 0xF);   // 150mA (will be lower due to auto-cal)
                    SetBits16(DCDC_CTRL2_REG, DCDC_AUTO_CAL, 0x7);  // ON during active = OFF during deep-sleep
                } else { // Buck-mode
                    SetWord16(DCDC_CAL1_REG, GetWord16(DCDC_CAL1_REG)); // Keep using the calibrated value
                    SetBits16(DCDC_CTRL2_REG, DCDC_AUTO_CAL, 0x0);  // OFF during active and deep-sleep
                }
#endif
            }

#if (ES4_CODE)
            SetBits16(CLK_16M_REG, XTAL16_BIAS_SH_DISABLE, 0);
#endif    

            
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
            WFI();
		}
		
		// restore interrupts
		GLOBAL_INT_START();
  
#if (USE_WDOG)        
        SetWord16(WATCHDOG_REG, 0xC8);          // Reset WDOG! 200 * 10.24ms active time for normal mode!
#endif
    }
}

/// @} DRIVERS
