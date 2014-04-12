/**
 ****************************************************************************************
 *
 * @file app_dis_task.c
 *
 * @brief Device Information Service Application Task
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

#include "rwip_config.h"        // SW Configuration
#include <string.h>             // srtlen()

#if (BLE_APP_DIS)

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "diss_task.h"          // Device Information Service Server Task API
#include "diss.h"               // Device Information Service Definitions

#include "app_dis.h"            // Device Information Service Application Definitions
#include "app_dis_task.h"       // Device Information Service Application Task API

#include "app_task.h"           // Application Task API

#include "ke_task.h"


/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

static int diss_create_db_cfm_handler(ke_msg_id_t const msgid,
                                      struct diss_create_db_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    uint8 len;
    
    if (ke_state_get(dest_id) == APP_DIS_DISABLED)
    {
        if (param->status == CO_ERROR_NO_ERROR)
        {
            // Go to the idle state
            ke_state_set(dest_id, APP_DIS_IDLE);

            {
                // Set Manufacturer Name value in the DB
                struct diss_set_char_val_req *req_name = KE_MSG_ALLOC_DYN(DISS_SET_CHAR_VAL_REQ,
                                                                          TASK_DISS, TASK_APP_DIS,
                                                                          diss_set_char_val_req,
                                                                          APP_DIS_MANUFACTURER_NAME_LEN);

                // Fill in the parameter structure
                req_name->char_code     = DIS_MANUFACTURER_NAME_CHAR;
                req_name->val_len       = APP_DIS_MANUFACTURER_NAME_LEN;
                memcpy(&req_name->val[0], APP_DIS_MANUFACTURER_NAME, APP_DIS_MANUFACTURER_NAME_LEN);

                // Send the message
                ke_msg_send(req_name);
            }

            // Set Model Number String value in the DB
            {
                struct diss_set_char_val_req *req_mod = KE_MSG_ALLOC_DYN(DISS_SET_CHAR_VAL_REQ,
                                                                         TASK_DISS, TASK_APP_DIS,
                                                                         diss_set_char_val_req,
                                                                         APP_DIS_MODEL_NB_STR_LEN);

                // Fill in the parameter structure
                req_mod->char_code     = DIS_MODEL_NB_STR_CHAR;
                req_mod->val_len       = APP_DIS_MODEL_NB_STR_LEN;
                memcpy(&req_mod->val[0], APP_DIS_MODEL_NB_STR, APP_DIS_MODEL_NB_STR_LEN);

                // Send the message
                ke_msg_send(req_mod);
            }

            // Set System ID value in the DB
            {
                struct diss_set_char_val_req *req_id = KE_MSG_ALLOC_DYN(DISS_SET_CHAR_VAL_REQ,
                                                                        TASK_DISS, TASK_APP_DIS,
                                                                        diss_set_char_val_req,
                                                                        APP_DIS_SYSTEM_ID_LEN);

                // Fill in the parameter structure
                req_id->char_code     = DIS_SYSTEM_ID_CHAR;
                req_id->val_len       = APP_DIS_SYSTEM_ID_LEN;
                memcpy(&req_id->val[0], APP_DIS_SYSTEM_ID, APP_DIS_SYSTEM_ID_LEN);

                // Send the message
                ke_msg_send(req_id);
            }            
            
            // Set the software version in the DB
            {
                len = strlen(APP_DIS_SW_REV);
                struct diss_set_char_val_req *req_id = KE_MSG_ALLOC_DYN(DISS_SET_CHAR_VAL_REQ,
                                                                        TASK_DISS, TASK_APP_DIS,
                                                                        diss_set_char_val_req,
                                                                        len);

                // Fill in the parameter structure
                req_id->char_code     = DIS_SW_REV_STR_CHAR;
                req_id->val_len       = len;
                memcpy(&req_id->val[0], APP_DIS_SW_REV, len);

                // Send the message
                ke_msg_send(req_id);
            }                        
        }

        // Inform the Application Manager
        struct app_module_init_cmp_evt *cfm = KE_MSG_ALLOC(APP_MODULE_INIT_CMP_EVT,
                                                           TASK_APP, TASK_APP_DIS,
                                                           app_module_init_cmp_evt);

        cfm->status = param->status;

        ke_msg_send(cfm);
    }

    return (KE_MSG_CONSUMED);
}

static int diss_disable_ind_handler(ke_msg_id_t const msgid,
                                    struct diss_disable_ind const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    if (ke_state_get(dest_id) == APP_DIS_CONNECTED)
    {
        // Go to the idle state
        ke_state_set(dest_id, APP_DIS_IDLE);
    }

    return (KE_MSG_CONSUMED);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Default State handlers definition
const struct ke_msg_handler app_dis_default_state[] =
{
    {DISS_CREATE_DB_CFM,            (ke_msg_func_t)diss_create_db_cfm_handler},
    {DISS_DISABLE_IND,              (ke_msg_func_t)diss_disable_ind_handler},
};

/// Specifies the message handlers that are common to all states.
const struct ke_state_handler app_dis_default_handler = KE_STATE_HANDLER(app_dis_default_state);
/// Defines the place holder for the states of all the task instances.
ke_state_t app_dis_state[APP_DIS_IDX_MAX] __attribute__((section("exchange_mem_case1"))); //@WIKRETENTION MEMORY 

#endif //(BLE_APP_DIS)

/// @} APP
