/**
****************************************************************************************
*
* @file app_button_led.c
*
* @brief Push Button and LED user interface
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

#include "app.h"                     // application definitions
#include "app_task.h"                // application task definitions
#include <string.h>                  // string manipulation and functions
#include "co_bt.h"
#include "arch.h"                      // platform definitions
#include "ke_timer.h"									// kernel timer 
#include "app_button_led.h"          // button/LED function definitions
#include "rwble_config.h"
#include "gpio.h"

#if (BLE_APP_PRESENT)
#if BLE_PROX_REPORTER
#include "app_proxr.h"              // findme target functions
#endif


#if BLE_FINDME_LOCATOR || BLE_FINDME_TARGET
#include "app_findme.h"
#endif
#if BLE_FINDME_LOCATOR
#include "findl_task.h"              // findme target functions
#endif

#if BLE_FINDME_TARGET
#include "findt_task.h"              // findme target functions
#endif




extern void periph_init(void);

#if BLE_PROX_REPORTER
extern app_alert_state alert_state;
#endif //BLE_PROX_REPORTER

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
*/
 
 /**
 ****************************************************************************************
 * Findme Target Application Functions
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @brief Handles Push Button event.
 *
 * @return void.
 ****************************************************************************************
 */

void GPIO3_Handler(void)
{

#if BLE_PROX_REPORTER	
		
		NVIC_DisableIRQ(GPIO3_IRQn);
	
		if (alert_state.lvl != PROXR_ALERT_NONE)
		{
			app_proxr_alert_stop();	
		}
        else 
#endif         
		
        {
#if BLE_FINDME_LOCATOR            
        if (ke_state_get(TASK_FINDL) == FINDL_CONNECTED)
        {
                    
            app_findl_set_alert();
                
        }
		
#endif 
        }
        
		SetWord16(GPIO_RESET_IRQ_REG, 0x8);		
		NVIC_EnableIRQ(GPIO3_IRQn);		

}

/**
 * WKUP_QUADEC_Handler: Wakeup when button is pushed
 */
void WKUP_QUADEC_Handler(void)
{

	/*
	* The system operates with RC16 clk
	*/
	
	/*
	* Prepare WDOG, i.e. stop
	*/
    
	SetWord16(SET_FREEZE_REG, FRZ_WDOG);
	
	SetBits16(CLK_PER_REG, WAKEUPCT_ENABLE, 1);  // enable clock of Wakeup Controller
	SetWord16(WKUP_RESET_IRQ_REG, 1); //Acknowledge it
	
	//No more interrupts of this kind
	SetBits16(WKUP_CTRL_REG, WKUP_ENABLE_IRQ, 0);
	NVIC_DisableIRQ(WKUP_QUADEC_IRQn);
	
	/*
	* Init System Power Domain blocks: GPIO, WD Timer, Sys Timer, etc.
	* Power up and init Peripheral Power Domain blocks,
	* and finally release the pad latches.
	*/
	if(GetBits16(SYS_STAT_REG, PER_IS_DOWN))
		periph_init();   
	
	//Wakeup BLE here
    //	SetBits32(BLE_DEEPSLCNTL_REG, SOFT_WAKEUP_REQ, 1);
	
	//We cannot call app_adv_start() here because it will end up in setting a kernel timer but 
	//BLE is not yet up and running and time has not been compensated! We'll send a kernel msg instead!
	//app_adv_start();
    
	return;
    
}


void button_enable_wakeup_irq(void)
{
	volatile int temp;
	
	SetBits16(CLK_PER_REG, WAKEUPCT_ENABLE, 1);  // enable clock of Wakeup Controller
	
	SetWord16(WKUP_COMPARE_REG, 1); //Wait for 1 event and wakeup
	//Setup IRQ
    
    SetWord16(WKUP_POL_P0_REG, 0x40);
    SetWord16(WKUP_SELECT_P0_REG, 0x40);
    SetBits16(WKUP_CTRL_REG, WKUP_DEB_VALUE, 0);
    SetBits16(WKUP_CTRL_REG, WKUP_ENABLE_IRQ, 1);
    
	//SetWord16(WKUP_CTRL_REG, 0xC0A); //Enable IRQ, Active Low, no debounce, Monitor P0[7]
	
	SetWord16(WKUP_RESET_IRQ_REG, 1); //clear any garbagge
	NVIC_ClearPendingIRQ(WKUP_QUADEC_IRQn); //clear it to be on the safe side...
	NVIC_SetPriority(WKUP_QUADEC_IRQn, 2); //set priority
	NVIC_EnableIRQ(WKUP_QUADEC_IRQn);
    
}


void app_button_enable(void)
{   
   	NVIC_DisableIRQ(GPIO3_IRQn);

    //Push Button input	
    GPIO_ConfigurePin(XCY_KB_1, INPUT_PULLUP, PID_GPIO, false );
	
	SetBits16(GPIO_INT_LEVEL_CTRL_REG, EDGE_LEVELn3, 1); //select falling edge P1.1
	SetBits16(GPIO_INT_LEVEL_CTRL_REG, INPUT_LEVEL3, 1); //select falling edge P1.1
	SetWord16(GPIO_IRQ3_IN_SEL_REG, 7); //P1.1 is selected, GPIO Input push buttton
	//SetWord16(GPIO_RESET_IRQ_REG, 0x8);
	
	NVIC_SetPriority(GPIO3_IRQn,1); 
	NVIC_EnableIRQ(GPIO3_IRQn);

}

void app_led_enable(void)
{
    // LED Output 
    GPIO_ConfigurePin( XCY_LED_GPIO, OUTPUT, PID_GPIO, false );
}

#if BLE_PREC

uint16_t prec_adv_count __attribute__((section("exchange_mem_case1")));
uint16_t prec_adv_interval __attribute__((section("exchange_mem_case1")));

/**
 ****************************************************************************************
 * @brief Start Timer to control Adertising interval.
 *
 ****************************************************************************************
 */
void app_prec_adv_start(void)
{
	prec_adv_count = 0;
    ke_timer_set(APP_PREC_ADV_TIMER, TASK_APP, 50);
}

/**
 ****************************************************************************************
 * @brief Stop Timer that controls Adertising interval.
 *
 ****************************************************************************************
 */
void app_prec_adv_stop(void)
{
    prec_adv_count = 0;
    ke_timer_clear(APP_PREC_ADV_TIMER, TASK_APP);
}

#endif 
#endif //(BLE_APP_PRESENT)
/// @} APP
