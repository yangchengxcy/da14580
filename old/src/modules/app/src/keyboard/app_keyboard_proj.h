/**
 ****************************************************************************************
 *
 * @file app_keyboard_proj.h
 *
 * @brief HID Keyboard hooks header file.
 *
 * Copyright (C) 2013. Dialog Semiconductor Ltd, unpublished work. This computer 
 * program includes Confidential, Proprietary Information and is a Trade Secret of 
 * Dialog Semiconductor Ltd.  All use, disclosure, and/or reproduction is prohibited 
 * unless authorized in writing. All Rights Reserved.
 *
 * <bluetooth.support@diasemi.com> and contributors.
 *
 ****************************************************************************************
 */

#ifndef APP_CUSTOM_KEYBOARD_H_
#define APP_CUSTOM_KEYBOARD_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwble_config.h"

#include "app.h"                      // application definitions
#include "app_task.h"                 // application task
#include "gapc_task.h"                // gapc functions and messages
#include "gapm_task.h"                // gapm functions and messages
#include "smpc_task.h"                // smpc functions and messages
#include "co_error.h"                 // error code definitions

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */
#undef APP_DFLT_DEVICE_NAME 
#define APP_DFLT_DEVICE_NAME "DA14580 Keyboard"

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

void app_disconnect_func(ke_task_id_t const dest_id, struct gapc_disconnect_ind const *param);

void app_init_func(void);

void app_kbd_sleep_control(bool enable);

void app_configuration_func(ke_task_id_t const dest_id, struct gapm_set_dev_config_cmd *cmd);

void app_adv_func(struct gapm_start_advertise_cmd *cmd);

void app_connection_func(ke_task_id_t const dest_id, struct gapc_connection_req_ind const *param);

void app_mitm_passcode_entry(ke_task_id_t const src_id, ke_task_id_t const dest_id);

void app_mitm_passcode_report(uint32_t code);

void app_sec_encrypt_complete_func(void);

void app_param_update_func(void);

void app_sec_init_func(void);

bool app_db_init_func(void);

#include "ke_task.h"        // kernel task
#include "ke_msg.h"         // kernel message

int app_hid_timer_handler(ke_msg_id_t const msgid,
                                   void const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id);


/// @} APP

#endif // APP_CUSTOM_KEYBOARD_H_
