/**
 ****************************************************************************************
 *
 * @file app_batt.c
 *
 * @brief Battery application.
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

#include "rwble_config.h"

#if BLE_BATT_SERVER

//#include <string.h>                  // string manipulation and functions

#include "app.h"                     // application definitions
#include "app_task.h"                // application task definitions

#include "bass_task.h"

#include "co_bt.h"

#include "arch.h"                      // platform definitions
#include "app_batt.h"
#include "gpio.h"

uint8_t batt_level __attribute__((section("exchange_mem_case1")));
uint8_t old_batt_level __attribute__((section("exchange_mem_case1"))); 
uint8_t batt_alert_en  __attribute__((section("exchange_mem_case1"))) = 0; 
uint8_t bat_led_state __attribute__((section("exchange_mem_case1")));

uint8_t batt_read_lvl(void);

#if BLE_BATT_SERVER


/**
 ****************************************************************************************
 * Add a Battery Service in the DB
 ****************************************************************************************
 */
void app_batt_create_db(void)
{
    // Add BASS in the database
    struct bass_create_db_req * req = KE_MSG_ALLOC(BASS_CREATE_DB_REQ, TASK_BASS,
                                                   TASK_APP, bass_create_db_req);

	req->bas_nb = 1;
	req->features[0] = BAS_BATT_LVL_NTF_SUP;
	req->features[1] = BAS_BATT_LVL_NTF_NOT_SUP;

    // Send the message
	//puts("Send BASS_CREATE_DB_REQ\r\n");
    ke_msg_send(req);
}


/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */
void app_batt_init(void)
{
	// called from app.c::app_init() when chip boots
	//puts("app_batt_init()\r\n");
}


/**
 ****************************************************************************************
 * batt Application Functions
 ****************************************************************************************
*/

void batt_init(void)
{
	//puts("batt_init()\r\n");
	// setup the hardware driver: registers etc for the battery voltage reading, etc.
	// called from app_batt_enable() below
	
	volatile int i;
	
#if !BLE_HID_DEVICE
    //Setup LED GPIO for battery alert
    GPIO_ConfigurePin( GPIO_PORT_1, GPIO_PIN_0, OUTPUT, PID_GPIO, false );
#endif	
}



void app_batt_enable(uint8_t batt_lvl, uint8_t old_batt_lvl)
{
	//puts("app_batt_enable\r\n");
	// called from app_task.c::gap_le_create_conn_req_cmp_evt_handler()

	// Allocate the message
	struct bass_enable_req * req = KE_MSG_ALLOC(BASS_ENABLE_REQ, TASK_BASS, TASK_APP,
											 bass_enable_req);

	// Fill in the parameter structure
	// Fill in the parameter structure
	req->conhdl             = app_env.conhdl;
	req->sec_lvl            = PERM(SVC, ENABLE);
	req->con_type           = PRF_CON_NORMAL; // PRF_CON_DISCOVERY;
	req->batt_level_ntf_cfg[0] = 1; // Notifiacation ON by default.
	req->batt_level_ntf_cfg[1] = 0;
	req->old_batt_lvl[0] = 0;
	req->old_batt_lvl[1] = 0;
	req->current_batt_lvl[0] = 0;
	req->current_batt_lvl[1] = 0;

	struct prf_char_pres_fmt *batt_level_pres_format = req->batt_level_pres_format;

	batt_level_pres_format[0].unit = 0x27AD;
	batt_level_pres_format[0].description = 0; // FIXME CORRECT?
	batt_level_pres_format[0].format = 4;
	batt_level_pres_format[0].exponent = 0;
	batt_level_pres_format[0].namespace = 1;
	
	batt_init();
	
	// Send the message
	//puts("Send BASS_ENABLE_REQ\r\n");
	ke_msg_send(req);

	// Reset the bass driver
	
}

void app_batt_lvl()
{   // may be called multiple times
	uint8_t batt_lvl;
	
	batt_lvl = batt_read_lvl();
	
	if (batt_lvl != old_batt_level)
		app_batt_set_level(batt_lvl);
	
	//update old_batt_lvl for the next use
	old_batt_level = batt_lvl;
	
	//if battery level is bellow 5% then start a battery alert to notify user
	if(batt_lvl <5)
		app_batt_alert_start();
}	
	
void app_batt_set_level(uint8_t batt_lvl)
{	
	// Allocate the message
	struct bass_batt_level_upd_req * req = KE_MSG_ALLOC(BASS_BATT_LEVEL_UPD_REQ, TASK_BASS, TASK_APP,
                                                 bass_batt_level_upd_req);
		
    // Fill in the parameter structure
    req->conhdl = app_env.conhdl;
    req->bas_instance = 0;
	req->batt_level = batt_lvl;
	
  // Send the message
	//puts("Send BASS_BATT_LEVEL_UPD_REQ");
	ke_msg_send(req);
		
}

