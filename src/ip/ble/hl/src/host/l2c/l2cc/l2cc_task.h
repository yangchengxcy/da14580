/**
 ****************************************************************************************
 *
 * @file l2cc_task.h
 *
 * @brief Header file - L2CCTASK.
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
 *
 ****************************************************************************************
 */

#ifndef L2CC_TASK_H_
#define L2CC_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup L2CCTASK Task
 * @ingroup L2CC
 * @brief Handles ALL messages to/from L2CC block.
 *
 * The L2CC task is responsible for L2CAP attribute and security block handling.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"

#if (BLE_L2CC)
#include "co_buf.h"
#include "llm_task.h"
#include "llc_task.h"
#include "l2cc.h"


/*
 * MACRO DEFINITIONS
 ****************************************************************************************
 */
#define L2CC_PDU_TO_PACKET(pdu) \
    ((struct l2cc_pdu_send_req*) (((uint8_t*) pdu ) - (2*sizeof(uint16_t))))


/*
 * STATES
 ****************************************************************************************
 */
/// L2CC states
enum
{
    /// Free state
    L2CC_FREE,
    /// Connection ready state
    L2CC_READY,
    /// Connection busy state (operation on going)
    L2CC_BUSY,

    /// Total number of defined L2CC states */
    L2CC_STATE_MAX
};

/*
 * MESSAGES
 ****************************************************************************************
 */
/// Message API of the L2CC task
enum l2cc_msg_id
{
    /// Send a PDU packet
    L2CC_PDU_SEND_REQ = KE_FIRST_MSG(TASK_L2CC),
    /// Reception of a PDU packet
    L2CC_PDU_RECV_IND,
    /// Inform that packet been sent.
    L2CC_DATA_SEND_RSP,
};

/// pass from l2cc to upper layer
struct l2cc_data_send_rsp
{
    /// Status of request.
    uint8_t status;
};

/// Send a PDU packet
struct l2cc_pdu_send_req
{
    /// PDU data
    struct l2cc_pdu pdu;
};

/// Send a PDU packet
struct l2cc_pdu_recv_ind
{
    /// PDU data
    struct l2cc_pdu pdu;
    /// reception status code.
    uint8_t status;
};
/*
 * TASK DESCRIPTOR DECLARATIONS
 ****************************************************************************************
 */
extern const struct ke_state_handler l2cc_state_handler[L2CC_STATE_MAX];
extern const struct ke_state_handler l2cc_default_handler;
extern ke_state_t l2cc_state[L2CC_IDX_MAX];
#endif // #if (BLE_L2CC)
/// @} L2CCTASK
#endif // L2CC_TASK_H_
