/**
 ****************************************************************************************
 *
 * @file spi_hci.c
 *
 * @brief SPI Driver for HCI over SPI operation.
 *
 * $Rev: 1 $
 *
 ****************************************************************************************
 */
 
 /**
 ****************************************************************************************
 * @addtogroup SPI
 * @{
 ****************************************************************************************
 */
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "spi_hci.h"            // spi definition
#include "reg_spi.h"   			// spi register
#include "gpio.h"


/*
 * DEFINES
 *****************************************************************************************
 */

/*
 * ENUMERATION DEFINITIONS
 *****************************************************************************************
 */

/*
 * STRUCT DEFINITIONS
 *****************************************************************************************
 */
/* TX and RX channel class holding data used for asynchronous read and write data
 * transactions
 */
/// SPI TX RX Channel
struct spi_txrxchannel
{
	/// size
	uint32_t  size;
	/// buffer pointer
	uint8_t  *bufptr;
	/// call back function pointer
	void (*callback) (uint8_t);
};

/// SPI environment structure
struct spi_env_tag
{
	/// tx channel
	struct spi_txrxchannel tx;
	/// rx channel
	struct spi_txrxchannel rx;	
	/// error detect
	uint8_t errordetect;
};

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
/// spi environment structure
static struct spi_env_tag spi_env __attribute__((section("retention_mem_area0"))); //@WIKRETENTION MEMORY

/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Serves the receive data interrupt requests. It reads incoming data from SPI
 *        while incoming SPI /CS is low, and saves them to the receive buffer
 *
 ****************************************************************************************
 */

static void spi_receive_data_isr(void)
{
	void (*callback) (uint8_t) = NULL;
	
	while (spi_data_rdy_getf())
	{
		SetWord16(SPI_CLEAR_INT_REG, 0x01);				// clean pending flag

		// Read the received in the FIFO
		*spi_env.rx.bufptr = spi_read_byte();
		
		// Update RX parameters
		spi_env.rx.size--;
		spi_env.rx.bufptr++;

		// Check if all expected data have been received
		if (spi_env.rx.size == 0)
		{
			// Reset RX parameters
			spi_env.rx.bufptr = NULL;
			
			// Disable TX interrupt
			NVIC_DisableIRQ(SPI_IRQn);

			
			// Retrieve callback pointer
			callback = spi_env.rx.callback;
			
			if(callback != NULL)
			{
				// Clear callback pointer
				spi_env.rx.callback = NULL;
				
				// Call handler
				callback(SPI_STATUS_OK);
			}
			else
			{
				ASSERT_ERR(0);
			}

			// Exit loop
			break;
		}
	}
}


/**
 ****************************************************************************************
 * @brief Serves the transmit data requests. It activates the DREADY signal to request
 *        transmit to the master, and when master provides with a clock signal, it sends
 *        the data. Upon completion, it deactivates DREADY signal.
 *
 ****************************************************************************************
 */

static void spi_transmit_isr(void)
{
	uint8_t tmp;
	void (*callback) (uint8_t) = NULL;

	while(GetBits16(P0_DATA_REG,0x02)==0);  // Polling CS to detect if data is being received
	
	NVIC_DisableIRQ(SPI_IRQn);							// Disable SPI interrupt
	
	do {
		SetWord16(P0_SET_DATA_REG,1<<7);  				// Activate DREADY signal, request transmit
		while	(GetBits16(SPI_CTRL_REG,SPI_INT_BIT)==0); // polling to wait for spi comple
		tmp = GetWord16(SPI_RX_TX_REG0);					// Get byte from SPI
		SetWord16(SPI_CLEAR_INT_REG, 0x01);				// clean pending flag
		if(tmp != 0x08) {    // if DREADY NOT acknowledged lower DREADY and wait for closed CS
			SetWord16(P0_RESET_DATA_REG,1<<7);  		// Deactivate DREADY signal
			while(GetBits16(P0_DATA_REG,0x02)==0);  // Polling CS to detect if data is being received
		}
	} while (tmp != 0x08);   // If DREADY NOT acknowledged, try again
	
	// Transmit bytes to be written
	while(spi_env.tx.size)
	{
		// Put a byte in the tx fifo
		spi_write_byte(*spi_env.tx.bufptr);
		
		// Update TX parameters
		spi_env.tx.size--;
		spi_env.tx.bufptr++;
		
		if (spi_env.tx.size == 0)
		{
			SetWord16(P0_RESET_DATA_REG,1<<7);  // Deactivate DREADY signal, transmit completed
			
			// Empty SPI Receive FIFO
			while(spi_data_rdy_getf())		// Still data in the SPI FIFO
			{
				GetWord16(SPI_RX_TX_REG0);						// Get byte from SPI FIFO
				SetWord16(SPI_CLEAR_INT_REG, 0x01);		// clean pending flag
			}
			SetWord16(SPI_CLEAR_INT_REG, 0x01);		// clean pending flag
						
			// Reset TX parameters
			spi_env.tx.bufptr = NULL;

			// Retrieve callback pointer
			callback = spi_env.tx.callback;

			if(callback != NULL)
			{
				// Clear callback pointer
				spi_env.tx.callback = NULL;
				
				// Call handler
				callback(SPI_STATUS_OK);
			}
			else
			{
				ASSERT_ERR(0);
			}

			// Exit loop
			break;
		}
	}
	NVIC_EnableIRQ(SPI_IRQn);							// Disable SPI interrupt
}


