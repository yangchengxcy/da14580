#include "rwip_config.h"      // SW configuration

#include "co_bt.h"            // BT standard definitions
#include "co_endian.h"
#include "co_utils.h"
#include "co_error.h"

#include "ke_event.h"         // kernel event
#include "ke_msg.h"

#include "gtl.h"
#include "gtl_env.h"
#include "gtl_eif.h"
#include "gtl_task.h"
#include "gtl_hci.h"

#include "llm.h"
#include "rf_580.h"
#include "uart.h"
#include "customer_prod.h"
#include "periph_setup.h"

#ifdef LUT_PATCH_ENABLED 
#include "em_map_ble.h" // For Freq Table address definition
#include "pll_vcocal_lut.h"
extern const volatile struct 		LUT_CFG_struct LUT_CFG ;
#endif 

#include "arch_sleep.h"

volatile uint8 test_state;//				    __attribute__((at(TEST_MODE_ADDRESS)));
volatile uint8 test_data_pattern;//		    	__attribute__((at(TEST_DATA_PATTERN)));
volatile uint8 test_freq;//				    	__attribute__((at(TEST_FREQ)));
volatile uint8 test_data_len;//			    	__attribute__((at(TEST_DATA_LEN)));

volatile uint16_t text_tx_nr_of_packets;//		__attribute__((at(TEST_TX_NR_OF_PACKETS)));
volatile uint16_t test_tx_packet_nr;//		    __attribute__((at(TEST_TX_PACKET_NR)));

volatile uint16_t test_rx_packet_nr;//			__attribute__((at(TEST_RX_PACKET_NR)));
volatile uint16_t test_rx_packet_nr_syncerr;//	__attribute__((at(TEST_RX_PACKET_NR_SYNCERR)));
volatile uint16_t test_rx_packet_nr_crcerr;//	__attribute__((at(TEST_RX_PACKET_NR_CRCERR)));

volatile uint16_t rx_test_rssi_1;//				__attribute__((at(TEST_RX_RSSI_1))); //rssi according RXdescriptor , 8 bits
volatile uint16_t rx_test_rssi_2;//				__attribute__((at(TEST_RX_RSSI_2))); //rssi according RF_RSSI_RESULT_REG, 16 bits

volatile uint16_t test_rx_irq_cnt;//				__attribute__((at(TEST_RX_IRQ_CNT)));

volatile uint16_t rf_calibration_request_cbt;  //RF_CALIBRATION_REQUEST_CBT

void app_init(void)
{
    app_disable_sleep();
    
	test_state			= STATE_IDLE;
	test_data_pattern 	= 2;	
	test_freq			= 0;	
	test_data_len		= 37;	
	text_tx_nr_of_packets = 50; 	
	test_tx_packet_nr = 0;	
	test_rx_packet_nr = 0;
	test_rx_packet_nr_syncerr = 0;
	test_rx_packet_nr_crcerr = 0;
	test_rx_irq_cnt = 0;
	rx_test_rssi_1 = 0;
	rx_test_rssi_2 = 0;

	SetBits32(BLE_CNTL2_REG, BLE_RSSI_SEL, 1); //SELECT CURRENT AVERAGE RSSI.    
    
	rf_calibration_request_cbt = 0;
	init_TXEN_RXEN_irqs();    
    
#if 0 //test starts automatically see mail Yannis, George
{
	
	// allocate the complete event message
    struct llm_le_tx_test_cmd *event = ke_msg_alloc(LLM_LE_TEST_TX_CMD, TASK_LLM, TASK_GTL, 3);
    ///TX frequency for Tx test
    event->tx_freq = 0;
    ///TX test data length
    event->test_data_len = 4;
    ///TX test payload type - see enum
    event->pk_payload_type = 0;
    // send the message
    ke_msg_send(event);   
	
}
#endif    
}

