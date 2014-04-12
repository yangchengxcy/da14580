/**
 ****************************************************************************************
 *
 * @file app.c
 *
 * @brief Application entry point
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
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

#if (BLE_APP_PRESENT)

#include "app_task.h"                // Application task Definition
#include "app.h"                 // Application Definition
#if (BLE_APP_SEC)
#include "app_sec.h"                 // Application security Definition
#endif // (BLE_APP_SEC)

#include "gap.h"                     // GAP Definition
#include "gapm_task.h"               // GAP Manager Task API
#include "gapc_task.h"               // GAP Controller Task API

#include "co_bt.h"                   // Common BT Definition
#include "co_math.h"                 // Common Maths Definition

#if (BLE_APP_HT)
#include "app_ht.h"                  // Health Thermometer Application Definitions
#endif //(BLE_APP_HT)

#if (BLE_APP_ACCEL)
#include "app_accel.h"               // Accelerometer Application Definitions
#endif //(BLE_APP_ACCEL)

#if (BLE_APP_DIS)
#include "app_dis.h"                 // Device Information Service Application Definitions
#endif //(BLE_APP_DIS)

#if (BLE_APP_NEB)
#include "app_neb.h"                 // Nebulizer Service Application Definitions
#endif //(BLE_APP_NEB)

#include "app_proxr_proj.h"             // Proximity Reporter Project definitions

#if (BLE_PROX_REPORTER)
#include "app_proxr.h"                 // Proximity Reporter Profile definitions
#endif //(BLE_PROX_REPORTER)

#if (BLE_HID_DEVICE)
#include "app_keyboard.h"            // Keyboard (HID) Profile definitions
#include "app_keyboard_proj.h"       // Keyboard (HID) Project definitions
#endif //(BLE_HID_DEVICE)

#if (BLE_SPOTA_RECEIVER)
#include "app_spotar.h"            //  SPOTAR application definitions
#include "app_spotar_proj.h" 
#endif //(BLE_SPOTA_RECEIVER)

#if (NVDS_SUPPORT)
#include "nvds.h"                    // NVDS Definitions
#endif //(NVDS_SUPPORT)

#if (DISPLAY_SUPPORT)
#include "app_display.h"             // Application Display Definition
#endif //(DISPLAY_SUPPORT)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */



/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Application Environment Structure
struct app_env_tag app_env __attribute__((section("exchange_mem_case1"))); //@WIKRETENTION MEMORY 

/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Application Task Descriptor
static const struct ke_task_desc TASK_DESC_APP = {NULL, &app_default_handler,
                                                  app_state, APP_STATE_MAX, APP_IDX_MAX};

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */
void app_custom_init(void);

void app_init(void)
{
    #if (NVDS_SUPPORT)
    uint8_t length = NVDS_LEN_SECURITY_ENABLE;
    #endif // NVDS_SUPPORT

    // Reset the environment
    memset(&app_env, 0, sizeof(app_env));

    // Initialize next_prf_init value for first service to add in the database
    app_env.next_prf_init = APP_PRF_LIST_START + 1;

    #if (NVDS_SUPPORT)
    // Get the security enable from the storage
    if (nvds_get(NVDS_TAG_SECURITY_ENABLE, &length, (uint8_t *)&app_env.sec_en) != NVDS_OK)
    #endif // NVDS_SUPPORT
    {
        // Set true by default (several profiles requires security)
        app_env.sec_en = true;
    }

	app_init_func();    
		
    // Create APP task
    ke_task_create(TASK_APP, &TASK_DESC_APP);

    // Initialize Task state
    ke_state_set(TASK_APP, APP_DISABLED);

    #if (BLE_APP_SEC)
    app_sec_init();
    #endif // (BLE_APP_SEC)
}


bool app_db_init(void)
{
    
    // Indicate if more services need to be added in the database
    bool end_db_create = false;
    
    end_db_create = app_db_init_func();
    
    
    return end_db_create;
}

