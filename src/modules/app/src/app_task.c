/**
 ****************************************************************************************
 *
 * @file app_task.c
 *
 * @brief RW APP Task implementation
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
#include <string.h> 

#if (BLE_APP_PRESENT)

#include "app_task.h"                  // Application Task API
#include "app.h"                       // Application Definition
#include "app_sec.h"                   // Application Definition
#include "gapc_task.h"                 // GAP Controller Task API
#include "gapm_task.h"                 // GAP Manager Task API
#include "gap.h"                       // GAP Definitions
#include "gapc.h"                      // GAPC Definitions
#include "co_error.h"                  // Error Codes Definition
#include "arch.h"                      // Platform Definitions
#include "app_sec_task.h"              // Application Security Task API

#if (BLE_APP_HT)
#include "app_ht.h"                    // Application Heath Thermometer Definition
#endif //(BLE_APP_HT)

#if (BLE_APP_DIS)
#include "app_dis.h"                   // Application Device Information Service Definition
#endif //(BLE_APP_DIS)

#if (BLE_ACCEL)
#include "app_accel.h"                 // Application Accelerometer Definition
#include "accel_task.h"
extern int8_t update_conn_params __attribute__((section("exchange_mem_case1")));
#endif //(BLE_APP_ACCEL)

#if (BLE_APP_NEB)
#include "app_neb.h"                   // Application Nebulizer Definition
#endif //(BLE_APP_NEB)

#if (DISPLAY_SUPPORT)
#include "app_display.h"               // Application Display Definition
#endif //(DISPLAY_SUPPORT)

#include "app_proxr_proj.h"
#include "app_proxr_proj_task.h"

#if (BLE_STREAMDATA_DEVICE)
#include "streamdatad_task.h"
#include "app_stream_proj_task.h"
#include "l2cc_task.h"
#endif //BLE_STREAMDATA_DEVICE 

#if (BLE_APP_KEYBOARD)
#include "hogpd_task.h"
#include "app_keyboard_proj.h"         // Application Keyboard (HID) Definition
#include "app_keyboard_proj_task.h"
#endif // (BLE_APP_KEYBOARD)

#if (BLE_APP_KEYBOARD_TESTER)
#include "hogprh.h"
#include "hogprh_task.h"
#include "keyboard_tester/app_kbdtest_proj.h"
#endif // (BLE_APP_KEYBOARD_TESTER)

#if (BLE_ALT_PAIR)
#include "app_alt_pair.h"
#endif // (BLE_ALT_PAIR)

#if BLE_ACCEL
#include "app_accel_proj.h"
#include "app_accel_proj_task.h"
#endif

#if (BLE_SPOTA_RECEIVER )
#include "spotar_task.h"
#include "app_spotar_proj.h"
#include "app_spotar_proj_task.h"
#endif
#if (BLE_APP_SEC)
extern struct app_sec_env_tag app_sec_env;
#endif

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles ready indication from the GAP. - Reset the stack
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gapm_device_ready_ind_handler(ke_msg_id_t const msgid,
                                         void const *param,
                                         ke_task_id_t const dest_id,
                                         ke_task_id_t const src_id)
{
    if (ke_state_get(dest_id) == APP_DISABLED)
    {
        // reset the lower layers.
        struct gapm_reset_cmd* cmd = KE_MSG_ALLOC(GAPM_RESET_CMD, TASK_GAPM, TASK_APP,
                gapm_reset_cmd);

        cmd->operation = GAPM_RESET;

        ke_msg_send(cmd);
    }
    else
    {
        // APP_DISABLED state is used to wait the GAP_READY_EVT message
        ASSERT_ERR(0);
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles GAP manager command complete events.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

static int gapm_cmp_evt_handler(ke_msg_id_t const msgid,
                                struct gapm_cmp_evt const *param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id)
{
    
    switch(param->operation)
    {
        // reset completed
        case GAPM_RESET:
        {
            if(param->status != GAP_ERR_NO_ERROR)
            {
                ASSERT_ERR(0); // unexpected error
            }
            else
            {
                // set device configuration
                struct gapm_set_dev_config_cmd* cmd = KE_MSG_ALLOC(GAPM_SET_DEV_CONFIG_CMD,
                        TASK_GAPM, TASK_APP, gapm_set_dev_config_cmd);

#if !(BLE_APP_KEYBOARD_TESTER)
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
#endif                
                app_configuration_func(dest_id, cmd);

                ke_msg_send(cmd);
            }
        }
        break;

        // device configuration updated
        case GAPM_SET_DEV_CONFIG:
        {
            if(param->status != GAP_ERR_NO_ERROR)
            {
                ASSERT_ERR(0); // unexpected error
            }
            else
            {
#if (BLE_APP_KEYBOARD_TESTER)
                // We are now initiating a connection
                ke_state_set(dest_id, APP_CONNECTABLE);
                
                app_connect_func();
#else
                // We are now in Initialization State
                ke_state_set(dest_id, APP_DB_INIT);

                // Add the first required service in the database
                if (app_db_init())
                {
                    // No service to add in the DB -> Start Advertising
                    app_adv_start();
                }
#endif                
            }
        }
        break;

        // Advertising finished
        case GAPM_ADV_UNDIRECT:
        {
            int i =0;
            
            i++;
            
            if(param->status != GAP_ERR_NO_ERROR)
            {
                ASSERT_ERR(0); // unexpected error
            }
        }
        break;
        
        // Directed advertising finished
        case GAPM_ADV_DIRECT:
        {
            if (ke_state_get(dest_id) == APP_CONNECTABLE)
                app_adv_start(); //restart advertising
        }
        break;

        default:
        {
            ASSERT_ERR(0); // unexpected error
        }
        break;
    }


    return (KE_MSG_CONSUMED);
}


/**
 ****************************************************************************************
 * @brief Handles connection complete event from the GAP. Will enable profile.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gapc_connection_req_ind_handler(ke_msg_id_t const msgid,
                                           struct gapc_connection_req_ind const *param,
                                           ke_task_id_t const dest_id,
                                           ke_task_id_t const src_id)
{
    // Connection Index

    if (ke_state_get(dest_id) == APP_CONNECTABLE)
    {
        app_env.conidx = KE_IDX_GET(src_id);

        // Check if the received Connection Handle was valid
        if (app_env.conidx != GAP_INVALID_CONIDX)
        {
            
            ke_state_set(dest_id, APP_CONNECTED);
            
            app_connection_func(dest_id, param);
            
            // Retrieve the connection info from the parameters
            app_env.conhdl = param->conhdl;

            app_env.peer_addr_type = param->peer_addr_type;
            memcpy(app_env.peer_addr.addr, param->peer_addr.addr, BD_ADDR_LEN);
            
#if BLE_ALT_PAIR        
            if (app_alt_pair_check_peer((struct bd_addr *) &param->peer_addr, param->peer_addr_type) == false)
                return (KE_MSG_CONSUMED);            
        
            //app_alt_pair_load_bond_data(&app_env.peer_addr, app_env.peer_addr_type);
#endif      
            
#if (BLE_APP_KEYBOARD_TESTER) // only for master?
# if (BLE_APP_SEC)
            // send connection confirmation
            app_connect_confirm(app_sec_env.auth);
# else // (BLE_APP_SEC)
            // send connection confirmation
            app_connect_confirm(GAP_AUTH_REQ_NO_MITM_NO_BOND);            
# endif // (BLE_APP_SEC)

#else // (BLE_APP_KEYBOARD_TESTER)

# if (BLE_APP_SEC)
            
            // send connection confirmation
            app_connect_confirm(app_sec_env.auth);
            
            // TODO [FBE] add connection confirm message

//            if(memcmp((uint8 *)param->peer_addr.addr, (uint8 *)app_sec_env.peer_addr.addr, BD_ADDR_LEN) )
//                memset((uint8 *)&app_sec_env, 0, sizeof(struct app_sec_env_tag));
            
            if( (gapc_get_role(app_env.conidx) == ROLE_MASTER) && (app_env.sec_en) && (!app_sec_env.auth) ) // Optional security step
            {
                // Start security
                app_security_start();
            }
            else // Param update step
# else // (BLE_APP_SEC)
            // send connection confirmation
            app_connect_confirm(GAP_AUTH_REQ_NO_MITM_NO_BOND);            
# endif // (BLE_APP_SEC)
            {
                // Start param update
                app_param_update_start();
            }
#endif // (BLE_APP_KEYBOARD_TESTER)            
        }
        else
        {
            // No connection has been establish, restart advertising
            app_adv_start();
        }
    }
    else
    {
        // APP_CONNECTABLE state is used to wait the GAP_LE_CREATE_CONN_REQ_CMP_EVT message
        ASSERT_ERR(0);
    }

    return (KE_MSG_CONSUMED);
}



/**
 ****************************************************************************************
 * @brief Handles GAP controller command complete events.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gapc_cmp_evt_handler(ke_msg_id_t const msgid,
                                struct gapc_cmp_evt const *param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id)
{
    switch(param->operation)
    {
        // reset completed
        case GAPC_UPDATE_PARAMS:
        {
            if (ke_state_get(dest_id) == APP_PARAM_UPD)
            {
                if ((param->status != CO_ERROR_NO_ERROR))
                {
                    ASSERT_INFO(0, param->status, APP_PARAM_UPD);

                    // Disconnect
                    app_disconnect();
                }
                else
                {
                    // Go to Connected State
                    ke_state_set(dest_id, APP_CONNECTED);
                }
            }
#if BLE_ACCEL            
//GZ tmp
            //rwip_env.sleep_enable = true;
            update_conn_params = 1;
            ke_timer_set(APP_ACCEL_TIMER, dest_id, 5);
#endif
        }
        break;

        default:
        {
            if(param->status != GAP_ERR_NO_ERROR)
            {
                ASSERT_ERR(0); // unexpected error
            }
        }
        break;
    }


    return (KE_MSG_CONSUMED);
}


/**
 ****************************************************************************************
 * @brief Handles disconnection complete event from the GAP.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gapc_disconnect_ind_handler(ke_msg_id_t const msgid,
                                      struct gapc_disconnect_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    uint8_t state = ke_state_get(dest_id);

    
    app_disconnect_func(dest_id, param);
    
#if !(BLE_APP_KEYBOARD_TESTER)
    if ((state == APP_SECURITY) || (state == APP_CONNECTED)  || (state == APP_PARAM_UPD))
    {
        // Restart Advertising
        app_adv_start();
    }
    else
    {
        // We are not in a Connected State
        ASSERT_ERR(0);
    }
#endif // (BLE_APP_KEYBOARD_TESTER)

    return (KE_MSG_CONSUMED);
}

//
//static int gap_bond_req_cmp_evt_handler(ke_msg_id_t const msgid,
//                                        struct gap_bond_req_cmp_evt *param,
//                                        ke_task_id_t const dest_id,
//                                        ke_task_id_t const src_id)
//{
//    if (ke_state_get(dest_id) == APP_SECURITY)
//    {
//        if ((param->status != CO_ERROR_NO_ERROR))
//        {
//            ASSERT_INFO(0, param->status, 0);
//
//            // Disconnect
//            app_disconnect();
//        }
//        else
//        {
//            // Start param update
//            app_param_update_start();
//        }
//    }
//    else
//    {
//        ASSERT_INFO(0, ke_state_get(dest_id), 0);
//    }
//
//    return (KE_MSG_CONSUMED);
////}

/**
 ****************************************************************************************
 * @brief Handles reception of the APP_MODULE_INIT_CMP_EVT messgage
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int app_module_init_cmp_evt_handler(ke_msg_id_t const msgid,
                                           const struct app_module_init_cmp_evt *param,
                                           ke_task_id_t const dest_id,
                                           ke_task_id_t const src_id)
{
    if (ke_state_get(dest_id) == APP_DB_INIT)
    {
        if (param->status == CO_ERROR_NO_ERROR)
        {
            // Add next required service in the database
            if (app_db_init())
            {
                // No more service to add in the database, start advertising
                app_adv_start();
            }
        }
        else
        {
            // An error has occurred during database creation
            ASSERT_ERR(0);
        }
    }
    else
    {
        // APP_DB_INIT state is used to wait the APP_MODULE_INIT_CMP_EVT message
        ASSERT_ERR(0);
    }

    return (KE_MSG_CONSUMED);
}
//
//#if (BLE_APP_SEC)
///**
// ****************************************************************************************
// * @brief Handles reception of the APP_SECURITY_CMP_EVT messgage
// * @param[in] msgid     Id of the message received.
// * @param[in] param     Pointer to the parameters of the message.
// * @param[in] dest_id   ID of the receiving task instance
// * @param[in] src_id    ID of the sending task instance.
// *
// * @return If the message was consumed or not.
// ****************************************************************************************
// */
//static int app_pairing_cmp_evt_handler(ke_msg_id_t const msgid,
//                                       const struct app_pairing_cmp_evt *param,
//                                       ke_task_id_t const dest_id,
//                                       ke_task_id_t const src_id)
//{
//    if (ke_state_get(dest_id) == APP_SECURITY)
//    {
//        // Go to Connected state
//        ke_state_set(dest_id, APP_CONNECTED);
//
//        if (param->status != CO_ERROR_NO_ERROR)
//        {
//            // Disconnect
//            app_disconnect();
//        }
//        else
//        {
//            app_param_update_start();
//        }
//    }
//    else
//    {
//        // APP_SECURITY state is used to wait the APP_SECURITY_CMP_EVT message
//        ASSERT_ERR(0);
//    }
//
//    return (KE_MSG_CONSUMED);
//}
//#endif //(BLE_APP_SEC)

