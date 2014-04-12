/**
 ****************************************************************************************
 *
 * @file app_batt.h
 *
 * @brief Battery Service Application entry point
 *
 * $Rev: $
 *
 ****************************************************************************************
 */

#ifndef APP_BATT_H_
#define APP_BATT_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief Battery Service Application entry point.
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwble_config.h"

#if (BLE_BATT_SERVER)

#include <stdint.h>          // standard integer definition
#include <co_bt.h>

extern uint8_t batt_level;
extern uint8_t old_batt_level; 
extern uint8_t batt_alert_en; 
extern uint8_t bat_led_state;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize Battery Service Application
 ****************************************************************************************
 */
void app_batt_init(void);
void batt_init(void);

/**
 ****************************************************************************************
 *
 * Battery Service Application Functions
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Enable the battery service profile
 *
 ****************************************************************************************
 */
void app_batt_enable(uint8_t batt_lvl, uint8_t old_batt_lvl);

/**
 ****************************************************************************************
 * @brief Enable the battery service profile
 *
 ****************************************************************************************
 */
void app_batt_create_db(void);
 
void app_batt_stop(void);

void app_batt_lvl(void);

void app_batt_set_level(uint8_t batt_level);

void app_batt_poll_start(void);

void app_batt_poll_stop(void);

void app_batt_alert_start(void);

void app_batt_alert_stop(void);

void app_batt_port_reinit(void);

#endif //BLE_BATT_SERVER

/// @} APP

#endif // APP_BATT_H
