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

// GPIO_PORT_1,GPIO_PIN_0

#define GPIO_ALERT_LED0_PORT     GPIO_PORT_1
#define GPIO_ALERT_LED0_PIN      GPIO_PIN_0
#define GPIO_ALERT_LED1_PORT     GPIO_PORT_1
#define GPIO_ALERT_LED1_PIN      GPIO_PIN_1
#define GPIO_ALERT_LED2_PORT     GPIO_PORT_1
#define GPIO_ALERT_LED2_PIN      GPIO_PIN_2
#define GPIO_ALERT_LED3_PORT     GPIO_PORT_1
#define GPIO_ALERT_LED3_PIN      GPIO_PIN_3

#define GPIO_BUTTON0_PORT        GPIO_PORT_2 
#define GPIO_BUTTON0_PIN         GPIO_PIN_3
#define GPIO_BUTTON1_PORT        GPIO_PORT_2 
#define GPIO_BUTTON1_PIN         GPIO_PIN_6
#define GPIO_BUTTON2_PORT        GPIO_PORT_2 
#define GPIO_BUTTON2_PIN         GPIO_PIN_9

#define GPIO_BAT_LED_PORT       GPIO_PORT_0
#define GPIO_BAT_LED_PIN        GPIO_PIN_7

#define XCY_BUZZER_GPIO	GPIO_BAT_LED_PORT,GPIO_BAT_LED_PIN
#define XCY_LED0_GPIO	GPIO_ALERT_LED0_PORT,GPIO_ALERT_LED0_PIN
#define XCY_LED1_GPIO	GPIO_ALERT_LED1_PORT,GPIO_ALERT_LED1_PIN
#define XCY_LED2_GPIO	GPIO_ALERT_LED2_PORT,GPIO_ALERT_LED2_PIN
#define XCY_LED3_GPIO	GPIO_ALERT_LED3_PORT,GPIO_ALERT_LED3_PIN
#define XCY_KB_ON		GPIO_BUTTON0_PORT,GPIO_BUTTON0_PIN
#define XCY_KB_JL		GPIO_BUTTON1_PORT,GPIO_BUTTON1_PIN
#define XCY_KB_SCH		GPIO_BUTTON2_PORT,GPIO_BUTTON2_PIN

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
 
void periph_init(void);

void GPIO_reservations(void);

