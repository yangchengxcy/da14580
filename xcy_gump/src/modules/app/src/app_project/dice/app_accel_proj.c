/**
 ****************************************************************************************
 *
 * @file app_accel.c
 *
 * @brief Accelerometer Application entry point
 *
 * Copyright (C) RivieraWaves 2009-2012
 *
 * $Rev: $
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "app_accel_proj.h"

#if BLE_ACCEL

#include <string.h>                  // string manipulation and functions

#include "app.h"                     // application definitions
#include "app_sec.h"
#include "app_task.h"                // application task definitions
#include "accel_task.h"              // accelerometer functions

#include "co_bt.h"

#include "arch.h"                      // platform definitions
#include "..\..\..\plf\refip\src\driver\accel\lis3dh_driver.h"                       // Accelerometer

#include "gpio.h"

u8_t LIS3DH_ReadReg(u8_t Reg, u8_t* Data);
u8_t LIS3DH_WriteReg(u8_t Reg, u8_t Data);

#if PLF_DISPLAY
#include "display.h"
#endif //PLF_DISPLAY

extern uint8_t accel_threshold __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY
extern uint8_t accel_mode __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY
extern uint8_t accel_latency __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY
extern uint8_t accel_window __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY

void set_accel_freefall(void);
void acc_enable_wakeup_irq(void);
void acc_init(void);
void set_accel_click (void);
void set_accel_doubleclick (void);

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */
void app_accel_init(void)
{
    acc_init();
//GZ 201312	acc_enable_wakeup_irq(); //enable wakeup irq
}

/**
 ****************************************************************************************
 * Accelerometer Application Functions
 ****************************************************************************************
 */
//#define DIR_OUPUT   0x3
//#define DIR_PULLUP  0x1
void acc_init_port (void)
{
#if 0    
#if 0 //DEEP_SLEEP_ENABLED 	//GZ int
 	//Set P1_5 to ACCEL's INT1
 	//SetBits16(P15_MODE_REG, PID, PID_GPIO_PUPD); // port function
 	//SetBits16(P15_MODE_REG, PUPD, DIR_INPUT);	
 	//Set P1_5 to ACCEL's INT1
    //RESERVE_GPIO( ACCEL_INT1, GPIO_PORT_1, GPIO_PIN_5, PID_GPIO);
    GPIO_ConfigurePin( GPIO_PORT_1, GPIO_PIN_5, INPUT, PID_GPIO, false );
#else
	//Set P0_7 to ACCEL's INT1
//	SetBits16(P07_MODE_REG, PID, PID_GPIO_PUPD); // port function
//	SetBits16(P07_MODE_REG, PUPD, DIR_INPUT);	
 	//Set P0_7 to ACCEL's INT1
    //RESERVE_GPIO( ACCEL_INT1, GPIO_PORT_0, GPIO_PIN_7, PID_GPIO);
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_7, INPUT, PID_GPIO, false );

//    GPIO_SetPinFunction(GPIO_PORT_0, GPIO_PIN_7, INPUT, PID_GPIO);
    
	//SetBits16(P02_MODE_REG, PID, PID_GPIO_PUPD); // port function
	//SetBits16(P02_MODE_REG, PUPD, DIR_INPUT);	
#endif // DEEP_SLEEP_ENABLED	
    
    //RESERVE_GPIO( SPI_EN, GPIO_PORT_0, GPIO_PIN_6, PID_SPI_EN);
    //RESERVE_GPIO( SPI_CLK, GPIO_PORT_0, GPIO_PIN_0, PID_SPI_CLK);
    //RESERVE_GPIO( SPI_DO, GPIO_PORT_0, GPIO_PIN_3, PID_SPI_DO);	
    //RESERVE_GPIO( SPI_DI, GPIO_PORT_0, GPIO_PIN_5, PID_SPI_DI);
    
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_6, OUTPUT, PID_SPI_EN, true );
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_0, OUTPUT, PID_SPI_CLK, false );
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_3, OUTPUT, PID_SPI_DO, false );	
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_5, INPUT_PULLUP, PID_SPI_DI, false );
#endif

