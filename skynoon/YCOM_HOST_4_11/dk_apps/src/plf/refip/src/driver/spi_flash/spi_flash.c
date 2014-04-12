/**
 ****************************************************************************************
 *
 * @file spi_flash.c
 *
 * @brief flash driver over spi interface.
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


#include "spi_flash.h"

// local copy of FLASH setup parameters
uint32_t spi_flash_size;         
uint32_t spi_flash_page_size;


/**
 ****************************************************************************************
 * @brief Initialize SPI Flash
 * @param[in ]spi_flash_size_param:         Flash Size
 * @param[in] spi_flash_page_size_param:    Flash Page Size
 ****************************************************************************************
 */
void spi_flash_init(uint32_t spi_flash_size_param, uint32_t spi_flash_page_size_param)
{
    spi_flash_size = spi_flash_size_param;
    spi_flash_page_size = spi_flash_page_size_param;
}

/**
 ****************************************************************************************
 * @brief Read Status Register
 * @return  Status Register value
 ****************************************************************************************
 */
uint8_t spi_flash_read_status_reg(void)
{
    //do not add spi_flash_wait_till_ready here
    spi_set_bitmode(SPI_MODE_16BIT);                          // set SPI bitmode to 16-bit      
    return spi_transaction((uint16_t)(READ_STATUS_REG<<8));
}

/**
 ****************************************************************************************
 * @brief Wait till flash is ready for next action 
* @return  Success : ERR_OK
*          Failure : ERR_TIMEOUT 
 ****************************************************************************************
 */
int8_t spi_flash_wait_till_ready (void)
{
  int count;
  for (count = 0; count < MAX_READY_WAIT_COUNT; count++)
  {
    if ((spi_flash_read_status_reg() & STATUS_BUSY) == 0)
        return ERR_OK;
  }
  return ERR_TIMEOUT;
}


/**
 ****************************************************************************************
 * @brief Issue a Write Enable Command  
 * @return error code or success (ERR_OK)  
 ****************************************************************************************
 */  
int8_t spi_flash_set_write_enable(void)
{
    uint32_t commandSendCount;
    uint32_t statusReadCount;
    uint8_t status;
    if (spi_flash_wait_till_ready() == ERR_OK)
    {
        spi_set_bitmode(SPI_MODE_8BIT);           // set SPI bitmode to 8-bit               
        for (commandSendCount = 0; commandSendCount < MAX_COMMAND_SEND_COUNT; commandSendCount++)   
        {        
            spi_transaction(WRITE_ENABLE);      // send instruction              
            for (statusReadCount = 0; statusReadCount < MAX_READY_WAIT_COUNT; statusReadCount++)
            {
                status = spi_flash_read_status_reg();
                if  ( ((status & STATUS_BUSY) == 0) && ((status & STATUS_WEL) != 0) ) 
                    return ERR_OK;    
            }
        }
    }
    return ERR_TIMEOUT;
}


/**
 ****************************************************************************************
 * @brief Issue a Write Enable Volatile Command  
 * @return error code or success (ERR_OK)  
 ****************************************************************************************
 */  
int8_t spi_flash_write_enable_volatile(void)
{
    uint32_t commandSendCount;
    uint32_t statusReadCount;
    uint8_t status;
    if (spi_flash_wait_till_ready() == ERR_OK)
    {
        spi_set_bitmode(SPI_MODE_8BIT);           // set SPI bitmode to 8-bit               
        for (commandSendCount = 0; commandSendCount < MAX_COMMAND_SEND_COUNT; commandSendCount++)   
        {        
            spi_transaction(WRITE_ENABLE_VOL);          // send instruction              
            for (statusReadCount = 0; statusReadCount < MAX_READY_WAIT_COUNT; statusReadCount++)
            {
                status = spi_flash_read_status_reg();
                if  ( ((status & STATUS_BUSY) == 0) && ((status & STATUS_WEL) != 0) ) 
                    return ERR_OK;    
            }
        }
    }
    return ERR_TIMEOUT;    
}