/*
 * GLOBAL VARIABLES DEFINITION
 ****************************************************************************************
 */

/* Default State handlers definition. */
const struct ke_msg_handler app_default_state[] =
{
    {GAPM_DEVICE_READY_IND,                 (ke_msg_func_t)gapm_device_ready_ind_handler},
    {GAPM_CMP_EVT,                          (ke_msg_func_t)gapm_cmp_evt_handler},

    {GAPC_CMP_EVT,                          (ke_msg_func_t)gapc_cmp_evt_handler},
    {GAPC_CONNECTION_REQ_IND,               (ke_msg_func_t)gapc_connection_req_ind_handler},
    {GAPC_DISCONNECT_IND,                   (ke_msg_func_t)gapc_disconnect_ind_handler},

    {APP_MODULE_INIT_CMP_EVT,               (ke_msg_func_t)app_module_init_cmp_evt_handler},

	//{GAPC_BOND_REQ_IND,	                    (ke_msg_func_t)gapc_bond_req_ind_handler},

#if BLE_ACCEL
    {ACCEL_START_IND,                       (ke_msg_func_t)accel_start_ind_handler},
    {ACCEL_STOP_IND,                        (ke_msg_func_t)accel_stop_ind_handler},
    {ACCEL_WRITE_LINE_IND,                  (ke_msg_func_t)accel_write_line_ind_handler},
    {APP_ACCEL_TIMER,                       (ke_msg_func_t)app_accel_timer_handler},
    {APP_ACCEL_ADV_TIMER,                   (ke_msg_func_t)app_accel_adv_timer_handler},
    {ACCEL_CREATE_DB_CFM,                   (ke_msg_func_t)accel_create_db_cfm_handler},
    {APP_ACCEL_MSG,							(ke_msg_func_t)accel_msg_handler},

#endif //BLE_ACCEL

#if BLE_PROX_REPORTER
    {PROXR_ALERT_IND,                       (ke_msg_func_t)proxr_alert_ind_handler},
    {APP_PXP_TIMER,                         (ke_msg_func_t)app_proxr_timer_handler},
    {PROXR_CREATE_DB_CFM,                   (ke_msg_func_t)proxr_create_db_cfm_handler},
    {PROXR_DISABLE_IND,                     (ke_msg_func_t)proxr_disable_ind_handler},
    {LLC_RD_TX_PW_LVL_CMP_EVT,				(ke_msg_func_t)llc_rd_tx_pw_lvl_cmp_evt_handler},			
#endif //BLE_PROXR_REPORTER	

#if (BLE_APP_KEYBOARD)
    {HOGPD_CREATE_DB_CFM,                   (ke_msg_func_t)keyboard_create_db_cfm_handler},
    {HOGPD_DISABLE_IND,                     (ke_msg_func_t)keyboard_disable_ind_handler},
    {HOGPD_NTF_SENT_CFM,                    (ke_msg_func_t)keyboard_ntf_sent_cfm_handler},
	{APP_HID_TIMER,                         (ke_msg_func_t)app_hid_timer_handler},
#endif    

#if (BLE_APP_KEYBOARD_TESTER)
    {HOGPRH_ENABLE_CFM,                     (ke_msg_func_t)hogprh_enable_cfm_handler},
    {HOGPRH_RD_CHAR_ERR_RSP,                (ke_msg_func_t)hogprh_err_rsp_handler},
    {HOGPRH_WR_CHAR_RSP,                    (ke_msg_func_t)hogprh_err_rsp_handler},
    {HOGPRH_REPORT_IND,                     (ke_msg_func_t)hogprh_report_ind_handler},
    {HOGPRH_DISABLE_IND,                    (ke_msg_func_t)hogprh_disable_ind_handler},
	{APP_HID_TIMER,                         (ke_msg_func_t)app_hid_timer_handler},
    {HOGPRH_REPORT_MAP_RD_RSP,              (ke_msg_func_t)hopgrh_report_map_rd_rsp_handler},
    {GAPC_PARAM_UPDATE_REQ_IND,             (ke_msg_func_t)gapc_param_update_req_ind_handler},
#endif

#if BLE_BATT_SERVER
    {BASS_CREATE_DB_CFM,                    (ke_msg_func_t)batt_create_db_cfm_handler},
    {BASS_BATT_LEVEL_UPD_CFM,               (ke_msg_func_t)batt_level_upd_cfm_handler},
    {BASS_BATT_LEVEL_NTF_CFG_IND,           (ke_msg_func_t)batt_level_ntf_cfg_ind_handler},
    {APP_BATT_TIMER,                        (ke_msg_func_t)app_batt_timer_handler},
    {APP_BATT_ALERT_TIMER,                  (ke_msg_func_t)app_batt_alert_timer_handler},
#endif //(BLE_BATT_SERVER)

#if BLE_FINDME_TARGET
    {FINDT_ALERT_IND,                       (ke_msg_func_t)findt_alert_ind_handler},
#endif //BLE_FINDME_TARGET
	
#if BLE_FINDME_LOCATOR
    {FINDL_ENABLE_CFM,					    (ke_msg_func_t)findl_enable_cfm_handler},
#endif //BLE_FINDME_LOCATOR

#if (BLE_ALT_PAIR)
    {APP_ALT_PAIR_TIMER,                    (ke_msg_func_t)app_alt_pair_timer_handler},
    {APP_DIR_ADV_TIMER,                     (ke_msg_func_t)app_directed_adv_timer_handler},
#endif

#if (BLE_STREAMDATA_DEVICE)
	{STREAMDATAD_CREATE_DB_CFM,             (ke_msg_func_t)stream_create_db_cfm_handler},
    {L2CC_DATA_SEND_RSP,                    (ke_msg_func_t)stream_more_data_handler},
    {STREAMDATAD_START_IND,                 (ke_msg_func_t)stream_start_ind_handler}, // tell the app that the host has enabled notifications
    {STREAMDATAD_STOP_IND,                  (ke_msg_func_t)stream_stop_ind_handler},  // tell the app that the host has disabled notifications
#endif //BLE_STREAMDATA_DEVICE    
 
#if BLE_SPOTA_RECEIVER
    {SPOTAR_PATCH_MEM_DEV_IND,              (ke_msg_func_t)spotar_patch_mem_dev_ind_handler},
    {SPOTAR_GPIO_MAP_IND,                   (ke_msg_func_t)spotar_gpio_map_ind_handler},
    {SPOTAR_PATCH_LEN_IND,                  (ke_msg_func_t)spotar_patch_len_ind_handler}, 
    {SPOTAR_PATCH_DATA_IND,                 (ke_msg_func_t)spotar_patch_data_ind_handler},   
    {SPOTAR_CREATE_DB_CFM,                  (ke_msg_func_t)spotar_create_db_cfm_handler},
#endif //BLE_SPOTA_RECEIVER	    
};

/* Specifies the message handlers that are common to all states. */
const struct ke_state_handler app_default_handler = KE_STATE_HANDLER(app_default_state);

/* Defines the place holder for the states of all the task instances. */
ke_state_t app_state[APP_IDX_MAX] __attribute__((section("exchange_mem_case1"))); //@WIKRETENTION MEMORY 

#endif //(BLE_APP_PRESENT)

/// @} APPTASK
