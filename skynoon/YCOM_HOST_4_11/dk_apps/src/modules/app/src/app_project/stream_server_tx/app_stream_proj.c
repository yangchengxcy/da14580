/**
****************************************************************************************
*
* @file app_stream_proj.c
*
* @brief Stream project source code.
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

#include "arch_sleep.h"

#include "app_sec.h"

#include "gapm_task.h"
#include "gapc.h"

#include "app_stream_proj.h"
#include "app_stream.h"

#if (BLE_BATT_SERVER)
#include "app_batt.h"
#endif

#if (BLE_DIS_SERVER)
#include "app_dis.h"
#endif

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
*/
void app_task_custom_init(void);
void app_set_adv_data(void);

 /**
 ****************************************************************************************
 * @brief Initialize Proximity Apllication GPIO ports.
 *
 * @return void.
 ****************************************************************************************
 */
 
void app_init_func(void)
{	
    
    app_stream_init();
    
    app_dis_init();
    
    app_set_adv_data();


    //Call one of the folowing functions to select sleep mode
    //app_set_deep_sleep();

    //app_set_extended_sleep();

    app_disable_sleep();

}

void app_sec_init_func(void)
{
    
#if (BLE_APP_SEC)
	app_sec_env.auth = (GAP_AUTH_REQ_MITM_BOND);
#endif
    
}

bool app_connection_func(ke_task_id_t task_id, struct gapc_connection_req_ind const *param)
{
    
    /*--------------------------------------------------------------
    * ENABLE REQUIRED PROFILES
    *-------------------------------------------------------------*/

    #if BLE_STREAMDATA_DEVICE
	app_stream_enable();
    #endif // BLE_STREAMDATA_DEVICE
    
    #if BLE_BATT_SERVER
	batt_level = 0;
	old_batt_level = 0;
	app_batt_enable(batt_level, old_batt_level);
    
	app_batt_poll_start();
	#endif // BLE_BATT_SERVER	
    
    #if (BLE_APP_PRESENT)
    app_dis_enable_prf(app_env.conhdl);
    #endif

    return true;

}

uint8_t              app_adv_data_length __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY
/// Advertising data
uint8_t              app_adv_data[ADV_DATA_LEN-3] __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY
/// Scan response data length- maximum 31 bytes
uint8_t              app_scanrsp_data_length __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY
/// Scan response data
uint8_t              app_scanrsp_data[SCAN_RSP_DATA_LEN] __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY

