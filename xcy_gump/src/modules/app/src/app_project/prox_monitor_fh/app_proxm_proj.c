/**
****************************************************************************************
*
* @file app_proxr_proj.c
*
* @brief Proximity project source code .
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

/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */
#include "rwip_config.h"             // SW configuration
#include "periph_setup.h"             // SW configuration
#include "wkupct_quadec.h"             // SW configuration

#if (BLE_APP_PRESENT)

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "app_sec.h"
#include "app_proxm_proj.h"
#include "arch_sleep.h"

#include "co_math.h"                 // Common Maths Definition

#include "disc.h"

#if (NVDS_SUPPORT)
#include "nvds.h"                    // NVDS Definitions
#endif //(NVDS_SUPPORT)

#define MAX_SCAN_DEVICES BLE_CONNECTION_MAX_USER
#define RSSI_SAMPLES	 5   
#define DIS_VAL_MAX_LEN                         (0x12)

typedef struct 
{
    unsigned char free;
    struct bd_addr adv_addr;
    unsigned short conidx;
    unsigned short conhdl;
    unsigned char idx;
    char  rssi;
    unsigned char  data_len;
    unsigned char  data[ADV_DATA_LEN + 1];
} ble_dev;

typedef struct 
{
    uint16_t    val_hdl;
    uint8_t     val[DIS_VAL_MAX_LEN + 1];
    uint16_t    len;
} dis_char;

//DIS information
typedef struct 
{
    dis_char chars[DISC_CHAR_MAX];
} dis_env;

//Proximity Reporter connected device
typedef struct 
{
    ble_dev device;
    unsigned char bonded;
    unsigned short ediv;
    struct rand_nb rand_nb[RAND_NB_LEN];
    struct gapc_ltk ltk;
    struct gapc_irk irk;
    struct gap_sec_key csrk;
    unsigned char llv;
    char txp;
    char rssi[RSSI_SAMPLES];
    char rssi_indx;
    char avg_rssi;
    unsigned char alert;
    dis_env dis;
} proxr_dev;


/// application environment structure
struct app_host_tag
{
    unsigned char state;
    unsigned char num_of_devices;
    ble_dev devices[MAX_SCAN_DEVICES];
    proxr_dev proxr_device;
};

struct app_host_tag app_host;

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
*/

static inline void test_led_io(uint8 flag, uint8 value, GPIO_PORT port, GPIO_PIN pin)
{
	if (value&flag) {
		GPIO_SetActive(port, pin);
	} else {
		GPIO_SetInactive(port, pin);
	}
}
void test_led(uint8_t value)
{
	test_led_io(0x01, value, XCY_LED0_GPIO);
	test_led_io(0x02, value, XCY_LED1_GPIO);
	test_led_io(0x04, value, XCY_LED2_GPIO);
	test_led_io(0x08, value, XCY_LED3_GPIO);
}

/**
 ****************************************************************************************
 * @brief Send Inquiry (devices discovery) request to GAP task.
 *
 * @return void.
 ****************************************************************************************
 */
void app_inq(void)
{
	struct gapm_start_scan_cmd *msg = KE_MSG_ALLOC_DYN(GAPM_START_SCAN_CMD, 
		TASK_GAPM, 
		TASK_APP,
		gapm_start_scan_cmd,
		sizeof(struct gap_bdaddr)*2);
	int i;

	test_led(2);

	for (i=0; i<MAX_SCAN_DEVICES; i++) {
		app_host.devices[i].free = true;
		app_host.devices[i].adv_addr.addr[0] = '\0';
		app_host.devices[i].data[0] = '\0';
		app_host.devices[i].data_len = 0;
		app_host.devices[i].rssi = 0;
	}

	msg->mode = GAP_GEN_DISCOVERY;
	msg->op.code = GAPM_SCAN_ACTIVE;
	msg->op.addr_src = GAPM_PUBLIC_ADDR;
	msg->filter_duplic = SCAN_FILT_DUPLIC_EN;
	msg->interval = 10;
	msg->window = 5;

	ke_msg_send((void *)msg);

	return;
}