#if 0
	SetWord16(P0_SET_DATA_REG,1<<6);
    SetBits16(P06_MODE_REG,PID,PID_SPI_EN);
    SetBits16(P06_MODE_REG,PUPD,DIR_OUPUT);

	SetWord16(P0_SET_DATA_REG,1<<6);				//set cs HIGH

	
    SetBits16(P00_MODE_REG,PID,PID_SPI_CLK);
    SetBits16(P00_MODE_REG,PUPD,DIR_OUPUT);


  	SetBits16(P03_MODE_REG,PID,PID_SPI_DO);	

    SetBits16(P05_MODE_REG,PID,PID_SPI_DI);
    SetBits16(P05_MODE_REG,PUPD,DIR_PULLUP);
#endif	
}

extern void periph_init(void);

/**
 * WUPCT_Handler:
 */
void WKUP_QUADEC_Handler(void)
{
	/*
	* The system operates with RC16 clk
	*/
#ifndef CFG_WD	
	/*
	* Prepare WDOG, i.e. stop
	*/
	SetWord16(SET_FREEZE_REG, FRZ_WDOG);
#endif
	
	SetBits16(CLK_PER_REG, WAKEUPCT_ENABLE, 1);  // enable clock of Wakeup Controller
	SetWord16(WKUP_RESET_IRQ_REG, 1); //Acknowledge it
	
	//No more interrupts of this kind
	SetBits16(WKUP_CTRL_REG, WKUP_ENABLE_IRQ, 0);
	NVIC_DisableIRQ(WKUP_QUADEC_IRQn);
	
	/*
	* Init System Power Domain blocks: GPIO, WD Timer, Sys Timer, etc.
	* Power up and init Peripheral Power Domain blocks,
	* and finally release the pad latches.
	*/
	if(GetBits16(SYS_STAT_REG, PER_IS_DOWN))
		periph_init();
	
	//Disable LIS3DH interrupt
	LIS3DH_WriteReg(0x22, 0x00); //CTRL_REG3: 
	LIS3DH_WriteReg(0x30, 0x00); //INT1_CFG: Disable all ints

	//Wakeup BLE here
	//SetBits32(BLE_DEEPSLCNTL_REG, SOFT_WAKEUP_REQ, 1);
	SetBits32(GP_CONTROL_REG, BLE_WAKEUP_REQ, 1); 

//set deep sleep with no periodical wakeup
    rwip_env.ext_wakeup_enable = 0;
    
	//We cannot call app_adv_start() here because it will end up in setting a kernel timer but 
	//BLE is not yet up and running and time has not been compensated! We'll send a kernel msg instead!
	//app_adv_start();
	
#if BLE_ACCEL
	/*
	* Notify ACCEL Application to start advertising
	*/
	ke_msg_send_basic(APP_ACCEL_MSG, TASK_APP, NULL);
#endif	

	return;
}

void acc_enable_wakeup_irq(void)
{
	volatile int temp;
	
	SetBits16(CLK_PER_REG, WAKEUPCT_ENABLE, 1);  // enable clock of Wakeup Controller
	
    SetWord16(WKUP_RESET_CNTR_REG, 0);        
	SetWord16(WKUP_COMPARE_REG, 1); //Wait for 1 event and wakeup
	//Setup IRQ
#if DEEP_SLEEP_ENABLED   //GZ int
 	//SetWord16(WKUP_CTRL_REG, 0x80); //Enable IRQ, Active High, no debounce, Monitor P1[5]
 	SetWord16(WKUP_SELECT_P1_REG, 0x20); //Enable IRQ, Active High, no debounce, Monitor P1[5]
    SetWord16(WKUP_POL_P1_REG, 0x00);
 #else	
	//SetWord16(WKUP_CTRL_REG, 0x80); //Enable IRQ, Active High, no debounce, Monitor P0[7]
	SetWord16(WKUP_SELECT_P0_REG, 0x80); //Enable IRQ, Active High, no debounce, Monitor P0[7]
    SetWord16(WKUP_POL_P0_REG, 0x00);
 	//SetWord16(WKUP_SELECT_P1_REG, 0x01); //Enable IRQ, Active High, no debounce, Monitor P1[5]
    //SetWord16(WKUP_POL_P1_REG, 0x00);
 #endif
	
	SetWord16(WKUP_RESET_IRQ_REG, 1); //clear any garbagge
	NVIC_ClearPendingIRQ(WKUP_QUADEC_IRQn); //clear it to be on the safe side...
	
    SetWord16(WKUP_CTRL_REG, 0x80); //Enable IRQ, no debounce

//set deep sleep with no periodical wakeup
    rwip_env.ext_wakeup_enable = 2;
    
	NVIC_EnableIRQ(WKUP_QUADEC_IRQn);
}

