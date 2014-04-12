/**
 ****************************************************************************************
 *
 * @file app.h
 *
 * @brief Application entry point
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
 *
 ****************************************************************************************
 */

#ifndef APP_H_
#define APP_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief Application entry point.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration
#include "rwble_hl_config.h"

#if (BLE_APP_PRESENT)

#include <stdint.h>          // Standard Integer Definition
#include <co_bt.h>           // Common BT Definitions

#include "arch.h"            // Platform Definitions

/*
 * DEFINES
 ****************************************************************************************
 */

/// List of Server Profile to Initialize
enum
{
    APP_PRF_LIST_START      = 0,

#if (BLE_APP_HT)
    APP_HT_TASK,
#endif //(BLE_APP_HT)
#if (BLE_APP_DIS)
    APP_DIS_TASK,
#endif //(BLE_APP_DIS)
#if (BLE_ACCEL)
    APP_ACCEL_TASK,
#endif //(BLE_APP_ACCEL)
#if (BLE_APP_NEB)
    APP_NEB_TASK,
#endif //(BLE_APP_NEB)
#if (BLE_PROX_REPORTER)
    APP_PROXR_TASK,
#endif //(BLE_PROX_REPORTER)
#if (BLE_STREAMDATA_DEVICE)
    APP_STREAM_TASK,
#endif //(BLE_STREAMDATA_DEVICE)
#if (BLE_SPOTA_RECEIVER)
    APP_SPOTAR_TASK,
#endif //(BLE_SPOTA_RECEIVER)
#if (BLE_BATT_SERVER)
    APP_BASS_TASK,
#endif //(BLE_BATT_SERVER)
#if (BLE_APP_KEYBOARD)
    APP_HOGPD_TASK,
#endif //(BLE_APP_KEYBOARD)

    APP_PRF_LIST_STOP,
};

/// Application MTU
#define APP_MTU                 (23)

/**
 * Default Device Name part in ADV Data
 * --------------------------------------------------------------------------------------
 * x09 - Length
 * x09 - Device Name Flag
 * Device Name
 * --------------------------------------------------------------------------------------
 */
//#define APP_DFLT_DEVICE_NAME            ("RW-BLE")

/// Advertising data maximal length
#define APP_ADV_DATA_MAX_SIZE           (ADV_DATA_LEN - 3)
/// Scan Response data maximal length
#define APP_SCAN_RESP_DATA_MAX_SIZE     (SCAN_RSP_DATA_LEN)

/**
 * Default Advertising data
 * --------------------------------------------------------------------------------------
 * x02 - Length
 * x01 - Flags
 * x06 - LE General Discoverable Mode + BR/EDR Not Supported
 * --------------------------------------------------------------------------------------
 * x03 - Length
 * x03 - Complete list of 16-bit UUIDs available
 * x09\x18 - Health Thermometer Service UUID
 *   or
 * x00\xFF - Nebulization Service UUID
 * --------------------------------------------------------------------------------------
 */

#define APP_DFLT_ADV_DATA        "\x07\x03\x03\x18\x02\x18\x04\x18"
#define APP_DFLT_ADV_DATA_LEN    (8)

#if (BLE_APP_HT)
#define APP_HT_ADV_DATA_UUID        "\x03\x03\x09\x18"
#define APP_HT_ADV_DATA_UUID_LEN    (4)
#endif //(BLE_APP_HT)

#if (BLE_APP_NEB)
#define APP_NEB_ADV_DATA_UUID       "\x03\x03\x00\xFF"
#define APP_NEB_ADV_DATA_UUID_LEN   (4)
#endif //(BLE_APP_NEB)

#if (BLE_PROX_REPORTER)
#define APP_ADV_DATA      "\x02\x01\x06\x07\x03\x03\x18\x02\x18\x04\x18"
#define APP_ADV_DATA_LENGTH (11)
#endif

