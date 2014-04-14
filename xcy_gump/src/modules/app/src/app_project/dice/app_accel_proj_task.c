/**
 ****************************************************************************************
 *
 * @file app_prox_proj_task.c
 *
 * @brief proximity project application task. 
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup APPTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"               // SW configuration

#if (BLE_APP_PRESENT) && (BLE_ACCEL)

#include "app_task.h"                  // Application Task API
#include "app.h"                       // Application Definition
#include "gapc_task.h"                 // GAP Controller Task API
#include "gapm_task.h"                 // GAP Manager Task API
#include "gap.h"                       // GAP Definitions
#include "co_error.h"                  // Error Codes Definition
#include "arch.h"                      // Platform Definitions
#include "battery.h"                      // Platform Definitions

#if BLE_ACCEL
#include "app_accel_proj.h"
#include "app_accel_proj_task.h"
#include "accel_task.h"
#endif

#include "co_math.h"                 // Common Maths Definition

#if (NVDS_SUPPORT)
#include "nvds.h"                    // NVDS Definitions
#endif //(NVDS_SUPPORT)

#if BLE_ACCEL
#define APP_ADV_DATA      "\x03\x03\xa0\xff"
//#define APP_ADV_DATA      "\x02\x01\x06\x03\x03\x09\x18"  //Thermometer! FIXME! Discuss with Steven!
#define APP_ADV_DATA_LENGTH (4)
#define APP_ADV_DATA_LEN (4)
#define APP_DEVICE_NAME "DA14580-Dice5"
#endif

#if BLE_ACCEL
#define APP_SCNRSP_DATA   "\x09\xFF\x00\x60\x52\x57\x2D\x42\x4C\x45"
#define APP_SCNRSP_DATA_LENGTH (10)
#endif

extern uint8_t accel_adv_interval1;
extern uint8_t accel_adv_interval2;
extern uint8_t accel_adv_interval3;

extern uint8_t accel_con_interval;

extern uint8_t accel_mode;
extern uint8_t accel_latency;
extern uint8_t accel_window;


/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */
void app_task_custom_init()
{
    accel_adv_interval1 = 30;	//GZ
    accel_adv_interval2 = 0;
    accel_adv_interval3 = 0;

    accel_con_interval = 10;

    accel_mode = 0;
    accel_latency = 0x8;
    accel_window = 0xFF;	
}

#if BLE_ACCEL
extern uint8_t accel_con_interval __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY
#endif

