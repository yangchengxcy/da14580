/**
 ****************************************************************************************
 *
 * @file app_kbd_key_matrix.h
 *
 * @brief HID Keyboard key scan matrix definitions.
 *
 * Copyright (C) 2014. Dialog Semiconductor Ltd, unpublished work. This computer 
 * program includes Confidential, Proprietary Information and is a Trade Secret of 
 * Dialog Semiconductor Ltd.  All use, disclosure, and/or reproduction is prohibited 
 * unless authorized in writing. All Rights Reserved.
 *
 * <bluetooth.support@diasemi.com> and contributors.
 *
 ****************************************************************************************
 */

#ifndef APP_KBD_KEY_MATRIX_H_
#define APP_KBD_KEY_MATRIX_H_

#include "app_kbd_config.h"
#include "gpio.h"


#define INIT_LED_PINS       0
#define INIT_EEPROM_PINS    0


#if MATRIX_SETUP == 0
/*****************************************************************************************************************************************
 *
 * Rev.1 or Rev.2 DKs (keyboard #1)
 *
 *****************************************************************************************************************************************/
 
#define HAS_EEPROM          (0)




#elif MATRIX_SETUP == 1
/*****************************************************************************************************************************************
 *
 * Rev.3 DK with QFN48 (keyboard #1)
 *
 *****************************************************************************************************************************************/
 
#define HAS_EEPROM          (0)

#if (HAS_MULTI_BOND)
#error "This keyboard setup does not support EEPROM!"    
#endif

// Switches required by the compiler
#undef INIT_LED_PINS
#undef INIT_EEPROM_PINS
#define INIT_LED_PINS       0
#define INIT_EEPROM_PINS    0


#if !defined(CFG_PRINTF)
#define DECLARE_UART_SHARED_PINS                                            \
        RESERVE_GPIO( OUTPUT_ROW_0, GPIO_PORT_0,  GPIO_PIN_0,  PID_GPIO);   \
#else
#define DECLARE_UART_SHARED_PINS
#endif

#ifndef CFG_PRINTF
#define DECLARE_UART_USED_PINS                                              \
    {                                                                       \
        RESERVE_GPIO( OUTPUT_ROW_4, GPIO_PORT_0,  GPIO_PIN_4,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_5, GPIO_PORT_0,  GPIO_PIN_5,  PID_GPIO);   \
    }
    
#else
#define DECLARE_UART_USED_PINS    
#endif

#define DECLARE_KEYBOARD_GPIOS                                              \
    {                                                                       \
        RESERVE_GPIO( INPUT_COL_0,  GPIO_PORT_2,  GPIO_PIN_0,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_1,  GPIO_PORT_2,  GPIO_PIN_1,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_2,  GPIO_PORT_2,  GPIO_PIN_2,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_3,  GPIO_PORT_3,  GPIO_PIN_6,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_4,  GPIO_PORT_2,  GPIO_PIN_3,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_5,  GPIO_PORT_3,  GPIO_PIN_7,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_6,  GPIO_PORT_2,  GPIO_PIN_4,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_7,  GPIO_PORT_2,  GPIO_PIN_5,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_8,  GPIO_PORT_2,  GPIO_PIN_6,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_9,  GPIO_PORT_2,  GPIO_PIN_7,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_10, GPIO_PORT_1,  GPIO_PIN_0,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_11, GPIO_PORT_1,  GPIO_PIN_1,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_12, GPIO_PORT_1,  GPIO_PIN_2,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_13, GPIO_PORT_1,  GPIO_PIN_3,  PID_GPIO);   \
        DECLARE_UART_SHARED_PINS                                            \
        RESERVE_GPIO( OUTPUT_ROW_1, GPIO_PORT_2,  GPIO_PIN_8,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_2, GPIO_PORT_2,  GPIO_PIN_9,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_3, GPIO_PORT_0,  GPIO_PIN_3,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_6, GPIO_PORT_0,  GPIO_PIN_6,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_7, GPIO_PORT_0,  GPIO_PIN_7,  PID_GPIO);   \
        DECLARE_UART_USED_PINS                                              \
    }





#elif MATRIX_SETUP == 2
/*****************************************************************************************************************************************
 *
 * Rev.3 DK with QFN48 (keyboard #2)
 *
 *****************************************************************************************************************************************/

#define HAS_EEPROM          (0)
    
#if (HAS_MULTI_BOND)
#error "This keyboard setup does not support EEPROM!"    
#endif

// Switches required by the compiler
#undef INIT_LED_PINS
#undef INIT_EEPROM_PINS
#define INIT_LED_PINS       0
#define INIT_EEPROM_PINS    0


