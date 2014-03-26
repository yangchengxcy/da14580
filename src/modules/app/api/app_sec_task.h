/**
 ****************************************************************************************
 *
 * @file app_sec_task.h
 *
 * @brief Application Security Task Header file
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
 *
 ****************************************************************************************
 */

#ifndef APP_SEC_TASK_H_
#define APP_SEC_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_APP_SEC)

#include "ke_task.h"        // Kernel Task
#include "ke_msg.h"         // Kernel Message
#include <stdint.h>         // Standard Integer

/*
 * DEFINES
 ****************************************************************************************
 */

/// Number of APP Sec Task Instances
#define APP_SEC_IDX_MAX                 (1)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// States of APP task
enum
{
    APP_SEC_IDLE,

    /// Number of defined states.
    APP_SEC_STATE_MAX
};

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */

extern const struct ke_state_handler app_sec_default_handler;
extern ke_state_t app_sec_state[APP_SEC_IDX_MAX];

#endif //(BLE_APP_SEC)

/// @} APPTASK


#endif // APP_TASK_H_
