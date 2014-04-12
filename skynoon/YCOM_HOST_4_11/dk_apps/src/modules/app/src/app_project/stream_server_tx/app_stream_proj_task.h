/**
****************************************************************************************
*
* @file app_stream_proj_task.h
*
* @brief Stream application
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

#ifndef APP_STREAM_PROJ_TASK_H_
#define APP_STREAM_PROJ_TASK_H_


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwble_config.h"

#if (BLE_STREAMDATA_DEVICE)

#include "ke_msg.h"
#include "streamdatad_task.h"
#include "l2cc_task.h"

void request_more_data_if_possible( void );

short app_audio_get_enable(void);

void app_audio_send_enable(short value);

int stream_create_db_cfm_handler(ke_msg_id_t const msgid,
                                    struct streamdatad_create_db_cfm const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id);
                                      


int stream_start_ind_handler(ke_msg_id_t const msgid,
                                    struct streamdatad_start_ind const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id);
                                   
                                   
int stream_more_data_handler(ke_msg_id_t const msgid,
                                    struct l2cc_data_send_rsp const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id);                        

int stream_stop_ind_handler(ke_msg_id_t const msgid,
                                    void const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id);
                                   
#endif // (BLE_STREAMDATA_DEVICE)


/// @} APP

#endif // PRJ1_PROJ_TASK
