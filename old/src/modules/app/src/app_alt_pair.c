/**
****************************************************************************************
*
* @file app_alt_pair.c
*
* @brief Special pairing procedure.
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
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "app.h"                     // application definitions
#include "app_task.h"                // application task definitions
#include "app_sec.h"                
#include <string.h>                  // string manipulation and functions
#include "co_bt.h"
#include "arch.h"                      // platform definitions
#include "ke_timer.h"                   // kernel timer 
#include "app_button_led.h"          // button/LED function definitions
#include "rwble_config.h"
#include "gpio.h"
#include "app_alt_pair.h"
#include "i2c_eeprom.h"


#if (BLE_APP_PRESENT)
#if (BLE_ALT_PAIR)

uint32_t alt_pair_peer_number       __attribute__((section("exchange_mem_case1")));
uint8_t alt_pair_enabled            __attribute__((section("exchange_mem_case1"))); 
uint8_t alt_pair_active_peer_pos    __attribute__((section("exchange_mem_case1"))); 
uint8_t alt_pair_next_peer_pos      __attribute__((section("exchange_mem_case1"))); 
bool alt_pair_adv_mode              __attribute__((section("exchange_mem_case1"))); 

extern struct app_sec_env_tag app_sec_env;

#include "app_alt_pair.h"


void app_alt_pair_disconnect(void)
{
    if((app_sec_env.auth & GAP_AUTH_BOND))
    {        
        alt_pair_enabled = 1;
    
        ke_timer_set(APP_ALT_PAIR_TIMER, TASK_APP, 6000);	//60 seconds
    }
    
    app_disconnect();
}


int app_alt_pair_timer_handler(void)
{
    alt_pair_enabled = 0;
    
    return (KE_MSG_CONSUMED);
}


int app_directed_adv_timer_handler(void)
{
    //TODO: This can cause too large reconnect delay. Can be commented out so that directed advertising
    // is done only once to the first known host. If it fails then normal advertising is done. (VK)
    //TODO: What if we decrease the number of entries in EEPROM to 2? Then we would delay at most 
    // 2 * 1.28s during boot and 1 * 1.28s during 'Fn'+'p'. This needs approval. (VK)
    
    if (alt_pair_adv_mode)
    {
        alt_pair_adv_mode = false;
        if (ke_state_get(TASK_APP) == APP_CONNECTABLE)
        {
    //        if (!app_alt_pair_get_next_bond_data(alt_pair_adv_mode)) 
                memset(&app_sec_env, 0, sizeof(app_sec_env));
    //        else
    //            ke_timer_set(APP_DIR_ADV_TIMER, TASK_APP, 128);	//1.28 seconds
                
            // do directed advertising to the next known host (if app_sec_env is valid) or
            // do normal advertising (if app_sec_env is invalid)
            // Note: advertising will be restarted when GAPM_CMP_EVT is get with param.operation == GAPM_ADV_DIRECT
            app_adv_stop();
        }
    }
    
    return (KE_MSG_CONSUMED);
}


bool app_alt_pair_check_peer(struct bd_addr *peer_addr, uint8_t peer_addr_type)
{
    if (alt_pair_enabled)
    {
        if ( (peer_addr_type == app_sec_env.peer_addr_type) && 
             (!memcmp(app_sec_env.peer_addr.addr, peer_addr->addr, BD_ADDR_LEN)) &&
             (app_sec_env.auth & GAP_AUTH_BOND) )
        {
            app_disconnect();
            
            return false;
        }
    }
    
    return true;
}


//TODO: Maybe this is used to store the position of the last connected bonded host. This way, after a reset,
// we would try to connect to it first. (VK)/
void app_alt_pair_read_peer_number(void)
{
    i2c_init();
    
    sequential_read_i2c_eeprom( (uint8_t *) &alt_pair_peer_number, EEPROM_BOND_PEER_NUM_ADDR, sizeof(struct app_sec_env_tag));
    
    i2c_release();
}


void app_alt_pair_store_bond_data(void)
{
    struct app_sec_env_tag tmp_sec_env;
    struct app_sec_env_tag *peeprom_sec_env = (struct app_sec_env_tag *) EEPROM_BOND_DATA_ADDR;
    struct app_sec_env_tag *pempty_sec_env = 0;
    uint8_t *addr_p;
    uint8_t addr;
    uint32_t i;
    uint8_t found = 0;
    
    i2c_init();
    
    //find entry 
    for (i = 0; i < MAX_BOND_PEER; i++)
    {
        addr_p = (uint8_t *)&peeprom_sec_env;
        addr = *addr_p;
        sequential_read_i2c_eeprom( (uint8_t *) &tmp_sec_env, addr, sizeof(struct app_sec_env_tag));
        
        if (!(tmp_sec_env.auth & GAP_AUTH_BOND) && (pempty_sec_env == 0))
        {
           pempty_sec_env = peeprom_sec_env;   
        }
        
        if ((tmp_sec_env.ediv == app_sec_env.ediv) && (!memcmp(&app_sec_env.rand_nb, &tmp_sec_env.rand_nb, RAND_NB_LEN)))
        {
           found = 1;
           break; 
        }
        else if ((tmp_sec_env.peer_addr_type == app_env.peer_addr_type) && (!memcmp(&app_env.peer_addr, &tmp_sec_env.peer_addr, BD_ADDR_LEN)))
        {
           found = 1;
           break; 
        }
        
        peeprom_sec_env++;
    }    
        
    if (found) ///entry for peer exists. Overwite
    {
        write_page_i2c_eeprom( addr, (uint8_t *)&app_sec_env, sizeof(struct app_sec_env_tag) );
    }
    else ///entry for peer does not exist. Write to first empty. If there is no space rewrite first entry.
    {
        if (pempty_sec_env == 0) //if buffer is full rewrite first entry
            pempty_sec_env = (struct app_sec_env_tag *) EEPROM_BOND_DATA_ADDR;
        
        addr_p = (uint8_t *)&pempty_sec_env;
        addr = *addr_p;
        write_page_i2c_eeprom( addr, (uint8_t *)&app_sec_env, sizeof(struct app_sec_env_tag));
    }
    
    i2c_release();
}


void app_alt_pair_load_bond_data(struct rand_nb *rand_nb, uint16_t ediv)
{
    struct app_sec_env_tag tmp_sec_env;
    struct app_sec_env_tag *peeprom_sec_env = (struct app_sec_env_tag *) EEPROM_BOND_DATA_ADDR;
    uint8_t *addr_p;
    uint8_t addr;
    uint32_t i;

    i2c_init();
    
    for (i = 0; i < MAX_BOND_PEER; i++)
    {
        addr_p = (uint8_t *)&peeprom_sec_env;
        addr = *addr_p;
        sequential_read_i2c_eeprom( (uint8_t *) &tmp_sec_env, addr, sizeof(struct app_sec_env_tag));
        
        if ((tmp_sec_env.ediv == ediv) && (!memcmp(rand_nb, &tmp_sec_env.rand_nb, RAND_NB_LEN))
                && (tmp_sec_env.auth & GAP_AUTH_BOND))
        {
           memcpy(&app_sec_env, &tmp_sec_env, sizeof(struct app_sec_env_tag));
           alt_pair_active_peer_pos = i;
           alt_pair_next_peer_pos = i + 1;
           break; 
        }
        
        peeprom_sec_env++;
    }
    
    i2c_release();    
}


/*
 * Params: bool init
 *           - true:  find the first valid entry starting from the beginning. 
 *                    Warning! Active and Next pointers will be reset! 
 *                    Used when no connection exists and advertising is about to start!
 *           - false: find the next valid entry (if any) starting just after the last position 
 *                    used. This can be the position of the current bonded host or, if directed
 *                    advertising has started, the position of the last host we did directed 
 *                    advertising to. Used when connected and intend to start directed advertising to other 
 *                    hosts for whom bonded data exist or while a cycle of directed advertising 
 *                    to all known bonded hosts, until a connection is established, is being executed.
 *
 * Returns:
 *           - true:  data have been copied to app_sec_env (a valid entry was found)
 *           - false: no valid entry was found. 
 *                    app_sec_env is reset to the Active pointer data in case of init == false!
 */ 