void app_disconnect(void)
{
    struct gapc_disconnect_cmd *cmd = KE_MSG_ALLOC(GAPC_DISCONNECT_CMD,
                                              KE_BUILD_ID(TASK_GAPC, app_env.conidx), TASK_APP,
                                              gapc_disconnect_cmd);

    cmd->operation = GAPC_DISCONNECT;
    cmd->reason = CO_ERROR_REMOTE_USER_TERM_CON;

    // Send the message
    ke_msg_send(cmd);
}


void app_connect_confirm(uint8_t auth)
{
    // confirm connection
    struct gapc_connection_cfm *cfm = KE_MSG_ALLOC(GAPC_CONNECTION_CFM,
            KE_BUILD_ID(TASK_GAPC, app_env.conidx), TASK_APP,
            gapc_connection_cfm);

    cfm->auth = auth;
    cfm->authorize = GAP_AUTHZ_NOT_SET;

    // Send the message
    ke_msg_send(cfm);
}


/**
 ****************************************************************************************
 * Advertising Functions
 ****************************************************************************************
 */

void app_adv_start(void)
{
    //  Device Name Length
    uint8_t device_name_length;
    uint8_t device_name_avail_space;
    uint8_t device_name_temp_buf[64];

    // Allocate a message for GAP
    struct gapm_start_advertise_cmd *cmd = KE_MSG_ALLOC(GAPM_START_ADVERTISE_CMD,
                                                TASK_GAPM, TASK_APP,
                                                gapm_start_advertise_cmd);

    cmd->op.code     = GAPM_ADV_UNDIRECT;
    cmd->op.addr_src = GAPM_PUBLIC_ADDR;
    cmd->intv_min    = APP_ADV_INT_MIN;
    cmd->intv_max    = APP_ADV_INT_MAX;
    cmd->channel_map = APP_ADV_CHMAP;

    cmd->info.host.mode = GAP_GEN_DISCOVERABLE;

    /*-----------------------------------------------------------------------------------
     * Set the Advertising Data and the Scan Response Data
     *---------------------------------------------------------------------------------*/
    cmd->info.host.adv_data_len       = APP_ADV_DATA_MAX_SIZE;
    cmd->info.host.scan_rsp_data_len  = APP_SCAN_RESP_DATA_MAX_SIZE;

    // Advertising Data
    #if (NVDS_SUPPORT)
    if(nvds_get(NVDS_TAG_APP_BLE_ADV_DATA, &cmd->info.host.adv_data_len,
                &cmd->info.host.adv_data[0]) != NVDS_OK)
    #endif //(NVDS_SUPPORT)
    {
        cmd->info.host.adv_data_len = APP_DFLT_ADV_DATA_LEN;
        memcpy(&cmd->info.host.adv_data[0], APP_DFLT_ADV_DATA, cmd->info.host.adv_data_len);

        //Add list of UUID
        #if (BLE_APP_HT)
        cmd->info.host.adv_data_len += APP_HT_ADV_DATA_UUID_LEN;
        memcpy(&cmd->info.host.adv_data[APP_DFLT_ADV_DATA_LEN], APP_HT_ADV_DATA_UUID, APP_HT_ADV_DATA_UUID_LEN);
        #else
            #if (BLE_APP_NEB)
            cmd->info.host.adv_data_len += APP_NEB_ADV_DATA_UUID_LEN;
            memcpy(&cmd->info.host.adv_data[APP_DFLT_ADV_DATA_LEN], APP_NEB_ADV_DATA_UUID, APP_NEB_ADV_DATA_UUID_LEN);
            #endif //(BLE_APP_NEB)
        #endif //(BLE_APP_HT)
    }

    // Scan Response Data
    #if (NVDS_SUPPORT)
    if(nvds_get(NVDS_TAG_APP_BLE_SCAN_RESP_DATA, &cmd->info.host.scan_rsp_data_len,
                &cmd->info.host.scan_rsp_data[0]) != NVDS_OK)
    #endif //(NVDS_SUPPORT)
    {
        cmd->info.host.scan_rsp_data_len = APP_SCNRSP_DATA_LENGTH;
        memcpy(&cmd->info.host.scan_rsp_data[0], APP_SCNRSP_DATA, cmd->info.host.scan_rsp_data_len);
    }

    // Get remaining space in the Advertising Data - 2 bytes are used for name length/flag
    device_name_avail_space = APP_ADV_DATA_MAX_SIZE - cmd->info.host.adv_data_len - 2;

    // Check if data can be added to the Advertising data
    if (device_name_avail_space > 0)
    {
        // Get the Device Name to add in the Advertising Data (Default one or NVDS one)
        #if (NVDS_SUPPORT)
        device_name_length = NVDS_LEN_DEVICE_NAME;
        if (nvds_get(NVDS_TAG_DEVICE_NAME, &device_name_length, &device_name_temp_buf[0]) != NVDS_OK)
        #endif //(NVDS_SUPPORT)
        {
            // Get default Device Name (No name if not enough space)
            device_name_length = strlen(APP_DFLT_DEVICE_NAME);
            memcpy(&device_name_temp_buf[0], APP_DFLT_DEVICE_NAME, device_name_length);
        }

        if(device_name_length > 0)
        {
            // Check available space
            device_name_length = co_min(device_name_length, device_name_avail_space);

            // Fill Length
            cmd->info.host.adv_data[cmd->info.host.adv_data_len]     = device_name_length + 1;
            // Fill Device Name Flag
            cmd->info.host.adv_data[cmd->info.host.adv_data_len + 1] = '\x09';
            // Copy device name
            memcpy(&cmd->info.host.adv_data[cmd->info.host.adv_data_len + 2], device_name_temp_buf, device_name_length);

            // Update Advertising Data Length
            cmd->info.host.adv_data_len += (device_name_length + 2);
        }
    }

    app_adv_func(cmd);
    
    // Send the message
    ke_msg_send(cmd);

    #if (DISPLAY_SUPPORT)
    // Update advertising state screen
    app_display_set_adv(true);
    #endif //DISPLAY_SUPPORT

    // We are now connectable
    ke_state_set(TASK_APP, APP_CONNECTABLE);
}

