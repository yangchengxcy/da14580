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
#include "periph_setup.h"             // periphera configuration
#include "global_io.h"
#include "gpio.h"
/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/*
 * Name         : GPIO_reservations - Globally reserved GPIOs 
 *
 * Scope        : LOCAL
 *
 * Arguments    : none
 *
 * Description  : Each application reserves its own GPIOs here.
 *                If there are GPIOs that have to be globally reserved (i.e. UART)
 *                then their reservation MUST be done BEFORE any application reservations.
 *
 * Returns      : void
 *
 */
//

#if DEVELOPMENT__NO_OTP
void GPIO_reservations(void)
{
    /*
     * Globally reserved GPIOs reservation
     */
#ifndef FPGA_USED

    // UART GPIOs
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

    /*
     * Application specific GPIOs reservation
     */    
#if (BLE_APP_PRESENT)
#if BLE_PROX_REPORTER
    RESERVE_GPIO( PUSH_BUTTON, GPIO_PORT_0, GPIO_PIN_6, PID_GPIO);   
    RESERVE_GPIO( GREEN_LED, GPIO_PORT_0, GPIO_PIN_7, PID_GPIO);
#endif
#if BLE_STREAMDATA_DEVICE
    RESERVE_GPIO( PUSH_BUTTON, GPIO_PORT_0, GPIO_PIN_6, PID_GPIO);   
#endif
#endif

#if BLE_BATT_SERVER
#if !BLE_HID_DEVICE   
	//Setup LED GPIO for battery alert
    RESERVE_GPIO( RED_LED, GPIO_PORT_1, GPIO_PIN_0, PID_GPIO);
#endif	
#endif

#if BLE_ACCEL
    RESERVE_GPIO( SPI_EN, GPIO_PORT_0, GPIO_PIN_6, PID_SPI_EN);
    RESERVE_GPIO( SPI_CLK, GPIO_PORT_0, GPIO_PIN_0, PID_SPI_CLK);
    RESERVE_GPIO( SPI_DO, GPIO_PORT_0, GPIO_PIN_3, PID_SPI_DO);	
    RESERVE_GPIO( SPI_DI, GPIO_PORT_0, GPIO_PIN_5, PID_SPI_DI);
#if DEEP_SLEEP_ENABLED 	//GZ int    
 	//Set P1_5 to ACCEL's INT1
    RESERVE_GPIO( ACCEL_INT1, GPIO_PORT_1, GPIO_PIN_5, PID_GPIO);
#else
 	//Set P0_7 to ACCEL's INT1
    RESERVE_GPIO( ACCEL_INT1, GPIO_PORT_0, GPIO_PIN_7, PID_GPIO);
#endif
#endif // BLE_ACCEL

#if BLE_APP_KEYBOARD_TESTER    
#if !(OLD_TEST_SETUP)
    RESERVE_GPIO( KEY_0, GPIO_PORT_2, GPIO_PIN_9, PID_GPIO);    // Q1
    RESERVE_GPIO( KEY_0, GPIO_PORT_0, GPIO_PIN_2, PID_GPIO);    // Q2
    RESERVE_GPIO( KEY_0, GPIO_PORT_2, GPIO_PIN_8, PID_GPIO);    // Q3
    RESERVE_GPIO( KEY_0, GPIO_PORT_0, GPIO_PIN_3, PID_GPIO);    // Q4
    RESERVE_GPIO( KEY_0, GPIO_PORT_2, GPIO_PIN_7, PID_GPIO);    // Q5
    RESERVE_GPIO( KEY_0, GPIO_PORT_0, GPIO_PIN_4, PID_GPIO);    // Q6
    RESERVE_GPIO( KEY_0, GPIO_PORT_2, GPIO_PIN_0, PID_GPIO);    // Q7
    RESERVE_GPIO( KEY_0, GPIO_PORT_0, GPIO_PIN_5, PID_GPIO);    // Q8
    RESERVE_GPIO( KEY_0, GPIO_PORT_2, GPIO_PIN_1, PID_GPIO);    // Q9
    RESERVE_GPIO( KEY_0, GPIO_PORT_0, GPIO_PIN_6, PID_GPIO);    // Q10 - STOP BUTTON?
    RESERVE_GPIO( KEY_0, GPIO_PORT_0, GPIO_PIN_7, PID_GPIO);    // Q11
    RESERVE_GPIO( KEY_0, GPIO_PORT_1, GPIO_PIN_0, PID_GPIO);    // Q12
    RESERVE_GPIO( KEY_0, GPIO_PORT_1, GPIO_PIN_1, PID_GPIO);    // Q13
    RESERVE_GPIO( KEY_0, GPIO_PORT_2, GPIO_PIN_2, PID_GPIO);    // Q14
    RESERVE_GPIO( KEY_0, GPIO_PORT_2, GPIO_PIN_3, PID_GPIO);    // Q15
    RESERVE_GPIO( KEY_0, GPIO_PORT_2, GPIO_PIN_4, PID_GPIO);    // Q16
    RESERVE_GPIO( KEY_0, GPIO_PORT_2, GPIO_PIN_5, PID_GPIO);    // Q17
    RESERVE_GPIO( KEY_0, GPIO_PORT_2, GPIO_PIN_6, PID_GPIO);    // Q18
    RESERVE_GPIO( KEY_0, GPIO_PORT_3, GPIO_PIN_0, PID_GPIO);    // Q19
    RESERVE_GPIO( KEY_0, GPIO_PORT_3, GPIO_PIN_1, PID_GPIO);    // Q20
    RESERVE_GPIO( KEY_0, GPIO_PORT_3, GPIO_PIN_2, PID_GPIO);    // Q21
    RESERVE_GPIO( KEY_0, GPIO_PORT_3, GPIO_PIN_3, PID_GPIO);    // Q22
    RESERVE_GPIO( KEY_0, GPIO_PORT_3, GPIO_PIN_4, PID_GPIO);    // Q23
    RESERVE_GPIO( KEY_0, GPIO_PORT_3, GPIO_PIN_5, PID_GPIO);    // Q24
    RESERVE_GPIO( KEY_0, GPIO_PORT_3, GPIO_PIN_6, PID_GPIO);    // Q25
    RESERVE_GPIO( KEY_0, GPIO_PORT_3, GPIO_PIN_7, PID_GPIO);    // Q26
#else
    RESERVE_GPIO( KEY_0, GPIO_PORT_0, GPIO_PIN_7, PID_GPIO);
    RESERVE_GPIO( KEY_0, GPIO_PORT_0, GPIO_PIN_6, PID_GPIO);
#endif    
#endif // BLE_APP_KEYBOARD_TESTER


#if BLE_HID_DEVICE && BLE_APP_PRESENT

// **************************************************************************************
# if (MATRIX_SETUP == 1)
    // Inputs (columns)
    RESERVE_GPIO( INPUT_COL_0,  GPIO_PORT_2,  GPIO_PIN_0,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_1,  GPIO_PORT_2,  GPIO_PIN_1,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_2,  GPIO_PORT_2,  GPIO_PIN_2,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_3,  GPIO_PORT_3,  GPIO_PIN_6,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_4,  GPIO_PORT_2,  GPIO_PIN_3,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_5,  GPIO_PORT_3,  GPIO_PIN_7,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_6,  GPIO_PORT_2,  GPIO_PIN_4,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_7,  GPIO_PORT_2,  GPIO_PIN_5,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_8,  GPIO_PORT_2,  GPIO_PIN_6,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_9,  GPIO_PORT_2,  GPIO_PIN_7,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_10, GPIO_PORT_1,  GPIO_PIN_0,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_11, GPIO_PORT_1,  GPIO_PIN_1,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_12, GPIO_PORT_1,  GPIO_PIN_2,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_13, GPIO_PORT_1,  GPIO_PIN_3,  PID_GPIO); 

    // Outputs (rows)
# if !defined(CFG_PRINTF)
    RESERVE_GPIO( OUTPUT_ROW_0, GPIO_PORT_0,  GPIO_PIN_0,  PID_GPIO); 
# endif
    RESERVE_GPIO( OUTPUT_ROW_1, GPIO_PORT_2,  GPIO_PIN_8,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_2, GPIO_PORT_2,  GPIO_PIN_9,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_3, GPIO_PORT_0,  GPIO_PIN_3,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_4, GPIO_PORT_0,  GPIO_PIN_4,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_5, GPIO_PORT_0,  GPIO_PIN_5,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_6, GPIO_PORT_0,  GPIO_PIN_6,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_7, GPIO_PORT_0,  GPIO_PIN_7,  PID_GPIO); 
    
// **************************************************************************************
# elif (MATRIX_SETUP == 2)
    // Inputs (columns)
    RESERVE_GPIO( INPUT_COL_0,  GPIO_PORT_0,  GPIO_PIN_5,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_1,  GPIO_PORT_0,  GPIO_PIN_4,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_2,  GPIO_PORT_1,  GPIO_PIN_3,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_3,  GPIO_PORT_1,  GPIO_PIN_2,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_4,  GPIO_PORT_1,  GPIO_PIN_1,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_5,  GPIO_PORT_1,  GPIO_PIN_0,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_6,  GPIO_PORT_2,  GPIO_PIN_7,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_7,  GPIO_PORT_2,  GPIO_PIN_6,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_8,  GPIO_PORT_2,  GPIO_PIN_5,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_9,  GPIO_PORT_2,  GPIO_PIN_4,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_10, GPIO_PORT_3,  GPIO_PIN_7,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_11, GPIO_PORT_2,  GPIO_PIN_3,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_12, GPIO_PORT_3,  GPIO_PIN_6,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_13, GPIO_PORT_2,  GPIO_PIN_2,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_14, GPIO_PORT_2,  GPIO_PIN_1,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_15, GPIO_PORT_2,  GPIO_PIN_0,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_16, GPIO_PORT_3,  GPIO_PIN_1,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_17, GPIO_PORT_3,  GPIO_PIN_2,  PID_GPIO); 

    // Outputs (rows)
    RESERVE_GPIO( OUTPUT_ROW_0, GPIO_PORT_3,  GPIO_PIN_4,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_1, GPIO_PORT_3,  GPIO_PIN_3,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_2, GPIO_PORT_3,  GPIO_PIN_5,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_3, GPIO_PORT_0,  GPIO_PIN_6,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_4, GPIO_PORT_2,  GPIO_PIN_9,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_5, GPIO_PORT_2,  GPIO_PIN_8,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_6, GPIO_PORT_0,  GPIO_PIN_7,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_7, GPIO_PORT_3,  GPIO_PIN_0,  PID_GPIO); 

// **************************************************************************************
# elif (MATRIX_SETUP == 3)
    // Inputs (columns)
    RESERVE_GPIO( INPUT_COL_0,  GPIO_PORT_1,  GPIO_PIN_0,  PID_GPIO);
    RESERVE_GPIO( INPUT_COL_1,  GPIO_PORT_2,  GPIO_PIN_0,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_2,  GPIO_PORT_2,  GPIO_PIN_7,  PID_GPIO);
    RESERVE_GPIO( INPUT_COL_3,  GPIO_PORT_2,  GPIO_PIN_6,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_4,  GPIO_PORT_1,  GPIO_PIN_2,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_5,  GPIO_PORT_1,  GPIO_PIN_1,  PID_GPIO);
    RESERVE_GPIO( INPUT_COL_6,  GPIO_PORT_2,  GPIO_PIN_9,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_7,  GPIO_PORT_2,  GPIO_PIN_4,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_8,  GPIO_PORT_0,  GPIO_PIN_5,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_9,  GPIO_PORT_2,  GPIO_PIN_3,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_10, GPIO_PORT_0,  GPIO_PIN_4,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_11, GPIO_PORT_2,  GPIO_PIN_1,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_12, GPIO_PORT_2,  GPIO_PIN_2,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_13, GPIO_PORT_1,  GPIO_PIN_3,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_14, GPIO_PORT_0,  GPIO_PIN_2,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_15, GPIO_PORT_0,  GPIO_PIN_3,  PID_GPIO); 

    // Outputs (rows)
    RESERVE_GPIO( OUTPUT_ROW_0, GPIO_PORT_3,  GPIO_PIN_1,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_1, GPIO_PORT_3,  GPIO_PIN_4,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_2, GPIO_PORT_3,  GPIO_PIN_6,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_3, GPIO_PORT_3,  GPIO_PIN_0,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_4, GPIO_PORT_3,  GPIO_PIN_2,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_5, GPIO_PORT_3,  GPIO_PIN_3,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_6, GPIO_PORT_3,  GPIO_PIN_5,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_7, GPIO_PORT_3,  GPIO_PIN_7,  PID_GPIO); 
    
    // EEPROM
    RESERVE_GPIO( EEPROM_SCL,   GPIO_PORT_0,  GPIO_PIN_7,  PID_I2C_SCL); 
    RESERVE_GPIO( EEPROM_SDA,   GPIO_PORT_0,  GPIO_PIN_6,  PID_I2C_SDA); 
    
    // LEDS
    RESERVE_GPIO( GREEN_LED,    GPIO_PORT_2,  GPIO_PIN_8,  PID_GPIO); 
    RESERVE_GPIO( RED_LED,      GPIO_PORT_2,  GPIO_PIN_5,  PID_GPIO); 
    

// **************************************************************************************
# elif (MATRIX_SETUP == 4)
    // Inputs (columns)
    RESERVE_GPIO( INPUT_COL_0,  GPIO_PORT_2,  GPIO_PIN_0,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_1,  GPIO_PORT_2,  GPIO_PIN_1,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_2,  GPIO_PORT_2,  GPIO_PIN_2,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_3,  GPIO_PORT_3,  GPIO_PIN_6,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_4,  GPIO_PORT_2,  GPIO_PIN_3,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_5,  GPIO_PORT_3,  GPIO_PIN_7,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_6,  GPIO_PORT_2,  GPIO_PIN_4,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_7,  GPIO_PORT_2,  GPIO_PIN_5,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_8,  GPIO_PORT_2,  GPIO_PIN_6,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_9,  GPIO_PORT_2,  GPIO_PIN_7,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_10, GPIO_PORT_1,  GPIO_PIN_0,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_11, GPIO_PORT_1,  GPIO_PIN_1,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_12, GPIO_PORT_1,  GPIO_PIN_2,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_13, GPIO_PORT_1,  GPIO_PIN_3,  PID_GPIO); 

    // Outputs (rows)
# if !defined(CFG_PRINTF)
    RESERVE_GPIO( OUTPUT_ROW_0, GPIO_PORT_0,  GPIO_PIN_0,  PID_GPIO); 
# endif
    RESERVE_GPIO( OUTPUT_ROW_1, GPIO_PORT_2,  GPIO_PIN_8,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_2, GPIO_PORT_2,  GPIO_PIN_9,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_3, GPIO_PORT_0,  GPIO_PIN_3,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_4, GPIO_PORT_0,  GPIO_PIN_4,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_5, GPIO_PORT_0,  GPIO_PIN_5,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_6, GPIO_PORT_3,  GPIO_PIN_1,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_7, GPIO_PORT_3,  GPIO_PIN_2,  PID_GPIO); 

    // EEPROM
    RESERVE_GPIO( EEPROM_SCL,   GPIO_PORT_0,  GPIO_PIN_7,  PID_I2C_SCL); 
    RESERVE_GPIO( EEPROM_SDA,   GPIO_PORT_0,  GPIO_PIN_6,  PID_I2C_SDA); 

// **************************************************************************************
# elif (MATRIX_SETUP == 5)
    // Inputs (columns)
    RESERVE_GPIO( INPUT_COL_0,  GPIO_PORT_2,  GPIO_PIN_0,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_1,  GPIO_PORT_2,  GPIO_PIN_1,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_2,  GPIO_PORT_2,  GPIO_PIN_2,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_3,  GPIO_PORT_3,  GPIO_PIN_6,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_4,  GPIO_PORT_2,  GPIO_PIN_3,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_5,  GPIO_PORT_3,  GPIO_PIN_7,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_6,  GPIO_PORT_2,  GPIO_PIN_4,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_7,  GPIO_PORT_2,  GPIO_PIN_5,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_8,  GPIO_PORT_2,  GPIO_PIN_6,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_9,  GPIO_PORT_2,  GPIO_PIN_7,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_10, GPIO_PORT_1,  GPIO_PIN_0,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_11, GPIO_PORT_1,  GPIO_PIN_1,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_12, GPIO_PORT_1,  GPIO_PIN_2,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_13, GPIO_PORT_1,  GPIO_PIN_3,  PID_GPIO); 

    // Outputs (rows)
# if !defined(CFG_PRINTF)
    RESERVE_GPIO( OUTPUT_ROW_0, GPIO_PORT_0,  GPIO_PIN_0,  PID_GPIO); 
# endif
    RESERVE_GPIO( OUTPUT_ROW_1, GPIO_PORT_2,  GPIO_PIN_8,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_2, GPIO_PORT_2,  GPIO_PIN_9,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_3, GPIO_PORT_0,  GPIO_PIN_3,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_4, GPIO_PORT_0,  GPIO_PIN_4,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_5, GPIO_PORT_0,  GPIO_PIN_5,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_6, GPIO_PORT_0,  GPIO_PIN_6,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_7, GPIO_PORT_0,  GPIO_PIN_7,  PID_GPIO); 

// **************************************************************************************
# elif (MATRIX_SETUP == 6)
    // Inputs (columns)
    RESERVE_GPIO( INPUT_COL_0,  GPIO_PORT_2,  GPIO_PIN_0,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_1,  GPIO_PORT_2,  GPIO_PIN_1,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_2,  GPIO_PORT_2,  GPIO_PIN_2,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_3,  GPIO_PORT_3,  GPIO_PIN_6,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_4,  GPIO_PORT_2,  GPIO_PIN_3,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_5,  GPIO_PORT_3,  GPIO_PIN_7,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_6,  GPIO_PORT_2,  GPIO_PIN_4,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_7,  GPIO_PORT_2,  GPIO_PIN_5,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_8,  GPIO_PORT_2,  GPIO_PIN_6,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_9,  GPIO_PORT_2,  GPIO_PIN_7,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_10, GPIO_PORT_1,  GPIO_PIN_0,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_11, GPIO_PORT_1,  GPIO_PIN_1,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_12, GPIO_PORT_1,  GPIO_PIN_2,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_13, GPIO_PORT_1,  GPIO_PIN_3,  PID_GPIO); 

    // Outputs (rows)
# if !defined(CFG_PRINTF)
    RESERVE_GPIO( OUTPUT_ROW_0, GPIO_PORT_0,  GPIO_PIN_0,  PID_GPIO); 
# endif
    RESERVE_GPIO( OUTPUT_ROW_1, GPIO_PORT_2,  GPIO_PIN_8,  PID_GPIO);  
    RESERVE_GPIO( OUTPUT_ROW_2, GPIO_PORT_2,  GPIO_PIN_9,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_3, GPIO_PORT_0,  GPIO_PIN_3,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_4, GPIO_PORT_0,  GPIO_PIN_4,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_5, GPIO_PORT_0,  GPIO_PIN_5,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_6, GPIO_PORT_3,  GPIO_PIN_1,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_7, GPIO_PORT_3,  GPIO_PIN_2,  PID_GPIO); 

    // EEPROM
    RESERVE_GPIO( EEPROM_SCL,   GPIO_PORT_0,  GPIO_PIN_7,  PID_I2C_SCL); 
    RESERVE_GPIO( EEPROM_SDA,   GPIO_PORT_0,  GPIO_PIN_6,  PID_I2C_SDA); 

// **************************************************************************************
# elif (MATRIX_SETUP == 7)
    // Inputs (columns)
    RESERVE_GPIO( INPUT_COL_0,  GPIO_PORT_0,  GPIO_PIN_5,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_1,  GPIO_PORT_0,  GPIO_PIN_4,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_2,  GPIO_PORT_1,  GPIO_PIN_3,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_3,  GPIO_PORT_1,  GPIO_PIN_2,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_4,  GPIO_PORT_1,  GPIO_PIN_1,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_5,  GPIO_PORT_1,  GPIO_PIN_0,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_6,  GPIO_PORT_2,  GPIO_PIN_7,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_7,  GPIO_PORT_2,  GPIO_PIN_6,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_8,  GPIO_PORT_2,  GPIO_PIN_5,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_9,  GPIO_PORT_2,  GPIO_PIN_4,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_10, GPIO_PORT_3,  GPIO_PIN_7,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_11, GPIO_PORT_2,  GPIO_PIN_3,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_12, GPIO_PORT_3,  GPIO_PIN_6,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_13, GPIO_PORT_2,  GPIO_PIN_2,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_14, GPIO_PORT_2,  GPIO_PIN_1,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_15, GPIO_PORT_2,  GPIO_PIN_0,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_16, GPIO_PORT_3,  GPIO_PIN_1,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_17, GPIO_PORT_3,  GPIO_PIN_2,  PID_GPIO); 

    // Outputs (rows)
    RESERVE_GPIO( OUTPUT_ROW_0, GPIO_PORT_3,  GPIO_PIN_4,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_1, GPIO_PORT_3,  GPIO_PIN_3,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_2, GPIO_PORT_3,  GPIO_PIN_5,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_3, GPIO_PORT_0,  GPIO_PIN_6,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_4, GPIO_PORT_2,  GPIO_PIN_9,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_5, GPIO_PORT_2,  GPIO_PIN_8,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_6, GPIO_PORT_0,  GPIO_PIN_7,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_7, GPIO_PORT_3,  GPIO_PIN_0,  PID_GPIO); 

    // EEPROM
    RESERVE_GPIO( EEPROM_SCL,   GPIO_PORT_0,  GPIO_PIN_3,  PID_I2C_SCL); 
    RESERVE_GPIO( EEPROM_SDA,   GPIO_PORT_0,  GPIO_PIN_2,  PID_I2C_SDA); 
    
# elif (MATRIX_SETUP == 8)
    // Inputs (columns)
    RESERVE_GPIO( INPUT_COL_0,  GPIO_PORT_0,  GPIO_PIN_5,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_1,  GPIO_PORT_0,  GPIO_PIN_4,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_2,  GPIO_PORT_0,  GPIO_PIN_1,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_3,  GPIO_PORT_0,  GPIO_PIN_0,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_4,  GPIO_PORT_1,  GPIO_PIN_1,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_5,  GPIO_PORT_1,  GPIO_PIN_0,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_6,  GPIO_PORT_2,  GPIO_PIN_7,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_7,  GPIO_PORT_2,  GPIO_PIN_6,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_8,  GPIO_PORT_2,  GPIO_PIN_5,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_9,  GPIO_PORT_2,  GPIO_PIN_4,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_10, GPIO_PORT_3,  GPIO_PIN_7,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_11, GPIO_PORT_2,  GPIO_PIN_3,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_12, GPIO_PORT_3,  GPIO_PIN_6,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_13, GPIO_PORT_2,  GPIO_PIN_2,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_14, GPIO_PORT_2,  GPIO_PIN_1,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_15, GPIO_PORT_2,  GPIO_PIN_0,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_16, GPIO_PORT_3,  GPIO_PIN_1,  PID_GPIO); 
    RESERVE_GPIO( INPUT_COL_17, GPIO_PORT_3,  GPIO_PIN_2,  PID_GPIO); 

    // Outputs (rows)
    RESERVE_GPIO( OUTPUT_ROW_0, GPIO_PORT_3,  GPIO_PIN_4,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_1, GPIO_PORT_3,  GPIO_PIN_3,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_2, GPIO_PORT_3,  GPIO_PIN_5,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_3, GPIO_PORT_0,  GPIO_PIN_6,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_4, GPIO_PORT_2,  GPIO_PIN_9,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_5, GPIO_PORT_2,  GPIO_PIN_8,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_6, GPIO_PORT_0,  GPIO_PIN_7,  PID_GPIO); 
    RESERVE_GPIO( OUTPUT_ROW_7, GPIO_PORT_3,  GPIO_PIN_0,  PID_GPIO); 

    // EEPROM
    RESERVE_GPIO( EEPROM_SCL,   GPIO_PORT_0,  GPIO_PIN_3,  PID_I2C_SCL); 
    RESERVE_GPIO( EEPROM_SDA,   GPIO_PORT_0,  GPIO_PIN_2,  PID_I2C_SDA); 
    
    // LEDs
    RESERVE_GPIO( GREEN_LED,    GPIO_PORT_1,  GPIO_PIN_5,  PID_GPIO); 
    RESERVE_GPIO( RED_LED,      GPIO_PORT_1,  GPIO_PIN_4,  PID_GPIO); 
    
# endif // (MATRIX_SETUP)

#endif //BLE_HID_DEVICE && BLE_APP_PRESENT
#endif  //FPGA_USED
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
#ifndef FPGA_USED    
#ifdef PROGRAM_ENABLE_UART
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_4, OUTPUT, PID_UART1_TX, false );
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_5, INPUT, PID_UART1_RX, false );    
# ifdef PROGRAM_ALTERNATE_UART_PINS
#if !(BLE_APP_PRESENT) 
#if DISABLE_UART_RTS_CTS    
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_7, OUTPUT, PID_UART1_RTSN, false ); //CD SOS
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_6, INPUT_PULLDOWN, PID_UART1_CTSN, false ); //CD SOS
#else
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_7, OUTPUT, PID_UART1_RTSN, false );//CD SOS
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_6, INPUT, PID_UART1_CTSN, false );//CD SOS
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

