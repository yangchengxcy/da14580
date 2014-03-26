/**
****************************************************************************************
*
* @file app_button_led.h
*
* @brief Push Button and LED handling header file.
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

#ifndef APP_BUTTON_LED_H_
#define APP_BUTTON_LED_H_

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


/*
 * DEFINES
 ****************************************************************************************
 */

#if BLE_PREC
extern uint16_t prec_adv_count;
extern uint16_t prec_adv_interval;
#endif

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 *
 * Button LED Functions
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Enable the proximity profile
 *
 ****************************************************************************************
 */
 
void GPIO3_Handler(void);

void button_enable_wakeup_irq(void);

void app_prec_adv_start(void);

void app_prec_adv_stop(void);

void app_button_enable(void);

void app_led_enable(void);
/// @} APP

#endif // APP_H_
