/**
****************************************************************************************
*
* @file app_alt_pair.h
*
* @brief Push Button and LED handling header file.
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

#ifndef APP_ALT_PAIR_H_
#define APP_ALT_PAIR_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "app.h" 

/*
 * DEFINES
 ****************************************************************************************
 */

#define EEPROM_BOND_PEER_NUM_ADDR   0x00
#define EEPROM_BOND_DATA_ADDR       0x04

#define MAX_BOND_PEER               0x07

extern uint8_t alt_pair_enabled;
extern bool alt_pair_adv_mode;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
 
void app_alt_pair_disconnect(void);

int app_alt_pair_timer_handler(void);

int app_directed_adv_timer_handler(void);

bool app_alt_pair_check_peer(struct bd_addr *peer_addr, uint8_t peer_addr_type);

void app_alt_pair_read_peer_number(void);

void app_alt_pair_store_bond_data(void);

void app_alt_pair_load_bond_data(struct rand_nb *rand_nb, uint16_t ediv);

bool app_alt_pair_get_next_bond_data(bool init);

void app_alt_pair_clear_all_bond_data(void);

#endif // APP_ALT_PAIR_H_
