/**
 ****************************************************************************************
 *
 * @file da14580_stack_config.h
 *
 * @brief RW stack configuration file.
 *
 * Copyright (C) 2014. Dialog Semiconductor Ltd, unpublished work. This computer 
 * program includes Confidential, Proprietary Information and is a Trade Secret of 
 * Dialog Semiconductor Ltd.  All use, disclosure, and/or reproduction is prohibited 
 * unless authorized in writing. All Rights Reserved.
 *
 * <bluetooth.support@diasemi.com> and contributors.
 *
 ****************************************************************************************
 */

#ifndef DA14580_STACK_CONFIG_H_
#define DA14580_STACK_CONFIG_H_

/////////////////////////////////////////////////////////////
/*Do not alter*/
#define CFG_ES4
#define CFG_EMB 
#define CFG_HOST 
#define CFG_GTL 
#define CFG_BLE 
#define CFG_HCI_UART 
#define CFG_EXT_DB  
#define nCFG_DBG_MEM 
#define nCFG_DBG_FLASH  
#define nCFG_DBG_NVDS  
#define nCFG_DBG_STACK_PROF  
#define CFG_RF_RIPPLE 
#define nCFG_PERIPHERAL  
#define CFG_ALLROLES        1  
#define CFG_CON             6  
#define CFG_SECURITY_ON     1  
#define CFG_ATTC 
#define CFG_ATTS  
#define CFG_DBG_NVDS
#define CFG_DBG_MEM
#define CFG_BLECORE_11 
#define POWER_OFF_SLEEP  
#define CFG_SLEEP 
#define CFG_CHNL_ASSESS

/*FPGA*/
#define nFPGA_USED

/*Radio interface*/
 
#ifdef FPGA_USED
#define RADIO_RIPPLE	    1
#define RIPPLE_ID           66
#else
#define RADIO_580           1 
#endif

/*Misc*/

#define __NO_EMBEDDED_ASM 


/*Scatterfile: Memory maps*/
#define DEEP_SLEEP_SETUP    1
#define EXT_SLEEP_SETUP     2

#endif // DA14580_STACK_CONFIG_H_
