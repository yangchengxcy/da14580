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

#include "app_spotar.h"

#if BLE_SPOTA_RECEIVER

#include <string.h>                  // string manipulation and functions

#include "app.h"                     // application definitions
#include "app_task.h"                // application task definitions
#include "spotar_task.h"             // SPOTA functions
#include "co_bt.h"
#include "arch.h"                    // platform definitions
#include "ke_timer.h"				 // kernel timer 
#include "gpio.h"
#include "arch_sleep.h"

//application SPOTA state structrure
app_spota_state spota_state __attribute__((section("exchange_mem_case1")));

uint8_t spota_new_pd[SPOTA_NEW_PD_SIZE] __attribute__((section("exchange_mem_case1")));

uint8_t spota_all_pd[SPOTA_OVERALL_PD_SIZE] __attribute__((section("spota_patch_data_area")));

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
*/
 
 /**
 ****************************************************************************************
 * SPOTAR Application Functions
 ****************************************************************************************
 */
 
/**
 ****************************************************************************************
 * @brief Initialize SPOTAR Apllication.
 *
 * @param[in] void
 *
 * @return void.
 ****************************************************************************************
 */

void app_spotar_init(void)
{
	
    spota_state.spota_pd_idx = 0;
    spota_state.new_patch_len = 0;
    memset( spota_all_pd, 0x00, 4); // Set first WORD to 0x00
    spota_state.mem_dev = SPOTAR_MEM_INVAL_DEV;
    		
    // LED Output 
    //GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_7, OUTPUT, PID_GPIO, false );
}

/**
 ****************************************************************************************
 * @brief Reset SPOTAR Apllication.
 *
 * @param[in] void
 *
 * @return void.
 ****************************************************************************************
 */
void app_spotar_reset(void)
{
	
    spota_state.spota_pd_idx = 0;
    spota_state.new_patch_len = 0;
    spotar_env.pd_flag = false;
    //spotar_send_status_update_req((uint8_t) SPOTAR_INVAL_MEM_TYPE);
}

/**
 ****************************************************************************************
 * @brief POTAR service create database.
 *
 * @param[in] void
 *
 * @return void.
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
 * @brief Inialize applacition and enable SPOTAR profile.
 *
 * @param[in] type      Alert type. Link Loss or Imediate Alert
 * @param[in] level     Alert level. Mild or High
 *
 * @return void.
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
#if 0    
    req->patch_data = 0;  // SPOTA initiator needs to specify mem device first
    req->patch_len  = 0;
    req->patch_status = 0;
#endif
    
    // Send the message
    ke_msg_send(req);
}

/**
 ****************************************************************************************
 * @brief Reads memory device and rights memory info.
 *
 * @param[in] mem_dev_typetype      MSbyte holds the Memory device type, rest is the base address.
 * @param[in] mem_info              16MSbits show number of patches, 16LSbits overall mem len
 *
 * @return void.
 ****************************************************************************************
 */
void app_spotar_read_mem(uint32_t mem_dev, uint32_t* mem_info)
{
    uint32_t mem_dev_start;
    
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
            mem_dev_start = (uint32_t)(spota_all_pd + (SPOTA_OVERALL_PD_SIZE - 4));
            *mem_info = get_patching_spota_length( (mem_dev_start & 0x0000ffff), 0);
        }
        else
        {
            // We do not have patch info for external mem devices
            *mem_info = 0;
        }
        
        // Valid memory device. Spotar service started
        spotar_send_status_update_req((uint8_t) SPOTAR_SRV_STARTED);
        app_spotar_start();
    }
    else
    {
        if( spota_state.mem_dev == SPOTAR_MEM_SERVICE_EXIT )
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
}
/**
 ****************************************************************************************
 * @brief Starts LED flashing to indicate SPOTAR apllication is ready.
 *
 * @param[in] void
 *
 * @return void.
 ****************************************************************************************
 */

void app_spotar_start(void)
{	        
    spota_state.sleep_mode = app_get_sleep_mode();  // store sleep mode before entering SPOTAR
    app_disable_sleep();    // Disable sleep when SPOTAR starts	
}

