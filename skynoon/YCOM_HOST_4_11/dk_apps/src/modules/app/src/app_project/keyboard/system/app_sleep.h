/*
 * Copyright (C) 2013 Dialog Semiconductor GmbH and its Affiliates, unpublished work
 * This computer program includes Confidential, Proprietary Information and is a Trade Secret 
 * of Dialog Semiconductor GmbH and its Affiliates. All use, disclosure, and/or 
 * reproduction is prohibited unless authorized in writing. All Rights Reserved.
 */

#include "rwip_config.h"

#include "arch.h"
#include "app.h"
#include "gpio.h"
#include "co_buf.h"
#include "ke_event.h"
#include "ke.h"

#include "rwip.h"

#include "app_kbd.h"
#include "app_kbd_fsm.h"
#include "app_kbd_scan_fsm.h"
#include "app_kbd_leds.h"

#include "app_multi_bond.h"



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

	do {
        // Synchronize with the BLE here! The time window of requesting a packet trm at the upcoming
        // anchor point is from the CSCNT event until the FINEGTIM event. If you pass the FINEGTIM
        // event then the packet will be sent at the next anchor point!
        //
        // Note that this synchronization is only possible in sleep modes!
        if ( !ke_event_get(KE_EVENT_KE_MESSAGE) ) {
            // Since pkt reqs can be silently discarded if no Tx bufs are available, check first!
            if (kbd_trm_list && app_kbd_check_conn_status() && !co_list_is_empty(&co_buf_env.tx_free)) {
                bool overflow = false;
                
                if (kbd_free_list == NULL)  // free list depletion! keycode_buffer may have unread key events.
                    overflow = true;
                
                if (app_kbd_send_key_report()) {
                    // One HID report is removed from the trm list. Check if other HID reports are to be
                    // prepared because of unread data in the keycode_buffer now that the free list is not NULL.
                    if (overflow)
                        app_kbd_prepare_keyreports();
                    
                    ret = true;
                    break;
                }
            }
        }
        
        if (user_disconnection_req) {
            if (app_alt_pair_disconnect()) {
                if (HAS_KEYBOARD_LEDS)
                {
                    if (!(GetWord16(SYS_STAT_REG) & DBG_IS_UP)) {
                        leds_set_disconnected();
                    }
                }
                app_state_update(SWITCH_EVT);
            }

            user_disconnection_req = false;
        }
	} while(0);

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

	do {
		fsm_scan_update();
		
        if (kbd_trm_list) {
            // If BLE is sleeping, wake it up!
            GLOBAL_INT_DISABLE();
            if (GetBits16(CLK_RADIO_REG, BLE_ENABLE) == 0) { // BLE clock is off
                SetBits16(GP_CONTROL_REG, BLE_WAKEUP_REQ, 1);
            } else
                ret = true;
            GLOBAL_INT_RESTORE();
        }
 	} while(0);

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
    if (wkup_hit) { // make sure we do not miss any wake up interrupts!
        fsm_scan_update();
    }
   
    if (HAS_KEYBOARD_MEASURE_EXT_SLP)
    {
        if ( (user_extended_sleep) && (current_scan_state == KEY_SCAN_IDLE)) {
            // see if we can enter into sleep
            if (!ke_sleep_check())
                return;

            if (GetBits16(SYS_STAT_REG, RAD_IS_DOWN))
                return; // already sleeping, will wake up eventually. wait until this happens.
                
            if (rwip_prevent_sleep_get() != 0)
                return;
                
            rwip_rf.sleep();
            while ( !(GetWord32(BLE_CNTL2_REG) & RADIO_PWRDN_ALLOW) ) {};
            SetBits16(CLK_RADIO_REG, BLE_ENABLE, 0);
            SetBits16(PMU_CTRL_REG, RADIO_SLEEP, 1);        // turn off radio
            
            SCB->SCR |= 1<<2;
            SetBits16(SYS_CTRL_REG, RET_SYSRAM, 1);         // retain System RAM
            SetBits16(SYS_CTRL_REG, OTP_COPY, 0);           // disable OTP copy
            SetBits16(SYS_CTRL_REG, PAD_LATCH_EN, 0);       // activate PAD latches
            SetBits16(PMU_CTRL_REG, PERIPH_SLEEP, 1);       // turn off peripheral power domain
            
            SetBits16(CLK_16M_REG, XTAL16_BIAS_SH_DISABLE, 0);

            WFI();
        }
    } 
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
extern void set_pxact_gpio(void);

static inline void app_sleep_prepare_proc(sleep_mode_t *sleep_mode)
{
    if ( (current_scan_state == KEY_STATUS_UPD) || (current_scan_state == KEY_SCANNING) ) 
    {
        *sleep_mode = mode_idle;                // block power-off
    }
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
    if ( *sleep_mode == mode_idle ) 
    {
        /*
        * Use a lower clock to preserve power (i.e. 2MHz)
        */
        SetBits16(CLK_AMBA_REG, PCLK_DIV, 3);   // lowest is 2MHz (div 8, source is RC @16MHz)
        SetBits16(CLK_AMBA_REG, HCLK_DIV, 3);
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
    /*
     * Restore clock
     */
    SetBits16(CLK_AMBA_REG, PCLK_DIV, 0);  
    SetBits16(CLK_AMBA_REG, HCLK_DIV, 0);
}
