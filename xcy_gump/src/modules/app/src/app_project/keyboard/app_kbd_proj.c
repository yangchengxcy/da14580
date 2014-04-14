/**
 ****************************************************************************************
 *
 * @file app_kbd_proj.c
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
 
#include "rwip_config.h"

#if (BLE_APP_PRESENT)

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "arch.h"
#include "app.h" 
#include "app_task.h"
#include "app_api.h"
#include "gapm_task.h"
#include "gapm_util.h"
#include "gapc.h"
#include "rwble_hl_config.h"

#include "app_kbd.h"
#include "app_kbd_proj.h"
#include "app_kbd_key_matrix.h"
#include "app_kbd_fsm.h"
#include "app_kbd_leds.h"
#include "app_kbd_debug.h"
#include "app_multi_bond.h"
#include "app_console.h"
#include "arch_sleep.h"
#include "gpio.h"
#include "nvds.h"

#include "app_sec.h"
#include "app_dis.h"
#include "app_batt.h"



extern struct gap_cfg_table_struct gap_timeout_table;

/*
 * LOCAL VARIABLES
 ****************************************************************************************
 */

uint8_t app_adv_data_length                  __attribute__((section("retention_mem_area0"), zero_init));        // Advertising data length
uint8_t app_adv_data[ADV_DATA_LEN-3]         __attribute__((section("retention_mem_area0"), zero_init));        // Advertising data
uint8_t app_scanrsp_data_length              __attribute__((section("retention_mem_area0"), zero_init));        // Scan response data length
uint8_t app_scanrsp_data[SCAN_RSP_DATA_LEN]  __attribute__((section("retention_mem_area0"), zero_init));        // Scan response data



/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
*/
static void app_set_adv_data(void);


/*
 * Name         : app_init_func - Initialize Keyboard app 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : Initialize state, GPIOs. Set advertising data. Set sleep mode.
 *
 * Returns      : void
 *
 */
void app_init_func(void)
{
	app_keyboard_init();        // Initialize Keyboard env
    
    app_set_adv_data();         // Prepare Advertising data

    app_dis_init();         // Initialize Device Information Service
    
#if (BLE_SPOTA_RECEIVER)    
	app_spotar_init();
#endif
    
//    app_set_deep_sleep();
//    app_set_extended_sleep();
    app_disable_sleep();
}


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
    // set device configuration
    cmd->operation = GAPM_SET_DEV_CONFIG;
    // Device Role
    cmd->role = GAP_PERIPHERAL_SLV;
    // Device Appearance
    cmd->appearance = 961;          // Keyboard
    // Device Appearance write permission requirements for peer device
    cmd->appearance_write_perm = GAPM_WRITE_DISABLE;
    // Device Name write permission requirements for peer device
    cmd->name_write_perm = GAPM_WRITE_DISABLE;
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

    // Set the reconnection address if privacy is set for this fearure
//    gapm_set_recon_addr(&my_addr);
    
    if (HAS_KEYBOARD_LEDS)
    {
        // This is a good place to initialize the LEDs
        leds_init();
    }
}


/*
 * Name         : app_set_dev_config_complete_func - Configuration completed 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : Handles what needs to be done after the completion of the cofiguration phase.
 *
 * Returns      : void
 *
 */
void app_set_dev_config_complete_func(void)
{
    // We are now in Initialization State
    ke_state_set(TASK_APP, APP_DB_INIT);

    // Add the first required service in the database
    if (app_db_init())
    {
        app_state_update(NO_EVENT);
    }
}

/*
 * Name         : app_db_init_complete_func - Modules' initialization 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : Handles what needs to be done after the completion of the initialization
 *              : of all modules.
 *
 * Returns      : void
 *
 */
void app_db_init_complete_func(void)
{
    app_state_update(NO_EVENT);
}


/*
 * Name         : app_adv_undirect_complete - Undirected advertising finished 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : Handles what needs to be done after Undirected advertising finishes.
 *
 * Returns      : void
 *
 */
void app_adv_undirect_complete(uint8_t status)
{
    if ( (status != GAP_ERR_NO_ERROR) && (status != GAP_ERR_TIMEOUT) )
    {
        ASSERT_(0); // unexpected error
    }
    
    if (status == GAP_ERR_TIMEOUT)
    {
        if (ke_state_get(TASK_APP) == APP_CONNECTABLE)
            app_state_update(TIMER_EXPIRED_EVT);
    }
}