void hci_start_prod_rx_test( uint8_t* ptr_data)
{	
	struct msg_start_prod_rx_cfm s;
	uint8_t* bufptr;
	test_rx_packet_nr = 0;
	test_rx_packet_nr_syncerr = 0;
	test_rx_packet_nr_crcerr  = 0;
	test_rx_irq_cnt=0;	
	test_freq = ptr_data[0];	
	//test_state = STATE_START_RX;	
	set_state_start_rx();
					
	s.packet_type	= HCI_EVT_MSG_TYPE;
	s.event_code 	= HCI_CMD_CMPL_EVT_CODE; 
	s.length     	= 3;
	s.param0		= 0x01;
	s.param_opcode	= HCI_START_PROD_RX_TEST_CMD_OPCODE;		
	bufptr = (uint8_t*)&s;							
	uart_write(bufptr,sizeof(struct msg_start_prod_rx_cfm),NULL);			
}

void hci_tx_send_nb_packages(uint8_t* ptr_data)
{				
	struct msg_tx_send_packages_cfm s;
	uint8_t* bufptr;
	s.packet_type	= HCI_EVT_MSG_TYPE;
	s.event_code 	= HCI_CMD_STATUS_EVT_CODE; 
	s.length     	= 3;
	s.param0		= 0x01;
	s.param_opcode	= HCI_LE_TX_TEST_CMD_OPCODE;		
	bufptr = (uint8_t*)&s;							
	uart_write(bufptr,sizeof(struct msg_tx_send_packages_cfm),NULL);					
	test_freq = ptr_data[0];					    
	test_data_len = ptr_data[1];	
	test_data_pattern = ptr_data[2]; 
	text_tx_nr_of_packets =  (ptr_data[4]<<8) | ptr_data[3];	
	//test_state = STATE_START_TX;
	set_state_start_tx();
}	

void hci_tx_start_continue_test_mode(uint8_t* ptr_data)
{
	struct msg_tx_start_continue_test_cfm s;
	uint8_t* bufptr;
	s.packet_type	= HCI_EVT_MSG_TYPE;
	s.event_code 	= HCI_CMD_CMPL_EVT_CODE; 
	s.length     	= 3;
	s.param0		= 0x01;
	s.param_opcode	= HCI_TX_START_CONTINUE_TEST_CMD_OPCODE;		
	bufptr = (uint8_t*)&s;							
	uart_write(bufptr,sizeof(struct msg_tx_start_continue_test_cfm),NULL);		
	test_freq = ptr_data[0];					    
	test_data_pattern = ptr_data[1]; 
	//test_state = STATE_START_CONTINUE_TX;	
	set_state_start_continue_tx();
	
}