/**
 ****************************************************************************************
 * @brief Issue a Write Disable Command  
 * @return error code or success (ERR_OK)  
 ****************************************************************************************
 */  
int8_t spi_flash_set_write_disable(void)
{
    uint32_t commandSendCount;
    uint32_t statusReadCount;
    uint8_t status;
    if (spi_flash_wait_till_ready() == ERR_OK)
    {
        spi_set_bitmode(SPI_MODE_8BIT);           // set SPI bitmode to 8-bit               
        for (commandSendCount = 0; commandSendCount < MAX_COMMAND_SEND_COUNT; commandSendCount++)   
        {        
            spi_transaction(WRITE_DISABLE);          // send instruction              
            for (statusReadCount = 0; statusReadCount < MAX_READY_WAIT_COUNT; statusReadCount++)
            {
                status = spi_flash_read_status_reg();
                if  ( ((status & STATUS_BUSY) == 0) && ((status & STATUS_WEL) == 0) ) 
                    return ERR_OK;    
            }
        }
    }
    return ERR_TIMEOUT;    
}

/**
 ****************************************************************************************
 * @brief Write Status Register
 * @param[in] dataToWrite:   Value to be written to Status Register
 * @return error code or success (ERR_OK)
 ****************************************************************************************
 */
int32_t spi_flash_write_status_reg(uint8_t dataToWrite)
{
    int8_t spi_flash_status;
    spi_flash_status = spi_flash_wait_till_ready();
    if (spi_flash_status != ERR_OK)
        return spi_flash_status; // an error has occured        
    
    spi_set_bitmode(SPI_MODE_16BIT);    
    spi_transaction((WRITE_STATUS_REG<<8) | dataToWrite);     // send  Write Status Register-1 instruction
    return spi_flash_wait_till_ready();
}

/**
 ****************************************************************************************
 * @brief Read data from a given starting address (up to the end of the flash)
 *
 * @param[in] *rd_data_ptr:  Points to the position the read data will be stored
 * @param[in] address:       Starting address of data to be read
 * @param[in] size:          Size of the data to be read
 * 
 * @return  Number of read bytes or error code
 ****************************************************************************************
 */
uint32_t spi_flash_read_data (uint8_t *rd_data_ptr, uint32_t address, uint32_t size)
{
    int8_t spi_flash_status;
	uint32_t bytes_read, i, temp_size;
	
	// check that all bytes to be retrieved are located in valid flash memory address space
	if (size + address > spi_flash_size)
    {
		temp_size = spi_flash_size - address;
		bytes_read = temp_size;
	}
	else
    {
		temp_size = size;
		bytes_read = size;
	}

    
    spi_flash_status = spi_flash_wait_till_ready();
    if (spi_flash_status != ERR_OK)
        return spi_flash_status; // an error has occured     

    
    spi_set_bitmode(SPI_MODE_32BIT);
    
	spi_cs_low();            			            	// pull CS low
    
    spi_access( (READ_DATA<<24) | address);             // Command for sequencial reading from memory
      
    spi_set_bitmode(SPI_MODE_8BIT);
    
    for(i=0; i<temp_size; i++)
    {
		*rd_data_ptr++ = (uint8_t)spi_access(0x0000);   // bare SPI transaction
    }
	
	spi_cs_high();               			            // push CS high
	
	return bytes_read;
}


/**
 ****************************************************************************************
 * @brief Program page (up to <SPI Flash page size> bytes) starting at given address
 *
 * @param[in] *wr_data_ptr:  Pointer to the data to be written
 * @param[in] address:       Starting address of data to be written
 * @param[in] size:          Size of the data to be written (should not be larger than SPI Flash page size)
 * @return error code or success (ERR_OK)
 ****************************************************************************************
 */
