/**
 ****************************************************************************************
 *
 * @file rf_580.c
 *
 * @brief DA14580 RF settings.
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
 
#include "rwip_config.h"        // RW SW configuration

#ifdef RADIO_580

#define EM_BLE_FREQ_TABLE_LEN  40



#include <string.h>             // for memcpy
#include "co_utils.h"           // common utility definition
#include "co_math.h"            // common math functions
#include "co_endian.h"          // endian definitions
#include "rf.h"                 // RF interface
#include "plf.h"                // Platform functions

#include "rwip.h"               // for RF API structure definition

#if (BLE_EMB_PRESENT)
#include "reg_blecore.h"        // ble core registers
#include "reg_ble_em_cs.h"      // control structure definitions
#endif //BLE_EMB_PRESENT

#if (BT_EMB_PRESENT)
#include "lc_epc.h"             // Enhanced Power Control definitions
#include "reg_btcore.h"         // bt core registers
#include "reg_bt_em_cs.h"       // control structure definitions
#endif //BT_EMB_PRESENT

#include "rf_580.h"

#include "pll_vcocal_lut.h"
#include "arch.h"
#include "gpio.h"

#define RXPWRUP_VAL 0x54
#define TXPWRUP_VAL 0x4C

extern uint16_t last_temp_count;

uint32_t rf_rpl_reg_rd (uint16_t address)
{
    return 0;
}

void rf_rpl_reg_wr (uint16_t address, uint32_t data)
{

}

/*****************************************************************************************
 * @brief Static function - Ripple TX CNTL1 by radio
 ****************************************************************************************
 */
void rf_rpl_set_txcntl1(void)
{

}

/*****************************************************************************************
 * @brief Static function - Ripple RF Power up sequence (all on)
 ****************************************************************************************
 */
void rf_rpl_pw_up(void)
{
 
}

/*****************************************************************************************
 * @brief Static function - Init modem for Ripple.
 ****************************************************************************************
 */
void rf_rpl_mdm_init(void)
{


}

/**
 ****************************************************************************************
 * @brief Static function - Measure Ripple VCO Frequency
 *
 * @param[in] vco_fc_value  VCO
 * @param[in] vco_freq      Pointer to frequency value.
 ****************************************************************************************
 */
void rf_rpl_measure_vco_freq(uint8_t vco_fc_value, int * vco_freq)
{
 
}

/**
 ****************************************************************************************
 * @brief Static function - for VCO Calibration
 *
 * @param[in] channel   channel
 * @param[in] vco_val   vco value
 ****************************************************************************************
 */
void rf_rpl_calib_vco_fq(uint8_t channel, uint8_t *vco_val)
{
 
}

/**
 ****************************************************************************************
 * @brief Static function for calibrating ICP value.
 *
 * @param[in] icp Pointer to value to calibrate.
 ****************************************************************************************
 */
void rf_rpl_calib_icp(uint8_t channel,uint8_t * icp)
{
  
}

/**
 ****************************************************************************************
 * @brief Static function for status lock.
 *
 * @param[in] chnl  channel
 * @param[in] icp   icp
 * @param[in] vco   vco value
 * @param[in] lock  pointer to lock
 ****************************************************************************************
 */
void rf_rpl_status_lock(uint8_t chnl, uint8_t icp, uint8_t vco, uint8_t *lock)
{
 
}


/***************************************************************************************
 * @brief Static function for radio PLL auto-calibration.
 ****************************************************************************************
 */
void rf_rpl_pll_autocalib(void)
{

}

/***************************************************************************************
 * @brief Static Ripple radio Calibration function.
 ***************************************************************************************
 */
void rf_rpl_calib(void)
{
  
}

/***************************************************************************************
 * @brief Static function - Sequencer settings Initialization for Ripple radio
 ****************************************************************************************
*/
void rf_rpl_sequencers_init(void)
{
}

/***************************************************************************************
 * @brief Static function - Tx Gain tables settings
 ****************************************************************************************
 */
void rf_rpl_txgain_set(void)
{
 
}

/***************************************************************************************
 * @brief Static function - Initialization sequence for Ripple radio
 ****************************************************************************************
 */
void rf_rpl_init_seq(void)
{
 
}

/**
 *****************************************************************************************
 * @brief Init RF sequence after reset.
 *****************************************************************************************
 */
void rf_reset(void)
{

}

/**
 *****************************************************************************************
 * @brief Enable/disable force AGC mechanism
 *
 * @param[in]  True: Enable / False: disable
 *****************************************************************************************
 */
 void rf_force_agc_enable(bool en)
{
 
}

