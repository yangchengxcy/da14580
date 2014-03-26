/**
 ****************************************************************************************
 *
 * @file app_sec.c
 *
 * @brief Application Security Entry Point
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

#include "rwip_config.h"


#if (BLE_APP_SEC)

#if (NVDS_SUPPORT)
#include "nvds.h"           // NVDS API Definition
#endif //(NVDS_SUPPORT)

#include "gapc_task.h"      // GAP Controller Task API Definition
#include "gap.h"            // GAP Definition

#include "smpc_task.h"      // Security Manager Profile Task API Definition

#include "app_sec_task.h"   // Application Security Task API Definition
#include "app_sec.h"        // Application Security API Definition
#include "app_task.h"       // Application Manager API Definition

#if (DISPLAY_SUPPORT)
#include "app_display.h"    // Display Application Definitions
#endif //(DISPLAY_SUPPORT)

#if (BLE_HID_DEVICE)
#include "app_keyboard.h"            // Keyboard (HID) Profile definitions
#include "app_keyboard_proj.h"       // Keyboard (HID) Project definitions
#endif //(BLE_HID_DEVICE)

#if (BLE_APP_KEYBOARD_TESTER)
#include "keyboard_tester/app_kbdtest_proj.h"
#endif // (BLE_APP_KEYBOARD_TESTER)

#if (BLE_PROX_REPORTER )
#include "app_proxr_proj.h"
#endif

#if (BLE_SPOTA_RECEIVER )
#include "app_spotar_proj.h"
#endif
#include <stdlib.h>
#include <string.h>

/*
 * DEFINES
 ****************************************************************************************
 */

// OOB Information
#define OOB_INFORMATION         (GAP_OOB_AUTH_DATA_NOT_PRESENT)

// IO Capabilities
#if (DISPLAY_SUPPORT)
#define IO_CAP                  (GAP_IO_CAP_DISPLAY_ONLY)
#else
#define IO_CAP                  (GAP_IO_CAP_NO_INPUT_NO_OUTPUT)
#endif //(DISPLAY_SUPPORT)

// Authentication Requirements
#define AUTH_REQ                (GAP_AUTH_REQ_NO_MITM_NO_BOND)
// Initiator Key Distribution - Peer Device
#define I_KEYS                  (GAP_KDIST_NONE)
// Responder Key Distribution
#define R_KEYS                  (GAP_KDIST_ENCKEY)

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Application Security Environment Structure
struct app_sec_env_tag app_sec_env __attribute__((section("exchange_mem_case1"))); //@WIKRETENTION MEMORY

/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Application Task Descriptor
static const struct ke_task_desc TASK_DESC_APP_SEC = {NULL, &app_sec_default_handler,
                                                      app_sec_state, APP_SEC_STATE_MAX, APP_SEC_IDX_MAX};

/*
 * LOCAL FUNCTION DECLARATIONS
 ****************************************************************************************
 */



/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

uint32_t app_sec_gen_tk(void)
{
    // Generate a PIN Code (Between 100000 and 999999)
    return (100000 + (rand()%900000));
}

void app_sec_gen_ltk(uint8_t key_size)
{
    // Counter
    uint8_t i;
    app_sec_env.key_size = key_size;

    // Randomly generate the LTK and the Random Number
    for (i = 0; i < RAND_NB_LEN; i++)
    {
        app_sec_env.rand_nb.nb[i] = rand()%256;

    }

    // Randomly generate the end of the LTK
    for (i = 0; i < KEY_LEN; i++)
    {
        app_sec_env.ltk.key[i] = (((key_size) < (16 - i)) ? 0 : rand()%256);
    }

    // Randomly generate the EDIV
    app_sec_env.ediv = rand()%65536;
}

void app_sec_init()
{
    // Reset Security Environment
    memset(&app_sec_env, 0, sizeof(app_sec_env));

    // Create APP_SEC task
    ke_task_create(TASK_APP_SEC, &TASK_DESC_APP_SEC);
	
//GZ	
	app_sec_init_func();
}
#endif //(BLE_APP_SEC)

/// @} APP
