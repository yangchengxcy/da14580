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

#include "app_custom_accel.h"

#if BLE_ACCEL

#include <string.h>                  // string manipulation and functions

#include "app.h"                     // application definitions
#include "app_task.h"                // application task definitions
#include "accel_task.h"              // accelerometer functions

#include "co_bt.h"

#include "arch.h"                      // platform definitions
#include "lis3dh_driver.h"                       // Accelerometer

u8_t LIS3DH_ReadReg(u8_t Reg, u8_t* Data);
u8_t LIS3DH_WriteReg(u8_t Reg, u8_t Data);

#if PLF_DISPLAY
#include "display.h"
#endif //PLF_DISPLAY

extern uint8_t accel_threshold __attribute__((section("exchange_mem_case1")));

void set_accel_freefall(void);
void acc_enable_wakeup_irq(void);
void acc_init(void);

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */
void app_accel_init(void)
{
    acc_init();
	acc_enable_wakeup_irq(); //enable wakeup irq
}

/**
 ****************************************************************************************
 * Accelerometer Application Functions
 ****************************************************************************************
 */

void acc_init_port (void)
{
    return;     //GZ tmp
	  SetWord16(P0_SET_DATA_REG,1<<6);
    SetBits16(P06_MODE_REG,PID,PID_SPI_EN);
    SetBits16(P06_MODE_REG,PUPD,DIR_OUPUT);

		SetWord16(P0_SET_DATA_REG,1<<6);				//set cs HIGH

	
    SetBits16(P00_MODE_REG,PID,PID_SPI_CLK);
    SetBits16(P00_MODE_REG,PUPD,DIR_OUPUT);


  	SetBits16(P03_MODE_REG,PID,PID_SPI_DO);	

    SetBits16(P05_MODE_REG,PID,PID_SPI_DI);
    SetBits16(P05_MODE_REG,PUPD,DIR_PULLUP);
	
}

extern void periph_init(void);

/**
 * WUPCT_Handler:
 */
void WUPCT_Handler(void)
{
	/*
	* The system operates with RC16 clk
	*/
	
	/*
	* Prepare WDOG, i.e. stop
	*/
	SetWord16(SET_FREEZE_REG, FRZ_WDOG);
	
	SetBits16(CLK_PER_REG, WAKEUPCT_ENABLE, 1);  // enable clock of Wakeup Controller
	SetWord16(WKUP_RESET_IRQ_REG, 1); //Acknowledge it
	
	//No more interrupts of this kind
	SetBits16(WKUP_CTRL_REG, ENABLE_IRQ, 0);
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
	SetBits32(BLE_DEEPSLCNTL_REG, SOFT_WAKEUP_REQ, 1);
	
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
	
	SetWord16(WKUP_COMPARE_REG, 1); //Wait for 1 event and wakeup
	//Setup IRQ
#if DEEP_SLEEP_ENABLED   //GZ int
 	SetWord16(WKUP_CTRL_REG, 0x80E); //Enable IRQ, Active High, no debounce, Monitor P1[5]
 #else	
	SetWord16(WKUP_CTRL_REG, 0x808); //Enable IRQ, Active High, no debounce, Monitor P0[7]
 #endif
	
	SetWord16(WKUP_RESET_IRQ_REG, 1); //clear any garbagge
	NVIC_ClearPendingIRQ(WKUP_QUADEC_IRQn); //clear it to be on the safe side...
	
	NVIC_EnableIRQ(WKUP_QUADEC_IRQn);
}

void acc_init(void)
{
	volatile uint8 response;  

return; //GZ tmp    
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

	set_accel_freefall(); //FIXME: test free fall INT
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
	LIS3DH_WriteReg(0x22, 0x00);
	LIS3DH_WriteReg(0x25, 0x80);
	LIS3DH_WriteReg(0x23, 0x20);
	LIS3DH_WriteReg(0x24, 0x08);
	LIS3DH_WriteReg(0x32, 0x00);
	LIS3DH_WriteReg(0x33, 0x00);
	LIS3DH_WriteReg(0x30, 0x00);

	LIS3DH_WriteReg(0x38, 0x01);
	LIS3DH_WriteReg(0x3a, 0x3f);
	LIS3DH_WriteReg(0x3b, 0x0a);
	LIS3DH_WriteReg(0x3c, 0x00);
	LIS3DH_WriteReg(0x3d, 0x00);
	
	
	LIS3DH_ReadReg(0x31, &read_value);
	LIS3DH_ReadReg(0x39, &read_value);

	
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

extern uint8_t accel_adv_count __attribute__((section("exchange_mem_case1")));
extern uint16_t accel_adv_interval __attribute__((section("exchange_mem_case1")));
extern int8_t update_conn_params __attribute__((section("exchange_mem_case1")));

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
    accel_adv_interval = APP_ADV_INT_MIN;
    accel_adv_count = 0;
    ke_timer_clear(APP_ACCEL_ADV_TIMER, TASK_APP);
}

#endif //BLE_ACCEL

/// @} APP
