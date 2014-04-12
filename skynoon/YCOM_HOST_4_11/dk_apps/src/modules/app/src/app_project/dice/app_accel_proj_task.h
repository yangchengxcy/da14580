/**
****************************************************************************************
*
* @file app_proxr.c
*
* @brief Proximity Reporter application.
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

#ifndef APP_PRJACCEL_PROJ_TASK_H_
#define APP_PRJACCEL_PROJ_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief Accelerometer Application entry point.
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwble_config.h"

#if BLE_ACCEL

#define APP_DFLT_DEVICE_NAME    "DA14580-DICE5"

void app_accel_create_db_send(void);

int accel_create_db_cfm_handler(ke_msg_id_t const msgid,
                                      struct accel_create_db_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id);

int accel_msg_handler(ke_msg_id_t const msgid,
									struct accel_create_db_cfm const *param,
									ke_task_id_t const dest_id,
									ke_task_id_t const src_id);

int accel_start_ind_handler(ke_msg_id_t const msgid,
                                   struct accel_start_ind const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id);
 
int accel_stop_ind_handler(ke_msg_id_t const msgid,
                                  void const *param,
                                  ke_task_id_t const dest_id,
                                  ke_task_id_t const src_id);

int accel_write_line_ind_handler(ke_msg_id_t const msgid,
                                        struct accel_write_line_ind const *param,
                                        ke_task_id_t const dest_id,
                                        ke_task_id_t const src_id);
                                        
int app_accel_timer_handler(ke_msg_id_t const msgid,
                                   void const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id);

int app_accel_adv_timer_handler(ke_msg_id_t const msgid,
                                   void const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id);
                                   
bool app_db_init_func(void);

#endif

/// @} APP

#endif // PRJACCEL_PROJ_TASK