/*
 * Name         : app_connect_func - Connect Report fh
 *
 * Scope        : PUBLIC
 *
 * Arguments    : task_id - id of the kernel task calling this function
 *                cmd - parameters to pass to the stack
 *
 * Description  : Configures Bluetooth settings for the Keyboard Test application.
 *
 * Returns      : void
 *
 */
void app_connect_func(uint8 indx)
{
    struct gapm_start_connection_cmd *msg;
	// uint8 indx = 0;

    if (app_host.devices[indx].free == true)
    {
        return;
    }
    
    msg = (struct gapm_start_connection_cmd *) KE_MSG_ALLOC_DYN(GAPM_START_CONNECTION_CMD, 
                                                                TASK_GAPM, 
                                                                TASK_APP, 
                                                                gapm_start_connection_cmd, 
                                                                sizeof(struct gap_bdaddr)*2);

    msg->nb_peers = 1;
    memcpy((void *) &msg->peers[0].addr, (void *)&app_host.devices[indx].adv_addr.addr, BD_ADDR_LEN);
    msg->con_intv_min = 100;
    msg->con_intv_max = 100;
    msg->ce_len_min = 0x0;
    msg->ce_len_max = 0x5;
    msg->con_latency = 0;
    msg->op.addr_src = GAPM_PUBLIC_ADDR;
    msg->peers[0].addr_type = GAPM_PUBLIC_ADDR;
    msg->superv_to = 0x1F4;// 500 -> 5000 ms ;
    msg->scan_interval = 0x180;
    msg->scan_window = 0x160;
    msg->op.code = GAPM_CONNECTION_DIRECT;

    // Send the message
    ke_msg_send(msg);    
}

/**
 ****************************************************************************************
 * @brief Extracts device name from adv data if present and copy it to app_env.
 *
 * @param[in] adv_data      Pointer to advertise data.
 * @param[in] adv_data_len  Advertise data length.
 * @param[in] dev_indx      Devices index in device list.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static void app_find_device_name(unsigned char * adv_data, unsigned char adv_data_len, unsigned char dev_indx)
{
    unsigned char indx = 0;

    while (indx < adv_data_len)
    {
        if (adv_data[indx+1] == 0x09)
        {
            memcpy(app_host.devices[dev_indx].data, &adv_data[indx+2], (size_t) adv_data[indx]);
            app_host.devices[dev_indx].data_len = (unsigned char ) adv_data[indx];
        }
        indx += (unsigned char ) adv_data[indx]+1;
    }
}

/**
 ****************************************************************************************
 * @brief bd adresses compare.
 *
 *  @param[in] bd_address1  Pointer to bd_address 1.
 *  @param[in] bd_address2  Pointer to bd_address 2.
 *
 * @return true if adresses are equal / false if not.
 ****************************************************************************************
 */
bool bdaddr_compare(struct bd_addr *bd_address1,
                    struct bd_addr *bd_address2)
{
    unsigned char idx;

    for (idx=0; idx < BD_ADDR_LEN; idx++)
    {
        /// checks if the addresses are similar
        if (bd_address1->addr[idx] != bd_address2->addr[idx])
        {
           return (false);
        }
    }
    return(true);
}

uint8 app_device_recorded(struct bd_addr *padv_addr)
{
    int i;

    for (i=0; i < MAX_SCAN_DEVICES; i++)
    {
        if (app_host.devices[i].free == false)
            if (bdaddr_compare(&app_host.devices[i].adv_addr, padv_addr))
                break;
    }

    return i;
}

/**
 ****************************************************************************************
 * @brief app_api function. Handles direct advertising completion.
 *
 * @param[in] status        Command complete message status
 *
 * @return void.
 ****************************************************************************************
*/
void app_scan_complete(struct gapm_adv_report_ind *param)
{
	int recorded;
	int a = 0;
	recorded = app_device_recorded(&param->report.adv_addr);
	if (recorded <MAX_SCAN_DEVICES) //update Name
	{
		// app_find_device_name(param->report.data,param->report.data_len, recorded); 
		// ConsoleScan();
	} else {
		app_host.devices[app_host.num_of_devices].free = false;
		app_host.devices[app_host.num_of_devices].rssi = (char)(((479 * param->report.rssi)/1000) - 112.5) ;
		memcpy(app_host.devices[app_host.num_of_devices].adv_addr.addr, param->report.adv_addr.addr, BD_ADDR_LEN );
		// app_find_device_name(param->report.data,param->report.data_len, app_host.num_of_devices); 
		// ConsoleScan();
		a = app_host.num_of_devices;
		app_host.num_of_devices++;
		test_led(a);
	}       
            
    return;
}

