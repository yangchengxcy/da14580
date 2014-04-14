/**
 ****************************************************************************************
 *
 * @file app_kbd_proj_task.c
 *
 * @brief HID Keyboard handlers.
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

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"               // SW configuration

#if (BLE_APP_PRESENT)

#include "arch.h"                      // Platform Definitions
#include "ke_task.h"
#include "co_error.h"                  // Error Codes Definition

#include "app.h"                       // Application Definition
#include "app_task.h"                  // Application Task API
#include "gapc_task.h"                 // GAP Controller Task API
#include "gapm_task.h"                 // GAP Manager Task API
#include "gap.h"                       // GAP Definitions

#include "app_kbd.h"
#include "hogpd_task.h"                // HID over GATT


/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */


/*
 * Name         : keyboard_create_db_cfm_handler - Writes specific DB values. 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : <various>
 *
 * Description  : Called when the DB has been initialized. It writes specific DB values
 *                and informs the Application that the initialization of this module has
 *                been completed.
 *
 * Returns      : KE_MSG_CONSUMED
 *
 */
int keyboard_create_db_cfm_handler(ke_msg_id_t const msgid,
                                      struct hogpd_create_db_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
	#define REPORT_MAP_LEN (65 + 75)
	// Report Descriptor == Report Map (HID1_11.pdf section E.6)
	static const uint8 report_map[REPORT_MAP_LEN] =
	{
		0x05, 0x01,         // Usage Page (Generic Desktop)
		0x09, 0x06,         // Usage (Keyboard)
		0xA1, 0x01,         // Collection: (Application)
        0x85, 0x01,         //  Report ID (1)
		0x05, 0x07,         //  Usage Page (Key Codes)
		0x19, 0xE0,         //  Usage Minimum (224)
		0x29, 0xE7,         //  Usage Maximum (231)
		0x15, 0x00,         //  Logical Minimum (0)
		0x25, 0x01,         //  Logical Maximum (1)
		0x75, 0x01,         //  Report Size (1)
		0x95, 0x08,         //  Report Count (8)
		0x81, 0x02,         //  Input: (Data, Variable, Absolute) ; Modifier byte
		0x95, 0x01,         //  Report Count (1)
		0x75, 0x08,         //  Report Size (8)
		0x81, 0x01,         //  Input: (Constant) ; Reserved byte
		0x95, 0x05,         //  Report Count (5)
		0x75, 0x01,         //  Report Size (1)
		0x05, 0x08,         //  Usage Page (LEDs)
		0x19, 0x01,         //  Usage Minimum (1)
		0x29, 0x05,         //  Usage Maximum (5)
		0x91, 0x02,         //  Output: (Data, Variable, Absolute) ; LED report
		0x95, 0x01,         //  Report Count (1)
		0x75, 0x03,         //  Report Size (3)
		0x91, 0x01,         //  Output: (Constant); LED report padding
		0x95, 0x06,         //  Report Count (6)
		0x75, 0x08,         //  Report Size (8)
		0x15, 0x00,         //  Log Minimum (0)
		0x25, 0x65,         //  Log Maximum (101)
		0x05, 0x07,         //  Usage Page (Key Codes)
		0x19, 0x00,         //  Usage Minimum (0)
		0x29, 0x65,         //  Usage Maximum (101)
		0x81, 0x00,         //  Input: (Data, Array) ; Key arrays (6 bytes)
		0xC0,               // End Collection
        0x05, 0x0C,         // Usage Page (Consumer Devices)
        0x09, 0x01,         // Usage (Consumer Control)
        0xA1, 0x01,         // Collection (Application)
        0x85, 0x03,         //  Report ID (3)
        0x15, 0x00,         //  Logical Minimum (0)
        0x25, 0x01,         //  Logical Maximum (1)
        0x75, 0x01,         //  Report Size (1)
        0x95, 0x08,         //  Report Count (8)
        0x09, 0xB5,         //  Usage (Scan Next Track)
        0x09, 0xB6,         //  Usage (Scan Previous Track)
        0x09, 0xB7,         //  Usage (Stop)
        0x09, 0xB8,         //  Usage (Eject)
        0x09, 0xCD,         //  Usage (Play/Pause)
        0x09, 0xE2,         //  Usage (Mute)
        0x09, 0xE9,         //  Usage (Volume Increment)
        0x09, 0xEA,         //  Usage (Volume Decrement)
        0x81, 0x02,         //  Input (Data,Var,Abs,NWrp,Lin,Pref,NNul,Bit)
        0x0A, 0x83, 0x01,   //  Usage (AL Consumer Control Configuration)
        0x0A, 0x8A, 0x01,   //  Usage (AL Email Reader)
        0x0A, 0x92, 0x01,   //  Usage (AL Calculator)
        0x0A, 0x94, 0x01,   //  Usage (AL Local Machine Browser)
        0x0A, 0x21, 0x02,   //  Usage (AC Search)
        0x1A, 0x23, 0x02,   //  Usage Minimum (AC Home)
        0x2A, 0x25, 0x02,   //  Usage Maximum (AC Forward)
        0x81, 0x02,         //  Input (Data,Var,Abs,NWrp,Lin,Pref,NNul,Bit)
        0x0A, 0x26, 0x02,   //  Usage (AC Stop)
        0x0A, 0x27, 0x02,   //  Usage (AC Refresh)
        0x0A, 0x2A, 0x02,   //  Usage (AC Bookmarks)
        0x95, 0x03,         //  Report Count (3)
        0x81, 0x02,         //  Input (Data,Var,Abs,NWrp,Lin,Pref,NNul,Bit)
        0x95, 0x05,         //  Report Count (5)
        0x81, 0x01,         //  Input (Cnst,Ary,Abs)
        0xC0                // End Collection
	};

	struct hogpd_set_report_map_req * req = KE_MSG_ALLOC_DYN(HOGPD_SET_REPORT_MAP_REQ, TASK_HOGPD, TASK_APP, hogpd_set_report_map_req, REPORT_MAP_LEN);
	req->report_map_len = REPORT_MAP_LEN;
	req->hids_nb = 0;
	memcpy(&(req->report_map[0]), report_map, REPORT_MAP_LEN);
	ke_msg_send(req);	
	
    // Inform the Application Manager
    struct app_module_init_cmp_evt *cfm = KE_MSG_ALLOC(APP_MODULE_INIT_CMP_EVT,
                                                       TASK_APP, TASK_APP_DIS,
                                                       app_module_init_cmp_evt);

    cfm->status = param->status;

    ke_msg_send(cfm);
    
    return (KE_MSG_CONSUMED);
}