void hci_unmodulated_cmd(uint8_t* ptr_data)
{
	bool cmd_flag =false;
	if(ptr_data[0]=='O') 		//UNMODULATED OFF
	{
		SetWord16(RF_BMCW_REG, 0);     //DISABLE OVERRULE CN_SEL IS 0
		//SetWord16(RF_OVERRULE_REG, RX_DIS_WR);
		//SetWord16(RF_OVERRULE_REG, TX_DIS_WR);
		SetWord16(RF_OVERRULE_REG, 0);  // SO DS IS NOT CORRECT, DISABLE DONE BY WRITING '0' TO ENABLE ISO '1' TO DISABLE 
		SetWord16(RF_SYNTH_CTRL3_REG, 0x0003);      //ENABLE MODULATION
 		NVIC_EnableIRQ(BLE_RF_DIAG_IRQn); 				
 		NVIC_ClearPendingIRQ(BLE_RF_DIAG_IRQn); //clear eventual pending bit, but not necessary becasuse this is already cleared automatically in HW
		cmd_flag=true;
		test_state = STATE_IDLE;
	}
	else if(ptr_data[0]=='T') //UNMODULATED TX
	{
		NVIC_DisableIRQ(BLE_RF_DIAG_IRQn);
		
		uint16_t cn = ptr_data[1];			

#ifdef LUT_PATCH_ENABLED	
		const volatile struct LUT_CFG_struct *pLUT_CFG;	// = (const volatile struct LUT_CFG_struct *)(jump_table_struct[lut_cfg_pos]);
		pLUT_CFG= (const volatile struct LUT_CFG_struct *)((uint32)&LUT_CFG);
		if(!pLUT_CFG->HW_LUT_MODE) //so SW_LUT_MODE
		{ 
			set_rf_cal_cap(cn); 
			SetBits16(RF_BMCW_REG, CN_SEL,1);     							  // ENABLE OVERRULE CN_SEL IS 1
			SetBits16(RF_BMCW_REG, CN_WR, cn); 	
		}
		else //so HW_LUT_MODE
		{
			//HW	
			uint8_t * BLE_freq_tbl;					
			BLE_freq_tbl=(uint8_t *)(REG_BLE_EM_BASE_ADDR+EM_BLE_FT_OFFSET);  // address = 0x80000 + EM_BLE_FT_OFFSET = 0x80020
			SetBits16(RF_BMCW_REG, CN_SEL,1);     							  // ENABLE OVERRULE CN_SEL IS 1
			SetBits16(RF_BMCW_REG, CN_WR, BLE_freq_tbl[cn]); 			
		}
#else //NO LUT 
		SetBits16(RF_BMCW_REG, CN_SEL,1);     							  // ENABLE OVERRULE CN_SEL IS 1
		SetBits16(RF_BMCW_REG, CN_WR, cn); 	
#endif
		
		SetWord16(RF_SYNTH_CTRL3_REG, 0x4000);      //DISABLE MODULATION
		SetWord16(RF_OVERRULE_REG, RX_DIS_WR); 		//DISABLE EVENTUAL RX
		SetWord16(RF_OVERRULE_REG, TX_EN_WR); 		//ENABLE EVENTUAL TX
		cmd_flag=true;
		test_state = STATE_UNMODULATED_ON;
	}
	else if(ptr_data[0]=='R') //UNMODULATED RX
	{
		NVIC_DisableIRQ(BLE_RF_DIAG_IRQn); 
		
		
		uint16_t cn = ptr_data[1];			

#ifdef LUT_PATCH_ENABLED	
		const volatile struct LUT_CFG_struct *pLUT_CFG;	// = (const volatile struct LUT_CFG_struct *)(jump_table_struct[lut_cfg_pos]);
		pLUT_CFG= (const volatile struct LUT_CFG_struct *)((uint32)&LUT_CFG);
		if(!pLUT_CFG->HW_LUT_MODE) //so SW_LUT_MODE
		{ 
			set_rf_cal_cap(cn); 
			SetBits16(RF_BMCW_REG, CN_SEL,1);     							  // ENABLE OVERRULE CN_SEL IS 1
			SetBits16(RF_BMCW_REG, CN_WR, cn); 	
		}
		else //so HW_LUT_MODE
		{
			//HW	
			uint8_t * BLE_freq_tbl;					
			BLE_freq_tbl=(uint8_t *)(REG_BLE_EM_BASE_ADDR+EM_BLE_FT_OFFSET);  // address = 0x80000 + EM_BLE_FT_OFFSET = 0x80020
			SetBits16(RF_BMCW_REG, CN_SEL,1);     							  // ENABLE OVERRULE CN_SEL IS 1
			SetBits16(RF_BMCW_REG, CN_WR, BLE_freq_tbl[cn]); 			
		}
#else //NO LUT 
		SetBits16(RF_BMCW_REG, CN_SEL,1);     							  // ENABLE OVERRULE CN_SEL IS 1
		SetBits16(RF_BMCW_REG, CN_WR, cn); 	
#endif		
		
		SetWord16(RF_SYNTH_CTRL3_REG, 0x4000);      //DISABLE MODULATION
		SetWord16(RF_OVERRULE_REG, TX_DIS_WR); 		//DISABLE EVENTUAL TX
		SetWord16(RF_OVERRULE_REG, RX_EN_WR); 		//ENABLE EVENTUAL RX
		cmd_flag=true;
		test_state = STATE_UNMODULATED_ON;
	}
	if(cmd_flag)
	{
		struct msg_unmodulated_cfm s;
		uint8_t* bufptr;
		s.packet_type	= HCI_EVT_MSG_TYPE;
		s.event_code 	= HCI_CMD_CMPL_EVT_CODE; 
		s.length     	= 3;
		s.param0		= 0x01;
		s.param_opcode	= HCI_UNMODULATED_ON_CMD_OPCODE;		
		bufptr = (uint8_t*)&s;							
		uart_write(bufptr,sizeof(struct msg_unmodulated_cfm),NULL);				
	}	
	
}