int gapm_adv_report_ind_handler(ke_msg_id_t msgid,
                                const struct gapm_adv_report_ind *param,
                                ke_task_id_t dest_id,
                                ke_task_id_t src_id)
{
	test_led(3);
	app_scan_complete((struct gapm_adv_report_ind*)param);  
	return 0;
}



/**
 ****************************************************************************************
 * @brief Button press callback function. Registered in WKUPCT driver.
 *
 * @return void.
 ****************************************************************************************
 */

void app_button_press_cb(void)
{
        
#if BLE_PROX_REPORTER	
		if (alert_state.lvl != PROXR_ALERT_NONE)
		{
			app_proxr_alert_stop();	
		}
        else 
#endif         

#if BLE_PROX_MONITOR
		if (alert_state.lvl != PROXM_ALERT_NONE)
		{
			app_proxm_alert_stop(); 
		}
		else 
#endif         
				

        {
#if BLE_FINDME_LOCATOR            
        if (ke_state_get(TASK_FINDL) == FINDL_CONNECTED)
        {
            app_findl_set_alert();  
        }
		
#endif 
        }
        
        if(GetBits16(SYS_STAT_REG, PER_IS_DOWN))
            periph_init(); 
        
        if (app_ble_ext_wakeup_get())
        {
        
            //Wakeup BLE here
            
#if (EXT_SLEEP_ENABLED)
            app_set_extended_sleep();
#elif (DEEP_SLEEP_ENABLED)
            app_set_deep_sleep();
#else
            app_disable_sleep();
#endif    
            
            SetBits32(GP_CONTROL_REG, BLE_WAKEUP_REQ, 1); 
            app_ble_ext_wakeup_off();
            
            ke_msg_send_basic(APP_WAKEUP_MSG, TASK_APP, NULL);
        }
        
        app_button_enable();
        
}

/**
 ****************************************************************************************
 * @brief Enable push button. Register callback function for button press event. Must be called in periph_init();
 *
 * @return void.
 ****************************************************************************************
 */


void app_button_enable(void)
{   
   wkupct_register_callback(app_button_press_cb);

   if (!GPIO_GetPinStatus(GPIO_PORT_2, GPIO_PIN_3))
        wkupct_enable_irq(0x480000, 0x0, 1, 0); // P2_3,6, polarity high , 1 event, debouncing time = 0
#if 0
   if (!GPIO_GetPinStatus(XCY_KB_JL))
        wkupct_enable_irq(0x400000, 0x0, 1, 0); // P2_6, polarity high , 1 event, debouncing time = 0
   if (GPIO_GetPinStatus(XCY_KB_SCH))
        wkupct_enable_irq(0x1000000, 0x1000000, 1, 0); // P2_9, polarity low , 1 event, debouncing time = 0
#endif
}

 /**
 ****************************************************************************************
 * @brief app_api function. Project's actions in app_init
 *
 * @return void.
 ****************************************************************************************
*/

void app_init_func(void)
{	
    
#if (BLE_PROX_MONITOR)    
	app_proxm_init(XCY_LED0_GPIO);
   
    alert_state.ll_alert_lvl = PROXM_ALERT_HIGH; //Link Loss default Alert Level 
#elif (BLE_FINDME_LOCATOR)
    app_findl_init();
#endif

#if (BLE_BATT_SERVER)    
    app_batt_init();
#endif
    
#if (BLE_DIS_SERVER)        
    app_dis_init();
#endif
    
}

/**
 ****************************************************************************************
 * @brief app_api function. Project's actions in app_sec_init during system initialization
 *
 * @return void.
 ****************************************************************************************
*/

