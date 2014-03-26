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

#if BLE_HID_DEVICE 
#include "app_keyboard.h"
#endif


/// Application Environment Structure
struct arch_sleep_env_tag sleep_env __attribute__((section("exchange_mem_case1"))); //@WIKRETENTION MEMORY 

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
	sleep_env.slp_state = ARCH_SLEEP_OFF;
#if (DEEP_SLEEP)
	rwip_env.sleep_enable = false;
#endif
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
    sleep_env.slp_state = ARCH_EXT_SLEEP_ON;

#if (DEEP_SLEEP)
	rwip_env.sleep_enable = true;
#endif
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
	sleep_env.slp_state = ARCH_DEEP_SLEEP_ON;
    
#if (DEEP_SLEEP)
	rwip_env.sleep_enable = true;
#endif
}

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