#ifndef CFG_PRINTF
#define DECLARE_UART_USED_PINS                                              \
    {                                                                       \
        RESERVE_GPIO( INPUT_COL_0,  GPIO_PORT_0,  GPIO_PIN_5,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_1,  GPIO_PORT_0,  GPIO_PIN_4,  PID_GPIO);   \
    }
    
#else
#define DECLARE_UART_USED_PINS    
#endif
    
#define DECLARE_KEYBOARD_GPIOS                                              \
    {                                                                       \
        RESERVE_GPIO( INPUT_COL_2,  GPIO_PORT_1,  GPIO_PIN_3,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_3,  GPIO_PORT_1,  GPIO_PIN_2,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_4,  GPIO_PORT_1,  GPIO_PIN_1,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_5,  GPIO_PORT_1,  GPIO_PIN_0,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_6,  GPIO_PORT_2,  GPIO_PIN_7,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_7,  GPIO_PORT_2,  GPIO_PIN_6,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_8,  GPIO_PORT_2,  GPIO_PIN_5,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_9,  GPIO_PORT_2,  GPIO_PIN_4,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_10, GPIO_PORT_3,  GPIO_PIN_7,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_11, GPIO_PORT_2,  GPIO_PIN_3,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_12, GPIO_PORT_3,  GPIO_PIN_6,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_13, GPIO_PORT_2,  GPIO_PIN_2,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_14, GPIO_PORT_2,  GPIO_PIN_1,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_15, GPIO_PORT_2,  GPIO_PIN_0,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_16, GPIO_PORT_3,  GPIO_PIN_1,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_17, GPIO_PORT_3,  GPIO_PIN_2,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_0, GPIO_PORT_3,  GPIO_PIN_4,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_1, GPIO_PORT_3,  GPIO_PIN_3,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_2, GPIO_PORT_3,  GPIO_PIN_5,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_3, GPIO_PORT_0,  GPIO_PIN_6,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_4, GPIO_PORT_2,  GPIO_PIN_9,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_5, GPIO_PORT_2,  GPIO_PIN_8,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_6, GPIO_PORT_0,  GPIO_PIN_7,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_7, GPIO_PORT_3,  GPIO_PIN_0,  PID_GPIO);   \
        DECLARE_UART_USED_PINS                                              \
    }





#elif MATRIX_SETUP == 3
/*****************************************************************************************************************************************
 *
 * Rev.3 DK with QFN48 (keyboard #3)
 *
 *****************************************************************************************************************************************/

#ifdef EEPROM_ON
    
#define HAS_EEPROM          (1)

#if ( !(HAS_MITM) && !(BLE_SPOTA_RECEIVER) )
 #warning "Using EEPROM without MITM is useless!"
#endif

#else
#define HAS_EEPROM          (0)
#endif // EEPROM_ON

#if (HAS_MULTI_BOND)
 #if ( !(HAS_EEPROM) )
  #error "Multiple bonding support requires EEPROM!"
 #endif
#endif // (HAS_MULTI_BOND)

    
// Pin definition (irrelevant to whether EEPROM or LEDs are used or not since the HW setup is fixed!)
#define I2C_SDA_PORT        GPIO_PORT_0
#define I2C_SDA_PIN         GPIO_PIN_6
#define I2C_SCL_PORT        GPIO_PORT_0
#define I2C_SCL_PIN         GPIO_PIN_7

#define KBD_GREEN_LED_PORT  GPIO_PORT_2
#define KBD_GREEN_LED_PIN   GPIO_PIN_8

#define KBD_RED_LED_PORT    GPIO_PORT_2
#define KBD_RED_LED_PIN     GPIO_PIN_5

// Switches used to initialize I2C and LED pins properly (even when not used) to avoid leakage
#undef INIT_LED_PINS
#undef INIT_EEPROM_PINS
#define INIT_LED_PINS       1
#define INIT_EEPROM_PINS    1


#define DECLARE_EEPROM_PINS                                                 \
    {                                                                       \
        RESERVE_GPIO( EEPROM_SDA,   GPIO_PORT_0,  GPIO_PIN_6,  PID_I2C_SDA);\
        RESERVE_GPIO( EEPROM_SCL,   GPIO_PORT_0,  GPIO_PIN_7,  PID_I2C_SCL);\
    }

#define DECLARE_LED_PINS                                                    \
    {                                                                       \
        RESERVE_GPIO( GREEN_LED,    GPIO_PORT_2,  GPIO_PIN_8,  PID_GPIO);   \
        RESERVE_GPIO( RED_LED,      GPIO_PORT_2,  GPIO_PIN_5,  PID_GPIO);   \
    }