/**
 *****************************************************************************************
 * @brief Get TX power in dBm from the index in the control structure
 *
 * @param[in] txpwr_idx  Index of the TX power in the control structure
 * @param[in] modulation Modulation: 1 or 2 or 3 MBPS
 *
 * @return The TX power in dBm
 *
 *****************************************************************************************
 */
uint8_t rf_txpwr_dbm_get(uint8_t txpwr_idx, uint8_t modulation)
{
    return 0;
}

static void rf_sleep(void)
{
    ble_deepslcntl_set(ble_deepslcntl_get() |
                      BLE_DEEP_SLEEP_ON_BIT |    // RW BLE Core sleep
                      BLE_RADIO_SLEEP_EN_BIT |   // Radio sleep
                      BLE_OSC_SLEEP_EN_BIT);     // Oscillator sleep
   // ble_deepslcntl_set(ble_deepslcntl_get() | BLE_DEEP_SLEEP_ON_BIT );     
   
}



static void RADIOCNTL_Handler(void)
{

}


/*
 * RADIO FUNCTION INTERFACE
 ****************************************************************************************
 */

//#pragma pack(2)
//const uint8_t dummy_pad[2] __attribute__((section("radio"))) = {0xAA,0x55}; 
//volatile uint8_t dummy_pad_var;

void rf_regs(void)
{
    // -- Preferred Settings File for DCTMON
    // -- Device          : DA14580a0m2_x_nl1_0
    // -- Package         : All packages, no depenency on package.
    // -- Last change date: September 24, 2013 - 10:26:10
    // -- Last change item: Register: RF_SYNTH_CTRL2_REG Field: BT_SEL Value: 0x1
    // -- File date       : September 25, 2013 - 14:38:06
    //
    // -- Preferred Settings File for DCTMON
    // -- Device          : DA14580a0m2_x_nl1_0
    // -- Package         : All packages, no depenency on package.
    // -- Last change date: September 27, 2013 - 11:39:28
    // -- Last change item: Register: RF_AFC_CTRL_REG Field: POLE1 Value: 0x1
    // -- File date       : September 28, 2013 - 20:51:27
    //
    SetWord32(BLE_RADIOPWRUPDN_REG, 0x754054C);
    SetWord16( RF_LF_CTRL_REG,0x4C);
    SetWord16( RF_CP_CTRL_REG,0x7F7F);
    SetWord16( RF_REF_OSC_REG,0x29AC);
    SetWord16( RF_ENABLE_CONFIG1_REG,0x909);
    SetWord16( RF_ENABLE_CONFIG2_REG,0x922);
    SetWord16( RF_ENABLE_CONFIG6_REG,0x22);
    SetWord16( RF_ENABLE_CONFIG9_REG,0x204);
    SetWord16( RF_ENABLE_CONFIG10_REG,0x422);
    SetWord16( RF_ENABLE_CONFIG13_REG,0xD030);
    SetWord16( RF_ENABLE_CONFIG14_REG,0x433);
    SetWord16( RF_ENABLE_CONFIG19_REG,0x11EE);
    SetWord16( RF_CNTRL_TIMER_3_REG,0x410);
    SetWord16( RF_CNTRL_TIMER_4_REG,0x22E);
    SetWord16( RF_CNTRL_TIMER_8_REG,0x23E);
    SetWord16( RF_CNTRL_TIMER_9_REG,0x22E);
    SetWord16( RF_CNTRL_TIMER_10_REG,0x22E);
    SetWord16( RF_CNTRL_TIMER_11_REG,0x230);
    SetWord16( RF_CNTRL_TIMER_12_REG,0x239); //ES4 0x23A);
    SetWord16( RF_CNTRL_TIMER_13_REG,0x145);
    SetWord16( RF_CNTRL_TIMER_14_REG,0x2044);
    SetWord16( BIAS_CTRL1_REG,0x6888);
    SetWord16( RF_DEM_CTRL_REG,0x59);
    SetWord16( RF_AGC_CTRL1_REG,0x950D);
    SetWord16( RF_AGC_CTRL2_REG,0x43);
    SetWord16( RF_AFC_CTRL_REG,0xD5);
    SetWord16( RF_DC_OFFSET_CTRL2_REG,0x1D2);
    SetWord16( RF_DC_OFFSET_CTRL3_REG,0xDCE4);
    SetWord16( RF_DC_OFFSET_CTRL4_REG,0x9210);
    SetWord16( RF_PA_CTRL_REG,0x7853);
    SetWord16( RF_SYNTH_CTRL2_REG,0x108B);
#if (LUT_PATCH_ENABLED)
    SetBits16( RF_VCOCAL_CTRL_REG, VCO_FREQTRIM_SEL, 0x1);
#else
    SetWord16( RF_VCOCAL_CTRL_REG, 0x63);
#endif
#if (MGCKMODA_PATCH_ENABLED)    
  SetWord16( RF_MGAIN_CTRL_REG,     0x9503); // GAUSS_GAIN_SEL=0x1 and KMOD_ALPHA=3 to have some range above/below the final SW based KMOD_ALPHA value 
#else
  SetWord16( RF_MGAIN_CTRL_REG,     0xF403);
#endif
    SetWord16( RF_VCO_CALCAP_BIT14_REG,0xD59D);
    SetWord16( RF_LF_RES_CTRL_REG,0x7F7F);
    SetWord16( RF_MGAIN_CTRL2_REG,0x8);	
}

