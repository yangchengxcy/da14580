/**
 ****************************************************************************************
 *
 * @file sample128_task.c
 *
 * @brief Sample128 Task implementation.
 *
 * @brief 128 UUID service. sample code
 *
 * Copyright (C) 2013 Dialog Semiconductor GmbH and its Affiliates, unpublished work
 * This computer program includes Confidential, Proprietary Information and is a Trade Secret 
 * of Dialog Semiconductor GmbH and its Affiliates. All use, disclosure, and/or 
 * reproduction is prohibited unless authorized in writing. All Rights Reserved.
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
*/

#include "rwble_config.h"

#if (BLE_SAMPLE128)

#include "gap.h"
#include "gapc.h"
#include "gattc_task.h"
#include "atts_util.h"
#include "sample128.h"
#include "sample128_task.h"
#include "attm_cfg.h"
#include "attm_db.h"
#include "prf_utils.h"

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref SAMPLE128_CREATE_DB_REQ message.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int sample128_create_db_req_handler(ke_msg_id_t const msgid,
                                       struct sample128_create_db_req const *param,
                                       ke_task_id_t const dest_id,
                                       ke_task_id_t const src_id)
{
    //Database Creation Status
    uint8_t status;
    uint8_t nb_att_16;
    uint8_t nb_att_128;
    uint8_t nb_att_32;
    uint16_t att_decl_svc = ATT_DECL_PRIMARY_SERVICE;
    uint16_t att_decl_char = ATT_DECL_CHARACTERISTIC;
    uint16_t sample128_1_val_hdl;
    uint16_t sample128_1_char_hdl; 

    //Save Profile ID
    sample128_env.con_info.prf_id = TASK_SAMPLE128;

    /*---------------------------------------------------*
     * Link Loss Service Creation
     *---------------------------------------------------*/

        //Add Service Into Database
        nb_att_16 = 2; // 2 UUID16 atts
        nb_att_32 = 0;// No UUID32 att
        nb_att_128 = 1; //1 UUID128 att
        status = attmdb_add_service(&(sample128_env.sample128_1_shdl), TASK_SAMPLE128,
                                             nb_att_16, nb_att_32, nb_att_128, 36); //total attributte size = 36, 16 (svc)  + 19 (desc_char) + 1 (att)

        if (status == ATT_ERR_NO_ERROR)
        {
            //add svc attribute
            status = attmdb_add_attribute(sample128_env.sample128_1_shdl, ATT_UUID_128_LEN, //Data size = 16 (ATT_UUID_128_LEN)
                                               ATT_UUID_16_LEN, (uint8_t*)&att_decl_svc, PERM(RD, ENABLE),
                                               &(sample128_env.sample128_1_shdl));
            
            
            
            status = attmdb_att_set_value(sample128_env.sample128_1_shdl, ATT_UUID_128_LEN, (uint8_t *)sample128_1_svc.uuid);
            
            
            
            //add char attribute
            status = attmdb_add_attribute(sample128_env.sample128_1_shdl, ATT_UUID_128_LEN + 3, //Data size = 19 (ATT_UUID_128_LEN + 3)
                                               ATT_UUID_16_LEN, (uint8_t*) &att_decl_char, PERM(RD, ENABLE),
                                               &(sample128_1_char_hdl));
            
            
            
            //add val attribute
            status = attmdb_add_attribute(sample128_env.sample128_1_shdl, sizeof(uint8_t), //Data size = 1
                                               ATT_UUID_128_LEN, (uint8_t*)&sample128_1_val.uuid, PERM(RD, ENABLE) | PERM(WR, ENABLE),
                                               &(sample128_1_val_hdl));
            
            memcpy(sample128_1_char.attr_hdl, &sample128_1_val_hdl, ATT_UUID_16_LEN);
            
            status = attmdb_att_set_value(sample128_1_char_hdl, sizeof(sample128_1_char), (uint8_t *)&sample128_1_char);
            
            //Disable sample128_1 service
            attmdb_svc_set_permission(sample128_env.sample128_1_shdl, PERM(SVC, DISABLE));

            //Go to Idle State
    
            //If we are here, database has been fulfilled with success, go to idle state
            ke_state_set(TASK_SAMPLE128, SAMPLE128_IDLE);
        }
        
        //Send CFM to application
        struct sample128_create_db_cfm * cfm = KE_MSG_ALLOC(SAMPLE128_CREATE_DB_CFM, src_id,
                                                    TASK_SAMPLE128, sample128_create_db_cfm);
        cfm->status = status;
        ke_msg_send(cfm);
    

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Enable the Sample128 role, used after connection.
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int sample128_enable_req_handler(ke_msg_id_t const msgid,
                                    struct sample128_enable_req const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    // Keep source of message, to respond to it further on
    sample128_env.con_info.appid = src_id;
    // Store the connection handle for which this profile is enabled
    sample128_env.con_info.conidx = gapc_get_conidx(param->conhdl);

    // Check if the provided connection exist
    if (sample128_env.con_info.conidx == GAP_INVALID_CONIDX)
    {
        // The connection doesn't exist, request disallowed
        prf_server_error_ind_send((prf_env_struct *)&sample128_env, PRF_ERR_REQ_DISALLOWED,
                                  SAMPLE128_ERROR_IND, SAMPLE128_ENABLE_REQ);
    }
    else
    {
        // Sample128 1
        attmdb_svc_set_permission(sample128_env.sample128_1_shdl, param->sec_lvl);

        //Set LLS Alert Level to specified value
        attmdb_att_set_value(sample128_env.sample128_1_shdl + SAMPLE128_1_IDX_VAL,
                             sizeof(uint8_t), (uint8_t *)&param->sample128_1_val);

        // Go to Connected state
        ke_state_set(TASK_SAMPLE128, SAMPLE128_CONNECTED);
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATT_WRITE_CMD_IND message.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gattc_write_cmd_ind_handler(ke_msg_id_t const msgid,
                                      struct gattc_write_cmd_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    uint8_t char_code = SAMPLE128_ERR_CHAR;
    uint8_t status = PRF_APP_ERROR;

    if (KE_IDX_GET(src_id) == sample128_env.con_info.conidx)
    {
        if (param->handle == sample128_env.sample128_1_shdl + SAMPLE128_1_IDX_VAL)
        {
            char_code = SAMPLE128_1_CHAR;
        }

        if (char_code != SAMPLE128_ERR_CHAR)
        {
            
            //Save value in DB
            attmdb_att_set_value(param->handle, sizeof(uint8_t), (uint8_t *)&param->value[0]);
            
            if(param->last)
            {
                sample128_send_val(param->value[0]);
            }

            status = PRF_ERR_OK;
            
            // Send Write Response
            atts_write_rsp_send(sample128_env.con_info.conidx, param->handle, status);
            
        }
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Disconnection indication to sample128.
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gap_disconnnect_ind_handler(ke_msg_id_t const msgid,
                                        struct gapc_disconnect_ind const *param,
                                        ke_task_id_t const dest_id,
                                        ke_task_id_t const src_id)
{

    // Check Connection Handle
    if (KE_IDX_GET(src_id) == sample128_env.con_info.conidx)
    {
        
        // In any case, inform APP about disconnection
        sample128_disable();
    }

    return (KE_MSG_CONSUMED);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Disabled State handler definition.
const struct ke_msg_handler sample128_disabled[] =
{
    {SAMPLE128_CREATE_DB_REQ,   (ke_msg_func_t) sample128_create_db_req_handler },
};

/// Idle State handler definition.
const struct ke_msg_handler sample128_idle[] =
{
    {SAMPLE128_ENABLE_REQ,      (ke_msg_func_t) sample128_enable_req_handler },
};

/// Connected State handler definition.
const struct ke_msg_handler sample128_connected[] =
{
    {GATTC_WRITE_CMD_IND,    (ke_msg_func_t) gattc_write_cmd_ind_handler},
};

/// Default State handlers definition
const struct ke_msg_handler sample128_default_state[] =
{
    {GAPC_DISCONNECT_IND,    (ke_msg_func_t) gap_disconnnect_ind_handler},
};

/// Specifies the message handler structure for every input state.
const struct ke_state_handler sample128_state_handler[SAMPLE128_STATE_MAX] =
{
    [SAMPLE128_DISABLED]    = KE_STATE_HANDLER(sample128_disabled),
    [SAMPLE128_IDLE]        = KE_STATE_HANDLER(sample128_idle),
    [SAMPLE128_CONNECTED]   = KE_STATE_HANDLER(sample128_connected),
};

/// Specifies the message handlers that are common to all states.
const struct ke_state_handler sample128_default_handler = KE_STATE_HANDLER(sample128_default_state);

/// Defines the place holder for the states of all the task instances.
ke_state_t sample128_state[SAMPLE128_IDX_MAX] __attribute__((section("exchange_mem_case1")));

#endif //BLE_SAMPLE128

