/**
 ****************************************************************************************
 *
 * @file app_dis.h
 *
 * @brief Device Information Service Application entry point
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
 *
 ****************************************************************************************
 */

#ifndef APP_DIS_H_
#define APP_DIS_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 *
 * @brief Device Information Service Application entry point.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW Configuration

#if (BLE_APP_DIS)

#include <stdint.h>          // Standard Integer Definition
#include <co_bt.h>
#include "ble_580_sw_version.h"
/*
 * DEFINES
 ****************************************************************************************
 */


#if (BLE_APP_PRESENT)

/// Manufacturer Name (up to 18 chars)
#define APP_DIS_MANUFACTURER_NAME       ("Dialog Semi")
#define APP_DIS_MANUFACTURER_NAME_LEN   (11)
/// Model Number String (up to 18 chars)
#define APP_DIS_MODEL_NB_STR            ("DA14580")
#define APP_DIS_MODEL_NB_STR_LEN        (7)
/// System ID - LSB -> MSB (FIXME)
#define APP_DIS_SYSTEM_ID               ("\x12\x34\x56\xFF\xFE\x9A\xBC\xDE")
#define APP_DIS_SYSTEM_ID_LEN           (8)
#define APP_DIS_SW_REV                  DA14580_SW_VERSION
#define APP_DIS_FEATURES                (DIS_MANUFACTURER_NAME_CHAR_SUP | DIS_MODEL_NB_STR_CHAR_SUP | DIS_SYSTEM_ID_CHAR_SUP | DIS_SW_REV_STR_CHAR_SUP)

#else

#define APP_DIS_FEATURES                (0)

#endif //(BLE_APP_PRESENT)

/*
 * FUNCTIONS DECLARATION
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 *
 * Device Information Service Application Functions
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize Device Information Service Application
 ****************************************************************************************
 */
void app_dis_init(void);

/**
 ****************************************************************************************
 * @brief Add a Device Information Service instance in the DB
 ****************************************************************************************
 */
void app_dis_create_db_send(void);

/**
 ****************************************************************************************
 * @brief Enable the Device Information Service
 ****************************************************************************************
 */
void app_dis_enable_prf(uint16_t);

#endif //BLE_APP_DIS

/// @} APP

#endif //APP_DIS_H_