#ifndef CFG_PRINTF
#define DECLARE_UART_USED_PINS                                              \
    {                                                                       \
        RESERVE_GPIO( INPUT_COL_8,  GPIO_PORT_0,  GPIO_PIN_5,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_10, GPIO_PORT_0,  GPIO_PIN_4,  PID_GPIO);   \
    }
    
#else
#define DECLARE_UART_USED_PINS    
#endif
    
#define DECLARE_KEYBOARD_GPIOS                                              \
    {                                                                       \
        RESERVE_GPIO( INPUT_COL_0,  GPIO_PORT_1,  GPIO_PIN_0,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_1,  GPIO_PORT_2,  GPIO_PIN_0,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_2,  GPIO_PORT_2,  GPIO_PIN_7,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_3,  GPIO_PORT_2,  GPIO_PIN_6,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_4,  GPIO_PORT_1,  GPIO_PIN_2,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_5,  GPIO_PORT_1,  GPIO_PIN_1,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_6,  GPIO_PORT_2,  GPIO_PIN_9,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_7,  GPIO_PORT_2,  GPIO_PIN_4,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_9,  GPIO_PORT_2,  GPIO_PIN_3,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_11, GPIO_PORT_2,  GPIO_PIN_1,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_12, GPIO_PORT_2,  GPIO_PIN_2,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_13, GPIO_PORT_1,  GPIO_PIN_3,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_14, GPIO_PORT_0,  GPIO_PIN_2,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_15, GPIO_PORT_0,  GPIO_PIN_3,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_0, GPIO_PORT_3,  GPIO_PIN_1,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_1, GPIO_PORT_3,  GPIO_PIN_4,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_2, GPIO_PORT_3,  GPIO_PIN_6,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_3, GPIO_PORT_3,  GPIO_PIN_0,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_4, GPIO_PORT_3,  GPIO_PIN_2,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_5, GPIO_PORT_3,  GPIO_PIN_3,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_6, GPIO_PORT_3,  GPIO_PIN_5,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_7, GPIO_PORT_3,  GPIO_PIN_7,  PID_GPIO);   \
        DECLARE_EEPROM_PINS                                                 \
        DECLARE_LED_PINS                                                    \
        DECLARE_UART_USED_PINS                                              \
    }




    
#elif MATRIX_SETUP == 4
/*****************************************************************************************************************************************
 *
 * Rev.3 DK with QFN48 (keyboard #1 with EEPROM)
 *
 *****************************************************************************************************************************************/
 
#ifdef EEPROM_ON
    
#define HAS_EEPROM          (1)

#if ( !(HAS_MITM) && !(BLE_SPOTA_RECEIVER) )
 #warning "Using EEPROM without MITM is useless!"
#endif

#else
#define HAS_EEPROM          (0)
#endif // EEPROM_ON

#if (HAS_MULTI_BOND)
 #if ( !(HAS_EEPROM) )
  #error "Multiple bonding support requires EEPROM!"
 #endif
#endif // (HAS_MULTI_BOND)

    
// Pin definition (irrelevant to whether EEPROM is used or not since the HW setup is fixed!)
#define I2C_SDA_PORT        GPIO_PORT_0
#define I2C_SDA_PIN         GPIO_PIN_6
#define I2C_SCL_PORT        GPIO_PORT_0
#define I2C_SCL_PIN         GPIO_PIN_7

// Switches used to initialize I2C pins properly (even when not used) to avoid leakage
#undef INIT_LED_PINS
#undef INIT_EEPROM_PINS
#define INIT_LED_PINS       0
#define INIT_EEPROM_PINS    1


#if !defined(CFG_PRINTF)
#define DECLARE_UART_SHARED_PINS                                            \
        RESERVE_GPIO( OUTPUT_ROW_0, GPIO_PORT_0,  GPIO_PIN_0,  PID_GPIO);   \
#else
#define DECLARE_UART_SHARED_PINS
#endif

#define DECLARE_EEPROM_PINS                                                 \
    {                                                                       \
        RESERVE_GPIO( EEPROM_SCL,   GPIO_PORT_0,  GPIO_PIN_7,  PID_I2C_SCL);\
        RESERVE_GPIO( EEPROM_SDA,   GPIO_PORT_0,  GPIO_PIN_6,  PID_I2C_SDA);\
    }

#ifndef CFG_PRINTF
#define DECLARE_UART_USED_PINS                                              \
    {                                                                       \
        RESERVE_GPIO( OUTPUT_ROW_4, GPIO_PORT_0,  GPIO_PIN_4,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_5, GPIO_PORT_0,  GPIO_PIN_5,  PID_GPIO);   \
    }
    
#else
#define DECLARE_UART_USED_PINS    
#endif

