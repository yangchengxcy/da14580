/**
 ****************************************************************************************
 *
 * @file periph_setup.h
 *
 * @brief Peripherals setup header file. 
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
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
 
#include "global_io.h"
#include "arch.h"


/*
 * DEFINES
 ****************************************************************************************
 */

/* Enable WKUPCT. Required by wkupct_quadec driver. */
#define WKUP_ENABLED

#define GPIO_ALERT_LED_PORT     GPIO_PORT_1
#define GPIO_ALERT_LED_PIN      GPIO_PIN_1
#define GPIO_ALERT_LED1_PIN      GPIO_PIN_2

#define GPIO_BUTTON_PORT        GPIO_PORT_0 
#define GPIO_BUTTON_PIN         GPIO_PIN_6

#define GPIO_BAT_LED_PORT       GPIO_PORT_1 
#define GPIO_BAT_LED_PIN        GPIO_PIN_0

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
 
void periph_init(void);

void GPIO_reservations(void);