void acc_init(void)
{
	volatile uint8 response;  

	//disable gpio irq
	NVIC_DisableIRQ(GPIO3_IRQn);
    acc_init_port();
	
	
#if 1	
	
#if 0	
	// REBOOT SENSOR
	LIS3DH_WriteReg(0x20, 0x80);
	LIS3DH_WriteReg(0x25, 0x10); //BOOT stat on INT1
// 	LIS3DH_WriteReg(0x32, 0x00); //INT threshold = LARGE!
// 	LIS3DH_WriteReg(0x33, 0x00); //INT duration = LARGE!
	LIS3DH_WriteReg(0x24, 0x88); //REBOOT - Latch int on INT1
	LIS3DH_WriteReg(0x24, 0x08); //Clear REBOOT - Latch int on INT1
	do{
		LIS3DH_ReadReg(0x39, &response); //Acknowledge all IRQs
		LIS3DH_ReadReg(0x31, &response); //Acknowledge all IRQs
		if(response & 0x40) {
			LIS3DH_WriteReg(0x25, 0x00); //Clear BOOT stat on INT1
			LIS3DH_WriteReg(0x24, 0x00); //Clear REBOOT - Latch int on INT1
		}
	}while(response & 0x40);
	LIS3DH_WriteReg(0x25, 0x00); //clear
	// REBOOT ENDS
#endif
	LIS3DH_WriteReg(0x24, 0x88); //REBOOT - Latch int on INT1
	
	LIS3DH_WriteReg(0x20, 0x77);
	LIS3DH_WriteReg(0x21, 0x00);
	LIS3DH_WriteReg(0x22, 0x00);
	LIS3DH_WriteReg(0x25, 0x00);
	LIS3DH_WriteReg(0x23, 0x80);
	LIS3DH_WriteReg(0x24, 0x08);

//	LIS3DH_WriteReg(0x32, 0x10);
//	LIS3DH_WriteReg(0x33, 0x03);
//	LIS3DH_WriteReg(0x30, 0x0a);

	LIS3DH_WriteReg(0x32, 0x00);
	LIS3DH_WriteReg(0x33, 0x00);
	//GZ LIS3DH_WriteReg(0x30, 0x00);

    if(accel_mode == 0)
	set_accel_freefall(); //FIXME: test free fall INT
    else
    set_accel_click();

#endif
#if 0
	response = LIS3DH_SetODR(LIS3DH_ODR_25Hz);
		
  //set PowerMode 
  response = LIS3DH_SetMode(LIS3DH_NORMAL);
  
  //set Fullscale
  response = LIS3DH_SetFullScale(LIS3DH_FULLSCALE_2);
  
  //set axis Enable
  response = LIS3DH_SetAxis(LIS3DH_X_ENABLE | LIS3DH_Y_ENABLE | LIS3DH_Z_ENABLE);
  
	#endif
}