#define DECLARE_KEYBOARD_GPIOS                                              \
    {                                                                       \
        RESERVE_GPIO( INPUT_COL_0,  GPIO_PORT_2,  GPIO_PIN_0,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_1,  GPIO_PORT_2,  GPIO_PIN_1,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_2,  GPIO_PORT_2,  GPIO_PIN_2,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_3,  GPIO_PORT_3,  GPIO_PIN_6,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_4,  GPIO_PORT_2,  GPIO_PIN_3,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_5,  GPIO_PORT_3,  GPIO_PIN_7,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_6,  GPIO_PORT_2,  GPIO_PIN_4,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_7,  GPIO_PORT_2,  GPIO_PIN_5,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_8,  GPIO_PORT_2,  GPIO_PIN_6,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_9,  GPIO_PORT_2,  GPIO_PIN_7,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_10, GPIO_PORT_1,  GPIO_PIN_0,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_11, GPIO_PORT_1,  GPIO_PIN_1,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_12, GPIO_PORT_1,  GPIO_PIN_2,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_13, GPIO_PORT_1,  GPIO_PIN_3,  PID_GPIO);   \
        DECLARE_UART_SHARED_PINS                                            \
        RESERVE_GPIO( OUTPUT_ROW_1, GPIO_PORT_2,  GPIO_PIN_8,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_2, GPIO_PORT_2,  GPIO_PIN_9,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_3, GPIO_PORT_0,  GPIO_PIN_3,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_6, GPIO_PORT_3,  GPIO_PIN_1,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_7, GPIO_PORT_3,  GPIO_PIN_2,  PID_GPIO);   \
        DECLARE_EEPROM_PINS                                                 \
        DECLARE_UART_USED_PINS                                              \
    }

        




#elif MATRIX_SETUP == 5
/*****************************************************************************************************************************************
 *
 * Rev.3 DK with QFN48 (keyboard #1 for Apple products)
 *
 *****************************************************************************************************************************************/

#define HAS_EEPROM          (0)
    
#if (HAS_MULTI_BOND)
#error "This keyboard setup does not support EEPROM!"    
#endif

// Switches required by the compiler
#undef INIT_LED_PINS
#undef INIT_EEPROM_PINS
#define INIT_LED_PINS       0
#define INIT_EEPROM_PINS    0

#if !defined(CFG_PRINTF)
#define DECLARE_UART_SHARED_PINS                                            \
        RESERVE_GPIO( OUTPUT_ROW_0, GPIO_PORT_0,  GPIO_PIN_0,  PID_GPIO);   \
#else
#define DECLARE_UART_SHARED_PINS
#endif

#ifndef CFG_PRINTF
#define DECLARE_UART_USED_PINS                                              \
    {                                                                       \
        RESERVE_GPIO( OUTPUT_ROW_4, GPIO_PORT_0,  GPIO_PIN_4,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_5, GPIO_PORT_0,  GPIO_PIN_5,  PID_GPIO);   \
    }
    
#else
#define DECLARE_UART_USED_PINS    
#endif

#define DECLARE_KEYBOARD_GPIOS                                              \
    {                                                                       \
        RESERVE_GPIO( INPUT_COL_0,  GPIO_PORT_2,  GPIO_PIN_0,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_1,  GPIO_PORT_2,  GPIO_PIN_1,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_2,  GPIO_PORT_2,  GPIO_PIN_2,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_3,  GPIO_PORT_3,  GPIO_PIN_6,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_4,  GPIO_PORT_2,  GPIO_PIN_3,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_5,  GPIO_PORT_3,  GPIO_PIN_7,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_6,  GPIO_PORT_2,  GPIO_PIN_4,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_7,  GPIO_PORT_2,  GPIO_PIN_5,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_8,  GPIO_PORT_2,  GPIO_PIN_6,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_9,  GPIO_PORT_2,  GPIO_PIN_7,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_10, GPIO_PORT_1,  GPIO_PIN_0,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_11, GPIO_PORT_1,  GPIO_PIN_1,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_12, GPIO_PORT_1,  GPIO_PIN_2,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_13, GPIO_PORT_1,  GPIO_PIN_3,  PID_GPIO);   \
        DECLARE_UART_SHARED_PINS                                            \
        RESERVE_GPIO( OUTPUT_ROW_1, GPIO_PORT_2,  GPIO_PIN_8,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_2, GPIO_PORT_2,  GPIO_PIN_9,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_3, GPIO_PORT_0,  GPIO_PIN_3,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_6, GPIO_PORT_0,  GPIO_PIN_6,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_7, GPIO_PORT_0,  GPIO_PIN_7,  PID_GPIO);   \
        DECLARE_UART_USED_PINS                                              \
    }


    