void app_custom_connection(struct gapc_connection_req_ind const *param)
{
#if BLE_ACCEL
    int temp=4;
    uint8_t *temp_v;
    
    app_accel_adv_stopped();
    
    if (!param->con_latency)
    {
        // Not completely verified, but this improves the stability of the connection.
        
        struct gapc_param_update_req_ind * req = KE_MSG_ALLOC(GAPC_PARAM_UPDATE_REQ_IND, TASK_GAPC, TASK_APP, gapc_param_update_req_ind);

        // Fill in the parameter structure
        //req->conhdl = param->conn_info.conhdl;
        //GZ req->conn_par.intv_min = param->conn_info.con_interval;
        //GZ req->conn_par.intv_max = param->conn_info.con_interval;
        attmdb_att_get_value(ACCEL_HANDLE(ACCEL_IDX_ACCEL_X_EN), &(temp), &(temp_v));
        if(temp_v[1] > 1)
            accel_con_interval = temp_v[1];
        
        req->params.intv_min = (accel_con_interval-1)*10/1.25;
        req->params.intv_max = (accel_con_interval)*10/1.25;
        req->params.latency = param->con_latency;
        if(param->sup_to * 8 / 1.25 <= 3200)
            req->params.time_out = param->sup_to * 8 / 1.25;
        else
            req->params.time_out = 3200/1.25;
    #if BLE_HID_DEVICE
        puts("Send GAP_PARAM_UPDATE_REQ");
    #endif
        ke_msg_send(req);
    }    
#endif
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
//GZ
#if BLE_ACCEL    
    
    //  Device Name Length
    uint8_t device_name_length;

    app_adv_data_length     = APP_ADV_DATA_MAX_SIZE;
    app_scanrsp_data_length = APP_SCAN_RESP_DATA_MAX_SIZE;

    /*-----------------------------------------------------------------------------
     * Set the Advertising Data
     *-----------------------------------------------------------------------------*/
 //   if(nvds_get(NVDS_TAG_APP_BLE_ADV_DATA, &app_env.app_adv_data_length, &app_env.app_adv_data[0]) != NVDS_OK)
    {
        app_adv_data_length = APP_ADV_DATA_LENGTH;
        memcpy(&app_adv_data[0], APP_ADV_DATA, app_adv_data_length);
    }
#if 1    
		{
		uint8_t tb;
		//adv element length
		tb = 4;
    memcpy(&app_adv_data[app_adv_data_length], &tb, 1);
		app_adv_data_length += 1;
		//adv element type
		tb = 0xFF;		//manufacturer specific data
    memcpy(&app_adv_data[app_adv_data_length], &tb, 1);
		app_adv_data_length += 1;
		//adv element manufacturer id
		tb = 0xFF;		//manufacturer specific data
    memcpy(&app_adv_data[app_adv_data_length], &tb, 1);
		app_adv_data_length += 1;
    memcpy(&app_adv_data[app_adv_data_length], &tb, 1);
		app_adv_data_length += 1;

	//adv element battery level
	tb = battery_get_lvl(BATT_CR2032);
    memcpy(&app_adv_data[app_adv_data_length], &tb, 1);
		app_adv_data_length += 1;
		}
#endif		
    /*-----------------------------------------------------------------------------
     * Set the Scan Response Data
     *-----------------------------------------------------------------------------*/
//    if(nvds_get(NVDS_TAG_APP_BLE_SCAN_RESP_DATA, &app_env.app_scanrsp_data_length, &app_env.app_scanrsp_data[0]) != NVDS_OK)
    {
        app_scanrsp_data_length = APP_SCNRSP_DATA_LENGTH;
        if (app_scanrsp_data_length > 0) memcpy(&app_scanrsp_data, APP_SCNRSP_DATA, app_scanrsp_data_length);
    }
		
#if !BLE_HID_DEVICE
		
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
            device_name_length = (device_name_length<strlen(APP_DFLT_DEVICE_NAME))?device_name_length:strlen(APP_DFLT_DEVICE_NAME);
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
#endif		

#if 0    
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
            device_name_length = (device_name_length<strlen(APP_DFLT_DEVICE_NAME))?device_name_length:strlen(APP_DFLT_DEVICE_NAME);
            memcpy(&app_scanrsp_data[app_scanrsp_data_length + 2], APP_DFLT_DEVICE_NAME, device_name_length);
        }

        // Length
        app_scanrsp_data[app_scanrsp_data_length]     = device_name_length + 1;
        // Device Name Flag
        app_scanrsp_data[app_scanrsp_data_length + 1] = '\x09';

        // Update Advertising Data Length
        app_scanrsp_data_length += (device_name_length + 2);
    }
#endif    
#endif
}

void app_adv_func(struct gapm_start_advertise_cmd *cmd)
{
#if 1
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
    
    app_set_adv_data();
    
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

    if(app_scanrsp_data_length != 0)
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
#else
    
     //  Device Name Length
    uint8_t device_name_length;
    int8_t device_name_avail_space;
    uint8_t device_name_temp_buf[64];

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
        cmd->info.host.adv_data_len = APP_ADV_DATA_LEN;
        memcpy(&cmd->info.host.adv_data[0], APP_ADV_DATA, cmd->info.host.adv_data_len);
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

#if 1    
		{
		uint8_t tb;
		//adv element length
		tb = 4;
    memcpy(&cmd->info.host.adv_data[cmd->info.host.adv_data_len], &tb, 1);
		cmd->info.host.adv_data_len += 1;
		//adv element type
		tb = 0xFF;		//manufacturer specific data
    memcpy(&cmd->info.host.adv_data[cmd->info.host.adv_data_len], &tb, 1);
		cmd->info.host.adv_data_len += 1;
		//adv element manufacturer id
		tb = 0xFF;		//manufacturer specific data
    memcpy(&cmd->info.host.adv_data[cmd->info.host.adv_data_len], &tb, 1);
		cmd->info.host.adv_data_len += 1;
    memcpy(&cmd->info.host.adv_data[cmd->info.host.adv_data_len], &tb, 1);
		cmd->info.host.adv_data_len += 1;

	//adv element battery level
	tb = battery_get_lvl(BATT_CR2032);
    memcpy(&cmd->info.host.adv_data[cmd->info.host.adv_data_len], &tb, 1);
		cmd->info.host.adv_data_len += 1;
		}
#endif		
    
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
            device_name_length = strlen(APP_DEVICE_NAME);
            memcpy(&device_name_temp_buf[0], APP_DEVICE_NAME, device_name_length);
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
#endif
    
#if BLE_ACCEL
    // Start adv schedule timer the first time we start advertising (after reset, or disconnect) 
    if( accel_adv_count == 0 /*accel_adv_interval == 0 || accel_adv_interval == APP_ADV_INT_MIN */)
    {
        app_accel_adv_started();
        accel_adv_interval = 0x20;  //0;
    }
    
    cmd->intv_min = accel_adv_interval;
    cmd->intv_max = accel_adv_interval;
#endif	
    
}

