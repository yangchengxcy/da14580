/**
 ****************************************************************************************
 *
 * @file app_spotar.h
 *
 * @brief SPOTA Receiver Application header file
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
 * @brief SPOTA Receiver Application header file.
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
#include "spi.h"
#include "spotar.h"
#include "spotar_task.h"

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
 
// FLAGS
#define SPOTAR_READ_MEM_DEV_TYPE 0xFF000000
#define SPOTAR_READ_MEM_BASE_ADD 0x00FFFFFF
#define SPOTAR_READ_NUM_OF_PATCHES 0xFFFF0000
#define SPOTAR_READ_MEM_PATCH_SIZE 0x0000FFFF

// Holds the retainable veriables of SPOTAR app
typedef struct
{
    uint8_t     mem_dev;
    uint32_t    mem_base_add;
    uint32_t    gpio_map;
    uint32_t    new_patch_len;
    uint8_t     spota_pd_idx;
    uint8       sleep_mode;

}app_spota_state;
 
// Defines the SPI GPIO type
typedef struct
{
	SPI_Pad_t cs;
	SPI_Pad_t mosi;
	SPI_Pad_t miso;
	SPI_Pad_t clk;
}spi_gpio_config_t;

// Defines the i2c GPIO type
typedef struct
{
	SPI_Pad_t scl;
	SPI_Pad_t sda;
    uint32_t  slave_addr;    
}i2c_gpio_config_t;


// Physical memory device to write the patch data
enum
{
    SPOTAR_MEM_INT_SYSRAM = 0x00,
    SPOTAR_MEM_INT_RETRAM = 0x01,
    SPOTAR_MEM_I2C_EEPROM = 0x02, 
    SPOTAR_MEM_SPI_FLASH  = 0x03,
    SPOTAR_MEM_INVAL_DEV  = 0x04,
    // When initiator selects 0xff, it wants to exit SPOTAR service.
    // This is used in case of unexplained failures. If SPOTAR process 
    // finishes correctly it will exit automatically.
    SPOTAR_MEM_SERVICE_EXIT   = 0xFF,    
};

/*
 * EXTERNAL DEFINITIONS
 ****************************************************************************************
 */

extern app_spota_state spota_state;
extern uint8_t spota_new_pd[SPOTA_NEW_PD_SIZE];
extern uint8_t spota_all_pd[SPOTA_OVERALL_PD_SIZE];


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initializes SPOTAR Apllication.
 *
 * @param[in]   void
 *
 * @return      void
 ****************************************************************************************
 */
void app_spotar_init(void);

/**
 ****************************************************************************************
 * @brief Resets SPOTAR Apllication.
 *
 * @param[in]   void
 *
 * @return      void
 ****************************************************************************************
 */
void app_spotar_reset(void);

/**
 ****************************************************************************************
 * @brief Creates SPOTAR service database.
 *
 * @param[in]   void
 *
 * @return      void
 ****************************************************************************************
 */
void app_spotar_create_db(void);

/**
 ****************************************************************************************
 * @brief Inializes applacition and enables SPOTAR profile.
 *
 * @param[in]   void
 *
 * @return      void
 ****************************************************************************************
 */
void app_spotar_enable(void);

/**
 ****************************************************************************************
 * @brief Reads memory device and writes memory info.
 *
 * @param[in]   MSbyte holds the Memory device type, rest is the base address.
 * @param[in]   16MSbits hold number of patches, 16LSbits hold overall mem len.
 *
 * @return      void
 ****************************************************************************************
 */
void app_spotar_read_mem(uint32_t mem_dev_type, uint32_t* mem_info);

/**
 ****************************************************************************************
 * @brief Starts SPOTAR serivce and disables sleep.
 *
 * @param[in]   void
 *
 * @return      void
 ****************************************************************************************
 */
void app_spotar_start(void);

/**
 ****************************************************************************************
 * @brief Stops SPOTAR service and resets application
 *
 * @param[in]   void
 *
 * @return      void
 ****************************************************************************************
 */
void app_spotar_stop(void);

/**
 ****************************************************************************************
 * @brief Handles patch execution. Should be called at system start up and after deep sleep.
 *
 * @param[in]   void
 *
 * @return      void
 *
 ****************************************************************************************
 */
void app_spotar_exec_patch(void);

/**
 ****************************************************************************************
 * @brief Updates SPOTAR status characteristic.
 *
 * @param[in]   SPOTAR application status.
 *
 * @return      void
 ****************************************************************************************
 */
void spotar_send_status_update_req( uint8_t status);

/**
 ****************************************************************************************
 * @brief Updates SPOTAR memory info characteristic.
 *
 * @param[in]   Patch memory info. Number of patches and overall patch length.
 *
 * @return      void.
 ****************************************************************************************
 */
void spotar_send_mem_info_update_req( uint32_t mem_info);

/**
 ****************************************************************************************
 * @brief SPOTA data handler. Validates patch and stores patch data to memory device.
 *
 * @param[in]   void
 *
 * @return      void
 *
 ****************************************************************************************
 */
void app_spotar_pd_hdlr(void);

/*
 * ROM FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles patch execution
 *
 * @param[in]   defines the physical memory and patch base address
 * @param[in]   defines the mapping of the interface signals to IO pins
 * @param[in]   Points to the intermediate buffer address which will be used to temporary store the
 *              patch payload fetched from external Non Volatile memories. Only valid if I2C or SPI
 * @param[in]   patch_length: defines the size of the patch to be applied (in 32bit words)
 *
 * @return      void
 *
 ****************************************************************************************
 */
void exec_patching_spota(WORD mem_dev, WORD gpio_map,WORD* ptr,WORD patch_length);

/**
 ****************************************************************************************
 * @brief Returns the stored number of patches and the overall patch length 
 *
 * @param[in]   defines the physical memory and patch base address
 * @param[in]   defines the mapping of the interface signals to IO pins
 *
 * @return      The number and size (in 32-bit words) of the exising patch area 
 *
 ****************************************************************************************
 */
uint32_t get_patching_spota_length(uint32_t mem_dev, uint32_t gpio_map);

  
#endif //BLE_SPOTA_RECEIVER

/// @} APP

#endif // APP_H_
