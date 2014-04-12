/**
 ****************************************************************************************
 *
 * @file app_keyboard.c
 *
 * @brief Keyboard (HID) over GATT Application entry point.
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

#include <stdlib.h>
#include "app_keyboard.h"
#include "app_keyboard_proj.h"
#include "app_keyboard_matrix.h"
#include "gpio.h"

#if (BLE_ALT_PAIR)
#include "app_alt_pair.h"
#endif

#if BLE_HID_DEVICE
#include <string.h>                                 
#include "app.h"                                                    // application definitions
#include "app_task.h"                                               // application task definitions
#include "app_utils.h"                                              // arch_printf() etc.

#include "gap.h"

#if BLE_DIS_SERVER
#include "diss_task.h"
#endif // BLE_DIS_SERVER

#if BLE_BATT_SERVER
#include "bass_task.h"
#endif // BLE_BATT_SERVER

#include "hogpd_task.h"                                             // hid functions
#include "gapc_task.h"
#include "llm_task.h"
#include "co_bt.h"
#include "arch.h"                                                   // platform definitions

#define __RETAINED __attribute__((section("exchange_mem_case2")))


__STATIC void kbd_enable_kbd_irq(void);

__STATIC bool kbd_reports_en __RETAINED;                            // flag to switch between key reporting and passcode entry modes

__STATIC uint32_t passcode   __RETAINED;                            // used to store the passcode in passcode mode

__STATIC uint16 kbd_keycode_buffer[KEYCODE_BUFFER_SIZE] __RETAINED; // Buffer to hold the scan results for the key presses / releases
                             
__STATIC uint8_t kbd_keycode_buffer_head __RETAINED;                // Read pointer for accessing the data of the keycode buffer
                             
__STATIC uint8_t kbd_keycode_buffer_tail __RETAINED;                // Write pointer for writing data to the keycode buffer
                             
volatile uint8_t kbd_do_scan_param __RETAINED;                      // low priority processing - Set by SysTick and Keyboard Handlers to enable SW scanning
                             
__STATIC bool kbd_keyreport_full __RETAINED;                        // The Key Report is full!
                             
__STATIC uint8_t kbd_key_report[MAX_REPORTS][8] __RETAINED;         // Key Report buffers
                             
__STATIC uint8_t normal_key_report_st[8] __RETAINED;                // Holds the contents of the last Key Report for normal keys sent to the Host

bool normal_key_report_ack_pending __RETAINED;                      // Keeps track of the acknowledgement of the last Key Report for normal keys sent to the Host

__STATIC uint8_t extended_key_report_st[3] __RETAINED;              // Holds the contents of the last Key Report for special functions sent to the Host

bool extended_key_report_ack_pending __RETAINED;                    // Keeps track of the acknowledgement of the last Key Report for special functions sent to the Host

__STATIC kbd_rep_info report_list[MAX_REPORTS] __RETAINED;          // The list of the reports instances (free or used)
                             
kbd_rep_info *kbd_trm_list __RETAINED;                              // Linked list of the pending Key Reports

__STATIC kbd_rep_info *kbd_free_list __RETAINED;                    // Linked list of the free Key Reports
    

bool user_extended_sleep;

__STATIC uint8_t kbd_membrane_status;                               // 0: outputs are low => SW is prohibited, 1: outputs are high-Z => SW can do scanning
__STATIC scan_t kbd_scandata[KBD_NR_OUTPUTS];                       // last status of the keyboard
__STATIC scan_t kbd_new_scandata[KBD_NR_OUTPUTS];                   // current status of the keyboard
__STATIC int kbd_fn_modifier;                                       // Fn key has been pressed

__STATIC int scan_cycle_time;                                       // time until the next wake up from SysTick
__STATIC uint16_t kbd_scanstate;                                    // state of the State Machine
__STATIC bool full_scan;                                            // when true a full keyboard scan is executed once. else only partial scan is done.
__STATIC bool kbd_kbint_event;                                      // Got an interrupt (key press) during partial scanning

__STATIC int kbd_output_mode_regs[KBD_NR_OUTPUTS];                  // MODE_REGs for the output GPIOs
__STATIC int kbd_output_reset_data_regs[KBD_NR_OUTPUTS];            // RESET_DATA_REGs for the output GPIOs
__STATIC uint16 kbd_out_bitmasks[KBD_NR_OUTPUTS];                   // mask to validate output GPIOs
__STATIC int kbd_input_mode_regs[KBD_NR_INPUTS];                    // MODE_REGs for the input GPIOs

__STATIC uint8_t kbd_bounce_active;                                 // flag indicating we are still in debouncing mode
__STATIC uint16 kbd_bounce_intersections[DEBOUNCE_BUFFER_SIZE];     // holds output - input pair (key) for which debouncing is on
__STATIC uint16 kbd_bounce_counters[DEBOUNCE_BUFFER_SIZE];          // counter for each intersection (key)





bool user_disconnection_req = false;

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

#ifdef ASCII_DEBUG
__STATIC  inline uint8 hut_to_ascii(const uint8 hut)
{
	// roughly
	return (hut & 0x80) ? '?' : kbd_hut_to_ascii_table[hut];
}
#endif


/*
 * (Report) List management functions
 ****************************************************************************************
 */
__STATIC void kbd_init_lists(void)
{
	int i;
	kbd_rep_info *node;
	
	kbd_trm_list = NULL;
	
	for (i = 0; i < MAX_REPORTS; i++) {
		node = &report_list[i];
		node->pBuf = kbd_key_report[i];
		node->type = FREE;
		node->modifier_report = false; // normal keys
		node->pNext = kbd_free_list;
		kbd_free_list = node;
	}
}

// pull one from the beginning
__STATIC kbd_rep_info *kbd_pull_from_list(kbd_rep_info **list)
{
	kbd_rep_info *node;
	
	if (!(*list))
		return NULL;
	
	node = *list;
	*list = node->pNext;
	node->pNext = NULL;
	
	return node;
}
	
// push one at the end
__STATIC int kbd_push_to_list(kbd_rep_info **list, kbd_rep_info *node)
{
	kbd_rep_info *p;
	
	if (!node)
		return 0;
	
	if (!(*list))
		*list = node;
	else {
		for (p = *list; p->pNext != NULL; p = p->pNext);
		
		p->pNext = node;
		node->pNext = NULL;
	}
	
	return 1;
}

#if 0
// finds the last instance of a specific type
__STATIC kbd_rep_info *kbd_search_in_list(kbd_rep_info *list, enum KEY_BUFF_TYPE type)
{
	kbd_rep_info *node;
	kbd_rep_info *last_found = NULL;
	
	for (node = list; node != NULL; node = node->pNext)
		if (node->type == type)
			last_found = node;
	
	return last_found;
}
#endif	

/*
 * SYSTICK functions
 ****************************************************************************************
 */
static inline void systick_start(const int ticks, const int mode /* = 0*/)
{
	SetWord32(0xE000E010, 0x00000000);      // disable systick
	SetWord32(0xE000E014, ticks);           // set systick timeout based on 16MHz clock
	SetWord32(0xE000E018, ticks);           // set systick timeout based on 16MHz clock
	GetWord32(0xE000E010);                  // make sure COUNTFLAG is reset
	SetWord32(0xE000E010, 1|/*4|*/mode);    // enable systick on 1MHz clock (always ON) with interrupt (mode)
}

__attribute__((unused)) static inline void systick_wait() 
{
	while (0 == (GetWord32(0xE000E010) & 0x00010000)) {} // wait for COUNTFLAG to become 1
}

static inline void systick_stop()
{
	// leave systick in a known state
	SetWord32(0xE000E010, 0x00000000);      // disable systick
	GetWord32(0xE000E010);                  // make sure COUNTFLAG is reset
}


/*
 * GPIO handling 
 ****************************************************************************************
 */
