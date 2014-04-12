/**
 ****************************************************************************************
 *
 * @file app_spotar.c
 *
 * @brief SPOTA Reporter Application entry point
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

/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwble_config.h"

#if (BLE_APP_PRESENT)
#if (BLE_SPOTA_RECEIVER)

#include "app.h"                     // application definitions
#include "app_task.h"                // application task definitions
#include "spotar_task.h"             // SPOTA functions
#include "app_spotar.h" 
#include "gpio.h"
#include "spi.h"
#include "spi_flash.h"
#include "arch_sleep.h"
#include "periph_setup.h"

#if (BLE_APP_KEYBOARD)    
#include "app_kbd_fsm.h"
#endif

//application SPOTA state structure
app_spota_state spota_state __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY

__align(4) uint8_t spota_new_pd[SPOTA_NEW_PD_SIZE] __attribute__((section("spotar_patch_area"),zero_init));  // word aligned buffer to store new patch from initiator
__align(4) uint8_t spota_all_pd[SPOTA_OVERALL_PD_SIZE] __attribute__((section("spotar_patch_area"),zero_init)); // word aligned buffer to read the patch to before patch execute 

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
*/

void app_spotar_spi_config(spi_gpio_config_t *spi_conf);
void app_spotar_i2c_config(i2c_gpio_config_t *i2c_conf);
 
 /**
 ****************************************************************************************
 * SPOTAR Application Functions
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
void app_spotar_init(void)
{	
    spota_state.spota_pd_idx = 0;
    spota_state.new_patch_len = 0;
    memset( spota_all_pd, 0x00, 4); // Set first WORD to 0x00
    spota_state.mem_dev = SPOTAR_MEM_INVAL_DEV;
}

/**
 ****************************************************************************************
 * @brief Resets SPOTAR Apllication.
 *
 * @param[in]   void
 *
 * @return      void
 ****************************************************************************************
 */
void app_spotar_reset(void)
{	
    spota_state.spota_pd_idx = 0;
    spota_state.new_patch_len = 0;
    spotar_env.pd_flag = false;
}

/**
 ****************************************************************************************
 * @brief Creates SPOTAR service database.
 *
 * @param[in]   void
 *
 * @return      void
 ****************************************************************************************
 */
void app_spotar_create_db(void)
{
    // Add SPOTAR in the database
    struct spotar_create_db_req * req = KE_MSG_ALLOC(SPOTAR_CREATE_DB_REQ, TASK_SPOTAR,
                                                        TASK_APP, spotar_create_db_req);

    req->features = 0;

    // Send the message
    ke_msg_send(req);
}

/**
 ****************************************************************************************
 * @brief Inializes applacition and enables SPOTAR profile.
 *
 * @param[in]   void
 *
 * @return      void
 ****************************************************************************************
 */
void app_spotar_enable(void)
{
		
    // Allocate the message
    struct spotar_enable_req * req = KE_MSG_ALLOC(SPOTAR_ENABLE_REQ, TASK_SPOTAR, TASK_APP,
                                                 spotar_enable_req);

    // Fill in the parameter structure
    req->conhdl = app_env.conhdl;
    req->sec_lvl = PERM(SVC, ENABLE);
    req->mem_dev = 0; // No Mem device specified yet.
    req->patch_mem_info = 0;  // SPOTA initiator needs to specify mem device first
    
    // Send the message
    ke_msg_send(req);
}

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