#if (BLE_HID_DEVICE)
#define APP_ADV_DATA      "\x03\x19\xc1\x03\x05\x02\x12\x18\x0f\x18"
#define APP_ADV_DATA_LENGTH (10)
#endif

/**
 * Default Scan response data
 * --------------------------------------------------------------------------------------
 * x09                             - Length
 * xFF                             - Vendor specific advertising type
 * x00\x60\x52\x57\x2D\x42\x4C\x45 - "RW-BLE"
 * --------------------------------------------------------------------------------------
 */
#define APP_SCNRSP_DATA         "\x09\xFF\x00\x60\x52\x57\x2D\x42\x4C\x45"
#define APP_SCNRSP_DATA_LENGTH  (10)

#if BLE_PROX_REPORTER
#define APP_SCNRSP_DATA   "\x09\xFF\x00\x60\x52\x57\x2D\x42\x4C\x45"
#define APP_SCNRSP_DATA_LENGH (10)
#endif

#if (BLE_HID_DEVICE)
#undef APP_SCNRSP_DATA
#define APP_SCNRSP_DATA   "\x00"
#undef APP_SCNRSP_DATA_LENGTH
#define APP_SCNRSP_DATA_LENGTH (0)
#endif

/// Local address type
#define APP_ADDR_TYPE     0
/// Advertising channel map
#define APP_ADV_CHMAP     0x07
/// Advertising filter policy
#define APP_ADV_POL       0
/// Advertising minimum interval
#define APP_ADV_INT_MIN   800
/// Advertising maximum interval
#define APP_ADV_INT_MAX   800

#if (BLE_HID_DEVICE)
#undef APP_ADV_INT_MIN
#define APP_ADV_INT_MIN   0x20		// *0.625ms	(+ pseudo random advDelay from 0 to 10ms)
#undef APP_ADV_INT_MAX
#define APP_ADV_INT_MAX   0x20		// *0.625ms (+ pseudo random advDelay from 0 to 10ms)
#endif

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


/// Application environment structure
struct app_env_tag
{
    /// Connection handle
    uint16_t conhdl;
    uint8_t  conidx; // Should be used only with KE_BUILD_ID()

    /// Last initialized profile
    uint8_t next_prf_init;

    /// Security enable
    bool sec_en;
    
    // Last paired peer address type 
    uint8_t peer_addr_type;
    
    // Last paired peer address 
    struct bd_addr peer_addr;
    
    #if BLE_HID_DEVICE
	uint8_t app_state;
	uint8_t app_flags;
    #endif	

};

/*
 * GLOBAL VARIABLE DECLARATION
 ****************************************************************************************
 */

/// Application environment
extern struct app_env_tag app_env;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize the BLE demo application.
 ****************************************************************************************
 */
void app_init(void);

/**
 ****************************************************************************************
 * @brief Add a required service in the database
 ****************************************************************************************
 */
bool app_db_init(void);

/**
 ****************************************************************************************
 * @brief Send a disconnection request
 ****************************************************************************************
 */
void app_disconnect(void);

/**
 ****************************************************************************************
 * @brief Confirm connection
 ****************************************************************************************
 */
void app_connect_confirm(uint8_t auth);


/**
 ****************************************************************************************
 * @brief Put the device in general discoverable and connectable mode
 ****************************************************************************************
 */
void app_adv_start(void);

/**
 ****************************************************************************************
 * @brief Put the device in non discoverable and non connectable mode
 ****************************************************************************************
 */
void app_adv_stop(void);

/**
 ****************************************************************************************
 * @brief Start security procedure
 ****************************************************************************************
 */
void app_security_start(void);

/**
 ****************************************************************************************
 * @brief Start a parameter update procedure
 ****************************************************************************************
 */
void app_param_update_start(void);

/**
 ****************************************************************************************
 * @brief Encryption procedure has finished
 ****************************************************************************************
 */
void app_sec_encrypt_complete(void);

/// @} APP

#endif //(BLE_APP_PRESENT)

#endif // APP_H_