//void rf_init(struct rw_rf_api *api) __attribute__ ((section("radio")));
void rf_init(struct rwip_rf_api *api)
{
	typedef void (*my_function)(struct rwip_rf_api *);
	my_function PtrFunc;   

  	PtrFunc = (my_function)(jump_table_struct[rf_init_pos]);    
	PtrFunc(api);
}

void rf_init_func(struct rwip_rf_api *api)
{

    uint32 tmp32 = 0;
    uint8 idx = 0;
    uint8 temp_freq_tbl[EM_BLE_FREQ_TABLE_LEN];

   // Initialize the RF driver API structure
    api->reg_rd = rf_rpl_reg_rd;
    api->reg_wr = rf_rpl_reg_wr;
    api->txpwr_dbm_get = rf_txpwr_dbm_get;

    //api->txpwr_max = RPL_POWER_MAX;
    api->sleep = rf_sleep;
    api->reset = rf_reset;
    #ifdef CFG_BLE
    api->isr = RADIOCNTL_Handler;
    api->force_agc_enable = rf_force_agc_enable;
    #endif //CFG_BLE

    #ifdef CFG_BT
    api->txpwr_inc = rf_txpwr_inc;
    api->txpwr_dec = rf_txpwr_dec;
    api->txpwr_epc_req = rf_txpwr_epc_req;
    api->txpwr_cs_get = rf_txpwr_cs_get;
    api->rssi_convert = rf_rssi_convert;
    api->rssi_high_thr = (uint8_t)RPL_RSSI_20dB_THRHLD;
    api->rssi_low_thr = (uint8_t)RPL_RSSI_60dB_THRHLD;
    api->rssi_interf_thr = (uint8_t)RPL_RSSI_70dB_THRHLD;
    #ifdef CFG_BTCORE_30
    api->wakeup_delay = RPL_WK_UP_DELAY;
    #endif //CFG_BTCORE_30
    api->skew = RPL_RADIO_SKEW;
    #endif //CFG_BT

	// CLK_FREQ_TRIM_REG initialization was moved to main_func() in arch_main.c
	// The initialization of this register is done by the Boot ROM code if a valid
	// value has been written to the corresponding position in the OTP header by
	// the customer.
	// main_func() will write this register with a default value and the customer
	// must remove this code when he has written the OTP header.
	


    SetBits32(&tmp32, RTRIP_DELAY, 7);    
    SetBits32(&tmp32, TXPWRDN,     0x5);  
	SetBits32(&tmp32, RXPWRUP,     RXPWRUP_VAL);  
    SetBits32(&tmp32, TXPWRUP,     TXPWRUP_VAL);
    SetWord32(BLE_RADIOPWRUPDN_REG,tmp32);   

    SetBits32(BLE_RADIOCNTL0_REG, DPCORR_EN, 0);  //THIS MAY NOT BE '1', THEN WE MISS 12 BITS IN THE SYNCWORD DURING A RX BURST
    SetBits32(BLE_RADIOCNTL1_REG, XRFSEL, 3);
    SetBits32(BLE_CNTL2_REG, SW_RPL_SPI ,0);
    SetBits32(BLE_CNTL2_REG, BB_ONLY,0);    	

    while(idx < EM_BLE_FREQ_TABLE_LEN)    
    {
		temp_freq_tbl[idx] = idx ;
		idx++;
    }

    em_ble_burst_wr(&temp_freq_tbl[0], EM_BLE_FT_OFFSET, EM_BLE_FREQ_TABLE_LEN);				
	rf_regs();
    
    last_temp_count = get_rc16m_count();
    
#if LUT_PATCH_ENABLED
    pll_vcocal_LUT_InitUpdate(LUT_INIT);    
#endif

	rf_calibration();		
}   