void hci_end_tx_continuous_test_cmd(void)
{
	struct msg_tx_end_continue_test_cfm s;
	uint8_t* bufptr;
	s.packet_type	= HCI_EVT_MSG_TYPE;
	s.event_code 	= HCI_CMD_CMPL_EVT_CODE; 
	s.length     	= 3;
	s.param0		= 0x01;
	s.param_opcode	= HCI_TX_END_CONTINUE_TEST_CMD_OPCODE;		
	bufptr = (uint8_t*)&s;							
	uart_write(bufptr,sizeof(struct msg_tx_end_continue_test_cfm),NULL);	
	set_state_stop();	
	SetWord32(BLE_RFTESTCNTL_REG,0); //disable continuous mode		
}

void hci_end_rx_prod_test_cmd(void)
{
	struct msg_rx_stop_send_info_back s;
	uint8_t* bufptr;
	s.packet_type	= HCI_EVT_MSG_TYPE;
	s.event_code 	= HCI_CMD_CMPL_EVT_CODE; 
	s.length     	= 11;
	s.param0		= 0x01;
	s.param_opcode	= HCI_LE_END_PROD_RX_TEST_CMD_OPCODE;		
	s.nb_packet_received = test_rx_packet_nr;
	s.nb_syncerr 		 = test_rx_packet_nr_syncerr;
	s.nb_crc_error		 = test_rx_packet_nr_crcerr;
	s.rx_test_rssi       = rx_test_rssi_2;
	
	bufptr = (uint8_t*)&s;
	uart_write(bufptr,sizeof(struct msg_rx_stop_send_info_back),NULL);
	set_state_stop();			
	
	
}

void set_state_stop(void)
{	
	if( (test_state==STATE_START_TX) || (test_state==STATE_START_RX) || test_state==STATE_START_CONTINUE_TX )// in case of default
																											 //direct TX or RX stack handles the end of these tasks
    {
		lld_test_stop(llm_le_env.evt);
        lld_evt_try_free(&llm_le_env.evt);
		//llm_init();
    }
	test_state = STATE_IDLE;
}

void set_state_start_tx(void)
{
	if(test_state==STATE_IDLE)
	{    
		test_tx_packet_nr=0;
		struct llm_le_tx_test_cmd  tx_test;
		tx_test.tx_freq = test_freq;
		tx_test.test_data_len = test_data_len; //37;
		tx_test.pk_payload_type = test_data_pattern;
					
		llm_test_mode_start_tx( (struct llm_le_tx_test_cmd const *)&tx_test);
        while(llm_le_env.evt == NULL)
            if(llm_test_mode_start_tx( (struct llm_le_tx_test_cmd const *)&tx_test) != CO_ERROR_NO_ERROR)
                while(1);
		
		test_state=STATE_START_TX;
	}
	
                   
}

void set_state_start_rx(void)
{
	if(test_state==STATE_IDLE)
	{                    
		test_rx_packet_nr = 0;
		test_rx_packet_nr_syncerr = 0;
		test_rx_packet_nr_crcerr  = 0;
		test_rx_irq_cnt=0;
		struct llm_le_rx_test_cmd  rx_test;
		rx_test.rx_freq = test_freq;
		
		llm_test_mode_start_rx( (struct llm_le_rx_test_cmd const *)&rx_test);
              
		test_state=STATE_START_RX;	
	}
}