void app_sec_init_func(void)
{
    
#if (BLE_APP_SEC)
	app_sec_env.auth = (GAP_AUTH_REQ_NO_MITM_BOND);
#endif
    
}


/**
 ****************************************************************************************
 * @brief app_api function. Project's actions in app_connect (device connection)
 *
 * @param[in] param      Received gapc_connection_req_ind.
 *
 * @return bool     true for connection request acception / false rejection.
 ****************************************************************************************
*/

void app_connection_func(struct gapc_connection_req_ind const *param)
{
    
    if (app_env.conidx != GAP_INVALID_CONIDX)
    {
        
        /*--------------------------------------------------------------
        * ENABLE REQUIRED PROFILES
        *-------------------------------------------------------------*/

        #if (BLE_PROX_REPORTER)
        app_proxr_enable();
        #endif //BLE_PROX_REPORTER
        
		#if (BLE_PROX_MONITOR)
        app_proxm_enable();
		#endif //BLE_PROX_MONITOR
        
        #if BLE_BATT_SERVER
        //cur_batt_level = 0;
        app_batt_enable(cur_batt_level, 1, GPIO_BAT_LED_PORT, GPIO_BAT_LED_PIN); //Battery level Alert enabled.
        
        app_batt_poll_start(6000); //10 mins
        #endif // BLE_BATT_SERVER

        #if BLE_FINDME_TARGET
        app_findt_enable();
        #endif // BLE_FINDME_TARGET
        
        #if BLE_FINDME_LOCATOR
        app_findl_enable();
        #endif //BLE_FINDME_LOCATOR	
        
        #if (BLE_APP_PRESENT)
        // app_dis_enable_prf(app_env.conhdl);
        #endif

        ke_state_set(TASK_APP, APP_CONNECTED);
            
        // Retrieve the connection info from the parameters
        app_env.conhdl = param->conhdl;

        app_env.peer_addr_type = param->peer_addr_type;
        memcpy(app_env.peer_addr.addr, param->peer_addr.addr, BD_ADDR_LEN);

# if (BLE_APP_SEC)
            
        // send connection confirmation
        app_connect_confirm(app_sec_env.auth);
        
# else // (BLE_APP_SEC)
        // send connection confirmation
        app_connect_confirm(GAP_AUTH_REQ_NO_MITM_NO_BOND);            
# endif // (BLE_APP_SEC)

        ke_timer_clear(APP_ADV_TIMER, TASK_APP); 
    }
    else
    {
        // No connection has been establish, restart advertising
        app_adv_start();
    }
    
    return;

}

/**
 ****************************************************************************************
 * @brief app_api function. Project advertising function. Setup advertise string.
 *
 * @return void.
 ****************************************************************************************
*/

void app_adv_func(struct gapm_start_advertise_cmd *cmd)
{
    return;
}

/**
 ****************************************************************************************
 * @brief app_api function. Project's actions in app_disconnect
 *
 * @param[in] taskid     App task's id.
 * @param[in] param      Received gapc_disconnect_ind msg.
 *
 * @return void.            
 ****************************************************************************************
*/

void app_disconnect_func(ke_task_id_t task_id, struct gapc_disconnect_ind const *param)
{
    
    uint8_t state = ke_state_get(task_id);
    
#if BLE_BATT_SERVER
	app_batt_poll_stop();
#endif // BLE_BATT_SERVER
    
    if ((state == APP_SECURITY) || (state == APP_CONNECTED)  || (state == APP_PARAM_UPD))
    {
        // Restart Advertising
        app_adv_start();
    }
    else
    {
        // We are not in a Connected State
        ASSERT_ERR(0);
    }
						
}

/**
 ****************************************************************************************
 * @brief app_api function. Project's actions for profiles' database initialization
 *
 * @return void.
 ****************************************************************************************
*/

bool app_db_init_func(void)
{
    return true;
}

/**
 ****************************************************************************************
 * @brief app_api function. Called upon connection param's update rejection
 *
 * @param[in] status        Error code
 *
 * @return void.
 ****************************************************************************************
*/