/**
 ****************************************************************************************
 * @brief Stops SPOTAR apllication alert.
 *
 * @return void.
 ****************************************************************************************
 */

void app_spotar_stop(void)
{

    if( spota_state.mem_dev == SPOTAR_MEM_I2C_EEPROM ){
        i2c_close_spota(spota_state.gpio_map);
    }
    
    if( spota_state.mem_dev == SPOTAR_MEM_SPI_FLASH ) {
        spi_close_spota(spota_state.gpio_map);
    }
    
    // Restore Sleep mode
    switch( spota_state.sleep_mode)
	{
		case ARCH_SLEEP_OFF: 
            app_disable_sleep(); break;
		case ARCH_EXT_SLEEP_ON:
            app_set_extended_sleep(); break;
		case ARCH_DEEP_SLEEP_ON: 
            app_set_deep_sleep(); break;
	}
    
    // Set memory device to invalid type so that service will not 
    // start untill the memory device is explisitly set upon service start
    spota_state.mem_dev = SPOTAR_MEM_INVAL_DEV;
}

/**
 ****************************************************************************************
 * @brief Inform SPOTAR task. Update status characteristic.
 *
 * @param[in] status        SPOTAR status
 *
 * @return void.
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
 * @brief Inform SPOTAR task. Update memory info characteristic.
 *
 * @param[in] mem_info        Patch memory info. Number of patches and overall patch length
 *
 * @return void.
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
 * @brief SPOTA data handler. Calculate CRC, store patch data to memory device.
 *
 * @param[in] void
 *
 * @return void.
 *
 ****************************************************************************************
 */
void app_spotar_pd_hdlr(void)
{
    
    uint32_t mem_info;
    uint32_t mem_dev;
    uint16_t overall_len_in_bytes;
    uint16_t newpatch_copy_pos;
    uint16_t flash_page_size;
    uint16_t status = SPOTAR_CMP_OK;
    uint8_t len; // remaining length to 16-byte page
    
    // SPOTAR_TODO. Calculate CRC
    
    // Check mem dev. 
    switch (spota_state.mem_dev)
    {            
        case SPOTAR_MEM_INT_SYSRAM:
        case SPOTAR_MEM_INT_RETRAM:
            mem_dev = (uint32_t)(spota_all_pd + (SPOTA_OVERALL_PD_SIZE - sizeof(uint32_t)));
            mem_info = get_patching_spota_length( (mem_dev & 0x0000ffff), 0);
            overall_len_in_bytes = (mem_info & 0xffff) << 2;
                        
            // Note that inorder to keep the same implementation as patching in OTP, the first patch        
            // header starts at the end of the allocated SYSRAM or RETRAM memory bank.
            
            // Counting from the end of the memory bank, write new patch starting at the location 
            // where the existing patch(es) end, plus the size of the new patch.
            // Note that patch data are coppied in words.
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
                *ptr_up = *ptr_down;
            }
    
            mem_info = get_patching_spota_length( (mem_dev & 0x0000ffff), 0);
            spotar_send_mem_info_update_req(mem_info);                        
            break;
        case SPOTAR_MEM_I2C_EEPROM:
            mem_info = get_patching_spota_length( ((spota_state.mem_dev << 24) | spota_state.mem_base_add), spota_state.gpio_map);
            overall_len_in_bytes = (mem_info & 0xffff) << 2;                                                          
            if( (overall_len_in_bytes % SPOTAR_I2C_PAGE_SIZE) != 0  )
            {
                len = SPOTAR_I2C_PAGE_SIZE - (overall_len_in_bytes % SPOTAR_I2C_PAGE_SIZE);
                // Write first the number of bytes to fill the 16-byte page, and then the rest of the patch
                write_i2c_eeprom(( spota_state.mem_base_add + overall_len_in_bytes), spota_new_pd, len );
                write_i2c_eeprom(( spota_state.mem_base_add + overall_len_in_bytes + len), &spota_new_pd[len], (spota_state.spota_pd_idx-len) );                
            }
            else
            {
                // No need to check for 16 bytes boundaries when writing the first patch.
                write_i2c_eeprom(( spota_state.mem_base_add + overall_len_in_bytes),spota_new_pd, spota_state.spota_pd_idx);
            }
            mem_info = get_patching_spota_length( ((spota_state.mem_dev << 24) | spota_state.mem_base_add), spota_state.gpio_map);
            spotar_send_mem_info_update_req(mem_info);
            break;
        case SPOTAR_MEM_SPI_FLASH:           
            mem_info = get_patching_spota_length( ((spota_state.mem_dev << 24) | spota_state.mem_base_add), spota_state.gpio_map);
            overall_len_in_bytes = (mem_info & 0xffff) << 2;     
            spi_init_sporta(spota_state.gpio_map);
            flash_page_size=( (read_spi_flash_status()&01)==0 )?264:256;               
            write_spi_flash_page( (spota_state.mem_base_add>>9) +(overall_len_in_bytes/flash_page_size), overall_len_in_bytes%flash_page_size, spota_new_pd, spota_state.spota_pd_idx);                
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
    
    // SPOTA finished succesfully. Send Indication to initiator
    spotar_send_status_update_req((uint8_t) status);
}
    
