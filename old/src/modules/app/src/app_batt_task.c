/**
****************************************************************************************
*
* @file app_batt_task.c
*
* @brief Battery application task.
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

/**
 ****************************************************************************************
 * @addtogroup APPTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"               // SW configuration

#if (BLE_APP_PRESENT)

#if (BLE_BATT_SERVER)

#include "app_task.h"                  // Application Task API
#include "app.h"                       // Application Definition
#include "gapc_task.h"                 // GAP Controller Task API
#include "gapm_task.h"                 // GAP Manager Task API
#include "gap.h"                       // GAP Definitions
#include "co_error.h"                  // Error Codes Definition
#include "arch.h"                      // Platform Definitions
#include "bass_task.h"                      
#include "app_batt.h"
#include "gpio.h"
/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @brief Handles the Create DB confirmation message for the Device Information Service.
 *        Set Advertising Mode
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
 
int batt_create_db_cfm_handler(ke_msg_id_t const msgid,
                                      struct bass_create_db_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    
#if 0    
	printf("batt_create_db_cfm_handler (status=%d)\r\n", (int)param->status);
#endif
    
    // Inform the Application Manager
    struct app_module_init_cmp_evt *cfm = KE_MSG_ALLOC(APP_MODULE_INIT_CMP_EVT,
                                                           TASK_APP, TASK_APP,
                                                           app_module_init_cmp_evt);

    cfm->status = param->status;

    ke_msg_send(cfm);

    return (KE_MSG_CONSUMED);
}

int batt_level_upd_cfm_handler(ke_msg_id_t const msgid,
                                      struct bass_batt_level_upd_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
#if 0    
	printf("batt_level_upd_cfm_handler (status=%d)\r\n", (int)param->status);	
#endif
	return (KE_MSG_CONSUMED);
}

int batt_level_ntf_cfg_ind_handler(ke_msg_id_t const msgid,
                                      struct bass_batt_level_ntf_cfg_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
#if 0
	printf("batt_level_ntf_cfg_ind_handler (ntf_cfg=%d, instance=%d)\r\n", (int)param->ntf_cfg, (int)param->bas_instance);	
#endif
	return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles Battery Level polling timer
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

int app_batt_timer_handler(ke_msg_id_t const msgid,
                                   void const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
	
	app_batt_lvl();
		
	ke_timer_set(APP_BATT_TIMER, dest_id, 500);//60000); //10 mins timeout
		
    return (KE_MSG_CONSUMED);
}



/**
 ****************************************************************************************
 * @brief Handles Battery Alert timer
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

int app_batt_alert_timer_handler(ke_msg_id_t const msgid,
                                        void const *param,
										ke_task_id_t const dest_id,
										ke_task_id_t const src_id)
{
		
	
		//Read LED GPIO state
		
		if (bat_led_state)
		{
#if !BLE_HID_DEVICE		
            GPIO_SetInactive( GPIO_PORT_1, GPIO_PIN_0);            	
#endif			
			bat_led_state = 0;
			ke_timer_set(APP_BATT_ALERT_TIMER, dest_id, 200);
		}
		else
		{
#if !BLE_HID_DEVICE			
            GPIO_SetActive( GPIO_PORT_1, GPIO_PIN_0);
#endif			
			bat_led_state = 1;
			ke_timer_set(APP_BATT_ALERT_TIMER, dest_id, 5);
		}
		
    return (KE_MSG_CONSUMED);
}
#endif //(BLE_BATT_SERVER)

#endif //(BLE_APP_PRESENT)

/// @} APPTASK
