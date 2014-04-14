/**
 ****************************************************************************************
 *
 * @file arch.h
 *
 * @brief This file contains the definitions of the macros and functions that are
 * architecture dependent.  The implementation of those is implemented in the
 * appropriate architecture directory.
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
 * $Rev:  $
 *
 ****************************************************************************************
 */


#ifndef _ARCH_H_
#define _ARCH_H_

/**
 ****************************************************************************************
 * @defgroup REFIP
 * @brief Reference IP Platform
 *
 * This module contains reference platform components - REFIP.
 *
 *
 * @{
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup DRIVERS
 * @ingroup REFIP
 * @brief Reference IP Platform Drivers
 *
 * This module contains the necessary drivers to run the platform with the
 * RW BT SW protocol stack.
 *
 * This has the declaration of the platform architecture API.
 *
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>        // standard integer definition
#include "compiler.h"      // inline functions
#include "stdbool.h"

/*
 * CPU WORD SIZE
 ****************************************************************************************
 */
/// ARM is a 32-bit CPU
#define CPU_WORD_SIZE   4

/*
 * CPU Endianness
 ****************************************************************************************
 */
/// ARM is little endian
#define CPU_LE          1

/*
 * DEBUG configuration
 ****************************************************************************************
 */
#if defined(CFG_DBG)
#define PLF_DEBUG          1
#else //CFG_DBG
#define PLF_DEBUG          0
#endif //CFG_DBG


typedef enum
{
	ARCH_SLEEP_OFF,
	ARCH_EXT_SLEEP_ON,
	ARCH_DEEP_SLEEP_ON,
} sleep_state_t;

/// Arch Sleep environment structure
struct arch_sleep_env_tag
{
 	sleep_state_t slp_state;
};

typedef enum
{
	mode_active = 0,
	mode_idle,
	mode_ext_sleep,
	mode_deep_sleep,
	mode_sleeping,
}sleep_mode_t;


#define	LP_CLK_XTAL32       0x00
#define LP_CLK_RCX20        0xAA
#define LP_CLK_FROM_OTP     0xFF

/*
 * NVDS
 ****************************************************************************************
 */

/// NVDS
#ifdef CFG_NVDS
#define PLF_NVDS             1
#else // CFG_NVDS
#define PLF_NVDS             0
#endif // CFG_NVDS

/*
 * LLD ROM defines
 ****************************************************************************************
 */
struct lld_sleep_env_tag
{
    uint32_t irq_mask;
};

/*
 * UART
 ****************************************************************************************
 */

/// UART
#define PLF_UART             1
#if !defined (CFG_APP) && !defined (CFG_PRF_ACCEL)
#define PROGRAM_ENABLE_UART
//#define PROGRAM_ALTERNATE_UART_PINS
//#define UART_MEGABIT
#elif defined (CFG_PRINTF)
#define PROGRAM_ENABLE_UART
#else
//#define PROGRAM_ENABLE_UART
//#define PROGRAM_ALTERNATE_UART_PINS
//#define UART_MEGABIT
#endif
/*
 * DEFINES
 ****************************************************************************************
 */

#if defined (CFG_EXT_SLEEP)
	#define EXT_SLEEP_ENABLED							1
#else
	#define EXT_SLEEP_ENABLED 							0
#endif /* CFG_EXT_SLEEP */

#if defined (CFG_DEEP_SLEEP)
	#if defined (CFG_EXT_SLEEP)
		#error "CFG_DEEP_SLEEP defined with CFG_EXT_SLEEP! Select one of them"
	#endif
	#define DEEP_SLEEP_ENABLED						    1
#else
	#define DEEP_SLEEP_ENABLED						    0
#endif

#if defined (CFG_DISABLE_RTS_CTS)
	#define DISABLE_UART_RTS_CTS					    1
#else
	#define DISABLE_UART_RTS_CTS 					    0
#endif /* CFG_DISABLE_RTS_CTS */

