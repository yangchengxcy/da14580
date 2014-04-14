/**
 ****************************************************************************************
 *
 * @file app_kbd_config.h
 *
 * @brief Keyboard (HID) Application configuration header file.
 *
 * Copyright (C) 2014. Dialog Semiconductor Ltd, unpublished work. This computer 
 * program includes Confidential, Proprietary Information and is a Trade Secret of 
 * Dialog Semiconductor Ltd.  All use, disclosure, and/or reproduction is prohibited 
 * unless authorized in writing. All Rights Reserved.
 *
 * <bluetooth.support@diasemi.com> and contributors.
 *
 ****************************************************************************************
 */

#ifndef APP_KBD_CONFIG_H_
#define APP_KBD_CONFIG_H_

/****************************************************************************************
 * Use Deep Sleep when in IDLE_ST (untested)                                            *
 ****************************************************************************************/
//#define DEEPSLEEP_ON


/****************************************************************************************
 * Use White List when exiting DIRECTED_ADV_ST unsuccessfully.                          *
 * Note: if White List is used, then the Device will be able to bond only to 1 Master!!!*
 ****************************************************************************************/
//#define WHITE_LIST_ON


/****************************************************************************************
 * Continuous key scanning. The chip won't go to sleep in this case.                    *
 ****************************************************************************************/
//#define SCAN_ALWAYS_ACTIVE_ON


/****************************************************************************************
 * Use BOOT MODE (FIXME: not supported yet)                                             *
 ****************************************************************************************/
//#define HOGPD_BOOT_PROTO_ON


/****************************************************************************************
 * Include BATT in HID                                                                  *
 ****************************************************************************************/
//#define BATT_EXTERNAL_REPORT_ON


/****************************************************************************************
 * Use different Fast and Partial scan periods                                          *
 ****************************************************************************************/
//#define ALTERNATIVE_SCAN_TIMES_ON


/****************************************************************************************
 * Send a ConnUpdateParam request after connection completion                           *
 ****************************************************************************************/
#define KBD_SWITCH_TO_PREFERRED_CONN_PARAMS_ON


/****************************************************************************************
 * Use 'Fn'+'Space' combination to put the device permanently in extended sleep         *
 ****************************************************************************************/
#define KEYBOARD_MEASURE_EXT_SLP_ON


/****************************************************************************************
 * Use Connection status LEDs                                                           *
 ****************************************************************************************/
//#define KEYBOARD_LEDS_ON


/****************************************************************************************
 * Set NormallyConnectable mode ON                                                      *
 ****************************************************************************************/
//#define NORMALLY_CONNECTABLE_ON


/****************************************************************************************
 * Enable timeout checking during PassCode entry                                        *
 ****************************************************************************************/
//#define PASSCODE_TIMEOUT_ON


/****************************************************************************************
 * Enable disconnection after a pre-defined inactivity timeout                          *
 ****************************************************************************************/
#define INACTIVITY_TIMEOUT_ON


/****************************************************************************************
 * Use MITM authentication mode                                                         *
 ****************************************************************************************/
#define MITM_ON


/****************************************************************************************
 * Use EEPROM to store bonding data (MITM should be ON)                                 *
 * It depends on the Keyboard setup whether an EEPROM will be used eventually.          *
 ****************************************************************************************/
#define EEPROM_ON


/****************************************************************************************
 * Define EEPROM size (0 = 256 bytes, 1 = 8192 bytes)                                   *
 ****************************************************************************************/
#define EEPROM_IS_8K                            (1)


/****************************************************************************************
 * Choose keyboard layout                                                               *
 ****************************************************************************************/
#define MATRIX_SETUP                            (8)




/****************************************************************************************
 * Debouncing parameters                                                                *
 ****************************************************************************************/
 
// HW debouncing time for key press
#define DEBOUNCE_TIME_PRESS                     (0)         // in msec

// SW debouncing times
#define ROW_SCAN_TIME                           (150)       // in usec

