/**
 ****************************************************************************************
 *
 * @file gpio.c
 *
 * @brief Hardware GPIO abstruction layer.
 *
 * Attribution: Niels Thomsen  <div xmlns:cc="http://creativecommons.org/ns#" 
 * xmlns:dct="http://purl.org/dc/terms/" 
 * about="http://www.embeddedrelated.com/showcode/330.php"><span 
 * property="dct:title">GPIO library</span> (<a rel="cc:attributionURL" property="cc:attributionName" 
 * href="http://www.embeddedrelated.com/code.php?submittedby=63051">Niels Thomsen</a>) / <a rel="license" 
 * href="http://creativecommons.org/licenses/by/3.0/">CC BY 3.0</a></div>
 *
 ****************************************************************************************
 */

#include "arch.h"
#include "rwip_config.h"
#include "gpio.h"

#if BLE_HID_DEVICE
#include "app_keyboard.h"
#endif

#if DEVELOPMENT__NO_OTP

#define NO_OF_PORTS 4   // cannot be bigger than 4
#define NO_OF_MAX_PINS_PER_PORT 10  // cannot be bigger than 16

static int GPIO[NO_OF_PORTS][NO_OF_MAX_PINS_PER_PORT];

static uint16_t p_mask[4] = { 0xFF, 0x3F, 0x3FF, 0x1FF };

volatile uint64_t GPIO_status;


/*
 * Local Functions
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

static void GPIO_reservations(void)
{
    /*
     * Globally reserved GPIOs reservation
     */

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
    RESERVE_GPIO( LED1,         GPIO_PORT_2,  GPIO_PIN_8,  PID_GPIO); 
    RESERVE_GPIO( LED2,         GPIO_PORT_2,  GPIO_PIN_5,  PID_GPIO); 

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
    
# endif // (MATRIX_SETUP)

#endif //BLE_HID_DEVICE && BLE_APP_PRESENT
}

#endif //DEVELOPMENT__NO_OTP


/*
 * Global Functions
 ****************************************************************************************
 */

/*
 * Name         : GPIO_init - Initialize gpio assignemnt check 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : none
 *
 * Description  : Initialize the GPIO assignemnt check variables
 *
 * Returns      : void
 *
 */
void GPIO_init( void )
{
#if DEVELOPMENT__NO_OTP
#warning "GPIO assignment checking is active! Deactivate before burning OTP..."
    
    int i, j;

    for (i = 0; i < NO_OF_PORTS; i++)
        for (j = 0; j < NO_OF_MAX_PINS_PER_PORT; j++)
            GPIO[i][j] = 0;
    
    GPIO_reservations();
    
    GPIO_status = 0;
    
    for (i = 0; i < NO_OF_PORTS; i++)
        for (j = 0; j < NO_OF_MAX_PINS_PER_PORT; j++) {
            uint16_t bitmask = (1 << j);
            
            if ( !(p_mask[i] & bitmask) ) // port pin does not exist! continue to next port...
                break;
            
            if (GPIO[i][j] == -1) {
                volatile int port = i;
                volatile int col = j;
                
                __asm("BKPT #0\n"); // this pin has been previously reserved!
            }
            
            if (GPIO[i][j] == 0)
                continue;
            
            GPIO_status |= ((uint64_t)GPIO[i][j] << j) << (i * 16);
        }
#endif    
}

/*
 * Name         : GPIO_SetPinFunction - Set the pin type and mode
 *
 * Scope        : PUBLIC
 *
 * Arguments    : port - GPIO port
 *                pin  - pin
 *                mode - pin mode (input, output...)
 *                function - pin usage (GPIO, UART, SPI...)
 *
 * Description  : Sets the GPIO type and mode
 *
 * Returns      : void
 *
 */
void GPIO_SetPinFunction( GPIO_PORT port, GPIO_PIN pin, GPIO_PUPD mode, GPIO_FUNCTION function )
{
#if DEVELOPMENT__NO_OTP
    if ( !(GPIO_status & ( ((uint64_t)1 << pin) << (port * 16) )) )
                __asm("BKPT #0\n"); // this pin has not been previously reserved!        
#endif    
    if (port == GPIO_PORT_3) port = GPIO_PORT_3_REMAP; // Set to 4 due to P30_MODE_REG address (0x50003086 instead of 0x50003066)
    
    const int data_reg = GPIO_BASE + (port << 5);
    const int mode_reg = data_reg + 0x6 + (pin << 1);
    
    SetWord16(mode_reg, mode | function);
}