void app_spotar_read_mem(uint32_t mem_dev, uint32_t* mem_info)
{  
    // store memory device type and base address
    spota_state.mem_dev = (uint8_t) ((mem_dev & SPOTAR_READ_MEM_DEV_TYPE) >> 24);
    spota_state.mem_base_add = mem_dev & SPOTAR_READ_MEM_BASE_ADD;
    
    // Validate memory and base address values
    if( spota_state.mem_dev < SPOTAR_MEM_INVAL_DEV )
    {       
        // read memory device and return memory info        
        if( (spota_state.mem_dev == SPOTAR_MEM_INT_SYSRAM) || 
            (spota_state.mem_dev == SPOTAR_MEM_INT_RETRAM) )
        {
            // SYSRAM patch base address is the addres of the last index of the spota_all_pd array 
            spota_state.mem_base_add = (uint32_t)(spota_all_pd + (SPOTA_OVERALL_PD_SIZE - sizeof(uint32_t)));
            *mem_info = get_patching_spota_length( (spota_state.mem_base_add & SPOTAR_READ_MEM_BASE_ADD), 0);
        }
        else
        {
            // At this point, no patch info for external mem devices
            *mem_info = 0;
        }
        
        // Valid memory device. Spotar service started
        spotar_send_status_update_req((uint8_t) SPOTAR_SRV_STARTED);
        app_spotar_start();
    }
    else if( spota_state.mem_dev == SPOTAR_MEM_SERVICE_EXIT )
    {
        app_spotar_stop();
        app_spotar_reset();

        // Initiator requested to exit service. Send notification to initiator
        spotar_send_status_update_req((uint8_t) SPOTAR_SRV_EXIT);            
    }
    else
    {
        spotar_send_status_update_req((uint8_t) SPOTAR_INVAL_MEM_TYPE);
        spota_state.mem_dev = SPOTAR_MEM_INVAL_DEV;
        *mem_info = 0;
    }
}

/**
 ****************************************************************************************
 * @brief Starts SPOTAR serivce and disables sleep.
 *
 * @param[in]   void
 *
 * @return      void
 ****************************************************************************************
 */

void app_spotar_start(void)
{	        
    spota_state.sleep_mode = app_get_sleep_mode();  // store sleep mode before entering SPOTAR
    //app_disable_sleep();    // Disable sleep when SPOTAR starts	
#if (BLE_APP_KEYBOARD)    
    app_state_update(SPOTAR_START_EVT);
#endif    
}

/**
 ****************************************************************************************
 * @brief Stops SPOTAR service and resets application
 *
 * @param[in]   void
 *
 * @return      void
 ****************************************************************************************
 */

void app_spotar_stop(void)
{

    if( spota_state.mem_dev == SPOTAR_MEM_I2C_EEPROM ){
        i2c_eeprom_release();
    }
    
    if( spota_state.mem_dev == SPOTAR_MEM_SPI_FLASH ) {
        spi_release();
    }
       
    // Set memory device to invalid type so that service will not 
    // start until the memory device is explicitly set upon service start
    spota_state.mem_dev = SPOTAR_MEM_INVAL_DEV;
    
#if (BLE_APP_KEYBOARD)    
    app_state_update(SPOTAR_END_EVT);
#endif
}

/**
 ****************************************************************************************
 * @brief Updates SPOTAR status characteristic.
 *
 * @param[in]   SPOTAR application status.
 *
 * @return      void
 ****************************************************************************************
 */
void spotar_send_status_update_req( uint8_t status )
{   

    // Inform SPOTAR task. 
    struct spotar_status_upadet_req *req = KE_MSG_ALLOC(SPOTAR_STATUS_UPDATE_REQ,
                      TASK_SPOTAR, spotar_env.con_info.appid,
                      spotar_status_upadet_req);

    req->conhdl = app_env.conhdl;
    req->status = status;

    // Send the message
    ke_msg_send(req);
}

/**
 ****************************************************************************************
 * @brief Updates SPOTAR memory info characteristic.
 *
 * @param[in]   Patch memory info. Number of patches and overall patch length.
 *
 * @return      void.
 ****************************************************************************************
 */
