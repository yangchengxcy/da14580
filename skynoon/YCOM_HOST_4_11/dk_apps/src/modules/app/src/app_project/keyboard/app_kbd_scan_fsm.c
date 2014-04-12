/**
 ****************************************************************************************
 *
 * @file app_kbd_scan_fsm.c
 *
 * @brief Keyboard scanning FSM implementation.
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

#include "app_kbd.h"
#include "app_kbd_fsm.h"
#include "app_kbd_scan_fsm.h"
#include "app_multi_bond.h"
#include "app_console.h"
#include "app_kbd_debug.h"

enum key_scan_states current_scan_state __attribute__((section("retention_mem_area0"), zero_init));
static int scanning_substate            __attribute__((section("retention_mem_area0"), zero_init));

//bool main_fsm_changed = true;   /* use if the key scanning needs to be disabled */

/*
 * Description  : Key scan FSM.
 *
 * Returns      : void
 *
 */
void fsm_scan_update(void)
{
    switch(current_scan_state)
    {
    case KEY_SCAN_INACTIVE:
//        if (main_fsm_changed)
        if (true)
        {
//            main_fsm_changed = false;
            app_kbd_enable_scanning();
            current_scan_state = KEY_SCAN_IDLE;             // Transition from KEY_SCAN_INACTIVE -> KEY_SCAN_IDLE
        }
        
        if (DEVELOPMENT__NO_OTP)
        {
            if (systick_hit || wkup_hit)
                __asm("BKPT #0\n");
        }
        break;
    case KEY_SCAN_IDLE:
//        if (main_fsm_changed)
        if (false)
        {
//            main_fsm_changed = false;
            app_kbd_disable_scanning();
            current_scan_state = KEY_SCAN_INACTIVE;         // Transition from KEY_SCAN_IDLE -> KEY_SCAN_INACTIVE
        }
        else if (wkup_hit)
        {
            scanning_substate = 0;
            app_kbd_start_scanning();
            current_scan_state = KEY_SCANNING;              // Transition from KEY_SCAN_IDLE -> KEY_SCANNING
        }
        wkup_hit = false;                                   // just in case...
        
        if (DEVELOPMENT__NO_OTP)
        {
            if (systick_hit)
                __asm("BKPT #0\n");
        }
        break;
    case KEY_STATUS_UPD:
        if (DEVELOPMENT__NO_OTP)
        {
            if (wkup_hit)
                __asm("BKPT #0\n");
        }
        
        if (systick_hit)
        {
            if (app_kbd_update_status())
            {
                scanning_substate = 0;
                current_scan_state = KEY_SCANNING;          // Transition from KEY_STATUS_UPD -> KEY_SCANNING
                // scan once to save time!
                if (app_kbd_scan_matrix(&scanning_substate))
                    current_scan_state = KEY_STATUS_UPD;    // Transition from KEY_SCANNING -> KEY_STATUS_UPD
            }
            else
            {
                current_scan_state = KEY_SCAN_IDLE;         // Transition from KEY_STATUS_UPD -> KEY_SCAN_IDLE
            }
            systick_hit = false;
        }
        break;
    case KEY_SCANNING:
        if (DEVELOPMENT__NO_OTP)
        {
            if (wkup_hit)
                __asm("BKPT #0\n");
        }

        if (systick_hit)
        {
            if (app_kbd_scan_matrix(&scanning_substate))
                current_scan_state = KEY_STATUS_UPD;        // Transition from KEY_SCANNING -> KEY_STATUS_UPD
            // else the state remains unchanged and next time we will scan the next row
            systick_hit = false;
        }
        break;
    default:
        break;
    }
}
