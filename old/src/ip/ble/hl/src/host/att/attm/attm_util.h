/**
 ****************************************************************************************
 *
 * @file attm_util.h
 *
 * @brief Header file - ATTMUTIL.
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
 *
 ****************************************************************************************
 */

#ifndef ATTM_UTIL_H_
#define ATTM_UTIL_H_
/**
 ****************************************************************************************
 * @addtogroup ATTMUTIL Utility
 * @ingroup ATTM
 * @brief Contains utility functions and macros for ATTM
 *
 * The ATTMUTIL is responsible for providing utility functions for
 * @ref ATTM "ATTM", in order to accomplish defined Generic Attribute
 * Profile functionalities.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "ke_task.h"
#include "co_bt.h"
#include "attm.h"

#if (BLE_CENTRAL || BLE_PERIPHERAL)

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Construct PDU and send characteristic value confirm.
 * This is attribute client side.
 *
 * @param[in] idx   connection record index
 *
 ****************************************************************************************
 */
void
attm_send_char_val_cfm(uint8_t idx);


/**
 ****************************************************************************************
 * @brief Function use to ease service database creation.
 *
 * Use @see attmdb_add_service function of attmdb module to create service database,
 * then use @see attmdb_add_attribute function of attmdb module to create attributes
 * according to database description array given in parameter.
 *
 * @note: database description array shall be const to reduce memory consuption (only ROM)
 * @note: It supports only 16 bits UUIDs
 *
 * @param[in|out] start_hdl Service start handle.
 * @param[in]     task_id   Task that manages service.
 * @param[in]     nb_att    Number of attributes in the database
 * @param[in]     db_desc   Database description pointer
 *
 * @return Command status code:
 *  - @ref ATT_ERR_NO_ERROR: If database creation succeeds.
 *  - @ref ATT_ERR_INVALID_HANDLE: If start_hdl given in parameter + nb of attribute override
 *                            some existing services handles.
 *  - @ref ATT_ERR_INSUFF_RESOURCE: There is not enough memory to allocate service buffer.
 *                           or of new attribute cannot be added because all expected
 *                           attributes already added or buffer overflow detected during
 *                           allocation
 ****************************************************************************************
 */
uint8_t attm_svc_create_db(uint16_t *shdl, uint8_t *cfg_flag, uint8_t max_nb_att,
                           uint8_t *att_tbl, ke_task_id_t const dest_id,
                           const struct attm_desc *att_db);
#endif // #if (BLE_CENTRAL || BLE_PERIPHERAL)
/// @} ATTMUTIL
#endif // ATTM_UTIL_H_
