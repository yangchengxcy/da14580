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

#if (BLE_APP_PRESENT) && (BLE_ACCEL)

#include "app_task.h"                  // Application Task API
#include "app.h"                       // Application Definition
#include "gapc_task.h"                 // GAP Controller Task API
#include "gapm_task.h"                 // GAP Manager Task API
#include "gap.h"                       // GAP Definitions
#include "co_error.h"                  // Error Codes Definition
#include "arch.h"                      // Platform Definitions

#if (BLE_APP_HT)
#include "app_ht.h"                    // Application Heath Thermometer Definition
#endif //(BLE_APP_HT)

#if (BLE_APP_DIS)
#include "app_dis.h"                   // Application Device Information Service Definition
#endif //(BLE_APP_DIS)

#if (BLE_ACCEL)
#include "app_accel.h"                 // Application Accelerometer Definition
#include "accel_task.h"
#include "..\..\..\plf\refip\src\driver\accel\lis3dh_driver.h"
#endif //(BLE_APP_ACCEL)

#if (BLE_APP_NEB)
#include "app_neb.h"                   // Application Nebulizer Definition
#endif //(BLE_APP_NEB)

#include "app_accel_proj.h"

#include "app_sec.h"

#include "ke_env.h"
#include "l2cm.h"

#if (BLE_ACCEL)
uint8_t accel_adv_count __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY
uint16_t accel_adv_interval __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY
int8_t update_conn_params __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY

uint8_t accel_threshold __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY
uint8_t accel_adv_interval1 __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY
uint8_t accel_adv_interval2 __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY
uint8_t accel_adv_interval3 __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY

uint8_t accel_con_interval __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY

uint8_t accel_mode __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY
uint8_t accel_latency __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY
uint8_t accel_window __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY

#endif

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

#if BLE_ACCEL
void app_accel_create_db_send(void)
{
    // Add HTS in the database
    struct accel_create_db_req *req = KE_MSG_ALLOC(ACCEL_CREATE_DB_REQ,
                                                  TASK_ACCEL, TASK_APP,
                                                  accel_create_db_req);

    //req->features = PROXR_IAS_TXPS_SUP;

    ke_msg_send(req);
}

