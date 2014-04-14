#include <stdint.h>
#include "compiler.h"
#include "arch.h"

__INLINE uint8_t spi_read_byte(void)
{
	uint8_t rd_byte;
	rd_byte = 0xFF&GetWord16(SPI_RX_TX_REG0);	// read byte from SPI
	SetWord16(SPI_CLEAR_INT_REG, 0x01);				// clean pending flag
	while (GetBits16(SPI_CTRL_REG1,SPI_BUSY)==1);
	return rd_byte;														// return data
}

__INLINE void spi_write_byte(uint8_t wr_byte)
{
	while (GetBits16(SPI_CTRL_REG,SPI_TXH)==1); 		// polling to wait for spi transmit completion
	SetWord16(SPI_RX_TX_REG0, 0xFF&wr_byte);  			// Send byte
	while (GetBits16(SPI_CTRL_REG1,SPI_BUSY)==1);
	GetWord16(SPI_RX_TX_REG0);
}

__INLINE void spi_mint_enable(void)
{
	SetBits16(SPI_CTRL_REG,SPI_ON,0);  					// disable SPI block
	SetBits16(SPI_CTRL_REG,SPI_MINT,1);	// enable or disable mint
	SetBits16(SPI_CTRL_REG,SPI_ON,1);  					// disable SPI block
}

__INLINE void spi_mint_disable(void)
{
	SetBits16(SPI_CTRL_REG,SPI_ON,0);  					// disable SPI block
	SetBits16(SPI_CTRL_REG,SPI_MINT,0);	// enable or disable mint
	SetBits16(SPI_CTRL_REG,SPI_ON,1);  					// disable SPI block
}

__INLINE uint8_t spi_data_rdy_getf(void)
{
	uint16_t localVal = GetBits16(SPI_CTRL_REG,SPI_INT_BIT);
	return ((localVal & ((uint8_t)0x0001)) >> 0);
}

__INLINE uint8_t spi_cs_getf(void)
{
	uint16_t localVal = GetBits16(P0_DATA_REG,0x02);
	return ((localVal & ((uint8_t)0x0001)) >> 0);
}