/*
 * EXPORTED FUNCTION DEFINITIONS
 ****************************************************************************************
 */
extern const uint32_t* const jump_table_base[];

void spi_init_func(void){
	SetBits16(CLK_PER_REG, SPI_DIV, 0);  							// enable  clock for SPI
	SetBits16(CLK_PER_REG, SPI_ENABLE, 1);  					// enable  clock for SPI

	// init SPI
	SetBits16(SPI_CTRL_REG,SPI_ON,0);    	  			// close SPI block, if opened
	SetBits16(SPI_CTRL_REG,SPI_WORD,0);						// set to 8bit mode
	SetBits16(SPI_CTRL_REG,SPI_SMN, 0x01); 				// Set SPI IN SLAVE MODE
	SetBits16(SPI_CTRL_REG,SPI_POL, 0x0);  				// MODE 3: SPI_POL = 1
	SetBits16(SPI_CTRL_REG,SPI_PHA, 0x0);					//     and SPI_PHA = 1
	SetBits16(SPI_CTRL_REG,SPI_MINT, 0x1);  			// SPI Maskable Interrupt enable
 	SetBits16(SPI_CTRL_REG1,SPI_FIFO_MODE,0x00);	// enable SPI rx and tx fifos
 	SetBits16(SPI_CTRL_REG,SPI_EN_CTRL,1);				// enable SPI block	
	SetBits16(SPI_CTRL_REG,SPI_ON,1);    	  			// enable SPI block
	
	// Configure SPI environment
	spi_env.errordetect = SPI_ERROR_DETECT_DISABLED;
	spi_env.rx.bufptr = NULL;
	spi_env.rx.size = 0;
	spi_env.tx.bufptr = NULL;
	spi_env.tx.size = 0;
	NVIC_SetPriority(SPI_IRQn,0);
}


void spi_flow_on_func(void)
{
	uint8_t tmp;

	while(GetBits16(P0_DATA_REG,0x02)==0);  // Polling CS to detect if data is being received
	
	NVIC_DisableIRQ(SPI_IRQn);							// Disable SPI interrupt
	
	do {
		SetWord16(P0_SET_DATA_REG,1<<7);  				// Activate DREADY signal, request transmit
		while	(GetBits16(SPI_CTRL_REG,SPI_INT_BIT)==0); // polling to wait for spi comple
		tmp = GetWord16(SPI_RX_TX_REG0);					// Get byte from SPI
		SetWord16(SPI_CLEAR_INT_REG, 0x01);				// clean pending flag
		if(tmp != 0x08) {    // if DREADY NOT acknowledged lower DREADY and wait for closed CS
			SetWord16(P0_RESET_DATA_REG,1<<7);  		// Deactivate DREADY signal
			while(GetBits16(P0_DATA_REG,0x02)==0);  // Polling CS to detect if data is being received
		}
	} while (tmp != 0x08);   // If DREADY NOT acknowledged, try again
	
	while (GetBits16(SPI_CTRL_REG,SPI_TXH)==1); 		// check for full SPI Tx FIFO
	SetWord16(SPI_RX_TX_REG0,0x06);  								// Send flow on byte
	while	(GetBits16(SPI_CTRL_REG,SPI_INT_BIT)==0); // polling to wait for SPI completion
	GetWord16(SPI_RX_TX_REG0);											// Read byte in SPI Rx FIFO
	SetWord16(SPI_CLEAR_INT_REG, 0x01);							// clean pending flag
	while (GetBits16(SPI_CTRL_REG1,SPI_BUSY)==1);
	SetWord16(P0_RESET_DATA_REG,1<<7);  						// Deactivate DREADY signal, transmit completed

	NVIC_ClearPendingIRQ(SPI_IRQn);			// Clear interrupt requests while disabled
	NVIC_EnableIRQ(SPI_IRQn);						// Disable spi interrupt, if MINT is '1'
}