void spotar_send_mem_info_update_req( uint32_t mem_info)
{   

    // Inform SPOTAR task. 
    struct spotar_patch_mem_info_upadet_req *req = KE_MSG_ALLOC(SPOTAR_PATCH_MEM_INFO_UPDATE_REQ,
                      TASK_SPOTAR, spotar_env.con_info.appid,
                      spotar_patch_mem_info_upadet_req);

    req->mem_info = mem_info;

    // Send the message
    ke_msg_send(req);
}

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
void app_spotar_pd_hdlr(void)
{
    
    uint32_t mem_info;
    uint32_t mem_dev;
    uint16_t overall_len_in_bytes;
    uint16_t newpatch_copy_pos;
    uint16_t status = SPOTAR_CMP_OK;
    spi_gpio_config_t spi_conf;
    i2c_gpio_config_t i2c_conf;
    uint32_t ret;
    
    // SPOTAR_TODO. Calculate CRC
    
    // Check mem dev. 
    switch (spota_state.mem_dev)
    {            
        case SPOTAR_MEM_INT_SYSRAM:
        case SPOTAR_MEM_INT_RETRAM:
            mem_dev = (uint32_t)(spota_all_pd + (SPOTA_OVERALL_PD_SIZE - sizeof(uint32_t)));
            mem_info = get_patching_spota_length( (mem_dev & SPOTAR_READ_MEM_BASE_ADD), 0);
            overall_len_in_bytes = (mem_info & SPOTAR_READ_MEM_PATCH_SIZE) << 2;
        
            if( overall_len_in_bytes + spota_state.spota_pd_idx >  SPOTA_OVERALL_PD_SIZE){
                // New patch len + existing patch len is larger than the SYSRAM buffer !!
                status = SPOTAR_INT_MEM_ERR;
                break;
            }
                        
            // Note that in order to keep the same implementation as patching in OTP, the first patch        
            // header starts at the end of the allocated SYSRAM or RETRAM memory bank.
            
            // Counting from the end of the memory bank, write new patch starting at the location 
            // where the existing patch(es) end, plus the size of the new patch.
            // Note that patch data are copied in words.
            {
                uint32_t *ptr_up, *ptr_down;
                
                newpatch_copy_pos = (SPOTA_OVERALL_PD_SIZE - overall_len_in_bytes) - spota_state.spota_pd_idx;
                ptr_down  = (uint32_t *) &spota_new_pd[spota_state.spota_pd_idx - sizeof(uint32_t)];
                ptr_up  = (uint32_t *) &spota_all_pd[newpatch_copy_pos];
                while(spota_state.spota_pd_idx>0){
                        *ptr_up = *ptr_down;                        
                        ptr_up++;
                        ptr_down--;
                        spota_state.spota_pd_idx -= sizeof(uint32_t);
                }
            }
    
            mem_info = get_patching_spota_length( (mem_dev & SPOTAR_READ_MEM_BASE_ADD), 0);
            spotar_send_mem_info_update_req(mem_info);

            if( spota_state.mem_dev == SPOTAR_MEM_INT_SYSRAM )
            {
                // Apply patch if SYSRAM has been selected. Can not reset when patch is stored in SYSRAM.
                if( (mem_info & 0xffff) > 0)
                {
                    exec_patching_spota((mem_dev & SPOTAR_READ_MEM_BASE_ADD), 0x0,(WORD *) spota_all_pd, (mem_info & SPOTAR_READ_MEM_PATCH_SIZE) );
                }
            }
            break;
        case SPOTAR_MEM_I2C_EEPROM:
            mem_info = get_patching_spota_length( ((spota_state.mem_dev << 24) | spota_state.mem_base_add), spota_state.gpio_map);
            overall_len_in_bytes = (mem_info & SPOTAR_READ_MEM_PATCH_SIZE) << 2;                                                          
            if( overall_len_in_bytes + spota_state.spota_pd_idx >  SPOTA_OVERALL_PD_SIZE){
                // New patch len + existing patch len is larger than the SYSRAM buffer !!
                status = SPOTAR_INT_MEM_ERR;
                break;
            }
            app_spotar_i2c_config(&i2c_conf);
            i2c_eeprom_init(i2c_conf.slave_addr, I2C_SPEED_MODE, I2C_ADDRESS_MODE, I2C_2BYTES_ADDR);            
            ret = i2c_eeprom_write_data (spota_new_pd, (spota_state.mem_base_add + overall_len_in_bytes), spota_state.spota_pd_idx);
            if( ret !=  spota_state.spota_pd_idx){
                status = SPOTAR_EXT_MEM_ERR;
            }
            i2c_wait_until_eeprom_ready();
            mem_info = get_patching_spota_length( ((spota_state.mem_dev << 24) | spota_state.mem_base_add), spota_state.gpio_map);
            spotar_send_mem_info_update_req(mem_info);
            
            break;
        case SPOTAR_MEM_SPI_FLASH:                                  
            mem_info = get_patching_spota_length( ((spota_state.mem_dev << 24) | spota_state.mem_base_add), spota_state.gpio_map);
            overall_len_in_bytes = (mem_info & SPOTAR_READ_MEM_PATCH_SIZE) << 2;     
            if( overall_len_in_bytes + spota_state.spota_pd_idx >  SPOTA_OVERALL_PD_SIZE){
                // New patch len + existing patch len is larger than the SYSRAM buffer !!
                status = SPOTAR_INT_MEM_ERR;
                break;
            }
            app_spotar_spi_config(&spi_conf);             
            spi_init(&spi_conf.cs, SPI_MODE_16BIT, SPI_ROLE_MASTER, SPI_CLK_IDLE_POL_LOW,	SPI_PHA_MODE_0, SPI_MINT_DISABLE, SPI_XTAL_DIV_8);
            spi_flash_init(SPI_FLASH_SIZE, SPI_FLASH_PAGE_SIZE );                                
            ret = spi_flash_write_data (spota_new_pd, (spota_state.mem_base_add + overall_len_in_bytes), spota_state.spota_pd_idx);
            if( ret !=  spota_state.spota_pd_idx){
                status = SPOTAR_EXT_MEM_ERR;
            }                    
            mem_info = get_patching_spota_length( ((spota_state.mem_dev << 24) | spota_state.mem_base_add), spota_state.gpio_map);            
            spotar_send_mem_info_update_req(mem_info);                    
            break;
        default:
            status = SPOTAR_INVAL_MEM_TYPE;
            break;
    }
             
    if( status == SPOTAR_CMP_OK )
    {
        app_spotar_stop();
        app_spotar_reset();
    }
    
    // SPOTA finished successfully. Send Indication to initiator
    spotar_send_status_update_req((uint8_t) status);
}
    
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
void app_spotar_exec_patch(void)
{
    // Example execute patch function. Executes patch from SPI
    uint32_t  len;
    
    // read patch len from spi flash
    len = get_patching_spota_length(0x03002800, 0x05060300 /*0x25262320*/);

    // read patch len from i2c eeprom
    //len = get_patching_spota_length( 0x02000800, 0x00500203 /*0x00500203*/);
    if( (len & SPOTAR_READ_MEM_PATCH_SIZE) > 0)
    {
        // execute patch from spi flash
        exec_patching_spota(0x03002800, 0x05060300 /*0x25262320*/ ,(WORD *) spota_all_pd , (len & 0xffff) );
        // execute patch from i2c eeprom
        //exec_patching_spota(0x02000800, 0x00500203 /*0x00502223*/ ,(WORD *) spota_all_pd , (len & 0xffff) );
    }
}

