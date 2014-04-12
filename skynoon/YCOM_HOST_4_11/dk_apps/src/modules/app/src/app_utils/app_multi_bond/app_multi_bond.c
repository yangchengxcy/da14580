/**
****************************************************************************************
*
* @file app_multi_bond.c
*
* @brief Special (multi) bonding procedure.
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

#include <string.h>                     // string manipulation and functions

#include "app.h"                        // application definitions
#include "app_task.h"                   // application task definitions
#include "app_api.h"                
#include "app_console.h"
#include "app_sec.h"                
#include "co_bt.h"
#include "arch.h"                       // platform definitions
#include "ke_timer.h"                   // kernel timer 
#include "rwble_config.h"
#include "gpio.h"

#include "app_multi_bond.h"
#include "i2c_eeprom.h"
#include "periph_setup.h"

#if (BLE_APP_KEYBOARD)
#include "app_kbd.h"
#include "app_kbd_key_matrix.h"
#endif


#if (BLE_APP_PRESENT)

uint8_t multi_bond_status                   __attribute__((section("retention_mem_area0"), zero_init));
uint8_t multi_bond_enabled                  __attribute__((section("retention_mem_area0"), zero_init)); 
static uint8_t multi_bond_active_peer_pos   __attribute__((section("retention_mem_area0"), zero_init)); 
static uint8_t multi_bond_next_peer_pos     __attribute__((section("retention_mem_area0"), zero_init)); 


bool app_alt_pair_disconnect(void)
{
    bool ret = false;
    
    if (HAS_MULTI_BOND)
    {
        if((app_sec_env.auth & GAP_AUTH_BOND))
        {        
            multi_bond_enabled = 1;

#ifdef CFG_MULTI_BOND
            app_timer_set(APP_ALT_PAIR_TIMER, TASK_APP, ALT_PAIR_DISCONN_TIME);	//60 seconds
#endif            
        }
        
        app_disconnect();
        
        ret = true;
    }

    return ret;
}


int app_alt_pair_timer_handler(void)
{
    if (HAS_MULTI_BOND)
    {
        multi_bond_enabled = 0;
    }
    
    return (KE_MSG_CONSUMED);
}


bool app_alt_pair_check_peer(struct bd_addr *peer_addr, uint8_t peer_addr_type)
{
    if (HAS_MULTI_BOND)
    {
        if (multi_bond_enabled)
        {
            if ( (peer_addr_type == app_sec_env.peer_addr_type) && 
                 (!memcmp(app_sec_env.peer_addr.addr, peer_addr->addr, BD_ADDR_LEN)) &&
                 (app_sec_env.auth & GAP_AUTH_BOND) )
            {
                app_disconnect();
                
                return false;
            }
        }
    }
    
    return true;
}


//TODO: Maybe this is used to store the position of the last connected bonded host. This way, after a reset,
// we would try to connect to it first. (VK)/
void app_alt_pair_read_status(void)
{
    if (HAS_EEPROM)
    {
        i2c_eeprom_init(I2C_SLAVE_ADDRESS, I2C_SPEED_MODE, I2C_ADDRESS_MODE, I2C_ADRESS_BYTES_CNT);
        
        i2c_eeprom_read_data(&multi_bond_status, EEPROM_BONDING_STATUS_ADDR, sizeof(uint8_t));
        
        i2c_eeprom_release();
    }
}


void app_alt_pair_store_status(void)
{
    if (HAS_EEPROM)
    {
        i2c_eeprom_init(I2C_SLAVE_ADDRESS, I2C_SPEED_MODE, I2C_ADDRESS_MODE, I2C_ADRESS_BYTES_CNT);
        
        i2c_eeprom_write_byte(EEPROM_BONDING_STATUS_ADDR, multi_bond_status);
        
        i2c_eeprom_release();
    }
}


void app_alt_pair_store_bond_data(void)
{
    if (HAS_EEPROM)
    {
        struct app_sec_env_tag tmp_sec_env;
        int addr = EEPROM_BOND_DATA_ADDR;
        int empty_pos_addr = 0; // address 0 is RESERVED!
        uint32_t i;
        bool found = false;
        
        i2c_eeprom_init(I2C_SLAVE_ADDRESS, I2C_SPEED_MODE, I2C_ADDRESS_MODE, I2C_ADRESS_BYTES_CNT);
        
        //find entry 
        for (i = 0; i < MAX_BOND_PEER; i++)
        {
            i2c_eeprom_read_data( (uint8_t *) &tmp_sec_env, addr, sizeof(struct app_sec_env_tag));
            
            if (!(tmp_sec_env.auth & GAP_AUTH_BOND) && (empty_pos_addr == 0))
            {
                empty_pos_addr = addr;   
            }
            
            if ((tmp_sec_env.ediv == app_sec_env.ediv) && (!memcmp(&app_sec_env.rand_nb, &tmp_sec_env.rand_nb, RAND_NB_LEN)))
            {
                found = true;
                break; 
            }
            else if ((tmp_sec_env.peer_addr_type == app_env.peer_addr_type) && (!memcmp(&app_env.peer_addr, &tmp_sec_env.peer_addr, BD_ADDR_LEN)))
            {
                empty_pos_addr = addr;  // overwrite previous used (invalid) entry
            }
            
            addr += sizeof(struct app_sec_env_tag);
        }    
            
        if (found)  // entry for peer exists. Overwite only if the peer is not using a Public or Static address (???)
        {
            if (app_env.peer_addr_type > GAPM_GEN_STATIC_RND_ADDR) // is it required? if keys are the same then it shouldn't be...
                i2c_eeprom_write_data((uint8_t *)&app_sec_env, addr, sizeof(struct app_sec_env_tag));
        }
        else        // entry for peer does not exist. Write to first empty. If there is no space rewrite first entry.
        {
            if (empty_pos_addr == 0)
                addr = EEPROM_BOND_DATA_ADDR;
            else
                addr = empty_pos_addr;
            
            i2c_eeprom_write_data((uint8_t *)&app_sec_env, addr, sizeof(struct app_sec_env_tag));
            
            addr -= EEPROM_BOND_DATA_ADDR;          // offset zero
            addr /= sizeof(struct app_sec_env_tag); // Nth entry
            multi_bond_status |= (1 << addr);       // update status
            i2c_eeprom_write_byte(EEPROM_BONDING_STATUS_ADDR, multi_bond_status);
        }
        
        i2c_eeprom_release();
    }
}


/*
 * Description  : Search if there's an entry in the EEPROM for the
 *              : connecting host. 
 *
 * Returns      : 2, if the new host is the same with the last connected host
 *              : 1, if the new host is different 
 *              : 0, if there's no entry in the EEPROM for this host
 *
 */
