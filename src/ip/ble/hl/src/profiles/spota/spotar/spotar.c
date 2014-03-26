/**
 ****************************************************************************************
 *
 * @file spotar.c
 *
 * @brief Software Programming Over The Air (SPOTA) Receiver Implementation.
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
 * @addtogroup SPOTAR
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwble_config.h"

#if (BLE_SPOTA_RECEIVER)

#include "gapc.h"
#include "spotar.h"
#include "spotar_task.h"
#include "attm_db.h"
#include "gap.h"
/*
 * SPOTA PROFILE ATTRIBUTES DEFINITION
 ****************************************************************************************
 */

// Use the Manufacturer test UUID value as a base UUID for SPOTA service until we get a valid UUID value via SIG.
#define SPOTA_BASE_UUID ATT_SVC_MANUF
#define SPOTA_MEM_DEV_UUID (SPOTA_BASE_UUID + 1 )
#define SPOTA_GPIO_MAP_UUID (SPOTA_MEM_DEV_UUID + 1)
#define SPOTA_MEM_INFO_UUID (SPOTA_GPIO_MAP_UUID + 1 )
#define SPOTA_PATCH_LEN_UUID (SPOTA_MEM_INFO_UUID + 1)
#define SPOTA_PATCH_DATA_UUID (SPOTA_PATCH_LEN_UUID + 1)
#define SPOTA_SERV_STATUS_UUID (SPOTA_PATCH_DATA_UUID + 1)

const char pdm_value_descr[] = "Mem Dev: B3=Mem, B[2:0]=Addr";
const char pdgpio_value_descr[] = "GPIO: b[3:0]= Pin, b[7:4]= Port Number";
const char pdmi_value_descr[] = "Mem Info: B[3:2]=#Patches, B[1:0]=Entire_Length";
const char pd_value_descr[] = "Patch Data: 20 bytes";
const char pdl_value_descr[] = "New patch len: 16 bits";
const char pds_value_descr[] = "SPOTA serv status: 8 bits";



