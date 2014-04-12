/**
 ****************************************************************************************
 *
 * @file jump_table.c
 *
 * @brief Jump table that holds function pointers and veriables used in ROM code.
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

#include "rwip_config.h"     // RW SW configuration

#include <string.h>          // for mem* functions
#include "co_version.h"
#include "rwip.h"            // RW definitions
#include "arch.h"            // Platform architecture definition
#include "em_map_ble.h"

#if (BLE_EMB_PRESENT)
#include "rwble.h"           // rwble definitions
#endif //BLE_EMB_PRESENT


#if (BLE_HOST_PRESENT)
#include "rwble_hl.h"        // BLE HL definitions
#include "gapc.h"
#include "smpc.h"
#include "gattc.h"
#include "attc.h"
#include "atts.h"
#include "l2cc.h"
#endif //BLE_HOST_PRESENT

#if (DEEP_SLEEP)
#if (BT_EMB_PRESENT)
#include "ld_sleep.h"        // definitions for sleep mode
#endif //BT_EMB_PRESENT
#if (BLE_EMB_PRESENT)
#include "lld_sleep.h"       // definitions for sleep mode
#endif //BLE_EMB_PRESENT
#include "led.h"             // led definitions
#endif //DEEP_SLEEP

#if (BLE_EMB_PRESENT)
#include "llc.h"
#endif //BLE_EMB_PRESENT
#if (DISPLAY_SUPPORT)
#include "display.h"         // display definitions
#endif //DISPLAY_SUPPORT

#if (BT_EMB_PRESENT || BLE_EMB_PRESENT)
#include "plf.h"             // platform definition
#include "rf.h"              // RF definitions
#endif //BT_EMB_PRESENT || BLE_EMB_PRESENT

#if (GTL_ITF)
#include "gtl.h"
#endif //GTL_ITF

#if (HCIC_ITF)
#if (BT_EMB_PRESENT)
#include "hci.h"             // HCI definition
#elif (BLE_EMB_PRESENT)
#include "hcic.h"            // HCI definition
#endif //BT_EMB_PRESENT / BLE_EMB_PRESENT
#endif //HCIC_ITF

#if (KE_SUPPORT)
#include "ke.h"              // kernel definition
#include "ke_event.h"        // kernel event
#include "ke_timer.h"        // definitions for timer
#include "ke_mem.h"          // kernel memory manager
#endif //KE_SUPPORT

#include "dbg.h"             // debug definition

#if (BT_EMB_PRESENT)
#include "reg_btcore.h"      // bt core registers
#endif //BT_EMB_PRESENT


#if ((BLE_APP_PRESENT) || ((BLE_HOST_PRESENT && (!GTL_ITF))))
#include "app.h"
#endif //BLE_APP_PRESENT

#include "nvds.h"


extern void main_func(void);
extern void rf_init_func(void);
extern void prf_init_func(void);
extern void uart_init_func(void);
extern void uart_flow_on_func(void);
extern void uart_flow_off_func(void);
extern void uart_finish_transfers_func(void);
extern void uart_read_func(void);
extern void uart_write_func(void);
extern void UART_Handler_func(void);
extern void lld_sleep_compensate_func(void);
extern void lld_sleep_init_func(void);
extern void lld_sleep_us_2_lpcycles_func(void);
extern void lld_sleep_lpcycles_2_us_func(void);
extern void hci_tx_done_func(void);
extern void hci_enter_sleep_func(void);
extern void lld_assessment_stat_func(struct co_buf_rx_desc *ptr_on_rxdesc,uint16_t conhdl);
extern void lld_evt_init_func(bool reset);
extern void ke_timer_init_func(void);
extern void ke_task_init_func(void);
extern void gtl_eif_init_func(void);
extern void llm_encryption_done_func(void);
extern uint8_t nvds_get_func(uint8_t, nvds_tag_len_t*, uint8_t*);
extern const struct rwip_eif_api* rwip_eif_get_func(uint8_t type);
extern const volatile struct LUT_CFG_struct LUT_CFG;

#if 0
#undef BLE_CONNECTION_MAX_USER
#define BLE_CONNECTION_MAX_USER 1

#undef OFFSET_BUFFERS
#undef EM_BLE_TX_OFFSET
#undef EM_BLE_RX_OFFSET
#undef EM_SYSMEM_START
#undef EM_SYSMEM_END
#undef EM_SYSMEM_SIZE

// WE DEFINE 6 LINKS AND SELECT EXCHANGE MEM. PAGES MAPPING CASE 23
#define OFFSET_BUFFERS (0x1300)  //BEGINNING OF BUFFERS IS 0x82000, START OF SYSRAM AFTER EMI


/// Offset of the TX buffer area
#define EM_BLE_TX_OFFSET         (OFFSET_BUFFERS)
/// Offset of the RX buffer area
//#define EM_BLE_RX_OFFSET         (EM_BLE_TX_OFFSET + BLE_TX_BUFFER_CNT_USER * REG_BLE_EM_TX_SIZE)
#define EM_BLE_RX_OFFSET         (EM_BLE_TX_OFFSET + BLE_TX_BUFFER_CNT * REG_BLE_EM_TX_SIZE)

#define EM_SYSMEM_START    (0x80000+EM_BLE_TX_OFFSET) //ABSOLUTE ADDRESS OF EM IN SYSRAM

#define EM_SYSMEM_END    (EM_BLE_RX_OFFSET + BLE_RX_BUFFER_CNT * REG_BLE_EM_RX_SIZE) //RELATIVE END ADDRESS OF EM IN SYSRAM

#define EM_SYSMEM_SIZE           (EM_SYSMEM_END-OFFSET_BUFFERS) //SIZE OF EM IN SYSRAM

#undef RWIP_HEAP_ENV_SIZE_USER
#define RWIP_HEAP_ENV_SIZE_USER         ((BLE_HEAP_ENV_SIZE+  BLEHL_HEAP_ENV_SIZE)  * BLE_CONNECTION_MAX_USER)

#undef BLE_HEAP_MSG_SIZE_USER
#undef BLEHL_HEAP_MSG_SIZE_USER
#undef RWIP_HEAP_MSG_SIZE_USER

#define BLE_HEAP_MSG_SIZE_USER     (256 * (BLE_CONNECTION_MAX_USER+1) + 80 * (BLE_CONNECTION_MAX_USER) + 96 * (2*BLE_CONNECTION_MAX_USER+1))
#define BLEHL_HEAP_MSG_SIZE_USER   (256 + 256 * BLE_CONNECTION_MAX_USER)
#define RWIP_HEAP_MSG_SIZE_USER    ( BLE_HEAP_MSG_SIZE_USER  +  BLEHL_HEAP_MSG_SIZE_USER   )

#endif

#define RWIP_HEAP_NON_RET_SIZE_JT	   	RWIP_HEAP_NON_RET_SIZE_USER //IN BYTES	
uint32_t rwip_heap_non_ret[RWIP_CALC_HEAP_LEN(RWIP_HEAP_NON_RET_SIZE_JT)]  __attribute__((section("heap_mem_area_not_ret")));

//in case of 8 links the DB heap is put in sysram

#define RWIP_HEAP_ENV_SIZE_JT	       	RWIP_HEAP_ENV_SIZE_USER 	//IN BYTES	

//#define RWIP_HEAP_DB_SIZE_JT			3000     	     // TBD IN BYTES
#if (BLE_APP_KEYBOARD)
#define RWIP_HEAP_DB_SIZE_JT			1512 //1024 //3072     	     // TBD IN BYTES
#else
#define RWIP_HEAP_DB_SIZE_JT			1024 //1024 //3072     	     // TBD IN BYTES
#endif

#if (BLE_APP_KEYBOARD)
#define RWIP_HEAP_MSG_SIZE_JT			RWIP_HEAP_MSG_SIZE_USER    //IN BYTES		
#else
#define RWIP_HEAP_MSG_SIZE_JT			1024 //RWIP_HEAP_MSG_SIZE_USER     	     // TBD IN BYTES
#endif
//uint8_t rwip_heap_ret[RWIP_HEAP_RET_END_JT-RWIP_HEAP_ENV_POS_JT]  __attribute__((at(RWIP_HEAP_ENV_POS_JT)));
#if (BLE_APP_KEYBOARD) || (BLE_APP_KEYBOARD_TESTER)
uint32_t rwip_heap_env_ret[RWIP_CALC_HEAP_LEN(RWIP_HEAP_ENV_SIZE_JT)]  	__attribute__((section("heap_mem_area1"),zero_init));
#else
uint32_t rwip_heap_env_ret[RWIP_CALC_HEAP_LEN(RWIP_HEAP_ENV_SIZE_JT)]  	__attribute__((section("heap_mem_area"),zero_init));
#endif
//uint32_t rwip_heap_db_ret[RWIP_CALC_HEAP_LEN(RWIP_HEAP_DB_SIZE_JT)]  	__attribute__((section("heap_mem_area"),zero_init));
#if (BLE_APP_KEYBOARD) || (BLE_APP_KEYBOARD_TESTER)
uint32_t rwip_heap_msg_ret[RWIP_CALC_HEAP_LEN(RWIP_HEAP_MSG_SIZE_JT)]  	__attribute__((section("heap_mem_area1"),zero_init));
#else
uint32_t rwip_heap_msg_ret[RWIP_CALC_HEAP_LEN(RWIP_HEAP_MSG_SIZE_JT)]  	__attribute__((section("heap_mem_area"),zero_init));
#endif

#if (BLE_APP_KEYBOARD) || (BLE_APP_KEYBOARD_TESTER)
uint32_t rwip_heap_db_ret[RWIP_CALC_HEAP_LEN(RWIP_HEAP_DB_SIZE_JT)]  	__attribute__((section("heap_mem_area0"),zero_init));
#else
uint32_t rwip_heap_db_ret[RWIP_CALC_HEAP_LEN(RWIP_HEAP_DB_SIZE_JT)]  	__attribute__((section("heap_mem_area"),zero_init));
#endif
//uint32_t rwip_heap_db_ret[RWIP_CALC_HEAP_LEN(RWIP_HEAP_DB_SIZE_JT)]  	__attribute__((section("heap_mem_area_not_ret")));

const uint32_t* const jump_table_base[52] __attribute__((section("jump_table_mem_area"))) =
        {
#if (BLE_APP_PRESENT)
                (const uint32_t*) TASK_APP,                  
#else           
                (const uint32_t*) TASK_GTL,
#endif
                (const uint32_t*) main_func,                     // 1. main_func
                (const uint32_t*) rf_init_func,                  // 2. rf_init_func
                (const uint32_t*) prf_init_func,                 // 3. prf_init_func
                (const uint32_t*) 0,                                      // 4. calibrate_rc32KHz_func
                (const uint32_t*) 0,                                      // 5. calibrate_RF_func
                (const uint32_t*) uart_init_func,                // 6. uart_init_func
                (const uint32_t*) uart_flow_on_func,             // 7. uart_flow_on_func
                (const uint32_t*) uart_flow_off_func,            // 8. uart_flow_off_func
                (const uint32_t*) uart_finish_transfers_func,    // 9. uart_finish_transfers_func
                (const uint32_t*) uart_read_func,                // 10.uart_read_func
                (const uint32_t*) uart_write_func,               // 11.uart_write_func
                (const uint32_t*) UART_Handler_func,             // 12.UART_Handler_func			
                (const uint32_t*) lld_sleep_compensate_func,     // 13.correcton counters after wakeUp
                (const uint32_t*) lld_sleep_init_func,           // 14.init sleep
                (const uint32_t*) lld_sleep_us_2_lpcycles_func,  // 15.calc us to lp
                (const uint32_t*) lld_sleep_lpcycles_2_us_func,  // 16.calc lp to us
                (const uint32_t*) 0,//(uint32) hci_tx_done_func,          // 17.hci tx done
                (const uint32_t*) 0,//(uint32) hci_enter_sleep_func,      // 18.hci allows sleep
#if (BLE_APP_PRESENT)
                (const uint32_t*) TASK_APP_SEC,		            // 19. app task
#else
                (const uint32_t*) TASK_GTL,
#endif
                (const uint32_t*) &rwip_heap_non_ret[0],          // 20. rwip_heap_non_ret_pos
                (const uint32_t*) RWIP_HEAP_NON_RET_SIZE_JT,      // 21. rwip_heap_non_ret_size
                (const uint32_t*) &rwip_heap_env_ret[0],          // 22. rwip_heap_env_pos
                (const uint32_t*) RWIP_HEAP_ENV_SIZE_JT,          // 23. rwip_heap_env_size
                (const uint32_t*) &rwip_heap_db_ret[0],           // 24. rwip_heap_db_pos
                (const uint32_t*) RWIP_HEAP_DB_SIZE_JT,           // 25. rwip_heap_db_size
                (const uint32_t*) &rwip_heap_msg_ret[0],          // 26. rwip_heap_msg_pos
                (const uint32_t*) RWIP_HEAP_MSG_SIZE_JT,          // 27. rwip_heap_msg_size	
				(const uint32_t*) EM_BLE_ET_OFFSET,               // 28. offset_em_et		         
				(const uint32_t*) EM_BLE_FT_OFFSET,               // 29. offset_em_ft	         
				(const uint32_t*) EM_BLE_ENC_PLAIN_OFFSET,        // 30. offset_em_enc_plain	
				(const uint32_t*) EM_BLE_ENC_CIPHER_OFFSET,       // 31. offset_em_enc_cipher	                
				(const uint32_t*) EM_BLE_CS_OFFSET,               // 32. offset_em_cs	                 
				(const uint32_t*) EM_BLE_WPB_OFFSET,              // 33. offset_em_wpb
                (const uint32_t*) EM_BLE_WPV_OFFSET,              // 34. offset_em_wpv                
				(const uint32_t*) EM_BLE_CNXADD_OFFSET,           // 35. offset_em_cnxadd	
				
                (const uint32_t*) EM_BLE_TXE_OFFSET,              // 36. offset_em_txe
				(const uint32_t*) EM_BLE_TX_OFFSET,               // 37. offset_em_tx
				(const uint32_t*) EM_BLE_RX_OFFSET,               // 38. offset_em_rx
				(const uint32_t*) BLE_CONNECTION_MAX_USER,	    // 39. nb_links_user
				(const uint32_t*) 5000,									// 40.
				(const uint32_t*) lld_assessment_stat_func,		// 41.
				(const uint32_t*) lld_evt_init_func,				// 42.
				(const uint32_t*) gtl_eif_init_func,				// 43.
				(const uint32_t*) ke_task_init_func,				// 44.
				(const uint32_t*) ke_timer_init_func,			// 45.
				(const uint32_t*) llm_encryption_done_func,		// 46. 
				(const uint32_t*) custom_nvds_get_func,					// 47.
				(const uint32_t*) 2,										// 48.
				(const uint32_t*) rwip_eif_get_func,				// 49.
                (const uint32_t*) &LUT_CFG,                     //50. lut_cfg_pos
        };


        
