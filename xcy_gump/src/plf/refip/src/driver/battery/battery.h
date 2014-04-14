/**
****************************************************************************************
*
* @file battery.h
*
* @brief Battery driver header.
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
#ifndef _BATTERY
#define _BATTERY

#include "stdint.h"


// Battery types definitions
#define BATT_CR2032                         1            //CR2032 coin cell battery

/**
 ****************************************************************************************
 * @brief Returns battery level percentage for the specific battery type.
 *
 ****************************************************************************************
 */
uint8_t battery_get_lvl(uint8_t batt_type);

#endif