#if defined (CFG_LUT_PATCH)
	#define LUT_PATCH_ENABLED   					    1
#else
	#define LUT_PATCH_ENABLED   					    0
#endif /* CFG_LUT_PATCH */

#if BLE_APP_PRESENT
# if !defined(CFG_PRINTF)
#  undef PROGRAM_ENABLE_UART
# else
#  define PROGRAM_ENABLE_UART
# endif
#endif // BLE_APP_PRESENT
    
// #if  LUT_PATCH_ENABLED
// #define PROGRAM_ALTERNATE_UART_PINS                     1 // ES4 cannot use P0_0, P0_1, P0_2, P0_3. Alternate ports must be used for UART iface.
// #endif 
    
#define MGCKMODA_PATCH_ENABLED                          1

// Control the operational frequency of the BLE core
#define BLE_CORE_CLK_AT_8MHz		                    0 //0: 16MHz, 1: 8MHz 

//gpio remove
#if 0
	//Monitor Power Down through GPIOs
	// P0[5]: High when entering Deep Sleep, Low on exit
	#define DPSLP_SetBits16( a, b, c )	( SetBits16( (a), (b), (c) ) )
	#define DPSLP_SetWord16( a, b )		( SetWord16( (a), (b) ) )
#else
	#define DPSLP_SetBits16( a, b, c ) 	{}	
	#define DPSLP_SetWord16( a, b )		{}
#endif

/*
 * Duration from Wakeup to XTAL trim finish. Measured in low power clock cycles.
 ****************************************************************************************
 */
#define XTAL_TRIMMING_TIME			(151) 
#define XTAL_TRIMMING_TIME_RCX		(62) 

		
/*
 * DEEP SLEEP: Power down configuration
 ****************************************************************************************
 */
#define TWIRQ_RESET_VALUE		    (1)
#define TWIRQ_SET_VALUE			    (XTAL_TRIMMING_TIME) //min val = (TWIRQ_RESET_VALUE+1). FIXME
#define TWEXT_VALUE				    (TWIRQ_RESET_VALUE+1)


/*
 * Deep sleep threshold. Application specific. Control if during deep sleep the system RAM will be powered off and if OTP copy will be required.
 ****************************************************************************************
*/		
#if (DEEP_SLEEP)
/// Sleep Duration Value in periodic wake-up mode
#define MAX_SLEEP_DURATION_PERIODIC_WAKEUP      0x0320  // 0.5s
/// Sleep Duration Value in external wake-up mode
#define MAX_SLEEP_DURATION_EXTERNAL_WAKEUP      0x3E80  //10s
#endif //DEEP_SLEEP

#define		DEEP_SLEEP_THRESHOLD    800000
		
		
		
/*
 * JTAG configuration
 ****************************************************************************************
 */
//moved to da14580_config.h per project
//#define DEVELOPMENT__NO_OTP		1	//0: code at OTP, 1: code via JTAG
		
//Remove this check only when compiling for OTP
//Note that if DEVELOPMENT__NO_OTP is set, SYS_CTRL_REG:RET_SYSRAM will be set. Refer to the datasheet for details.		


/*
 * Fab Calibration configuration
 ****************************************************************************************
 */
#ifdef CFG_CALIBRATED_AT_FAB
#define UNCALIBRATED_AT_FAB		0	//0: OTP header has calibration values, 1: OTP header is empty
#else
#define UNCALIBRATED_AT_FAB		1	//0: OTP header has calibration values, 1: OTP header is empty
#endif

/// Possible errors detected by FW
#define    RESET_NO_ERROR         0x00000000
#define    RESET_MEM_ALLOC_FAIL   0xF2F2F2F2

/// Reset platform and stay in ROM
#define    RESET_TO_ROM           0xA5A5A5A5
/// Reset platform and reload FW
#define    RESET_AND_LOAD_FW      0xC3C3C3C3