#if BLE_ACCEL
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_6, OUTPUT, PID_SPI_EN, true );
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_0, OUTPUT, PID_SPI_CLK, false );
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_3, OUTPUT, PID_SPI_DO, false );	
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_5, INPUT, PID_SPI_DI, false );
#if DEEP_SLEEP_ENABLED 	//GZ int
    //Set P1_5 to ACCEL's INT1
    GPIO_ConfigurePin( GPIO_PORT_1, GPIO_PIN_5, INPUT, PID_GPIO, false );
#else
	//Set P0_7 to ACCEL's INT1
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_7, INPUT, PID_GPIO, false );
#endif
#endif // BLE_ACCEL
#else   //FPGA_USED
    RESERVE_GPIO( UART1_TX, GPIO_PORT_0,  GPIO_PIN_0, PID_UART1_TX);
    RESERVE_GPIO( UART1_RX, GPIO_PORT_0,  GPIO_PIN_1, PID_UART1_RX);    
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_0, OUTPUT, PID_UART1_TX, false );
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_1, INPUT, PID_UART1_RX, false );    
    
    RESERVE_GPIO( UART1_TX, GPIO_PORT_2, GPIO_PIN_5, PID_UART1_RTSN);
    RESERVE_GPIO( UART1_RX, GPIO_PORT_2, GPIO_PIN_6, PID_UART1_CTSN);      
    GPIO_ConfigurePin( GPIO_PORT_2, GPIO_PIN_5, OUTPUT, PID_UART1_RTSN, false );//CD SOS
    GPIO_ConfigurePin( GPIO_PORT_2, GPIO_PIN_6, INPUT_PULLDOWN, PID_UART1_RTSN/*PID_UART1_CTSN*/, false );//FPGA issue?
		
	