/**
 ****************************************************************************************
 * @brief SPOTA execute patch.
 *
 * @param[in] void
 *
 * @return void.
 *
 ****************************************************************************************
 */
void app_spotar_exec_patch(void)
{
        // Example execute patch function. Executes patch from SPI
        uint32_t  len;
        
        // read patch len from spi flash
        len = read_spi_patch_table_length(0x03002800, 0x05060300 /*0x25262320*/);
    
        // read patch len from i2c eeprom
        //len = read_i2c_patch_table_length( 0x02000800, 0x00500203 /*0x00500203*/);
        if( len > 0)
        {
            // execute patch from spi flash
            exec_patching_spota(0x03002800, 0x05060300 /*0x25262320*/ ,(WORD *) spota_all_pd , (len & 0xffff) );
            // execute patch from i2c eeprom
            //exec_patching_spota(0x02000800, 0x00500203 /*0x00502223*/ ,(WORD *) spota_all_pd , (len & 0xffff) );
        }
}

/*
 * SPOTA  mem access application functions
 ****************************************************************************************
*/

void spi_erase_page(int page_address)
{    
    char spics_location;

    spics_location= (spota_state.gpio_map>>8)&0xFF;
    
    /////// BUFFER TO MAIN MEMORY
    spota_spi_open_cs(spics_location);
    spi_serial_flash_access(0x81);                     // buffer to main with build-in erase;
    spi_serial_flash_access( (page_address>>7) &0xFF); // start at zero buffer address
    spi_serial_flash_access( (page_address<<1) &0xFF); 
    spi_serial_flash_access( (0) &0xFF); 
    spota_spi_close_cs(spics_location);
    while(  (read_spi_flash_status()& 0x80) ==0);      // wait until flash not busy
 
}

// It should be called after setting the GPIO map for the SPI
void spi_mem_erase(void)
{
    uint8_t i;
    for ( i=0; i <= 73; i++ ){    
        spi_erase_page(i);
    }
}

char spi_serial_flash_access(char wr_data)
{
	char rd_data;
	SetWord16(SPI_RX_TX_REG0, (short)wr_data);
	do{
	}while (GetBits16(SPI_CTRL_REG,SPI_INT_BIT)==0);    // polling to wait for spi have data
	rd_data =0xFF&GetWord16(SPI_RX_TX_REG0);  		    // read byte from SPI
	SetWord16(SPI_CLEAR_INT_REG, 0x01);		            // clean pending flag
	return rd_data;							            // return data
} 

short read_spi_flash_status(){
	short value;
    char spics_location;
	
	spics_location= (spota_state.gpio_map>>8)&0xFF;
    spota_spi_open_cs(spics_location);
    spi_serial_flash_access(0xD7);	
	value=spi_serial_flash_access( 0x00)<<8;
	value+=spi_serial_flash_access( 0x00);	
    spota_spi_close_cs(spics_location);
	return value;
}

