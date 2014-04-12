#ifndef _HCI_SPI_
#define _HCI_SPI_

/**
 ****************************************************************************************
 *
 * @file spi_hci.h
 *
 * @brief Header file for SPI Driver for HCI over SPI operation.
 *
 * $Rev: 1 $
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>       // integer definition
#include <stdbool.h>      // boolean definition

/*
 * DEFINES
 *****************************************************************************************
 */

/*
 * ENUMERATION DEFINITIONS
 *****************************************************************************************
 */

/* Error detection */
enum
{
    /// error detection disabled
    SPI_ERROR_DETECT_DISABLED = 0,
    /// error detection enabled
    SPI_ERROR_DETECT_ENABLED  = 1
};

/// status values
enum
{
    /// status ok
    SPI_STATUS_OK,
    /// status not ok
    SPI_STATUS_ERROR
};

/// SPI Slave state
enum
{
    /// Slave On Sleep
    SLAVE_IDLE,
    /// Slave Active
    SLAVE_RECEIVE,
 
};

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initializes the SPI to default values.
 *****************************************************************************************
 */
void spi_init_func(void);

#ifndef CFG_ROM
/**
 ****************************************************************************************
 * @brief Enable SPI flow.
 *****************************************************************************************
 */
void spi_flow_on_func(void);

/**
 ****************************************************************************************
 * @brief Disable SPI flow.
 *****************************************************************************************
 */
bool spi_flow_off_func(void);
#endif //CFG_ROM

/**
 ****************************************************************************************
 * @brief Starts a data reception.
 *
 * As soon as the end of the data transfer or a buffer overflow is detected,
 * the hci_uart_rx_done function is executed.
 *
 * @param[in,out]  bufptr Pointer to the RX buffer
 * @param[in]      size   Size of the expected reception
 *****************************************************************************************
 */
void spi_read_func(uint8_t *bufptr, uint32_t size, void (*callback) (uint8_t));

/**
 ****************************************************************************************
 * @brief Starts a data transmission.
 *
 * As soon as the end of the data transfer is detected, the hci_uart_tx_done function is
 * executed.
 *
 * @param[in]  bufptr Pointer to the TX buffer
 * @param[in]  size   Size of the transmission
 *****************************************************************************************
 */
void spi_write_func(uint8_t *bufptr, uint32_t size, void (*callback) (uint8_t));


/// @} HCI_SPI
#endif // _HCI_SPI_