//ALLOCATE SPI SIGNALS
    RESERVE_GPIO( SPI_EN, GPIO_PORT_1, GPIO_PIN_0, PID_SPI_EN);
    RESERVE_GPIO( SPI_CLK, GPIO_PORT_0, GPIO_PIN_4, PID_SPI_CLK);
    RESERVE_GPIO( SPI_DO, GPIO_PORT_0, GPIO_PIN_6, PID_SPI_DO);	
    RESERVE_GPIO( SPI_DI, GPIO_PORT_0, GPIO_PIN_7, PID_SPI_DI);

    GPIO_ConfigurePin( GPIO_PORT_1, GPIO_PIN_0, OUTPUT, PID_SPI_EN, true );
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_4, OUTPUT, PID_SPI_CLK, false );
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_6, OUTPUT, PID_SPI_DO, false );	
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_7, INPUT, PID_SPI_DI, false );

		
//    SetWord16(P24_MODE_REG,0x300);
//    SetWord16(P23_MODE_REG,0x300);
//    SetWord16(P2_RESET_DATA_REG,0x18);
#endif
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
	
	// TODO: Application specific - Modify accordingly!
	// Example: Activate UART and SPI.
	
    // Initialize UART component
#ifdef PROGRAM_ENABLE_UART
    SetBits16(CLK_PER_REG, UART1_ENABLE, 1);    // enable clock - always @16MHz
	
    // baudr=9-> 115k2
    // mode=3-> no parity, 1 stop bit 8 data length
