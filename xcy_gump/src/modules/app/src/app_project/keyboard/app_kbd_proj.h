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

#ifndef APP_KEYBOARD_PROJ_H_
#define APP_KEYBOARD_PROJ_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include <stdint.h>
#include "app.h"                    // application definitions
#include "gapc_task.h"              // gapc functions and messages
#include "gapm_task.h"              // gapm functions and messages
#include "ke_task.h"                // kernel task
#include "ke_msg.h"                 // kernel message

#include "diss_task.h"              // diss message IDs
#include "app_dis_task.h"           // diss message handlers
#include "bass_task.h"              // bass message IDs
#include "app_batt_task.h"          // bass message handlers
#include "hogpd_task.h"             // hogpd message IDs
#include "app_kbd_proj_task.h"      // hogpd message handlers
#include "app_kbd_leds.h"           // leds message handlers
#include "app_multi_bond.h"         // multiple bonding message handlers

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
 

/*
 * DEFINES
 ****************************************************************************************
 */
#undef APP_DFLT_DEVICE_NAME 
#define APP_DFLT_DEVICE_NAME    "DA14580 Keyboard"

#define APP_ADV_DATA            "\x03\x19\xc1\x03\x05\x02\x12\x18\x0f\x18"
#define APP_ADV_DATA_LENGTH     (10)
#define APP_SCNRSP_DATA         "\x00"
#define APP_SCNRSP_DATA_LENGTH  (0)

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

void app_mitm_passcode_report(uint32_t code);

void set_adv_data(struct gapm_start_advertise_cmd *cmd);



int app_hid_timer_handler(ke_msg_id_t const msgid,
                                   void const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id);

int app_hid_enc_timer_handler(ke_msg_id_t const msgid,
                                   void const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id);


/// @} APP

#endif // APP_KEYBOARD_PROJ_H_