int32_t spi_flash_page_program(uint8_t *wr_data_ptr, uint32_t address, uint16_t size)
{
    int8_t spi_flash_status;
	uint16_t temp_size = size;
    	
	if (temp_size > spi_flash_page_size)                // check for max page size
		temp_size = spi_flash_page_size;
	
    spi_flash_status = spi_flash_wait_till_ready();
    if (spi_flash_status != ERR_OK)
        return spi_flash_status; // an error has occured   
  
    spi_flash_status = spi_flash_set_write_enable();    // send [Write Enable] instruction
    if (spi_flash_status != ERR_OK)  
        return spi_flash_status; // an error has occured       
    
    spi_set_bitmode(SPI_MODE_32BIT);
    
	spi_cs_low();            			            	// pull CS low

    spi_access( (PAGE_PROGRAM<<24) | address);          // Command for page programming
    
    //for (i=0; i<2000; i++);
    
    spi_set_bitmode(SPI_MODE_8BIT);           
	while(temp_size>0)                                  // Write data bytes
    {
		spi_access(*wr_data_ptr++);
		temp_size--;
	}
	
    spi_cs_high();                                      // push CS high  
  	return spi_flash_wait_till_ready();
}


/**
 ****************************************************************************************
 * @brief Issue a comamnd to Erase a given address
 *
 * @param[in] address:  Address that belongs to the block64/block32/sector range
 * @param[in] spiEraseModule: BLOCK_ERASE_64, BLOCK_ERASE_32, SECTOR_ERASE
 * @return error code or success (ERR_OK)
 ****************************************************************************************
 */
int8_t spi_flash_block_erase(uint32_t address, SPI_erase_module_t spiEraseModule)
{
 	if (spi_flash_set_write_enable() != ERR_OK)         // send [Write Enable] instruction
		return ERR_TIMEOUT;
   
    spi_set_bitmode(SPI_MODE_32BIT);
	
    spi_transaction( (spiEraseModule<<24) | address);   // Command for erasing a sector    
   
  	return spi_flash_wait_till_ready();                 
 }

/**
 ****************************************************************************************
 * @brief Erase chip
 * @return error code or success (ERR_OK)
 ****************************************************************************************
 */
int8_t spi_flash_chip_erase(void)
{
	//TI GINETAI SE AN H MNIMI EINAI PROTECTED
    uint8_t status;
    
    if (spi_flash_set_write_enable() != ERR_OK)         // send [Write Enable] instruction
		return ERR_TIMEOUT;
    
    spi_set_bitmode(SPI_MODE_8BIT);
        
    spi_transaction(CHIP_ERASE);                    // Command for Chip Erase    
    
    status = spi_flash_wait_till_ready();
    
    return status;
}


/**
 ****************************************************************************************
 * @brief verify erasure
 * @return error code or success (ERR_OK)
 ****************************************************************************************
 */
int8_t TO_BE_IMPLEMENTED_spi_flash_check_erase(unsigned long dest_addr, unsigned long len)
{
     //to be implemented   
    return ERR_TIMEOUT;
}
    

/**
 ****************************************************************************************
 * @brief Get Manufacturer / Device ID
 * @return  Manufacturer/Device ID
 ****************************************************************************************
 */
int16_t spi_read_flash_memory_man_and_dev_id(void)
{ 
    int8_t spi_flash_status;
    uint16_t idWord = 0;
    
    spi_flash_status = spi_flash_wait_till_ready();
    if (spi_flash_status != ERR_OK)
        return spi_flash_status; // an error has occured   
    
    spi_set_bitmode(SPI_MODE_16BIT);    
    
	spi_cs_low();            	// pull CS low

	spi_access(MAN_DEV_ID<<8);    // SPI transaction to send command    
    spi_access(0x0000);           // dummy   SPI transaction to send (A23-A0)
    idWord = spi_access(0x0000);  // SPI transaction to read Manufacturer Id, Device ID

    spi_cs_high();               // push CS high  

	return idWord;
}



/**
 ****************************************************************************************
 * @brief Get Unique ID Number
 * @return  Unique ID Number
 ****************************************************************************************
 */