/*
 * Name         : app_adv_direct_complete - Directed advertising finished 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : Handles what needs to be done after Directed advertising finishes.
 *
 * Returns      : void
 *
 */
void app_adv_direct_complete(uint8_t status)
{
    if ( (status != GAP_ERR_NO_ERROR) && (status != GAP_ERR_TIMEOUT) )
    {
        ASSERT_(0); // unexpected error
    }
    
    if (status == GAP_ERR_TIMEOUT)
    {
        if (ke_state_get(TASK_APP) == APP_CONNECTABLE)
            app_state_update(TIMER_EXPIRED_EVT);
    }
}

/*
 * Name         : app_sec_init_func - Initialize security 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : empty
 *
 * Returns      : void
 *
 */
#define HID_KEY_SIZE (16)

void app_sec_init_func(void)
{
#if (BLE_APP_SEC)
//    if (HAS_MITM)
//    {
//        app_sec_env.auth = GAP_AUTH_REQ_MITM_BOND;
//    }
//    else
//    {
//        app_sec_env.auth = GAP_AUTH_REQ_NO_MITM_BOND;
//    }

//    app_sec_gen_ltk(HID_KEY_SIZE);
#endif
}
#undef HID_KEY_SIZE



/*
 * Name         : app_send_pairing_rsp_func - Reply to GAPC_PAIRING_REQ from host 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : param - the message from the host
 *
 * Description  : Prepares and sends the reply to the GAPC_PAIRING_REQ msg
 *
 * Returns      : void
 *
 */
void app_send_pairing_rsp_func(struct gapc_bond_req_ind *param)
{
    struct gapc_bond_cfm* cfm = KE_MSG_ALLOC(GAPC_BOND_CFM, TASK_GAPC, TASK_APP, gapc_bond_cfm);

    cfm->request = GAPC_PAIRING_RSP;
    cfm->accept = true;

    dbg_puts(DBG_CONN_LVL, "    GAPC_PAIRING_REQ\r\n");
    
    // OOB information
    cfm->data.pairing_feat.oob              = GAP_OOB_AUTH_DATA_NOT_PRESENT;
    // Encryption key size
    cfm->data.pairing_feat.key_size         = KEY_LEN;

    app_state_update(PAIRING_REQ_EVT);
    
    if (HAS_MITM)
    {
        // IO capabilities
        cfm->data.pairing_feat.iocap        = GAP_IO_CAP_KB_ONLY;
        // Authentication requirements
        cfm->data.pairing_feat.auth         = GAP_AUTH_REQ_MITM_BOND;
        //Security requirements
        cfm->data.pairing_feat.sec_req      = GAP_SEC1_AUTH_PAIR_ENC;
    }
    else
    {
        // IO capabilities
        cfm->data.pairing_feat.iocap        = GAP_IO_CAP_NO_INPUT_NO_OUTPUT;
        // Authentication requirements
        cfm->data.pairing_feat.auth         = GAP_AUTH_REQ_NO_MITM_BOND;
        //Security requirements
        cfm->data.pairing_feat.sec_req      = GAP_SEC1_NOAUTH_PAIR_ENC;
    }
    
    //Initiator key distribution
    cfm->data.pairing_feat.ikey_dist        = GAP_KDIST_SIGNKEY;
    //Responder key distribution
    cfm->data.pairing_feat.rkey_dist        = GAP_KDIST_ENCKEY;

    ke_msg_send(cfm);
}


/*
 * Name         : app_send_tk_exch_func - N/A 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : param - the message from the host
 *
 * Description  : N/A for the keyboard app
 *
 * Returns      : void
 *
 */
void app_send_tk_exch_func(struct gapc_bond_req_ind *param)
{
}


/*
 * Name         : app_mitm_passcode_entry_func - Start keyboard for Passcode entry 
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
static ke_task_id_t mitm_src_id, mitm_dest_id;

void app_mitm_passcode_entry_func(ke_task_id_t const src_id, ke_task_id_t const dest_id)
{
    dbg_puts(DBG_CONN_LVL, "    GAPC_TK_EXCH\r\n");
    
    // store task IDs
    mitm_src_id = src_id;
    mitm_dest_id = dest_id;
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

    dbg_printf(DBG_CONN_LVL, "Code: %d\n\r", code);
    
    app_kbd_start_reporting();          // start sending notifications    
    app_state_update(PASSKEY_ENTERED);  // called in asynch mode here!
}


/*
 * Name         : app_send_irk_exch_func - N/A
 *
 * Scope        : PUBLIC
 *
 * Arguments    : param - the message from the Host
 *
 * Description  : N/A for the keyboard app
 *
 * Returns      : void
 *
 */
