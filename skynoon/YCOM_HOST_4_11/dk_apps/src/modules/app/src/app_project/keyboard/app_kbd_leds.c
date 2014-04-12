/**
 ****************************************************************************************
 *
 * @file app_kbd_leds.c
 *
 * @brief HID Keyboard LED functionality.
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
#include "rwip_config.h"
#include "app.h"
#include "app_task.h"
#include "app_console.h"
#include "arch_sleep.h"
#include "gpio.h"

#include "app_kbd.h"
#include "app_kbd_proj.h"
#include "app_kbd_key_matrix.h"
#include "app_kbd_leds.h"


enum HID_LED_ST green_led_st;
enum HID_LED_ST red_led_st;

static void green_led_off(void)
{
    if (HAS_KEYBOARD_LEDS)
    {
        GPIO_SetActive(KBD_GREEN_LED_PORT, KBD_GREEN_LED_PIN);          // green: high - off
        green_led_st = LED_OFF;
    }
}

static void green_led_on(void)
{
    if (HAS_KEYBOARD_LEDS)
    {
        GPIO_SetInactive(KBD_GREEN_LED_PORT, KBD_GREEN_LED_PIN);        // green: low - on
        green_led_st = LED_ON;
    }
}

static void green_led_blink(void)
{
    if (HAS_KEYBOARD_LEDS)
    {
        GPIO_SetInactive(KBD_GREEN_LED_PORT, KBD_GREEN_LED_PIN);        // green: low - on
        green_led_st = BLINK_LED_IS_ON__TURN_OFF;
    }
}

static void red_led_off(void)
{
    if (HAS_KEYBOARD_LEDS)
    {
        GPIO_SetActive(KBD_RED_LED_PORT, KBD_RED_LED_PIN);              // red: high - off
        red_led_st = LED_OFF;
    }
}

static void red_led_on(void)
{
    if (HAS_KEYBOARD_LEDS)
    {
        GPIO_SetInactive(KBD_RED_LED_PORT, KBD_RED_LED_PIN);            // red: low - on
        red_led_st = LED_ON;
    }
}


/*
 * Name         : leds_init - Initialize LEDs
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : Initialize the LEDs.
 *
 * Returns      : void
 *
 */
void leds_init(void)
{
    if (HAS_KEYBOARD_LEDS)
    {
        if (!(GetWord16(SYS_STAT_REG) & DBG_IS_UP)) {
            GPIO_ConfigurePin(KBD_GREEN_LED_PORT, KBD_GREEN_LED_PIN, OUTPUT, PID_GPIO, true);   // green: high - off
            green_led_st = LED_OFF;	
            GPIO_ConfigurePin(KBD_RED_LED_PORT, KBD_RED_LED_PIN, OUTPUT, PID_GPIO, true);       // red: high - off
            red_led_st = LED_OFF;	
        }
    }
}


/*
 * Name         : leds_set_disconnected - Set LEDs up for disconnection indication
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : Configures the LEDs to indicate disconnection status.
 *
 * Returns      : void
 *
 */
void leds_set_disconnected(void)
{
    if (HAS_KEYBOARD_LEDS)
    {
        green_led_off();
        red_led_on();
        app_timer_set(APP_RED_LED_TIMER, TASK_APP, RED_ON);
        if (app_get_sleep_mode())
            app_force_active_mode();                                    // prevent sleep only if enabled
    }
}


/*
 * Name         : leds_set_connection_in_progress - Set LEDs up for connection in progress indication
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : Configures the LEDs to indicate connection in progress status.
 *
 * Returns      : void
 *
 */
void leds_set_connection_in_progress(void)
{
    if (!(GetWord16(SYS_STAT_REG) & DBG_IS_UP)) {
        red_led_off();
        green_led_blink();
        app_timer_set(APP_GREEN_LED_TIMER, TASK_APP, BLINK_GREEN_ON);
        if (app_get_sleep_mode())
            app_force_active_mode();                                    // prevent sleep only if enabled
    }
}


