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

#ifdef PROGRAM_ENABLE_UART
#include "uart.h"                   // UART initialization
#endif

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
* Globally reserved GPIOs reservation
*/


#ifdef PROGRAM_ENABLE_UART
    RESERVE_GPIO( UART1_TX, GPIO_PORT_0,  GPIO_PIN_4, PID_UART1_TX);
    RESERVE_GPIO( UART1_RX, GPIO_PORT_0,  GPIO_PIN_5, PID_UART1_RX);    
# ifdef PROGRAM_ALTERNATE_UART_PINS
#if !(BLE_APP_PRESENT)
    RESERVE_GPIO( UART1_TX, GPIO_PORT_0, GPIO_PIN_7, PID_UART1_RTSN);
    RESERVE_GPIO( UART1_RX, GPIO_PORT_0, GPIO_PIN_6, PID_UART1_CTSN);  
    # endif // !BLE_APP_PRESENT
# else
    #if !(BLE_APP_PRESENT)
    RESERVE_GPIO( UART1_TX, GPIO_PORT_0, GPIO_PIN_3, PID_UART1_RTSN);
    RESERVE_GPIO( UART1_RX, GPIO_PORT_0, GPIO_PIN_2, PID_UART1_CTSN);  
# endif // !BLE_APP_PRESENT    
# endif // PROGRAM_ALTERNATE_UART_PINS
#endif // PROGRAM_ENABLE_UART
    
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

#ifdef PROGRAM_ENABLE_UART
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_4, OUTPUT, PID_UART1_TX, false );
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_5, INPUT, PID_UART1_RX, false );    
#ifdef PROGRAM_ALTERNATE_UART_PINS
#if !(BLE_APP_PRESENT) 
#if DISABLE_UART_RTS_CTS    
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_7, OUTPUT, PID_UART1_RTSN, false ); 
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_6, INPUT_PULLDOWN, PID_UART1_CTSN, false ); 
#else
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_7, OUTPUT, PID_UART1_RTSN, false );
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_6, INPUT, PID_UART1_CTSN, false );
#endif // DISABLE_UART_RTS_CTS
#endif // !BLE_APP_PRESENT  
# else //PROGRAM_ALTERNATE_UART_PINS
#if !(BLE_APP_PRESENT) 
#if DISABLE_UART_RTS_CTS    
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_3, OUTPUT, PID_UART1_RTSN, false );
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_2, INPUT_PULLDOWN, PID_UART1_CTSN, false );
#else
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_3, OUTPUT, PID_UART1_RTSN, false );
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_2, INPUT, PID_UART1_CTSN, false );
#endif // DISABLE_UART_RTS_CTS
#endif // !BLE_APP_PRESENT
# endif // PROGRAM_ALTERNATE_UART_PINS
    
#endif // PROGRAM_ENABLE_UART 
    
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
	
    // Initialize UART component

    SetBits16(CLK_PER_REG, UART1_ENABLE, 1);    // enable clock - always @16MHz
	
    // baudr=9-> 115k2
    // mode=3-> no parity, 1 stop bit 8 data length
#ifdef UART_MEGABIT
    uart_init(UART_BAUDRATE_1M, 3);
#else
    uart_init(UART_BAUDRATE_115K2, 3);
#endif // UART_MEGABIT
    SetBits16(CLK_PER_REG, SPI_ENABLE, 1);    // enable  clock
    SetBits16(CLK_PER_REG, SPI_DIV, 1);	    // set divider to 1	
	
	//rom patch
	patch_func();
	
	//Init pads
	set_pad_functions();

    // Enable the pads
	SetBits16(SYS_CTRL_REG, PAD_LATCH_EN, 1);
}