void app_update_params_rejected_func(uint8_t status)
{
    return;
}

/**
 ****************************************************************************************
 * @brief app_api function. Sends request to update connection's paramaters.
 *
 * @return void.
 ****************************************************************************************
*/
void app_param_update_func(void)
{

    return;
}

/**
 ****************************************************************************************
 * @brief app_api function. Project configures GAPM. Called upon reset completion
 *
 * @param[in] task_id    id of the kernel task calling this function
 * @param[in] cmd        parameters to pass to the stack
 *
 * @return void.
 ****************************************************************************************
*/

void app_configuration_func(ke_task_id_t const task_id, struct gapm_set_dev_config_cmd *cmd)
{
    // set device configuration
    cmd->operation = GAPM_SET_DEV_CONFIG;
    // Device Role
    cmd->role = GAP_CENTRAL_MST; // GAP_PERIPHERAL_SLV; // 
    // Device Appearance
    cmd->appearance = 0x0000;
    // Device Appearance write permission requirements for peer device
    cmd->appearance_write_perm = GAPM_WRITE_DISABLE;
    // Device Name write permission requirements for peer device
    cmd->name_write_perm = GAPM_WRITE_DISABLE;    
    return;   
}

/**
 ****************************************************************************************
 * @brief app_api function. Handles Set device's configuration complete message
 *
 * @return void.
 ****************************************************************************************
*/

void app_set_dev_config_complete_func(void)
{
	app_inq();
#if 0
    // We are now in Initialization State
    ke_state_set(TASK_APP, APP_DB_INIT);

    // Add the first required service in the database
    if (app_db_init())
    {
        // No service to add in the DB -> Start Advertising
        app_adv_start();
    }
#endif
    return;
}

/**
 ****************************************************************************************
 * @brief app_api function. Handles Update connection's params complete message
 *
 * @return void.
 ****************************************************************************************
*/

void app_update_params_complete_func(void)
{
    return;
}

/**
 ****************************************************************************************
 * @brief app_api function. Handles undirect advertising completion.
 *
 * @param[in] status        Command complete message status
 *
 * @return void.
 ****************************************************************************************
*/

void app_adv_undirect_complete(uint8_t status)
{
    
    ke_timer_clear(APP_ADV_TIMER, TASK_APP);
 	app_inq();
	test_led(4);
    return;
}

/**
 ****************************************************************************************
 * @brief app_api function. Handles direct advertising completion.
 *
 * @param[in] status        Command complete message status
 *
 * @return void.
 ****************************************************************************************
*/

void app_adv_direct_complete(uint8_t status)
{
    return;
}

/**
 ****************************************************************************************
 * @brief app_api function. Handles Database creation. Start application.
 *
 * @return void.
 ****************************************************************************************
*/

void app_db_init_complete_func(void)
{
    
    app_adv_start();
    
    return;
}

#if (BLE_APP_SEC)
void app_send_pairing_rsp_func(struct gapc_bond_req_ind *param)
{
    
    struct gapc_bond_cfm* cfm = KE_MSG_ALLOC(GAPC_BOND_CFM, TASK_GAPC, TASK_APP, gapc_bond_cfm);

    cfm->request = GAPC_PAIRING_RSP;
    cfm->accept = true;

    // OOB information
    cfm->data.pairing_feat.oob            = GAP_OOB_AUTH_DATA_NOT_PRESENT;
    // Encryption key size
    cfm->data.pairing_feat.key_size       = KEY_LEN;
    // IO capabilities
    cfm->data.pairing_feat.iocap          = GAP_IO_CAP_NO_INPUT_NO_OUTPUT;
    // Authentication requirements
    cfm->data.pairing_feat.auth           = GAP_AUTH_REQ_NO_MITM_BOND;
    //Security requirements
    cfm->data.pairing_feat.sec_req        = GAP_NO_SEC;
    //Initiator key distribution
    cfm->data.pairing_feat.ikey_dist      = GAP_KDIST_SIGNKEY;
    //Responder key distribution
    cfm->data.pairing_feat.rkey_dist      = GAP_KDIST_ENCKEY;
    
    ke_msg_send(cfm);
}