#ifdef UART_MEGABIT
    uart_init(UART_BAUDRATE_1M, 3);
#else
    uart_init(UART_BAUDRATE_115K2, 3);
#endif // UART_MEGABIT
    //NVIC_SetPriority(UART_IRQn, 1);             // remove if the bug in uart.c is fixed in ES4(b)!
#endif // PROGRAM_ENABLE_UART

    //FPGA  
#ifdef FPGA_USED    
    SetBits16(CLK_PER_REG, SPI_ENABLE, 1);    // enable  clock
    SetBits16(CLK_PER_REG, SPI_DIV, 1);	    // set divider to 1	
#endif

#if BLE_ACCEL
    SetBits16(CLK_PER_REG, SPI_ENABLE, 1);      // enable  clock
    SetBits16(CLK_PER_REG, SPI_DIV, 1);	        // set divider to 1	
	SetBits16(CLK_PER_REG, WAKEUPCT_ENABLE, 1); // enable clock of Wakeup Controller
#endif

#if 0
	//Example: Do something with the timer if need be...
    SetWord16(TIMER0_CTRL_REG, 0); 
    SetWord16(TIMER0_RELOAD_M_REG, 0);
    SetWord16(TIMER0_RELOAD_N_REG, 0);
    SetWord16(TIMER0_ON_REG, 0);
