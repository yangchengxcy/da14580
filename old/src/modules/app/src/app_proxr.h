/**
****************************************************************************************
*
* @file app_proxr.h
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

#ifndef APP_PROXR_H_
#define APP_PROXR_H_

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

#include <stdint.h>          // standard integer definition
#include <co_bt.h>

#include "proxr.h"

#include "llc_task.h"

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
typedef struct
{
uint32_t blink_timeout;
	
uint8_t blink_toggle;
	
uint8_t lvl;
	
uint8_t ll_alert_lvl;
	
int8_t  txp_lvl;

}app_alert_state;

extern app_alert_state alert_state;

/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize Proximity Application
 ****************************************************************************************
 */
void app_proxr_init(void);

/**
 ****************************************************************************************
 *
 * Proximity Application Functions
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Enable the proximity profile
 *
 ****************************************************************************************
 */
void app_proxr_enable(void);

/**
 ****************************************************************************************
 * @brief Start proximity Alert
 *
 ****************************************************************************************
 */
void app_proxr_alert_start(uint8_t lvl);

/**
 ****************************************************************************************
 * @brief Stop proximity Alert.
 *
 ****************************************************************************************
 */
void app_proxr_alert_stop(void);


/**
 ****************************************************************************************
 * @brief Read Tx Power Level.
 *
 ****************************************************************************************
 */
void app_proxr_rd_tx_pwr(void);

/**
 ****************************************************************************************
 * @brief Create proximity reporter Database
 *
 ****************************************************************************************
 */

void app_proxr_create_db_send(void);

void proxr_init_port (void);

void app_proxr_port_reinit(void);

#define XCY_LED_GPIO	GPIO_PORT_0,GPIO_PIN_7
#define XCY_KB_1		GPIO_PORT_0,GPIO_PIN_6

#endif //BLE_PROX_REPORTER

/// @} APP

#endif // APP_H_