/**
 ****************************************************************************************
 * @brief Reserves SPI GPIO pins, reads the GPIO map set by initiator, and configures the SPI pins.
 *
 * @param[in]   Pointer to port/pin pad structure.
 *
 * @return      void.
 *
 ****************************************************************************************
 */
void app_spotar_spi_config(spi_gpio_config_t *spi_conf)
{
    spi_conf->clk.port      = ((GPIO_PORT)((spota_state.gpio_map & 0x000000f0) >>  4));
    spi_conf->clk.pin       = ((GPIO_PIN) ((spota_state.gpio_map & 0x0000000f)));
    spi_conf->cs.port       = ((GPIO_PORT)((spota_state.gpio_map & 0x0000f000) >> 12));
    spi_conf->cs.pin        = ((GPIO_PIN) ((spota_state.gpio_map & 0x00000f00) >> 8));
    spi_conf->mosi.port     = ((GPIO_PORT)((spota_state.gpio_map & 0x00f00000) >> 20));
    spi_conf->mosi.pin      = ((GPIO_PIN) ((spota_state.gpio_map & 0x000f0000) >> 16));
    spi_conf->miso.port     = ((GPIO_PORT)((spota_state.gpio_map & 0xf0000000) >> 28));
    spi_conf->miso.pin      = ((GPIO_PIN) ((spota_state.gpio_map & 0x0f000000) >> 24));


    RESERVE_GPIO( SPI_CLK, spi_conf->clk.port, spi_conf->clk.pin, PID_SPI_CLK);
    RESERVE_GPIO( SPI_DO, spi_conf->mosi.port, spi_conf->mosi.pin, PID_SPI_DO);
    RESERVE_GPIO( SPI_DI, spi_conf->miso.port, spi_conf->miso.pin, PID_SPI_DI);
    RESERVE_GPIO( SPI_EN, spi_conf->cs.port, spi_conf->cs.pin, PID_SPI_EN);
    
    GPIO_ConfigurePin( spi_conf->cs.port, spi_conf->cs.pin, OUTPUT, PID_SPI_EN, true );
    GPIO_ConfigurePin( spi_conf->clk.port, spi_conf->clk.pin, OUTPUT, PID_SPI_CLK, false );
    GPIO_ConfigurePin( spi_conf->mosi.port, spi_conf->mosi.pin, OUTPUT, PID_SPI_DO, false );	
    GPIO_ConfigurePin( spi_conf->miso.port, spi_conf->miso.pin, INPUT, PID_SPI_DI, false );
}