void app_send_irk_exch_func(struct gapc_bond_req_ind *param)
{
}


/*
 * Name         : app_send_csrk_exch_func - N/A
 *
 * Scope        : PUBLIC
 *
 * Arguments    : param - the message from the Host
 *
 * Description  : N/A for the keyboard app
 *
 * Returns      : void
 *
 */
void app_send_csrk_exch_func(struct gapc_bond_req_ind *param)
{
}


/*
 * Name         : app_send_ltk_exch_func - LTK exchange
 *
 * Scope        : PUBLIC
 *
 * Arguments    : param - the message from the Host
 *
 * Description  : Handles the exchange of the LTK with the Host
 *
 * Returns      : void
 *
 */
void app_send_ltk_exch_func(struct gapc_bond_req_ind *param)
{
    struct gapc_bond_cfm* cfm = KE_MSG_ALLOC(GAPC_BOND_CFM, TASK_GAPC, TASK_APP, gapc_bond_cfm);

    dbg_puts(DBG_CONN_LVL, "--> GAPC_LTK_EXCH\r\n");
    
    // generate ltk
    app_sec_gen_ltk(param->data.key_size);

    cfm->request = GAPC_LTK_EXCH;

    cfm->accept = true;

    cfm->data.ltk.key_size = app_sec_env.key_size;
    cfm->data.ltk.ediv = app_sec_env.ediv;

    memcpy(&(cfm->data.ltk.randnb), &(app_sec_env.rand_nb), RAND_NB_LEN);
    memcpy(&(cfm->data.ltk.ltk), &(app_sec_env.ltk), KEY_LEN);

    ke_msg_send(cfm);
}


/*
 * Name         : app_paired_func - Pairing completed 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : Handles the completion of Pairing
 *
 * Returns      : void
 *
 */
void app_paired_func(void)
{
    dbg_puts(DBG_CONN_LVL, "    GAPC_PAIRING_SUCCEED\r\n");
    
    if (app_sec_env.auth & GAP_AUTH_BOND)
    {
        if (HAS_EEPROM)
        {
            app_alt_pair_store_bond_data();
            eeprom_is_read = false;
        }
    }
            
    app_param_update_start();
}


/*
 * Name         : app_validate_encrypt_req_func - Check if encryption will be accepted
 *
 * Scope        : PUBLIC
 *
 * Arguments    : params - the message from the Host
 *
 * Description  : Check if encryption is goind to be accepted for this link.
 *
 * Returns      : true, if everything is ok
 *              : false, if the request is rejected
 *
 */
bool app_validate_encrypt_req_func(struct gapc_encrypt_req_ind *param)
{
    if (HAS_EEPROM)
    {
        if(((app_sec_env.auth & GAP_AUTH_BOND) != 0)
            && (memcmp(&(app_sec_env.rand_nb), &(param->rand_nb), RAND_NB_LEN) == 0)
            && (app_sec_env.ediv == param->ediv))
        {
            if (HAS_MULTI_BOND)
            {
                // the connecting host is the last host we connected to
                if (multi_bond_enabled)
                {
                    return false;
                }
                // if it's not blocked then no EEPROM access is required to load keys.
            }
        }
        else if (app_alt_pair_load_bond_data(&param->rand_nb, param->ediv) == 1)
            app_state_update(NEW_HOST_EVT);
    }
    
    return true;
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
    app_kbd_start_reporting();          // start sending notifications    
    
    if (HAS_MITM)
        ke_timer_clear(APP_HID_ENC_TIMER, TASK_APP);

        
    // no need to store anything to the EEPROM
}


/*
 * Name         : app_sec_encrypt_ind_func - Enryption indication
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : 
 *
 * Returns      : void
 *
 */
void app_sec_encrypt_ind_func(void)
{
    if (HAS_MULTI_BOND)
    {
        multi_bond_enabled = 0;
    }
    
    app_param_update_start();
}


/*
 * Name         : app_connection_func - Configure app at connection establishment 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : param - parameters passed from the stack
 *
 * Description  : Configures keyboard application when connection is established.
 *
 * Returns      : none
 *
 */
