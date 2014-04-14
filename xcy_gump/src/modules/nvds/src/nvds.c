/**
 ****************************************************************************************
 *
 * @file nvds.c
 *
 * @brief Non Volatile Data Storage (NVDS) driver
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup NVDS
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <string.h>      // string definitions
#include <stddef.h>      // standard definitions
#include "nvds.h"        // nvds definitions
#include "arch.h"        // main
#include "co_math.h"     // math operations
#include "co_bt.h"

#include "rwip_config.h"

extern struct nvds_data_struct *nvds_data_ptr __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY


const struct nvds_data_struct nvds_data_storage __attribute__((section("nvds_data_storage_area"))) =	
{
    .NVDS_VALIDATION_FLAG                   = 0x1ffff,	
    .NVDS_TAG_UART_BAUDRATE                 = 115200,
    .NVDS_TAG_DIAG_SW                       = 0,
    .NVDS_TAG_DIAG_BLE_HW                   = 0,
    .NVDS_TAG_NEB_ID                        = 0,
    .NVDS_TAG_LPCLK_DRIFT                   = DRIFT_BLE_DFT,
    .NVDS_TAG_SLEEP_ENABLE                  = 1,
    .NVDS_TAG_EXT_WAKEUP_ENABLE             = 0,
    .NVDS_TAG_SECURITY_ENABLE               = 1,
    .NVDS_TAG_APP_BLE_ADV_DATA              = "\x02\x01\x06\x03\x03\xa0\xff",
    .NVDS_TAG_APP_BLE_SCAN_RESP_DATA        = "\x09\xFF\x00\x60\x52\x57\x2D\x42\x4C\x45",
    .NVDS_TAG_DEVICE_NAME                   = "MISSMEC",
    .NVDS_TAG_BD_ADDRESS                    = {0x01, 0x23, 0x45, 0x55, 0x89, 0x11},
    .ADV_DATA_TAG_LEN                       = 7,
    .SCAN_RESP_DATA_TAG_LEN                 = 10,	
    .DEVICE_NAME_TAG_LEN                    = 8,
    /// Default Channel Assessment Timer duration (20s - Multiple of 10ms)
    .NVDS_TAG_BLE_CA_TIMER_DUR              = 2000,
    /// Default Channel Reassessment Timer duration (Multiple of Channel Assessment Timer duration)
    .NVDS_TAG_BLE_CRA_TIMER_DUR             = 6,
    /// Default Minimal RSSI Threshold - -48dBm
    .NVDS_TAG_BLE_CA_MIN_RSSI               = 0x90,
    /// Default number of packets to receive for statistics
    .NVDS_TAG_BLE_CA_NB_PKT                 = 100,
    /// Default number of bad packets needed to remove a channel
    .NVDS_TAG_BLE_CA_NB_BAD_PKT             = 10,
};

#define BDADDR_FROM_OTP 0x7fd4     //OTP address offset with BDADDR

/// NULL BD address
extern const struct bd_addr co_null_bdaddr;
/// Device BD address
struct bd_addr dev_bdaddr __attribute__((section("retention_mem_area0"), zero_init));

uint8_t custom_nvds_get_func(uint8_t tag, nvds_tag_len_t * lengthPtr, uint8_t *buf)
{

uint8_t status = NVDS_FAIL;	
	
	switch (tag)
	{
    case NVDS_TAG_BD_ADDRESS:
#ifdef BDADDR_FROM_OTP   //check if dev_bdaddr is not zero
        {
        if(memcmp(&dev_bdaddr, &co_null_bdaddr, NVDS_LEN_BD_ADDRESS))
        {
			memcpy(buf,&dev_bdaddr,NVDS_LEN_BD_ADDRESS);
            *lengthPtr = NVDS_LEN_BD_ADDRESS;
            return NVDS_OK;
        }
        }
#endif    
			if (nvds_data_ptr->NVDS_VALIDATION_FLAG & BD_ADDRESS_VALID)
			{
        if (*lengthPtr < NVDS_LEN_BD_ADDRESS)
        {		
						*lengthPtr = 0;
            status = NVDS_LENGTH_OUT_OF_RANGE;
        }
        else 
        {
					memcpy(buf,nvds_data_ptr->NVDS_TAG_BD_ADDRESS,NVDS_LEN_BD_ADDRESS);
          *lengthPtr = NVDS_LEN_BD_ADDRESS;
					status = NVDS_OK;
        }
			}
		break;
    case NVDS_TAG_DEVICE_NAME:
			if (nvds_data_ptr->NVDS_VALIDATION_FLAG & DEVICE_NAME_VALID)
			{
        if (*lengthPtr < NVDS_LEN_DEVICE_NAME)
        {		
						*lengthPtr = 0;
            status = NVDS_LENGTH_OUT_OF_RANGE;
        }
        else 
        {
					memcpy(buf,nvds_data_ptr->NVDS_TAG_DEVICE_NAME,nvds_data_ptr->DEVICE_NAME_TAG_LEN);
          *lengthPtr = nvds_data_ptr->DEVICE_NAME_TAG_LEN;
					status = NVDS_OK;
        }
			}
		break;
		case NVDS_TAG_LPCLK_DRIFT:
			if (nvds_data_ptr->NVDS_VALIDATION_FLAG & LPCLK_DRIFT_VALID)
			{
        if (*lengthPtr < NVDS_LEN_LPCLK_DRIFT)
        {		
						*lengthPtr = 0;
            status = NVDS_LENGTH_OUT_OF_RANGE;
        }
        else 
        {
					memcpy(buf,&nvds_data_ptr->NVDS_TAG_LPCLK_DRIFT,NVDS_LEN_LPCLK_DRIFT);
          *lengthPtr = NVDS_LEN_LPCLK_DRIFT;
					status = NVDS_OK;
        }
			}
		break;
		case   NVDS_TAG_APP_BLE_ADV_DATA: 
			if (nvds_data_ptr->NVDS_VALIDATION_FLAG & APP_BLE_ADV_DATA_VALID)
			{
        if (*lengthPtr < NVDS_LEN_APP_BLE_ADV_DATA)
        {		
						*lengthPtr = 0;
            status = NVDS_LENGTH_OUT_OF_RANGE;
        }
        else 
        {
					memcpy(buf,nvds_data_ptr->NVDS_TAG_APP_BLE_ADV_DATA,nvds_data_ptr->ADV_DATA_TAG_LEN);
          *lengthPtr = nvds_data_ptr->ADV_DATA_TAG_LEN;
					status = NVDS_OK;
        }

			}
		break;
    case NVDS_TAG_APP_BLE_SCAN_RESP_DATA:
			if (nvds_data_ptr->NVDS_VALIDATION_FLAG & APP_BLE_SCAN_RESP_DATA_VALID)
			{
				if (*lengthPtr < NVDS_LEN_APP_BLE_SCAN_RESP_DATA)
        {		
						*lengthPtr = 0;
            status = NVDS_LENGTH_OUT_OF_RANGE;
        }
        else 
        {
					memcpy(buf,nvds_data_ptr->NVDS_TAG_BD_ADDRESS,nvds_data_ptr->SCAN_RESP_DATA_TAG_LEN);
          *lengthPtr = nvds_data_ptr->SCAN_RESP_DATA_TAG_LEN;
					status = NVDS_OK;
        }

			}
		break;
		case    NVDS_TAG_UART_BAUDRATE:
			if (nvds_data_ptr->NVDS_VALIDATION_FLAG & UART_BAUDRATE_VALID)
			{
				if (*lengthPtr < NVDS_LEN_UART_BAUDRATE)
        {		
						*lengthPtr = 0;
            status = NVDS_LENGTH_OUT_OF_RANGE;
        }
        else 
        {
					memcpy(buf,&nvds_data_ptr->NVDS_TAG_UART_BAUDRATE,NVDS_LEN_UART_BAUDRATE);
          *lengthPtr = NVDS_LEN_UART_BAUDRATE;
					status = NVDS_OK;
        }
			}
		break;
		case   NVDS_TAG_SLEEP_ENABLE:
			if (nvds_data_ptr->NVDS_VALIDATION_FLAG & SLEEP_ENABLE_VALID)
			{
				if (*lengthPtr < NVDS_LEN_SLEEP_ENABLE)
        {		
						*lengthPtr = 0;
            status = NVDS_LENGTH_OUT_OF_RANGE;
        }
        else 
        {
					memcpy(buf,&nvds_data_ptr->NVDS_TAG_SLEEP_ENABLE,NVDS_LEN_SLEEP_ENABLE);
          *lengthPtr = NVDS_LEN_LPCLK_DRIFT;
					status = NVDS_OK;
        }

			}
		break;
		case    NVDS_TAG_EXT_WAKEUP_ENABLE:
			if (nvds_data_ptr->NVDS_VALIDATION_FLAG & EXT_WAKEUP_ENABLE_VALID)
			{
				if (*lengthPtr < NVDS_LEN_EXT_WAKEUP_ENABLE)
        {		
						*lengthPtr = 0;
            status = NVDS_LENGTH_OUT_OF_RANGE;
        }
        else 
        {
					memcpy(buf,&nvds_data_ptr->NVDS_TAG_EXT_WAKEUP_ENABLE,NVDS_LEN_EXT_WAKEUP_ENABLE);
          *lengthPtr = NVDS_LEN_LPCLK_DRIFT;
					status = NVDS_OK;
        }
			}
		break;
		case    NVDS_TAG_DIAG_BLE_HW:
			if (nvds_data_ptr->NVDS_VALIDATION_FLAG & DIAG_BLE_HW_VALID)
			{
				if (*lengthPtr < NVDS_LEN_DIAG_BLE_HW)
        {		
						*lengthPtr = 0;
            status = NVDS_LENGTH_OUT_OF_RANGE;
        }
        else 
        {
					memcpy(buf,&nvds_data_ptr->NVDS_TAG_DIAG_BLE_HW,NVDS_LEN_DIAG_BLE_HW);
          *lengthPtr = NVDS_LEN_DIAG_BLE_HW;
					status = NVDS_OK;
        }
			}
		break;
		case    NVDS_TAG_DIAG_SW:
			if (nvds_data_ptr->NVDS_VALIDATION_FLAG & DIAG_SW_VALID)
			{
				if (*lengthPtr < NVDS_LEN_DIAG_SW)
        {		
						*lengthPtr = 0;
            status = NVDS_LENGTH_OUT_OF_RANGE;
        }
        else 
        {
					memcpy(buf,&nvds_data_ptr->NVDS_TAG_DIAG_SW,NVDS_LEN_DIAG_SW);
          *lengthPtr = NVDS_LEN_DIAG_SW;
					status = NVDS_OK;
        }
			}
		break;
		case    NVDS_TAG_SECURITY_ENABLE:
			if (nvds_data_ptr->NVDS_VALIDATION_FLAG & SECURITY_ENABLE_VALID)
			{
				if (*lengthPtr < NVDS_LEN_SECURITY_ENABLE)
        {		
						*lengthPtr = 0;
            status = NVDS_LENGTH_OUT_OF_RANGE;
        }
        else 
        {
					memcpy(buf,&nvds_data_ptr->NVDS_TAG_SECURITY_ENABLE,NVDS_LEN_SECURITY_ENABLE);
          *lengthPtr = NVDS_LEN_SECURITY_ENABLE;
					status = NVDS_OK;
        }
			}
		break;
		case    NVDS_TAG_NEB_ID:
			if (nvds_data_ptr->NVDS_VALIDATION_FLAG & NEB_ID_VALID)
			{
				if (*lengthPtr < NVDS_LEN_NEB_ID)
        {		
						*lengthPtr = 0;
            status = NVDS_LENGTH_OUT_OF_RANGE;
        }
        else 
        {
					memcpy(buf,&nvds_data_ptr->NVDS_TAG_NEB_ID,NVDS_LEN_NEB_ID);
          *lengthPtr = NVDS_LEN_NEB_ID;
					status = NVDS_OK;
        }
			}
		break;
#if 1			
		/// BLE Channel Assessment tags
    case NVDS_TAG_BLE_CA_TIMER_DUR:
			if (nvds_data_ptr->NVDS_VALIDATION_FLAG & NVDS_BLE_CA_TIMER_DUR_VALID)
			{
				if (*lengthPtr < NVDS_LEN_BLE_CA_TIMER_DUR)
        {		
						*lengthPtr = 0;
            status = NVDS_LENGTH_OUT_OF_RANGE;
        }
        else 
        {
					memcpy(buf,&nvds_data_ptr->NVDS_TAG_BLE_CA_TIMER_DUR,NVDS_LEN_BLE_CA_TIMER_DUR);
          *lengthPtr = NVDS_LEN_BLE_CA_TIMER_DUR;
					status = NVDS_OK;
        }
			}
		break;

		case NVDS_TAG_BLE_CRA_TIMER_DUR:
			if (nvds_data_ptr->NVDS_VALIDATION_FLAG & NVDS_BLE_CRA_TIMER_DUR_VALID)
			{
				if (*lengthPtr < NVDS_LEN_BLE_CRA_TIMER_DUR)
        {		
						*lengthPtr = 0;
            status = NVDS_LENGTH_OUT_OF_RANGE;
        }
        else 
        {
					memcpy(buf,&nvds_data_ptr->NVDS_TAG_BLE_CRA_TIMER_DUR,NVDS_LEN_BLE_CRA_TIMER_DUR);
          *lengthPtr = NVDS_LEN_BLE_CRA_TIMER_DUR;
					status = NVDS_OK;
        }
			}
		break;

		case NVDS_TAG_BLE_CA_MIN_RSSI:
			if (nvds_data_ptr->NVDS_VALIDATION_FLAG & NVDS_BLE_CA_MIN_RSSI_VALID)
			{
				if (*lengthPtr < NVDS_LEN_BLE_CA_MIN_RSSI)
        {		
						*lengthPtr = 0;
            status = NVDS_LENGTH_OUT_OF_RANGE;
        }
        else 
        {
					memcpy(buf,&nvds_data_ptr->NVDS_TAG_BLE_CA_MIN_RSSI,NVDS_LEN_BLE_CA_MIN_RSSI);
          *lengthPtr = NVDS_LEN_BLE_CA_MIN_RSSI;
					status = NVDS_OK;
        }
			}
		break;
		case NVDS_TAG_BLE_CA_NB_PKT:
			if (nvds_data_ptr->NVDS_VALIDATION_FLAG & NVDS_BLE_CA_NB_PKT_VALID)
			{
				if (*lengthPtr < NVDS_LEN_BLE_CA_NB_PKT)
        {		
						*lengthPtr = 0;
            status = NVDS_LENGTH_OUT_OF_RANGE;
        }
        else 
        {
					memcpy(buf,&nvds_data_ptr->NVDS_TAG_BLE_CA_NB_PKT,NVDS_LEN_BLE_CA_NB_PKT);
          *lengthPtr = NVDS_LEN_BLE_CA_NB_PKT;
					status = NVDS_OK;
        }
			}
		break;

		case NVDS_TAG_BLE_CA_NB_BAD_PKT:
			if (nvds_data_ptr->NVDS_VALIDATION_FLAG & NVDS_BLE_CA_NB_BAD_PKT_VALID)
			{
				if (*lengthPtr < NVDS_LEN_BLE_CA_NB_BAD_PKT)
        {		
						*lengthPtr = 0;
            status = NVDS_LENGTH_OUT_OF_RANGE;
        }
        else 
        {
					memcpy(buf,&nvds_data_ptr->NVDS_TAG_BLE_CA_NB_BAD_PKT,NVDS_LEN_BLE_CA_NB_BAD_PKT);
          *lengthPtr = NVDS_LEN_BLE_CA_NB_BAD_PKT;
					status = NVDS_OK;
        }
			}
		break;
#endif

	}

	return status;
}

void nvds_read_bdaddr_from_otp()
{
#ifdef BDADDR_FROM_OTP

#if DEVELOPMENT__NO_OTP    
    int cnt=100000;
#define XPMC_MODE_MREAD   0x1
    uint8_t *otp_bdaddr = (uint8_t *)0x40000 + BDADDR_FROM_OTP;   //where in OTP header is BDADDR
    
    SetBits16(CLK_AMBA_REG, OTP_ENABLE, 1);		// enable OTP clock	
    while ((GetWord16(ANA_STATUS_REG) & LDO_OTP_OK) != LDO_OTP_OK && cnt--)
        /* Just wait */;
        
    // set OTP in read mode 
    SetWord32 (OTPC_MODE_REG,XPMC_MODE_MREAD);
#else
    uint8_t *otp_bdaddr = (uint8_t *)0x20000000 + BDADDR_FROM_OTP;   //where in OTP header is BDADDR
#endif    
    
    memcpy(&dev_bdaddr, otp_bdaddr, sizeof(dev_bdaddr));
    SetBits16(CLK_AMBA_REG, OTP_ENABLE, 0);     //disable OTP clock    
#endif    
}
/// @} NVDS

