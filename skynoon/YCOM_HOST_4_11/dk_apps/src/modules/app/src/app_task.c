/**
 ****************************************************************************************
 *
 * @file app_task.c
 *
 * @brief RW APP Task implementation
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup APPTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"               // SW configuration
#include <string.h> 

#if (BLE_APP_PRESENT)

#include "app_task.h"                  // Application Task API
#include "app.h"                       // Application Definition
#include "app_sec.h"                   // Application Definition
#include "app_sec_task.h"              // Application Security Task API
#include "gapc_task.h"                 // GAP Controller Task API
#include "gapm_task.h"                 // GAP Manager Task API
#include "gap.h"                       // GAP Definitions
#include "gapc.h"                      // GAPC Definitions
#include "co_error.h"                  // Error Codes Definition
#include "arch.h"                      // Platform Definitions

#define APP_TASK_HANDLERS_INCLUDE
#include "app_task_handlers.h"
#undef APP_TASK_HANDLERS_INCLUDE

#if(ROLE_MASTER_YCOM)
extern struct xapp_env_tag xapp_env;    //randy
extern  unsigned char app_device_recorded(struct bd_addr *padv_addr);  //randy
extern void GPIO_SetActive( GPIO_PORT port, GPIO_PIN pin );
#define GPIO_ALERT_LED_PORT     GPIO_PORT_1
#define GPIO_ALERT_LED_PIN      GPIO_PIN_1
#define GPIO_ALERT_LED1_PIN     GPIO_PIN_2
#define GPIO_ALERT_LED2_PIN     GPIO_PIN_3

extern void app_cancel(void);
extern bool bdaddr_compare(struct bd_addr *bd_address1,
                    struct bd_addr *bd_address2);
extern void app_proxm_enable(void);
extern void app_disc_enable(void);
extern void app_security_enable(void);
extern void app_start_encryption(void);
extern void app_proxm_read_txp(void);
extern bool existence_flag;
#endif
/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles ready indication from the GAP. - Reset the stack
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int gapm_device_ready_ind_handler(ke_msg_id_t const msgid,
                                         void const *param,
                                         ke_task_id_t const dest_id,
                                         ke_task_id_t const src_id)
{
		int i;
    if (ke_state_get(dest_id) == APP_DISABLED)
    {
        // reset the lower layers.
        struct gapm_reset_cmd* cmd = KE_MSG_ALLOC(GAPM_RESET_CMD, TASK_GAPM, TASK_APP,
                gapm_reset_cmd);
        cmd->operation = GAPM_RESET;
#if (ROLE_MASTER_YCOM)			
				xapp_env.state = XAPP_IDLE;  
         //init scan devices list
        xapp_env.num_of_devices = 0;
			
    for (i=0; i < MAX_SCAN_DEVICES; i++)
    {
        xapp_env.devices[i].free = true;
        xapp_env.devices[i].adv_addr.addr[0] = '\0';
        xapp_env.devices[i].data[0] = '\0';
        xapp_env.devices[i].data_len = 0;
        xapp_env.devices[i].rssi = 0;

    }
#endif
        ke_msg_send(cmd);		
    }
    else
    {
        // APP_DISABLED state is used to wait the GAP_READY_EVT message
        ASSERT_ERR(0);
    }

    return (KE_MSG_CONSUMED);
}
#if (ROLE_MASTER_YCOM)
static void app_find_device_name(unsigned char * adv_data, unsigned char adv_data_len, unsigned char dev_indx)
{
    unsigned char indx = 0;

    while (indx < adv_data_len)
    {
        if (adv_data[indx+1] == 0x09)
        {
            memcpy(xapp_env.devices[dev_indx].data, &adv_data[indx+2], (size_t) adv_data[indx]);
            xapp_env.devices[dev_indx].data_len = (unsigned char ) adv_data[indx];
        }
        indx += (unsigned char ) adv_data[indx]+1;
    }
}

void app_inq(void)                  //randy
{
    struct gapm_start_scan_cmd *cmd = KE_MSG_ALLOC(GAPM_START_SCAN_CMD, TASK_GAPM, TASK_APP,
                                                 gapm_start_scan_cmd);

		 int i;	//app_env.xxx changed to xapp --randy
    //init scan devices list
    xapp_env.num_of_devices = 0;
	
    for (i=0; i < MAX_SCAN_DEVICES; i++)
    {
        
        xapp_env.devices[i].free = true;
        xapp_env.devices[i].adv_addr.addr[0] = '\0';
        xapp_env.devices[i].data[0] = '\0';
        xapp_env.devices[i].data_len = 0;
        xapp_env.devices[i].rssi = 0;
    }
    //init scan devices list

    cmd->mode = GAP_GEN_DISCOVERY;
    cmd->op.code = GAPM_SCAN_ACTIVE;
    cmd->op.addr_src = GAPM_PUBLIC_ADDR;
    cmd->filter_duplic = SCAN_FILT_DUPLIC_EN;
    cmd->interval = 10;
    cmd->window = 5;

    ke_msg_send(cmd);
    return;
}

void app_connect(unsigned char indx)
{
	  if (xapp_env.devices[indx].free == true)
    {
        return;
    }
    struct gapm_start_connection_cmd *cmd= KE_MSG_ALLOC(GAPM_START_CONNECTION_CMD , TASK_GAPM, TASK_APP,gapm_start_connection_cmd );                                              
    cmd->nb_peers = 1;
    //cmd->nb_peers = 0;
    memcpy((void *) &cmd->peers[0].addr, (void *)&xapp_env.devices[indx].adv_addr.addr, BD_ADDR_LEN);
    cmd->con_intv_min = 100;
    cmd->con_intv_max = 100;
    cmd->ce_len_min = 0x0;
    cmd->ce_len_max = 0x5;
    cmd->con_latency = 0;
    cmd->op.addr_src = GAPM_PUBLIC_ADDR;
    cmd->peers[0].addr_type = GAPM_PUBLIC_ADDR;
    cmd->superv_to = 0x1F4;// 500 -> 5000 ms ;
    cmd->scan_interval = 0x180;
		cmd->scan_window = 0x160;
    cmd->op.code = GAPM_CONNECTION_DIRECT;
    //cmd->op.code = GAPM_CONNECTION_AUTO; //自动连接		
    ke_msg_send(cmd);
	
		
}


int gapm_adv_report_ind_handler(ke_msg_id_t msgid,  
                                struct gapm_adv_report_ind *param,
                                ke_task_id_t dest_id,
                                ke_task_id_t src_id)
{	
	
 unsigned char recorded;

	//unsigned char  i;
    if (xapp_env.state != XAPP_SCAN)
        return -1;

        recorded = app_device_recorded(&param->report.adv_addr); //only 0 and 9 active mack
		/*
        if(recorded<5)
				{
			  	  GPIO_ConfigurePin( GPIO_PORT_1, GPIO_PIN_1, OUTPUT, PID_GPIO, false ); //Alert LED
				    GPIO_ConfigurePin( GPIO_PORT_1, GPIO_PIN_2, OUTPUT, PID_GPIO, false ); //Alert LED
				}
				if(recorded>=20)
				{
				   // GPIO_ConfigurePin( GPIO_PORT_1, GPIO_PIN_1, OUTPUT, PID_GPIO, true ); //Alert LED
				   // GPIO_ConfigurePin( GPIO_PORT_1, GPIO_PIN_2, OUTPUT, PID_GPIO, true ); //Alert LED
				}*/
        if (recorded <MAX_SCAN_DEVICES) //update Name
        {

            app_find_device_name(param->report.data,param->report.data_len, recorded); 
        }
        else
        {
            xapp_env.devices[xapp_env.num_of_devices].free = false;
            xapp_env.devices[xapp_env.num_of_devices].rssi = (char)(((479 * param->report.rssi)/1000) - 112.5) ;
            memcpy(xapp_env.devices[xapp_env.num_of_devices].adv_addr.addr, param->report.adv_addr.addr, BD_ADDR_LEN );
            app_find_device_name(param->report.data,param->report.data_len, xapp_env.num_of_devices); 
            //app_find_device_name(param->report.data,param->report.data_len, i); 
            xapp_env.num_of_devices++;
					
         }
				
				if( xapp_env.num_of_devices!=0)
				{
					existence_flag=1;

				}
				else
				{
					existence_flag=0;
				}
				
				return 0;
		
}				
#endif
/**
 ****************************************************************************************
 * @brief Handles GAP manager command complete events.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int gapm_cmp_evt_handler(ke_msg_id_t const msgid,
                                struct gapm_cmp_evt const *param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id)
{
    switch(param->operation)
    {
        // reset completed
        case GAPM_RESET:
        {
            if(param->status != GAP_ERR_NO_ERROR)
            {
                ASSERT_ERR(0); // unexpected error
            }
            else
            {
                // set device configuration
                struct gapm_set_dev_config_cmd* cmd = KE_MSG_ALLOC(GAPM_SET_DEV_CONFIG_CMD,
                        TASK_GAPM, TASK_APP, gapm_set_dev_config_cmd);

                app_configuration_func(dest_id, cmd);
	
                ke_msg_send(cmd);

            }
        }
        break;

        // device configuration updated
        case GAPM_SET_DEV_CONFIG:
        {
            if(param->status != GAP_ERR_NO_ERROR)
            {
                ASSERT_ERR(0); // unexpected error
            }
            else
            {
#if (ROLE_MASTER_YCOM)
				xapp_env.state=XAPP_SCAN;	
				app_inq();			//主模式		开启扫描
#else
        app_set_dev_config_complete_func();//从模式
#endif
            }
        }
        break;
#if (ROLE_MASTER_YCOM)
        case GAPM_SCAN_ACTIVE:
				{
					    xapp_env.state = XAPP_IDLE;
					    app_connect(0);//connect to number 1
				}
				break;
				case GAPM_SCAN_PASSIVE:
				{
						//xapp_env.state = XAPP_IDLE;
						//app_connect(0);//connect to number 1
				}
				break;
#endif     					
        // Advertising finished
        case GAPM_ADV_UNDIRECT:
        {
            app_adv_undirect_complete(param->status);
        }
        break;
        
        // Directed advertising finished
        case GAPM_ADV_DIRECT:
        {
            app_adv_direct_complete(param->status);  
        }
        break;

        case GAPM_CANCEL:
        {
            if(param->status != GAP_ERR_NO_ERROR)
            {
                ASSERT_ERR(0); // unexpected error
            }
        }
        
        default:
        {
            ASSERT_ERR(0); // unexpected error
        }
        break;
    }


    return (KE_MSG_CONSUMED);
}


/**
 ****************************************************************************************
 * @brief Handles connection complete event from the GAP. Will enable profile.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

unsigned int start_pair;
int gapc_connection_req_ind_handler(ke_msg_id_t const msgid,
                                           struct gapc_connection_req_ind *param,
                                           ke_task_id_t const dest_id,
                                           ke_task_id_t const src_id)
{

#if (ROLE_MASTER_YCOM)
	 int i;
    start_pair = 1;

    if (xapp_env.state == XAPP_IDLE)
    {
        // We are now connected

        xapp_env.state = APP_CONNECTED;

        // Retrieve the connection index from the GAPC task instance for this connection
        xapp_env.proxr_device.device.conidx = KE_IDX_GET(src_id);

        // Retrieve the connection handle from the parameters
        xapp_env.proxr_device.device.conhdl = param->conhdl;

        // On Reconnection check if device is bonded and send pairing request. Otherwise it is not bonded.
        if (bdaddr_compare(&xapp_env.proxr_device.device.adv_addr, &param->peer_addr))
        {
            if (xapp_env.proxr_device.bonded)
                start_pair = 0;
        }
		    memcpy(xapp_env.proxr_device.device.adv_addr.addr, param->peer_addr.addr, sizeof( struct bd_addr));

        for (i =0 ; i<RSSI_SAMPLES; i++)
            xapp_env.proxr_device.rssi[i] =(char) -127;

        xapp_env.proxr_device.alert = 0;
        xapp_env.proxr_device.rssi_indx = 0;
        xapp_env.proxr_device.avg_rssi =(char)-127;
        xapp_env.proxr_device.txp =(char) -127;
        xapp_env.proxr_device.llv = 0xFF;
        memset(&xapp_env.proxr_device.dis, 0, sizeof(dis_env));
         //GPIO_ConfigurePin( GPIO_PORT_1, GPIO_PIN_1, OUTPUT, PID_GPIO, true);//Alert LED       
        app_connect_confirm(GAP_AUTH_REQ_NO_MITM_NO_BOND);

        app_proxm_enable();
        app_disc_enable();
	
		}
    return 0;
#else
	
    // Connection Index

    if (ke_state_get(dest_id) == APP_CONNECTABLE)
    {
        app_env.conidx = KE_IDX_GET(src_id);
        
        app_connection_func(param);
    }
    else
    {
        // APP_CONNECTABLE state is used to wait the GAP_LE_CREATE_CONN_REQ_CMP_EVT message
        ASSERT_ERR(0);
    }
		return (KE_MSG_CONSUMED);
#endif

}



/**
 ****************************************************************************************
 * @brief Handles GAP controller command complete events.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int gapc_cmp_evt_handler(ke_msg_id_t const msgid,
                                struct gapc_cmp_evt const *param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id)
{
    switch(param->operation)
    {
        // reset completed
        case GAPC_UPDATE_PARAMS:
        {
            if (ke_state_get(dest_id) == APP_PARAM_UPD)
            {
                if ((param->status != CO_ERROR_NO_ERROR))
                {
                    // it's application specific what to do when the Param Upd request is rejected
                    app_update_params_rejected_func(param->status);
                }
                else
                {
                    // Go to Connected State
                    ke_state_set(dest_id, APP_CONNECTED);

                    // if state is APP_CONNECTED then the request was accepted
		            app_update_params_complete_func();
                }
            }
        }
        break;
        default:
        {
            if(param->status != GAP_ERR_NO_ERROR)
            {
                ASSERT_ERR(0); // unexpected error
            }
        }
        break;
    }


    return (KE_MSG_CONSUMED);
}


/**
 ****************************************************************************************
 * @brief Handles disconnection complete event from the GAP.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int gapc_disconnect_ind_handler(ke_msg_id_t const msgid,
                                      struct gapc_disconnect_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    app_disconnect_func(dest_id, param);
    
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the APP_MODULE_INIT_CMP_EVT messgage
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int app_module_init_cmp_evt_handler(ke_msg_id_t const msgid,
                                           const struct app_module_init_cmp_evt *param,
                                           ke_task_id_t const dest_id,
                                           ke_task_id_t const src_id)
{
    if (ke_state_get(dest_id) == APP_DB_INIT)
    {
        if (param->status == CO_ERROR_NO_ERROR)
        {
            // Add next required service in the database
            if (app_db_init())
            {
                // No more service to add in the database, start application
                app_db_init_complete_func();
            }
        }
        else
        {
            // An error has occurred during database creation
            ASSERT_ERR(0);
        }
    }
    else
    {
        // APP_DB_INIT state is used to wait the APP_MODULE_INIT_CMP_EVT message
        ASSERT_ERR(0);
    }

    return (KE_MSG_CONSUMED);
}

#if(ROLE_MASTER_YCOM)
int  proxm_enable_cfm_handler(ke_msg_id_t msgid,
                              struct proxm_enable_cfm *param,
                              ke_task_id_t dest_id,
                              ke_task_id_t src_id)
{
    if (param->status == CO_ERROR_NO_ERROR)
    {
        //initialize proximity reporter's values to non valid
        xapp_env.proxr_device.llv = 0xFF;
        xapp_env.proxr_device.txp = 0xFF;
            

       // ConsoleConnected(1);

        if (start_pair)
        {
            xapp_env.proxr_device.bonded = 0;
            app_security_enable();
        }
        else
           app_start_encryption();
			}

    app_proxm_read_txp();

    return 0;
}
#endif

/*
 * GLOBAL VARIABLES DEFINITION
 ****************************************************************************************
 */


/* Specifies the message handlers that are common to all states. */

const struct ke_state_handler app_default_handler = KE_STATE_HANDLER(app_default_state);

/* Defines the place holder for the states of all the task instances. */
ke_state_t app_state[APP_IDX_MAX] __attribute__((section("retention_mem_area0"), zero_init)); //RETENTION MEMORY 

#endif //(BLE_APP_PRESENT)

/// @} APPTASK
