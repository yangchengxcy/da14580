/**
 ****************************************************************************************
 *
 * @file app_dis_task.h
 *
 * @brief Header file - APPDISTASK.
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
 *
 ****************************************************************************************
 */

#ifndef APP_DIS_TASK_H_
#define APP_DIS_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup APPDISTASK Task
 * @ingroup APPDIS
 * @brief Device Information Service Application Task
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration

#if (BLE_APP_DIS)

#include "ke_task.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// Maximal number of APP DIS Task instances
#define APP_DIS_IDX_MAX        (1)

/// Possible states of the APP_DIS task
enum
{
    /// Disabled state
    APP_DIS_DISABLED,
    /// Idle state
    APP_DIS_IDLE,
    /// Connected state
    APP_DIS_CONNECTED,

    /// Number of defined states.
    APP_DIS_STATE_MAX
};

/*
 * TASK DESCRIPTOR DECLARATIONS
 ****************************************************************************************
 */

extern const struct ke_state_handler app_dis_default_handler;
extern ke_state_t app_dis_state[APP_DIS_IDX_MAX];

#endif //(BLE_APP_DIS)

/// @} APPDISTASK

#endif //APP_DIS_TASK_H_