void app_connection_func(struct gapc_connection_req_ind const *param)
{
    // Check if the received Connection Handle was valid
    if (app_env.conidx != GAP_INVALID_CONIDX)
    {
       /*--------------------------------------------------------------
        * ENABLE REQUIRED PROFILES
        *-------------------------------------------------------------*/
        dbg_printf(DBG_ALL, "gap_le_create_conn_req_cmp_evt_handler() (%d, %d, %d, %d)\r\n", 
                (int)param->con_interval,
                (int)param->con_latency,
                (int)param->sup_to,
                (int)param->clk_accuracy
              );

        dbg_printf(DBG_ALL, "Peer addr %02x:%02x:%02x:%02x:%02x:%02x\r\n", 
            param->peer_addr.addr[0], param->peer_addr.addr[1], param->peer_addr.addr[2], 
            param->peer_addr.addr[3], param->peer_addr.addr[4], param->peer_addr.addr[5]);
        
        app_state_update(CONN_REQ_EVT);
                
        ke_state_set(TASK_APP, APP_CONNECTED);  // Update TASK_APP state (MUST always be done!)

        if (HAS_MULTI_BOND)
        {
            if (app_alt_pair_check_peer((struct bd_addr *) &param->peer_addr, param->peer_addr_type) == false)
                return;            
        }
        // Connection is accepted!
        
        app_env.conhdl = param->conhdl;         // Store the connection handle

        app_dis_enable_prf(param->conhdl);  // Enable DIS for this conhdl
                
        app_batt_enable(99, 0, GPIO_PORT_0, GPIO_PIN_0);
        app_batt_poll_start(30000);         // Enable BATT for this conhdl (with fake levels). Start polling every 5min.
        
        app_keyboard_enable();                  // Enable HOGPD for this conhdl
                
        app_kbd_stop_reporting();               // Default is passcode mode
        
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
            app_timer_set(APP_HID_ENC_TIMER, TASK_APP, (KBD_ENC_SAFEGUARD_TIMEOUT / 10));
            dbg_puts(DBG_FSM_LVL, "  (+) HID ENC timer\r\n");
        } 
            
        // Retrieve the connection info from the parameters (MUST always be done!)
        app_env.conhdl = param->conhdl;
        app_env.peer_addr_type = param->peer_addr_type;
        memcpy(app_env.peer_addr.addr, param->peer_addr.addr, BD_ADDR_LEN);
        
#if (BLE_SPOTA_RECEIVER)
        app_spotar_enable();
#endif //BLE_SPOTA_RECEIVER
        
# if (BLE_APP_SEC)
        // send connection confirmation
        app_connect_confirm(app_sec_env.auth);
# else
        // send connection confirmation
        app_connect_confirm(GAP_AUTH_REQ_NO_MITM_NO_BOND);            
# endif
    }
    else
    {
        // No connection has been established. Update state.
        app_state_update(TIMER_EXPIRED_EVT);
    }
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
    uint8_t state = ke_state_get(task_id);

    if ( (state == APP_CONNECTED) || (state == APP_PARAM_UPD) || (state == APP_SECURITY) )
    {
        ke_timer_clear(APP_HID_TIMER, task_id);
        
        if (!HAS_MITM)
        {
            ke_timer_clear(APP_HID_ENC_TIMER, task_id);
        }
        
        app_kbd_stop_reporting();

        app_batt_poll_stop();    // stop battery polling
        
        if (HAS_KEYBOARD_LEDS)
        {
            if (HAS_MULTI_BOND)
            {
                if (!(GetWord16(SYS_STAT_REG) & DBG_IS_UP) && !multi_bond_enabled)
                    leds_set_disconnected();
            }
            else
            {
                if (!(GetWord16(SYS_STAT_REG) & DBG_IS_UP))
                    leds_set_disconnected();
            }
        }
        app_state_update(DISCONN_EVT);
    }
}


/*
 * Name         : app_param_update_func - Request update of connection params 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : After connection and, optionally, pairing is completed, this function 
 *                is called to (optionally) modify the connection parameters.
 *
 * Returns      : void
 *
 */