static inline void set_row_to_low(int idx)
{
    if (kbd_out_bitmasks[idx]) {
        SetWord16(kbd_output_reset_data_regs[idx], kbd_out_bitmasks[idx]);  // output low
        SetWord16(kbd_output_mode_regs[idx], 0x300);                        // mode gpio output
    }
}

static inline void set_column_to_input_pullup(int idx)
{
    if (kbd_input_mode_regs[idx])
        SetWord16(kbd_input_mode_regs[idx], 0x100);                         // mode gpio input pullup
}

static inline void set_row_to_input_highz(int idx)
{
    if (kbd_out_bitmasks[idx])
        SetWord16(kbd_output_mode_regs[idx], 0x000);                        // mode gpio input highz
}


/*
 * Keyboard "membrane" handling
 ****************************************************************************************
 */

// Set all columns to "input pull-up" state
static void kbd_membrane_input_setup()
{	
	for (int i = 0; i < KBD_NR_INPUTS; ++i) 
        set_column_to_input_pullup(i);
}

// Set all rows to "high-Z" state to enable SW scanning
static void kbd_membrane_output_wakeup()
{
	for (int i = 0; i < KBD_NR_OUTPUTS; ++i)
        set_row_to_input_highz(i);
	
	kbd_membrane_status = 1;
}

// Set all rows to "low" state to enable HW scanning (WKUP-TIMER)
static void kbd_membrane_output_sleep()
{	
	kbd_membrane_status = 0;
	
	// Pull all outputs low to quickly discharge the membrane.
	for(int i = 0; i < KBD_NR_OUTPUTS; ++i) {
        set_row_to_low(i);
	}
	
	// The inputs are pullup, so their transition to low will trigger the keyboard interrupt
}



/*
 * Keyboard initialization
 ****************************************************************************************
 */


/*
 * Name         : kbd_init_retained_scan_vars - Initializes all retained scan variables 
 *
 * Scope        : PRIVATE
 *
 * Arguments    : none
 *
 * Description  : Handles the initialization of all retained variables used for key 
 *                scanning.
 *
 * Returns      : void
 *
 */
static void kbd_init_retained_scan_vars(void)
{
    int i;
    
	kbd_keycode_buffer_head = 0;
    kbd_keycode_buffer_tail = 0;

	uint16 const *base = (uint16 *)P0_DATA_REG;
	
	for (i = 0; i < KBD_NR_OUTPUTS; ++i)
	{
        int port = kbd_output_ports[i];
        if (port >= 0x30) port += 0x10; // fix for the mapping of port 3 regs!
        uint16 const *data_reg = &base[port & 0xF0];
        const int databit = (port & 0xF);
        const int bitmask = 1<<databit;
        kbd_out_bitmasks[i] = (port < 0x50) ? bitmask : 0;

        kbd_output_reset_data_regs[i] = (int)&(data_reg[2]);
        kbd_output_mode_regs[i] = (int)&(data_reg[3 + databit]);
	}
	
	for (i = 0; i < KBD_NR_INPUTS; ++i)
	{
        int port = kbd_input_ports[i];
        
        if (port >= 0x40) {
            kbd_input_mode_regs[i] = 0;
            continue;
        }
        
        if (port >= 0x30) port += 0x10; // fix for the mapping of port 3 regs!
        
        uint16 const *data_reg = &base[port & 0xF0];
        const int databit = (port & 0xF);

        kbd_input_mode_regs[i] = (int)&(data_reg[3 + databit]);
	}
}

/*
 * Name         : kbd_init_scan_vars - Initializes all non retained scan variables 
 *
 * Scope        : PRIVATE
 *
 * Arguments    : none
 *
 * Description  : Handles the initialization of all non retained variables used for key 
 *                scanning.
 *
 * Returns      : void
 *
 */
static void kbd_init_scan_vars(void)
{
    int i;
    
	kbd_scanstate = 0;
    full_scan = false;
    kbd_kbint_event = false;
    
	kbd_membrane_status = 0;

	kbd_scanstate = 0;
	kbd_fn_modifier = 0;
    
	for (i = 0; i < DEBOUNCE_BUFFER_SIZE ; ++i)
	{
		kbd_bounce_intersections[i] = 0xFFFF;
		kbd_bounce_counters[i] = 0;
	}
	kbd_bounce_active = 0;
	
#ifdef ASCII_DEBUG
	kbd_keybuffer_idx = 0;
#endif
	
	for (i = 0; i < KBD_NR_OUTPUTS; ++i)
		kbd_scandata[i] = (1 << KBD_NR_INPUTS) - 1;
}

// retainable data - call this function once at the beginning
static void kbd_init_keyreport(void)
{
	int i;
	
	for (i = 0; i < MAX_REPORTS; i++)
		memset(kbd_key_report[i], 0, 8);
	kbd_init_lists();
    
	normal_key_report_st[0] = 0xFF;     // invalidate
    normal_key_report_ack_pending = false;
    
    extended_key_report_st[0] = 0;
    extended_key_report_st[1] = 0;
    extended_key_report_st[2] = 0;
    extended_key_report_ack_pending = false;
    
	kbd_keyreport_full = false;
}


/*
 * Keyboard scanning
 ****************************************************************************************
 */

// Returns: true when all valid outputs have been scanned so as to proceed with the next FSM state
static inline int kbd_scan_matrix()
{
	// 1. Find which row has the pressed key 
	// 2. Drive this row 'low' for 1 scan cycle
	// In the next scan cycle: 
	// 3. Read inputs
	// 4. Turn the row to 'high-Z'
	// 5. Check if there's another row with a pressed key and if so go to 2
	// 6. Program Keyboard Controller to scan the rest of the rows
	// 7. Exit
	
	uint8_t i = kbd_scanstate & 0xFF;
	int8_t prev_line = -1;
	const scan_t scanmask = (1 << KBD_NR_INPUTS) - 1;
//	uint32_t kbd_inputs;

	// Update SysTick next tick time
	scan_cycle_time -= (int)((int)(KBD_US_FAST_SCAN / (KBD_NR_OUTPUTS+SPLIT_PROCESS_KBD_SCANDATA+FSM_STEPS)) * SYSTICK_TICKS_PER_US );
	
	// Step 0. Check if we've already driven a row to low. In that case we should scan the inputs...
	if ( (i > 0) && (GetWord16(kbd_output_mode_regs[i-1]) == 0x300) ) 
		prev_line = i - 1;
	
	// Step 1. Find which row has the pressed key
	if (!full_scan)
		for (; i < KBD_NR_OUTPUTS; ++i) {
			if(kbd_out_bitmasks[i]) { // valid output
				if(kbd_scandata[i] != scanmask) //This line has a pressed key
					break;
			}
		}
	
	kbd_scanstate &= 0xFF00;
	kbd_scanstate += i;

	if ( prev_line != -1 )
	{
		scan_t scanword = 0;
		volatile scan_t my_scanword = 0;
        uint16 kbd_gpio_in[5]; // no need to be global but I'll leave it for now...
		
        kbd_gpio_in[4] = 0xFFFF;
        
		// Step 3. Read the input registers.
#ifdef P0_HAS_INPUT					
		kbd_gpio_in[0] = GetWord16(P0_DATA_REG);
#endif
#ifdef P1_HAS_INPUT					
		kbd_gpio_in[1] = GetWord16(P1_DATA_REG);
#endif
#ifdef P2_HAS_INPUT					
		kbd_gpio_in[2] = GetWord16(P2_DATA_REG);
#endif
#ifdef P3_HAS_INPUT					
		kbd_gpio_in[3] = GetWord16(P3_DATA_REG);
#endif
//		kbd_inputs = GetWord16(P0_DATA_REG) | GetWord16(P1_DATA_REG) << 10 | GetWord16(P2_DATA_REG) << 20;

		// Step 4. Turn previous output to "high-Z" (input)
		if (kbd_out_bitmasks[prev_line])
			SetWord16(kbd_output_mode_regs[prev_line], 0x000);    // mode gpio input highz

		// Process input data
		// we're waiting for the signals to stabilize for the the next readout
		// so this is a great time to spend some cycles to process the previous readout
			
		// fill in the scanword bits (we have plenty of time to burn these cycles
		for (int j = KBD_NR_INPUTS - 1; j >= 0; --j)
		{
			const uint8_t input_port = kbd_input_ports[j];
			scanword = (scanword << 1) | ((kbd_gpio_in[input_port >> 4] >> (input_port & 0x0F)) & 1);
//			my_scanword = (my_scanword << 1) | ((kbd_inputs >> kbd_input_ports_test[j]) & 1);
		}
		kbd_new_scandata[prev_line] = scanword;
	} 

	// Step 2. Pull next "used" output low hard
	if (i < KBD_NR_OUTPUTS && kbd_out_bitmasks[i]) {
		SetWord16(kbd_output_reset_data_regs[i], kbd_out_bitmasks[i]);  // level 0
		SetWord16(kbd_output_mode_regs[i], 0x300);                      // mode gpio output
	}

	kbd_scanstate++;
	
	if (full_scan) {
		if (i == KBD_NR_OUTPUTS) {
			full_scan = false;
        }
	} else if (i == KBD_NR_OUTPUTS) { //All scanwords are empty. We've finished scanning the rows that have keys being pressed...
		// Step 6. Program Keyboard Controller to scan the rest of the rows
		uint8_t j;
		
		//All outputs are in high-Z now. Enable Keyboard Controller
		if ((app_env.app_flags & KBD_CONTROLLER_ACTIVE) == 0)
			kbd_enable_kbd_irq();
		
		//Find which rows have not any key being pressed, drive them low and calculate for how long the Keyboard Controller will be active
		for (j = 0; j < KBD_NR_OUTPUTS; ++j)
			if(kbd_new_scandata[j] == scanmask) { //This line is "used" and has no pressed keys
				// Pull output low hard
				SetWord16(kbd_output_reset_data_regs[j], kbd_out_bitmasks[j]);  // level 0
				SetWord16(kbd_output_mode_regs[j], 0x300);                      // mode gpio output
			}
		
		if (scan_cycle_time > 0) {
			scan_cycle_time += GetWord32(0xE000E018);
			systick_stop();
			systick_start(scan_cycle_time, 2);
		}
	}

	return (i == KBD_NR_OUTPUTS); 
}