void app_adv_stop(void)
{
    // Disable Advertising
    struct gapm_cancel_cmd *cmd = KE_MSG_ALLOC(GAPM_CANCEL_CMD,
                                           TASK_GAPM, TASK_APP,
                                           gapm_cancel_cmd);

    cmd->operation = GAPM_CANCEL;

    // Send the message
    ke_msg_send(cmd);
}

#if (BLE_APP_SEC)
void app_security_start(/*struct bd_addr p_addr*/)
{
    // Send security request command
    struct gapc_security_cmd * cmd = KE_MSG_ALLOC(GAPC_SECURITY_CMD,
            KE_BUILD_ID(TASK_GAPC, app_env.conidx), TASK_APP, gapc_security_cmd);

    // Security request
    cmd->operation = GAPC_SECURITY_REQ;

    cmd->auth      = ((app_sec_env.auth == 0) ? GAP_AUTH_REQ_NO_MITM_NO_BOND : app_sec_env.auth);

    // Send the message
    ke_msg_send(cmd);

    // Go to security State
    ke_state_set(TASK_APP, APP_SECURITY);
}
#endif // (BLE_APP_SEC)

void app_param_update_start(void)
{
    app_param_update_func();
}

#if (BLE_APP_SEC)
void app_sec_encrypt_complete(void)
{
    app_sec_encrypt_complete_func();
}
#endif // (BLE_APP_SEC)

#else
void app_init()
{
}
#endif //(BLE_APP_PRESENT)

/// @} APP