int app_alt_pair_load_bond_data(struct rand_nb *rand_nb, uint16_t ediv)
{
    if (HAS_EEPROM)
    {
        struct app_sec_env_tag tmp_sec_env;
        int addr = EEPROM_BOND_DATA_ADDR;
        uint32_t i;
        int retval = 0;

        i2c_eeprom_init(I2C_SLAVE_ADDRESS, I2C_SPEED_MODE, I2C_ADDRESS_MODE, I2C_ADRESS_BYTES_CNT);
        
        for (i = 0; i < MAX_BOND_PEER; i++)
        {
            i2c_eeprom_read_data( (uint8_t *) &tmp_sec_env, addr, sizeof(struct app_sec_env_tag));
            
            if ((tmp_sec_env.ediv == ediv) && (!memcmp(rand_nb, &tmp_sec_env.rand_nb, RAND_NB_LEN))
                    && (tmp_sec_env.auth & GAP_AUTH_BOND))
            {
                if ( (app_sec_env.ediv == ediv) && (!memcmp(rand_nb, &app_sec_env.rand_nb, RAND_NB_LEN))
                      && (app_sec_env.auth & GAP_AUTH_BOND) )
                    retval = 2;
                else
                    retval = 1;
                
                memcpy(&app_sec_env, &tmp_sec_env, sizeof(struct app_sec_env_tag));
                multi_bond_active_peer_pos = i;
                multi_bond_next_peer_pos = i + 1;
                break; 
            }
            
            addr += sizeof(struct app_sec_env_tag);
        }
        
        i2c_eeprom_release();

        return retval;
    }
    else
        return 0;
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
    if (HAS_EEPROM)
    {
        struct app_sec_env_tag tmp_sec_env;
        uint32_t i;
        bool status = false;
        uint8_t addr;

        i2c_eeprom_init(I2C_SLAVE_ADDRESS, I2C_SPEED_MODE, I2C_ADDRESS_MODE, I2C_ADRESS_BYTES_CNT);
        
        // after reset 'multi_bond_next_peer_pos' is 0. if it's not 0 then this is the position we should use
        i = multi_bond_next_peer_pos;
        
        if (init) 
            multi_bond_active_peer_pos = MAX_BOND_PEER;

        do
        {
            if (i == MAX_BOND_PEER) 
                i = 0;
                
            addr = (i * sizeof(struct app_sec_env_tag)) + EEPROM_BOND_DATA_ADDR;

            i2c_eeprom_read_data((uint8_t *) &tmp_sec_env, addr, sizeof(struct app_sec_env_tag));
            
            if (tmp_sec_env.peer_addr_type == 0 && tmp_sec_env.auth & GAP_AUTH_BOND) //used entry (public address)
            { //TODO: Can we do the same for random addresses? (VK)
               memcpy(&app_sec_env, &tmp_sec_env, sizeof(struct app_sec_env_tag));
               multi_bond_next_peer_pos = i + 1;
               status = true;
               break; 
            }
            
            i++;
            
            if (i == multi_bond_active_peer_pos) 
            {
                if (multi_bond_active_peer_pos == MAX_BOND_PEER) // special case
                    multi_bond_next_peer_pos = 0;
                else
                    multi_bond_next_peer_pos = i + 1;
                    
                break;
            }
                
        } while(1);
        
        if (!status) 
        {
            if (init)
            {
                if (HAS_MITM)
                {
                    app_sec_env.auth = GAP_AUTH_REQ_MITM_BOND;
                }
                else
                {
                    app_sec_env.auth = GAP_AUTH_REQ_NO_MITM_BOND;
                }
            }
            else
            {
                addr = (multi_bond_active_peer_pos * sizeof(struct app_sec_env_tag)) + EEPROM_BOND_DATA_ADDR;
                i2c_eeprom_read_data( (uint8_t *) &app_sec_env, addr, sizeof(struct app_sec_env_tag));
            }
        }
            
        i2c_eeprom_release();

        return status;
    }
    else
        return false;
}


void app_alt_pair_clear_all_bond_data(void)
{
    if (HAS_EEPROM)
    {
        uint32_t i;
        uint8_t zero_data[32] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                                 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                                 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                                 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
            
        i2c_eeprom_init(I2C_SLAVE_ADDRESS, I2C_SPEED_MODE, I2C_ADDRESS_MODE, I2C_ADRESS_BYTES_CNT);
                                    
        for (i = 0; i < 8; i++)
        {
            uint8_t addr;
            addr = i * 32;
            i2c_eeprom_write_data( zero_data, (uint8_t) addr, 32);
        }
        multi_bond_status = 0;

        if (DEVELOPMENT__NO_OTP)
        {
            uint8_t addr;
            int i, j;
            uint8_t read_data[4];

            for (i = 0; i < 64; i++)
            {
                addr = i * 4;
                
                for (j = 0; j < 4; j++)
                    read_data[j] = 0xFF;
                
                i2c_eeprom_read_data(read_data, addr, 4);
                
                if (memcmp(zero_data, read_data, 4))
                    __asm("BKPT #0\n"); 
            }
        }
        
        i2c_eeprom_release();
    }
}

#endif //(BLE_APP_PRESENT)
/// @} APP
