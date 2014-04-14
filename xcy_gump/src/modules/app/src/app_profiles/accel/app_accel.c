/**
 ****************************************************************************************
 *
 * @file app_accel.c
 *
 * @brief Accelerometer Application entry point
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
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

#include "app_accel.h"

#if (BLE_APP_ACCEL)

#include <string.h>                  // string manipulation and functions

#include "app.h"                     // application definitions
#include "app_task.h"                // application task definitions
#include "accel_task.h"              // accelerometer functions

#include "co_bt.h"

#include "arch.h"                      // platform definitions

#if (DISPLAY_SUPPORT)
#include "display.h"
#endif //DISPLAY_SUPPORT

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */
void app_accel_init(void)
{
}

/**
 ****************************************************************************************
 * Accelerometer Application Functions
 ****************************************************************************************
 */

void app_accel_enable(void)
{
    // Allocate the message
    struct accel_enable_req * req = KE_MSG_ALLOC(ACCEL_ENABLE_REQ, TASK_ACCEL, TASK_APP,
                                                 accel_enable_req);

    // Fill in the parameter structure
    req->appid = TASK_APP;
    req->conhdl = app_env.conhdl;

    // Send the message
    ke_msg_send(req);

    // Reset the accelerometer driver
    acc_init();
}

#endif //BLE_APP_ACCEL

/// @} APP
