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

/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwble_config.h"

#if (BLE_APP_PRESENT)
#if (BLE_PROX_REPORTER)

#include <string.h>                  // string manipulation and functions

#include "app.h"                     // application definitions
#include "app_task.h"                // application task definitions
#include "proxr_task.h"              // proximity functions
#include "app_proxr.h"


#include "co_bt.h"

#include "arch.h"                      // platform definitions
#include "gpio.h"
#include "ke_timer.h"                   // kernel timer
#include "app_button_led.h"
//application allert state structrure
app_alert_state alert_state __attribute__((section("exchange_mem_case1")));

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
*/
 
 /**
 ****************************************************************************************
 * Proximity Reporter Application Functions
 ****************************************************************************************
 */
 
 /**
 ****************************************************************************************
 * @brief Initialize Proximity Apllication GPIO ports.
 *
 * @return void.
 ****************************************************************************************
 */
 
/**
 ****************************************************************************************
 * @brief Initialize Proximity Apllication. Ports and interrupts.
 *
 * @return void.
 ****************************************************************************************
 */

void app_proxr_init(void)
{
	
	app_button_enable();
	app_led_enable();
}

/**
 ****************************************************************************************
 * @brief Inialize applacition and enable proximity profile.
 *
 * @param[in] type      Alert type. Link Loss or Imediate Alert
 * @param[in] level     Alert level. Mild or High
 *
 * @return void.
 ****************************************************************************************
 */

void app_proxr_enable(void)
{
		
    // Allocate the message
    struct proxr_enable_req * req = KE_MSG_ALLOC(PROXR_ENABLE_REQ, TASK_PROXR, TASK_APP,
                                                 proxr_enable_req);

  	// init application alert state
		app_proxr_alert_stop();
	
    // Fill in the parameter structure
        req->conhdl = app_env.conhdl;
		req->sec_lvl = PERM(SVC, ENABLE);
		req->lls_alert_lvl = (uint8_t) alert_state.ll_alert_lvl;  
		req->txp_lvl = alert_state.txp_lvl; 
	
    // Send the message
    ke_msg_send(req);

}

/**
 ****************************************************************************************
 * @brief Starts proximity apllication alert.
 *
 * @param[in] lvl     Alert level. Mild or High
 *
 * @return void.
 ****************************************************************************************
 */

void app_proxr_alert_start(uint8_t lvl)
{
	
	alert_state.lvl = lvl;
	
	if (alert_state.lvl == PROXR_ALERT_MILD)
		alert_state.blink_timeout = 150;
	else
		alert_state.blink_timeout = 50;
	
	alert_state.blink_toggle = 1;
    GPIO_SetActive( GPIO_PORT_0, GPIO_PIN_7);
	
    ke_timer_set(APP_PXP_TIMER, TASK_APP, alert_state.blink_timeout);	
}

/**
 ****************************************************************************************
 * @brief Stops proximity apllication alert.
 *
 * @return void.
 ****************************************************************************************
 */

void app_proxr_alert_stop(void)
{

	alert_state.lvl = PROXR_ALERT_NONE; //level;
	
	alert_state.blink_timeout = 0;
	alert_state.blink_toggle = 0;
	
    GPIO_SetInactive( GPIO_PORT_0, GPIO_PIN_7);
	
    ke_timer_clear(APP_PXP_TIMER, TASK_APP);
}


/**
 ****************************************************************************************
 * @brief Read Tx Power Level.
 *
 * @return void.
 ****************************************************************************************
 */

void app_proxr_rd_tx_pwr(void)
{
		
    // Allocate the message
    struct llc_rd_tx_pw_lvl_cmd * req = KE_MSG_ALLOC(LLC_RD_TX_PW_LVL_CMP_EVT, TASK_LLC, TASK_APP,
                                                 llc_rd_tx_pw_lvl_cmd);
		
    req->conhdl = app_env.conhdl;
    req->type = TX_LVL_CURRENT;

    // Send the message
    ke_msg_send(req);

}


void app_proxr_create_db_send(void)
{
    // Add HTS in the database
    struct proxr_create_db_req *req = KE_MSG_ALLOC(PROXR_CREATE_DB_REQ,
                                                  TASK_PROXR, TASK_APP,
                                                  proxr_create_db_req);

    req->features = PROXR_IAS_TXPS_SUP;

    ke_msg_send(req);
}

void app_proxr_port_reinit(void)
{
    app_proxr_init();

	if(alert_state.blink_toggle == 1){
        GPIO_SetActive( GPIO_PORT_0, GPIO_PIN_7);
    }
	else{
        GPIO_SetInactive( GPIO_PORT_0, GPIO_PIN_7);
	}
	
}

#endif //BLE_PROXR
#endif //BLE_APP_PRESENT

/// @} APP