void app_param_update_func(void)
{
#if 1 
    int temp=4;    
    uint8_t *temp_v;

    attmdb_att_get_value(ACCEL_HANDLE(ACCEL_IDX_ACCEL_X_EN), &(temp), &(temp_v));
    if(temp_v[1] > 1)
        accel_con_interval = temp_v[1]-1;
    if(accel_con_interval < 2 || accel_con_interval > 500)
        return;
    
    // Send a param update request
    struct gapc_param_update_cmd *cmd = KE_MSG_ALLOC(GAPC_PARAM_UPDATE_CMD,
            KE_BUILD_ID(TASK_GAPC, app_env.conidx), TASK_APP,
            gapc_param_update_cmd);

    // Fill up parameters
    cmd->operation = GAPC_UPDATE_PARAMS;
#if 0
    cmd->params.intv_min = 70;         // 10ms (8*1.25ms)
    cmd->params.intv_max = 80;        // 20ms (16*1.25ms)
    cmd->params.latency  = 0;
//GZ    cmd->params.time_out = 100;
    cmd->params.time_out = 320;
#endif
    
    cmd->params.intv_min = (accel_con_interval-1)*10/1.25;
    cmd->params.intv_max = (accel_con_interval)*10/1.25;
    cmd->params.latency = 0;
    if(accel_con_interval < 30)
        cmd->params.time_out = accel_con_interval*32;
    else
        cmd->params.time_out = 10000;
        
    // Send message
    ke_msg_send(cmd);

//GZ tmp
rwip_env.sleep_enable = false;
    
    // Go to Param update state
    ke_state_set(TASK_APP, APP_PARAM_UPD);
#endif

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
                // set device configuration
                cmd->operation = GAPM_SET_DEV_CONFIG;
                // Device Role
                cmd->role = GAP_PERIPHERAL_SLV;
                // Device IRK
                // cmd->irk = ; TODO NOT set

                // Device Appearance
                #if (BLE_APP_HT)
                cmd->appearance = 728;
                #else
                // Device Appearance
                cmd->appearance = 0x0000;
                #endif

                // Device Appearance write permission requirements for peer device
                cmd->appearance_write_perm = GAPM_WRITE_DISABLE;
                // Device Name write permission requirements for peer device
                cmd->name_write_perm = GAPM_WRITE_DISABLE;

                // Peripheral only: *****************************************************************
                // Slave preferred Minimum of connection interval
                cmd->con_intv_min = 8;         // 10ms (8*1.25ms)
                // Slave preferred Maximum of connection interval
                cmd->con_intv_max = 16;        // 20ms (16*1.25ms)
                // Slave preferred Connection latency
                cmd->con_latency  = 0;
                // Slave preferred Link supervision timeout
                cmd->superv_to    = 100;

                // Privacy settings bit field
                cmd->flags = 0;
    
    return;   
}

void app_adv_undirect_complete(uint8_t status)
{
    
}

void app_adv_direct_complete(uint8_t status)
{
    
}

#endif //(BLE_APP_PRESENT)

/// @} APPTASK
