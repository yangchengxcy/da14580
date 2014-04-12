/**
 ****************************************************************************************
 *
 * @file app_keyboard.h
 *
 * @brief Keyboard (HID) Application entry point header file.
 *
 * Copyright (C) 2013. Dialog Semiconductor Ltd, unpublished work. This computer 
 * program includes Confidential, Proprietary Information and is a Trade Secret of 
 * Dialog Semiconductor Ltd.  All use, disclosure, and/or reproduction is prohibited 
 * unless authorized in writing. All Rights Reserved.
 *
 * <bluetooth.support@diasemi.com> and contributors.
 *
 ****************************************************************************************
 */

#ifndef APP_HID_H_
#define APP_HID_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup HID
 *
 * @brief HID (Keyboard) Application entry point.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwble_config.h"

#if BLE_HID_DEVICE

#include <stdint.h>          // standard integer definition
#include <co_bt.h>

#define ENABLE_WKUP_INT			0x01
#define KBD_START_SCAN			0x02
#define KBD_SCANNING			0x04
#define UPD_PENDING				0x08
#define UPD_SENT				0x10
#define KBD_CONTROLLER_ACTIVE	0x20

#define TIME_TO_SWITCH_TO_PREFERRED_MODE    4000    //N * 10ms
#define TIME_TO_REQUEST_PARAM_UPDATE	    3000	//N * 10ms

#define PREFERRED_CONN_INTERVAL_MIN         6		//N * 1.25ms
#define PREFERRED_CONN_INTERVAL_MAX         6		//N * 1.25ms
#define	PREFERRED_CONN_LATENCY              31
#define PREFERRED_CONN_TIMEOUT              200	    //N * 10ms

// Set to 1 if you want to be able to scan continuously. The chip won't go to sleep in this case. Default is 0.
#define SCAN_ALWAYS_ACTIVE                        0

// Uncomment the next line to use BOOT MODE (FIXME: not yet supported)
//#define HOGPD_BOOT_PROTO                        1

// Uncomment the next line to include BATT in HID
#define USE_BATT_EXTERNAL_REPORT                  1

// Uncomment the next line to use different Fast and Partial scan periods
#define ALTERNATIVE_SCAN_TIMES                  1

// Uncomment the next line to send a ConnUpdateParam request after connection completion
#define KBD_SWITCH_TO_PREFERRED_CONN_PARAMS     1

// Uncomment the next line to activate MITM authentication mode
#define MITM_REQUIRED                           1

// Uncomment the following line to enable 'Fn'+'Space' combination to put the device permanently in ext. sleep
#define KEYBOARD_MEASURE_EXT_SLP                1

// Select keyboard layout
#define MATRIX_SETUP                            3

// Are we using a 256 byte EEPROM (=0) or an 8KB (=1)?
#define EEPROM_IS_8K                            0

// Select EEPROM pins
#if (MATRIX_SETUP == 7)
#define I2C_SDA_PORT    GPIO_PORT_0
#define I2C_SDA_PIN     GPIO_PIN_2
#define I2C_SCL_PORT    GPIO_PORT_0
#define I2C_SCL_PIN     GPIO_PIN_3
#else
#define I2C_SDA_PORT    GPIO_PORT_0
#define I2C_SDA_PIN     GPIO_PIN_6
#define I2C_SCL_PORT    GPIO_PORT_0
#define I2C_SCL_PIN     GPIO_PIN_7
#endif

#define KBD_US_FAST_SCAN            (1500)
#define KBD_US_PART_SCAN            (900)
#define KBD_US_PER_SCAN             (3000)

#define SYSTICK_CLOCK_RATE          (1000000)
#define SYSTICK_TICKS_PER_US        (SYSTICK_CLOCK_RATE / 1000000)
#define SPLIT_PROCESS_KBD_SCANDATA  0
//#define SPLIT_PROCESS_KBD_SCANDATA 7
#define FSM_STEPS	1

#define KEYCODE_BUFFER_SIZE (64)	// if set to more than 255, change the type of the rd & wr pointers from 8- to 16-bit

// HW debouncing time for key press
#define DEBOUNCE_TIME_PRESS		0	        //in msec

