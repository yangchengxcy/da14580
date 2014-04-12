/**
 ****************************************************************************************
 *
 * @file app_keyboard_proj.c
 *
 * @brief HID Keyboard hooks.
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

/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */
 
#include "rwip_config.h"             // SW configuration

#if (BLE_APP_PRESENT)

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "app.h" 
#include "app_keyboard_proj.h"
#include "app_keyboard.h"
#include "app_utils.h"
#include "arch_sleep.h"

#include "app_sec.h"
#include "app_dis.h"
#include "app_batt.h"

#if (BLE_ALT_PAIR)
#include "app_alt_pair.h"
#endif

#include "gapm_task.h"
#include "gapm_util.h"
#include "gapc.h"

extern struct gap_cfg_table_struct gap_timeout_table;
extern uint8_t batt_level;
extern uint8_t old_batt_level; 
extern uint8_t bat_led_state;

/*
 * LOCAL VARIABLES
 ****************************************************************************************
 */
// struct bd_addr peer_addr    __attribute__((section("exchange_mem_case1")));
struct bd_addr peer_addr;

 
/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
*/
void app_task_custom_init(void);
static void app_set_adv_data(void);

/*
 * Name         : app_update_gap_cfg - Update gap_cfg_table[]. 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : Updates gap_cfg_table[] with the preferred values (if any). Must be called
 *                at initialization and after a deep sleep wake up (because SysRAM contents
 *                are lost and gap_cfg_table[] is reset to the default values).
 *
 * Returns      : void
 *
 */
void app_update_gap_cfg(void)
{
    if (strlen(APP_DFLT_DEVICE_NAME) < 30)
        memcpy(gap_timeout_table.GAP_DEV_NAME_VAR, APP_DFLT_DEVICE_NAME, strlen(APP_DFLT_DEVICE_NAME));
#if  DEVELOPMENT__NO_OTP
    else
        __asm("BKPT #0\n");
#endif // DEVELOPMENT__NO_OTP
}
 

/*
 * Name         : app_init_func - Initialize Keyboard app 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : Initialize state, GPIOs. Set GAP cfg and advertising data. Set keys (?). Set sleep mode.
 *
 * Returns      : void
 *
 */
void app_init_func(void)
{
	app_task_custom_init();	
    
	app_keyboard_init();
    app_env.app_state = IDLE;
    
    app_set_adv_data();
    app_update_gap_cfg();
    
    // Initialize Device Information Service
    app_dis_init();
    
//    app_set_deep_sleep();
    app_set_extended_sleep();
//    app_disable_sleep();
}

// void app_kbd_sleep_control(bool enable)
// {
//     if (enable) {
//         app_set_extended_sleep();
//     } else
//         app_disable_sleep();
// }


/*
 * Name         : app_configuration_func - Configure Keyboard BT settings 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : task_id - id of the kernel task calling this function
 *                cmd - parameters to pass to the stack
 *
 * Description  : Configures Bluetooth settings for the Keyboard application.
 *
 * Returns      : void
 *
 */
void app_configuration_func(ke_task_id_t const task_id, struct gapm_set_dev_config_cmd *cmd)
{
    // Device Appearance
    cmd->appearance = 961;          // Keyboard
    // Slave preferred Minimum of connection interval
    cmd->con_intv_min = 6;         // 7.5ms (6*1.25ms)
    // Slave preferred Maximum of connection interval
    cmd->con_intv_max = 16;        // 20ms (16*1.25ms)
    // Slave preferred Connection latency
    cmd->con_latency  = 31;
    // Slave preferred Link supervision timeout
    cmd->superv_to    = 200;

    // Device IRK
    memcpy(cmd->irk.key, "0123456789012345", KEY_LEN);
    
    // Privacy settings bit field (0b1 = enabled, 0b0 = disabled)
    //  - [bit 0]: Privacy Support
    //  - [bit 1]: Multiple Bond Support (Peripheral only); If enabled, privacy flag is
    //             read only.
    //  - [bit 2]: Reconnection address visible.
    cmd->flags = 0;
}

/*
 * Name         : app_sec_init_func - Initialize security 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : Initialize security to "no man in the middle - no bonding".
 *
 * Returns      : void
 *
 */
#define HID_KEY_SIZE (16)

void app_sec_init_func(void)
{
#if (BLE_APP_SEC)
//# ifdef MITM_REQUIRED
//	app_sec_env.auth = GAP_AUTH_REQ_MITM_BOND;
//# else
//	app_sec_env.auth = GAP_AUTH_REQ_NO_MITM_BOND;
//# endif

# if (BLE_ALT_PAIR)
    alt_pair_adv_mode = true;
    if (!app_alt_pair_get_next_bond_data(alt_pair_adv_mode))
        alt_pair_adv_mode = false;
# endif
//    app_sec_gen_ltk(HID_KEY_SIZE);
#endif
}


