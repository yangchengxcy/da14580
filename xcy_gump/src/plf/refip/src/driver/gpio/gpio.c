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
#include "gpio.h"
#include "periph_setup.h"

#if DEVELOPMENT__NO_OTP

int GPIO[NO_OF_PORTS][NO_OF_MAX_PINS_PER_PORT];

static const uint16_t p_mask[4] = { 0xFF, 0x3F, 0x3FF, 0x1FF };

volatile uint64_t GPIO_status;


/*
 * Local Functions
 ****************************************************************************************
 */


#endif //DEVELOPMENT__NO_OTP

/*
 * Global Functions
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @brief Initialize the GPIO assignemnt check variables
 *
 * @return void
 ****************************************************************************************
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


/**
 ****************************************************************************************
 * @brief Set the pin type and mode
 *
 * @param[in] port     GPIO port
 * @param[in] pin      GPIO pin
 * @param[in] mode     GPIO pin mode.     INPUT = 0, INPUT_PULLUP = 0x100, INPUT_PULLDOWN = 0x200, OUTPUT = 0x300,
 * @param[in] function GPIO pin usage. GPIO_FUNCTION enumaration.
 *
 * @return void
 ****************************************************************************************
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

/**
 ****************************************************************************************
 * @brief Combined function to set the state and the type and mode of the GPIO pin 
 *
 * @param[in] port     GPIO port
 * @param[in] pin      GPIO pin
 * @param[in] mode     GPIO pin mode.     INPUT = 0, INPUT_PULLUP = 0x100, INPUT_PULLDOWN = 0x200, OUTPUT = 0x300,
 * @param[in] function GPIO pin usage. GPIO_FUNCTION enumaration.
 * @param[in] high     set to TRUE to set the pin into high else low
 *
 * @return void
 ****************************************************************************************
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

/**
 ****************************************************************************************
 * @brief Sets a pin high. The GPIO should have been previously configured as output!
 *
 * @param[in] port     GPIO port
 * @param[in] pin      GPIO pin
 *
 * @return void
 ****************************************************************************************
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

/**
 ****************************************************************************************
 * @brief Sets the GPIO low. The GPIO should have been previously configured as output!
 *
 * @param[in] port     GPIO port
 * @param[in] pin      GPIO pin
 *
 * @return void
 ****************************************************************************************
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

/**
 ****************************************************************************************
 * @brief Gets the GPIO status. The GPIO should have been previously configured as input!
 *
 * @param[in] port     GPIO port
 * @param[in] pin      GPIO pin
 *
 * @return bool        TRUE if the pin is high, FALSE if low.
 ****************************************************************************************
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