void app_set_adv_data(void)
{
    //  Device Name Length
    uint8_t device_name_length;

    app_adv_data_length     = APP_ADV_DATA_MAX_SIZE;
    app_scanrsp_data_length = APP_SCAN_RESP_DATA_MAX_SIZE;

    /*-----------------------------------------------------------------------------
     * Set the Advertising Data
     *-----------------------------------------------------------------------------*/
 //   if(nvds_get(NVDS_TAG_APP_BLE_ADV_DATA, &app_env.app_adv_data_length, &app_env.app_adv_data[0]) != NVDS_OK)
    {
        app_adv_data_length = APP_DFLT_ADV_DATA_LEN;
        memcpy(&app_adv_data[0], APP_DFLT_ADV_DATA, app_adv_data_length);
    }
		
    /*-----------------------------------------------------------------------------
     * Set the Scan Response Data
     *-----------------------------------------------------------------------------*/
//    if(nvds_get(NVDS_TAG_APP_BLE_SCAN_RESP_DATA, &app_env.app_scanrsp_data_length, &app_env.app_scanrsp_data[0]) != NVDS_OK)
    {
        app_scanrsp_data_length = APP_SCNRSP_DATA_LENGTH;
        if (app_scanrsp_data_length > 0) memcpy(&app_scanrsp_data, APP_SCNRSP_DATA, app_scanrsp_data_length);
    }
		

		
    /*-----------------------------------------------------------------------------
     * Add the Device Name in the Advertising Data
     *-----------------------------------------------------------------------------*/
    // Get available space in the Advertising Data
    device_name_length = APP_ADV_DATA_MAX_SIZE - app_adv_data_length - 2;

    // Check if data can be added to the Advertising data
    if (device_name_length > 0)
    {
        // Get the Device Name to add in the Advertising Data (Default one or NVDS one)
//        if (nvds_get(NVDS_TAG_DEVICE_NAME, &device_name_length, &app_env.app_adv_data[app_env.app_adv_data_length + 2]) != NVDS_OK)
        {
            // Get default Device Name (No name if not enough space)
            //GZ device_name_length = (strlen(APP_DFLT_DEVICE_NAME) < device_name_length) ? strlen(APP_DFLT_DEVICE_NAME) : 0;
            device_name_length = strlen(APP_DFLT_DEVICE_NAME);
            memcpy(&app_adv_data[app_adv_data_length + 2], APP_DFLT_DEVICE_NAME, device_name_length);
        }

        // Length
        app_adv_data[app_adv_data_length]     = device_name_length + 1;
        // Device Name Flag
        app_adv_data[app_adv_data_length + 1] = '\x09';

        // Update Advertising Data Length
        //app_adv_data_length = app_adv_data_length+ device_name_length + 2;
        app_adv_data_length = app_adv_data_length + 2;
        app_adv_data_length = app_adv_data_length + device_name_length;
    }
		

    /*-----------------------------------------------------------------------------
     * Add the Device Name in the Advertising Scan Response Data
     *-----------------------------------------------------------------------------*/
    // Get available space in the Advertising Data
    device_name_length = APP_ADV_DATA_MAX_SIZE - app_scanrsp_data_length - 2;

    // Check if data can be added to the Advertising data
    if (device_name_length > 0)
    {
        // Get the Device Name to add in the Advertising Data (Default one or NVDS one)
//        if (nvds_get(NVDS_TAG_DEVICE_NAME, &device_name_length, &app_env.app_scanrsp_data[app_env.app_scanrsp_data_length + 2]) != NVDS_OK)
        {
            // Get default Device Name (No name if not enough space)
            //GZ device_name_length = (strlen(APP_DFLT_DEVICE_NAME) < device_name_length) ? strlen(APP_DFLT_DEVICE_NAME) : 0;
            device_name_length = strlen(APP_DFLT_DEVICE_NAME);
            memcpy(&app_scanrsp_data[app_scanrsp_data_length + 2], APP_DFLT_DEVICE_NAME, device_name_length);
        }

        // Length
        app_scanrsp_data[app_scanrsp_data_length]     = device_name_length + 1;
        // Device Name Flag
        app_scanrsp_data[app_scanrsp_data_length + 1] = '\x09';

        // Update Advertising Data Length
        app_scanrsp_data_length += (device_name_length + 2);
    }
}

void app_adv_func(struct gapm_start_advertise_cmd *cmd)
{
//    return;
#if 0    
    if(app_adv_data_length != 0)
    {
        if(1)   //if(cmd->info.host.adv_data_len == APP_DFLT_ADV_DATA_LEN)
        {
            memcpy(&cmd->info.host.adv_data[0], app_adv_data, app_adv_data_length);
            cmd->info.host.adv_data_len = app_adv_data_length;
        }
        else
        {
            memcpy(&cmd->info.host.adv_data[cmd->info.host.adv_data_len], app_adv_data, app_adv_data_length);
            cmd->info.host.adv_data_len += app_adv_data_length;
        }
    }

    if(0)   //if(app_scanrsp_data_length != 0)
    {
        if(1)   //if(cmd->info.host.scan_rsp_data_len == APP_SCNRSP_DATA_LEN)
        {
            memcpy(&cmd->info.host.scan_rsp_data[0], app_scanrsp_data, app_scanrsp_data_length);
            cmd->info.host.scan_rsp_data_len = app_scanrsp_data_length;
        }
        else
        {
            memcpy(&cmd->info.host.scan_rsp_data[cmd->info.host.scan_rsp_data_len], app_scanrsp_data, app_scanrsp_data_length);
            cmd->info.host.scan_rsp_data_len += app_scanrsp_data_length;
        }
    }
#endif    
}