void set_state_start_continue_tx(void)
{
	if(test_state==STATE_IDLE)
	{ 				    
		struct llm_le_tx_test_cmd  tx_con_test;
		tx_con_test.tx_freq = test_freq;
		tx_con_test.test_data_len = 37; //select a valid value.
		tx_con_test.pk_payload_type = test_data_pattern;
					
		llm_test_mode_start_tx( (struct llm_le_tx_test_cmd const *)&tx_con_test);
				
		SetBits32(BLE_RFTESTCNTL_REG,INFINITETX,1);
		SetBits32(BLE_RFTESTCNTL_REG,TXLENGTHSRC,0);
		SetBits32(BLE_RFTESTCNTL_REG,TXPLDSRC,0);
		SetBits32(BLE_RFTESTCNTL_REG,TXLENGTH,0);
		
		test_state = STATE_START_CONTINUE_TX;
	}		
	
}

void hci_sleep_test_cmd(uint8_t* ptr_data)
{				
	struct msg_sleep_test_cfm s;
	uint8_t* bufptr;
	s.packet_type	= HCI_EVT_MSG_TYPE;
	s.event_code 	= HCI_CMD_STATUS_EVT_CODE; 
	s.length     	= 3;
	s.param0		= 0x01;
	s.param_opcode	= HCI_SLEEP_TEST_CMD_OPCODE;
	bufptr = (uint8_t*)&s;							
	uart_write(bufptr,sizeof(struct msg_sleep_test_cfm),NULL);					

    if(ptr_data[0] == 1)
    {
        if(ptr_data[1] == 0 && ptr_data[2] == 0)
            rwip_env.ext_wakeup_enable = 2; 
        else
        {
            rwip_env.ext_wakeup_enable = 1; 
            jump_table_struct[max_sleep_duration_external_wakeup_pos] = ptr_data[1] * 1600 * 60 + ptr_data[2] * 1600;
        }
        app_set_extended_sleep();
    }
    else if(ptr_data[0] == 2)
    {
        rwip_env.ext_wakeup_enable = 2;
        app_set_deep_sleep();
    }
    else
        app_disable_sleep();
}	
  
void set_XTAL_trim_reg(uint16_t reg_value) {
   SetWord16(CLK_FREQ_TRIM_REG, reg_value);   
}

void hci_xtal_trim_cmd(uint8_t* ptr_data)
{
    uint16_t delta = ptr_data[2]<<8|ptr_data[1];
	struct msg_xtal_trim_cfm s;
	uint8_t* bufptr;
	s.packet_type	= HCI_EVT_MSG_TYPE;
	s.event_code 	= HCI_CMD_CMPL_EVT_CODE; 
	s.length     	= 5;
    s.param0		= 0x0001;
    if(ptr_data[0] == 0)
        s.xtrim_val		= GetWord16(CLK_FREQ_TRIM_REG);
    else
        s.xtrim_val		= 0x0;
	s.param_opcode	= HCI_XTAL_TRIM_CMD_OPCODE;
	bufptr = (uint8_t*)&s;							
	uart_write(bufptr,sizeof(struct msg_xtal_trim_cfm),NULL);					

    if(ptr_data[0] == 1)
    {
        set_XTAL_trim_reg(delta);
    }
    else if(ptr_data[0] == 2)
    {
       SetWord32(TEST_CTRL_REG, 1);
       SetWord32(P05_MODE_REG, 0x300);
    }
    else if(ptr_data[0] == 3)
    {
        uint16_t reg_value = GetWord16(CLK_FREQ_TRIM_REG);   
        if (reg_value <= 0xFFFF - delta)
            set_XTAL_trim_reg(reg_value + delta);
        //for (i=0; i<400; i++); //wait for at least 200us
    }        
    else if(ptr_data[0] == 4)
    {
        uint16_t reg_value = GetWord16(CLK_FREQ_TRIM_REG);   
        if (reg_value >= delta)
            set_XTAL_trim_reg(reg_value - delta);
        //for (i=0; i<400; i++); //wait for at least 200us        
    }
    else if(ptr_data[0] == 5)
    {
       SetWord32(TEST_CTRL_REG, 0);
       SetWord32(P05_MODE_REG, 0x200);
    }
}	
                   
               
					
                
					
				

				