bool spi_flow_off_func(void)
{
	uint8_t tmp;
	
	while(GetBits16(P0_DATA_REG,0x02)==0);  // Polling CS to detect if data is being received
	
	NVIC_DisableIRQ(SPI_IRQn);							// Disable SPI interrupt
	
	do {
		SetWord16(P0_SET_DATA_REG,1<<7);  				// Activate DREADY signal, request transmit
		while	(GetBits16(SPI_CTRL_REG,SPI_INT_BIT)==0); // polling to wait for spi comple
		tmp = GetWord16(SPI_RX_TX_REG0);					// Get byte from SPI
		SetWord16(SPI_CLEAR_INT_REG, 0x01);				// clean pending flag
		if(tmp != 0x08) {    // if DREADY NOT acknowledged lower DREADY and wait for closed CS
			SetWord16(P0_RESET_DATA_REG,1<<7);  		// Deactivate DREADY signal
			while(GetBits16(P0_DATA_REG,0x02)==0);  // Polling CS to detect if data is being received
		}
	} while (tmp != 0x08);   // If DREADY NOT acknowledged, try again
	
	while (GetBits16(SPI_CTRL_REG,SPI_TXH)==1); 		// check for full SPI Tx FIFO
	SetWord16(SPI_RX_TX_REG0,0x07);  								// Send flow on byte
	while	(GetBits16(SPI_CTRL_REG,SPI_INT_BIT)==0); // polling to wait for SPI completion
	GetWord16(SPI_RX_TX_REG0);											// Read byte in SPI Rx FIFO
	SetWord16(SPI_CLEAR_INT_REG, 0x01);							// clean pending flag
	while (GetBits16(SPI_CTRL_REG1,SPI_BUSY)==1);
	SetWord16(P0_RESET_DATA_REG,1<<7);  						// Deactivate DREADY signal, transmit completed

	return 1;
}


void spi_read_func(uint8_t *bufptr, uint32_t size,void (*callback) (uint8_t))
{
	// Sanity check
	ASSERT_ERR(bufptr != NULL);
	ASSERT_ERR(size != 0);
	ASSERT_ERR(spi_env.rx.bufptr == NULL);
	
	// Prepare RX parameters
	spi_env.rx.size = size;
	spi_env.rx.bufptr = bufptr;
	spi_env.rx.callback = callback; 
	
	NVIC_ClearPendingIRQ(SPI_IRQn);	// Clear interrupt requests while disabled
	NVIC_EnableIRQ(SPI_IRQn);				// Enable spi interrupt
}


void spi_write_func(uint8_t *bufptr, uint32_t size, void (*callback) (uint8_t))
{
	// Sanity check
	ASSERT_ERR(bufptr != NULL);
	ASSERT_ERR(size != 0);
	ASSERT_ERR(spi_env.tx.bufptr == NULL);	
	
	// Prepare TX parameters
	spi_env.tx.size = size;
	spi_env.tx.bufptr = bufptr;
	spi_env.tx.callback = callback; 
	
	/* start data transaction
	 * first isr execution is done without interrupt generation to reduce
	 * interrupt load
	 */
		
	spi_flow_off_func();										// Send flow off character
	
	spi_transmit_isr();											// Send data over SPI

	spi_flow_on_func();											// Send flow on character
}

void SPI_Handler_func(void)
{
	spi_receive_data_isr();
}

void SPI_Handler(void)
{ 
    SPI_Handler_func();
}