void acc_start(uint16_t * axis_enables, uint8_t range)
{
#if 0
volatile uint8 response;  
  //set Fullscale
  response = LIS3DH_SetFullScale(LIS3DH_FULLSCALE_2);
  
  //set axis Enable
  response = LIS3DH_SetAxis(axis_enables[0] | axis_enables[1] | axis_enables[2]);
#endif
	//Enter normal mode
	LIS3DH_WriteReg(0x20, 0x77); //CTRL_REG1: Turn on the sensor, enable X, Y, and Z. ODR = 400Hz. LPen = 0 "Normal" mode
	LIS3DH_WriteReg(0x23, 0x80); //CTRL_REG4: FS = 2g. HR = 0 "Normal" mode with low resolution?	
}
void app_accel_enable(void)
{
    // Allocate the message
    struct accel_enable_req * req = KE_MSG_ALLOC(ACCEL_ENABLE_REQ, TASK_ACCEL, TASK_APP,
                                                 accel_enable_req);

    // Fill in the parameter structure
    req->appid = TASK_APP;
    req->conhdl = app_env.conhdl;

    // Send the message
    ke_msg_send(req);

    // Reset the accelerometer driver
//vm	
//    acc_init();
}
void acc_stop(void)
{
	
	// Turn off Accell
	LIS3DH_WriteReg(0x20, 0x00);
#if 0
	LIS3DH_WriteReg(0x20, 0x27);
	LIS3DH_WriteReg(0x21, 0x00);
	LIS3DH_WriteReg(0x22, 0x00);
	LIS3DH_WriteReg(0x25, 0x82);
	LIS3DH_WriteReg(0x23, 0x80);
	LIS3DH_WriteReg(0x24, 0x08);


	LIS3DH_WriteReg(0x32, 0x00);
	LIS3DH_WriteReg(0x33, 0x00);
	LIS3DH_WriteReg(0x30, 0x00);

	
	LIS3DH_WriteReg(0x3a, 0x3f);

//	LIS3DH_WriteReg(0x3b, 0x33);
//	LIS3DH_WriteReg(0x3c, 0x12);
//	LIS3DH_WriteReg(0x3d, 0x42);

	LIS3DH_WriteReg(0x3b, 0x0a);
	LIS3DH_WriteReg(0x3c, 0x00);
	LIS3DH_WriteReg(0x3d, 0x00);


	LIS3DH_WriteReg(0x38, 0x01);
	
#endif
//setup gpio irq

		SetWord16(P27_MODE_REG,0x100);
//		SetWord16(P06_MODE_REG,0x100);

    SetWord16(GPIO_IRQ3_IN_SEL_REG, 22); //P0.0 is selected, is diagnostic port RXEN
//		SetWord16(GPIO_IRQ3_IN_SEL_REG, 7); //P0.0 is selected, is diagnostic port RXEN
    SetWord16(GPIO_RESET_IRQ_REG, 1);
    SetBits16(GPIO_INT_LEVEL_CTRL_REG, EDGE_LEVELn3, 1); //select rising edge P0.6
    SetBits16(GPIO_INT_LEVEL_CTRL_REG, INPUT_LEVEL3, 1); //select rising edge P0.6

    NVIC_SetPriority(GPIO3_IRQn,1); 
    NVIC_EnableIRQ(GPIO3_IRQn);

}

void GPIO3_Handler (void)
{
	
#if 1
	AxesRaw_t accel_data;
	volatile  uint8_t read_value;

	SetWord16(GPIO_RESET_IRQ_REG,0x08);
  NVIC_DisableIRQ(GPIO3_IRQn);

	acc_init_port();
	
	LIS3DH_ReadReg(0x31, (u8_t *)&read_value);
	LIS3DH_ReadReg(0x39, (u8_t *)&read_value);
		
	LIS3DH_GetAccAxesRaw(&accel_data);
#endif
#if 0	
	AxesRaw_t accel_data;
	volatile  uint8_t read_value;
 
	LIS3DH_ReadReg(0x31, (u8_t *)&read_value);
	LIS3DH_ReadReg(0x39, (u8_t *)&read_value);
		
 LIS3DH_GetAccAxesRaw(&accel_data);
//    sprintf((char*)buffer2, " X=%X Y=%X Z=%X \r\n", accel_data.AXIS_X, accel_data.AXIS_Y, accel_data.AXIS_Z);

	SetWord16(GPIO_RESET_IRQ_REG,0x08);
#endif
	
}
void set_accel_wakeup(void)
{
	
	LIS3DH_WriteReg(0x20, 0x00);
	LIS3DH_WriteReg(0x20, 0x47);
	LIS3DH_WriteReg(0x21, 0x00);
	LIS3DH_WriteReg(0x22, 0x00);
	LIS3DH_WriteReg(0x25, 0x40);
	LIS3DH_WriteReg(0x23, 0x80);
	LIS3DH_WriteReg(0x24, 0x08);
	LIS3DH_WriteReg(0x32, 0x10);
	LIS3DH_WriteReg(0x33, 0x03);
	LIS3DH_WriteReg(0x30, 0x0a);

}

