/*
 * Copyright (C) 2013 Dialog Semiconductor GmbH and its Affiliates, unpublished work
 * This computer program includes Confidential, Proprietary Information and is a Trade Secret 
 * of Dialog Semiconductor GmbH and its Affiliates. All use, disclosure, and/or 
 * reproduction is prohibited unless authorized in writing. All Rights Reserved.
 */

#include "arch.h"
#include "app.h"
#include "gpio.h"
#include "ke_event.h"

#if BLE_HID_DEVICE 
#include "rwip_config.h"
#include "rwip.h"
#include "co_buf.h"
#include "app_keyboard.h"

#if BLE_ALT_PAIR
#include "../src/app_alt_pair.h"
#endif // BLE_ALT_PAIR

#if KEYBOARD_MEASURE_EXT_SLP   
#include "ke.h"
#endif // KEYBOARD_MEASURE_EXT_SLP

extern struct co_buf_env_tag co_buf_env;
#endif // BLE_HID_DEVICE

#if BLE_STREAMDATA_DEVICE
#include "app_stream.h"
#endif
#if BLE_PROX_REPORTER
#include "app_button_led.h"
#endif

#include "rf_580.h"
#include "customer_prod.h"
extern uint32_t last_temp_time; // time of last temperature count measurement
extern uint16_t last_temp_count; /// temperature counter
uint16_t temp_count_check_value; // value for checking if temp is changes more then 'x' degrees.

#include "gtl_env.h"
#include "gtl_task.h"

/*
 ********************************* Hooks ************************************
 */

/*
 * Name         : app_asynch_trm - Hook #1
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : See documentation.
 *
 * Returns      : true to force calling of schedule(), else false
 *
 */
static inline bool app_asynch_trm(void)
{
	bool ret = false;
    static int cnt2sleep=100;
    
	do {
//GZ        
        if(test_state==STATE_START_TX)
        {
            if(llm_le_env.evt == NULL)
            {
                test_state=STATE_IDLE;
                set_state_start_tx();
            }
        }
        
        //SetWord16(0x50002000, 0x181); test voor enno
		if((test_tx_packet_nr>=text_tx_nr_of_packets) &&  (test_state==STATE_START_TX) ) 
		{
			test_tx_packet_nr=0;
			struct msg_tx_packages_send_ready s;
			uint8_t* bufptr;
			s.packet_type	= HCI_EVT_MSG_TYPE;
			s.event_code 	= HCI_CMD_CMPL_EVT_CODE; 
			s.length     	= 3;
			s.param0		= 0x01;
			s.param_opcode	= HCI_TX_PACKAGES_SEND_READY;		
			bufptr = (uint8_t*)&s;							
			uart_write(bufptr,sizeof(struct msg_tx_packages_send_ready),NULL);								
			
			set_state_stop();
			//btw.state is STATE_IDLE after set_state_stop()				
		}	
        	
        rwip_schedule();  
		
		if( rf_calibration_request_cbt == 1)
		{
			last_temp_count = get_rc16m_count();
			temp_count_check_value = last_temp_count;			
			#ifdef LUT_PATCH_ENABLED
				pll_vcocal_LUT_InitUpdate(LUT_UPDATE);
			#endif			
			rf_calibration();
			rf_calibration_request_cbt = 0 ;
		}
	} while(0);
    
    if(app_get_sleep_mode() == 0)
    {
        ret = true;
        cnt2sleep = 100;
    }
    else
    {
        if(cnt2sleep-->0)
        {
            ret = true;
        }
    }
    
	return ret;
}

/*
 * Name         : app_asynch_proc - Hook #2
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : See documentation.
 *
 * Returns      : true to force calling of schedule(), else false
 *
 */
static inline bool app_asynch_proc(void)
{
	bool ret = false;


	return ret;
}

/*
 * Name         : app_asynch_sleep_proc - Hook #3
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : See documentation.
 *
 * Returns      : none
 *
 */
static inline void app_asynch_sleep_proc(void)
{
}

/*
 * Name         : app_sleep_prepare_proc - Hook #4
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : See documentation.
 *
 * Returns      : none
 *
 */
static inline void app_sleep_prepare_proc(sleep_mode_t *sleep_mode)
{
}

/*
 * Name         : app_sleep_entry_proc - Hook #5
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : See documentation.
 *
 * Returns      : none
 *
 */
static inline void app_sleep_entry_proc(sleep_mode_t *sleep_mode)
{
    if(*sleep_mode == mode_deep_sleep)
    {
        SetBits16(SYS_CTRL_REG, DEBUGGER_ENABLE, 0);    // close debugger
        SetBits16(SYS_CTRL_REG, RET_SYSRAM, 0);         // turn System RAM off => all data will be lost!
    }
}

/*
 * Name         : app_sleep_exit_proc - Hook #6
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : See documentation.
 *
 * Returns      : none
 *
 */
static inline void app_sleep_exit_proc(sleep_mode_t sleep_mode)
{
    app_disable_sleep();
}
