/**
 ****************************************************************************************
 *
 * @file i2c_eeprom.h
 *
 * @brief eeprom driver over i2c interface header file.
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

#include "arch.h"
#include "rwip_config.h"

void i2c_init(void);

void i2c_release(void);

int8_t random_read_i2c_eeprom(uint8_t address);

uint16_t sequential_read_i2c_eeprom(uint8_t * rd_data_ptr , uint8_t address , uint16_t size);

void write_byte_i2c_eeprom(uint8_t address , uint8_t wr_data);

void write_page_i2c_eeprom(uint8_t address , uint8_t* wr_data_ptr , uint8_t size);