// returns the unread entries in the keycode_buffer
static inline int keycode_buffer_written_sz(void)
{
	int ret_val;
	
	if (kbd_keycode_buffer_tail >= kbd_keycode_buffer_head)
		ret_val = kbd_keycode_buffer_tail - kbd_keycode_buffer_head;
	else
		ret_val = KEYCODE_BUFFER_SIZE + kbd_keycode_buffer_tail - kbd_keycode_buffer_head;
	
	return ret_val;
}

// adds a key report of the specified type in the trm list
static kbd_rep_info* add_report(enum KEY_BUFF_TYPE type)
{
    kbd_rep_info *pReportInfo, *_pReportInfo, *pTmpRep;
    
    // Get the last pending normal key report, if any
    if (!kbd_trm_list)
        _pReportInfo = NULL;
    else {
        pTmpRep = NULL;

        _pReportInfo = kbd_trm_list;
        do {
            if (_pReportInfo->char_id == 0) 
                pTmpRep = _pReportInfo;
            _pReportInfo = _pReportInfo->pNext;
        } while (_pReportInfo != NULL);

        _pReportInfo = pTmpRep;
    }

    // add one <type> report
    pReportInfo = kbd_pull_from_list(&kbd_free_list);
    DEBUG_BP(!pReportInfo);
    pReportInfo->type = type;
    pReportInfo->modifier_report = false;
    pReportInfo->char_id = 0;
    pReportInfo->len = 8;

    if (!_pReportInfo) { // first entry - copy last one sent
        if (normal_key_report_st[0] != 0xFF) {
            memcpy(pReportInfo->pBuf, normal_key_report_st, 8);
        } else {
            memset(pReportInfo->pBuf, 0, 8); // should not happen
        }
    } else /*if (_pReportInfo)*/ // last report pending 
        memcpy(pReportInfo->pBuf, _pReportInfo->pBuf, 8);

    kbd_push_to_list(&kbd_trm_list, pReportInfo);

    return pReportInfo;
}

// returns 0 when full, 1 when done, -1 when full but the current keycode has to be processed again
static inline int modify_kbd_keyreport(const char keymode, const char keychar, uint8_t pressed)
{
    int i;
    kbd_rep_info *pReportInfo, *_pReportInfo, *pTmpRep;

    // 1. The Key Report is filled from pos 2 to pos 7. It monitors the state of up to 6 keys. If more are pressed then
    //    RollOver functionality (Phantom state) should be applied.
    // 2. When something is written at a pos the host translates it as a key press.
    // 3. When something is written at (pos+1) the host translates it as a key press for key@(pos+1)
    // 4. If the report has '0' in all other positions then the host we'll be informed for only these two key presses.  
    //    The host will think that the last entry written (i.e. key@(pos+1)) is still being pressed and update it 
    //    (i.e. in a Word Processing application) with the "key repetition rate" as set in its settings.
    // 5. For the host to be informed for a key release of key@(pos+1) there are two options:
    //    a. A Key Report is sent with all '0's which clears all key presses.
    //    b. The last Key Report is sent with 0x00@(pos+1) which means that the rest of the keys are still being pressed.
    // 6. Allowed trm sequence for all keys: PRESS (x N) -> RELEASE -> PRESS (x N) -> RELEASE..., 
    //                                       RELEASE -> PRESS (x N) -> RELEASE -> PRESS (x N) -> RELEASE...

    switch(keymode & 0xFC)
    {
        case 0x00: // normal key
        {
            if (pressed) {
                // add one press report
                pReportInfo = add_report(PRESS);

                for (i = 2; i < 8; i++)
                    if (!pReportInfo->pBuf[i])
                        break;

                if (i == 8) // full! FIXME: RollOver
                    return 0;

                pReportInfo->pBuf[i] = keychar; 				
			} else { // released
                // add one RELEASE report
                pReportInfo = add_report(RELEASE);

                for (i = 2; i < 8; i++) 
                    if (pReportInfo->pBuf[i] == keychar) {
                        pReportInfo->pBuf[i] = 0x00;
                        break;
                    }
			}
            
            break;
        }
			
        case 0xFC: // modifier key
        {
            // if pressed, add a PRESS report after copying the data of the last report pending
            // if released, add a RELEASE report after copying the data of the last report pending
            char modifier, new_modifier;
            uint8_t *pLastKeyStatus = NULL;

            // get last report to find the modifiers' status
            if (!kbd_trm_list) {
                if (normal_key_report_st[0] != 0xFF)
                    pLastKeyStatus = normal_key_report_st;
                else
                    /*pLastKeyStatus = NULL*/;
            } else {
                pTmpRep = NULL;

                _pReportInfo = kbd_trm_list;
                do {
                    if (_pReportInfo->char_id == 0) 
                        pTmpRep = _pReportInfo;
                    _pReportInfo = _pReportInfo->pNext;
                } while (_pReportInfo != NULL);

                if (pTmpRep)
                    pLastKeyStatus = pTmpRep->pBuf;
                else
                    /*pLastKeyStatus = NULL*/;
            } 
            
            if (pLastKeyStatus)
                modifier = pLastKeyStatus[0];
            else
                modifier = 0x00;
            
            new_modifier = (modifier & (~keychar)) | (pressed ? keychar : 0);
            
            if (new_modifier != modifier) { // normally this will always be true
                // add "modifier" report in the trm list.
                pReportInfo = kbd_pull_from_list(&kbd_free_list);
                DEBUG_BP(!pReportInfo);

                if (pressed) 
                    pReportInfo->type = PRESS;
                else
                    pReportInfo->type = RELEASE;
                    
                pReportInfo->modifier_report = true;
                pReportInfo->char_id = 0;
                pReportInfo->len = 8;

                // copy last key status
                if (pLastKeyStatus)
                    memcpy(pReportInfo->pBuf, pLastKeyStatus, 8);
                else
                    memset(pReportInfo->pBuf, 0, 8);

                kbd_push_to_list(&kbd_trm_list, pReportInfo);

                pReportInfo->pBuf[0] = new_modifier;
            }
            break;
        }

        default: // Other key that is not directly reportable in the kbd_key_report
            break;
    }
	
    return 1;
}

