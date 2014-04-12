/**
 ****************************************************************************************
 *
 * @file arch_sleep.h
 *
 * @brief Sleep control function API.
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

#if !defined(_ARCH_SLEEP_H_)
#define _ARCH_SLEEP_H_

#include "arch.h"
#include "app.h"
#include "stdbool.h"

void app_disable_sleep(void);
void app_set_extended_sleep(void);
void app_set_deep_sleep(void);
uint8_t app_get_sleep_mode(void);

#endif // _ARCH_SLEEP_H_