/// Full SPOTA Database Description - Used to add attributes into the database
const struct attm_desc spotar_att_db[SPOTA_IDX_NB] =
{    
    // SPOTA Service Declaration
    [SPOTA_IDX_SVC]                 =   {ATT_DECL_PRIMARY_SERVICE, PERM(RD, ENABLE), sizeof(spotar_svc),
                                            sizeof(spotar_svc), (uint8_t *)&spotar_svc},
       
    /* 
    ** Memory Device Characteristic 
    */
    // Patch Memory Device Characteristic Declaration                                         
    [SPOTA_IDX_PATCH_MEM_DEV_CHAR]  =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), sizeof(spotar_patch_mem_dev_char),
                                            sizeof(spotar_patch_mem_dev_char), (uint8_t *)&spotar_patch_mem_dev_char},
    // Patch Memory Device Characteristic Value
    [SPOTA_IDX_PATCH_MEM_DEV_VAL]      =   {SPOTA_MEM_DEV_UUID, PERM(RD, ENABLE) | PERM(WR, ENABLE), sizeof(uint32_t),
                                                0, NULL},
    // Patch Memory Device Characteristic Value description
    [SPOTA_IDX_PATCH_MEM_DEV_VAL_DESCR]  =  {ATT_DESC_CHAR_USER_DESCRIPTION, PERM(RD, ENABLE), sizeof(pdm_value_descr),
                                                sizeof(pdm_value_descr), (uint8_t *)pdm_value_descr},                                               
                              
    /* 
    ** GPIO mapping for accessing SPI or I2C mem device Characteristic 
    */
    [SPOTA_IDX_GPIO_MAP_CHAR] = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), sizeof(spotar_gpio_map_char),
                                            sizeof(spotar_gpio_map_char), (uint8_t *)&spotar_gpio_map_char},
    [SPOTA_IDX_GPIO_MAP_VAL] =  {SPOTA_GPIO_MAP_UUID, PERM(RD, ENABLE) | PERM(WR, ENABLE), sizeof(uint32_t),
                                                0, NULL},
    [SPOTA_IDX_GPIO_MAP_VAL_DESCR] = {ATT_DESC_CHAR_USER_DESCRIPTION, PERM(RD, ENABLE), sizeof(pdgpio_value_descr),
                                                sizeof(pdgpio_value_descr), (uint8_t *)pdgpio_value_descr},
                                                
    /* 
    ** Memory Information Characteristic 
    */
    // Patch Memory Information Characteristic Declaration                                         
    [SPOTA_IDX_PATCH_MEM_INFO_CHAR]  =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), sizeof(spotar_patch_mem_info_char),
                                            sizeof(spotar_patch_mem_info_char), (uint8_t *)&spotar_patch_mem_info_char},
    // Patch Memory Information Characteristic Value
    [SPOTA_IDX_PATCH_MEM_INFO_VAL]      =   {SPOTA_MEM_INFO_UUID, PERM(RD, ENABLE), sizeof(uint32_t),
                                                0, NULL},
    // Patch Memory Information Characteristic Value description
    [SPOTA_IDX_PATCH_MEM_INFO_VAL_DESCR]  =  {ATT_DESC_CHAR_USER_DESCRIPTION, PERM(RD, ENABLE), sizeof(pdmi_value_descr),
                                                sizeof(pdmi_value_descr), (uint8_t *)pdmi_value_descr},                                                
                                                
    /* 
    ** Patch Length Characteristic 
    */
    // Patch Length Characteristic Declaration
    [SPOTA_IDX_PATCH_LEN_CHAR]     =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), sizeof(spotar_patch_len_char),
                                         sizeof(spotar_patch_len_char), (uint8_t *)&spotar_patch_len_char},
    // Patch Length Characteristic Value
    [SPOTA_IDX_PATCH_LEN_VAL]      =   {SPOTA_PATCH_LEN_UUID, PERM(RD, ENABLE) | PERM(WR, ENABLE), sizeof(uint16_t),
                                                0, NULL},
    // Patch Length Characteristic Value description
    [SPOTA_IDX_PATCH_LEN_VAL_DESCR]  =  {ATT_DESC_CHAR_USER_DESCRIPTION, PERM(RD, ENABLE), sizeof(pdl_value_descr),
                                                sizeof(pdl_value_descr), (uint8_t *)pdl_value_descr},

    /* 
    ** Patch Data Characteristic 
    */
    // Patch Data Characteristic Declaration
    [SPOTA_IDX_PATCH_DATA_CHAR]     =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), sizeof(spotar_patch_data_char),
                                            sizeof(spotar_patch_data_char), (uint8_t *)&spotar_patch_data_char},
    // Patch Data Characteristic Value
    [SPOTA_IDX_PATCH_DATA_VAL]      =   {SPOTA_PATCH_DATA_UUID, PERM(RD, ENABLE) | PERM(WR, ENABLE), ((sizeof(uint8_t)) * SPOTA_PD_CHAR_SIZE),
                                         0, NULL},
    // Patch Data Characteristic Value description
    [SPOTA_IDX_PATCH_DATA_VAL_DESCR]      =   {ATT_DESC_CHAR_USER_DESCRIPTION, PERM(RD, ENABLE), sizeof(pd_value_descr),
                                                sizeof(pd_value_descr), (uint8_t *)pd_value_descr},
                                              
    /* 
    ** Patch Status Characteristic 
    */
    // Patch Status Characteristic Declaration
    [SPOTA_IDX_PATCH_STATUS_CHAR]     =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), sizeof(spotar_patch_status_char),
                                         sizeof(spotar_patch_status_char), (uint8_t *)&spotar_patch_status_char},
    // Patch Status Characteristic Value
    [SPOTA_IDX_PATCH_STATUS_VAL]      =   {SPOTA_SERV_STATUS_UUID, (PERM(RD, ENABLE) | PERM(NTF, ENABLE)), sizeof(uint8_t),
                                                0, NULL},
                                                                                               
    // Patch Status Characteristic Value description
    [SPOTA_IDX_PATCH_STATUS_NTF_CFG] = {ATT_DESC_CLIENT_CHAR_CFG,(PERM(RD, ENABLE) | PERM(WR, ENABLE)), sizeof(uint16_t), 
                                            0, (uint8_t *)NULL},                                             
                                            
    // Patch Status Characteristic Value description
    [SPOTA_IDX_PATCH_STATUS_VAL_DESCR]  =  {ATT_DESC_CHAR_USER_DESCRIPTION, PERM(RD, ENABLE), sizeof(pds_value_descr),
                                                sizeof(pds_value_descr), (uint8_t *)pds_value_descr},                                                


};


