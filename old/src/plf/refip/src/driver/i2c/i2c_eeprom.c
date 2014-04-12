/**
 ****************************************************************************************
 *
 * @file i2c_eeprom.c
 *
 * @brief eeprom driver over i2c interface.
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
#include "gpio.h"
#include "i2c_eeprom.h"
#include "app_keyboard.h"


void i2c_init(void)
{
	SetBits16(CLK_PER_REG, I2C_ENABLE, 1);  	// enable  clock for I2C (was SPI)
	//SetWord16(P02_MODE_REG,FUNC_I2C_SCL);	// set P0_2 I2C SCL function, auto enable pull-up
	//SetWord16(P03_MODE_REG,FUNC_I2C_SDA);	// set P0_3 I2C SDA function, auto enable pull-up
	// change connector pins - MODA
//	SetWord16(P07_MODE_REG,PID_I2C_SCL);	// set P0_7 I2C SCL function, auto enable pull-up
//	SetWord16(P06_MODE_REG,PID_I2C_SDA);	// set P0_6 I2C SDA function, auto enable pull-up
    GPIO_SetPinFunction(I2C_SCL_PORT, I2C_SCL_PIN, INPUT, PID_I2C_SCL);
    GPIO_SetPinFunction(I2C_SDA_PORT, I2C_SDA_PIN, INPUT, PID_I2C_SDA);
	SetWord16 (I2C_ENABLE_REG, 0x0); 		// Disable the I2C controller	
	SetWord16 (I2C_CON_REG,I2C_MASTER_MODE | I2C_SLAVE_DISABLE|I2C_RESTART_EN);	// Slave is disable
	//SetBits16 (I2C_CON_REG,I2C_SPEED,I2C_STANDARD);	   		// Standard mode
	SetBits16 (I2C_CON_REG,I2C_SPEED,2);	   		// Fast mode
	//SetWord16(I2C_CON_REG,0x73);
	SetWord16(I2C_TAR_REG, 0x50);		// Set Slave device address
	SetWord16(I2C_ENABLE_REG, 0x1); 				// Enable the I2C controller
	while(( GetWord16(I2C_STATUS_REG)&0x20)!=0);  // Wait I2c mater FSM to be IDLE	
}

void i2c_release(void)
{
	SetWord16 (I2C_ENABLE_REG, 0x0); 		// Disable the I2C controller	
	SetBits16(CLK_PER_REG, I2C_ENABLE, 1);  	// enable  clock for SPI
//	SetWord16(P02_MODE_REG,FUNC_GPIO|DIR_INPUT);	// set P0_2 gpio input
//	SetWord16(P03_MODE_REG,FUNC_GPIO|DIR_INPUT);	// set P0_3 gpio input
}

int8_t random_read_i2c_eeprom(uint8_t address)
{
	GetWord16(I2C_CLR_TX_ABRT_REG);                 // Clear the TX abord flash
	
#if EEPROM_IS_8K
	SetWord16(I2C_DATA_CMD_REG, 0);                 // set read memory address high byte, write access
	SetWord16(I2C_DATA_CMD_REG, address);           // set read memory address low byte, write access
#else    
	SetWord16(I2C_DATA_CMD_REG, address);           // set read memory address, write access
#endif
	SetWord16(I2C_DATA_CMD_REG, 0x0100);            // set read memory address to 0x00, read access

	while( (0x06 & GetWord16(I2C_STATUS_REG)) !=6 );// Wait until I2C TX fifo empty
	
	do{} while(GetWord16(I2C_RXFLR_REG) == 0);

    return (0xFF & GetWord16(I2C_DATA_CMD_REG));
}

uint16_t sequential_read_i2c_eeprom(uint8_t * rd_data_ptr, uint8_t address, uint16_t size)
{
	uint8_t j, mult, tmp_address;
	uint16_t i, tmp_size;
	uint16_t bytes_read;
	
	mult = 0;
	tmp_address = address;
	
	if (size > 256 - address){    // Check for max bytes to be read from a 256x8 eeprom
		tmp_size = 256 - address;
		bytes_read = tmp_size;
	}
	else{
		tmp_size = size;
		bytes_read = size;
	}

	// Calculate tmp_size%64 due to receive fifo limitation 
	for (i = tmp_size; i>63; i-=64){
		mult++;
	}

#if (USE_WDOG)        
    SetWord16(WATCHDOG_REG, 0xFF);                                      // Reset WDOG! 255 * 10.24ms active time for I2C access! Remove when the driver is written properly!
#endif

	// Read 64 bytes at a time
	for(i = 0; i < mult; i++)
    {
#if EEPROM_IS_8K
		SetWord16(I2C_DATA_CMD_REG, 0);                         // set read memory address high byte, write access
		SetWord16(I2C_DATA_CMD_REG, tmp_address);               // set read memory address low byte (=tmp_address), write access
#else        
		SetWord16(I2C_DATA_CMD_REG, tmp_address);               // set read memory address to tmp_address, write access
#endif        
		
		for(j = 0; j < 64; j++)
        {
			while( (GetWord16(I2C_STATUS_REG) & TFNF) == 0 );   // Wait I2c Transmit fifo is full
			SetWord16(I2C_DATA_CMD_REG, 0x0100);                // send read command <size> times, read access
		}
        
		for(j = 0; j < 64; j++)                                 // get the received data
        {
			 do{} while(GetWord16(I2C_RXFLR_REG) == 0); 
                 
			 *rd_data_ptr =(0xFF & GetWord16(I2C_DATA_CMD_REG));
			 rd_data_ptr++;
		}
		tmp_address += 64;                                      // Update base address for read
		tmp_size -= 64;                                         // Update tmp_size for bytes remaining to be read
	}
	
	// Read the remaining bytes (not a multiple of 64)
#if EEPROM_IS_8K
    SetWord16(I2C_DATA_CMD_REG, 0);                             // set read memory address high byte, write access
    SetWord16(I2C_DATA_CMD_REG, tmp_address);                   // set read memory address low byte (=tmp_address), write access
#else        
    SetWord16(I2C_DATA_CMD_REG, tmp_address);                   // set read memory address to tmp_address, write access
#endif        
		
	for (j = 0; j < tmp_size; j++) 
    {
	    while( (GetWord16(I2C_STATUS_REG) & TFNF) == 0 );       // Wait while I2c Transmit fifo is full
		SetWord16(I2C_DATA_CMD_REG, 0x0100);                    // send read command <tmp_size> times, read access
	}
    
	for(j = 0; j < tmp_size; j++)                               // get the received data
    {
		 do{} while(GetWord16(I2C_RXFLR_REG) == 0);
             
		 *rd_data_ptr =(0xFF & GetWord16(I2C_DATA_CMD_REG));
		 rd_data_ptr++;
	}
    
#if (USE_WDOG)        
    SetWord16(WATCHDOG_REG, 0xC8);                              // Reset WDOG! 200 * 10.24ms active time for normal mode!
#endif

	return bytes_read;
}

void write_byte_i2c_eeprom(uint8_t address, uint8_t wr_data)
{
	short temp;
		
#if EEPROM_IS_8K
	SetWord16(I2C_DATA_CMD_REG, 0);                     // set memory address high bute, write access
	SetWord16(I2C_DATA_CMD_REG, address & 0xFF);        // set memory address low byte, write access
#else    
	SetWord16(I2C_DATA_CMD_REG, address & 0xFF);        // set memory address, write access
#endif

	while( (GetWord16(I2C_STATUS_REG) & TFNF) == 0 );   // Wait I2c Transmit fifo is full
	SetWord16(I2C_DATA_CMD_REG, wr_data & 0xFF);        // send write data
	
	while( ( GetWord16(I2C_STATUS_REG)&TFE)==0 );  // wait until TX fifo is empty
	while((GetWord16(I2C_STATUS_REG)&MST_ACTIVITY)!=0 );  // wait until no master activity   
		
	// polling until eeprom ACK again
	do {
		SetWord16(I2C_DATA_CMD_REG, 0x08);	  		// send a dummy access
		while( ( GetWord16(I2C_STATUS_REG)&TFE)==0 );  // wait until TX fifo is empty
		while((GetWord16(I2C_STATUS_REG)&MST_ACTIVITY)!=0 );  // wait until no master activity   
		temp=GetWord16(I2C_TX_ABRT_SOURCE_REG);   // get the resulrta
		GetWord16(I2C_CLR_TX_ABRT_REG);				  	// Clear the TX abord flash
	}	while((temp&ABRT_7B_ADDR_NOACK)!=0);  // while not ACK
}

// write to I2C eeprom and wait until data is written, polling ACK
void write_page_i2c_eeprom(uint8_t address, uint8_t *wr_data_ptr, uint8_t size)
{
	uint8_t temp;
    uint8_t temp2;
	uint8_t temp_size;
	uint8_t temp_address;
	
		////////////////////// write
	temp_address = address;
	temp_size = size;
	
#if (USE_WDOG)        
    SetWord16(WATCHDOG_REG, 0xFF);                                      // Reset WDOG! 255 * 10.24ms active time for I2C access! Remove when the driver is written properly!
#endif

	do{
		temp = temp_size;
		if(temp > 4)
			temp = 4;
        
		temp2 = temp;
        //SetWord16(I2C_DATA_CMD_REG, (temp_address>>8)&0xFF);
#if EEPROM_IS_8K
		SetWord16(I2C_DATA_CMD_REG, 0);                                 // set memory address high byte, write access
		SetWord16(I2C_DATA_CMD_REG, temp_address & 0xFF);               // set memory address low byte, write access
#else        
		SetWord16(I2C_DATA_CMD_REG, temp_address & 0xFF);               // set memory address, write access
#endif        
		
		do {
			while( (GetWord16(I2C_STATUS_REG) & TFNF) == 0 );           // Wait I2c Transmit fifo is not full
			SetWord16(I2C_DATA_CMD_REG, (*wr_data_ptr & 0xFF));         // send write data
			wr_data_ptr++;
			temp--;
		} while(temp != 0);
	
		while( (GetWord16(I2C_STATUS_REG) & TFE) == 0 );                // wait until TX fifo is empty - STOP
		while( (GetWord16(I2C_STATUS_REG) & MST_ACTIVITY) != 0 );       // wait until no master activity   

		// polling until eeprom ACK again
		do {
            SetWord16(I2C_DATA_CMD_REG, 0x08);	  		                // send a dummy access
            while( (GetWord16(I2C_STATUS_REG) & TFE) == 0 );            // wait until TX fifo is empty
            while( (GetWord16(I2C_STATUS_REG) & MST_ACTIVITY) != 0 );   // wait until no master activity   
            temp = GetWord16(I2C_TX_ABRT_SOURCE_REG);                   // get the result
            GetWord16(I2C_CLR_TX_ABRT_REG);				  	            // Clear the TX abort flash
		} while( (temp & ABRT_7B_ADDR_NOACK) != 0 );                    // while not ACK
	
        temp_size -= temp2;
        temp_address += 4;	
	} while(temp_size > 0);

#if (USE_WDOG)        
    SetWord16(WATCHDOG_REG, 0xC8);                       // Reset WDOG! 200 * 10.24ms active time for normal mode!
#endif
}
