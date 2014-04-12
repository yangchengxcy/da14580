/**
****************************************************************************************
*
* @file app_multi_bond.h
*
* @brief Special (multi) bonding procedure header file.
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

#ifndef APP_MULTI_BOND_H_
#define APP_MULTI_BOND_H_

/*
 * USAGE
 *
 * To use this module the following must be properly defined in your project and include the header file in app_multi_bond.c:
 *     ALT_PAIR_DISCONN_TIME : Time to block previous host during a "host-switch" (in 10th of msec) 
 *
 * and the following must be defined as (1) if used or (0) if not used:
 *     HAS_MULTI_BOND : set to (1) if multiple bonding is to be used.
 *     HAS_EEPROM : if there's no EEPROM then using this module is useless.
 *     HAS_MITM : set to (1) if MITM is used or (0) if no MITM is supported.
 *
 * Note that the following configuration makes sense:
 *     HAS_MULTI_BOND (0) and HAS_EEPROM (1)
 * meaning that only one bond will exist in EEPROM without the possibility of switching.
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>
#include <stdbool.h>
#include "app.h" 
#include "co_bt.h"

/*
 * DEFINES
 ****************************************************************************************
 */

#define EEPROM_BONDING_STATUS_ADDR  0x00
#define EEPROM_BOND_DATA_ADDR       0x04

#define MAX_BOND_PEER               0x07

extern uint8_t multi_bond_enabled;
extern uint8_t multi_bond_status;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
 
bool app_alt_pair_disconnect(void);

int app_alt_pair_timer_handler(void);

bool app_alt_pair_check_peer(struct bd_addr *peer_addr, uint8_t peer_addr_type);

void app_alt_pair_read_status(void);

void app_alt_pair_store_bond_data(void);

int app_alt_pair_load_bond_data(struct rand_nb *rand_nb, uint16_t ediv);

bool app_alt_pair_get_next_bond_data(bool init);

void app_alt_pair_clear_all_bond_data(void);

#endif // APP_MULTI_BOND_H_
