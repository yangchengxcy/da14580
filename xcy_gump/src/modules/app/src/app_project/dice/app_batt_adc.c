/**
****************************************************************************************
*
* @file app_batt_adc.c
*
* @brief ADC module.
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
#include "global_io.h"
#include "stdint.h"
#include "adc.h"

//GZ
uint8_t batt_level __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY
uint8_t old_batt_level __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY
uint8_t batt_alert_en  __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY
uint8_t bat_led_state __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY

uint8_t batt_read_lvl(void)
{
		uint8_t batt_lvl;
		uint16_t adc_sample;
		volatile int i;

	adc_init(GP_ADC_SE, GP_ADC_SIGN);
	
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
