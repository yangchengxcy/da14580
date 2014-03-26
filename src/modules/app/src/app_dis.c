/**
 ****************************************************************************************
 *
 * @file app_dis.c
 *
 * @brief Device Information Service Application entry point
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

#include "rwip_config.h"     // SW configuration

#if (BLE_APP_DIS)

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "app_dis.h"                 // Device Information Service Application Definitions
#include "app_dis_task.h"            // Device Information Service Application Task API
#include "app.h"                     // Application Definitions
#include "app_task.h"                // Application Task Definitions
#include "diss_task.h"               // Health Thermometer Functions
#include "co_bt.h"
#include "arch.h"                    // Platform Definitions

#if (DISPLAY_SUPPORT)
#include "app_display.h"
#include "display.h"
#endif //DISPLAY_SUPPORT

/*
 * LOCAL VARIABLES DECLARATIONS
 ****************************************************************************************
 */

/// Device Information Service Application Task Descriptor
static const struct ke_task_desc TASK_DESC_APP_DIS = {NULL, &app_dis_default_handler, app_dis_state, APP_DIS_STATE_MAX, APP_DIS_IDX_MAX};

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void app_dis_init(void)
{
    // Create APP_DIS task
    ke_task_create(TASK_APP_DIS, &TASK_DESC_APP_DIS);

    // Go to disabled state
    ke_state_set(TASK_APP_DIS, APP_DIS_DISABLED);
}

void app_dis_create_db_send(void)
{
    // Add DIS in the database
    struct diss_create_db_req *req = KE_MSG_ALLOC(DISS_CREATE_DB_REQ,
                                                  TASK_DISS, TASK_APP_DIS,
                                                  diss_create_db_req);

    req->features = APP_DIS_FEATURES;

    // Send the message
    ke_msg_send(req);
}

void app_dis_enable_prf(uint16_t conhdl)
{
    // Allocate the message
    struct diss_enable_req *req = KE_MSG_ALLOC(DISS_ENABLE_REQ,
                                               TASK_DISS, TASK_APP_DIS,
                                               diss_enable_req);

    // Fill in the parameter structure
    req->conhdl             = conhdl;
    req->sec_lvl            = PERM(SVC, ENABLE);
    req->con_type           = PRF_CON_DISCOVERY;

    // Send the message
    ke_msg_send(req);

    // Go to Connected state
    ke_state_set(TASK_APP_DIS, APP_DIS_CONNECTED);
}

#endif //BLE_APP_DIS

/// @} APP