// Write to SPI FLASH
void write_spi_flash_page(WORD page_address, short offset_in_page, unsigned char* ptr, WORD size){
	WORD counter;
	counter=size;
    char spics_location;
	
	spics_location= (spota_state.gpio_map>>8)&0xFF;
    
	// max page size 264;
	if (counter>264)
        counter=264;

	////// MAIN MEMORY to BUFFER the page_address page
    spota_spi_open_cs(spics_location);
	spi_serial_flash_access(0x53);	// buffer to main with build-in erase;
	spi_serial_flash_access( (page_address>>7) &0xFF); 
	spi_serial_flash_access( (page_address<<1) &0xFF); 
	spi_serial_flash_access( (0) &0xFF); 
	
    spota_spi_close_cs(spics_location);
    ////// WAIT UNTIL NOT BUSY
	while( 	(read_spi_flash_status()& 0x80) ==0); // wait until flash not busy
	
	////// WRITE TO BUFFER at offset_in_page
    spota_spi_open_cs(spics_location);
	spi_serial_flash_access(0x84);	// write to buffer 
	spi_serial_flash_access( 0);	// 
	spi_serial_flash_access( (offset_in_page) &0x01); 
	spi_serial_flash_access( (offset_in_page) &0xFF);
	do {
        spi_serial_flash_access(*ptr);
        ptr++;
        counter--;
	} while(counter>0);
    spota_spi_close_cs(spics_location);
    
	/////// BUFFER TO MAIN MEMORY the page_address page
    spota_spi_open_cs(spics_location);
	spi_serial_flash_access(0x83);	// buffer to main with build-in erase;
	spi_serial_flash_access( (page_address>>7) &0xFF);	// start at zero buffer address
	spi_serial_flash_access( (page_address<<1) &0xFF); 
	spi_serial_flash_access( (0) &0xFF); 
	
    spota_spi_close_cs(spics_location);
	
	////// WAIT UNTIL NOT BUSY
	while( 	(read_spi_flash_status()& 0x80) ==0); 	
}

// write to I2C eeprom and wait until data is written, polling ACK
void write_i2c_eeprom(short address,unsigned  char * ptr, short size){
                
    short temp;
    short temp_size;
    short temp_address;

    ////////////////////// write
    temp_address=address;
    temp_size=size;

    i2c_init_spota(spota_state.gpio_map);
    do{
        temp=temp_size;
        if(temp>16)
            temp=16;

        SetWord16(I2C_DATA_CMD_REG, (temp_address>>8)&0xFF);    // set address MSB
        SetWord16(I2C_DATA_CMD_REG, temp_address&0xFF);         // set address LSB

        do {
            while(( GetWord16(I2C_STATUS_REG)&TFNF)==0 );   // Wait I2c Transmit fifo is full
            SetWord16(I2C_DATA_CMD_REG, (*ptr&0xFF) );      // send write data
            ptr++;
            temp--;
        } while(temp!=0);

        while( ( GetWord16(I2C_STATUS_REG)&TFE)==0 );  // wait until TX fifo is empty
        while((GetWord16(I2C_STATUS_REG)&MST_ACTIVITY)!=0 );  // wait until no master activity   

        // polling until eeprom ACK again
        do {
            SetWord16(I2C_DATA_CMD_REG, 0x08);                                               // send a dummy access
            while( ( GetWord16(I2C_STATUS_REG)&TFE)==0 );  // wait until TX fifo is empty
            while((GetWord16(I2C_STATUS_REG)&MST_ACTIVITY)!=0 );  // wait until no master activity   
            temp=GetWord16(I2C_TX_ABRT_SOURCE_REG);   // get the resulrta
            GetWord16(I2C_CLR_TX_ABRT_REG);                                                                     // Clear the TX abord flash
        }while((temp&ABRT_7B_ADDR_NOACK)!=0);  // while not ACK

        temp_size-=16;
        temp_address+=16;        

    }while(temp_size>0);
    
    i2c_close_spota(spota_state.gpio_map);
}

#endif //BLE_SPOTA_RECEIVER

/// @} APP