uint64_t spi_read_flash_unique_id(void)
{
    int8_t spi_flash_status;
	uint64_t unique_id;
              
    spi_flash_status = spi_flash_wait_till_ready();
    if (spi_flash_status != ERR_OK)
        return spi_flash_status; // an error has occured      

    spi_set_bitmode(SPI_MODE_8BIT);    
    
	spi_cs_low();            				  // pull CS low

	spi_access(READ_UNIQUE_ID);               // SPI access to send [Read Unique ID] command
    
    spi_set_bitmode(SPI_MODE_32BIT);           // dummy transaction for the 4 dummy bytes
    
    spi_access(0x0000);                      // dummy bare SPI transaction
    
    unique_id = ((uint64_t)spi_access(0x0000) << 32);     // SPI access to get the high part of unique id

    unique_id |= spi_access(0x0000);          // bare SPI access to get the high part of unique id   
    
    spi_cs_high();                           // push CS high  

	return unique_id;	
}


/**
 ****************************************************************************************
 * @brief Get JEDEC ID
 * @return  JEDEC ID
 ****************************************************************************************
 */
int32_t spi_read_flash_jedec_id(void)
{
    int8_t spi_flash_status;
    
	uint32_t jedec_id;
    
    spi_flash_status = spi_flash_wait_till_ready();
    if (spi_flash_status != ERR_OK)
        return spi_flash_status; // an error has occured      
    
    spi_set_bitmode(SPI_MODE_8BIT);
    
    spi_cs_low();            		   	  // pull CS low

	spi_access(JEDEC_ID);                 //  SPI accsss to send [Read Unique ID] command
    
    jedec_id = spi_access(0x0000) << 16;  //  SPI accsss to get the Manufacture ID

    jedec_id |= spi_access(0x0000) << 8;  //  SPI accsss to get the Memory Type
    
    jedec_id |= spi_access(0x0000);       //  SPI accsss to get the Capacity
    
    spi_cs_high();                        // push CS high  

	return jedec_id;	   
}

/**
 ****************************************************************************************
 * @brief Write data to flash across page boundaries and at any starting address
 *
 * @param[in] *wr_data_ptr:  Pointer to the data to be written
 * @param[in] address:       Starting address of page to be written (must be a multiple of SPI Flash page size)
 * @param[in] size:          Size of the data to be written (can be larger than SPI Flash page size)
 * 
 * @return  Number of bytes actually written
 ****************************************************************************************
 */
int32_t spi_flash_write_data (uint8_t * wr_data_ptr, uint32_t address, uint32_t size)
{
	uint32_t bytes_written; 
	uint32_t feasible_size = size;
    uint32_t currentAddress = address;
	uint32_t currentEndOfPage = (currentAddress / spi_flash_page_size + 1) * spi_flash_page_size - 1;
    uint32_t bytes_left_to_send;

    spi_set_bitmode(SPI_MODE_8BIT);
    
  	// limit to the maximum count of bytes that can be written to a (SPI_FLASH_SIZE x 8) flash
	if (size > spi_flash_size - address)
      feasible_size = spi_flash_size - address;
 
    bytes_left_to_send = feasible_size;
    bytes_written = 0;
    
    while (bytes_written < feasible_size)
    {
        // limit the transaction to the upper limit of the current page
        if (currentAddress + bytes_left_to_send > currentEndOfPage)
            bytes_left_to_send = currentEndOfPage - currentAddress + 1;             
        if (spi_flash_page_program(wr_data_ptr + bytes_written, currentAddress, bytes_left_to_send) != ERR_OK) //write the current page data
			return ERR_TIMEOUT;
        bytes_written += bytes_left_to_send;                                                     
        currentAddress = currentEndOfPage + 1;  //address points to the first memory position of the next page
        currentEndOfPage += spi_flash_page_size;
        bytes_left_to_send = feasible_size - bytes_written;
    }
    return bytes_written;
}