/**
 ****************************************************************************************
 * @brief Handles start indication from the Proximity Reporter profile.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int accel_create_db_cfm_handler(ke_msg_id_t const msgid,
                                      struct accel_create_db_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    // If state is not idle, ignore the message
    if (ke_state_get(dest_id) == APP_DB_INIT)
    {
							
				app_accel_init();
        
        // Inform the Application Manager
        struct app_module_init_cmp_evt *cfm = KE_MSG_ALLOC(APP_MODULE_INIT_CMP_EVT,
                                                           TASK_APP, TASK_APP,
                                                           app_module_init_cmp_evt);

        cfm->status = CO_ERROR_NO_ERROR;    //param->status;

        ke_msg_send(cfm);
			
    }

    return (KE_MSG_CONSUMED);
}


/**
 ****************************************************************************************
 * @brief Handles accelerometer start advertising request from the Wakeup ISR.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int accel_msg_handler(ke_msg_id_t const msgid,
									struct accel_create_db_cfm const *param,
									ke_task_id_t const dest_id,
									ke_task_id_t const src_id)
{
	// If state is not idle, ignore the message
	if (ke_state_get(dest_id) == APP_CONNECTABLE)
		app_adv_start();

	return (KE_MSG_CONSUMED);
}


int accel_start_ind_handler(ke_msg_id_t const msgid,
                                   struct accel_start_ind const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    // Start the accelerometer with the correct parameters
//vm	
    acc_start((uint16_t*)&param->accel_en[0], param->range);

    if(update_conn_params)
        update_conn_params = 2;
    
    // Start the accelerometer timer
    ke_timer_set(APP_ACCEL_TIMER, TASK_APP, 5);


    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles accelerometer stop indication from the Accelerometer profile.
 *        No axis of the accelerometer has been enabled through ATT, so upt it in stand by.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int accel_stop_ind_handler(ke_msg_id_t const msgid,
                                  void const *param,
                                  ke_task_id_t const dest_id,
                                  ke_task_id_t const src_id)
{
    // Stop the accelerometer
//vm	
    acc_stop();

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles accelerometer start indication from the Accelerometer profile.
 *        At least one axis of the accelerometer has been enabled through ATT.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int accel_write_line_ind_handler(ke_msg_id_t const msgid,
                                        struct accel_write_line_ind const *param,
                                        ke_task_id_t const dest_id,
                                        ke_task_id_t const src_id)
{
    #if PLF_DISPLAY
    // Display the line
    lcd_printf(param->line, param->text);
    #endif //PLF_DISPLAY

    return (KE_MSG_CONSUMED);
}

#if 0   //GZ tmp simulated values
static i8_t x_val,y_val,z_val __attribute__((section("retention_mem_area0"),zero_init));  //GZ tmp
#endif

void updateData()
{
#if 1
	  AxesRaw_t data;
		volatile unsigned char response;
		i8_t x_val_new,y_val_new,z_val_new;
    
	LIS3DH_ReadReg(LIS3DH_STATUS_REG, &response);
	if (response & 8) 
	{

		response = LIS3DH_GetAccAxesRaw(&data);
		data.AXIS_X = data.AXIS_X/256;//acc_read_x();
		data.AXIS_Y = data.AXIS_Y/256;//acc_read_y();
		data.AXIS_Z = data.AXIS_Z/256;//acc_read_z();
		
		if((data.AXIS_X)>50)
			data.AXIS_X = 50; 
		if((data.AXIS_X)<-50)
			data.AXIS_X = -50; 

		if((data.AXIS_Y)>50)
			data.AXIS_Y = 50; 
		if((data.AXIS_Y)<-50)
			data.AXIS_Y = -50; 

		if((data.AXIS_Z)>50)
			data.AXIS_Z = 50; 
		if((data.AXIS_Z)<-50)
			data.AXIS_Z = -50; 
		
		x_val_new = data.AXIS_X;
		y_val_new = data.AXIS_Y;
		z_val_new = data.AXIS_Z;

//	if ((x_val_new!=x_val)||(y_val_new!=y_val)||(z_val_new!=z_val))
	{
    //send value change request to ACCEL profile
    struct accel_value_req *req = KE_MSG_ALLOC(ACCEL_VALUE_REQ, TASK_ACCEL,
                                                TASK_APP, accel_value_req);
//vm
		req->accel[0] = x_val_new;//data.AXIS_X/256;//acc_read_x();
    req->accel[1] = y_val_new;// data.AXIS_Y/256;//acc_read_y();
    req->accel[2] = z_val_new;// data.AXIS_Z/256;//acc_read_z();

// 		x_val = x_val_new;
// 		y_val = y_val_new;
// 		z_val = z_val_new;
		
    ke_msg_send(req);
	}
}
#endif
//vm simulated accel reading
#if 0   //GZ tmp	

    //send value change request to ACCEL profile
    struct accel_value_req *req = KE_MSG_ALLOC(ACCEL_VALUE_REQ, TASK_ACCEL,
                                                TASK_APP, accel_value_req);
		req->accel[0] = x_val;//data.AXIS_X;//acc_read_x();
    req->accel[1] = y_val;//data.AXIS_Y;//acc_read_y();
    req->accel[2] = z_val;//data.AXIS_Z;//acc_read_z();
    ke_msg_send(req);

x_val++;
y_val++;
z_val++;	
#endif
}

/**
 ****************************************************************************************
 * @brief Handles accelerometer timer
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int app_accel_timer_handler(ke_msg_id_t const msgid,
                                   void const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
//vm actual accel hardware used	

#if 0    
	if (!update_conn_params) {
		update_conn_params = 1;
//	    acc_init();

		// Not completely verified, but this improves the stability of the connection.
		
		struct gapc_param_update_req_ind * req = KE_MSG_ALLOC(GAPC_PARAM_UPDATE_REQ_IND, TASK_GAPC, TASK_APP, gapc_param_update_req_ind);

		// Fill in the parameter structure
		//req->conhdl = app_env.conhdl;
		req->params.intv_min = 80;
		req->params.intv_max = 80;
		req->params.latency = 0;
		req->params.time_out = 2000;
		ke_msg_send(req);
	}
#endif
	if (update_conn_params) {
        rwip_env.sleep_enable = true;
        if(update_conn_params != 2)
        {
            update_conn_params = 0;            
            ke_timer_clear(APP_ACCEL_TIMER, TASK_APP);
            return (KE_MSG_CONSUMED);
        }
        update_conn_params = 0;            
    }

    if(*((uint16_t *)ke_env.heap[KE_MEM_KE_MSG]+4) < 0x180 || l2cm_get_nb_buffer_available()<4)
    {
        // Restart the 2nd accelerometer timer
        ke_timer_set(APP_ACCEL_TIMER, TASK_APP, 5*4);
        //ke_timer_set(APP_ACCEL_TIMER, TASK_APP, 7*3);
    }
    else
    {
    updateData();
    // Restart the accelerometer timer
    ke_timer_set(APP_ACCEL_TIMER, TASK_APP, 5);
    }
    
    return (KE_MSG_CONSUMED);
}


/**
 ****************************************************************************************
 * @brief Handles accelerometer timer for controlling advertising interval
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance .
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
#if 1
int app_accel_adv_timer_handler(ke_msg_id_t const msgid,
                                   void const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
//GZ
		app_set_adv_data();
		
	
    // Shedule interval is: adv starts with APP_ADV_INT_MIN, then after 500ms goes to 200ms, 
    // then after 3 mins goes to 1sec, then after 3 mins goes to 2 sec
    
    // Check if we need to change Advertising interval
    if( accel_adv_count == 0 )
    {
        // First schedule for advertising interval has been reached. Move to the next schedule
        accel_adv_interval = 0x20;  //0x140; // 200ms  (200ms/0.625)
        // Increment counter in retension memory
        //accel_adv_count += 1; 
//        ke_timer_set(APP_ACCEL_ADV_TIMER, TASK_APP, 18000);
				if(accel_adv_interval1 < 10)
					accel_adv_interval1 = 10;
        ke_timer_set(APP_ACCEL_ADV_TIMER, TASK_APP, accel_adv_interval1*100);
		//stop Accelerometer
		acc_stop();
        rwip_env.sleep_enable = true;                
    }    
    else if( accel_adv_count == 1 )
    {
        accel_adv_interval = 0x640; // 1sec  (1000ms/0.625)
        // Increment counter in retension memory
        //accel_adv_count += 1; 
//        ke_timer_set(APP_ACCEL_ADV_TIMER, TASK_APP, 18000);
        ke_timer_set(APP_ACCEL_ADV_TIMER, TASK_APP, accel_adv_interval2*100);
    }
    else if( accel_adv_count == 2 )
    {
        accel_adv_interval = 0xc80; // 2sec  (2000ms/0.625)
        // Increment counter in retension memory
        //accel_adv_count += 1; 
//        ke_timer_set(APP_ACCEL_ADV_TIMER, TASK_APP, 18000);
        ke_timer_set(APP_ACCEL_ADV_TIMER, TASK_APP, accel_adv_interval3*100);
    }
    
	if( accel_adv_count >= 3 )
    {
        accel_adv_count = 0; 
		accel_adv_interval = 0x20;  //APP_ADV_INT_MIN;
        // Max schedule for advertising interval has been reached. Stop timer
        ke_timer_clear(APP_ACCEL_ADV_TIMER, TASK_APP);
		// Stop Advertising
		app_adv_stop();
		// Update APP State
		ke_state_set(TASK_APP, APP_CONNECTABLE);
		// Activate Accelerometer
//GZ		set_accel_freefall();
        if(accel_mode == 0)
			set_accel_freefall();
        else
            set_accel_click();

		//acc_int_restart();
		acc_enable_wakeup_irq();
        rwip_env.sleep_enable = true;
    }
	else
	{
		accel_adv_count += 1;
		app_adv_start();
	}
    
    return (KE_MSG_CONSUMED);
}
#else
int app_accel_adv_timer_handler(ke_msg_id_t const msgid,
                                   void const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    // Shedule interval is: adv starts with APP_ADV_INT_MIN, then after 500ms goes to 200ms, 
    // then after 3 mins goes to 1sec, then after 3 mins goes to 2 sec
    
    // Check if we need to change Advertising interval
    if( accel_adv_count == 0 )
    {
        // First schedule for advertising interval has been reached. Move to the next schedule
        accel_adv_interval = 0x20;  //0x140; // 200ms  (200ms/0.625)
        // Increment counter in retension memory
        accel_adv_count += 1; 
//        ke_timer_set(APP_ACCEL_ADV_TIMER, TASK_APP, 18000);
        ke_timer_set(APP_ACCEL_ADV_TIMER, TASK_APP, 3000);
		//stop Accelerometer
		acc_stop();
    }    
    else if( accel_adv_count == 1 )
    {
        accel_adv_interval = 0x640; // 1sec  (1000ms/0.625)
        // Increment counter in retension memory
        accel_adv_count += 1; 
//        ke_timer_set(APP_ACCEL_ADV_TIMER, TASK_APP, 18000);
        ke_timer_set(APP_ACCEL_ADV_TIMER, TASK_APP, 3000);
    }
    else if( accel_adv_count == 2 )
    {
        accel_adv_interval = 0xc80; // 2sec  (2000ms/0.625)
        // Increment counter in retension memory
        accel_adv_count += 1; 
//        ke_timer_set(APP_ACCEL_ADV_TIMER, TASK_APP, 18000);
        ke_timer_set(APP_ACCEL_ADV_TIMER, TASK_APP, 3000);
    }
    
	if( accel_adv_count == 3 )
    {
        accel_adv_count = 0; 
		accel_adv_interval = 0x20;  //APP_ADV_INT_MIN;
        // Max schedule for advertising interval has been reached. Stop timer
        ke_timer_clear(APP_ACCEL_ADV_TIMER, TASK_APP);
		// Stop Advertising
		app_adv_stop();
		// Update APP State
		ke_state_set(TASK_APP, APP_IDLE);
		// Activate Accelerometer
		set_accel_freefall();
		acc_enable_wakeup_irq();
    }
	else
		app_adv_start();
    
    return (KE_MSG_CONSUMED);
}
#endif

bool app_db_init_func(void)
{
    
    // Indicate if more services need to be added in the database
    bool end_db_create = false;
    
    // Check if another should be added in the database
    if (app_env.next_prf_init < APP_PRF_LIST_STOP)
    {
        switch (app_env.next_prf_init)
        {
            case (APP_ACCEL_TASK):
            {
                // Add Service in the DB
                app_accel_create_db_send();
            } break;
            default:
            {
                ASSERT_ERR(0);
            } break;

        }

        // Select following service to add
        app_env.next_prf_init++;
    }
    else
    {
        end_db_create = true;
    }

    return end_db_create;
}

void app_init_func(void)
{
    
	app_task_custom_init();	
    
	//app_accel_init();
    
    app_set_adv_data();
    
#if 0    
#if SLEEP_ENABLED
    #if DEEP_SLEEP_ENABLED
        app_set_deep_sleep();
    #else
        app_set_extended_sleep(false);
    #endif
#else
    app_disable_sleep();
#endif // SLEEP_ENABLED    
#endif
}

void app_connection_func(struct gapc_connection_req_ind const *param)
{
    
    if (app_env.conidx != GAP_INVALID_CONIDX)
    {
        accel_adv_interval = 0x20;  //APP_ADV_INT_MIN;
        accel_adv_count = 0;
        ke_timer_clear(APP_ACCEL_ADV_TIMER, TASK_APP);

        /*--------------------------------------------------------------
        * ENABLE REQUIRED PROFILES
        *-------------------------------------------------------------*/

        app_accel_enable();
        
        ke_state_set(TASK_APP, APP_CONNECTED);
            
        // Retrieve the connection info from the parameters
        app_env.conhdl = param->conhdl;

        app_env.peer_addr_type = param->peer_addr_type;
        memcpy(app_env.peer_addr.addr, param->peer_addr.addr, BD_ADDR_LEN);

        # if (BLE_APP_SEC)
                
        // send connection confirmation
        app_connect_confirm(app_sec_env.auth);

        if( (gapc_get_role(app_env.conidx) == ROLE_MASTER) && (app_env.sec_en) && (!app_sec_env.auth) ) // Optional security step
        {
            // Start security
            app_security_start();
        }
        else // Param update step
        # else // (BLE_APP_SEC)
        // send connection confirmation
        app_connect_confirm(GAP_AUTH_REQ_NO_MITM_NO_BOND);            
        # endif // (BLE_APP_SEC)

        {
            // Start param update
            app_param_update_start();
        }
    }
    else
    {
        // No connection has been establish, restart advertising
        app_adv_start();
    }
    
    return;

}

void app_disconnect_func(ke_task_id_t task_id, struct gapc_disconnect_ind const *param)
{
    
    #if BLE_ACCEL
    //Stop the accelerometer
    acc_stop();
    // Disable accelerometer profile
    ke_msg_send_basic(ACCEL_DISABLE_REQ, TASK_ACCEL, TASK_APP);
    // Stop the accelerometer timer
    update_conn_params = 0;
    ke_timer_clear(APP_ACCEL_TIMER, TASK_APP);
    // Set accel advertising interval to initial value
    accel_adv_interval = 0x20;  //APP_ADV_INT_MIN;
    accel_adv_count = 0;
    rwip_env.sleep_enable = true;
    #endif //(BLE_ACCEL)

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

void app_sec_init_func(void)
{
    
#if (BLE_APP_SEC)
	app_sec_env.auth = (GAP_AUTH_REQ_MITM_BOND);
#endif
    
}

#endif	//BLE_APP_ACCEL

#endif //(BLE_APP_PRESENT)

/// @} APPTASK