#elif MATRIX_SETUP == 6
/*****************************************************************************************************************************************
 *
 * Rev.3 DK with QFN48 (keyboard #1 for Apple products)
 *
 *****************************************************************************************************************************************/
 
#ifdef EEPROM_ON
    
#define HAS_EEPROM          (1)

#if ( !(HAS_MITM) && !(BLE_SPOTA_RECEIVER) )
 #warning "Using EEPROM without MITM is useless!"
#endif

#else
#define HAS_EEPROM          (0)
#endif // EEPROM_ON

#if (HAS_MULTI_BOND)
 #if ( !(HAS_EEPROM) )
  #error "Multiple bonding support requires EEPROM!"
 #endif
#endif // (HAS_MULTI_BOND)

    
// Pin definition (irrelevant to whether EEPROM is used or not since the HW setup is fixed!)
#define I2C_SDA_PORT        GPIO_PORT_0
#define I2C_SDA_PIN         GPIO_PIN_6
#define I2C_SCL_PORT        GPIO_PORT_0
#define I2C_SCL_PIN         GPIO_PIN_7

// Switches used to initialize I2C pins properly (even when not used) to avoid leakage
#undef INIT_LED_PINS
#undef INIT_EEPROM_PINS
#define INIT_LED_PINS       0
#define INIT_EEPROM_PINS    1


#if !defined(CFG_PRINTF)
#define DECLARE_UART_SHARED_PINS                                            \
        RESERVE_GPIO( OUTPUT_ROW_0, GPIO_PORT_0,  GPIO_PIN_0,  PID_GPIO);   \
#else
#define DECLARE_UART_SHARED_PINS
#endif

#define DECLARE_EEPROM_PINS                                                 \
    {                                                                       \
        RESERVE_GPIO( EEPROM_SCL,   GPIO_PORT_0,  GPIO_PIN_7,  PID_I2C_SCL);\
        RESERVE_GPIO( EEPROM_SDA,   GPIO_PORT_0,  GPIO_PIN_6,  PID_I2C_SDA);\
    }

#ifndef CFG_PRINTF
#define DECLARE_UART_USED_PINS                                              \
    {                                                                       \
        RESERVE_GPIO( OUTPUT_ROW_4, GPIO_PORT_0,  GPIO_PIN_4,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_5, GPIO_PORT_0,  GPIO_PIN_5,  PID_GPIO);   \
    }
    
#else
#define DECLARE_UART_USED_PINS    
#endif

#define DECLARE_KEYBOARD_GPIOS                                              \
    {                                                                       \
        RESERVE_GPIO( INPUT_COL_0,  GPIO_PORT_2,  GPIO_PIN_0,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_1,  GPIO_PORT_2,  GPIO_PIN_1,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_2,  GPIO_PORT_2,  GPIO_PIN_2,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_3,  GPIO_PORT_3,  GPIO_PIN_6,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_4,  GPIO_PORT_2,  GPIO_PIN_3,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_5,  GPIO_PORT_3,  GPIO_PIN_7,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_6,  GPIO_PORT_2,  GPIO_PIN_4,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_7,  GPIO_PORT_2,  GPIO_PIN_5,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_8,  GPIO_PORT_2,  GPIO_PIN_6,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_9,  GPIO_PORT_2,  GPIO_PIN_7,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_10, GPIO_PORT_1,  GPIO_PIN_0,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_11, GPIO_PORT_1,  GPIO_PIN_1,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_12, GPIO_PORT_1,  GPIO_PIN_2,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_13, GPIO_PORT_1,  GPIO_PIN_3,  PID_GPIO);   \
        DECLARE_UART_SHARED_PINS                                            \
        RESERVE_GPIO( OUTPUT_ROW_1, GPIO_PORT_2,  GPIO_PIN_8,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_2, GPIO_PORT_2,  GPIO_PIN_9,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_3, GPIO_PORT_0,  GPIO_PIN_3,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_6, GPIO_PORT_3,  GPIO_PIN_1,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_7, GPIO_PORT_3,  GPIO_PIN_2,  PID_GPIO);   \
        DECLARE_EEPROM_PINS                                                 \
        DECLARE_UART_USED_PINS                                              \
    }





#elif MATRIX_SETUP == 7
/*****************************************************************************************************************************************
 *
 * Rev.3 DK with QFN48 (keyboard #2 with EEPROM)
 *
 *****************************************************************************************************************************************/

#ifdef EEPROM_ON
    
#define HAS_EEPROM          (1)

#if ( !(HAS_MITM) && !(BLE_SPOTA_RECEIVER) )
 #warning "Using EEPROM without MITM is useless!"