void rf_reinit(void)
{
    uint32 tmp32 = 0;

    SetBits32(&tmp32, RTRIP_DELAY, 7);    
    SetBits32(&tmp32, TXPWRDN, 0x5);  
    SetBits32(&tmp32, RXPWRUP, RXPWRUP_VAL);  
    SetBits32(&tmp32, TXPWRUP, TXPWRUP_VAL);  
    SetWord32(BLE_RADIOPWRUPDN_REG, tmp32);    

    SetBits32(BLE_RADIOCNTL1_REG, XRFSEL, 3);
    SetBits32(BLE_RWBTLECNTL_REG, SYNCERR, 0); //this must be always '0'

    SetBits16(CLK_RADIO_REG, RFCU_DIV, 1); //RFCU clock must always be 8MHz!
    SetBits16(CLK_RADIO_REG, RFCU_ENABLE, 1);
    
#ifdef UNCALIBRATED_AT_FAB
//    SetWord32(BANDGAP_REG, 0x3C00);
    SetBits16(BANDGAP_REG, BGR_TRIM, 0x0);  // trim RET Bandgap

#if ES4_CODE
    SetBits16(BANDGAP_REG, LDO_RET_TRIM, 0xF);  // trim RET LDO
#else
    SetBits16(BANDGAP_REG, LDO_RET_TRIM, 0x7);  // trim RET LDO
#endif

    SetBits16(PMU_CTRL_REG, RETENTION_MODE, 0xF);
//    SetWord16(BANDGAP_REG, 0x0F);
    SetWord16(RF_LNA_CTRL1_REG, 0x24E);
    SetWord16(RF_LNA_CTRL2_REG, 0x26);
    SetWord16(RF_LNA_CTRL3_REG, 0x7);
    SetWord16(RF_REF_OSC_REG, 0x29AC); 
    SetWord16(RF_RSSI_COMP_CTRL_REG, 0x7777);
    SetWord16(RF_VCO_CTRL_REG, 0x1);
    SetBits16(CLK_16M_REG, RC16M_TRIM, 0xA);
#endif
 
//     SetBits32(&tmp32, TXPWRDN,     0x0A);  // default: 0x03 // 0x0A. 10us after TX_EN/RX_EN has been made low, the radio is completely turned off, for both Rx/Tx <Johan Haanstra (Fri 12/14/2012 3:54 PM)>
 
//     SetWord16(RF_ENABLE_CONFIG14_REG, 0x0333); // -	rf_enable_config14_reg 0333
//     SetWord16(RF_CP_CTRL_REG, 4); //TO PREVENT A SWUNG AT BEGIN OF POWERPICTURE
//     SetWord16(RF_AGC_LUT_01_REG,    0x1000);
//     SetWord16(RF_AGC_LUT_23_REG,    0x4909);
//     SetWord16(RF_AGC_LUT_45_REG,    0x4B4A);
//     SetWord16(RF_AGC_LUT_67_REG,    0x5B53);
//     SetWord16(RF_AGC_LUT_89_REG,    0x6B63);
//     SetWord16(RF_AGC_CTRL2_REG,     0x0003); 
//     SetWord16(RF_AFC_CTRL_REG,         0x0005);
//     SetWord16(RF_DC_OFFSET_CTRL1_REG,  0x8080);
//     SetWord16(RF_MIXER_CTRL1_REG,      0x0035);
  
    rf_regs();  
        
#if LUT_PATCH_ENABLED
    const volatile struct LUT_CFG_struct *pLUT_CFG;          // = (const volatile struct LUT_CFG_struct *)(jump_table_struct[lut_cfg_pos]);
    pLUT_CFG= (const volatile struct LUT_CFG_struct *)(jump_table_struct[lut_cfg_pos]);
    if (!pLUT_CFG->HW_LUT_MODE)
    {
        enable_rf_diag_irq(RF_DIAG_IRQ_MODE_RXTX); 
    }
    else
    {
        SetWord16(RF_VCOCAL_CTRL_REG, vcocal_ctrl_reg_val);
#if MGCKMODA_PATCH_ENABLED
        enable_rf_diag_irq(RF_DIAG_IRQ_MODE_TXONLY);                               // This just enables the TX_EN int. RX_EN int enable status remains as it was
#endif //MGCKMODA_PATCH_ENABLED
    }
#else //LUT_PATCH_ENABLED
#if MGCKMODA_PATCH_ENABLED
    enable_rf_diag_irq(RF_DIAG_IRQ_MODE_TXONLY);                               // This just enables the TX_EN int. RX_EN int enable status remains as it was
#endif //MGCKMODA_PATCH_ENABLED
#endif //LUT_PATCH_ENABLED
}

#endif
///@} RF_RPL
