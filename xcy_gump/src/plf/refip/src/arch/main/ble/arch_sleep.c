/**
 ****************************************************************************************
 *
 * @file arch_sleep.c
 *
 * @brief Sleep control functions.
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

#include "arch.h"
#include "arch_sleep.h"
#include "app.h"
#include "rwip.h"

/// Application Environment Structure
extern struct arch_sleep_env_tag    sleep_env;    // __attribute__((section("retention_mem_area0")));
uint8_t sleep_md                    __attribute__((section("retention_mem_area0"), zero_init));
uint8_t sleep_pend                  __attribute__((section("retention_mem_area0"), zero_init));
uint8_t sleep_cnt                   __attribute__((section("retention_mem_area0"), zero_init));
bool sleep_ext_force                __attribute__((section("retention_mem_area0"), zero_init));

/*
 * Name         : app_disable_sleep - Disable all sleep modes 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : Disable sleep modes. System operates in active / idle modes only.
 *
 * Returns      : void
 *
 */
void app_disable_sleep(void)
{
    if (sleep_md == 0)
    {
        sleep_env.slp_state = ARCH_SLEEP_OFF;
#if (DEEP_SLEEP)
        rwip_env.sleep_enable = false;
#endif
    }
    else
        sleep_pend = 0x80 | 0x00;
}

/*
 * Name         : app_set_extended_sleep - Activates extended sleep mode
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : Activates the extended sleep mode. The system operates in idle / active / extended sleep modes.
 *
 * Returns      : void
 *
 */
void app_set_extended_sleep(void)
{
    if (sleep_md == 0)
    {
        sleep_env.slp_state = ARCH_EXT_SLEEP_ON;
#if (DEEP_SLEEP)
        rwip_env.sleep_enable = true;
#endif
    }
    else
        sleep_pend = 0x80 | 0x01;
}

/*
 * Name         : app_set_deep_sleep - Activates deep sleep mode 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : node
 *
 * Description  : Activates the deep sleep mode. The system operates in idle / active / deep sleep modes.
 *
 * Returns      : void
 *
 */
void app_set_deep_sleep(void)
{
    if (sleep_md == 0)
    {
        sleep_env.slp_state = ARCH_DEEP_SLEEP_ON;
#if (DEEP_SLEEP)
        rwip_env.sleep_enable = true;
#endif
    }
    else
        sleep_pend = 0x80 | 0x02;
}

#if 0
/*
 * Name         : app_get_sleep_mode - Get the current mode of operation
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : Returns the current mode of operation. 
 *
 * Returns      : 0 if sleep is disabled
 *                1 if extended sleep is enabled
 *                2 if deep sleep is enabled
 *
 */
uint8_t app_get_sleep_mode(void)
{
	uint8_t ret = 0;

	switch(sleep_env.slp_state)
	{
		case ARCH_SLEEP_OFF: 
            ret = 0; break;
		case ARCH_EXT_SLEEP_ON:
            ret = 1; break;
		case ARCH_DEEP_SLEEP_ON: 
            ret = 2; break;
	}

	return ret;
}
#endif
/*
 * Name         : app_restore_sleep_mode - Restore the sleep mode to what it was before disabling.
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : Restores the sleep mode. App should not modify the sleep mode directly.
 *
 * Returns      : none
 *
 */
void app_restore_sleep_mode(void)
{
    uint8_t cur_mode;
    
    if (sleep_cnt > 0)
        sleep_cnt--;
    
    if (sleep_cnt > 0)
        return;     // cannot restore it yet. someone else has requested active mode and we'll wait him to release it.
        
    if (sleep_pend != 0)
    {
        sleep_md = sleep_pend & 0x3;
        sleep_pend = 0;
    }
    else if (sleep_md)
        sleep_md--;
    
    cur_mode = sleep_md;
    sleep_md = 0;
    
    switch(cur_mode) 
    {
       case 0:  break;
       case 1:  app_set_extended_sleep(); break;
       case 2:  app_set_deep_sleep(); break;
       default: break;
    }
}

/*
 * Name         : app_force_active_mode - Disable sleep but keep the sleep mode status.
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : Disable sleep. Store the sleep mode used by the app.
 *
 * Returns      : none
 *
 */
void app_force_active_mode(void)
{
    uint8_t cur_mode;
    
    sleep_cnt++;
    
    if (sleep_md == 0)  // add this check for safety! If it's called again before restore happens then sleep_md won't get overwritten
    {
        cur_mode = app_get_sleep_mode();
        cur_mode++;     // 1: disabled, 2: extended, 3: deep sleep (!=0 ==> sleep is in forced state)
        app_disable_sleep();
        sleep_md = cur_mode;
    }
}

/*
 * Name         : app_ble_ext_wakeup_on.
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : BLE sleeps forever waiting a forced wakeup. After waking up from an external event, if the system has to wake BLE up
 *              : it needs to restore the default mode of operation by calling app_ble_ext_wakeup_off() or the BLE won't be able to
 *              : wake up in order to serve BLE events!
 *
 * Returns      : none
 *
 */
void app_ble_ext_wakeup_on(void)
{
    sleep_ext_force = true;
}

/*
 * Name         : app_ble_ext_wakeup_off.
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : Restore BLE cores' operation to default mode. The BLE core will wake up every 10sec even if no BLE events are 
 *              : scheduled. If an event is to be scheduled earlier, then BLE will wake up sooner to serve it.
 *
 * Returns      : none
 *
 */
void app_ble_ext_wakeup_off(void)
{
    sleep_ext_force = false;
}

/*
 * Name         : app_ble_ext_wakeup_get.
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : Returns the current mode of operation of the BLE core (external or default).
 *
 * Returns      : false, if default mode is selected
 *              : true, if BLE sleeps forever waiting for a forced wakeup
 *
 */
bool app_ble_ext_wakeup_get(void)
{
    return sleep_ext_force;
}

