/*
 * Copyright (C) 2013 Dialog Semiconductor GmbH and its Affiliates, unpublished work
 * This computer program includes Confidential, Proprietary Information and is a Trade Secret 
 * of Dialog Semiconductor GmbH and its Affiliates. All use, disclosure, and/or 
 * reproduction is prohibited unless authorized in writing. All Rights Reserved.
 */

#include "arch.h"
#include "app.h"
#include "gpio.h"
#include "ke_event.h"

/*
 ********************************* Hooks ************************************
 */

/**
 ****************************************************************************************
 * @brief Used for sending messages to kernel tasks generated from
 *         asynchronous events that have been processed in app_asynch_proc.
 *
 * @return true to force calling of schedule(), else false
 ****************************************************************************************
 */

static inline bool app_asynch_trm(void)
{
	bool ret = false;

	return ret;
}

/**
 ****************************************************************************************
 * @brief Used for processing of asynchronous events at “user” level. The
 *                   corresponding ISRs should be kept as short as possible and the
 *                   remaining processing should be done at this point.
 *
 * @return true to force calling of schedule(), else false
 ****************************************************************************************
 */

static inline bool app_asynch_proc(void)
{
	bool ret = false;
        
	return ret;
}

/**
 ****************************************************************************************
 * @brief Used for updating the state of the application just before sleep checking starts.
 *
 * @return void
 ****************************************************************************************
 */

static inline void app_asynch_sleep_proc(void)
{
    return; 
}

/**
 ****************************************************************************************
 * @brief Used to disallow extended or deep sleep based on the current application state. BLE and Radio are still powered off.
 *
 * @param[in] sleep_mode     Sleep Mode
 *
 * @return void
 ****************************************************************************************
 */

static inline void app_sleep_prepare_proc(sleep_mode_t *sleep_mode)
{
    
    
}

/**
 ****************************************************************************************
 * @brief Used for application specific tasks just before entering the low power mode.
 *
 * @param[in] sleep_mode     Sleep Mode 
 *
 * @return void
 ****************************************************************************************
 */

static inline void app_sleep_entry_proc(sleep_mode_t *sleep_mode)
{
        
    return;      
}

/**
 ****************************************************************************************
 * @brief Used for application specific tasks immediately after exiting the low power mode.
 *
 * @param[in] sleep_mode     Sleep Mode 
 *
 * @return void
 ****************************************************************************************
 */

static inline void app_sleep_exit_proc(sleep_mode_t sleep_mode)
{

    return;
}
