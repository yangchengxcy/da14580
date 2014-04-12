/**
 ****************************************************************************************
 *
 * @file app_accel.h
 *
 * @brief Accelerometer Application entry point
 *
 * Copyright (C) RivieraWaves 2009-2012
 *
 * $Rev: $
 *
 ****************************************************************************************
 */

#ifndef APP_ACCEL_H_
#define APP_ACCEL_H_

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

//GZ
#define ACCEL_MIN_THRESHOLD		0x12
#define ACCEL_DEF_THRESHOLD		0x17

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwble_config.h"

#if BLE_ACCEL

#include <stdint.h>          // standard integer definition
#include <co_bt.h>



/*
 * GLOBAL VARIABLE DECLARATION
 ****************************************************************************************
 */
 
extern uint8_t accel_adv_count;
extern uint16_t accel_adv_interval;
extern int8_t update_conn_params;


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize Accelerometer Application
 ****************************************************************************************
 */
void app_accel_init(void);

/**
 ****************************************************************************************
 *
 * Accelerometer Application Functions
 *
 ****************************************************************************************
 */
 void acc_stop(void);
void acc_start(uint16_t*, uint8_t );

/**
 ****************************************************************************************
 * @brief Enable the accelerometer profile
 *
 ****************************************************************************************
 */
void app_accel_enable(void);

/**
 ****************************************************************************************
 * @brief Start Timer to control Advertising interval.
 *
 ****************************************************************************************
 */
void app_accel_adv_started(void);

/**
 ****************************************************************************************
 * @brief Stop Timer that controls Advertising interval.
 *
 ****************************************************************************************
 */
void app_accel_adv_stopped(void);

#endif //BLE_ACCEL

/// @} APP

#endif // APP_H_