bool app_alt_pair_get_next_bond_data(bool init)
{
    struct app_sec_env_tag tmp_sec_env;
    uint32_t i;
    bool status = false;
    uint8_t addr;

    i2c_init();
    
    // after reset 'alt_pair_next_peer_pos' is 0. if it's not 0 then this is the position we should use
    i = alt_pair_next_peer_pos;
    
    if (init) 
        alt_pair_active_peer_pos = MAX_BOND_PEER;

    do
    {
        if (i == MAX_BOND_PEER) 
            i = 0;
            
        addr = (i * sizeof(struct app_sec_env_tag)) + EEPROM_BOND_DATA_ADDR;

        sequential_read_i2c_eeprom((uint8_t *) &tmp_sec_env, addr, sizeof(struct app_sec_env_tag));
        
        if (tmp_sec_env.peer_addr_type == 0 && tmp_sec_env.auth & GAP_AUTH_BOND) //used entry (public address)
        { //TODO: Can we do the same for random addresses? (VK)
           memcpy(&app_sec_env, &tmp_sec_env, sizeof(struct app_sec_env_tag));
           alt_pair_next_peer_pos = i + 1;
           status = true;
           break; 
        }
        
        i++;
        
        if (i == alt_pair_active_peer_pos) 
        {
            if (alt_pair_active_peer_pos == MAX_BOND_PEER) // special case
                alt_pair_next_peer_pos = 0;
            else
                alt_pair_next_peer_pos = i + 1;
                
            break;
        }
            
    } while(1);
    
    if (!status) 
    {
        if (init)
        {
#ifdef MITM_REQUIRED
            app_sec_env.auth = GAP_AUTH_REQ_MITM_BOND;
#else
            app_sec_env.auth = GAP_AUTH_REQ_NO_MITM_BOND;
#endif
        }
        else
        {
            addr = (alt_pair_active_peer_pos * sizeof(struct app_sec_env_tag)) + EEPROM_BOND_DATA_ADDR;
            sequential_read_i2c_eeprom( (uint8_t *) &app_sec_env, addr, sizeof(struct app_sec_env_tag));
        }
    }
        
    i2c_release();  

    return status;
}


void app_alt_pair_clear_all_bond_data(void)
{
    uint32_t i;
    uint8_t zero_data[32] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                             0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                             0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                             0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
        
    i2c_init();                                    
                                
    for (i = 0; i < 8; i++)
    {
        uint8_t addr;
        addr = i * 32;
        write_page_i2c_eeprom( (uint8_t) addr, (uint8_t *)&zero_data[0], 32);
    }
    
    i2c_release();
}

#endif //(BLE_ALT_PAIR)
#endif //(BLE_APP_PRESENT)
/// @} APP
