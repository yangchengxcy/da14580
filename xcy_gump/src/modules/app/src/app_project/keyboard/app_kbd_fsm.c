/**
 ****************************************************************************************
 *
 * @file app_kbd_fsm.c
 *
 * @brief HID Keyboard main FSM.
 *
 * Copyright (C) 2014. Dialog Semiconductor Ltd, unpublished work. This computer 
 * program includes Confidential, Proprietary Information and is a Trade Secret of 
 * Dialog Semiconductor Ltd.  All use, disclosure, and/or reproduction is prohibited 
 * unless authorized in writing. All Rights Reserved.
 *
 * <bluetooth.support@diasemi.com> and contributors.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */
 
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"             // SW configuration
#include "app.h" 
#include "app_task.h"
#include "app_sec.h"
#include "gap_cfg.h"
#include "app_console.h"
#include "arch_sleep.h"
#include "gpio.h"

#include "app_kbd.h"
#include "app_kbd_proj.h"
#include "app_kbd_key_matrix.h"
#include "app_kbd_leds.h"
#include "app_kbd_fsm.h"
#include "app_kbd_debug.h"
#include "app_multi_bond.h"


enum main_fsm_states current_fsm_state  __attribute__((section("retention_mem_area0"), zero_init));
enum adv_states current_adv_state       __attribute__((section("retention_mem_area0"), zero_init));
uint32_t adv_timeout                    __attribute__((section("retention_mem_area0"), zero_init));
bool is_bonded                          __attribute__((section("retention_mem_area0"), zero_init));
bool eeprom_is_read                     __attribute__((section("retention_mem_area0"), zero_init));
bool conn_upd_pending                   __attribute__((section("retention_mem_area0"), zero_init));

bool white_list_written = false;


static void start_adv_limited(void)
{
    // Allocate a message for GAP
    struct gapm_start_advertise_cmd *cmd = KE_MSG_ALLOC(GAPM_START_ADVERTISE_CMD,
                                                TASK_GAPM, TASK_APP,
                                                gapm_start_advertise_cmd);

    cmd->op.code = GAPM_ADV_UNDIRECT;
    cmd->op.addr_src = GAPM_PUBLIC_ADDR;
    cmd->channel_map = APP_ADV_CHMAP;

    switch(current_adv_state)
    {
    case SLOW_ADV:
        cmd->info.host.mode = GAP_GEN_DISCOVERABLE;
        cmd->intv_min = SLOW_BONDED_ADV_INT_MIN;
        cmd->intv_max = SLOW_BONDED_ADV_INT_MAX;
        break;
    case UNBONDED_ADV:
        gap_cfg_table->GAP_TMR_LIM_ADV_TIMEOUT_VAR = (adv_timeout / 10);
        cmd->info.host.mode = GAP_LIM_DISCOVERABLE;
        cmd->intv_min = NORMAL_ADV_INT_MIN;
        cmd->intv_max = NORMAL_ADV_INT_MAX;
        break;
    case BONDED_ADV:
        gap_cfg_table->GAP_TMR_LIM_ADV_TIMEOUT_VAR = (adv_timeout / 10);
        cmd->info.host.mode = GAP_LIM_DISCOVERABLE;
        cmd->intv_min = FAST_BONDED_ADV_INT_MIN;
        cmd->intv_max = FAST_BONDED_ADV_INT_MAX;
        break;
    default:
        break;
    }
    
    if (HAS_WHITE_LIST)
    {
        if (white_list_written)
            cmd->info.host.adv_filt_policy = ADV_ALLOW_SCAN_WLST_CON_WLST;
        else
            cmd->info.host.adv_filt_policy = ADV_ALLOW_SCAN_ANY_CON_ANY;
    }
    else
    {
        cmd->info.host.adv_filt_policy = ADV_ALLOW_SCAN_ANY_CON_ANY;
    }
    /*-----------------------------------------------------------------------------------
     * Set the Advertising Data and the Scan Response Data
     *---------------------------------------------------------------------------------*/
    set_adv_data(cmd);
    
    // Send the message
    ke_msg_send(cmd);

    // We are now connectable
    ke_state_set(TASK_APP, APP_CONNECTABLE);
}