void app_send_disconnect(uint16_t dst, uint16_t conhdl, uint8_t reason)
{
    struct gapc_disconnect_ind * disconnect_ind = KE_MSG_ALLOC(GAPC_DISCONNECT_IND,
            dst, TASK_APP, gapc_disconnect_ind);

    // fill parameters
    disconnect_ind->conhdl   = conhdl;
    disconnect_ind->reason   = reason;

    // send indication
    ke_msg_send(disconnect_ind);
}


void app_disconnect_func(ke_task_id_t task_id, struct gapc_disconnect_ind const *param)
{
    
#if(BLE_STREAMDATA_DEVICE)
    app_send_disconnect(TASK_STREAMDATAD, param->conhdl, param->reason);
#endif
    
#if BLE_BATT_SERVER
    app_send_disconnect(TASK_BASS, param->conhdl, param->reason);
    
	app_batt_stop();
#endif // BLE_BATT_SERVER
						
    
}

bool app_db_init_func(void)
{
    
    // Indicate if more services need to be added in the database
    bool end_db_create = false;
    
    // Check if another should be added in the database
    if (app_env.next_prf_init < APP_PRF_LIST_STOP)
    {
        switch (app_env.next_prf_init)
        {
            #if (BLE_STREAMDATA_DEVICE)
            case (APP_STREAM_TASK):
            {
                // Add proxr Service in the DB
                app_stream_create_db();
            } break;
            #endif //BLE_PROX_REPORTER
            #if (BLE_BATT_SERVER)
            case (APP_BASS_TASK):
            {   
                app_batt_create_db();
            } break;
            #endif //BLE_BATT_SERVER
            #if (BLE_DIS_SERVER)
            case (APP_DIS_TASK):
            {
                app_dis_create_db_send();
            } break;
            #endif //BLE_DIS_SERVER
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

void app_param_update_func(void)
{
#if 0    
    // Send a param update request
    struct gapc_param_update_cmd *cmd = KE_MSG_ALLOC(GAPC_PARAM_UPDATE_CMD,
            KE_BUILD_ID(TASK_GAPC, app_env.conidx), TASK_APP,
            gapc_param_update_cmd);

    // Fill up parameters
    cmd->operation = GAPC_UPDATE_PARAMS;

    cmd->params.intv_min = 9;         // 10ms (8*1.25ms)
    cmd->params.intv_max = 9;        // 10ms (8*1.25ms)
    cmd->params.latency  = 0;

    cmd->params.time_out = 320;

    // Send message
    ke_msg_send(cmd);
    
    // Go to Param update state
    ke_state_set(TASK_APP, APP_PARAM_UPD);
#endif    
    return;
}

void app_sec_encrypt_complete_func(void)
{
}

void app_mitm_passcode_entry(ke_task_id_t const src_id, ke_task_id_t const dest_id)
{
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
    return;   
}

/**
 ****************************************************************************************
 * @brief app_api function. Called upon device's configuration completion
 *
 * @return void.
 ****************************************************************************************
*/

void app_set_dev_config_complete_func(void)
{
    // We are now in Initialization State
    ke_state_set(TASK_APP, APP_DB_INIT);

    // Add the first required service in the database
    if (app_db_init())
    {
        // No service to add in the DB -> Start Advertising
        app_adv_start();
    }
 
    return;
}

/**
 ****************************************************************************************
 * @brief app_api function. Called upon connection param's update rejection
 *
 * @param[in] status        Error code
 *
 * @return void.
 ****************************************************************************************
*/

void app_update_params_rejected_func(uint8_t status)
{
    return;
}

/**
 ****************************************************************************************
 * @brief app_api function. Called upon connection param's update completion
 *
 * @return void.
 ****************************************************************************************
*/

void app_update_params_complete_func(void)
{
    return;
}

#if (BLE_APP_SEC)
void app_send_pairing_rsp_func(struct gapc_bond_req_ind *param)
{
    
    struct gapc_bond_cfm* cfm = KE_MSG_ALLOC(GAPC_BOND_CFM, TASK_GAPC, TASK_APP, gapc_bond_cfm);

    cfm->request = GAPC_PAIRING_RSP;
    cfm->accept = true;

    // OOB information
    cfm->data.pairing_feat.oob            = GAP_OOB_AUTH_DATA_NOT_PRESENT;
    // Encryption key size
    cfm->data.pairing_feat.key_size       = KEY_LEN;
    // IO capabilities
    cfm->data.pairing_feat.iocap          = GAP_IO_CAP_NO_INPUT_NO_OUTPUT;
    // Authentication requirements
    cfm->data.pairing_feat.auth           = GAP_AUTH_REQ_NO_MITM_BOND;
    //Security requirements
    cfm->data.pairing_feat.sec_req        = GAP_NO_SEC;
    //Initiator key distribution
    //GZ cfm->data.pairing_feat.ikey_dist      = GAP_KDIST_NONE;
    cfm->data.pairing_feat.ikey_dist      = GAP_KDIST_SIGNKEY;
    //Responder key distribution
    cfm->data.pairing_feat.rkey_dist      = GAP_KDIST_ENCKEY;
    
    ke_msg_send(cfm);
}

void app_send_tk_exch_func(struct gapc_bond_req_ind *param)
{
    struct gapc_bond_cfm* cfm = KE_MSG_ALLOC(GAPC_BOND_CFM, TASK_GAPC, TASK_APP, gapc_bond_cfm);
    uint32_t pin_code = app_sec_gen_tk();
    cfm->request = GAPC_TK_EXCH;
    cfm->accept = true;
    
    memset(cfm->data.tk.key, 0, KEY_LEN);
    
    cfm->data.tk.key[3] = (uint8_t)((pin_code & 0xFF000000) >> 24);
    cfm->data.tk.key[2] = (uint8_t)((pin_code & 0x00FF0000) >> 16);
    cfm->data.tk.key[1] = (uint8_t)((pin_code & 0x0000FF00) >>  8);
    cfm->data.tk.key[0] = (uint8_t)((pin_code & 0x000000FF) >>  0);
    
    ke_msg_send(cfm);
}

void app_send_irk_exch_func(struct gapc_bond_req_ind *param)
{
    return;
}

void app_send_csrk_exch_func(struct gapc_bond_req_ind *param)
{
    struct gapc_bond_cfm* cfm = KE_MSG_ALLOC(GAPC_BOND_CFM, TASK_GAPC, TASK_APP, gapc_bond_cfm);

    // generate ltk
    app_sec_gen_ltk(param->data.key_size);

    cfm->request = GAPC_CSRK_EXCH;

    cfm->accept = true;

    memset((void *) cfm->data.csrk.key, 0, KEY_LEN);
    memcpy((void *) cfm->data.csrk.key, (void *)"\xAB\xAB\x45\x55\x23\x01", 6);

    ke_msg_send(cfm);

}

void app_send_ltk_exch_func(struct gapc_bond_req_ind *param)
{
    
    struct gapc_bond_cfm* cfm = KE_MSG_ALLOC(GAPC_BOND_CFM, TASK_GAPC, TASK_APP, gapc_bond_cfm);

    // generate ltk
    app_sec_gen_ltk(param->data.key_size);

    cfm->request = GAPC_LTK_EXCH;

    cfm->accept = true;

    cfm->data.ltk.key_size = app_sec_env.key_size;
    cfm->data.ltk.ediv = app_sec_env.ediv;

    memcpy(&(cfm->data.ltk.randnb), &(app_sec_env.rand_nb) , RAND_NB_LEN);
    memcpy(&(cfm->data.ltk.ltk), &(app_sec_env.ltk) , KEY_LEN);

    ke_msg_send(cfm);

}

void app_paired_func(void)
{
    return;
}

bool app_validate_encrypt_req_func(struct gapc_encrypt_req_ind *param)
{
    return true;
}

void app_sec_encrypt_ind_func(void)
{
    
    return; 
}
#endif //BLE_APP_SEC

#endif  //BLE_APP_PRESENT
/// @} APP