#endif

#else
#define HAS_EEPROM          (0)
#endif // EEPROM_ON

#if (HAS_MULTI_BOND)
 #if ( !(HAS_EEPROM) )
  #error "Multiple bonding support requires EEPROM!"
 #endif
#endif // (HAS_MULTI_BOND)

    
// Pin definition (irrelevant to whether EEPROM is used or not since the HW setup is fixed!)
#define I2C_SDA_PORT        GPIO_PORT_0
#define I2C_SDA_PIN         GPIO_PIN_2
#define I2C_SCL_PORT        GPIO_PORT_0
#define I2C_SCL_PIN         GPIO_PIN_3

// Switches used to initialize I2C pins properly (even when not used) to avoid leakage
#undef INIT_LED_PINS
#undef INIT_EEPROM_PINS
#define INIT_LED_PINS       0
#define INIT_EEPROM_PINS    1


#define DECLARE_EEPROM_PINS                                                 \
    {                                                                       \
        RESERVE_GPIO( EEPROM_SCL,   GPIO_PORT_0,  GPIO_PIN_3,  PID_I2C_SCL);\
        RESERVE_GPIO( EEPROM_SDA,   GPIO_PORT_0,  GPIO_PIN_2,  PID_I2C_SDA);\
    }

#ifndef CFG_PRINTF
#define DECLARE_UART_USED_PINS                                              \
    {                                                                       \
        RESERVE_GPIO( INPUT_COL_0,  GPIO_PORT_0,  GPIO_PIN_5,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_1,  GPIO_PORT_0,  GPIO_PIN_4,  PID_GPIO);   \
    }
    
#else
#define DECLARE_UART_USED_PINS    
#endif

#define DECLARE_KEYBOARD_GPIOS                                              \
    {                                                                       \
        RESERVE_GPIO( INPUT_COL_2,  GPIO_PORT_1,  GPIO_PIN_3,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_3,  GPIO_PORT_1,  GPIO_PIN_2,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_4,  GPIO_PORT_1,  GPIO_PIN_1,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_5,  GPIO_PORT_1,  GPIO_PIN_0,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_6,  GPIO_PORT_2,  GPIO_PIN_7,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_7,  GPIO_PORT_2,  GPIO_PIN_6,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_8,  GPIO_PORT_2,  GPIO_PIN_5,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_9,  GPIO_PORT_2,  GPIO_PIN_4,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_10, GPIO_PORT_3,  GPIO_PIN_7,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_11, GPIO_PORT_2,  GPIO_PIN_3,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_12, GPIO_PORT_3,  GPIO_PIN_6,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_13, GPIO_PORT_2,  GPIO_PIN_2,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_14, GPIO_PORT_2,  GPIO_PIN_1,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_15, GPIO_PORT_2,  GPIO_PIN_0,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_16, GPIO_PORT_3,  GPIO_PIN_1,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_17, GPIO_PORT_3,  GPIO_PIN_2,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_0, GPIO_PORT_3,  GPIO_PIN_4,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_1, GPIO_PORT_3,  GPIO_PIN_3,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_2, GPIO_PORT_3,  GPIO_PIN_5,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_3, GPIO_PORT_0,  GPIO_PIN_6,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_4, GPIO_PORT_2,  GPIO_PIN_9,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_5, GPIO_PORT_2,  GPIO_PIN_8,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_6, GPIO_PORT_0,  GPIO_PIN_7,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_7, GPIO_PORT_3,  GPIO_PIN_0,  PID_GPIO);   \
        DECLARE_EEPROM_PINS                                                 \
        DECLARE_UART_USED_PINS                                              \
    }





#elif MATRIX_SETUP == 8
/*****************************************************************************************************************************************
 *
 * Reference Design (#1)
 *
 *****************************************************************************************************************************************/

#ifdef EEPROM_ON
    
#define HAS_EEPROM          (1)

#if ( !(HAS_MITM) && !(BLE_SPOTA_RECEIVER) )
 #warning "Using EEPROM without MITM is useless!"
#endif

#else
#define HAS_EEPROM          (0)
#endif // EEPROM_ON

#if (HAS_MULTI_BOND)
 #if ( !(HAS_EEPROM) )
  #error "Multiple bonding support requires EEPROM!"
 #endif
#endif // (HAS_MULTI_BOND)

    
// Pin definition (irrelevant to whether EEPROM or LEDs are used or not since the HW setup is fixed!)
#define I2C_SDA_PORT        GPIO_PORT_0
#define I2C_SDA_PIN         GPIO_PIN_2
#define I2C_SCL_PORT        GPIO_PORT_0
#define I2C_SCL_PIN         GPIO_PIN_3