/*
 * Name         : GPIO_ConfigurePin - Combined function to set the state
 *                                     and the type and mode of the GPIO pin 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : port - GPIO port
 *                pin  - pin
 *                mode - pin mode (input, output...)
 *                function - pin usage (GPIO, UART, SPI...)
 *                high - set to TRUE to set the pin into high else low
 *
 * Description  : Sets the GPIO state and then its type and mode
 *
 * Returns      : void
 *
 */
void GPIO_ConfigurePin( GPIO_PORT port, GPIO_PIN pin, GPIO_PUPD mode, GPIO_FUNCTION function,
						 const bool high )
{
#if DEVELOPMENT__NO_OTP
    if ( !(GPIO_status & ( ((uint64_t)1 << pin) << (port * 16) )) )
                __asm("BKPT #0\n"); // this pin has not been previously reserved!        
#endif    
    
    if (high)
        GPIO_SetActive( port, pin );
    else
        GPIO_SetInactive( port, pin );
    
	GPIO_SetPinFunction( port, pin, mode, function );
}

/*
 * Name         : GPIO_SetActive - Sets a pin high
 *
 * Scope        : PUBLIC
 *
 * Arguments    : port - GPIO port
 *                pin  - pin
 *
 * Description  : Sets the GPIO high. The GPIO should have been previously configured 
 *                as output!
 *
 * Returns      : void
 *
 */
void GPIO_SetActive( GPIO_PORT port, GPIO_PIN pin )
{
#if DEVELOPMENT__NO_OTP
    if ( !(GPIO_status & ( ((uint64_t)1 << pin) << (port * 16) )) )
                __asm("BKPT #0\n"); // this pin has not been previously reserved!        
#endif    
    if (port == GPIO_PORT_3) port = GPIO_PORT_3_REMAP; // Set to 4 due to P30_MODE_REG address (0x50003086 instead of 0x50003066)
    
    const int data_reg = GPIO_BASE + (port << 5);
    const int set_data_reg = data_reg + 2;
    
	SetWord16(set_data_reg, 1 << pin);
}

/*
 * Name         : GPIO_SetInactive - Sets a pin low
 *
 * Scope        : PUBLIC
 *
 * Arguments    : port - GPIO port
 *                pin  - pin
 *
 * Description  : Sets the GPIO low. The GPIO should have been previously configured 
 *                as output!
 *
 * Returns      : void
 *
 */
void GPIO_SetInactive( GPIO_PORT port, GPIO_PIN pin )
{
#if DEVELOPMENT__NO_OTP
    if ( !(GPIO_status & ( ((uint64_t)1 << pin) << (port * 16) )) )
                __asm("BKPT #0\n"); // this pin has not been previously reserved!        
#endif    
    if (port == GPIO_PORT_3) port = GPIO_PORT_3_REMAP; // Set to 4 due to P30_MODE_REG address (0x50003086 instead of 0x50003066)
    
    const int data_reg = GPIO_BASE + (port << 5);
    const int reset_data_reg = data_reg + 4;
    
	SetWord16(reset_data_reg, 1 << pin);
}

/*
 * Name         : GPIO_GetPinStatus - Gets the state of the pin
 *
 * Scope        : PUBLIC
 *
 * Arguments    : port - GPIO port
 *                pin  - pin
 *
 * Description  : Gets the GPIO status. The GPIO should have been previously configured 
 *                as input!
 *
 * Returns      : TRUE if the pin is high,
 *                FALSE if low.
 *
 */
bool GPIO_GetPinStatus( GPIO_PORT port, GPIO_PIN pin )
{
#if DEVELOPMENT__NO_OTP
    if ( !(GPIO_status & ( ((uint64_t)1 << pin) << (port * 16) )) )
                __asm("BKPT #0\n"); // this pin has not been previously reserved!        
#endif    
    if (port == GPIO_PORT_3) port = GPIO_PORT_3_REMAP; // Set to 4 due to P30_MODE_REG address (0x50003086 instead of 0x50003066)
    
    const int data_reg = GPIO_BASE + (port << 5);
    
    return ( (GetWord16(data_reg) & (1 << pin)) != 0 );
}