static void start_adv_directed(void)
{
    // Allocate a message for GAP
    struct gapm_start_advertise_cmd *cmd = KE_MSG_ALLOC(GAPM_START_ADVERTISE_CMD,
                                                TASK_GAPM, TASK_APP,
                                                gapm_start_advertise_cmd);

    cmd->op.code = GAPM_ADV_DIRECT;
    cmd->op.addr_src = GAPM_PUBLIC_ADDR;
    cmd->channel_map = APP_ADV_CHMAP;
    cmd->intv_min = APP_ADV_INT_MIN;
    cmd->intv_max = APP_ADV_INT_MAX;
    cmd->info.host.mode = GAP_GEN_DISCOVERABLE;
    cmd->info.direct.addr_type = app_sec_env.peer_addr_type;
    memcpy((void *)cmd->info.direct.addr.addr, app_sec_env.peer_addr.addr, BD_ADDR_LEN);
    
    // Send the message
    ke_msg_send(cmd);

    // We are now connectable
    ke_state_set(TASK_APP, APP_CONNECTABLE);
}


static void send_connection_upd_req(void)
{    
    ke_state_t app_state = ke_state_get(TASK_APP);
    
	// Modify Conn Params
	if (app_state == APP_SECURITY || app_state == APP_PARAM_UPD || app_state == APP_CONNECTED) 
	{
		struct gapc_param_update_cmd * req = KE_MSG_ALLOC(GAPC_PARAM_UPDATE_CMD, TASK_GAPC, TASK_APP, gapc_param_update_cmd);

		// Fill in the parameter structure
        req->operation = GAPC_UPDATE_PARAMS;
		req->params.intv_min = PREFERRED_CONN_INTERVAL_MIN;	    // N * 1.25ms
		req->params.intv_max = PREFERRED_CONN_INTERVAL_MAX;	    // N * 1.25ms
		req->params.latency  = PREFERRED_CONN_LATENCY;		    // Conn Events skipped
		req->params.time_out = PREFERRED_CONN_TIMEOUT;		    // N * 10ms
		dbg_puts(DBG_FSM_LVL, "Send GAP_PARAM_UPDATE_REQ\r\n");
		ke_msg_send(req);
        
        ke_state_set(TASK_APP, APP_PARAM_UPD);
	}
}

static void clear_white_list(void)
{
    struct gapm_white_list_mgt_cmd * req = KE_MSG_ALLOC(GAPM_WHITE_LIST_MGT_CMD, TASK_GAPM, TASK_APP, gapm_white_list_mgt_cmd);

    // Fill in the parameter structure
    req->operation = GAPM_CLEAR_WLIST;
    req->nb = 0;
    
    dbg_puts(DBG_CONN_LVL, "    Clear White List\r\n");
    ke_msg_send(req);
    
    white_list_written = false;
}

static void wake_ble_up(void)
{
    // If BLE is sleeping, wake it up!
    GLOBAL_INT_DISABLE();
    if (GetBits16(CLK_RADIO_REG, BLE_ENABLE) == 0) { // BLE clock is off
        SetBits16(GP_CONTROL_REG, BLE_WAKEUP_REQ, 1);
    }
    GLOBAL_INT_RESTORE();
    app_ble_ext_wakeup_off();
}


__attribute__((unused)) static const char state_names[8][24] = { "IDLE", "ADVERTISE", "CONNECTION_IN_PROGRESS", "CONNECTED_PAIRING", "CONNECTED",
                        "DISCONNECTED_IDLE", "DISCONNECTED_INIT", "DIRECTED_ADV" };

__attribute__((unused)) static const char events_names[13][17] = { "NO_EVENT", "KEY_PRESS", "TIMER_EXPIRED", "PAIRING_REQ", "CONN_REQ",
                         "CONN_CMP", "DISCONN", "CONN_UPD_RESP", "PASSKEY_ENTERED", "SWITCH_EVT", "NEW_HOST_EVT", "SPOTAR_START_EVT", "SPOTAR_END_EVT" };


