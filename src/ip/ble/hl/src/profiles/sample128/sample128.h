/**
 ****************************************************************************************
 *
 * @file sample128.h
 *
* @brief 128 UUID service. sample code
 *
 * Copyright (C) 2013 Dialog Semiconductor GmbH and its Affiliates, unpublished work
 * This computer program includes Confidential, Proprietary Information and is a Trade Secret 
 * of Dialog Semiconductor GmbH and its Affiliates. All use, disclosure, and/or 
 * reproduction is prohibited unless authorized in writing. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SAMPLE128_H_
#define SAMPLE128_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#if (BLE_SAMPLE128)

#include "ke_task.h"
#include "atts.h"
#include "prf_types.h"

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// LLS Handles offsets
enum
{
    SAMPLE128_1_IDX_SVC,

    SAMPLE128_1_IDX_CHAR,
    SAMPLE128_1_IDX_VAL,

    SAMPLE128_1_IDX_NB,
};

///Characteristics Code for Write Indications
enum
{
    SAMPLE128_ERR_CHAR,
    SAMPLE128_1_CHAR,
};

/*
 * STRUCTURES
 ****************************************************************************************
 */

/// sample128 environment variable
struct sample128_env_tag
{
    /// Connection Information
    struct prf_con_info con_info;

    /// LLS Start Handle
    uint16_t sample128_1_shdl;

};

/*
 * SAMPLE128 PROFILE ATTRIBUTES DECLARATION
 ****************************************************************************************
 */

/// Sample 128 service Description
//extern const struct atts_desc sample128_1_att_db[SAMPLE128_1_IDX_NB];

/*
 *  SAMPLE128 PROFILE ATTRIBUTES VALUES DECLARATION
 ****************************************************************************************
 */

/// sample128_1 Service
extern const struct att_uuid_128 sample128_1_svc;
/// sample128_1 - Alert Level Characteristic
extern struct att_char128_desc sample128_1_char;

extern const struct att_uuid_128 sample128_1_val;



/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */

extern struct sample128_env_tag sample128_env;
/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialization of the SAMPLE128 module.
 * This function performs all the initializations of the SAMPLE128 module.
 ****************************************************************************************
 */
void sample128_init(void);

/**
 ****************************************************************************************
 * @brief Send value change to application.
 * @param val Value.
 ****************************************************************************************
 */
 
void sample128_send_val(uint8_t val);

/**
 ****************************************************************************************
 * @brief Disable role.
 ****************************************************************************************
 */
void sample128_disable(void);

#endif //BLE_SAMPLE128

#endif // SAMPLE128_H_
