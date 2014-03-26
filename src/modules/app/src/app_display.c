/**
 ****************************************************************************************
 *
 * @file app_display.c
 *
 * @brief Application Display entry point
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
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

#if (DISPLAY_SUPPORT)

#include "arch.h"
#include <stdio.h>
#include <string.h>                  // string manipulation and functions
#include "display.h"

#include "app_display.h"
#include "app.h"
#if (BLE_APP_HT)
#include "app_ht.h"                  // health thermometer application definitions
#endif //BLE_APP_HT
#if (BLE_APP_ACCEL)
#include "app_accel.h"               // accelerometer application definitions
#endif //BLE_APP_ACCEL

#if RTC_SUPPORT
#include "rtc.h"
#include "ke_event.h"
#endif //RTC_SUPPORT

/*
 * STRUCT DEFINITIONS
 ****************************************************************************************
 */

struct app_display_env_tag
{
    uint8_t screen_id_adv;             //Advertising
    uint8_t screen_id_con;             //Connection State
    uint8_t screen_id_pin;             //Pin Code

    #if RTC_SUPPORT
    uint8_t screen_id_rtc;             //RTC
    #endif //RTC_SUPPORT

    #if (BLE_APP_HT)
    //Level 3 - Health Thermometer
    uint8_t screen_id_ht_value;        //Temperature Value
    uint8_t screen_id_ht_type;         //Temperature Type
    uint8_t screen_id_ht_unit;         //Unit
    #endif //BLE_APP_HT

    /// advertising state (true = is advertising, false = no advertising)
    bool advertising;
    /// connected (true = connected, false = idle)
    bool connected;
};

/*
 * GLOBAL VARIABLE DECLARATION
 ****************************************************************************************
 */

/// Application display environment
struct app_display_env_tag app_display_env __attribute__((section("exchange_mem_case1"))); //@WIKRETENTION MEMORY