#define KBD_GREEN_LED_PORT  GPIO_PORT_1
#define KBD_GREEN_LED_PIN   GPIO_PIN_5

#define KBD_RED_LED_PORT    GPIO_PORT_1
#define KBD_RED_LED_PIN     GPIO_PIN_4

// Switches used to initialize I2C and LED pins properly (even when not used) to avoid leakage
#undef INIT_LED_PINS
#undef INIT_EEPROM_PINS
#define INIT_LED_PINS       1
#define INIT_EEPROM_PINS    1


#define DECLARE_EEPROM_PINS                                                 \
    {                                                                       \
        RESERVE_GPIO( EEPROM_SCL,   GPIO_PORT_0,  GPIO_PIN_3,  PID_I2C_SCL);\
        RESERVE_GPIO( EEPROM_SDA,   GPIO_PORT_0,  GPIO_PIN_2,  PID_I2C_SDA);\
    }

#define DECLARE_LED_PINS                                                    \
    {                                                                       \
        RESERVE_GPIO( GREEN_LED,    GPIO_PORT_1,  GPIO_PIN_5,  PID_GPIO);   \
        RESERVE_GPIO( RED_LED,      GPIO_PORT_1,  GPIO_PIN_4,  PID_GPIO);   \
    }

#ifndef CFG_PRINTF
#define DECLARE_UART_USED_PINS                                              \
    {                                                                       \
        RESERVE_GPIO( INPUT_COL_0,  GPIO_PORT_0,  GPIO_PIN_5,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_1,  GPIO_PORT_0,  GPIO_PIN_4,  PID_GPIO);   \
    }
    
#else
#define DECLARE_UART_USED_PINS    
#endif

#define DECLARE_KEYBOARD_GPIOS                                              \
    {                                                                       \
        RESERVE_GPIO( INPUT_COL_2,  GPIO_PORT_0,  GPIO_PIN_1,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_3,  GPIO_PORT_0,  GPIO_PIN_0,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_4,  GPIO_PORT_1,  GPIO_PIN_1,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_5,  GPIO_PORT_1,  GPIO_PIN_0,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_6,  GPIO_PORT_2,  GPIO_PIN_7,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_7,  GPIO_PORT_2,  GPIO_PIN_6,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_8,  GPIO_PORT_2,  GPIO_PIN_5,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_9,  GPIO_PORT_2,  GPIO_PIN_4,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_10, GPIO_PORT_3,  GPIO_PIN_7,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_11, GPIO_PORT_2,  GPIO_PIN_3,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_12, GPIO_PORT_3,  GPIO_PIN_6,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_13, GPIO_PORT_2,  GPIO_PIN_2,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_14, GPIO_PORT_2,  GPIO_PIN_1,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_15, GPIO_PORT_2,  GPIO_PIN_0,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_16, GPIO_PORT_3,  GPIO_PIN_1,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_17, GPIO_PORT_3,  GPIO_PIN_2,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_0, GPIO_PORT_3,  GPIO_PIN_4,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_1, GPIO_PORT_3,  GPIO_PIN_3,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_2, GPIO_PORT_3,  GPIO_PIN_5,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_3, GPIO_PORT_0,  GPIO_PIN_6,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_4, GPIO_PORT_2,  GPIO_PIN_9,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_5, GPIO_PORT_2,  GPIO_PIN_8,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_6, GPIO_PORT_0,  GPIO_PIN_7,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_7, GPIO_PORT_3,  GPIO_PIN_0,  PID_GPIO);   \
        DECLARE_EEPROM_PINS                                                 \
        DECLARE_LED_PINS                                                    \
        DECLARE_UART_USED_PINS                                              \
    }





#elif MATRIX_SETUP == 9
/*****************************************************************************************************************************************
 *
 * RC 12-keys with QFN48 (based on keyboard #3)
 *
 *****************************************************************************************************************************************/

#ifdef EEPROM_ON
    
#define HAS_EEPROM          (1)
    
#if ( !(HAS_MITM) && !(BLE_SPOTA_RECEIVER) )
 #warning "Using EEPROM without MITM is useless!"
#endif

#else
#define HAS_EEPROM          (0)
#endif // EEPROM_ON


#if (HAS_MULTI_BOND)
 #if ( !(HAS_EEPROM) )
  #error "Multiple bonding support requires EEPROM!"
 #endif
#endif // (HAS_MULTI_BOND)


// Pin definition (irrelevant to whether EEPROM or LEDs are used or not since the HW setup is fixed!)
#define I2C_SDA_PORT        GPIO_PORT_0
#define I2C_SDA_PIN         GPIO_PIN_6
#define I2C_SCL_PORT        GPIO_PORT_0
#define I2C_SCL_PIN         GPIO_PIN_7