/**
 ****************************************************************************************
 * @brief Reserves I2C GPIO pins, reads the GPIO map set by initiator, and configures the the I2C pins.
 *
 * @param[in]   pointer to port/pin pad structure.
 *
 * @return      void
 *
 ****************************************************************************************
 */
void app_spotar_i2c_config(i2c_gpio_config_t *i2c_conf)
{
    i2c_conf->sda.port      = ((GPIO_PORT)((spota_state.gpio_map & 0x000000f0) >>  4));
    i2c_conf->sda.pin       = ((GPIO_PIN) ((spota_state.gpio_map & 0x0000000f)));
    i2c_conf->scl.port      = ((GPIO_PORT)((spota_state.gpio_map & 0x0000f000) >> 12));
    i2c_conf->scl.pin       = ((GPIO_PIN) ((spota_state.gpio_map & 0x00000f00) >> 8));
    i2c_conf->slave_addr    = ((spota_state.gpio_map & 0xffff0000) >> 16);
#if (!BLE_APP_KEYBOARD) 
    RESERVE_GPIO( I2C_SCL, i2c_conf->scl.port, i2c_conf->scl.pin, PID_I2C_SCL);
    RESERVE_GPIO( I2C_SDA, i2c_conf->sda.port, i2c_conf->sda.pin, PID_I2C_SDA);
#endif
    GPIO_ConfigurePin(i2c_conf->scl.port, i2c_conf->scl.pin, INPUT, PID_I2C_SCL, false);
    GPIO_ConfigurePin(i2c_conf->sda.port, i2c_conf->sda.pin, INPUT, PID_I2C_SDA, false);
}
#endif // BLE_APP_PRESENT
#endif //BLE_SPOTA_RECEIVER

/// @} APP
