/**
****************************************************************************************
*
* @file app_stream_proj.h
*
* @brief Data stream Project application header file.
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

#ifndef APP_STREAM_PROJ_H_
#define APP_STREAM_PROJ_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief Data Stream Application entry point.
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwble_config.h"
#include "app_task.h"                  // application task
#include "gapc_task.h"                  // gap functions and messages
#include "gapm_task.h"                  // gap functions and messages
#include "app.h"                       // application definitions
#include "co_error.h"                  // error code definitions
#include "smpc_task.h"                  // error code definitions
#include "app_stream_proj_task.h"


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */
#if BLE_STREAMDATA_DEVICE
#define APP_DFLT_DEVICE_NAME "DA14580 STREAM"
#else
#define APP_DFLT_DEVICE_NAME "DA14580"
#endif

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

void app_disconnect_func(ke_task_id_t task_id, struct gapc_disconnect_ind const *param);

void app_init_func(void);

void app_connection_func(ke_task_id_t task_id, struct gapc_connection_req_ind const *param);

void app_sec_init_func(void);

bool app_db_init_func(void);

void app_adv_func(struct gapm_start_advertise_cmd *cmd);

void app_param_update_func(void);

void app_sec_encrypt_complete_func(void);

void app_mitm_passcode_entry(ke_task_id_t const src_id, ke_task_id_t const dest_id);

/// @} APP

#endif //APP_STREAM_PROJ_H_
