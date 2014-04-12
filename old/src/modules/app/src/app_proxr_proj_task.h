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

#ifndef APP_PRJ1_PROJ_TASK_H_
#define APP_PRJ1_PROJ_TASK_H_

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

#if BLE_PROX_REPORTER

#include "proxr.h"
#include "proxr_task.h"
#include "app_proxr.h"

int proxr_alert_ind_handler(ke_msg_id_t const msgid,
                                      struct proxr_alert_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id);
int app_proxr_timer_handler(ke_msg_id_t const msgid,
                                   void const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id);
int app_proxr_adv_handler(ke_msg_id_t const msgid,
                                   void const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id);
int proxr_create_db_cfm_handler(ke_msg_id_t const msgid,
                                      struct proxr_create_db_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id);
int proxr_disable_ind_handler(ke_msg_id_t const msgid,
                                      struct proxr_disable_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id);
int llc_rd_tx_pw_lvl_cmp_evt_handler(ke_msg_id_t const msgid,
                                      struct llc_rd_tx_pw_lvl_cmd_complete const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id);

#endif

#if BLE_BATT_SERVER
#include "bass.h"
#include "bass_task.h"
#include "app_batt.h"

int batt_create_db_cfm_handler(ke_msg_id_t const msgid,
                                      struct bass_create_db_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id);
                                      
int batt_level_upd_cfm_handler(ke_msg_id_t const msgid,
                                      struct bass_batt_level_upd_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id);
                                      
int batt_level_ntf_cfg_ind_handler(ke_msg_id_t const msgid,
                                      struct bass_batt_level_ntf_cfg_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id);                                   

int app_batt_timer_handler(ke_msg_id_t const msgid,
                                   void const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id);
                                   
int app_batt_alert_timer_handler(ke_msg_id_t const msgid,
                                        void const *param,
										ke_task_id_t const dest_id,
										ke_task_id_t const src_id);
    
#endif 

#if BLE_FINDME_LOCATOR
#include "findl.h"
#include "findl_task.h"
#include "app_findme.h"

int findl_enable_cfm_handler(ke_msg_id_t const msgid,
                                      struct findl_enable_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id);
                                      

int findt_alert_ind_handler(ke_msg_id_t const msgid,
                                      struct findt_alert_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id);
#endif 

/// @} APP

#endif // PRJ1_PROJ_TASK