void set_accel_freefall(void)
{
	int temp;
	int cnt=100;
// 	LIS3DH_WriteReg(0x20, 0x00);
// 	LIS3DH_WriteReg(0x20, 0x17);
// 	LIS3DH_WriteReg(0x21, 0x00);
// 	LIS3DH_WriteReg(0x22, 0x00);
// 	LIS3DH_WriteReg(0x25, 0x40);
// 	LIS3DH_WriteReg(0x23, 0x80);
// 	LIS3DH_WriteReg(0x24, 0x08);
// 	LIS3DH_WriteReg(0x32, 0x16);
// 	LIS3DH_WriteReg(0x33, 0x01);
// 	LIS3DH_WriteReg(0x30, 0x95);

LIS3DH_WriteReg(0x22, 0x00); //CTRL_REG3: Interrupt driven to INT1 pad
	
	
	//GZ LIS3DH_WriteReg(0x20, 0x00); //CTRL_REG1: disable X, Y, Z updates
//	LIS3DH_WriteReg(0x20, 0xA7); //CTRL_REG1: Turn on the sensor, enable X, Y, and Z. ODR = 100Hz. LPen = 1 "Low Power" mode
//	LIS3DH_WriteReg(0x20, 0xAF); //CTRL_REG1: Turn on the sensor, enable X, Y, and Z. ODR = 100Hz. LPen = 1 "Low Power" mode
	LIS3DH_WriteReg(0x20, 0x2F); //CTRL_REG1: Turn on the sensor, enable X, Y, and Z. ODR = 25Hz. LPen = 1 "Low Power" mode
	LIS3DH_WriteReg(0x21, 0x00); //CTRL_REG2: High-pass filter disabled
//	LIS3DH_WriteReg(0x23, 0x00); //CTRL_REG4: FS = 2g. HR = 0 "Low Power" mode
	LIS3DH_WriteReg(0x23, 0x00); //CTRL_REG4: Allow updates, FS = 2g. HR = 0 "Low Power" mode
	LIS3DH_WriteReg(0x24, 0x08); //CTRL_REG5: Interrupt latched
//	LIS3DH_WriteReg(0x32, 0x16); //INT1_THS: Set free-fall threshold = 350 mg
//	LIS3DH_WriteReg(0x32, 0x01); //INT1_THS: Set free-fall threshold = XX mg
//	LIS3DH_WriteReg(0x33, 0x03); //INT1_DURATION: Set minimum event duration

	//GZ LIS3DH_WriteReg(0x30, 0x00); //INT1_CFG: Disable all INTs
	//GZ LIS3DH_WriteReg(0x32, 0x00); //INT threshold = LARGE value to prevent unecessary interrupts
	//GZ LIS3DH_WriteReg(0x33, 0x00); //INT1_DURATION = LARGE value to prevent unecessary interrupts
	//GZ LIS3DH_ReadReg(0x31, &temp); //Acknowledge all IRQs
	
//GZ	LIS3DH_WriteReg(0x32, 0x30); //INT1_THS: Set free-fall threshold = XX mg - The bigger the more sensitive! 
	if(accel_threshold >= ACCEL_MIN_THRESHOLD)
	{
		LIS3DH_WriteReg(0x32, accel_threshold); //INT1_THS: Set free-fall threshold = XX mg - The bigger the more sensitive! 
	}
	else
	LIS3DH_WriteReg(0x32, ACCEL_DEF_THRESHOLD); //INT1_THS: Set free-fall threshold = XX mg - The bigger the more sensitive! 

	LIS3DH_WriteReg(0x33, 0x01); //INT1_DURATION: Set minimum event duration (1/25Hz = 40msec)
	while(cnt--)
	{
		LIS3DH_ReadReg(0x31, (u8_t *)&temp); //Acknowledge all IRQs
		if((temp & 0x40) == 0)
			break;
	}

	LIS3DH_ReadReg(0x31, (u8_t *)&temp); //Acknowledge all IRQs
	LIS3DH_WriteReg(0x22, 0x40); //CTRL_REG3: Interrupt driven to INT1 pad

	LIS3DH_WriteReg(0x30, 0x95); //INT1_CFG: Configure free-fall recognition

    cnt = 100;
	while(cnt--)
	{
		LIS3DH_ReadReg(0x31, (u8_t *)&temp); //Acknowledge all IRQs
		if((temp & 0x40) == 0)
			break;
	}
}

void acc_int_restart(void)
{
	int temp;
	int cnt=100;
	LIS3DH_WriteReg(0x24, 0x08); //CTRL_REG5: Interrupt latched
	LIS3DH_WriteReg(0x20, 0x2F); //CTRL_REG1: Turn on the sensor, enable X, Y, and Z. ODR = 25Hz. LPen = 1 "Low Power" mode
	//LIS3DH_WriteReg(0x21, 0x00); //CTRL_REG2: High-pass filter disabled
	//LIS3DH_WriteReg(0x23, 0x00); //CTRL_REG4: Allow updates, FS = 2g. HR = 0 "Low Power" mode
	//LIS3DH_WriteReg(0x32, 0x30); //INT1_THS: Set free-fall threshold = XX mg - The bigger the more sensitive! 
	//LIS3DH_WriteReg(0x33, 0x01); //INT1_DURATION: Set minimum event duration (1/25Hz = 40msec)
	while(cnt--)
	{
		LIS3DH_ReadReg(0x31, (u8_t *)&temp); //Acknowledge all IRQs
		if((temp & 0x40) == 0)
			break;
	}
	LIS3DH_WriteReg(0x22, 0x40); //CTRL_REG3: Interrupt driven to INT1 pad
	LIS3DH_ReadReg(0x31, (u8_t *)&temp); //Acknowledge all IRQs
}