/*
 * Name         : leds_set_connection_established - Set LEDs up for connection established indication
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : Configures the LEDs to indicate connection established status.
 *
 * Returns      : void
 *
 */
void leds_set_connection_established(void)
{
    if (!(GetWord16(SYS_STAT_REG) & DBG_IS_UP)) {
        red_led_off();
        green_led_on();
        app_timer_set(APP_GREEN_LED_TIMER, TASK_APP, GREEN_ON);
        if (app_get_sleep_mode())
            app_force_active_mode();                                    // prevent sleep only if enabled
    }
}


/*
 * Name         : app_green_led_timer_handler - Handler of the Green LED Timer 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : <various>
 *
 * Description  : Handles the green led.
 *
 * Returns      : KE_MSG_CONSUMED
 *
 */
int app_green_led_timer_handler(ke_msg_id_t const msgid,
                           void const *param,
                           ke_task_id_t const dest_id,
                           ke_task_id_t const src_id)
{
    if (HAS_KEYBOARD_LEDS)
    {
        do {
            if ((GetWord16(SYS_STAT_REG) & DBG_IS_UP) == DBG_IS_UP)
                break; // GPIOs are being used by the debugger
                
            switch(green_led_st)
            {
                case LED_OFF:
                    green_led_off();
                    break;
                
                case BLINK_LED_IS_ON__TURN_OFF:
                    GPIO_SetActive(KBD_GREEN_LED_PORT, KBD_GREEN_LED_PIN);              // high - off
                    app_timer_set(APP_GREEN_LED_TIMER, TASK_APP, BLINK_GREEN_OFF);
                    green_led_st = BLINK_LED_IS_OFF__TURN_ON;
                    if ((red_led_st == LED_OFF) || (red_led_st == BLINK_LED_IS_OFF__TURN_ON))
                        app_restore_sleep_mode();                                       // restore sleep
                    break;
                    
                case BLINK_LED_IS_OFF__TURN_ON:
                    green_led_blink();
                    app_timer_set(APP_GREEN_LED_TIMER, TASK_APP, BLINK_GREEN_ON);
                    if (app_get_sleep_mode())
                        app_force_active_mode();                                        // prevent sleep only if enabled
                    break;
                
                case LED_ON:
                    green_led_off();
                    if ((red_led_st == LED_OFF) || (red_led_st == BLINK_LED_IS_OFF__TURN_ON))
                        app_restore_sleep_mode();                                       // restore sleep
                    break;
                
                default:
                    break;
            }
        } while(0);
    }
	
	return (KE_MSG_CONSUMED);
}


/*
 * Name         : app_red_led_timer_handler - Handler of the Red LED Timer 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : <various>
 *
 * Description  : Handles the red led.
 *
 * Returns      : KE_MSG_CONSUMED
 *
 */
int app_red_led_timer_handler(ke_msg_id_t const msgid,
                           void const *param,
                           ke_task_id_t const dest_id,
                           ke_task_id_t const src_id)
{
    if (HAS_KEYBOARD_LEDS)
    {
        do {
            if ((GetWord16(SYS_STAT_REG) & DBG_IS_UP) == DBG_IS_UP)
                break;  // GPIOs are being used by the debugger
                
            switch(red_led_st)
            {
                case LED_OFF:
                    red_led_off();
                    break;
                
                case BLINK_LED_IS_ON__TURN_OFF:
                case BLINK_LED_IS_OFF__TURN_ON:
                    break;
                
                case LED_ON:
                    red_led_off();
                    if ((green_led_st == LED_OFF) || (green_led_st == BLINK_LED_IS_OFF__TURN_ON))
                        app_restore_sleep_mode();                                       // restore sleep
                    break;
                
                default:
                    break;
            }
        } while(0);
    }
		
	return (KE_MSG_CONSUMED);
}