void app_state_update(enum main_fsm_events evt)
{
    if ( (KEY_PRESS_EVT == evt) && (IDLE_ST != current_fsm_state) && (CONNECTED_ST != current_fsm_state) )
        return;
        
    dbg_printf(DBG_FSM_LVL, "\r\n**> %s\r\n", state_names[current_fsm_state]);
    dbg_printf(DBG_FSM_LVL, "--> %s\r\n", events_names[evt]);
    
    switch(current_fsm_state) 
    {
    case IDLE_ST:
    
        if (HAS_EEPROM)
        {
            if (!eeprom_is_read)
                is_bonded = false;
            
            if ( (!is_bonded) && (!eeprom_is_read) )
            {
                app_alt_pair_read_status();
                is_bonded = app_alt_pair_get_next_bond_data(true);
                eeprom_is_read = true;
            }
        }
        
        
        switch(evt)
        {
        case NO_EVENT:
            if (HAS_NORMALLY_CONNECTABLE)
            {
                if (is_bonded)
                {
                    start_adv_directed();
                    current_fsm_state = DIRECTED_ADV_ST;
                }
                else
                {
                    current_adv_state = SLOW_ADV;
                    start_adv_limited();
                    current_fsm_state = ADVERTISE_ST;
                }
                
                wake_ble_up();
                if (HAS_DEEPSLEEP)
                {
                    app_set_extended_sleep();
                }
            }
            else if (HAS_DEEPSLEEP)
            {
                app_set_deep_sleep();
                app_ble_ext_wakeup_on();
            }
            else
            {
                app_ble_ext_wakeup_on();
            }
            break;
            
        case KEY_PRESS_EVT:
            if (is_bonded)
            {
                start_adv_directed();
                current_fsm_state = DIRECTED_ADV_ST;
            }
            else
            {
                adv_timeout = KBD_UNBONDED_DISCOVERABLE_TIMEOUT;
                current_adv_state = UNBONDED_ADV;
                start_adv_limited();
                current_fsm_state = ADVERTISE_ST;
            }
            
            wake_ble_up();
            if (HAS_DEEPSLEEP)
            {
                app_set_extended_sleep();
            }
            break;
            
        default:
            ASSERT_(0);
            break;
        }
        break;
        
        
    case ADVERTISE_ST:
        switch(evt)
        {
        case TIMER_EXPIRED_EVT:
            app_kbd_flush_buffer();
            app_kbd_flush_reports();
            if (HAS_NORMALLY_CONNECTABLE)
            {
                current_adv_state = SLOW_ADV;
                start_adv_limited();
            }
            else
            {
                if (HAS_WHITE_LIST)
                {
                    clear_white_list();
                }
                // timer is cleared automatically
                
                if (HAS_DEEPSLEEP)
                {
                    app_set_deep_sleep();
                }
                app_ble_ext_wakeup_on();
                current_fsm_state = IDLE_ST;
            }
            break;
            
        case CONN_REQ_EVT:
            current_fsm_state = CONNECTION_IN_PROGRESS_ST;
            break;
            
        default:
            ASSERT_(0);
            break;
        }
        break;
        
        
    case CONNECTION_IN_PROGRESS_ST:
        if (!HAS_MITM)
        {
            dbg_puts(DBG_FSM_LVL, "  (-) HID ENC timer\r\n");
            ke_timer_clear(APP_HID_ENC_TIMER, TASK_APP);        // Timer expire results in CONN_CMP_EVT and is handled below
        }
        
        switch(evt)
        {
        case PAIRING_REQ_EVT:
            if (HAS_PASSCODE_TIMEOUT)
            {
                dbg_puts(DBG_FSM_LVL, "  (+) passcode timer\r\n");
                app_timer_set(APP_HID_TIMER, TASK_APP, (KBD_PASSCODE_TIMEOUT / 10));
            }
            app_kbd_flush_buffer(); // entering "passcode" mode
            current_fsm_state = CONNECTED_PAIRING_ST;
            break;
            
        case NEW_HOST_EVT:
            app_kbd_flush_buffer();
            app_kbd_flush_reports();
            break;
        
        case CONN_CMP_EVT:
            // LEDs are controlled by the caller ("established")
            
            if (HAS_KBD_SWITCH_TO_PREFERRED_CONN_PARAMS)
            {
                // Timer for sending the CONN_PARAM_UPDATE is set by the caller
                dbg_puts(DBG_FSM_LVL, "  (+) update params timer\r\n");
                conn_upd_pending = true;
            }
            else
            {
                if (HAS_INACTIVITY_TIMEOUT)
                {
                    dbg_puts(DBG_FSM_LVL, "  (+) inactivity timer\r\n");
                    app_timer_set(APP_HID_TIMER, TASK_APP, (KBD_INACTIVITY_TIMEOUT / 10));
                }
            }
            
            current_fsm_state = CONNECTED_ST;
            break;
            
        case DISCONN_EVT:
            // LEDs are controlled by the caller ("disconnected")
            
            // Advertising settings remain unchanged!
            start_adv_limited();
            current_fsm_state = ADVERTISE_ST;
            break;
            
        default:
            ASSERT_(0);
            break;
        }
        break;
        
        
    case CONNECTED_PAIRING_ST:
        if (HAS_PASSCODE_TIMEOUT)
        {
            dbg_puts(DBG_FSM_LVL, "  (-) passcode timer\r\n");
            ke_timer_clear(APP_HID_TIMER, TASK_APP);
        }
        
        switch(evt)
        {
        case PASSKEY_ENTERED:
            if (HAS_PASSCODE_TIMEOUT)
            {
                dbg_puts(DBG_FSM_LVL, "  (+) passcode timer\r\n");
                app_timer_set(APP_HID_TIMER, TASK_APP, (KBD_PASSCODE_TIMEOUT / 10));
            }
            break;
            
        case CONN_CMP_EVT:
            // LEDs are controlled by the caller ("established")
            
            if (HAS_KBD_SWITCH_TO_PREFERRED_CONN_PARAMS)
            {
                // Timer for sending the CONN_PARAM_UPDATE is set by the caller
                dbg_puts(DBG_FSM_LVL, "  (+) update params timer\r\n");
                conn_upd_pending = true;
            }
            else
            {
                if (HAS_INACTIVITY_TIMEOUT)
                {
                    dbg_puts(DBG_FSM_LVL, "  (+) inactivity timer\r\n");
                    app_timer_set(APP_HID_TIMER, TASK_APP, (KBD_INACTIVITY_TIMEOUT / 10));
                }
            }
            
            current_fsm_state = CONNECTED_ST;
            break;
            
        case DISCONN_EVT:
            // LEDs are controlled by the caller ("disconnected")
            
            // Advertising settings remain unchanged!
            start_adv_limited();
            current_fsm_state = ADVERTISE_ST;
            break;
            
        case TIMER_EXPIRED_EVT:                                                                     // ************ 1
            if (HAS_PASSCODE_TIMEOUT)
            {
                app_disconnect();
                current_fsm_state = DISCONNECTED_INIT_ST;
            }
            else 
                ASSERT_(0);
            break;
            
        default:
            ASSERT_(0);
            break;
        }
        break;
        
        
    case CONNECTED_ST:
        if (conn_upd_pending)
        {
            // wait for the timer to elapse
        }
        else
        {
            if (HAS_INACTIVITY_TIMEOUT)
            {
                dbg_puts(DBG_FSM_LVL, "  (-) timer\r\n");
                ke_timer_clear(APP_HID_TIMER, TASK_APP);
            }
        }

        switch(evt)
        {
        case SPOTAR_START_EVT:
            if ( (HAS_INACTIVITY_TIMEOUT) && !(conn_upd_pending) )
            {
                dbg_puts(DBG_FSM_LVL, "  (-) inactivity timer\r\n");
                ke_timer_clear(APP_HID_TIMER, TASK_APP);
            }
            break;
            
        case SPOTAR_END_EVT:
            if ( (HAS_INACTIVITY_TIMEOUT) && !(conn_upd_pending) )
            {
                dbg_puts(DBG_FSM_LVL, "  (!) inactivity timer\r\n");
                app_timer_set(APP_HID_TIMER, TASK_APP, (KBD_INACTIVITY_TIMEOUT / 10));
            }
            break;
            
        case KEY_PRESS_EVT:
            if ( (HAS_INACTIVITY_TIMEOUT) && !(conn_upd_pending) && ke_timer_active(APP_HID_TIMER, TASK_APP) )
            {
                dbg_puts(DBG_FSM_LVL, "  (!) inactivity timer\r\n");
                app_timer_set(APP_HID_TIMER, TASK_APP, (KBD_INACTIVITY_TIMEOUT / 10));
            }
            break;
            
        case CONN_CMP_EVT:  // CONN_PARAM_UPDATE Timer expired!
            if (HAS_INACTIVITY_TIMEOUT)
            {
                dbg_puts(DBG_FSM_LVL, "  (+) inactivity timer\r\n");
                app_timer_set(APP_HID_TIMER, TASK_APP, (KBD_INACTIVITY_TIMEOUT / 10));
            }
            break;
            
        case SWITCH_EVT:
            // LEDs are controlled by the caller ("disconnected")
            
            current_fsm_state = DISCONNECTED_INIT_ST;
            break;
            
        case DISCONN_EVT:
            // LEDs are controlled by the caller ("disconnected")
            
            if (app_sec_env.peer_addr_type <= GAPM_GEN_STATIC_RND_ADDR)
            {
                start_adv_directed();
                current_fsm_state = DIRECTED_ADV_ST;
            }
            else
            {
                adv_timeout = KBD_BONDED_DISCOVERABLE_TIMEOUT;
                current_adv_state = BONDED_ADV;
                start_adv_limited();
                current_fsm_state = ADVERTISE_ST;
            }
            break;
            
        case TIMER_EXPIRED_EVT:
            if (HAS_INACTIVITY_TIMEOUT)
            {
                if ( (HAS_KBD_SWITCH_TO_PREFERRED_CONN_PARAMS) && (conn_upd_pending) )
                {
                    send_connection_upd_req();
                    conn_upd_pending = false;
                }
                else
                {
                    app_disconnect();
                    current_fsm_state = DISCONNECTED_IDLE_ST;
                }
            }
            else 
                ASSERT_(0);
            break;
            
        default:
            ASSERT_(0);
            break;
        }
        break;
        
    case DISCONNECTED_IDLE_ST:
        switch(evt)
        {
        case DISCONN_EVT:
            // LEDs are controlled by the caller ("disconnected")
            
            if (HAS_NORMALLY_CONNECTABLE)
            {
                current_adv_state = SLOW_ADV;
                start_adv_limited();
                current_fsm_state = ADVERTISE_ST;
            }
            else
            {
                if (HAS_DEEPSLEEP)
                {
                    app_set_deep_sleep();
                }
                app_ble_ext_wakeup_on();
                current_fsm_state = IDLE_ST;
            }
            break;

        default:
            ASSERT_(0);
            break;
        }
        break;
        
    case DISCONNECTED_INIT_ST:
        switch(evt)
        {
        case DISCONN_EVT:
            // LEDs are controlled by the caller ("disconnected")
            
            if (__builtin_popcount(multi_bond_status) > 1)
            {
                adv_timeout = KBD_BONDED_DISCOVERABLE_TIMEOUT;
                current_adv_state = BONDED_ADV;
            }
            else
            {
                adv_timeout = KBD_UNBONDED_DISCOVERABLE_TIMEOUT;
                current_adv_state = UNBONDED_ADV;
            }
            start_adv_limited();
            current_fsm_state = ADVERTISE_ST;
            break;

        default:
            ASSERT_(0);
            break;
        }
        break;
        
        
    case DIRECTED_ADV_ST:
        switch(evt)
        {
        case TIMER_EXPIRED_EVT:
            if (HAS_WHITE_LIST)
            {
                struct gapm_white_list_mgt_cmd * req = KE_MSG_ALLOC_DYN(GAPM_WHITE_LIST_MGT_CMD, TASK_GAPM, TASK_APP, gapm_white_list_mgt_cmd, sizeof(struct gap_bdaddr));

                // Fill in the parameter structure
                req->operation = GAPM_ADD_DEV_IN_WLIST;
                req->nb = 1;
                req->devices[0].addr_type = app_sec_env.peer_addr_type;
                memcpy(req->devices[0].addr.addr, app_sec_env.peer_addr.addr, BD_ADDR_LEN);
                
                dbg_puts(DBG_CONN_LVL, "    Added Host in White List\r\n");
                ke_msg_send(req);
                
                white_list_written = true;
            }
            adv_timeout = KBD_BONDED_DISCOVERABLE_TIMEOUT;
            current_adv_state = BONDED_ADV;
            start_adv_limited();
            current_fsm_state = ADVERTISE_ST;
            break;
            
        case CONN_REQ_EVT:
            if (HAS_MITM)
            {
                if (HAS_KEYBOARD_LEDS)
                {
                    leds_set_connection_in_progress();
                }
            }
            else
            {
                // set a timer in case encyption does not follow
                dbg_puts(DBG_FSM_LVL, "  (+) HID ENC timer\r\n");
                app_timer_set(APP_HID_ENC_TIMER, TASK_APP, (KBD_ENC_SAFEGUARD_TIMEOUT / 10));
            }
            // prepare advertising settings in case connection setup fails
            adv_timeout = KBD_BONDED_DISCOVERABLE_TIMEOUT;
            current_adv_state = BONDED_ADV;
            current_fsm_state = CONNECTION_IN_PROGRESS_ST;
            break;
        
        default:
            ASSERT_(0);
            break;
        }
        break;
        
    default:
        ASSERT_(0);
        break;
    }
    
    dbg_printf(DBG_FSM_LVL, "    (N) %s\r\n", state_names[current_fsm_state]);
}


void reset_bonding_data(void)
{
    switch(current_fsm_state)
    {
    case ADVERTISE_ST:
        if (HAS_WHITE_LIST)
        {
            clear_white_list();
        }
    case IDLE_ST:
    case DISCONNECTED_IDLE_ST:
    case DISCONNECTED_INIT_ST:
        memset( (uint8_t *)&app_sec_env, 0, sizeof(struct app_sec_env_tag) );
        is_bonded = false;
        break;
    default:
        break;
    }
}