#endif
	
	//rom patch
	patch_func();
	
	//Init pads
	set_pad_functions();

#ifndef FPGA_USED
#if (BLE_APP_PRESENT)
#if BLE_HID_DEVICE
	SetBits16(CLK_PER_REG, WAKEUPCT_ENABLE, 1); // enable clock of Wakeup Controller
//	if (app_env.app_flags & KBD_START_SCAN) // reinit kbd only if needed
	if ( !(app_env.app_flags & KBD_START_SCAN) && !(app_env.app_flags & KBD_SCANNING) )
		app_kbd_reinit_matrix();
#endif  // BLE_HID_DEVICE
    
#if BLE_PROX_REPORTER
    app_proxr_port_reinit();
#elif BLE_FINDME_LOCATOR
    app_button_enable();
#endif //BLE_PROX_REPORTER

#if BLE_SPOTA_RECEIVER
    app_spotar_init();
#endif // BLE_SPOTA_RECEIVER
#if BLE_STREAMDATA_DEVICE
    stream_start_button_init();
#endif  //BLE_STREAMDATA_DEVICE
#if BLE_BATTERY_SERVER
    app_batt_port_reinit();
#endif //BLE_BATTERY_SERVER
#endif 
#endif    
    // Enable the pads
	SetBits16(SYS_CTRL_REG, PAD_LATCH_EN, 1);
}
