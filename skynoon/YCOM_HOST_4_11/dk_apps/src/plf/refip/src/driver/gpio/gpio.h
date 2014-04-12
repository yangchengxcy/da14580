/**
 ****************************************************************************************
 *
 * @file gpio.h
 *
 * @brief Hardware GPIO abstruction layer API.
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

#if !defined(_GPIO_H_)
#define _GPIO_H_

#include "arch.h"
#include <stdbool.h>

typedef enum {
    INPUT = 0,
    INPUT_PULLUP = 0x100,
    INPUT_PULLDOWN = 0x200,
    OUTPUT = 0x300,
} GPIO_PUPD;

typedef enum {
    GPIO_PORT_0 = 0,
    GPIO_PORT_1 = 1,
    GPIO_PORT_2 = 2,
    GPIO_PORT_3 = 3,
    GPIO_PORT_3_REMAP = 4,
} GPIO_PORT;

typedef enum {
    GPIO_PIN_0 = 0,
    GPIO_PIN_1 = 1,
    GPIO_PIN_2 = 2,
    GPIO_PIN_3 = 3,
    GPIO_PIN_4 = 4,
    GPIO_PIN_5 = 5,
    GPIO_PIN_6 = 6,
    GPIO_PIN_7 = 7,
    GPIO_PIN_8 = 8,
    GPIO_PIN_9 = 9,
} GPIO_PIN;

typedef enum {
    PID_GPIO = 0,
    PID_UART1_RX,
    PID_UART1_TX,
    PID_UART2_RX,
    PI_UART2_TX,
    PID_SPI_DI,
    PID_SPI_DO,
    PID_SPI_CLK,
    PID_SPI_EN,
    PID_I2C_SCL,
    PID_I2C_SDA,
    PID_UART1_IRDA_RX,
    PID_UART1_IRDA_TX,
    PID_UART2_IRDA_RX,
    PID_UART2_IRDA_TX,
    PID_ADC,
    PID_PWM0,
    PID_PWM1,
    PID_BLE_DIAG,
    PID_UART1_CTSN,
    PID_UART1_RTSN,
    PID_UART2_CTSN,
    PID_UART2_RTSN,
    PID_PWM2,
    PID_PWM3,
    PID_PWM4,
} GPIO_FUNCTION;

// GPIO base address
#define GPIO_BASE   P0_DATA_REG

//
// Macro for pin definition structure
//      name: usage and/or module using it
//      func: GPIO, UART1_RX, UART1_TX, etc.
//
#if DEVELOPMENT__NO_OTP
#define RESERVE_GPIO( name, port, pin, func )   { GPIO[##port##][##pin##] = (GPIO[##port##][##pin##] != 0) ? (-1) : 1;GPIO_status |= ((uint64_t)GPIO[##port##][##pin##] << ##pin##) << (##port## * 16);}
#else
#define RESERVE_GPIO( name, port, pin, func )   {}
#endif    

#if DEVELOPMENT__NO_OTP

#define NO_OF_PORTS 4   // cannot be bigger than 4
#define NO_OF_MAX_PINS_PER_PORT 10  // cannot be bigger than 16

extern int GPIO[NO_OF_PORTS][NO_OF_MAX_PINS_PER_PORT];
    
extern volatile uint64_t GPIO_status;
#endif
    
//
// the GPIO pin allocation is done in another file
//
extern inline void GPIO_init(void);
    
extern inline void GPIO_SetPinFunction( GPIO_PORT port, GPIO_PIN pin, GPIO_PUPD mode, GPIO_FUNCTION function );

extern inline void GPIO_ConfigurePin( GPIO_PORT port, GPIO_PIN pin, GPIO_PUPD mode, GPIO_FUNCTION function,
                               const bool high );
    
extern inline void GPIO_SetActive( GPIO_PORT port, GPIO_PIN pin );
    
extern inline void GPIO_SetInactive( GPIO_PORT port, GPIO_PIN pin );
    
extern inline bool GPIO_GetPinStatus( GPIO_PORT port, GPIO_PIN pin );

#endif // _GPIO_H_