// Returns: 1 if a key report is pending or there are unread entries in the keycode buffer
static inline bool kbd_buffer_has_data(void) 
{ 
    return (kbd_keycode_buffer_head != kbd_keycode_buffer_tail); 
}

// Returns: true if we are waiting for a key to be released
static inline bool wait_for_key_release(void) 
{
    int i;
    const scan_t scanmask = (1 << KBD_NR_INPUTS) - 1;

    for (i = KBD_NR_OUTPUTS - 1; i >= 0; --i)
        if (kbd_scandata[i] != scanmask) // wait for key release...
            return true;

    return false; 
}

// returns 0 when full, 1 when done, -1 when full but the current keycode has to be processed again
static int kbd_process_keycode(uint16 keycode)
{
    int ret = 1;

    if (keycode)
    {
        const char keymode = keycode >> 8;
        const char keychar = keycode & 0xFF;
        uint8_t pressed = (~keymode) & 1;

#ifdef ASCII_DEBUG
        if (pressed)
        {
            switch(keymode & 0xFE)
            {
                case 0x00: // normal key
                    kbd_buffer_ascii[kbd_keybuffer_idx] = kbd_hut_to_ascii(keychar);
                    break;
                default: // Other key
                    kbd_buffer_ascii[kbd_keybuffer_idx] = '~';
                    break;
            }
            arch_printf("[%c]\r\n", kbd_buffer_ascii[kbd_keybuffer_idx]);
            kbd_keybuffer_idx = (kbd_keybuffer_idx + 1) & (MAX_KEYBUFFER-1);
        }
#endif

        switch(keymode & 0xFC)
        {
            case 0x00: // normal key
            case 0xFC: // modifier key
                ret = modify_kbd_keyreport(keymode, keychar, pressed);
				break;
			case 0xF8: // Fn Modifier
            {
                // When Fn key is pressed or released all key state must be reset
                // On release, optionally, a reset of kbd_scandata and a full key scan can be done to
                // re-detect any other keys that were pressed with the Fn key!
                kbd_rep_info *pReportInfo;
                
                // add a "full release" report in the trm list.
                pReportInfo = add_report(RELEASE);
                memset(pReportInfo->pBuf, 0, 8); 
                
                kbd_fn_modifier = (kbd_fn_modifier & (~keychar)) | (pressed ? keychar : 0);
                break;
            }
            case 0xF4: // Special function
                {
                    int byte = keychar >> 4;
                    uint8_t code = keychar & 0x0F;
                    uint8_t mask = 1 << (code);                     // Low order nibble gets values up to 7!
                    kbd_rep_info *pReportInfo;
                    
                    if (byte == 0xF) {                              // Exception: do something proprietary!
                        switch (code) {
                        case 0: // ~
                            if (pressed) {
                                kbd_process_keycode(0xFC02);        // Left Shift
                                kbd_process_keycode(0x0035);        // ~
                            } else {
                                kbd_process_keycode(0xFD02);        // Clear Left Shift
                                kbd_process_keycode(0x0135);        // Clear ~
                            }
                            break;
            #if (BLE_ALT_PAIR)
                        case 1: // pair to another
                            if (!pressed)
                                user_disconnection_req = true;
                            break;
                        case 2: // clear pairing data (EEPROM)
                            if (!pressed)
                                app_alt_pair_clear_all_bond_data();
                            break;
            #endif // BLE_ALT_PAIR  
            #if (KEYBOARD_MEASURE_EXT_SLP)
                        case 3: // put the chip in extended sleep constantly (for measurements)
                            if (pressed)
                                user_extended_sleep = true;
                            break;
            #endif // KEYBOARD_MEASURE_EXT_SLP
                        default:
                            break;
                        }
                    } else { // Normal case
                        // Add an extended key report for each press / release
                        pReportInfo = kbd_pull_from_list(&kbd_free_list);
                        DEBUG_BP(!pReportInfo);

                        pReportInfo->type = EXTENDED;
                        pReportInfo->modifier_report = false;
                        pReportInfo->char_id = 2;
                        pReportInfo->len = 3;
                        
                        if (pressed)
                            extended_key_report_st[byte] |= mask;
                        else
                            extended_key_report_st[byte] &= ~mask;
                        
                        memcpy(pReportInfo->pBuf, extended_key_report_st, 3); 
                        kbd_push_to_list(&kbd_trm_list, pReportInfo);
                    }
                    break;
                }
            default: // Other key that is not directly reportable in the kbd_key_report
                break;
        }
    }
    return ret;
}

// 0: failed, 1: OK
static int prepare_kbd_keyreport(void)
{
    ke_state_t app_state = ke_state_get(TASK_APP);
    if (app_state != APP_SECURITY && app_state != APP_PARAM_UPD && app_state != APP_CONNECTED) {
        if (ke_state_get(TASK_LLM) != LLM_ADVERTISING) {
//			app_adv_restart();
            app_adv_start();
        }
		
        return 0;
    }

    if (kbd_keycode_buffer_head == kbd_keycode_buffer_tail) 
        return 0;

    if (kbd_keyreport_full)
        return 0;

    if (kbd_free_list == NULL)
        return 0;

    do {
        int ret;
        
        ret = kbd_process_keycode(kbd_keycode_buffer[kbd_keycode_buffer_head]);
        if (ret == -1 || ret == 0) 
            kbd_keyreport_full = true;
        
        if (ret != -1)
            kbd_keycode_buffer_head = (kbd_keycode_buffer_head + 1) % KEYCODE_BUFFER_SIZE;
    } while ( kbd_free_list && !kbd_keyreport_full && (keycode_buffer_written_sz() > 0) );

    return 1;
}

// 0: did not send, 1: Report sent
static inline int send_kbd_keyreport(void)
{
    kbd_rep_info *p;
    struct hogpd_report_info *req;
    int ret = 0;

    do {
        // Allocate the message
#ifdef HOGPD_BOOT_PROTO 
        req = KE_MSG_ALLOC_DYN(HOGPD_BOOT_KB_IN_UPD_REQ, TASK_HOGPD, TASK_APP, hogpd_report_info, 8);
#else        
        req = KE_MSG_ALLOC_DYN(HOGPD_REPORT_UPD_REQ, TASK_HOGPD, TASK_APP, hogpd_report_info, 8);
#endif        
        if (!req)
            break;

        kbd_keyreport_full = false;
        p = kbd_pull_from_list(&kbd_trm_list);
        
        // Fill in the parameter structure
        req->conhdl = app_env.conhdl;
        req->hids_nb = 0;
        req->report_nb = p->char_id;
        req->report_length = p->len;
        memcpy(req->report, p->pBuf, 8);

        arch_printf("Sending HOGPD_REPORT_UPD_REQ %02x:[%02x:%02x:%02x:%02x:%02x:%02x]\r\n", 
                    (int)p->pBuf[0], (int)p->pBuf[2], (int)p->pBuf[3], (int)p->pBuf[4], (int)p->pBuf[5], (int)p->pBuf[6], (int)p->pBuf[7]);
                    
        ke_msg_send(req);

        switch (p->char_id) {
        case 0:
            memcpy(normal_key_report_st, p->pBuf, 8);
            normal_key_report_ack_pending = true;
            break;
        case 2:
            memcpy(extended_key_report_st, p->pBuf, 3);
            extended_key_report_ack_pending = true;
            break;
        default:
            break;
        }
        
        p->type = FREE;
        kbd_push_to_list(&kbd_free_list, p);
        
        ret = 1;
    } while (0);

    return ret;
}

