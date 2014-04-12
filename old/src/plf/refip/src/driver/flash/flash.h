/**
 ****************************************************************************************
 *
 * @file flash.h
 *
 * @brief Flash driver interface
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
 * $Rev: $
 *
 ****************************************************************************************
 */

#ifndef FLASH_H_
#define FLASH_H_

/**
 ****************************************************************************************
 * @addtogroup FLASH
 * @ingroup DRIVERS
 *
 * @brief Flash memory driver
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>      // standard integer definition

/*
 * DEFINES
 ****************************************************************************************
 */

///Flash type code used to select the correct erasing and programming algorithm
#define FLASH_TYPE_UNKNOWN             0
#define FLASH_TYPE_INTEL_28F320C3      1
#define FLASH_TYPE_INTEL_28F800C3      2
#define FLASH_TYPE_NUMONYX_M25P128     3

///Base address of Flash on system bus
#define FLASH_BASE_ADDR          0x03000000

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize flash driver.
 ****************************************************************************************
 */
void flash_init(void);

/**
 ****************************************************************************************
 * @brief Identify the flash.
 * @return Flash ID
 ****************************************************************************************
 */
uint8_t flash_identify(void);

/**
 ****************************************************************************************
 * @brief Remove content of the Flash.
 *
 * @param[in] FlashType    Type of the Flash
 * @param[in] StartOffset  Offset
 * @param[in] Size         Length
 *
 * @return Flash type or 0 if the FLASH not recognized.
 ****************************************************************************************
 */
uint8_t flash_erase(uint8_t FlashType, uint32_t StartOffset, uint32_t Size);

/**
 ****************************************************************************************
 * @brief Write to Flash.
 *
 * @param[in] FlashType    Type of the Flash
 * @param[in] StartOffset  Offset
 * @param[in] Length       Size of the bin to write
 * @param[in] Buffer       Pointer to the bin
 *
 * @return status
 ****************************************************************************************
 */
uint8_t flash_write(uint8_t FlashType, uint32_t Offset, uint32_t Length, uint8_t *Buffer);

/**
 ****************************************************************************************
 * @brief   Read a flash section.
 *
 * @param   flash_type     Flash type
 * @param   offset         Starting offset from the beginning of the flash device
 * @param   length         Size of the portion of flash to read
 * @param   buffer         Pointer on data to read
 *
 * @return  status         0 if operation successful, otherwise, failure
 ****************************************************************************************
 */
uint8_t flash_read(uint8_t flash_type, uint32_t offset, uint32_t length, uint8_t *buffer);


/// @} FLASH

#endif // FLASH_H_
