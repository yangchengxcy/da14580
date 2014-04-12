/**
 ****************************************************************************************
 *
 * @file app_task.h
 *
 * @brief Header file - APPTASK.
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
 *
 ****************************************************************************************
 */

#ifndef APP_TASK_H_
#define APP_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup APPTASK Task
 * @ingroup APP
 * @brief Routes ALL messages to/from APP block.
 *
 * The APPTASK is the block responsible for bridging the final application with the
 * RWBLE software host stack. It communicates with the different modules of the BLE host,
 * i.e. @ref SMP, @ref GAP and @ref GATT.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"             // SW configuration

#if (BLE_APP_PRESENT)

#include "ke_task.h"        // Kernel Task
#include "ke_msg.h"         // Kernel Message
#include <stdint.h>         // Standard Integer

/*
 * DEFINES
 ****************************************************************************************
 */

/// Number of APP Task Instances
#define APP_IDX_MAX                 (1)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// States of APP task
enum APP_STATE
{
    /// Disabled State
    APP_DISABLED,
    /// Database Initialization State
    APP_DB_INIT,
    /// Connectable state
    APP_CONNECTABLE,

    /**
     * CONNECTED STATES
     */
    /// Security state,
    APP_SECURITY,
    /// Connection Parameter Update State
    APP_PARAM_UPD,
    /// Connected state
    APP_CONNECTED,

    /// Number of defined states.
    APP_STATE_MAX
};

/// APP Task messages
enum APP_MSG
{
    APP_MODULE_INIT_CMP_EVT = KE_FIRST_MSG(TASK_APP),
#if BLE_ACCEL
	APP_ACCEL_TIMER,
    APP_ACCEL_ADV_TIMER,
	APP_ACCEL_MSG,
#endif //BLE_ACCEL
    
#if BLE_PROX_REPORTER
    APP_PXP_TIMER,
#endif //BLE_PROX_REPORTER

#if (BLE_HID_DEVICE)
    APP_HID_TIMER,
#endif //BLE_HID_DEVICE

#if (BLE_HID_REPORT_HOST)
    APP_HID_TIMER,
#endif //BLE_HID_REPORT_HOST

#if BLE_BATT_SERVER
    APP_BATT_TIMER,
    APP_BATT_ALERT_TIMER,
#endif //BLE_BATT_SERVER

#if BLE_ALT_PAIR
    APP_ALT_PAIR_TIMER,
    APP_DIR_ADV_TIMER,
#endif //BLE_ALT_PAIR
};

/*
 * API MESSAGES STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the @ref APP_MODULE_INIT_CMP_EVT message
struct app_module_init_cmp_evt
{
    ///Status
    uint8_t status;
};

/// Parameters of the @ref APP_SECURITY_CMP_EVT message
struct app_pairing_cmp_evt
{
    ///Status
    uint8_t status;
};

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */

extern const struct ke_state_handler app_default_handler;
extern ke_state_t app_state[APP_IDX_MAX];

/// @} APPTASK

#endif //(BLE_APP_PRESENT)

#endif // APP_TASK_H_