// SW debouncing times
#ifdef ALTERNATIVE_SCAN_TIMES	
#define DEBOUNCE_COUNTER_PRESS_TIME 8       //ms
//#define DEBOUNCE_COUNTER_PRESS (int)( (DEBOUNCE_COUNTER_PRESS_TIME * 1000) / 1000)

// subtract 1 due to power-on delay of 1.1ms - add 1 because 1 partial scan will be needed after debouncing time elapses to check the status of the key
#define DEBOUNCE_COUNTER_PRESS (int)(( ((DEBOUNCE_COUNTER_PRESS_TIME * 1000) - KBD_US_FAST_SCAN) / (float)(KBD_US_PART_SCAN + 105)) + 1 + 0.5) - 1 + 1
#else
#define DEBOUNCE_COUNTER_PRESS_TIME 12      //ms (multiple of 3)
#define DEBOUNCE_COUNTER_PRESS (int)( (DEBOUNCE_COUNTER_PRESS_TIME * 1000) / KBD_US_PER_SCAN)
#endif

#ifdef ALTERNATIVE_SCAN_TIMES
#define DEBOUNCE_COUNTER_RELEASE_TIME 8     //ms
//#define DEBOUNCE_COUNTER_RELEASE (int)( (DEBOUNCE_COUNTER_RELEASE_TIME * 1000) / 1000)
#define DEBOUNCE_COUNTER_RELEASE (int)( (((DEBOUNCE_COUNTER_RELEASE_TIME * 1000) - KBD_US_FAST_SCAN) / (float)(KBD_US_PART_SCAN + 105)) + 1 + 0.5) - 1 + 1
#else
#define DEBOUNCE_COUNTER_RELEASE_TIME 12    //ms (multiple of 3)
#define DEBOUNCE_COUNTER_RELEASE (int)( (DEBOUNCE_COUNTER_RELEASE_TIME * 1000) / KBD_US_PER_SCAN)
#endif

#define DEBOUNCE_BUFFER_SIZE (16)

enum {
	IDLE,
	DISCONNECTED,
	CONNECTED_FAST,
	CONNECTED_SLOW,
	BONDED_DISCONNECTED
};

extern volatile uint8_t kbd_do_scan_param;
extern bool normal_key_report_ack_pending;
extern bool extended_key_report_ack_pending;
extern bool user_disconnection_req;
extern bool user_extended_sleep;

#define MAX_REPORTS 5 

enum KEY_BUFF_TYPE {
	FREE,
	PRESS,
	RELEASE,
    EXTENDED
};

typedef struct __kbd_rep_info {
	enum KEY_BUFF_TYPE type;
	bool modifier_report;
    uint8_t char_id;
    uint8_t len;
	uint8_t *pBuf;
	struct __kbd_rep_info *pNext;
} kbd_rep_info;

extern kbd_rep_info *kbd_trm_list;


//#define DEBUG_HID 
#ifdef DEBUG_HID
# if (DEVELOPMENT__NO_OTP == 0)
	#warning "KBD: DEBUG BreakPoints are enabled! Make sure you are not compiling for OTP!"
# endif    

	#define DEBUG_BP(cond)	\
		do { if (cond) __asm("BKPT #0\n"); } while(0);

	#define __STATIC
#else
	#define DEBUG_BP(cond) {}
	#define __STATIC static
#endif

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
void app_hid_create_db(void);
        
/**
 ****************************************************************************************
 * @brief Initialize HID Application
 ****************************************************************************************
 */
void app_keyboard_init(void);

/**
 ****************************************************************************************
 *
 * HID Application Functions
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Enable the hid profile
 *
 ****************************************************************************************
 */
void app_keyboard_enable(void);

/**
 ****************************************************************************************
 * @brief Enable the hid profile
 *
 ****************************************************************************************
 */
void app_kbd_stop(void);

void app_kbd_enable_scanning(void);

void app_kbd_start_reporting(void);

void app_hid_ntf_cfm(uint8_t status);

void app_kbd_reinit_matrix(void);

void app_kbd_enable_wakeup_irq(void);

void app_kbd_start_scanning(void);

int app_kbd_send_key_report(void);

int check_connection_status(void);

void app_kbd_do_scan(void);

#endif //BLE_HID

/// @} APP

#endif // APP_H_