void set_accel_thr()
{
	if(accel_threshold >= ACCEL_MIN_THRESHOLD)
	{
		LIS3DH_WriteReg(0x32, accel_threshold); //INT1_THS: Set free-fall threshold = XX mg - The bigger the more sensitive! 
	}
	else
	LIS3DH_WriteReg(0x32, ACCEL_DEF_THRESHOLD); //INT1_THS: Set free-fall threshold = XX mg - The bigger the more sensitive! 	
}

void set_accel_6dpos (void)
{
	LIS3DH_WriteReg(0x20, 0x47);
	LIS3DH_WriteReg(0x21, 0x00);
	LIS3DH_WriteReg(0x22, 0x00);
	LIS3DH_WriteReg(0x25, 0x40);
	LIS3DH_WriteReg(0x23, 0x80);
	LIS3DH_WriteReg(0x24, 0x08);
	LIS3DH_WriteReg(0x32, 0x21);
	LIS3DH_WriteReg(0x33, 0x00);
	LIS3DH_WriteReg(0x30, 0xff);

}

void set_accel_6dmov (void)
{
	LIS3DH_WriteReg(0x20, 0x47);
	LIS3DH_WriteReg(0x21, 0x00);
	LIS3DH_WriteReg(0x22, 0x00);
	LIS3DH_WriteReg(0x25, 0x40);
	LIS3DH_WriteReg(0x23, 0x80);
	LIS3DH_WriteReg(0x24, 0x00);
	LIS3DH_WriteReg(0x32, 0x21);
	LIS3DH_WriteReg(0x33, 0x00);
	LIS3DH_WriteReg(0x30, 0x7f);

}


void set_accel_click (void)
{
	 uint8_t read_value;
	int cnt=100;
    
	acc_init_port();
#if 0	
	LIS3DH_WriteReg(0x20, 0x00);
	LIS3DH_WriteReg(0x20, 0x27);
	LIS3DH_WriteReg(0x21, 0x00);
	LIS3DH_WriteReg(0x22, 0x00);
	LIS3DH_WriteReg(0x25, 0x80);
	LIS3DH_WriteReg(0x23, 0x80);
	LIS3DH_WriteReg(0x24, 0x08);
	LIS3DH_WriteReg(0x32, 0x00);
	LIS3DH_WriteReg(0x33, 0x00);
	LIS3DH_WriteReg(0x30, 0x00);
	LIS3DH_WriteReg(0x3a, 0x3f);
//	LIS3DH_WriteReg(0x3b, 0x33);
//	LIS3DH_WriteReg(0x3c, 0x12);
//	LIS3DH_WriteReg(0x3d, 0x42);
	LIS3DH_WriteReg(0x3b, 0x0a);
	LIS3DH_WriteReg(0x3c, 0x00);
	LIS3DH_WriteReg(0x3d, 0x00);
	LIS3DH_WriteReg(0x38, 0x01);
#endif

	LIS3DH_WriteReg(0x20, 0x00);
	LIS3DH_WriteReg(0x20, 0x27);
	LIS3DH_WriteReg(0x21, 0x00);
	LIS3DH_WriteReg(0x22, 0x80);
	LIS3DH_WriteReg(0x25, 0x80);
	LIS3DH_WriteReg(0x23, 0x20);
	LIS3DH_WriteReg(0x24, 0x08);
	LIS3DH_WriteReg(0x32, 0x00);
	LIS3DH_WriteReg(0x33, 0x00);
	LIS3DH_WriteReg(0x30, 0x00);

	LIS3DH_WriteReg(0x38, 0x15);
	LIS3DH_WriteReg(0x3a, 0x3f);
	LIS3DH_WriteReg(0x3b, 0x01);
	LIS3DH_WriteReg(0x3c, 0x01);
	LIS3DH_WriteReg(0x3d, 0xff);
	
	
	//if(accel_threshold >= ACCEL_MIN_THRESHOLD)
	{
		LIS3DH_WriteReg(0x3a, accel_threshold); //INT1_THS: Set free-fall threshold = XX mg - The bigger the more sensitive! 
	}
	//else
	//LIS3DH_WriteReg(0x3a, ACCEL_DEF_THRESHOLD); //INT1_THS: Set free-fall threshold = XX mg - The bigger the more sensitive! 	

	if(accel_mode > 1)
        set_accel_doubleclick();
	
	while(cnt--)
	{
	LIS3DH_ReadReg(0x31, &read_value);
	LIS3DH_ReadReg(0x39, &read_value);
		if((read_value & 0x40) == 0)
			break;
	}

	
}

