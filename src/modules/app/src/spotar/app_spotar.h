/**
 ****************************************************************************************
 *
 * @file app_spotar.h
 *
 * @brief SPOTA Receiver Application entry point
 *
 * Copyright (C) 2013. Dialog Semiconductor Ltd, unpublished work. This computer 
 * program includes Confidential, Proprietary Information and is a Trade Secret of 
 * Dialog Semiconductor Ltd.  All use, disclosure, and/or reproduction is prohibited 
 * unless authorized in writing. All Rights Reserved.
 *
 * <bluetooth.support@diasemi.com> and contributors.
 *
 ****************************************************************************************
 */

#ifndef APP_SPOTAR_H_
#define APP_SPOTAR_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief SPOTA Receiver Application entry point.
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwble_config.h"

#if BLE_SPOTA_RECEIVER

#include <stdint.h>          // standard integer definition
#include <co_bt.h>

#include "spotar.h"

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
 
// FLAGS
#define SPOTAR_READ_MEM_DEV_TYPE 0xFF000000
#define SPOTAR_READ_MEM_BASE_ADD 0x00FFFFFF
#define SPOTAR_READ_NUM_OF_PATCHES 0xFFFF0000
#define SPOTAR_READ_MEM_PATCH_SIZE 0x0000FFFF

// The smallest page size for i2c memory
#define SPOTAR_I2C_PAGE_SIZE 16

typedef struct
{
    uint8_t     mem_dev;
    uint32_t    mem_base_add;
    uint32_t    gpio_map;
    uint32_t    new_patch_len;
    uint8_t     spota_pd_idx;
    uint8       sleep_mode;

}app_spota_state;

extern app_spota_state spota_state;
extern uint8_t spota_new_pd[SPOTA_NEW_PD_SIZE];
extern uint8_t spota_all_pd[SPOTA_OVERALL_PD_SIZE];


// Physical memory device to write the patchdata
enum
{
    SPOTAR_MEM_INT_SYSRAM = 0x00,
    SPOTAR_MEM_INT_RETRAM = 0x01,
    SPOTAR_MEM_I2C_EEPROM = 0x02, 
    SPOTAR_MEM_SPI_FLASH  = 0x03,
    SPOTAR_MEM_INVAL_DEV  = 0x04,
    // When initiator selects 0xff, it wants to exit SPOTAR service.
    // This is used in case of unexplained failures. If spotar process 
    // finishes correctly it will exit automatically.
    SPOTAR_MEM_SERVICE_EXIT   = 0xFF,  
  
};

/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize SPOTA Receiver Application
 ****************************************************************************************
 */
void app_spotar_init(void);

/**
 ****************************************************************************************
 * @brief Reset SPOTA Receiver Application
 ****************************************************************************************
 */
void app_spotar_reset(void);

/**
 ****************************************************************************************
 *
 * SPOTA Receiver Application Functions
 *
 ****************************************************************************************
 */

void app_spotar_create_db(void);

/**
 ****************************************************************************************
 * @brief Enable the SPOTA profile
 *
 ****************************************************************************************
 */
void app_spotar_enable(void);

/**
 ****************************************************************************************
 * @brief Read Memory Device and return mem info
 *
 ****************************************************************************************
 */
void app_spotar_read_mem(uint32_t mem_dev_type, uint32_t* mem_info);

/**
 ****************************************************************************************
 * @brief Start SPOTAR
 *
 ****************************************************************************************
 */
void app_spotar_start(void);

/**
 ****************************************************************************************
 * @brief Stop SPOTAR
 *
 ****************************************************************************************
 */
void app_spotar_stop(void);

/**
 ****************************************************************************************
 * @brief SPOTAR execute patch
 *
 ****************************************************************************************
 */
void app_spotar_exec_patch(void);

void spotar_send_status_update_req( uint8_t status);
void spotar_send_mem_info_update_req( uint32_t mem_info);
void app_spotar_pd_hdlr(void);

/*
 * External Memory access functions
 */

// Common functions
void exec_patching_spota(WORD mem_dev, WORD gpio_map,WORD* ptr,WORD patch_length);
uint32_t get_patching_spota_length(uint32_t mem_dev, uint32_t gpio_map);

// SPI functions
void spi_master_init(void);
char spi_serial_flash_access(char wr_data);
short read_spi_flash_status(void);
void write_spi_flash_page(WORD page_address, short offset_in_page, unsigned char* ptr, WORD size);
WORD read_spi_patch_table_length(WORD mem_dev, WORD gpio_map);
void spi_init_sporta(WORD gpio_map);
void spi_close_spota(WORD gpio_map);
void spota_spi_open_cs(char spics_location);
void spota_spi_close_cs(char spics_location);
void spi_mem_erase(void);

// i2c functions
void i2c_master_init(void);
void write_i2c_eeprom(short address,unsigned char * ptr, short size);
uint32_t read_i2c_patch_table_length(uint32_t mem_dev, uint32_t gpio_map);
void i2c_init_spota(uint32_t gpio_map);
void i2c_close_spota (uint32_t gpio_map);
    
#endif //BLE_SPOTA_RECEIVER

/// @} APP

#endif // APP_H_