#define KBD_GREEN_LED_PORT  GPIO_PORT_2
#define KBD_GREEN_LED_PIN   GPIO_PIN_8

#define KBD_RED_LED_PORT    GPIO_PORT_2
#define KBD_RED_LED_PIN     GPIO_PIN_5

// Switches used to initialize I2C and LED pins properly (even when not used) to avoid leakage
#undef INIT_LED_PINS
#undef INIT_EEPROM_PINS
#define INIT_LED_PINS       1
#define INIT_EEPROM_PINS    1


#define DECLARE_EEPROM_PINS                                                 \
    {                                                                       \
        RESERVE_GPIO( EEPROM_SCL,   GPIO_PORT_0,  GPIO_PIN_7,  PID_I2C_SCL);\
        RESERVE_GPIO( EEPROM_SDA,   GPIO_PORT_0,  GPIO_PIN_6,  PID_I2C_SDA);\
    }

#define DECLARE_LED_PINS                                                    \
    {                                                                       \
        RESERVE_GPIO( GREEN_LED,    GPIO_PORT_2,  GPIO_PIN_8,  PID_GPIO);   \
        RESERVE_GPIO( RED_LED,      GPIO_PORT_2,  GPIO_PIN_5,  PID_GPIO);   \
    }

#define DECLARE_KEYBOARD_GPIOS                                              \
    {                                                                       \
        RESERVE_GPIO( INPUT_COL_2,  GPIO_PORT_2,  GPIO_PIN_7,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_3,  GPIO_PORT_2,  GPIO_PIN_6,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_4,  GPIO_PORT_1,  GPIO_PIN_2,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_5,  GPIO_PORT_1,  GPIO_PIN_1,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_5, GPIO_PORT_3,  GPIO_PIN_3,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_6, GPIO_PORT_3,  GPIO_PIN_5,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_7, GPIO_PORT_3,  GPIO_PIN_7,  PID_GPIO);   \
        DECLARE_EEPROM_PINS                                                 \
        DECLARE_LED_PINS                                                    \
    }
    
        
        
        

#elif MATRIX_SETUP == 10
/*****************************************************************************************************************************************
 *
 * DK 2-keys with QFN48
 *
 *****************************************************************************************************************************************/

#define HAS_EEPROM          (0)
    
#if (HAS_MULTI_BOND)
#warning "This keyboard setup does not support EEPROM!"    
#endif

#define KBD_GREEN_LED_PORT  GPIO_PORT_0
#define KBD_GREEN_LED_PIN   GPIO_PIN_7

#define KBD_RED_LED_PORT    GPIO_PORT_1
#define KBD_RED_LED_PIN     GPIO_PIN_0

#undef INIT_LED_PINS
#define INIT_LED_PINS       1

#define INIT_EEPROM_PINS    0


#define DECLARE_LED_PINS                                                    \
    {                                                                       \
        RESERVE_GPIO( GREEN_LED,    GPIO_PORT_0,  GPIO_PIN_7,  PID_GPIO);   \
        RESERVE_GPIO( RED_LED,      GPIO_PORT_1,  GPIO_PIN_0,  PID_GPIO);   \
    }

#define DECLARE_KEYBOARD_GPIOS                                              \
    {                                                                       \
        RESERVE_GPIO( INPUT_COL_0,  GPIO_PORT_1,  GPIO_PIN_1,  PID_GPIO);   \
        RESERVE_GPIO( INPUT_COL_1,  GPIO_PORT_0,  GPIO_PIN_6,  PID_GPIO);   \
        RESERVE_GPIO( OUTPUT_ROW_0, GPIO_PORT_1,  GPIO_PIN_3,  PID_GPIO);   \
        DECLARE_LED_PINS                                                    \
    }

#endif // MATRIX_SETUP


// Dummy declarations to satisfy the compiler...
#if !(INIT_EEPROM_PINS)
#define I2C_SDA_PORT        GPIO_PORT_1
#define I2C_SDA_PIN         GPIO_PIN_8
#define I2C_SCL_PORT        GPIO_PORT_1
#define I2C_SCL_PIN         GPIO_PIN_9
#endif    

#if !(INIT_LED_PINS)
#define KBD_GREEN_LED_PORT  GPIO_PORT_1
#define KBD_GREEN_LED_PIN   GPIO_PIN_8

#define KBD_RED_LED_PORT    GPIO_PORT_1
#define KBD_RED_LED_PIN     GPIO_PIN_9
#endif

#endif // APP_KBD_KEY_MATRIX_H_