#undef HID_KEY_SIZE

static ke_task_id_t mitm_src_id, mitm_dest_id;

/*
 * Name         : app_mitm_passcode_entry - Start keyboard for Passcode entry 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : src_id - id of the kernel task caused this to happen
 *                dest_id - id of the kernel task that received the message
 *
 * Description  : Starts keyboard scanning and sets the keyboard to passcode entry mode
 *
 * Returns      : void
 *
 */
void app_mitm_passcode_entry(ke_task_id_t const src_id, ke_task_id_t const dest_id)
{
    // store task IDs
    mitm_src_id = src_id;
    mitm_dest_id = dest_id;
    
    app_kbd_enable_scanning();          // enable key scanning    
}


/*
 * Name         : app_mitm_passcode_report - Report Passcode 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : code - the code to report to the Host
 *
 * Description  : Sends the passcode that the user entered to the Host
 *
 * Returns      : void
 *
 */
void app_mitm_passcode_report(uint32_t code)
{
    struct gapc_bond_cfm* cfm = KE_MSG_ALLOC(GAPC_BOND_CFM, mitm_src_id, mitm_dest_id, gapc_bond_cfm);
    cfm->request = GAPC_TK_EXCH;
    cfm->accept = true;

    memset(cfm->data.tk.key, 0, KEY_LEN);

    cfm->data.tk.key[3] = (uint8_t)((code & 0xFF000000) >> 24);
    cfm->data.tk.key[2] = (uint8_t)((code & 0x00FF0000) >> 16);
    cfm->data.tk.key[1] = (uint8_t)((code & 0x0000FF00) >>  8);
    cfm->data.tk.key[0] = (uint8_t)((code & 0x000000FF) >>  0);

    ke_msg_send(cfm);

    arch_printf("Code: %d\n\r", code);
    app_kbd_start_reporting();          // start sending notifications    
}


/*
 * Name         : app_sec_encrypt_complete_func - Enryption is completed -> Start normal keyboard operation
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : After completion of the encryption of the link, it sets the keyboard to normal
 *                report mode.
 *
 * Returns      : void
 *
 */
void app_sec_encrypt_complete_func(void)
{
    app_kbd_enable_scanning();          // enable key scanning    
    app_kbd_start_reporting();          // start sending notifications    
}


/*
 * Name         : app_connection_func - Configure app at connection establishment 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : dest_id - id of the kernel task calling this function
 *                param - parameters passed from the stack
 *
 * Description  : Configures keyboard application when connection is established.
 *
 * Returns      : void
 *
 */
void app_connection_func(ke_task_id_t const dest_id, struct gapc_connection_req_ind const *param)
{
    /*--------------------------------------------------------------
    * ENABLE REQUIRED PROFILES
    *-------------------------------------------------------------*/
	arch_printf("gap_le_create_conn_req_cmp_evt_handler() (%d, %d, %d, %d)\r\n", 
			(int)param->con_interval,
			(int)param->con_latency,
			(int)param->sup_to,
			(int)param->clk_accuracy
		  );

    peer_addr = param->peer_addr;
    arch_printf("Peer addr %02x:%02x:%02x:%02x:%02x:%02x\r\n", 
        peer_addr.addr[0], peer_addr.addr[1], peer_addr.addr[2], 
        peer_addr.addr[3], peer_addr.addr[4], peer_addr.addr[5]);
    
    gapm_set_recon_addr(&peer_addr);
    
    app_env.conhdl = param->conhdl;     // Store the connection handle

#if (BLE_ALT_PAIR)
    alt_pair_adv_mode = false;
#endif    
    
    app_dis_enable_prf(param->conhdl);
    batt_level = 99;
    old_batt_level = 40;
    app_batt_enable(batt_level, old_batt_level);
    app_batt_poll_start();
    
    app_keyboard_enable();              // enable HOGPD profile
        
    app_env.app_state = CONNECTED_FAST;
}


/*
 * Name         : app_disconnect_func - Configure app at disconnection 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : task_id - id of the kernel task calling this function
 *                param - parameters passed from the stack
 *
 * Description  : Configures keyboard application when connection is terminated.
 *
 * Returns      : void
 *
 */
void app_disconnect_func(ke_task_id_t const task_id, struct gapc_disconnect_ind const *param)
{
    ke_timer_clear(APP_HID_TIMER, task_id);
    arch_printf("** Clear param update timer\r\n");
    app_kbd_stop();                                 // disable scanning here?
    app_env.app_state = DISCONNECTED;
}


