/**
 ****************************************************************************************
 *
 * @file app_keyboard_proj_task.h
 *
 * @brief HID Keyboard handlers header file.
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

#ifndef APP_KEYBOARD_PROJ_TASK_H_
#define APP_KEYBOARD_PROJ_TASK_H_


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
int keyboard_create_db_cfm_handler(ke_msg_id_t const msgid,
                                      struct hogpd_create_db_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id);
                                      
int keyboard_disable_ind_handler(ke_msg_id_t const msgid,
                                    struct hogpd_disable_ind const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id);

int keyboard_ntf_sent_cfm_handler(ke_msg_id_t const msgid,
                                      struct hogpd_ntf_sent_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id);

#endif // APP_KEYBOARD_PROJ_TASK_H_