void app_param_update_func(void)
{
    if (HAS_KBD_SWITCH_TO_PREFERRED_CONN_PARAMS)
    {
        //Set a timer to update connection params
        app_timer_set(APP_HID_TIMER, TASK_APP, (TIME_TO_REQUEST_PARAM_UPDATE / 10));
        dbg_puts(DBG_CONN_LVL, "** Set param update timer\r\n");
    }

    if (!HAS_MITM)
    {
        app_kbd_start_reporting();          // start sending notifications    
    }
    
    if (HAS_KEYBOARD_LEDS)
    {
        leds_set_connection_established();
    }
    
    app_state_update(CONN_CMP_EVT);
}


/*
 * Name         : app_update_params_rejected_func - Handle rejection of connection params 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : status
 *
 * Description  : Actions taken after rejection of a connection update params from the host.
 *
 * Returns      : void
 *
 */
void app_update_params_rejected_func(uint8_t status)
{
    if (status != GAP_ERR_REJECTED)
    {
        ASSERT_INFO(0, param->status, APP_PARAM_UPD);

        // Disconnect
        app_disconnect();
    }
    else
    {
        // it's application specific what to do when the Param Upd request is rejected
        ;
    }
}


/*
 * Name         : app_update_params_complete_func - Handle completion of connection params 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : Actions taken after the recption of the host's reply to a connection update 
 *              : params we sent.
 *
 * Returns      : void
 *
 */
void app_update_params_complete_func(void)
{
    app_state_update(CONN_CMP_EVT);
}


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
    #if (NVDS_SUPPORT)
    if(nvds_get(NVDS_TAG_APP_BLE_ADV_DATA, &app_adv_data_length,
                &app_adv_data[0]) != NVDS_OK)
    #endif //(NVDS_SUPPORT)
    {
        app_adv_data_length = APP_ADV_DATA_LENGTH;
        memcpy(&app_adv_data[0], APP_ADV_DATA, app_adv_data_length);
    }
        
    /*-----------------------------------------------------------------------------
     * Set the Scan Response Data
     *-----------------------------------------------------------------------------*/
    #if (NVDS_SUPPORT)
    if(nvds_get(NVDS_TAG_APP_BLE_SCAN_RESP_DATA, &app_scanrsp_data_length,
                &app_scanrsp_data[0]) != NVDS_OK)
    #endif //(NVDS_SUPPORT)
    {
        app_scanrsp_data_length = APP_SCNRSP_DATA_LENGTH;
        if (app_scanrsp_data_length > 0) 
            memcpy(&app_scanrsp_data[0], APP_SCNRSP_DATA, app_scanrsp_data_length);
    }

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
 * Name         : set_adv_data - Set advertising data in the start adv cmd 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : Sets the advertising and the scan response data in the GAP Start ADV command.
 *
 * Returns      : void
 *
 */
void set_adv_data(struct gapm_start_advertise_cmd *cmd)
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
#if (BLE_SPOTA_RECEIVER)
            case (APP_SPOTAR_TASK):
            {
                // Add spotar Service in the DB
                app_spotar_create_db();
            } break;
#endif //BLE_SPOTA_RECEIVER
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
 * Description  : Action depends on the app state
 *
 * Returns      : KE_MSG_CONSUMED
 *
 */
int app_hid_timer_handler(ke_msg_id_t const msgid,
                           void const *param,
                           ke_task_id_t const dest_id,
                           ke_task_id_t const src_id)
{
	dbg_puts(DBG_FSM_LVL, "HID timer exp\r\n");
	
    app_state_update(TIMER_EXPIRED_EVT);
    
	return (KE_MSG_CONSUMED);
}


/*
 * Name         : app_hid_enc_timer_handler - Handler of the HID Enc Timer 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : <various>
 *
 * Description  : In case encryption is not activated by the remote host and 
 *              : the connection is still alive (if it wasn't then the timer
 *              : would have been cleared), the handler will call
 *              : app_param_update_func() to complete the connection setup.
 *              : This situation appears in certain cases when MITM is not
 *              : used.
 *
 * Returns      : KE_MSG_CONSUMED
 *
 */
int app_hid_enc_timer_handler(ke_msg_id_t const msgid,
                           void const *param,
                           ke_task_id_t const dest_id,
                           ke_task_id_t const src_id)
{
    if (!HAS_MITM)
    {
        dbg_puts(DBG_FSM_LVL, "HID_ENC timer exp\r\n");
        
        app_param_update_func();
        
    }
    return (KE_MSG_CONSUMED);
}


#endif  //BLE_APP_PRESENT
/// @} APP