#if RTC_SUPPORT
static char* const Months[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
static char* const Days[] = {"Mon","Tue","Wen","Thu","Fri","Sat","Sun"};
#endif //RTC_SUPPORT

/*
 * LOCAL FUNCTION DECLARATIONS
 ****************************************************************************************
 */
static void app_display_hdl_adv(enum display_input input);
#if (BLE_APP_HT)
static void app_display_hdl_ht_temp_type(enum display_input input);
static void app_display_hdl_ht_temp_value(enum display_input input);
#endif //BLE_APP_HT

/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */
static void app_display_hdl_adv(enum display_input input)
{
    switch(input)
    {
        case DISPLAY_INPUT_SELECT:
        {
            display_screen_update(app_display_env.screen_id_adv, 0, "o ADVERTISING");
        }
        break;
        case DISPLAY_INPUT_DESELECT:
        {
            display_screen_update(app_display_env.screen_id_adv, 0, "< ADVERTISING");
        }
        break;
        case DISPLAY_INPUT_LEFT:
        case DISPLAY_INPUT_RIGHT:
        {
            if (!app_display_env.advertising)
            {
                app_adv_start();
                app_display_env.advertising = true;
                display_screen_update(app_display_env.screen_id_adv, 1, "s ON");
            }
            else
            {
                app_adv_stop();
                app_display_env.advertising = false;
                display_screen_update(app_display_env.screen_id_adv, 1, "s OFF");
            }
        }
        break;
        default:
        break;
    }
}

#if (BLE_APP_HT)
static void app_display_hdl_ht_temp_type(enum display_input input)
{
    switch(input)
    {
        case DISPLAY_INPUT_SELECT:
        {
            display_screen_update(app_display_env.screen_id_ht_type, 0, "o TEMP TYPE");
        }
        break;
        case DISPLAY_INPUT_DESELECT:
        {
            display_screen_update(app_display_env.screen_id_ht_type, 0, "< TEMP TYPE");
        }
        break;
        case DISPLAY_INPUT_LEFT:
        {
            app_htpt_temp_type_dec();
        }
        break;
        case DISPLAY_INPUT_RIGHT:
        {
            app_htpt_temp_type_inc();
        }
        break;
        default:
        break;
    }
}

static void app_display_hdl_ht_temp_value(enum display_input input)
{
    switch(input)
    {
        case DISPLAY_INPUT_SELECT:
        {
            display_screen_update(app_display_env.screen_id_ht_value, 0, "o TEMP VALUE");
        }
        break;
        case DISPLAY_INPUT_DESELECT:
        {
            display_screen_update(app_display_env.screen_id_ht_value, 0, "< TEMP VALUE");
        }
        break;
        case DISPLAY_INPUT_LEFT:
        {
            app_htpt_temp_dec();
        }
        break;
        case DISPLAY_INPUT_RIGHT:
        {
            app_htpt_temp_inc();
        }
        break;
        default:
        break;
    }
}
#endif //BLE_APP_HT

#if RTC_SUPPORT
/**
 ****************************************************************************************
 * @brief Handles the 1 second tick (event handler).
 *****************************************************************************************
 */
static void app_display_sec_tick_evt_hdl(void)
{
    char * ptr;
    char line0[DISPLAY_LINE_SIZE];
    char line1[DISPLAY_LINE_SIZE];
    struct rtc_time time;

    // Clear DISPLAY kernel event
    ke_event_clear(KE_EVENT_RTC_1S_TICK);

    // Read current time from RTC
    rtc_get_time(&time);

    // Format the date
    ptr = &line0[0];
    *ptr++ = '<';
    memcpy(ptr, Months[time.tm_mon-1], strlen(Months[time.tm_mon-1]));
    ptr += strlen(Months[time.tm_mon-1]);
    *ptr++ = ' ';
    if(time.tm_mday > 9)
    {
        *ptr++ = 48 + time.tm_mday / 10;
        time.tm_mday = time.tm_mday - (10 * (time.tm_mday / 10));
    }
    *ptr++ = 48 + time.tm_mday;
    *ptr++ = ' ';
    memcpy(ptr, Days[time.tm_wday-1], strlen(Days[time.tm_wday-1]));
    ptr += strlen(Days[time.tm_wday-1]);
    *ptr++ = ' ';
    *ptr++ = '2';
    *ptr++ = '0';
    time.tm_year -= 2000;
    *ptr++ = 48 + time.tm_year / 10;
    time.tm_year = time.tm_year - (10 * (time.tm_year / 10));
    *ptr++ = 48 + time.tm_year;
    *ptr++ = 0;
//    sprintf(line0, "<%s %d %s %d", Months[time.tm_mon-1], time.tm_mday, Days[time.tm_wday-1], time.tm_year);

    // Format the time
    ptr = &line1[0];
    *ptr++ = 48 + time.tm_hour / 10;
    time.tm_hour = time.tm_hour - (10 * (time.tm_hour / 10));
    *ptr++ = 48 + time.tm_hour;
    *ptr++ = ':';
    *ptr++ = 48 + time.tm_min / 10;
    time.tm_min = time.tm_min - (10 * (time.tm_min / 10));
    *ptr++ = 48 + time.tm_min;
    *ptr++ = ':';
    *ptr++ = 48 + time.tm_sec / 10;
    time.tm_sec = time.tm_sec - (10 * (time.tm_sec / 10));
    *ptr++ = 48 + time.tm_sec;
    *ptr++ = 0;
//    sprintf(line1, "%02d:%02d:%02d", time.tm_hour, time.tm_min, time.tm_sec);


    // Update screen content
    display_screen_update(app_display_env.screen_id_rtc, 0, line0);
    display_screen_update(app_display_env.screen_id_rtc, 1, line1);

    // Program a screen refreshment
    display_refresh();
//    display_goto_screen(app_display_env.screen_id_rtc);
}

/**
 ****************************************************************************************
 * @brief Handles the 1 second tick (call back).
 *****************************************************************************************
 */
static void app_display_sec_tick_cbck(void)
{
    ke_event_set(KE_EVENT_RTC_1S_TICK);
}
#endif // RTC_SUPPORT

/*
 * EXPORTED FUNCTION DEFINITIONS
 ****************************************************************************************
 */
void app_display_init(void)
{
/* ***********************************************************************/
/*                              Screen Allocation                        */
/* ***********************************************************************/

    //Level 1
    uint8_t s_prof = display_screen_alloc();            //Profiles
    uint8_t s_app_mgmt = display_screen_alloc();        //Application Management
    uint8_t s_info = display_screen_alloc();            //Platform Information


    //Level 2 - Features
    uint8_t s_feat_top;
    #if (BLE_APP_HT)
    uint8_t s_ht = display_screen_alloc();              //Health Thermometer
    #endif //BLE_APP_HT
    #if (BLE_APP_ACCEL)
    uint8_t s_accel = display_screen_alloc();           //Accelerometer
    #endif //BLE_APP_ACCEL

    //Level 2 - Application Management
    uint8_t s_adv = display_screen_alloc();             //Advertising
    uint8_t s_con = display_screen_alloc();             //Connection State
    uint8_t s_pin = display_screen_alloc();             //PIN Code
    #if RTC_SUPPORT
    uint8_t s_rtc = display_screen_alloc();             //RTC
    struct rtc_time alarm = {0,0,0,0,0,0,0,0};
    #endif //RTC_SUPPORT

    #if (BLE_APP_HT)
    //Level 3 - Health Thermometer
    uint8_t s_ht_value = display_screen_alloc();        //Temperature Value
    uint8_t s_ht_type = display_screen_alloc();         //Temperature Type
    uint8_t s_ht_unit = display_screen_alloc();         //Unit
    #endif //BLE_APP_HT

    app_display_env.screen_id_adv = s_adv;
    app_display_env.screen_id_con = s_con;
    app_display_env.screen_id_pin = s_pin;
    #if RTC_SUPPORT
    app_display_env.screen_id_rtc = s_rtc;
    #endif //RTC_SUPPORT

    #if (BLE_APP_HT)
    app_display_env.screen_id_ht_value = s_ht_value;
    app_display_env.screen_id_ht_type = s_ht_type;
    app_display_env.screen_id_ht_unit = s_ht_unit;
    #endif //BLE_APP_HT

/* ***********************************************************************/
/*                              Screen Insertion                         */
/* ***********************************************************************/

    //Level 1
    display_screen_insert(s_app_mgmt, s_prof);
    display_screen_insert(s_info, s_prof);

    //Level 2 - Features
    #if (BLE_APP_HT)
    s_feat_top = s_ht;
    #elif BLE_APP_ACCEL
    s_feat_top = s_accel;
    #else
    //No profile
    s_feat_top = display_screen_alloc();
    display_screen_set(s_feat_top, NULL, "< NO PROFILE", "");
    #endif

    #if (BLE_APP_HT)
    display_screen_insert(s_ht, s_feat_top);
    #endif //BLE_APP_HT
    #if (BLE_APP_ACCEL)
    display_screen_insert(s_accel, s_feat_top);
    #endif //BLE_APP_ACCEL

    //Level 2 - Application Management
    display_screen_insert(s_con, s_adv);
    display_screen_insert(s_pin, s_adv);
    #if RTC_SUPPORT
    display_screen_insert(s_rtc, s_adv);
    #endif //RTC_SUPPORT

    //Level 3 - Health Thermometer
    #if (BLE_APP_HT)
    display_screen_insert(s_ht_type, s_ht_value);
    display_screen_insert(s_ht_unit, s_ht_value);
    #endif //BLE_APP_HT

/* ***********************************************************************/
/*                              Screen List Link                         */
/* ***********************************************************************/

    //Link Level 1 - Feature with Level 2
    display_screen_link(s_prof, s_feat_top);

    //Link Level 1 - Application Management with Level 2
    display_screen_link(s_app_mgmt, s_adv);

    //Link Level 1 - Platform Information with Level 2
    display_screen_link(s_info, 0);

    #if (BLE_APP_HT)
    //Link Level 2 - Health Thermometer with Level 3
    display_screen_link(s_ht, s_ht_value);
    #endif //BLE_APP_HT

/* ***********************************************************************/
/*                              Screen Set Content                       */
/* ***********************************************************************/

    //Level 1
    display_screen_set(s_prof, NULL, " PROFILES      >", "");
    display_screen_set(s_app_mgmt, NULL, " APPLICATION   >", " MANAGEMENT");
    display_screen_set(s_info, NULL, " PLATFORM      >", " INFORMATION");

    //Level 2 - Features
    #if (BLE_APP_HT)
    display_screen_set(s_ht, NULL, "< HEALTH       >", "  THERMOMETER");
    #endif //BLE_APP_HT
    #if (BLE_APP_ACCEL)
    display_screen_set(s_accel, NULL, "< ACCEL.        >", "");
    #endif //BLE_APP_ACCEL

    //Level 2 - Application Management
    display_screen_set(s_adv, &app_display_hdl_adv, "< ADVERTISING", "s OFF");
    display_screen_set(s_con, NULL, "< CONNECTED", "  NO");
    display_screen_set(s_pin, NULL, "< PIN CODE", "  ------");
    #if RTC_SUPPORT
    display_screen_set(s_rtc, NULL, "< No time", "");
    #endif //RTC_SUPPORT

    #if (BLE_APP_HT)
    //Level 3 - Health Thermometer
    display_screen_set(s_ht_value, &app_display_hdl_ht_temp_value, "< TEMP VALUE", "s --.--");
    display_screen_set(s_ht_type, &app_display_hdl_ht_temp_type, "< TEMP TYPE", "s NONE");
    display_screen_set(s_ht_unit, NULL, "< TEMP UNIT", "  CELCIUS");
    #endif //BLE_APP_HT

    // Start with Profile screen
    display_start(s_prof);

    #if RTC_SUPPORT
    // Register event to handle 1sec tick
    ke_event_callback_set(KE_EVENT_RTC_1S_TICK, &app_display_sec_tick_evt_hdl);
    // Start the periodic RTC interrupt
    rtc_enable_alarm0(RTC_ALARM_SEC, &alarm , &app_display_sec_tick_cbck);
    #endif //RTC_SUPPORT
}

void app_display_set_adv(bool enable)
{
    app_display_env.advertising = enable;
    if (enable)
    {
        display_screen_update(app_display_env.screen_id_adv, 1, "s ON");
    }
    else
    {
        display_screen_update(app_display_env.screen_id_adv, 1, "s OFF");
    }
    display_refresh();
}

void app_display_pin_code(uint32_t pin_code)
{
    // PIN Code line text
    char string[DISPLAY_LINE_SIZE];
    char pin_code_str[6];

    sprintf(pin_code_str, "%d", (int)pin_code);
    //String creation
    strcpy(string, "  ");
    strcat(string, pin_code_str);

    display_screen_update(app_display_env.screen_id_pin, 1, string);

    display_goto_screen(app_display_env.screen_id_pin);
}

void app_display_set_con(bool state)
{
    app_display_env.connected = state;

    if (state)
    {
        display_screen_update(app_display_env.screen_id_con, 1, "  ON");
    }
    else
    {
        display_screen_update(app_display_env.screen_id_con, 1, "  OFF");
    }

    display_refresh();
}

#if (BLE_APP_HT)
void app_display_update_temp_val_screen(float value)
{
    char int_part[4];
    char dec_part[4];
    char string[DISPLAY_LINE_SIZE];
    int int_part_int = (int)(app_ht_env.temp_value)/100;
    int dec_part_int = (int)(app_ht_env.temp_value)%100;

    sprintf(int_part, "%d", int_part_int);
    sprintf(dec_part, "%d", dec_part_int);

    //String creation
    strcpy(string, "s ");
    strcat(string, int_part);
    strcat(string, ".");
    if (dec_part_int < 10)
    {
        strcat(string, "0");
    }
    strcat(string, dec_part);

    display_screen_update(app_display_env.screen_id_ht_value, 1, string);
    display_refresh();
}

void app_display_update_temp_type_screen(const char* type_string)
{
    char string[DISPLAY_LINE_SIZE];

    //String creation
    strcpy(string, "s ");
    strcat(string, type_string);

    display_screen_update(app_display_env.screen_id_ht_type, 1, string);
    display_refresh();
}

#endif //BLE_APP_HT

#endif //DISPLAY_SUPPORT

/// @} APP
