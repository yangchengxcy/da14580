/**
 ****************************************************************************************
 *
 * @file app_sec_task.c
 *
 * @brief Application Security Task implementation
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup APPSECTASK
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_APP_SEC)

#include "app_sec.h"                   // Application Security Definition
#include "app_sec_task.h"              // Application Security Definition
#include "gapc_task.h"                 // GAP Controller Task API
#include "app_api.h"                

#if (NVDS_SUPPORT)
#include "nvds.h"                      // NVDS Definitions
#endif //(NVDS_SUPPORT)

/*
 * GLOBAL VARIABLES DEFINITION
 ****************************************************************************************
 */



/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @brief Handles reception of bond request command
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int gapc_bond_req_ind_handler(ke_msg_id_t const msgid,
        struct gapc_bond_req_ind *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    
    switch(param->request)
    {
        // Bond Pairing request
        case GAPC_PAIRING_REQ:
        {
            app_send_pairing_rsp_func(param);
        }
        break;

        // Used to retrieve pairing Temporary Key
        case GAPC_TK_EXCH:
        {
            if(param->data.tk_type == GAP_TK_DISPLAY)
            {
                app_send_tk_exch_func(param);
            }
            else if (param->data.tk_type == GAP_TK_KEY_ENTRY) 
            {
                app_mitm_passcode_entry_func(src_id, dest_id);
            }
            else
            {
                ASSERT_ERR(0);
            }
        }
        break;

        case GAPC_IRK_EXCH:
            
            app_send_irk_exch_func(param);

            break;
            
        case GAPC_CSRK_EXCH:
        {
            app_send_csrk_exch_func(param);
        }
        break;


        // Used for Long Term Key exchange
        case GAPC_LTK_EXCH:
        {
            app_send_ltk_exch_func(param);
        }
        break;
        
        
        default:
        {
            ASSERT_ERR(0);
        }
        break;
    }
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of bond indication
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int gapc_bond_ind_handler(ke_msg_id_t const msgid,
        struct gapc_bond_ind *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    
    switch(param->info)
    {
        // Bond Pairing request
        case GAPC_PAIRING_SUCCEED:
        {
            // Save Authentication level
            app_sec_env.auth =  param->data.auth;

            if (app_sec_env.auth & GAP_AUTH_BOND)
            {
                app_sec_env.peer_addr_type = app_env.peer_addr_type;
                           
                memcpy(app_sec_env.peer_addr.addr, app_env.peer_addr.addr, BD_ADDR_LEN);
            }
            app_paired_func();
        }
        break;


        case GAPC_PAIRING_FAILED:
        {
            // disconnect
            app_disconnect();

            // clear bond data.
            app_sec_env.auth = 0;
        }
        break;


        case (GAPC_LTK_EXCH):
        {
            // app_store_ltk_func();
        } 
        break;

        
        case (GAPC_IRK_EXCH):
        {
            // app_store_irk_func();
        } 
        break;

        
        case (GAPC_CSRK_EXCH):
        {
            // app_store_csrk_func();
        } 
        break;


        default:
        {
            ASSERT_ERR(0);
        }
        break;
    }
    return (KE_MSG_CONSUMED);
}


/**
 ****************************************************************************************
 * @brief Handles reception of encrypt request command
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int gapc_encrypt_req_ind_handler(ke_msg_id_t const msgid,
        struct gapc_encrypt_req_ind *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapc_encrypt_cfm* cfm = KE_MSG_ALLOC(GAPC_ENCRYPT_CFM, src_id, dest_id, gapc_encrypt_cfm);
    
    if (!app_validate_encrypt_req_func(param))
    {
        cfm->found = false;
    }
    else
    {
        if(((app_sec_env.auth & GAP_AUTH_BOND) != 0)
            && (memcmp(&(app_sec_env.rand_nb), &(param->rand_nb), RAND_NB_LEN) == 0)
            && (app_sec_env.ediv == param->ediv))
        {
            cfm->found = true;
            cfm->key_size = app_sec_env.key_size;
            memcpy(&(cfm->ltk), &(app_sec_env.ltk), KEY_LEN);
            // update connection auth
            app_connect_confirm(app_sec_env.auth);
            app_sec_encrypt_complete_func();
        }
        else
        {
            cfm->found = false;
        }
    }
    
    ke_msg_send(cfm);
    
    if (cfm->found == false)
        app_disconnect();

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of encrypt indication
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int gapc_encrypt_ind_handler(ke_msg_id_t const msgid,
        struct gapc_encrypt_ind *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    app_sec_encrypt_ind_func();

    return (KE_MSG_CONSUMED);
}

#endif //(BLE_APP_SEC)

/// @} APPSECTASK
