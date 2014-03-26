/**
 ****************************************************************************************
 *
 * @file l2cc.h
 *
 * @brief Header file - L2CC.
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
 *
 ****************************************************************************************
 */

#ifndef L2CC_H_
#define L2CC_H_

/**
 ****************************************************************************************
 * @addtogroup L2CC L2CAP Controller
 * @ingroup L2C
 * @brief L2CAP block for data processing and per device connection
 *
 * The L2CC is responsible for all the data processing related
 * functions of the L2CAP block per device connection.
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "l2cm.h"
#if (BLE_CENTRAL || BLE_PERIPHERAL)
/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */
/* Flag for connection handle function */
#define L2C_RST_NB_CMPLT_PKTS               0x00
#define L2C_GET_NB_CMPLT_PKTS               0x01
#define L2C_INC_NB_CMPLT_PKTS               0x02
#define L2C_DEC_NB_CMPLT_PKTS               0x03
/// Maximum number of instances of the L2CC task
#define L2CC_IDX_MAX                        BLE_CONNECTION_MAX

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Create and Initialize the L2CAP controller task.
 *
 * @param[in] reset   true if it's requested by a reset; false if it's boot initialization
 *
 ****************************************************************************************
 */
void l2cc_init(bool reset);

/**
 ****************************************************************************************
 * @brief Initialize the link layer controller task.
 *
 * @param[in] conidx            Connection index
 *
 ****************************************************************************************
 */
void l2cc_create(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief De-initialize the task.
 *
 * @param[in] conidx            Connection index
 *
 ****************************************************************************************
 */
void l2cc_cleanup(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Update state of all L2Cap controller task according to number of buffer
 * available
 ****************************************************************************************
 */
void l2cc_update_state(void);

#endif /* #if (BLE_CENTRAL || BLE_PERIPHERAL) */
/// @} L2CC
#endif // L2CC_H_