// return zero if the keypress or release is ignored (debounce, ghost, buffer full)
static inline int record_key(const uint16 output, const uint16 input, const int pressed)
{
    // first, debounce based on output+input+pressed
    const uint16 my_intersection = (output << 8) | input;
    int empty_slot = -1;
    for (int i = 0; i < DEBOUNCE_BUFFER_SIZE ; ++i)
    {
        const uint16 intersection = kbd_bounce_intersections[i];
        if (my_intersection == intersection) return 0;
        if (intersection == 0xFFFF) empty_slot = i;
    }
    if (-1 == empty_slot) return 0;

    kbd_bounce_intersections[empty_slot] = my_intersection;
    kbd_bounce_counters[empty_slot] = pressed ? DEBOUNCE_COUNTER_PRESS : DEBOUNCE_COUNTER_RELEASE;

    // No bounce, continue to check for ghosting


    if (pressed)
    {
        // Deghosting magic happens here
        {
            scan_t imask = 1 << input;
            scan_t nimask = ((~(imask)) & ((1 << KBD_NR_INPUTS) - 1));
            for(int o = 0; o < KBD_NR_OUTPUTS; ++o)
            {
                scan_t scandata = kbd_scandata[o];
                if (output == o) continue;//scandata &= ~imask;
                if (!(scandata & imask))
                {
                    scandata &= kbd_scandata[output];
                    // then for every input that is low (other than input), there can't be a key defined at output
                    scan_t mask = 1;
                    if (scandata != nimask) for (int i = 0; i < KBD_NR_INPUTS; ++i, mask<<=1)
                    {
//						uint16 nmimask = nimask & (~mask);
                        if (input == i) continue;
                        if (!(scandata & mask))
                        {
                            for(int o2 = 0; o2 < KBD_NR_OUTPUTS; ++o2)
                            {
                                if ((output == o2) || (o == o2)) continue;
                                if (!(kbd_scandata[o2] & mask))
                                {
                                    return 0;
                                }
                            }
                            
                            if ((output != o) || (input != i))
                            {
                                // FIXME kbd_fn_modifier and buffering...
                                if (kbd_keymap[kbd_fn_modifier][output][i] &&
                                    kbd_keymap[kbd_fn_modifier][o][input] &&
                                    kbd_keymap[kbd_fn_modifier][o][i]
                                   )
                                {
                                    return 0;
                                }
                            }
                        }
                    }
                }
            }
        }
        
        {
            scan_t imask = 1 << input;
            const scan_t scandata = kbd_scandata[output] & ~imask;
            scan_t mask = 1;
            if (scandata != ~imask) for (int i = 0; i < KBD_NR_INPUTS; ++i, mask<<=1)
            {
                if (input == i) continue;
                if (!(scandata & mask))
                {
                    for(int o = 0; o < KBD_NR_OUTPUTS; ++o)
                    {
                        scan_t oscandata = kbd_scandata[o];
                        if (output == o) continue;//oscandata &= ~imask;
                        if (!(oscandata & mask))
                        {
                            oscandata &= ~imask;
                            scan_t nmimask = ((~(mask|imask)) & ((1 << KBD_NR_INPUTS) - 1));
                            if (oscandata != nmimask)
                            {
                                return 0;
                            }
                            
                            if ((output != o) || (input != i))
                            {
                                // FIXME kbd_fn_modifier and buffering...
                                if (kbd_keymap[kbd_fn_modifier][output][i] &&
                                    kbd_keymap[kbd_fn_modifier][o][input] &&
                                    kbd_keymap[kbd_fn_modifier][o][i]
                                   )
                                {
                                    return 0;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // if no ghosting, then continue to buffer.
    int next_tail = (kbd_keycode_buffer_tail + 1) % KEYCODE_BUFFER_SIZE;
    if (next_tail != kbd_keycode_buffer_head) //FIXME : The check for buffer space should be at the begining. It's fool to do all this work just to find out that there's no space...
    {
        kbd_keycode_buffer[kbd_keycode_buffer_tail] = kbd_keymap[kbd_fn_modifier][output][input] | (pressed ? 0 : (0x0100));
        kbd_keycode_buffer_tail = next_tail;
        return 1;
    }
    return 0;
}

// processes scan results and records the keys in the keycode buffer
static inline void kbd_process_scandata()
{	
    // Now extract the (new) keys
#if SPLIT_PROCESS_KBD_SCANDATA
    const int i = kbd_scanstate & 0xFF;
#else
    for(int i = 0; i < KBD_NR_OUTPUTS; ++i)
#endif
    if (kbd_out_bitmasks[i])
    {
        const scan_t scanword = kbd_scandata[i];
        scan_t newscanword = kbd_new_scandata[i];
        scan_t xorword = scanword ^ newscanword;
        
        if (xorword) // if any key state changed
        {
            scan_t bit = 0;
            scan_t mask = 1;
            do
            {
                if (xorword & 1)
                {
                    int press = scanword & mask;
                    if (!record_key(i, bit, press))
                    {
                        // This keypress is ignored, copy the bit from kbd_scandata[i] to newscanword
                        newscanword = (newscanword & (~mask)) | press;
                    }
                }
                
                int shift = 1;
                if (!(xorword & 0x0F)) 
                {
                    shift = 4;
                    if (!(xorword & 0xFF)) 
                    {
                        shift = 8;
                        if (!(xorword & 0xFFF)) shift = 12;
                    }
                } 
                xorword >>= shift;
                mask <<= shift;
                bit += shift;					
            } while (xorword);
            
            kbd_scandata[i] = newscanword;
        }
    }
#if SPLIT_PROCESS_KBD_SCANDATA
    kbd_scanstate++;
    if (i >= (KBD_NR_OUTPUTS-1)) kbd_scanstate = (kbd_scanstate & ~0xFF) + 0x0100;
#else
    kbd_scanstate = (kbd_scanstate & ~0xFF) + 0x0100;
#endif
}


/*
 * Name         : kbd_start_sw_scanning - start scanning 
 *
 * Scope        : PRIVATE
 *
 * Arguments    : none
 *
 * Description  : Activates key scanning hardware and state machine (by enabling the SysTick IRQ).
 *
 * Returns      : void
 *
 */
static void kbd_start_sw_scanning(void)
{
    //arch_puts("kbd_start_sw_scanning()\r\n");

#ifdef ALTERNATIVE_SCAN_TIMES	
    scan_cycle_time = (KBD_US_FAST_SCAN * SYSTICK_TICKS_PER_US) - (int)((int)(KBD_US_FAST_SCAN / (KBD_NR_OUTPUTS+SPLIT_PROCESS_KBD_SCANDATA+FSM_STEPS)) * SYSTICK_TICKS_PER_US );
#else    
    scan_cycle_time = (KBD_US_PER_SCAN * SYSTICK_TICKS_PER_US) - (int)((int)(KBD_US_FAST_SCAN / (KBD_NR_OUTPUTS+SPLIT_PROCESS_KBD_SCANDATA+FSM_STEPS)) * SYSTICK_TICKS_PER_US );
#endif    

    full_scan = true;                   // Force a full key scan
    
    GLOBAL_INT_DISABLE();
    app_env.app_flags |= KBD_SCANNING;
    kbd_membrane_output_wakeup();       // Set outputs to 'high-Z' to enable SW scanning

    NVIC_SetPriority(SysTick_IRQn, 3);         
    NVIC_EnableIRQ(SysTick_IRQn);
    systick_start((int)((int)(KBD_US_FAST_SCAN / (KBD_NR_OUTPUTS+SPLIT_PROCESS_KBD_SCANDATA+FSM_STEPS)) * SYSTICK_TICKS_PER_US ), 2);
    GLOBAL_INT_RESTORE();
}


/*
 * Name         : kbd_enable_kbd_irq - Sets up the Keyboard Controller 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : It programs the Keyboard Controller to scan any key press 
 *                coming from the "inactive rows" and inform the system after 
 *                DEBOUNCE_TIME_PRESS msec to do a full key scan.
 *
 * Returns      : void
 *
 */
__STATIC void kbd_enable_kbd_irq(void)
{
#define KBRD_IRQ_IN_SEL2_REG            (0x50001416) /* GPIO interrupt selection for KBRD_IRQ for P3 */    
    SetWord16(GPIO_DEBOUNCE_REG, 0x2000 | DEBOUNCE_TIME_PRESS); // T ms debouncing

    uint16 mask_p0 = 0x4000;
    uint16 mask_p12 = 0x0000;
    uint16 mask_p3 = 0x0000;
    for(int i = 0; i < KBD_NR_INPUTS; ++i)
    {
        const int port = kbd_input_ports[i];
        const int bit = port & 0x0F;
        switch(port & 0xF0)
        {
            case 0x00:
                mask_p0 |= 0x0001 << bit;
                break;
            case 0x10:
                mask_p12 |= 0x0400 << bit;
                break;
            case 0x20:
                mask_p12 |= 0x0001 << bit;
                break;
            case 0x30:
                mask_p3 |= 0x0001 << bit;
                break;
            default:
                break;
        }
    }

    SetWord16(KBRD_IRQ_IN_SEL2_REG, mask_p3);
    SetWord16(KBRD_IRQ_IN_SEL1_REG, mask_p12);
    SetWord16(KBRD_IRQ_IN_SEL0_REG, mask_p0);

    SetWord16(GPIO_RESET_IRQ_REG, 0x20);    // clear any garbage
    NVIC_ClearPendingIRQ(KEYBRD_IRQn);      // clear it to be on the safe side...

    kbd_kbint_event = false;

    NVIC_SetPriority(KEYBRD_IRQn, 2);         
    NVIC_EnableIRQ(KEYBRD_IRQn);
    app_env.app_flags |= KBD_CONTROLLER_ACTIVE;
}


/*
 * Name         : app_kbd_enable_wakeup_irq - Sets up the Wakeup Timer 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : It programs the Wakeup Timer to scan any key press 
 *                and wake-up the system after DEBOUNCE_TIME_PRESS msec.
 *
 * Returns      : void
 *
 */
void app_kbd_enable_wakeup_irq(void)
{
    uint16_t port_pins[4] = {0, 0, 0, 0};

    SetWord16(WKUP_RESET_CNTR_REG, 0);                              // Clear event counter (for safety...)
    SetWord16(WKUP_COMPARE_REG, 1);                                 // Wait for 1 event and wakeup

    for(int i = 0; i < KBD_NR_INPUTS; ++i)
    {
        const int gpio = kbd_input_ports[i];
        const int bit = gpio & 0x0F;
        const int port = (gpio & 0xF0) >> 4;

        if (port < 4)
            port_pins[port] |= (1 << bit);
    }

    SetWord16(WKUP_SELECT_P0_REG, port_pins[0]);
    SetWord16(WKUP_SELECT_P1_REG, port_pins[1]);
    SetWord16(WKUP_SELECT_P2_REG, port_pins[2]);
    SetWord16(WKUP_SELECT_P3_REG, port_pins[3]);

    SetWord16(WKUP_POL_P0_REG, port_pins[0]);                       // generate INT when input goes low
    SetWord16(WKUP_POL_P1_REG, port_pins[1]);    
    SetWord16(WKUP_POL_P2_REG, port_pins[2]);    
    SetWord16(WKUP_POL_P3_REG, port_pins[3]);    

    SetWord16(WKUP_RESET_IRQ_REG, 1);                               // clear any garbagge
    NVIC_ClearPendingIRQ(WKUP_QUADEC_IRQn);                         // clear it to be on the safe side...

    app_env.app_flags |= ENABLE_WKUP_INT;                           // Set flag for the periph_init()
    SetWord16(WKUP_CTRL_REG, 0x80 | (DEBOUNCE_TIME_PRESS & 0x3F));  // Setup IRQ: Enable IRQ, T ms debounce
    NVIC_SetPriority(WKUP_QUADEC_IRQn, 1);
    NVIC_EnableIRQ(WKUP_QUADEC_IRQn);    
}


/*
 * Name         : app_kbd_do_scan - Key scanning FSM 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : This is the key scanning FSM. Three states are defined:
 *                  0x0000 : KEY SCANNING
 *                           Every SysTick interval one output is pushed low and the inputs are 
 *                           read (scanned) to get a snapshot of the key activity of this row.
 *                  0x0100 : KEY PROCESSING
 *                           After all rows have been scanned, this step does debouncing and
 *                           deghosting. Then, it updates the key buffer, processes its data,
 *                           and prepares one or more Key Reports, if need be.
 *                  0x0400 : KEY_SCAN_INACTIVE
 *                           Key scanning is done. The hw is setup to scan any key so as to
 *                           wakeup the system.
 *
 * Returns      : void
 *
 */
void app_kbd_do_scan(void)
{
    kbd_do_scan_param = 0;

    if (!kbd_membrane_status) {
        arch_puts("kbd_do_scan() status=0 => param=0\r\n");
        return;
    }

//	arch_printf("scan %04x\r\n", kbd_scanstate);

    switch(kbd_scanstate & 0xFF00)
    {
        case 0x0000:
            if (kbd_scan_matrix()) kbd_scanstate = 0x0100; 
            break;
        
        case 0x0100:
            // FIXME: SysTick could also be resetted in the KEYBRD ISR which could also set the kbd_do_scan_param = 1 to force 
            //		  re-entry to the state machine and this would result to rescanning with minimal delay!
        
            // Stop SysTick
            systick_stop();
        
            GLOBAL_INT_DISABLE();
            if(app_env.app_flags & KBD_CONTROLLER_ACTIVE) {
                // Stop KBD Controller
                app_env.app_flags &= ~KBD_CONTROLLER_ACTIVE;
                SetWord16(KBRD_IRQ_IN_SEL2_REG, 0);
                SetWord16(KBRD_IRQ_IN_SEL1_REG, 0);
                SetWord16(KBRD_IRQ_IN_SEL0_REG, 0x4000);
                NVIC_DisableIRQ(KEYBRD_IRQn);
            } 
            GLOBAL_INT_RESTORE();
            
            // Start SysTick
            systick_start((int)((int)(KBD_US_FAST_SCAN / (KBD_NR_OUTPUTS + SPLIT_PROCESS_KBD_SCANDATA + FSM_STEPS)) * SYSTICK_TICKS_PER_US), 2); // normal scan rate
            // Re-init scan period time
#ifdef ALTERNATIVE_SCAN_TIMES
            if (full_scan)
                scan_cycle_time = (KBD_US_FAST_SCAN * SYSTICK_TICKS_PER_US) - (int)((int)(KBD_US_FAST_SCAN / (KBD_NR_OUTPUTS+SPLIT_PROCESS_KBD_SCANDATA+FSM_STEPS)) * SYSTICK_TICKS_PER_US );
            else
                scan_cycle_time = (KBD_US_PART_SCAN * SYSTICK_TICKS_PER_US) - (int)((int)(KBD_US_FAST_SCAN / (KBD_NR_OUTPUTS+SPLIT_PROCESS_KBD_SCANDATA+FSM_STEPS)) * SYSTICK_TICKS_PER_US );                
#else    
            scan_cycle_time = (KBD_US_PER_SCAN * SYSTICK_TICKS_PER_US) - (int)((int)(KBD_US_FAST_SCAN / (KBD_NR_OUTPUTS+SPLIT_PROCESS_KBD_SCANDATA+FSM_STEPS)) * SYSTICK_TICKS_PER_US );
#endif    
            
            // Turn all outputs to 'high-Z' so as to be ready for the next scan and to preserve some power in the meantime
            for (int i = 0; i < KBD_NR_OUTPUTS; ++i)
                set_row_to_input_highz(i);
                    
            // If an INT from KBD Controller hit then the kbd_new_scandata are not valid. A re-scan is necessary (after debouncing).
            // FIXME: This may not be correct. Scan data were valid when collected. They sumply do not reflect the current status 
            //        of the keyboard because of the least keypress. But processing could be done.
            if(!kbd_kbint_event)
                kbd_process_scandata();

            // Debouncing
            kbd_bounce_active = 0;
            for (int i = 0; i < DEBOUNCE_BUFFER_SIZE ; ++i)
            {
                if (kbd_bounce_counters[i]) 
                {
    //					arch_printf("bounce %d\r\n", kbd_bounce_counters[i]);
                    
                    --kbd_bounce_counters[i];
                    
                    if (kbd_bounce_counters[i])
                        kbd_bounce_active = 1;					
                    else
                        kbd_bounce_intersections[i] = 0xFFFF;
                }
            }
            
            if (kbd_bounce_active)
                kbd_scanstate = 0x0000; //debouncing needed. scan again.
            else {
                //debouncing is finished - scanning is done!
                
                if (kbd_buffer_has_data()) {
                    if (kbd_reports_en) { // normal mode
                        if (prepare_kbd_keyreport()) {// this MUST be called or the call to wait_for_key_release() later fails!
                            ;
                        }
                    } else {
                        // bonding mode
                        while (kbd_keycode_buffer_head != kbd_keycode_buffer_tail) 
                        {
                            uint16_t keycode = kbd_keycode_buffer[kbd_keycode_buffer_head];
                            const char keymode = keycode >> 8;
                            const char keychar = keycode & 0xFF;
                            int pressed = (~keymode) & 1;
                            
                            if (!pressed) {
                                if ( (keymode & 0xFC) == 0x00 ) { // normal key
                                    if ((keychar == 0x28) || (keychar == 0x58)) { // Enter or Keypad Enter
                                        app_mitm_passcode_report(passcode);
                                    } else {
                                        int num = -1;
                                        
                                        if (keychar >= 0x1E && keychar <= 0x27)
                                            num = keychar - 0x1D;
                                        else if (keychar >= 0x59 && keychar <= 0x62)
                                            num = keychar - 0x58;
                                        if (num == 10)
                                            num = 0;
                                        
                                        if (num != -1) {
                                            passcode *= 10;
                                            passcode += num;
                                        }
                                    }
                                }
                            }
                            
                            kbd_keycode_buffer_head = (kbd_keycode_buffer_head + 1) % KEYCODE_BUFFER_SIZE;
                        }
                        
                    }
                }
                
                // FIXME: We will loop between 0000 - 0100 - 0200 if connection is lost. Maybe use a timer to wait for some time and then reset...
                
                if (wait_for_key_release()) // We have to keep scanning for key release... //FIXME: Bug! We could have lost connection in between...
                    kbd_scanstate = 0x0000;
                else
#if (SCAN_ALWAYS_ACTIVE)
                    kbd_scanstate = 0x0000;
#else                
                    kbd_scanstate = 0x0400; 
#endif                
            }
            
            if (kbd_kbint_event) // A key has been pressed - Force Rescan!
                kbd_scanstate = 0x0000;
            
            break;
            
        case 0x0400:
            app_env.app_flags &= ~KBD_SCANNING;
            systick_stop();
            if (user_disconnection_req) {
                ; // disconnect sychronously!
            } else { // restart only if connected!
                app_kbd_enable_wakeup_irq();
                kbd_membrane_output_sleep();
            }
            kbd_scanstate = 0x0000;
        
            break;
    }
}


/*
 * Name         : SysTick_Handler - ISR of the SysTick IRQ 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : Forces execution of the state machine once every SysTick interval.
 *
 * Returns      : void
 *
 */
void SysTick_Handler(void) 
{ 
//	DEBUG_BP(kbd_membrane_status == 0);
    if (kbd_membrane_status == 0) {
        systick_stop();
        return;
    }

    kbd_do_scan_param = 1; // has to be lowest priority
}

/*
 * Name         : KEYBRD_Handler - ISR of the Keyboard Controller IRQ 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : Clears the interrupt. Starts a full key scan.
 *
 * Returns      : void
 *
 */
void KEYBRD_Handler(void)
{
    kbd_kbint_event = true;

    // Clear it first
    SetWord16(KBRD_IRQ_IN_SEL0_REG, 0); 
    SetWord16(KBRD_IRQ_IN_SEL1_REG, 0);
    SetWord16(GPIO_RESET_IRQ_REG, 0x20); 

    // No more KEYBRD interrupts from now on
    NVIC_DisableIRQ(KEYBRD_IRQn); 

    // We do not know which row has the key. Rescan all rows!
    full_scan = true;
        
    app_env.app_flags &= ~KBD_CONTROLLER_ACTIVE;
    kbd_do_scan_param = 1; // force execution of State Machine
}


/**
 * Keyboard (HID) Application Functions
 ****************************************************************************************
 */


/*
 * Name         : app_keyboard_init - Creates the HID over GATT DB 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : Creates the HID over GATT database.
 *
 * Returns      : void
 *
 */
void app_keyboard_init(void)
{
    kbd_do_scan_param = 0;          // Disable the execution of the state machine
    systick_stop();                 // Make sure SysTick is stopped

    kbd_init_keyreport();           // Initialize key report buffers and vars and the fn modifier var

    kbd_init_retained_scan_vars();  // Initialize retained variables
    kbd_init_scan_vars();           // Initialize non-retained variables

    kbd_reports_en = false;         // reporting is OFF
    user_extended_sleep = false;    // default mode is ON

    // Uncomment if scanning needs to be active from startup
// 	kbd_membrane_input_setup();     // Setup the "inputs" of the key-matrix
// 	app_kbd_enable_wakeup_irq();    // Setup the wakeup irq
// 	kbd_membrane_output_sleep();    // Setup the "outputs" of the key-matrix
}

uint8_t (* atts_find_uuid)(uint16_t *start_hdl, uint16_t end_hdl, uint8_t uuid_len, uint8_t *uuid) = (uint8_t (*)(uint16_t *, uint16_t, uint8_t, uint8_t *))0x28829;
uint16_t (* atts_find_end)(uint16_t start_hdl) = (uint16_t (*)(uint16_t))0x288c1;
extern uint8_t atts_get_att_chk_perm(uint8_t conidx, uint8_t access, uint16_t handle, struct attm_elmt** attm_elmt);

/*
 * Name         : app_hid_create_db - Creates the HID over GATT DB 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : Creates the HID over GATT database.
 *
 * Returns      : void
 *
 */
void app_hid_create_db(void)
{
    // Add HOGPD in the database
    // see rw-ble-prf-hogpd-is section 2.2.1.1
    struct hogpd_create_db_req * req = KE_MSG_ALLOC(HOGPD_CREATE_DB_REQ, TASK_HOGPD,
                                                    TASK_APP, hogpd_create_db_req);
    struct hogpd_hids_cfg *cfg = &req->cfg[0];              // table 5.1
    struct hogpd_features *features = &cfg->features;       // table 5.2 features supported in the hid service
    struct hids_hid_info *hid_info = &cfg->hid_info;        // table 5.5 value of the hid information characteristic
    struct att_incl_desc *ext_rep_ref = &cfg->ext_rep_ref;  // table 5.7 // external report reference = included service value

    req->hids_nb = 1;

    features->svc_features =   HOGPD_CFG_KEYBOARD
#ifdef USE_BATT_EXTERNAL_REPORT
                             | HOGPD_CFG_MAP_EXT_REF
#endif
#ifdef HOGPD_BOOT_PROTO    
                             | HOGPD_CFG_BOOT_KB_WR
#endif    
                             | HOGPD_CFG_PROTO_MODE; 
    features->report_nb          = 3; 
    features->report_char_cfg[0] = HOGPD_CFG_REPORT_IN | HOGPD_REPORT_NTF_CFG_MASK | HOGPD_CFG_REPORT_WR;
    features->report_char_cfg[1] = HOGPD_CFG_REPORT_OUT;
    features->report_char_cfg[2] = HOGPD_CFG_REPORT_IN | HOGPD_REPORT_NTF_CFG_MASK | HOGPD_CFG_REPORT_WR;
    features->report_char_cfg[3] = 0;
    features->report_char_cfg[4] = 0;

    hid_info->bcdHID = 0x100;
    hid_info->bCountryCode = 0;
    hid_info->flags = HIDS_REMOTE_WAKE_CAPABLE;// | HIDS_NORM_CONNECTABLE;

#ifdef USE_BATT_EXTERNAL_REPORT
    {
        uint16_t start_hdl = 0;
        uint16_t end_hdl = 0xffff;
        uint16_t uuid = ATT_DECL_PRIMARY_SERVICE;
        uint16_t val = 0x180F;
        uint8_t ret;
        struct attm_elmt * attm_elmt = NULL;
        
        do {
            ret = atts_find_uuid(&start_hdl, end_hdl, 2, (uint8_t *)&uuid);
        
            if (ret == ATT_ERR_NO_ERROR) {
                /* retrieve attribute +  check permission */
                ret = atts_get_att_chk_perm(app_env.conidx, 0x00, start_hdl, &attm_elmt);
                
                /* look for the Battery Service (0x180F) */
                if (ret == ATT_ERR_NO_ERROR && ((uint16_t *)attm_elmt->value)[0] == val) {
                    ext_rep_ref->start_hdl = start_hdl;
                    ext_rep_ref->end_hdl = atts_find_end(start_hdl);
                    ext_rep_ref->uuid = 0x180F;
                    break;
                } else
                    ret = ATT_ERR_ATTRIBUTE_NOT_FOUND;
            }
            start_hdl++;
        } while (start_hdl != 0 && ret != ATT_ERR_NO_ERROR);
    }    
    cfg->ext_rep_ref_uuid = ATT_CHAR_BATTERY_LEVEL; // external report reference value
#else
//    ext_rep_ref->start_hdl = 0;
//    ext_rep_ref->end_hdl = 0;
//    ext_rep_ref->uuid = 0;
//    cfg->ext_rep_ref_uuid = 0;
#endif

    // Send the message
    arch_puts("Send HOGPD_CREATE_DB_REQ\r\n");
    ke_msg_send(req);
}


/*
 * Name         : app_keyboard_enable - Enables the HOGPD profile 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : Enables the HID profile
 *
 * Returns      : void
 *
 */
void app_keyboard_enable(void)
{
    arch_puts("app_keyboard_enable()\r\n");

    // Allocate the message
    struct hogpd_enable_req * req = KE_MSG_ALLOC(HOGPD_ENABLE_REQ, TASK_HOGPD, TASK_APP,
                                                 hogpd_enable_req);

    // Fill in the parameter structure
    req->conhdl = app_env.conhdl;
    req->sec_lvl = PERM(SVC, ENABLE); 
    req->con_type = PRF_CON_NORMAL; // The Server always does PRF_CON_NORMAL
    req->ntf_cfg[0].boot_kb_in_report_ntf_en = 0;
    req->ntf_cfg[0].boot_mouse_in_report_ntf_en = 0;
    req->ntf_cfg[0].report_ntf_en[0] = 1;
    req->ntf_cfg[0].report_ntf_en[2] = 1;

    // Send the message
    ke_msg_send(req);
}


// The stack sends NTF to inform the app about the result status of the requested procedure
// (i.e. send Report Notification). The function below does not check this status at all. 
// It is not simple to implement this checking when sending multiple requests to the stack
// (i.e. Report NTF for key press + Report Release NTF) since at this point you would like to  
// be able to check for which of these requests you got the reported result (i.e. error) and 
// in this case, probably try to retransmit it (considering that it has been buffered 
// somewhere...).
// In any case, the assumption that this NTF can be used as a trigger to send the next
// pending Report Notification is completely wrong. This functionality has now been commented
// out from the code.
void app_hid_ntf_cfm(uint8_t status)
{
    //arch_puts("app_hid_ntf_cfm()\r\n");
    DEBUG_BP(status != PRF_ERR_OK);
}


/*
 * Name         : app_kbd_start_scanning - start scanning 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : Activates key scanning hardware and state machine.
 *
 * Returns      : void
 *
 */
void app_kbd_start_scanning(void)
{
	arch_puts("app_kbd_start_scanning()\r\n");
	
    kbd_init_scan_vars();       // initalize non-retained scan variables
	kbd_start_sw_scanning();    // start SW scanning
}


/*
 * Name         : app_kbd_enable_scanning - enable scanning hw
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : Sets up the hardware for key scanning.
 *
 * Note(s)      : Normally, the keyboard scanning is active only when
 *                connected or disconnected but bonded.
 *
 * Returns      : void
 *
 */
void app_kbd_enable_scanning(void)
{
    arch_puts("app_kbd_enable_scanning()\r\n");

    passcode = 0;
    kbd_reports_en = false;

    kbd_membrane_input_setup();     // Setup the "inputs" of the key-matrix
    app_kbd_enable_wakeup_irq();    // Setup the wakeup irq
    kbd_membrane_output_sleep();    // Setup the "outputs" of the key-matrix
}


/*
 * Name         : app_kbd_start_reporting - Activate reporting
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : Activates reporting mode
 *
 * Returns      : void
 *
 */
void app_kbd_start_reporting(void)
{   
    // may be called multiple times
	arch_puts("app_kbd_start_reporting()\r\n");
    
    kbd_reports_en = true;
}


/*
 * Name         : app_kbd_stop - Keyboard (HID) is stopped
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : Activates passcode mode (default).
 *
 * Returns      : void
 *
 */
void app_kbd_stop(void)
{
    arch_puts("app_kbd_stop()\r\n");

    kbd_reports_en = false;
    
    memset(normal_key_report_st, 0, 8);
    memset(extended_key_report_st, 0, 3);
}


/*
 * Name         : app_kbd_send_key_report - Sends a Key Report to the Host
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : Sends a Key Report Notification to the Host.
 *
 * Returns      : 0 - failed
 *                1 - succeeded
 *
 */
int app_kbd_send_key_report(void)
{
    return send_kbd_keyreport();
}


/*
 * Name         : check_connection_status - Checks connection status
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : Checks if we are connected or not
 *
 * Returns      : false - disconnected
 *                true  - connected
 *
 */
int check_connection_status(void)
{
    bool connected = true;
    ke_state_t app_state = ke_state_get(TASK_APP);
    
	if (app_state != APP_SECURITY && app_state != APP_PARAM_UPD && app_state != APP_CONNECTED) 
        connected = false;
    
	return (connected); 
}


// Called after each 'idle' wake up
void app_kbd_reinit_matrix(void)
{
#if (BLE_ALT_PAIR)          
# if (BLE_ALT_I2C_ON) 
#  if (MATRIX_SETUP == 3)
    // fix LED leakage
    GPIO_ConfigurePin(GPIO_PORT_2, GPIO_PIN_8, OUTPUT, PID_GPIO, true);
    GPIO_ConfigurePin(GPIO_PORT_2, GPIO_PIN_5, OUTPUT, PID_GPIO, true);
#  endif // (MATRIX_SETUP)    
    GPIO_SetPinFunction(I2C_SCL_PORT, I2C_SCL_PIN, INPUT, PID_I2C_SCL);
    GPIO_SetPinFunction(I2C_SDA_PORT, I2C_SDA_PIN, INPUT, PID_I2C_SDA);
# endif // BLE_ALT_I2C_ON    
#endif  // BLE_ALT_PAIR    
    
    kbd_membrane_input_setup();             // setup inputs (pull-up)
    kbd_membrane_output_sleep();            // setup outputs 'low'. they are latched but have to be reinitialized
}
#endif //BLE_HID_DEVICE

/// @} APP