#ifdef ALTERNATIVE_SCAN_TIMES
#define FULL_SCAN_IN_MS                         (2)
#else
#define FULL_SCAN_IN_MS                         (3)
#endif
#define FULL_SCAN_TIME                          (FULL_SCAN_IN_MS * 1000)

#ifdef ALTERNATIVE_SCAN_TIMES
# define PARTIAL_SCAN_IN_MS                     (1)
#else
# define PARTIAL_SCAN_IN_MS                     (FULL_SCAN_IN_MS)
#endif
#define PARTIAL_SCAN_TIME                       (PARTIAL_SCAN_IN_MS * 1000)




/****************************************************************************************
 * Timeouts                                                                             *
 ****************************************************************************************/
 
// Time in Limited Discoverable mode in ADVERTISE_ST:UNBONDED           (when NORMALLY_CONNECTABLE_ON is undefined)
#define KBD_UNBONDED_DISCOVERABLE_TIMEOUT       (180000)    // in msec

// Time in Limited Discoverable mode in ADVERTISE_ST:BONDED
#define KBD_BONDED_DISCOVERABLE_TIMEOUT         (30000)     // in msec

// Time to inform about the completion of the Connection in case encryption does not follow
#define KBD_ENC_SAFEGUARD_TIMEOUT               (500)       // in msec

// Time in CONNECTED_NO_PAIR_ST until passcode is entered               (when PASSCODE_TIMEOUT_ON is defined)
#define KBD_PASSCODE_TIMEOUT                    (60000)     // in msec

// Idle time in CONNECTED_ST until disconnection is requested           (when INACTIVITY_TIMEOUT_ON is defined)
#define KBD_INACTIVITY_TIMEOUT                  (300000)    // in msec

// Time to request update of connection parameters                      (when KBD_SWITCH_TO_PREFERRED_CONN_PARAMS_ON is defined)
#define TIME_TO_REQUEST_PARAM_UPDATE	        (40000)	    // in msec

// Time to block previous host during a "host-switch"
#define ALT_PAIR_DISCONN_TIME                   (6000)      // in 10msec

// ADVERTISE_ST:UNBONDED : minimum advertising interval (* 0.625ms)
#define NORMAL_ADV_INT_MIN                      (0x30)      // 30 msec  (+ pseudo random advDelay from 0 to 10msec)

// ADVERTISE_ST:UNBONDED : maximum advertising interval (* 0.625ms)
#define NORMAL_ADV_INT_MAX                      (0x50)      // 50 msec  (+ pseudo random advDelay from 0 to 10msec)

// ADVERTISE_ST:BONDED : minimum advertising interval   (* 0.625ms)
#define FAST_BONDED_ADV_INT_MIN                 (0x20)      // 20 msec  (+ pseudo random advDelay from 0 to 10msec)

// ADVERTISE_ST:BONDED : maximum advertising interval   (* 0.625ms)
#define FAST_BONDED_ADV_INT_MAX                 (0x20)      // 20 msec  (+ pseudo random advDelay from 0 to 10msec)

// ADVERTISE_ST:SLOW : minimum advertising interval     (* 0.625ms)     (when NORMALLY_CONNECTABLE_ON is undefined) 
#define SLOW_BONDED_ADV_INT_MIN                 (0x640)     // 1 s      (+ pseudo random advDelay from 0 to 10msec)

// ADVERTISE_ST:SLOW : maximum advertising interval     (* 0.625ms)     (when NORMALLY_CONNECTABLE_ON is undefined)
#define SLOW_BONDED_ADV_INT_MAX                 (0xFA0)     // 2.5 s    (+ pseudo random advDelay from 0 to 10msec)




/****************************************************************************************
 * Prefered connection parameters                                                       *
 ****************************************************************************************/
#define PREFERRED_CONN_INTERVAL_MIN             (6)         //N * 1.25ms
#define PREFERRED_CONN_INTERVAL_MAX             (6)         //N * 1.25ms
#define	PREFERRED_CONN_LATENCY                  (31)
#define PREFERRED_CONN_TIMEOUT                  (200)       //N * 10ms

#endif // APP_KBD_CONFIG_H_