/*
 * Name         : app_param_update_func - Request update of connection params 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : task_id - id of the kernel task calling this function
 *                param - parameters passed from the stack
 *
 * Description  : After connection and, optionally, pairing is completed, this function 
 *                is called to (optionally) modify the connection parameters.
 *
 * Returns      : void
 *
 */
void app_param_update_func(void)
{
#ifdef KBD_SWITCH_TO_PREFERRED_CONN_PARAMS    
    //Set a timer to update connection params after i.e. 5sec
    ke_timer_set(APP_HID_TIMER, TASK_APP, TIME_TO_SWITCH_TO_PREFERRED_MODE);
    arch_printf("** Set param update timer\r\n");
#endif    
    arch_puts("app_param_update_func\r\n");
#ifndef MITM_REQUIRED
    app_kbd_enable_scanning();          // enable key scanning    
    app_kbd_start_reporting();          // start sending notifications    
#endif    
}


static uint8_t app_adv_data_length                  __attribute__((section("exchange_mem_case2"))) = 0;  // Advertising data length
static uint8_t app_adv_data[ADV_DATA_LEN-3]         __attribute__((section("exchange_mem_case2")));      // Advertising data
static uint8_t app_scanrsp_data_length              __attribute__((section("exchange_mem_case2"))) = 0;  // Scan response data length
static uint8_t app_scanrsp_data[SCAN_RSP_DATA_LEN]  __attribute__((section("exchange_mem_case2")));      // Scan response data

/*
 * Name         : app_set_adv_data - Set advertising data for the Keyboard app 
 *
 * Scope        : LOCAL
 *
 * Arguments    : none
 *
 * Description  : Sets the advertising and the scan response data.
 *
 * Returns      : void
 *
 */
static void app_set_adv_data(void)
{
    int8_t device_name_length;      // Device Name Length

    /*-----------------------------------------------------------------------------
     * Set the Advertising Data
     *-----------------------------------------------------------------------------*/
    app_adv_data_length = APP_ADV_DATA_LENGTH;
    memcpy(&app_adv_data[0], APP_ADV_DATA, app_adv_data_length);
        
    /*-----------------------------------------------------------------------------
     * Set the Scan Response Data
     *-----------------------------------------------------------------------------*/
    app_scanrsp_data_length = APP_SCNRSP_DATA_LENGTH;
    if (app_scanrsp_data_length > 0) 
        memcpy(&app_scanrsp_data[0], APP_SCNRSP_DATA, app_scanrsp_data_length);
        

#ifdef APP_DFLT_DEVICE_NAME
    /*-----------------------------------------------------------------------------
     * Add the Device Name in the Advertising Data
     *-----------------------------------------------------------------------------*/
    // Get available space in the Advertising Data
    device_name_length = APP_ADV_DATA_MAX_SIZE - app_adv_data_length - 2;

    // Check if data can be added to the Advertising data
    if (device_name_length > 0)
    {
        // Get default Device Name (No name if not enough space)
        device_name_length = (strlen(APP_DFLT_DEVICE_NAME) < device_name_length) ? strlen(APP_DFLT_DEVICE_NAME) : 0;
        if (device_name_length > 0) {
            memcpy(&app_adv_data[app_adv_data_length + 2], APP_DFLT_DEVICE_NAME, device_name_length);

            app_adv_data[app_adv_data_length]     = device_name_length + 1;         // Length
            app_adv_data[app_adv_data_length + 1] = '\x09';                         // Device Name Flag

            app_adv_data_length += (device_name_length + 2);                        // Update Advertising Data Length
        }
    }
        
    if (device_name_length > 0)
        return; // device name has been added

    /*-----------------------------------------------------------------------------
     * Add the Device Name in the Advertising Scan Response Data
     *-----------------------------------------------------------------------------*/
    // Get available space in the Advertising Data
    device_name_length = APP_ADV_DATA_MAX_SIZE - app_scanrsp_data_length - 2;

    // Check if data can be added to the Advertising data
    if (device_name_length > 0)
    {
        // Get default Device Name (No name if not enough space)
        device_name_length = (strlen(APP_DFLT_DEVICE_NAME) < device_name_length) ? strlen(APP_DFLT_DEVICE_NAME) : 0;
        if (device_name_length > 0) {
            memcpy(&app_scanrsp_data[app_scanrsp_data_length + 2], APP_DFLT_DEVICE_NAME, device_name_length);

            app_scanrsp_data[app_scanrsp_data_length]     = device_name_length + 1; // Length
            app_scanrsp_data[app_scanrsp_data_length + 1] = '\x09';                 // Device Name Flag
            
            app_scanrsp_data_length += (device_name_length + 2);                    // Update Scan response Data Length
        }
    }
#endif // APP_DFLT_DEVICE_NAME
}

