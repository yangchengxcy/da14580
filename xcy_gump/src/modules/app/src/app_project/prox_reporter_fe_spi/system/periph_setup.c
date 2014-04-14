/**
 ****************************************************************************************
 *
 * @file periph_setup.c
 *
 * @brief Peripherals setup and initialization. 
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
#include "rwip_config.h"             // SW configuration
#include "periph_setup.h"            // periphera configuration
#include "global_io.h"
#include "gpio.h"
#include "spi_hci.h"                    // UART initialization
/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

#if DEVELOPMENT__NO_OTP

/**
 ****************************************************************************************
 * @brief GPIO_reservations. Globally reserved GPIOs
 *
 * @return void 
 ****************************************************************************************
*/

void GPIO_reservations(void)
{

/*
* Application specific GPIOs reservation
*/    
  
   RESERVE_GPIO( SPI_CLK, GPIO_PORT_0, GPIO_PIN_0, PID_SPI_CLK);
   RESERVE_GPIO( SPI_EN, GPIO_PORT_0, GPIO_PIN_1, PID_SPI_EN);    
   RESERVE_GPIO( SPI_DO, GPIO_PORT_0, GPIO_PIN_2, PID_SPI_DO);  
   RESERVE_GPIO( SPI_DI, GPIO_PORT_0, GPIO_PIN_3, PID_SPI_DI); 
   RESERVE_GPIO( SPI_DREADY, GPIO_PORT_0, GPIO_PIN_7, PID_GPIO);
}
#endif

/**
 ****************************************************************************************
 * @brief Map port pins
 *
 * The Uart and SPI port pins and GPIO ports(for debugging) are mapped
 ****************************************************************************************
 */
void set_pad_functions(void)        // set gpio port function mode
{
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_0, INPUT_PULLUP,  PID_SPI_CLK, false);
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_1, INPUT_PULLUP,  PID_SPI_EN, false);
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_2, OUTPUT,  PID_SPI_DO, false); 
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_3, INPUT_PULLUP,  PID_SPI_DI, false); 
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_7, OUTPUT, PID_GPIO, false);	
}


/**
 ****************************************************************************************
 * @brief Enable pad's and peripheral clocks assuming that peripherals' power domain is down.
 *
 * The Uart and SPi clocks are set. 
 ****************************************************************************************
 */
void periph_init(void)  // set i2c, spi, uart, uart2 serial clks
{
	// Power up peripherals' power domain
    SetBits16(PMU_CTRL_REG, PERIPH_SLEEP, 0);
    while (!(GetWord16(SYS_STAT_REG) & PER_IS_UP)) ; 
    
#if ES4_CODE
    SetBits16(CLK_16M_REG,XTAL16_BIAS_SH_DISABLE, 1);
#endif    	

    spi_init_func();
	
	//rom patch
	patch_func();
	
	//Init pads
	set_pad_functions();

    // Enable the pads
	SetBits16(SYS_CTRL_REG, PAD_LATCH_EN, 1);
}
