 /**
 ****************************************************************************************
 *
 * @file app_prox_proj_task.c
 *
 * @brief Proximity project application task.
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

/**
 ****************************************************************************************
 * @addtogroup APPTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"               // SW configuration

#if (BLE_APP_PRESENT)

#include "app_task.h"                  // Application Task API
#include "app.h"                       // Application Definition
#include "gapc_task.h"                 // GAP Controller Task API
#include "gapm_task.h"                 // GAP Manager Task API
#include "gap.h"                       // GAP Definitions
#include "co_error.h"                  // Error Codes Definition
#include "arch.h"                      // Platform Definitions


/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */
void app_task_custom_init()
{
	
}

#endif //(BLE_APP_PRESENT)

/// @} APPTASK
