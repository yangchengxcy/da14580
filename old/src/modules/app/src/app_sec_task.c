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

#include "app.h"
#include "app_task.h"                  // Application Task API
#include "app_utils.h"                 // arch_printf() etc.

#include "app_sec.h"                   // Application Security Definition
#include "app_sec_task.h"              // Application Security Definition

#include "gapc_task.h"                 // GAP Controller Task API
#include "smpc_task.h"                 // SMPC Task API
#include "smpm_task.h"                 // SMPM Task API

#include "co_error.h"                  // Error Codes Definition
#include "arch.h"                      // Platform Definitions

#if (NVDS_SUPPORT)
#include "nvds.h"                      // NVDS Definitions
#endif //(NVDS_SUPPORT)

#if (DISPLAY_SUPPORT)
#include "app_display.h"               // Application Display Definition
#endif //(DISPLAY_SUPPORT)

#if (BLE_ALT_PAIR)
#include "app_alt_pair.h"
#endif

#if (BLE_HID_DEVICE)
#include "app_keyboard.h"
#include "app_keyboard_proj.h"
#endif

#if (BLE_APP_KEYBOARD_TESTER)
#include "keyboard_tester/app_kbdtest_proj.h"
#endif // (BLE_APP_KEYBOARD_TESTER)

#if (BLE_PROX_REPORTER )
#include "app_proxr_proj.h"
#endif
#if (BLE_SPOTA_RECEIVER )
#include "app_spotar_proj.h"
#endif
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
    arch_printf("----> request: %d\r\n", param->request);
    
    switch(param->request)
    {
        // Bond Pairing request
        case GAPC_PAIRING_REQ:
        {
            struct gapc_bond_cfm* cfm = KE_MSG_ALLOC(GAPC_BOND_CFM, src_id, dest_id, gapc_bond_cfm);

            cfm->request = GAPC_PAIRING_RSP;
            cfm->accept = true;

            // OOB information
            cfm->data.pairing_feat.oob            = GAP_OOB_AUTH_DATA_NOT_PRESENT;
            // Encryption key size
            cfm->data.pairing_feat.key_size       = KEY_LEN;
#if BLE_HID_DEVICE 
# ifdef MITM_REQUIRED      
            // IO capabilities
            cfm->data.pairing_feat.iocap          = GAP_IO_CAP_KB_ONLY;
            // Authentication requirements
            cfm->data.pairing_feat.auth           = GAP_AUTH_REQ_MITM_BOND;
            //Security requirements
            cfm->data.pairing_feat.sec_req        = GAP_SEC1_AUTH_PAIR_ENC;
# else
            // IO capabilities
            cfm->data.pairing_feat.iocap          = GAP_IO_CAP_NO_INPUT_NO_OUTPUT;
            // Authentication requirements
            cfm->data.pairing_feat.auth           = GAP_AUTH_REQ_NO_MITM_BOND;
            //Security requirements
            cfm->data.pairing_feat.sec_req        = GAP_NO_SEC;
# endif // MITM_REQUIRED            
#else            
            // IO capabilities
            cfm->data.pairing_feat.iocap          = GAP_IO_CAP_NO_INPUT_NO_OUTPUT;
            // Authentication requirements
            cfm->data.pairing_feat.auth           = GAP_AUTH_REQ_NO_MITM_BOND;
            //cfm->data.pairing_feat.auth           = GAP_AUTH_REQ_MITM_NO_BOND;
            //Security requirements
            cfm->data.pairing_feat.sec_req        = GAP_NO_SEC;
#endif // BLE_HID_DEVICE           
            //Initiator key distribution
            //GZ cfm->data.pairing_feat.ikey_dist      = GAP_KDIST_NONE;
			cfm->data.pairing_feat.ikey_dist      = GAP_KDIST_SIGNKEY;
            //Responder key distribution
            cfm->data.pairing_feat.rkey_dist      = GAP_KDIST_ENCKEY;

            ke_msg_send(cfm);
        }
        break;

        // Used to retrieve pairing Temporary Key
        case GAPC_TK_EXCH:
        {
            if(param->data.tk_type == GAP_TK_DISPLAY)
            {
                struct gapc_bond_cfm* cfm = KE_MSG_ALLOC(GAPC_BOND_CFM, src_id, dest_id, gapc_bond_cfm);
                uint32_t pin_code = app_sec_gen_tk();
                cfm->request = GAPC_TK_EXCH;
                cfm->accept = true;

                memset(cfm->data.tk.key, 0, KEY_LEN);

//                cfm->data.tk.key[12] = (uint8_t)((pin_code & 0xFF000000) >> 24);
//                cfm->data.tk.key[13] = (uint8_t)((pin_code & 0x00FF0000) >> 16);
//                cfm->data.tk.key[14] = (uint8_t)((pin_code & 0x0000FF00) >>  8);
//                cfm->data.tk.key[15] = (uint8_t)((pin_code & 0x000000FF) >>  0);
                cfm->data.tk.key[3] = (uint8_t)((pin_code & 0xFF000000) >> 24);
                cfm->data.tk.key[2] = (uint8_t)((pin_code & 0x00FF0000) >> 16);
                cfm->data.tk.key[1] = (uint8_t)((pin_code & 0x0000FF00) >>  8);
                cfm->data.tk.key[0] = (uint8_t)((pin_code & 0x000000FF) >>  0);

                #if (DISPLAY_SUPPORT)
                // Display PIN Code on LCD Screen
                app_display_pin_code(pin_code);
                #endif //(DISPLAY_SUPPORT)

                arch_printf("Enter code and press <Enter> key: %ld\r\n", pin_code);
                
                ke_msg_send(cfm);
            }
            else if (param->data.tk_type == GAP_TK_KEY_ENTRY) {
                app_mitm_passcode_entry(src_id, dest_id);
            }
            else
            {
                ASSERT_ERR(0);
            }
        }
        break;
        
#if (BLE_APP_KEYBOARD_TESTER) // only for master?
        case GAPC_IRK_EXCH:
            break;
            
        case GAPC_CSRK_EXCH:
        {
            struct gapc_bond_cfm* cfm = KE_MSG_ALLOC(GAPC_BOND_CFM, src_id, dest_id, gapc_bond_cfm);

            // generate ltk
            app_sec_gen_ltk(param->data.key_size);

            cfm->request = GAPC_CSRK_EXCH;

            cfm->accept = true;

            memset((void *) cfm->data.csrk.key, 0, KEY_LEN);
            memcpy((void *) cfm->data.csrk.key, (void *)"\xAB\xAB\x45\x55\x23\x01", 6);

            ke_msg_send(cfm);
            arch_puts("CSRK set\r\n");
        }
        break;
#endif // (BLE_APP_KEYBOARD_TESTER)

        // Used for Long Term Key exchange
        case GAPC_LTK_EXCH:
        {
            struct gapc_bond_cfm* cfm = KE_MSG_ALLOC(GAPC_BOND_CFM, src_id, dest_id, gapc_bond_cfm);

            // generate ltk
            app_sec_gen_ltk(param->data.key_size);

            cfm->request = GAPC_LTK_EXCH;

            cfm->accept = true;

            cfm->data.ltk.key_size = app_sec_env.key_size;
            cfm->data.ltk.ediv = app_sec_env.ediv;

            memcpy(&(cfm->data.ltk.randnb), &(app_sec_env.rand_nb) , RAND_NB_LEN);
            memcpy(&(cfm->data.ltk.ltk), &(app_sec_env.ltk) , KEY_LEN);

            ke_msg_send(cfm);
            arch_puts("LTK set\r\n");
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
static int gapc_bond_ind_handler(ke_msg_id_t const msgid,
        struct gapc_bond_ind *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    arch_printf("----> info: %d\r\n", param->info);
    
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
                
#if (BLE_ALT_PAIR)
                app_alt_pair_store_bond_data();
#endif
            }
            
            app_param_update_start();
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
static int gapc_encrypt_req_ind_handler(ke_msg_id_t const msgid,
        struct gapc_encrypt_req_ind *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapc_encrypt_cfm* cfm = KE_MSG_ALLOC(GAPC_ENCRYPT_CFM, src_id, dest_id, gapc_encrypt_cfm);
    
#if BLE_ALT_PAIR 
    
    if (alt_pair_enabled)
    {
        if(((app_sec_env.auth & GAP_AUTH_BOND) != 0)
            && (memcmp(&(app_sec_env.rand_nb), &(param->rand_nb), RAND_NB_LEN) == 0)
            && (app_sec_env.ediv == param->ediv))
        {
                cfm->found = false;
                ke_msg_send(cfm);
    
                app_disconnect();

                return (KE_MSG_CONSUMED);
        }
    }
    
    app_alt_pair_load_bond_data(&param->rand_nb, param->ediv);
#endif 

    if(((app_sec_env.auth & GAP_AUTH_BOND) != 0)
        && (memcmp(&(app_sec_env.rand_nb), &(param->rand_nb), RAND_NB_LEN) == 0)
        && (app_sec_env.ediv == param->ediv))
    {
        cfm->found = true;
        cfm->key_size = app_sec_env.key_size;
        memcpy(&(cfm->ltk), &(app_sec_env.ltk), KEY_LEN);
        // update connection auth
        app_connect_confirm(app_sec_env.auth);
        app_sec_encrypt_complete();

#if (BLE_ALT_PAIR)
        app_sec_env.peer_addr_type = app_env.peer_addr_type;
        memcpy(app_sec_env.peer_addr.addr, app_env.peer_addr.addr, BD_ADDR_LEN);

        app_alt_pair_store_bond_data();
#endif
    }
    else
    {
        cfm->found = false;
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
static int gapc_encrypt_ind_handler(ke_msg_id_t const msgid,
        struct gapc_encrypt_ind *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    
#if (BLE_ALT_PAIR)
    alt_pair_enabled = 0;
#endif
    
    app_param_update_start();

    return (KE_MSG_CONSUMED);
}

/*
 * GLOBAL VARIABLES DEFINITION
 ****************************************************************************************
 */

/* Default State handlers definition. */
const struct ke_msg_handler app_sec_default_state[] =
{
    {GAPC_BOND_REQ_IND,        (ke_msg_func_t)gapc_bond_req_ind_handler},
    {GAPC_BOND_IND,            (ke_msg_func_t)gapc_bond_ind_handler},

    {GAPC_ENCRYPT_REQ_IND,     (ke_msg_func_t)gapc_encrypt_req_ind_handler},
    {GAPC_ENCRYPT_IND,         (ke_msg_func_t)gapc_encrypt_ind_handler},
};

/* Specifies the message handlers that are common to all states. */
const struct ke_state_handler app_sec_default_handler = KE_STATE_HANDLER(app_sec_default_state);

/* Defines the place holder for the states of all the task instances. */
ke_state_t app_sec_state[APP_SEC_IDX_MAX] __attribute__((section("exchange_mem_case1"))); //@WIKRETENTION MEMORY 

#endif //(BLE_APP_SEC)

/// @} APPSECTASK