/*
 *  SPOTA PROFILE ATTRIBUTES VALUES DEFINTION
 ****************************************************************************************
 */

/// SPOTA Service
const att_svc_desc_t spotar_svc     = SPOTA_BASE_UUID;

/// SPOTA Service - Patch Memory Device Characteristic
const struct att_char_desc spotar_patch_mem_dev_char = ATT_CHAR(ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR,
                                                                    0,
                                                                    SPOTA_MEM_DEV_UUID);

/// SPOTA Service - GPIO mapping Characteristic
const struct att_char_desc spotar_gpio_map_char = ATT_CHAR(ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR,
                                                                    0,
                                                                    SPOTA_GPIO_MAP_UUID);

/// SPOTA Service - Patch Memory Information Characteristic
const struct att_char_desc spotar_patch_mem_info_char = ATT_CHAR(ATT_CHAR_PROP_RD,
                                                                    0,
                                                                    SPOTA_MEM_INFO_UUID);
                                                                    
                                                                                                                                        
/// SPOTA Service - Patch Length Characteristic
const struct att_char_desc spotar_patch_len_char = ATT_CHAR(ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR,
                                                                    0,
                                                                    SPOTA_PATCH_LEN_UUID);

/// SPOTA Service - Patch Data Characteristic
const struct att_char_desc spotar_patch_data_char = ATT_CHAR(ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR,
                                                                    0,
                                                                    SPOTA_PATCH_DATA_UUID);


/// SPOTA Service - Patch Status Characteristic
const struct att_char_desc spotar_patch_status_char = ATT_CHAR(ATT_CHAR_PROP_RD | ATT_CHAR_PROP_NTF,
                                                                    0,
                                                                    SPOTA_SERV_STATUS_UUID);


/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

struct spotar_env_tag spotar_env __attribute__((section("exchange_mem_case1")));

/// SPOTAR task descriptor
static const struct ke_task_desc TASK_DESC_SPOTAR = {spotar_state_handler, &spotar_default_handler, spotar_state, SPOTAR_STATE_MAX, SPOTAR_IDX_MAX};


/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void spotar_init(void)
{
    // Reset Environment
    memset(&spotar_env, 0, sizeof(spotar_env));

    // Create SPOTAR task
    ke_task_create(TASK_SPOTAR, &TASK_DESC_SPOTAR);

    ke_state_set(TASK_SPOTAR, SPOTAR_DISABLED);
}

void spotar_patched_data_rcved(uint8_t char_code)
{
    // Allocate the Patch Data value change indication
   struct spotar_patch_data_ind *ind = KE_MSG_ALLOC(SPOTAR_PATCH_DATA_IND,
                                              spotar_env.con_info.appid, TASK_SPOTAR,
                                              spotar_patch_data_ind);
   // Fill in the parameter structure
   ind->conhdl =  gapc_get_conhdl(spotar_env.con_info.conidx);

   ind->char_code = char_code;

   // Send the message
   ke_msg_send(ind);
}

void spotar_disable(uint16_t conhdl)
{
    // Disable SPOTA service in database
    attmdb_svc_set_permission(spotar_env.spota_shdl, PERM_RIGHT_DISABLE);

    struct spotar_disable_ind *ind = KE_MSG_ALLOC(SPOTAR_DISABLE_IND,
                                                 spotar_env.con_info.appid, TASK_SPOTAR,
                                                 spotar_disable_ind);
    
    ind->conhdl  = conhdl;
    
    // Send the message
    ke_msg_send(ind);

    // Go to idle state
    ke_state_set(TASK_SPOTAR, SPOTAR_IDLE);
}

#endif //BLE_PROX_REPORTER

/// @} SPOTAR
