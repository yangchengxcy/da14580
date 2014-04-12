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

#ifndef APP_KEYBOARD_H_
#define APP_KEYBOARD_H_

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

#include <stdint.h>          // standard integer definition
#include <co_bt.h>

#include "app_kbd_config.h"

#if (FULL_SCAN_IN_MS < PARTIAL_SCAN_IN_MS)
#error "Wrong scanning parameters!"
#endif




/*
 * Code switches
 ****************************************************************************************
 */

#ifdef DEEPSLEEP_ON
#define HAS_DEEPSLEEP                           1
#else
#define HAS_DEEPSLEEP                           0
#endif

#ifdef WHITE_LIST_ON
#define HAS_WHITE_LIST                          1
#else
#define HAS_WHITE_LIST                          0
#endif

#ifdef SCAN_ALWAYS_ACTIVE_ON
#define HAS_SCAN_ALWAYS_ACTIVE                  1
#else
#define HAS_SCAN_ALWAYS_ACTIVE                  0
#endif

#ifdef HOGPD_BOOT_PROTO_ON
#define HAS_HOGPD_BOOT_PROTO                    1
#else
#define HAS_HOGPD_BOOT_PROTO                    0
#endif

#ifdef BATT_EXTERNAL_REPORT_ON
#define HAS_BATT_EXTERNAL_REPORT                1
#else
#define HAS_BATT_EXTERNAL_REPORT                0
#endif

#ifdef ALTERNATIVE_SCAN_TIMES_ON
#define HAS_ALTERNATIVE_SCAN_TIMES              1
#else
#define HAS_ALTERNATIVE_SCAN_TIMES              0
#endif

#ifdef KBD_SWITCH_TO_PREFERRED_CONN_PARAMS_ON
#define HAS_KBD_SWITCH_TO_PREFERRED_CONN_PARAMS 1
#else
#define HAS_KBD_SWITCH_TO_PREFERRED_CONN_PARAMS 0
#endif

#ifdef MITM_ON
#define HAS_MITM                                1
#else
#define HAS_MITM                                0
#endif

#ifdef KEYBOARD_MEASURE_EXT_SLP_ON
#define HAS_KEYBOARD_MEASURE_EXT_SLP            1
#else
#define HAS_KEYBOARD_MEASURE_EXT_SLP            0
#endif

#ifdef KEYBOARD_LEDS_ON
#define HAS_KEYBOARD_LEDS                       1
#else
#define HAS_KEYBOARD_LEDS                       0
#endif

#if (MATRIX_SETUP != 3) && (MATRIX_SETUP != 8) && (HAS_KEYBOARD_LEDS)
#error "The chosen keyboard setup does not support LEDS!"
#endif



#ifdef NORMALLY_CONNECTABLE_ON
#define HAS_NORMALLY_CONNECTABLE                1
#else
#define HAS_NORMALLY_CONNECTABLE                0
#endif

#ifdef PASSCODE_TIMEOUT_ON
#define HAS_PASSCODE_TIMEOUT                    1
#else
#define HAS_PASSCODE_TIMEOUT                    0
#endif

#ifdef INACTIVITY_TIMEOUT_ON
#define HAS_INACTIVITY_TIMEOUT                  1
#else
#define HAS_INACTIVITY_TIMEOUT                  0
#endif


/*
 * Debouncing internals
 ****************************************************************************************
 */
 
// In general, debounce counters cannot be applied accurately. The reason is that 
// debouncing is done in SW, based on SysTick interrupt events that are used to 
// trigger the execution of the FSM.
// Thus, in the final application it is suggested to modify the definitions of the two
// counters (below) so that the real debouncing time is applied accurately in all
// cases. First, key presses need to be examined. It should be checked whether the
// debouncing period is correct both after wake-up (when the RC16 is used which is 
// approx. 14.5MHz) and when a 2nd key is pressed (assuming that in this case the BLE 
// core woke up to send the HID report of the 1st key press, which means that the 
// system runs with XTAL16).
// Release debouncing period must also be enforced. In all cases, though, the XTAL16 clk
// is used and there are no special cases.

#define DEBOUNCE_COUNTER_P_IN_MS    (12)
#define DEBOUNCE_COUNTER_PRESS      (int)(1 + ( ((DEBOUNCE_COUNTER_P_IN_MS - FULL_SCAN_IN_MS) / PARTIAL_SCAN_IN_MS) + 0.999 ) )

#define DEBOUNCE_COUNTER_R_IN_MS    (24)
#define DEBOUNCE_COUNTER_RELEASE    (int)( (DEBOUNCE_COUNTER_R_IN_MS / PARTIAL_SCAN_IN_MS) + 0.999 )

#define SYSTICK_CLOCK_RATE          (1000000)
#define SYSTICK_TICKS_PER_US        (SYSTICK_CLOCK_RATE / 1000000)

#define KEYCODE_BUFFER_SIZE (64)	// if set to more than 255, change the type of the rd & wr pointers from 8- to 16-bit

#define DEBOUNCE_BUFFER_SIZE (16)

enum DEBOUNCE_STATE {
    IDLE = 0,
    PRESS_DEBOUNCING,
    WAIT_RELEASE,
    RELEASE_DEBOUNCING,
};

struct debounce_counter_t {
    enum DEBOUNCE_STATE state;
    uint8_t cnt;
};



/*
 * Reporting
 ****************************************************************************************
 */

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


struct roll_over_tag {
    uint8_t keys[16];
    int cnt;
};


/*
 * Public variables
 ****************************************************************************************
 */

extern kbd_rep_info *kbd_trm_list;
extern kbd_rep_info *kbd_free_list;
//extern bool normal_key_report_ack_pending;
//extern bool extended_key_report_ack_pending;
extern bool user_disconnection_req;
extern bool user_extended_sleep;
extern bool systick_hit;
extern bool wkup_hit;


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
void app_hid_create_db(void);
        
void app_keyboard_init(void);

void app_keyboard_enable(void);

void app_kbd_enable_scanning(void);

void app_kbd_start_scanning(void);

bool app_kbd_scan_matrix(int *row);

bool app_kbd_update_status(void);

void app_kbd_disable_scanning(void);

void app_hid_ntf_cfm(uint8_t status);

void app_kbd_reinit_matrix(void);

void app_kbd_start_reporting(void);

void app_kbd_stop_reporting(void);

void app_kbd_flush_buffer(void);

void app_kbd_flush_reports(void);

int app_kbd_prepare_keyreports(void);

int app_kbd_send_key_report(void);

int app_kbd_check_conn_status(void);

/// @} APP

#endif // APP_KEYBOARD_H_