void set_accel_doubleclick (void)
{
	LIS3DH_WriteReg(0x38, 0x2A);    
	LIS3DH_WriteReg(0x3c, accel_latency);
	LIS3DH_WriteReg(0x3d, accel_window);
}

#if 0

void test_accel(void)
{
	
	
	volatile unsigned char response;
  AxesRaw_t data;
	uint8_t buffer[64]; 
//	uint8_t buffer1[64]; 

  uint8_t position=0, old_position=0;
	volatile unsigned i;
	  uint8_t value;
  uint8_t reg_val[0x3d];
   uint8_t reg_val1[0x3d];

//	*(volatile unsigned*)(0x40000200)&=~0x40000;
	
	  SetWord16(P0_SET_DATA_REG,1<<6);
    SetBits16(P06_MODE_REG,PID,PID_SPI_EN);
    SetBits16(P06_MODE_REG,PUPD,DIR_OUPUT);

		SetWord16(P0_SET_DATA_REG,1<<6);				//set cs HIGH

	
    SetBits16(P00_MODE_REG,PID,PID_SPI_CLK);
    SetBits16(P00_MODE_REG,PUPD,DIR_OUPUT);


  	SetBits16(P05_MODE_REG,PID,PID_SPI_DO);	

    SetBits16(P03_MODE_REG,PID,PID_SPI_DI);
    SetBits16(P03_MODE_REG,PUPD,DIR_PULLUP);
	
	
		 SetBits16(SPI_CTRL_REG,SPI_WORD,1);  			    // set to 16bit mode
		SetBits16(SPI_CTRL_REG,SPI_POL,1);  			    // set to spi mode 3
		SetBits16(SPI_CTRL_REG,SPI_PHA,1);  			    // set to spi mode 

		SetBits16(SPI_CTRL_REG,SPI_ON,1);    	  			// enable SPI block
    SetBits16(SPI_CTRL_REG,SPI_CLK,2);    	  		// fastest clock
	
	
	


		SetWord16(P27_MODE_REG,0x100);


for (i=0;i<0x3d;i++)
{
	LIS3DH_ReadReg(i, &reg_val[i]);
}

//set_accel_wakeup();
//set_accel_freefall();
//set_accel_6dpos();
//set_accel_6dmov();
set_accel_click();



		SetWord16(P27_MODE_REG,0x100);

    SetWord16(GPIO_IRQ3_IN_SEL_REG, 22); //P0.0 is selected, is diagnostic port RXEN
    SetWord16(GPIO_RESET_IRQ_REG, 1);
    SetBits16(GPIO_INT_LEVEL_CTRL_REG, EDGE_LEVELn3, 1); //select rising edge P0.6
    SetBits16(GPIO_INT_LEVEL_CTRL_REG, INPUT_LEVEL3, 0); //select rising edge P0.6

    NVIC_SetPriority(GPIO3_IRQn,1); 
    NVIC_EnableIRQ(GPIO3_IRQn);




	
	while (1);
	
#if 0	
	while (1)
	{
		if((GetWord16(P0_DATA_REG)&0x80))
		{
			LIS3DH_ReadReg(0x31, &value);
			#if 0
		  response = LIS3DH_GetAccAxesRaw(&data);
			if(response==1){
			//print data values
			//   sprintf((char*)buffer, "X=%6d Y=%6d Z=%6d \r\n", data.AXIS_X, data.AXIS_Y, data.AXIS_Z);
			//   sprintf((char*)buffer1, " X=%X Y=%X Z=%X \r\n", data.AXIS_X, data.AXIS_Y, data.AXIS_Z);

			}			
	#endif			
		}
	}
	
#endif	
	
	
#if 0
	response = LIS3DH_SetODR(LIS3DH_ODR_100Hz);
		
  //set PowerMode 
  response = LIS3DH_SetMode(LIS3DH_NORMAL);
  
  //set Fullscale
  response = LIS3DH_SetFullScale(LIS3DH_FULLSCALE_2);
  
  //set axis Enable
  response = LIS3DH_SetAxis(LIS3DH_X_ENABLE | LIS3DH_Y_ENABLE | LIS3DH_Z_ENABLE);
	
	response = LIS3DH_SetInt2Pin( LIS3DH_I2_INT2_ON_PIN_INT2_ENABLE ) ;
	
#endif  
for (i=0;i<0x3d;i++)
{
	LIS3DH_ReadReg(i, &reg_val1[i]);
}
  



  while(1){
  //get Acceleration Raw data  
		
		for (i=0;i<100;i++);

//	LIS3DH_ReadReg(0x07, &value);
	LIS3DH_ReadReg(LIS3DH_STATUS_REG, &value);
//if ((value & 7) == 7) {
if (value & 8) {		
  response = LIS3DH_GetAccAxesRaw(&data);
	if(response==1){
    //print data values
    sprintf((char*)buffer, "X=%6d Y=%6d Z=%6d \r\n", data.AXIS_X, data.AXIS_Y, data.AXIS_Z);
//    sprintf((char*)buffer, "X=%6d Y=%6d Z=%6d \r\n", data.AXIS_X, data.AXIS_Y, data.AXIS_Z);
//    sprintf((char*)buffer1, " X=%X Y=%X Z=%X \r\n", data.AXIS_X, data.AXIS_Y, data.AXIS_Z);

    old_position = position;
  }
}
 }

}

