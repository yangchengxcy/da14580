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
#include "ke_msg.h"

#if (BLE_APP_PRESENT)

#include <stdint.h>          // Standard Integer Definition
#include <co_bt.h>           // Common BT Definitions

#include "arch.h"            // Platform Definitions
#include "prf_types.h"       //randy 20140414
#include "proxm_task.h"
/*
 * DEFINES
 ****************************************************************************************
 */


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

#define MAX_SCAN_DEVICES 9 //randy
#define RSSI_SAMPLES	 5   //randy
#define DIS_VAL_MAX_LEN                         (0x12)
/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
 enum
{
    DISC_MANUFACTURER_NAME_CHAR,
    DISC_MODEL_NB_STR_CHAR,
    DISC_SERIAL_NB_STR_CHAR,
    DISC_HARD_REV_STR_CHAR,
    DISC_FIRM_REV_STR_CHAR,
    DISC_SW_REV_STR_CHAR,
    DISC_SYSTEM_ID_CHAR,
    DISC_IEEE_CHAR,
    DISC_PNP_ID_CHAR,

    DISC_CHAR_MAX,
};
enum
{
    /// Start the find me locator profile - at connection
    DISC_ENABLE_REQ = KE_FIRST_MSG(TASK_DISC),
    /// Confirm that cfg connection has finished with discovery results, or that normal cnx started
    DISC_ENABLE_CFM,
    /// Inform APP that the profile client role has been disabled after a disconnection
    DISC_DISABLE_IND,

    /// Generic message to read a DIS characteristic value
    DISC_RD_CHAR_REQ,
    /// Generic message for read responses for APP
    DISC_RD_CHAR_RSP,
};
 typedef struct           //randy
{
    unsigned char free;
    struct bd_addr adv_addr;
    unsigned short conidx;
    unsigned short conhdl;
    unsigned char idx;
    char  rssi;
    unsigned char  data_len;
    unsigned char  data[ADV_DATA_LEN + 1];
} ble_dev;
typedef struct 
{
    uint16_t    val_hdl;
    uint8_t     val[DIS_VAL_MAX_LEN + 1];
    uint16_t    len;
} dis_char;
//Proximity Reporter connected device
typedef struct 
{
   dis_char chars[DISC_CHAR_MAX];
} dis_env;
typedef struct                       //randy
{
    ble_dev device;
    unsigned char bonded;
    unsigned short ediv;
   // struct rand_nb rand_nb[RAND_NB_LEN];
   //truct gapc_ltk ltk;
	  char ltk;        //randy 20140415
   // struct gapc_irk irk;
   // struct gap_sec_key csrk;
    unsigned char llv;
    char txp;
    char rssi[RSSI_SAMPLES];
    char rssi_indx;
    char avg_rssi;
    unsigned char alert;
    dis_env dis;
} proxr_dev;
 /// application environment structure
struct xapp_env_tag   //randy
{
    unsigned char state;
    unsigned char num_of_devices;
    ble_dev devices[MAX_SCAN_DEVICES];
    proxr_dev proxr_device;
};
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
 * @brief Start a parameter update procedure
 ****************************************************************************************
 */
void app_param_update_start(void);


/**
 ****************************************************************************************
 * @brief Start a kernel timer
 ****************************************************************************************
 */
void app_timer_set(ke_msg_id_t const timer_id, ke_task_id_t const task_id, uint16_t delay);


/// @} APP

#endif //(BLE_APP_PRESENT)

#endif // APP_H_