void app_send_tk_exch_func(struct gapc_bond_req_ind *param)
{
    struct gapc_bond_cfm* cfm = KE_MSG_ALLOC(GAPC_BOND_CFM, TASK_GAPC, TASK_APP, gapc_bond_cfm);
    uint32_t pin_code = app_sec_gen_tk();
    cfm->request = GAPC_TK_EXCH;
    cfm->accept = true;
    
    memset(cfm->data.tk.key, 0, KEY_LEN);
    
    cfm->data.tk.key[3] = (uint8_t)((pin_code & 0xFF000000) >> 24);
    cfm->data.tk.key[2] = (uint8_t)((pin_code & 0x00FF0000) >> 16);
    cfm->data.tk.key[1] = (uint8_t)((pin_code & 0x0000FF00) >>  8);
    cfm->data.tk.key[0] = (uint8_t)((pin_code & 0x000000FF) >>  0);
    
    ke_msg_send(cfm);
}

void app_send_irk_exch_func(struct gapc_bond_req_ind *param)
{
    return;
}

void app_send_csrk_exch_func(struct gapc_bond_req_ind *param)
{
    struct gapc_bond_cfm* cfm = KE_MSG_ALLOC(GAPC_BOND_CFM, TASK_GAPC, TASK_APP, gapc_bond_cfm);

    // generate ltk
    app_sec_gen_ltk(param->data.key_size);

    cfm->request = GAPC_CSRK_EXCH;

    cfm->accept = true;

    memset((void *) cfm->data.csrk.key, 0, KEY_LEN);
    memcpy((void *) cfm->data.csrk.key, (void *)"\xAB\xAB\x45\x55\x23\x01", 6);

    ke_msg_send(cfm);

}

void app_send_ltk_exch_func(struct gapc_bond_req_ind *param)
{
    
    struct gapc_bond_cfm* cfm = KE_MSG_ALLOC(GAPC_BOND_CFM, TASK_GAPC, TASK_APP, gapc_bond_cfm);
    
    // generate ltk
    app_sec_gen_ltk(param->data.key_size);

    cfm->request = GAPC_LTK_EXCH;

    cfm->accept = true;

    cfm->data.ltk.key_size = app_sec_env.key_size;
    cfm->data.ltk.ediv = app_sec_env.ediv;

    memcpy(&(cfm->data.ltk.randnb), &(app_sec_env.rand_nb) , RAND_NB_LEN);
    memcpy(&(cfm->data.ltk.ltk), &(app_sec_env.ltk) , KEY_LEN);

    ke_msg_send(cfm);

}

void app_paired_func(void)
{
    return;
}

bool app_validate_encrypt_req_func(struct gapc_encrypt_req_ind *param)
{
    return true;
}

void app_sec_encrypt_ind_func(void)
{
    
    return; 
}

void app_sec_encrypt_complete_func(void)
{
    return;
}

void app_mitm_passcode_entry_func(ke_task_id_t const src_id, ke_task_id_t const dest_id)
{
    return;
}
#endif //BLE_APP_SEC

/**
 ****************************************************************************************
 * @brief Advertise timer handler. Set device in deep sleep mode
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 * @return int.
 ****************************************************************************************
*/

int app_adv_timer_handler(ke_msg_id_t const msgid,
                                   void const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
  
    app_adv_stop();
    
    ke_timer_clear(APP_ADV_TIMER, TASK_APP);
    
    app_set_deep_sleep();
    app_ble_ext_wakeup_on();
    app_button_enable();
    
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles APP_WAKEUP_MSG sent when device exits deep sleep. Trigerred by button press.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 * @return int.
 ****************************************************************************************
*/

int app_wakeup_handler(ke_msg_id_t const msgid,
									void *param,
									ke_task_id_t const dest_id,
									ke_task_id_t const src_id)
{
	// If state is not idle, ignore the message
	if (ke_state_get(dest_id) == APP_CONNECTABLE)
		app_adv_start();

	return (KE_MSG_CONSUMED);
}

#endif  //BLE_APP_PRESENT
/// @} APP
