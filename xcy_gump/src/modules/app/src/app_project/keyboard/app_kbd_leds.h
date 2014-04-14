/**
 ****************************************************************************************
 *
 * @file app_kbd_leds.h
 *
 * @brief HID Keyboard LEDs header file.
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

#ifndef APP_CUSTOM_KBD_LEDS_H_
#define APP_CUSTOM_KBD_LEDS_H_

enum HID_LED_ST {
    LED_OFF = 0,
    BLINK_LED_IS_ON__TURN_OFF,
    BLINK_LED_IS_OFF__TURN_ON,
    LED_ON,
};

extern enum HID_LED_ST green_led_st;
extern enum HID_LED_ST red_led_st;

#define BLINK_GREEN_ON      10      // 100ms
#define BLINK_GREEN_OFF     40      // 400ms
#define GREEN_ON            150     // 1.5s

#define RED_ON              150     // 1.5s


void leds_init(void);
void leds_set_disconnected(void);
void leds_set_connection_in_progress(void);
void leds_set_connection_established(void);

#include "ke_task.h"        // kernel task
#include "ke_msg.h"         // kernel message

int app_green_led_timer_handler(ke_msg_id_t const msgid,
                                   void const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id);

int app_red_led_timer_handler(ke_msg_id_t const msgid,
                                   void const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id);

#endif // APP_CUSTOM_KBD_LEDS_H_
