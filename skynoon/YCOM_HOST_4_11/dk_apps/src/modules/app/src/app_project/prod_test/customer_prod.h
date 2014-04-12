#ifndef _CUSTOMER_PROD_H_
#define _CUSTOMER_PROD_H_

/**
****************************************************************************************
*
* @file rf_580.h
*
* @brief rf related functions for 580.
*
* Copyright (C) 2012. Dialog Semiconductor Ltd, unpublished work. This computer 
 * program includes Confidential, Proprietary Information and is a Trade Secret of 
 * Dialog Semiconductor Ltd.  All use, disclosure, and/or reproduction is prohibited 
 * unless authorized in writing. All Rights Reserved.
*
* <bluetooth.support@diasemi.com> and contributors.
*
****************************************************************************************
*/

#include "global_io.h"

enum
{
    ///NO_TEST_RUNNING
    STATE_IDLE         						 = 0x00,
    ///START_TX_TEST
    STATE_START_TX,   						//1
    ///START_RX_TEST
    STATE_START_RX,							//2
	///DIRECT_TX_TEST					
	STATE_DIRECT_TX_TEST,					//3 activated via default hci command
	///DIRECT_RX_TEST					
	STATE_DIRECT_RX_TEST,					//4 activated via default hci command
	///CONTINUE_TX
	STATE_START_CONTINUE_TX,				//5
	///UNMODULATED_ON						
	STATE_UNMODULATED_ON,					//6
};

enum
{
    ///ES2
    ES2           = 0x20,
    ///ES3
    ES3           = 0x30,
	///ES4		  
	ES4		  	  = 0x40,
	///ES5		  
	ES5			  = 0x50,
    
};

extern volatile uint8 test_state;
extern volatile uint8 test_data_pattern;
extern volatile uint8 test_freq;
extern volatile uint8 test_data_len;

extern volatile uint16_t text_tx_nr_of_packets;
extern volatile uint16_t test_tx_packet_nr;

extern volatile uint16_t test_rx_packet_nr;
extern volatile uint16_t test_rx_packet_nr_syncerr;
extern volatile uint16_t test_rx_packet_nr_crcerr;
extern volatile uint16_t test_rx_irq_cnt;
extern volatile uint16_t rx_test_rssi_1;
extern volatile uint16_t rx_test_rssi_2;	

extern volatile uint16_t  rf_calibration_request_cbt;

void set_state_stop(void);
void set_state_start_tx(void);
void set_state_start_rx(void);
void set_state_start_continue_tx(void);


void check_rx_production_test_data(uint8* data );
void init_data_patterns(void);
void hci_start_prod_rx_test( uint8_t*);
void hci_tx_send_nb_packages(uint8_t* );
void hci_tx_start_continue_test_mode(uint8_t*);
void hci_unmodulated_cmd(uint8_t* );
void hci_end_tx_continuous_test_cmd(void);
void hci_end_rx_prod_test_cmd(void);
void hci_sleep_test_cmd(uint8_t* ptr_data);

//additional HCI commands for production test  //START WITH 0x40xx


#define HCI_UNMODULATED_ON_CMD_OPCODE	   (0x4010) 
/* Unmodulated TX/RX
 0x01   is HCI_CMD_MSG_TYPE, SOH , type ALT-1 followed by CTRL-A
 0x10 
 0x40
 0x02   = length of parameters is 2 for this command
 MODE TX/RX, T/R
 FREQ  
*/
//RETURN MESSAGE	
struct msg_unmodulated_cfm
{
    /// Status of the command reception
	uint8_t 	packet_type;
	uint8_t 	event_code;
	uint8_t		length;
	uint8_t		param0;
	uint16_t 	param_opcode;
};

#define HCI_START_PROD_RX_TEST_CMD_OPCODE 	(0x4020)
struct msg_start_prod_rx_cfm
{
    /// Status of the command reception
	uint8_t 	packet_type;
	uint8_t 	event_code;
	uint8_t		length;
	uint8_t		param0;
	uint16_t 	param_opcode;
};



#define HCI_LE_END_PROD_RX_TEST_CMD_OPCODE  (0x4030)
struct msg_rx_stop_send_info_back
{
    /// Status of the command reception
	uint8_t 	packet_type;
	uint8_t 	event_code;
	uint8_t		length;
	uint8_t		param0;
	uint16_t 	param_opcode;
    uint16_t    nb_packet_received;
	uint16_t    nb_syncerr;
	uint16_t	nb_crc_error;
	uint16_t	rx_test_rssi;
};


//HCI_LE_TX_TEST_CMD_OPCODE with length 5 is added. Lenght 3 is default.
//length is 2 bytes more because of 16 bits value for nr_of_packages.
//so this msg is confirmation of the message, so packages are not transmitted yet
//after packages are transmitted there's an additional message 'HCI_TX_PACKAGES_SEND_READY'.
struct msg_tx_send_packages_cfm
{
    /// Status of the command reception
	uint8_t 	packet_type;
	uint8_t 	event_code;
	uint8_t		length;
	uint8_t		param0;
	uint16_t 	param_opcode;
};

#define HCI_TX_PACKAGES_SEND_READY  	(0x4040)
struct msg_tx_packages_send_ready
{
    /// Status of the command reception
	uint8_t 	packet_type;
	uint8_t 	event_code;
	uint8_t		length;
	uint8_t		param0;
	uint16_t 	param_opcode;
};


#define HCI_TX_START_CONTINUE_TEST_CMD_OPCODE  	(0x4050)
struct msg_tx_start_continue_test_cfm
{
    /// Status of the command reception
	uint8_t 	packet_type;
	uint8_t 	event_code;
	uint8_t		length;
	uint8_t		param0;
	uint16_t 	param_opcode;
};

#define HCI_TX_END_CONTINUE_TEST_CMD_OPCODE  	(0x4060)
struct msg_tx_end_continue_test_cfm
{
    /// Status of the command reception
	uint8_t 	packet_type;
	uint8_t 	event_code;
	uint8_t		length;
	uint8_t		param0;
	uint16_t 	param_opcode;
};

#define HCI_SLEEP_TEST_CMD_OPCODE  	(0x4070)
struct msg_sleep_test_cfm
{
    /// Status of the command reception
	uint8_t 	packet_type;
	uint8_t 	event_code;
	uint8_t		length;
	uint8_t		param0;
	uint16_t 	param_opcode;
};

#define HCI_XTAL_TRIM_CMD_OPCODE  	(0x4080)
struct msg_xtal_trim_cfm
{
    /// Status of the command reception
	uint8_t 	packet_type;
	uint8_t 	event_code;
	uint8_t		length;
	uint8_t     param0;
	uint16_t 	param_opcode;
	uint16_t 	xtrim_val;
};


#endif // _CUSTOMER_PROD_H_