/*
 * Name         : app_adv_func - Pass app defined advertise and scan response data to the stack
 *
 * Scope        : PUBLIC
 *
 * Arguments    : cmd - message to GAPM 
 *
 * Description  : If the advertising and scan response data set by app_adv_start are not correct
 *                for this application, this function overwrites them with the ones prepared in 
 *                app_set_adv_data().
 *                Note that this costs in retention memory (since the data are not prepared
 *                dynamically but are stored in the retention memory). (FIXME)
 *
 * Returns      : none
 *
 */
void app_adv_func(struct gapm_start_advertise_cmd *cmd)
{
    if (app_adv_data_length != 0)
    {
        memcpy(&cmd->info.host.adv_data[0], app_adv_data, app_adv_data_length);
        cmd->info.host.adv_data_len = app_adv_data_length;
    }

    if (app_scanrsp_data_length != 0)
    {
        memcpy(&cmd->info.host.scan_rsp_data[0], app_scanrsp_data, app_scanrsp_data_length);
        cmd->info.host.scan_rsp_data_len = app_scanrsp_data_length;
    }

#if (BLE_ALT_PAIR)
    if (alt_pair_adv_mode)
    {
        cmd->op.code = GAPM_ADV_DIRECT;
        ///  Direct address information (GAPM_ADV_DIRECT)
        /// (used only if reconnection address isn't set or privacy disabled)
        cmd->info.direct.addr_type = app_sec_env.peer_addr_type;
        memcpy((void *)cmd->info.direct.addr.addr, app_sec_env.peer_addr.addr, BD_ADDR_LEN);
        ke_timer_set(APP_DIR_ADV_TIMER, TASK_APP, 130);	//1.30 seconds
    }
#endif    
}


/*
 * Name         : app_db_init_func - Initialize HID server DB 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : Initializes the database for the keyboard application.
 *
 * Returns      : true if initialization is done, else false.
 *
 */
bool app_db_init_func(void)
{
    // Indicate if more services need to be added in the database
    bool end_db_create = false;
    
    // Check if another should be added in the database
    if (app_env.next_prf_init < APP_PRF_LIST_STOP)
    {
        switch (app_env.next_prf_init)
        {
            case (APP_DIS_TASK):
            {
                app_dis_create_db_send();
            } break;
            case (APP_BASS_TASK):
            {
                app_batt_create_db(); 
            } break;
            case (APP_HOGPD_TASK):
            {
                // Add HID Service in the DB
                app_hid_create_db();
            } break;
            default:
            {
                ASSERT_ERR(0);
            } break;

        }

        // Select following service to add
        app_env.next_prf_init++;
    }
    else
    {
        end_db_create = true;
    }

    return end_db_create;
}


/*
 * Name         : app_hid_timer_handler - Handler of the HID Timer 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : <various>
 *
 * Description  : Sends a Connection Parameters Update Request to the Host
 *
 * Returns      : KE_MSG_CONSUMED
 *
 */
int app_hid_timer_handler(ke_msg_id_t const msgid,
                           void const *param,
                           ke_task_id_t const dest_id,
                           ke_task_id_t const src_id)
{
	arch_puts("app_hid_timer_handler()\r\n");
	
    ke_state_t app_state = ke_state_get(TASK_APP);
    
	// Modify Conn Params
	if (app_state == APP_SECURITY || app_state == APP_PARAM_UPD || app_state == APP_CONNECTED) 
	{
		struct gapc_param_update_cmd * req = KE_MSG_ALLOC(GAPC_PARAM_UPDATE_CMD, TASK_GAPC, TASK_APP, gapc_param_update_cmd);

        req->operation = GAPC_UPDATE_PARAMS;
		// Fill in the parameter structure
		req->params.intv_min = PREFERRED_CONN_INTERVAL_MIN;	    // N * 1.25ms
		req->params.intv_max = PREFERRED_CONN_INTERVAL_MAX;	    // N * 1.25ms
		req->params.latency  = PREFERRED_CONN_LATENCY;		    // Conn Events skipped
		req->params.time_out = PREFERRED_CONN_TIMEOUT;		    // N * 10ms
		arch_puts("Send GAP_PARAM_UPDATE_REQ\r\n");
		ke_msg_send(req);
	}
	
	return (KE_MSG_CONSUMED);
}


#endif  //BLE_APP_PRESENT
/// @} APP