/*
 * EXPORTED FUNCTION DECLARATION
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Compute size of SW stack used.
 *
 * This function is compute the maximum size stack used by SW.
 *
 * @return Size of stack used (in bytes)
 ****************************************************************************************
 */
uint16_t get_stack_usage(void);

/**
 ****************************************************************************************
 * @brief Re-boot FW.
 *
 * This function is used to re-boot the FW when error has been detected, it is the end of
 * the current FW execution.
 * After waiting transfers on UART to be finished, and storing the information that
 * FW has re-booted by itself in a non-loaded area, the FW restart by branching at FW
 * entry point.
 *
 * Note: when calling this function, the code after it will not be executed.
 *
 * @param[in] error      Error detected by FW
 ****************************************************************************************
 */
void platform_reset(uint32_t error);

#if PLF_DEBUG
/**
 ****************************************************************************************
 * @brief Print the assertion error reason and loop forever.
 *
 * @param condition C string containing the condition.
 * @param file C string containing file where the assertion is located.
 * @param line Line number in the file where the assertion is located.
 ****************************************************************************************
 */
void assert_err(const char *condition, const char * file, int line);

/**
 ****************************************************************************************
 * @brief Print the assertion error reason and loop forever.
 * The parameter value that is causing the assertion will also be disclosed.
 *
 * @param param parameter value that is caused the assertion.
 * @param file C string containing file where the assertion is located.
 * @param line Line number in the file where the assertion is located.
 ****************************************************************************************
 */
void assert_param(int param0, int param1, const char * file, int line);

/**
 ****************************************************************************************
 * @brief Print the assertion warning reason.
 *
 * @param condition C string containing the condition.
 * @param file C string containing file where the assertion is located.
 * @param line Line number in the file where the assertion is located.
 ****************************************************************************************
 */
void assert_warn(const char *condition, const char * file, int line);
#endif //PLF_DEBUG


void set_pxact_gpio(void);

void patch_func(void); 

void calibrate_rcx20(uint16_t cal_time);

void read_rcx_freq(uint16_t cal_time);

uint32_t lld_sleep_lpcycles_2_us_sel_func(uint32_t lpcycles);

uint32_t lld_sleep_us_2_lpcycles_sel_func(uint32_t us);

bool check_gtl_state(void);

/*
 * ASSERTION CHECK
 ****************************************************************************************
 */
#if PLF_DEBUG
/// Assertions showing a critical error that could require a full system reset
#define ASSERT_ERR(cond)                              \
    do {                                              \
        if (!(cond)) {                                \
            assert_err(#cond, __MODULE__, __LINE__);  \
        }                                             \
    } while(0)

/// Assertions showing a critical error that could require a full system reset
#define ASSERT_INFO(cond, param0, param1)             \
    do {                                              \
        if (!(cond)) {                                \
            assert_param((int)param0, (int)param1, __MODULE__, __LINE__);  \
        }                                             \
    } while(0)

/// Assertions showing a non-critical problem that has to be fixed by the SW
#define ASSERT_WARN(cond)                             \
    do {                                              \
        if (!(cond)) {                                \
            assert_warn(#cond, __MODULE__, __LINE__); \
        }                                             \
    } while(0)

#else
/// Assertions showing a critical error that could require a full system reset
#define ASSERT_ERR(cond)

/// Assertions showing a critical error that could require a full system reset
#define ASSERT_INFO(cond, param0, param1)

/// Assertions showing a non-critical problem that has to be fixed by the SW
#define ASSERT_WARN(cond)

#endif //PLF_DEBUG

extern const uint32_t * const jump_table_base[88];
#define jump_table_struct (uint32_t)jump_table_base

/// Object allocated in shared memory - check linker script
#define __SHARED __attribute__ ((section("shram")))

// required to define GLOBAL_INT_** macros as inline assembly. This file is included after
// definition of ASSERT macros as they are used inside ll.h
#include "ll.h"     // ll definitions
/// @} DRIVERS
#endif // _ARCH_H_