void app_batt_stop(void)
{
	app_batt_poll_stop();
}


/**
 ****************************************************************************************
 * @brief Starts battery level polling.
 *
 
 * @return void.
 ****************************************************************************************
 */

void app_batt_poll_start(void)
{
#if (BLE_APP_KEYBOARD)
	ke_timer_set(APP_BATT_TIMER, TASK_APP, 6000);	// 60sec
#else
	ke_timer_set(APP_BATT_TIMER, TASK_APP, 150); //60000); //10 mins timeout
#endif
}

/**
 ****************************************************************************************
 * @brief Stops battery level polling.
 *
 * @return void.
 ****************************************************************************************
 */

void app_batt_poll_stop(void)
{
  ke_timer_clear(APP_BATT_TIMER, TASK_APP);
}

/**
 ****************************************************************************************
 * @brief Starts battery alert. Battery Low.
 *
 
 * @return void.
 ****************************************************************************************
*/

void app_batt_alert_start(void)
{
	batt_alert_en = 1;
#if !BLE_HID_DEVICE	
    GPIO_SetActive( GPIO_PORT_1, GPIO_PIN_0);
#endif	
	bat_led_state = 1;
	ke_timer_set(APP_BATT_ALERT_TIMER, TASK_APP, 200); //60000); //10 mins timeout
}

/**
 ****************************************************************************************
 * @brief Stops battery alert. Battery Low.
 *
 * @return void.
 ****************************************************************************************
 */

void app_batt_alert_stop(void)
{	
	batt_alert_en = 0;
#if !BLE_HID_DEVICE
    GPIO_SetInactive( GPIO_PORT_1, GPIO_PIN_0);
#endif	
	bat_led_state = 0;
	ke_timer_clear(APP_BATT_ALERT_TIMER, TASK_APP);
}

#endif //BLE_BATT_SERVER

/**
 ****************************************************************************************
 * ADC module Functions
 ****************************************************************************************
*/

void adc_init(void){
	SetWord16(GP_ADC_CTRL_REG,  GP_ADC_LDO_EN | GP_ADC_SE);
	SetWord16(GP_ADC_CTRL_REG,  GP_ADC_LDO_EN | GP_ADC_SE | GP_ADC_EN);
	SetWord16(GP_ADC_CTRL2_REG, 0x000E);																	// Enable 3x attenuation
//	SetWord16(GP_ADC_OFFP_REG, 0x297);
	
}

void adc_enable_channel(uint16_t a){
	SetBits16(GP_ADC_CTRL_REG,GP_ADC_SEL,a&0xF);
	
}

void adc_disable(void){
	SetWord16(GP_ADC_CTRL_REG,  0);
}

int adc_get_sample(void){
	SetBits16(GP_ADC_CTRL_REG, GP_ADC_START, 1);
	while ((GetWord16(GP_ADC_CTRL_REG) & GP_ADC_START) != 0x0000); 
	SetWord16(GP_ADC_CLEAR_INT_REG, 0x0000); // Clear interrupt
	return GetWord16(GP_ADC_RESULT_REG); 

}


uint8_t batt_read_lvl(void)
{
	uint8_t batt_lvl;
	uint16_t adc_sample;
	volatile int i;

	adc_init();
	
//	for (i = 0; i<=1000; i++); //delay
	
	adc_enable_channel(0x07);
	
	adc_sample = adc_get_sample();
	
	adc_disable();
			  
	//calculate remaining battery life 
	if (adc_sample > 0x308)
		batt_lvl = 100;
	else if (adc_sample <= 0x308 && adc_sample > 0x2D6) 
		batt_lvl = 28 + (uint8_t)(((float)((float)(adc_sample - 0x2D6)/(float)(0x308 - 0x2D6))) * 72) ;
	else if (adc_sample <= 0x2D6 && adc_sample > 0x26C) 
		batt_lvl = 4 + (uint8_t)(((float)((float)(adc_sample - 0x26C)/(float)(0x2D6 - 0x26C))) * 24) ;
	else if (adc_sample <= 0x26C && adc_sample > 0x205) 
		batt_lvl = (uint8_t)(((float)((float)(adc_sample - 0x205)/(float)(0x26C - 0x205))) * 4) ;
	else 
		batt_lvl = 0;
		
	return batt_lvl;	
}

void app_batt_port_reinit(void)
{
    
	batt_init();

#if !BLE_HID_DEVICE	
	if(bat_led_state == 1){
        GPIO_SetActive( GPIO_PORT_1, GPIO_PIN_0);
    }
	else{
        GPIO_SetInactive( GPIO_PORT_1, GPIO_PIN_0);
    }
#endif	

}

#endif //BLE_BATT_SERVER
/// @} APP
