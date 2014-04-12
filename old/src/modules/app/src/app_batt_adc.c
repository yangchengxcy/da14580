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


//GZ
uint8_t batt_level __attribute__((section("exchange_mem_case1")));
uint8_t old_batt_level __attribute__((section("exchange_mem_case1"))); 
uint8_t batt_alert_en  __attribute__((section("exchange_mem_case1"))) = 0; 
uint8_t bat_led_state __attribute__((section("exchange_mem_case1")));

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
        int cnt=100000;
    
		SetBits16(GP_ADC_CTRL_REG, GP_ADC_START, 1);
		while (cnt-- && (GetWord16(GP_ADC_CTRL_REG) & GP_ADC_START) != 0x0000); 
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