/*
 * Name         : keyboard_ntf_sent_cfm_handler - Ack of the notification we sent. 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : <various>
 *
 * Description  : Called when the HID report has been ACK'ed from the master.
 *
 * Returns      : KE_MSG_CONSUMED
 *
 */
int keyboard_ntf_sent_cfm_handler(ke_msg_id_t const msgid,
                                      struct hogpd_ntf_sent_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    //Clear pending ack's for param->report_nb == 0 (normal key report) and == 2 (ext. key report)
    if (param->hids_nb == 0) 
    {
        if (param->report_nb == 0) 
        {
//            normal_key_report_ack_pending = false;
        } 
        else if (param->report_nb == 2) 
        {
//            extended_key_report_ack_pending = false;
        }
    }
            
    return (KE_MSG_CONSUMED);
}


/*
 * Name         : keyboard_disable_ind_handler - Profile is disabled. 
 *
 * Scope        : PUBLIC
 *
 * Arguments    : <various>
 *
 * Description  : Called when the HID profile is being disabled.
 *
 * Returns      : KE_MSG_CONSUMED
 *
 */
int keyboard_disable_ind_handler(ke_msg_id_t const msgid,
                                    struct hogpd_disable_ind const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    if (ke_state_get(dest_id) == APP_CONNECTED)
    {
        // Go to the idle state
        ke_state_set(dest_id, APP_CONNECTABLE);
    }

    return (KE_MSG_CONSUMED);
}


#endif //(BLE_APP_PRESENT)

/// @} APPTASK