#endif

#if 0
void GPIO0_Handler (void)
{
	AxesRaw_t accel_data;
	uint8_t buffer2[64]; 
	volatile  uint8_t read_value;
 
	LIS3DH_ReadReg(0x31, &read_value);
	LIS3DH_ReadReg(0x39, &read_value);
		
 LIS3DH_GetAccAxesRaw(&accel_data);
//    sprintf((char*)buffer2, " X=%X Y=%X Z=%X \r\n", accel_data.AXIS_X, accel_data.AXIS_Y, accel_data.AXIS_Z);

	SetWord16(GPIO_RESET_IRQ_REG,0x01);
}
#endif

extern uint8_t accel_adv_count __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY
extern uint16_t accel_adv_interval __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY
extern int8_t update_conn_params __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY

/**
 ****************************************************************************************
 * @brief Start Timer to control Adertising interval.
 *
 ****************************************************************************************
 */
void app_accel_adv_started(void)
{
    ke_timer_set(APP_ACCEL_ADV_TIMER, TASK_APP, 50);
}

/**
 ****************************************************************************************
 * @brief Stop Timer that controls Adertising interval.
 *
 ****************************************************************************************
 */
void app_accel_adv_stopped(void)
{
    accel_adv_interval = 0x20;  //APP_ADV_INT_MIN;
    accel_adv_count = 0;
    ke_timer_clear(APP_ACCEL_ADV_TIMER, TASK_APP);
}

void app_set_dev_config_complete_func(void)
{
    // We are now in Initialization State
    ke_state_set(TASK_APP, APP_DB_INIT);

    // Add the first required service in the database
    if (app_db_init())
    {
        // No service to add in the DB -> Start Advertising
        app_adv_start();
    }
 
    return;
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
    ke_state_set(TASK_APP, APP_CONNECTED);


    update_conn_params = 1;
    ke_timer_set(APP_ACCEL_TIMER, TASK_APP, 5);

    return;
}

/**
 ****************************************************************************************
 * @brief app_api function. Called upon connection param's update completion
 *
 * @return void.
 ****************************************************************************************
*/

void app_update_params_complete_func(void)
{
        //rwip_env.sleep_enable = true;
    update_conn_params = 1;
    ke_timer_set(APP_ACCEL_TIMER, TASK_APP, 5);

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
    //GZ cfm->data.pairing_feat.ikey_dist      = GAP_KDIST_NONE;
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
    
    app_param_update_func();
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

#endif //BLE_APP_SEC

#endif //BLE_ACCEL

/// @} APP
