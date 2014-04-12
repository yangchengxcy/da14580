/**
****************************************************************************************
*
* @file app_spotar_proj.c
*
* @brief SPOTAR project source code.
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

#include "arch_sleep.h"

#include "app_sec.h"

#include "gapm_task.h"
#include "gapc.h"

#if (BLE_SPOTA_RECEIVER)
#include "app_spotar.h"
#include "app_spotar_proj.h"
#endif

#if (BLE_BATT_SERVER)
#include "app_batt.h"
#endif

#if (BLE_FINDME_TARGET) || (BLE_FINDME_LOCATOR)
#include "app_findme.h"
#endif

#if (BLE_APP_DIS)
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
    
#if (BLE_SPOTA_RECEIVER)    
	app_spotar_init();
#endif
    
    app_dis_init();
    
    app_set_adv_data();
}

void app_sec_init_func(void)
{
    
#if (BLE_APP_SEC)
	app_sec_env.auth = (GAP_AUTH_REQ_MITM_BOND);
#endif
    
}

void app_sec_encrypt_complete_func(void)
{
}

void app_mitm_passcode_entry(ke_task_id_t const src_id, ke_task_id_t const dest_id)
{
}

void app_connection_func(ke_task_id_t task_id, struct gapc_connection_req_ind const *param)
{
    
    /*--------------------------------------------------------------
    * ENABLE REQUIRED PROFILES
    *-------------------------------------------------------------*/

	#if (BLE_SPOTA_RECEIVER)
    app_spotar_enable();
	#endif //BLE_SPOTA_RECEIVER
    
    #if BLE_BATT_SERVER
	batt_level = 0;
	old_batt_level = 0;
	app_batt_enable(batt_level, old_batt_level);
    
	app_batt_poll_start();
	#endif // BLE_BATT_SERVER

	#if BLE_FINDME_TARGET
	app_findt_enable();
	#endif // BLE_FINDME_TARGET
    
    #if BLE_FINDME_LOCATOR
	app_findl_enable();
    #endif //BLE_FINDME_LOCATOR	
    
    #if (BLE_APP_PRESENT)
    app_dis_enable_prf(app_env.conhdl);
    #endif

}

uint8_t              app_adv_data_length __attribute__((section("exchange_mem_case1"))) = 0;
/// Advertising data
uint8_t              app_adv_data[ADV_DATA_LEN-3] __attribute__((section("exchange_mem_case1")));
/// Scan response data length- maximum 31 bytes
uint8_t              app_scanrsp_data_length __attribute__((section("exchange_mem_case1"))) = 0;
/// Scan response data
uint8_t              app_scanrsp_data[SCAN_RSP_DATA_LEN] __attribute__((section("exchange_mem_case1")));

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
    
    
#if BLE_BATT_SERVER
    
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
            #if (BLE_SPOTA_RECEIVER)
            case (APP_SPOTAR_TASK):
            {
                // Add spotar Service in the DB
                app_spotar_create_db();
            } break;
            #endif //BLE_SPOTA_RECEIVER
            #if (BLE_BATT_SERVER)
            case (APP_BASS_TASK):
            {   
                app_batt_create_db();
            } break;
            #endif //BLE_BATT_SERVER
            #if (BLE_APP_DIS)
            case (APP_DIS_TASK):
            {
                app_dis_create_db_send();
            } break;
            #endif //BLE_APP_DIS
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

    return;
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

#endif  //BLE_APP_PRESENT
/// @} APP
