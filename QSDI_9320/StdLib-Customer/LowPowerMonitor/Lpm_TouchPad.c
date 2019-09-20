//******************************************************************************
// COPYRIGHT (C) STMicroelectronics 2011.
//
// All rights reserved. This document contains proprietary and
// confidential information of the STMicroelectronics Group. This
// document is not to be copied in whole or part. STMicroelectronics
// assumes no responsibility for the consequences of use of such
// information nor for any infringement of patents or other rights of
// third parties which may result from its use. No license is granted by
// implication or otherwise under any patent or patent rights of
// STMicroelectronics.
// STMicroelectronics
// products are not authorized for use as critical components in life
// support devices or systems without express written approval of
// STMicroelectronics.
//=============================================================================
//
// MODULE:      Lpm_TouchPad.c
//
// USAGE:       
//
//******************************************************************************

//******************************************************************************
//  I N C L U D E    F I L E S
//******************************************************************************
#include ".\LowPowerMonitor\Lpm_All.h"


#ifdef TOUCHPAD_SUPPORT

#define TOUCHPAD_CONTROLLER		TPAD_STMPE16M31	//TPAD_STMPE1208S

//#define TOUCHPAD_DEBUG

#ifdef TOUCHPAD_DEBUG
   #define msg(a)            gm_Print(a,0)
#else
   #define msg(a)
#endif

//******************************************************************************
//  G L O B A L    V A R I A B L E S
//******************************************************************************	
//static WORD W_TPadData[8]={0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF};
BYTE TouchKeyIntFlag = 0;
BYTE HorV_Flag;
BYTE WheelBuffer[3];
extern BOOL powerUpRequest;

// 	Below Buffer is enhance percision of touch pad  
BYTE ROM TouchPadWheelValue[24][3]=   		//Right+down
{
	{0x01, 0x02,0x04},
	{0x02, 0x04,0x08},
	{0x04, 0x08,0x10},
	{0x08, 0x10,0x20},	
	{0x10, 0x20,0x40},
	{0x20, 0x40,0x80},
	{0x40, 0x80,0x01},
	{0x80, 0x01,0x02},	
	{0x01, 0x02,0x08},
	{0x02, 0x04,0x10},
	{0x04, 0x08,0x20},
	{0x08, 0x10,0x40},	
	{0x10, 0x20,0x80},
	{0x20, 0x40,0x01},
	{0x40, 0x80,0x02},
	{0x80, 0x01,0x04},
	{0x01, 0x04,0x08},
	{0x02, 0x08,0x10},
	{0x04, 0x10,0x20},
	{0x08, 0x20,0x40},	
	{0x10, 0x40,0x80},
	{0x20, 0x80,0x01},
	{0x40, 0x01,0x02},
	{0x80, 0x02,0x04}		
};

/*********************  Touchpad registers *******************/
#if (TOUCHPAD_CONTROLLER == TPAD_STMPE16M31)
#define TPAD_SYSCON_1				(unsigned char) 0x03
#define TPAD_SYSCON_2				(unsigned char) 0x04
#define TPAD_INT_CTRL				(unsigned char) 0x06
#define TPAD_INT_STA				(unsigned char) 0x08
#define TPAD_INT_EN				(unsigned char) 0x09
#define TPAD_GPIO_DIR				(unsigned char) 0x10
#define TPAD_GPIO_MP_STA			(unsigned char) 0x12
#define TPAD_GPIO_SET_PIN			(unsigned char) 0x14
#define TPAD_GPIO_AF				(unsigned char) 0x16
#define TPAD_GPIO0_PWM_CFG			(unsigned char) 0x20
#define TPAD_GPIO1_PWM_CFG			(unsigned char) 0x21
#define TPAD_GPIO2_PWM_CFG			(unsigned char) 0x22
#define TPAD_GPIO3_PWM_CFG			(unsigned char) 0x23
#define TPAD_GPIO4_PWM_CFG			(unsigned char) 0x24
#define TPAD_GPIO5_PWM_CFG			(unsigned char) 0x25
#define TPAD_GPIO6_PWM_CFG			(unsigned char) 0x26
#define TPAD_GPIO7_PWM_CFG			(unsigned char) 0x27
#define TPAD_GPIO8_PWM_CFG			(unsigned char) 0x28
#define TPAD_GPIO9_PWM_CFG			(unsigned char) 0x29
#define TPAD_GPIO10_PWM_CFG			(unsigned char) 0x2A
#define TPAD_GPIO11_PWM_CFG			(unsigned char) 0x2B
#define TPAD_GPIO12_PWM_CFG			(unsigned char) 0x2C
#define TPAD_GPIO13_PWM_CFG			(unsigned char) 0x2D
#define TPAD_GPIO14_PWM_CFG			(unsigned char) 0x2E
#define TPAD_GPIO15_PWM_CFG			(unsigned char) 0x2F
#define TPAD_PWM_MASTER_EN			(unsigned char) 0x30
#define TPAD_PWM_0_SET				(unsigned char) 0x40
#define TPAD_PWM_1_SET				(unsigned char) 0x44
#define TPAD_PWM_2_SET				(unsigned char) 0x48
#define TPAD_PWM_3_SET				(unsigned char) 0x4C
#define TPAD_PWM_4_SET				(unsigned char) 0x50
#define TPAD_PWM_5_SET				(unsigned char) 0x54
#define TPAD_PWM_6_SET				(unsigned char) 0x58
#define TPAD_PWM_7_SET				(unsigned char) 0x5C
#define TPAD_CAP_SEN_CTRL			(unsigned char) 0x70
#define TPAD_CH_SEL				(unsigned char) 0x72
#define TPAD_CAL_INT				(unsigned char) 0x76
#define TPAD_CAL_MOD				(unsigned char) 0x77
#define TPAD_MAF_SET				(unsigned char) 0x78
#define TPAD_DATA_TYPE				(unsigned char) 0x7C
#define TPAD_RATIO_ENG_SET			(unsigned char) 0x80
#define TPAD_RATIO_ENG1_CFG			(unsigned char) 0x81
#define TPAD_RATIO_ENG2_CFG			(unsigned char) 0x82
#define TPAD_KEY_FILT_CFG			(unsigned char) 0x90
#define TPAD_KEY_FILT_GROUP1			(unsigned char) 0x92
#define TPAD_KEY_FILT_GROUP2			(unsigned char) 0x96
#define TPAD_KEY_FILT_DATA			(unsigned char) 0x9A
#define TPAD_TOUCH_DET				(unsigned char) 0xB4
#define TPAD_CH_DATA0				(unsigned char) 0xC0
#define TPAD_CH_DATA1				(unsigned char) 0xC2
#define TPAD_CH_DATA2				(unsigned char) 0xC4
#define TPAD_CH_DATA3				(unsigned char) 0xC6
#define TPAD_CH_DATA4				(unsigned char) 0xC8
#define TPAD_CH_DATA14				(unsigned char) 0xDC
#define TPAD_CH_DATA15				(unsigned char) 0xDE


#elif (TOUCHPAD_CONTROLLER == TPAD_STMPE1208S)
#define FEATURE_SELECT 				(unsigned char) 0x00
#define TOUCH_VARIANCE				(unsigned char) 0x01
#define ENVIRONMENTAL_VARIANCE 		(unsigned char) 0x0D
#define ETC_WAIT 					(unsigned char) 0x0E
#define STRENGTH_THRESHOLD 			(unsigned char) 0x10
#define INTEGRATION_TIME 			(unsigned char) 0x1C
#define GPIO_REG 					(unsigned char) 0x1E
#define GPIO_CONFIGURATION 			(unsigned char) 0x20
#define GPIO_DIRECTION 				(unsigned char) 0x22
#define CALIBRATION_INTERVAL 		(unsigned char) 0x2A
#define CONTROL_1 					(unsigned char) 0x24
#define TOUCH_BYTE 					(unsigned char) 0x75
#endif


//******************************************************************************
//  C O D E
//******************************************************************************
gmt_RET_STAT Lpm_TouchPadCheckDevice(void)
{
	BYTE B_StartLimit = DEVICE_CHECK_LIMIT;

	while(B_StartLimit--)
	{
		if (Lpm_I2CMasterCheck(0, TOUCH_PAD_DEVICE) == gmd_PASS)
			return (OK);
	}
	msg("--- TouchPad CheckDevice error ");
	return (ERR_BUSY);
}

BYTE Lpm_TouchPadReadByte(WORD W_RegAddr)
{
	BYTE B_Data,Ba_RegAddr[2];
    gmt_RET_STAT W_Result;

	Ba_RegAddr[0] = (BYTE)(W_RegAddr);

	if (Lpm_TouchPadCheckDevice() == ERR_BUSY)
		return (ERR_BUSY);
	
	W_Result = Lpm_I2CMasterReadAddrBuffer(TOUCH_PAD_DEVICE, Ba_RegAddr, 1, &B_Data, 1);

	if (W_Result == OK)
		return (B_Data);		
	else
		return (0);			
}

gmt_RET_STAT Lpm_TouchPadWriteByte(WORD W_RegAddr, BYTE B_Data)
{
	BYTE Ba_RegAddr[2];
	gmt_RET_STAT W_Result;

	Ba_RegAddr[0] = (BYTE)(W_RegAddr);			

	if (Lpm_TouchPadCheckDevice() == ERR_BUSY)		// Wait for device available
		return (ERR_BUSY);

	do
	{
		W_Result = Lpm_I2CMasterWriteAddrBuffer(TOUCH_PAD_DEVICE, Ba_RegAddr, 1, &B_Data, 1);
	}while (W_Result != OK);

	return (W_Result);							// Return status
}

#if 0
WORD Lpm_TouchPadReadWord(WORD W_RegAddr)
{
	BYTE B_Data[2],Ba_RegAddr[2];
    gmt_RET_STAT W_Result;

	Ba_RegAddr[0] = (BYTE)(W_RegAddr);

	if (Lpm_TouchPadCheckDevice() == ERR_BUSY)
		return (ERR_BUSY);
	
	W_Result = Lpm_I2CMasterReadAddrBuffer(TOUCH_PAD_DEVICE, Ba_RegAddr, 1, B_Data, 2);

	if (W_Result == OK)
		return *(WORD *)(B_Data);		
	else
		return (0);			
}

gmt_RET_STAT Lpm_TouchPadWriteWord(WORD W_RegAddr, WORD W_Data)
{
	BYTE Ba_RegAddr[2];
	BYTE B_Data[2];
	gmt_RET_STAT W_Result;

	Ba_RegAddr[0] = (BYTE)(W_RegAddr);
	B_Data[0] = (BYTE)(W_Data);
	B_Data[1] = (BYTE)(W_Data>>8);

	if (Lpm_TouchPadCheckDevice() == ERR_BUSY)		// Wait for device available
		return (ERR_BUSY);

	do
	{
		W_Result = Lpm_I2CMasterWriteAddrBuffer(TOUCH_PAD_DEVICE, Ba_RegAddr, 1, B_Data, 2);
	}while (W_Result != OK);

	return (W_Result);							// Return status
}
#endif

//Below 2 Funcition is to recongize horizonal or vertical slider bar 
void Lpm_ChangeTouchPadSliderDirectionToVertical(void)
{
	HorV_Flag = 0;
}

void Lpm_ChangeTouchPadSliderDirectionToHorizontal(void)
{
	HorV_Flag = 1;
}

void Lpm_TouchPadInit(void)
{
#if (TOUCHPAD_CONTROLLER == TPAD_STMPE16M31)

	// Set macro engine's clock to 2MHz 
	Lpm_TouchPadWriteByte(TPAD_SYSCON_1, 0);	

	// Set the sensor clock to 12.5KHz (2MHz/(2.5*64)); SCLK_DIV=64, PCLK_DIV=32KHz 
	Lpm_TouchPadWriteByte(TPAD_SYSCON_2, 0x68); 

	// Configure GPIO 7-13 as output for LED	  
	Lpm_TouchPadWriteByte(TPAD_GPIO_DIR, 0x80);
	Lpm_TouchPadWriteByte(TPAD_GPIO_DIR+1, 0x3F);

	// Configure GPIO 7-13 output to HIGH	
	Lpm_TouchPadWriteByte(TPAD_GPIO_SET_PIN, 0x80);
	Lpm_TouchPadWriteByte(TPAD_GPIO_SET_PIN+1, 0x3F);

	// Configure GPIO 5-13 as GPIO; 0-4 ,14-15 as capacitive sensors 
	Lpm_TouchPadWriteByte(TPAD_GPIO_AF, 0xE0); 
	Lpm_TouchPadWriteByte(TPAD_GPIO_AF+1, 0x3F); 

	// Configure GPIO 7-13 to be output of PWM 0-6 
	Lpm_TouchPadWriteByte(TPAD_GPIO7_PWM_CFG, 0x80);  
	Lpm_TouchPadWriteByte(TPAD_GPIO8_PWM_CFG, 0x81); 
	Lpm_TouchPadWriteByte(TPAD_GPIO9_PWM_CFG, 0x82); 
	Lpm_TouchPadWriteByte(TPAD_GPIO10_PWM_CFG, 0x83); 
	Lpm_TouchPadWriteByte(TPAD_GPIO11_PWM_CFG, 0x84); 
	Lpm_TouchPadWriteByte(TPAD_GPIO12_PWM_CFG, 0x85); 
	Lpm_TouchPadWriteByte(TPAD_GPIO13_PWM_CFG, 0x86); 

	// Set duty cycle (25% brightness) for PWM 0-6 
	Lpm_TouchPadWriteByte(TPAD_PWM_0_SET, 0x30);  
	Lpm_TouchPadWriteByte(TPAD_PWM_1_SET, 0x30); 
	Lpm_TouchPadWriteByte(TPAD_PWM_2_SET, 0x30); 
	Lpm_TouchPadWriteByte(TPAD_PWM_3_SET, 0x30); 
	Lpm_TouchPadWriteByte(TPAD_PWM_4_SET, 0x30); 
	Lpm_TouchPadWriteByte(TPAD_PWM_5_SET, 0x30); 
	Lpm_TouchPadWriteByte(TPAD_PWM_6_SET, 0x30); 

	// Turn off PWM channel 0-6 
	Lpm_TouchPadWriteByte(TPAD_PWM_MASTER_EN, 0x7F);

	// Configure the active capactive sensing channels  
	Lpm_TouchPadWriteByte(TPAD_CH_SEL, 0x1F);	//set sensing channel 0-4
	Lpm_TouchPadWriteByte(TPAD_CH_SEL+1, 0xC0);	//set sensing channel 14, 15


	// Set the Calibration Interval to 80ms  
	Lpm_TouchPadWriteByte(TPAD_CAL_INT, 0x01);	//1*10ms*8
	Lpm_TouchPadWriteByte(TPAD_CAL_MOD, 0x45);	//set auto-calibration in auto tuning with 80ms interval

	// Set MAF (Median Averaging Filter)  
	Lpm_TouchPadWriteByte(TPAD_MAF_SET, 0x05);	//Enable MAF, collect 20 samples, remove 2 samples

	// Set TVR (Touch Variance Register)  
	Lpm_TouchPadWriteByte(TPAD_DATA_TYPE, 0x01);
	Lpm_TouchPadWriteByte(TPAD_CH_DATA0, 0x05);
	Lpm_TouchPadWriteByte(TPAD_CH_DATA1, 0x05);
	Lpm_TouchPadWriteByte(TPAD_CH_DATA2, 0x05);
	Lpm_TouchPadWriteByte(TPAD_CH_DATA3, 0x05);
	Lpm_TouchPadWriteByte(TPAD_CH_DATA4, 0x05);
	Lpm_TouchPadWriteByte(TPAD_CH_DATA14, 0x05);
	Lpm_TouchPadWriteByte(TPAD_CH_DATA15, 0x05);

	// Set Ratio Engine  
	Lpm_TouchPadWriteByte(TPAD_RATIO_ENG_SET, 0x51); //Real-time position only data, slider mode, filter enabled, RE1
	Lpm_TouchPadWriteByte(TPAD_RATIO_ENG1_CFG, 0xED); //Slider starts at ch14, size=8

	// Set Touch Key Controller
	Lpm_TouchPadWriteByte(TPAD_KEY_FILT_CFG, 0x01); //Key filter1 uses 1-key mode
	Lpm_TouchPadWriteByte(TPAD_KEY_FILT_GROUP1, 0x1F); //Key filter1 gets data from ch0-4, 14-15
	Lpm_TouchPadWriteByte(TPAD_KEY_FILT_GROUP1+1, 0xC0);

	// Enable interrupt source from touchkey event & ration engine1 
	Lpm_TouchPadWriteByte(TPAD_INT_EN, 0x04);

	// Control the interrupt signal generation: Active Low, Level Trigger, Enable 
	Lpm_TouchPadWriteByte(TPAD_INT_CTRL, 0x01); 

	// Set LPM ext. interrupt input pin in LPM_GPIO0 to receive the notice from touchkey 
	gm_SetRegBitsWord(LPM_GPIO_ENABLE1,LPM_GPIO0_EN);	
	gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL1,LPM_GPIO0_IO);
	Lpm_InterruptEnable(gmd_EXTINT);

	// Clear the touch-key interrupt status 
	Lpm_TouchPadWriteByte(TPAD_INT_STA, 0x04);

	// Set EVR (Environment Variance Register)  
	Lpm_TouchPadWriteByte(TPAD_DATA_TYPE, 0x02);
	Lpm_TouchPadWriteByte(TPAD_CH_DATA0, 0x05);
	Lpm_TouchPadWriteByte(TPAD_CH_DATA1, 0x05);
	Lpm_TouchPadWriteByte(TPAD_CH_DATA2, 0x05);
	Lpm_TouchPadWriteByte(TPAD_CH_DATA3, 0x05);
	Lpm_TouchPadWriteByte(TPAD_CH_DATA4, 0x05);
	Lpm_TouchPadWriteByte(TPAD_CH_DATA14, 0x05);
	Lpm_TouchPadWriteByte(TPAD_CH_DATA15, 0x05);	

	// Configure the active capactive sensing channels   
	Lpm_TouchPadWriteByte(TPAD_CAP_SEN_CTRL, 0x85);	//enable sensing, HYS 8 steps, Execute auto-tune

	// Configure GPIO 7-13 output to LOW
	Lpm_TouchPadWriteByte(TPAD_GPIO_SET_PIN, 0);
	Lpm_TouchPadWriteByte(TPAD_GPIO_SET_PIN+1, 0);

	Lpm_ChangeTouchPadSliderDirectionToVertical();
	msg("----Touch Initial Ok----");


#elif (TOUCHPAD_CONTROLLER == TPAD_STMPE1208S)

	/* Configure all the DIOs as GPIO                 */
	Lpm_TouchPadWriteByte(GPIO_CONFIGURATION,0xFF);	
	Lpm_TouchPadWriteByte(GPIO_CONFIGURATION+1,0xFF);

	/* Configure all the GPIO as Output               */
	Lpm_TouchPadWriteByte(GPIO_DIRECTION,0x00);		
	Lpm_TouchPadWriteByte(GPIO_DIRECTION+1,0x00);

	/* Configure all the GPIO Output Values 	  */
	Lpm_TouchPadWriteByte(GPIO_REG,0x54);				
	Lpm_TouchPadWriteByte(GPIO_REG+1,0xF8);

	/* Set the Calibration Interval to high            */
	Lpm_TouchPadWriteByte(CALIBRATION_INTERVAL,0x01);	

	/* ETC Wait - Set all to  high  */
	Lpm_TouchPadWriteByte(ETC_WAIT,0x0FF);				

	/* Select AFS Mode 3  */
	Lpm_TouchPadWriteByte(FEATURE_SELECT,0x02);		

	/* Set the Environmental Variance - The value we generally suggest 1-4 */
	Lpm_TouchPadWriteByte(ENVIRONMENTAL_VARIANCE,0x02);

	/* Set the Environmental Variance  -  suggested value. */
	Lpm_TouchPadWriteByte(INTEGRATION_TIME,0x80);

	/* Set System Clock to 400Khz and Activate ETC */
	Lpm_TouchPadWriteByte(CONTROL_1,0x24);		

	/* TVR for Volume Slider - Pad 0 ~ 11  */
	Lpm_TouchPadWriteByte(TOUCH_VARIANCE,0x06);
	Lpm_TouchPadWriteByte(TOUCH_VARIANCE+1,0x06);
	Lpm_TouchPadWriteByte(TOUCH_VARIANCE+2,0x06);
	Lpm_TouchPadWriteByte(TOUCH_VARIANCE+3,0x06);
	Lpm_TouchPadWriteByte(TOUCH_VARIANCE+4,0x06);
	Lpm_TouchPadWriteByte(TOUCH_VARIANCE+5,0x06);
	Lpm_TouchPadWriteByte(TOUCH_VARIANCE+6,0x06);
	Lpm_TouchPadWriteByte(TOUCH_VARIANCE+7,0x06);
	Lpm_TouchPadWriteByte(TOUCH_VARIANCE+8,0x010);
	Lpm_TouchPadWriteByte(TOUCH_VARIANCE+9,0x010);
	Lpm_TouchPadWriteByte(TOUCH_VARIANCE+10,0x10);
	Lpm_TouchPadWriteByte(TOUCH_VARIANCE+11,0x10);

	/* Threshold for AFS mode to detect touch  */
	Lpm_TouchPadWriteByte(STRENGTH_THRESHOLD,0x70);
	Lpm_TouchPadWriteByte(STRENGTH_THRESHOLD+1,0x70);
	Lpm_TouchPadWriteByte(STRENGTH_THRESHOLD+2,0x70);
	Lpm_TouchPadWriteByte(STRENGTH_THRESHOLD+3,0x70);
	Lpm_TouchPadWriteByte(STRENGTH_THRESHOLD+4,0x70);
	Lpm_TouchPadWriteByte(STRENGTH_THRESHOLD+5,0x70);
	Lpm_TouchPadWriteByte(STRENGTH_THRESHOLD+6,0x70);
	Lpm_TouchPadWriteByte(STRENGTH_THRESHOLD+7,0x70);
	Lpm_TouchPadWriteByte(STRENGTH_THRESHOLD+8,0x70);
	Lpm_TouchPadWriteByte(STRENGTH_THRESHOLD+9,0x70);
	Lpm_TouchPadWriteByte(STRENGTH_THRESHOLD+10,0x70);
	Lpm_TouchPadWriteByte(STRENGTH_THRESHOLD+11,0x70);

	Lpm_ChangeTouchPadSliderDirectionToVertical();
	msg("----Touch Initial Ok----");
#endif	
}

BYTE Lpm_IsWheelRightKey(void)
{
	BYTE i=0,result=FALSE;
	
	for(i=0;i<24;i++)
	{
		if((WheelBuffer[0]==TouchPadWheelValue[i][0]) && (WheelBuffer[1]==TouchPadWheelValue[i][1])&&(WheelBuffer[2]==TouchPadWheelValue[i][2]))
		{
			result = TRUE;	
			break;		
		}
	}

 	if(result == TRUE)
	{	
		if(HorV_Flag == 0)
		{
			msg("--U--");
			return Key_KEY_PHYSICAL_UP;
		}			
		else
		{
			msg("--L--");
			return Key_KEY_PHYSICAL_LEFT;
		} 	
	}
	else
		return Key_None;
	
}

BYTE Lpm_IsWheelLeftKey(void)
{
	BYTE i=0,result=FALSE;
	
	for(i=0;i<24;i++)
	{
		if((WheelBuffer[0]==TouchPadWheelValue[i][2]) && (WheelBuffer[1]==TouchPadWheelValue[i][1]) && (WheelBuffer[2]==TouchPadWheelValue[i][0]) )
		{
			result = TRUE;	
			break;
		}	
	}

 	if(result == TRUE)
	{	
		if(HorV_Flag == 0)
		{
			msg("--D--");
			return Key_KEY_PHYSICAL_DOWN;
		}			
		else
		{
			msg("--R--");
			return Key_KEY_PHYSICAL_RIGHT;
		}
 	}
	else
		return Key_None;

}

void Lpm_TouchPadHandler(void)
{
#if (TOUCHPAD_CONTROLLER == TPAD_STMPE16M31)

	BYTE TPadDetVal[2];
	KEY_INDEX B_Key;

/*
	if (gm_ScanAdcKey(1)==0xdf)
		msg("-----debug=0x%x",Global_debug);
		
 	msg("-----debug=0x%d",gm_W_GetSystemTime());		
 */
 
	// STMPE1208S generate a High TINT(Touch Interrupt) to notice system
	// touch pad detect finger touch, 
	// After Read TOUCH_BYTE , TINT back to Low 
	// LowValue is 4 key wheel , HighValue is 8 key wheel 
	// 
 	//if(Lpm_IoExpanderGetBitWord(IO_EXP_PCF8575, IO_EXP_P01) == 0x2)
 	//if(gm_ReadRegWord(LPM_IRQ_STATUS) & LPM_EXTINT_STATUS)	//Lpm Ext-Int is detected
 	if(TouchKeyIntFlag)
   	{
		if(Lpm_TouchPadReadByte(TPAD_INT_STA) & BIT0)	//Ratio Engine1 event is detected
		{
			//Process Ratio Engine1
		}
		if(Lpm_TouchPadReadByte(TPAD_INT_STA) & BIT2)	//Touch-key event is detected
		{
			TPadDetVal[0] = Lpm_TouchPadReadByte(TPAD_TOUCH_DET);
			TPadDetVal[1] = Lpm_TouchPadReadByte(TPAD_TOUCH_DET+1);

			if(TPadDetVal[0])
			{
				WheelBuffer[2] = WheelBuffer[1];
				WheelBuffer[1] = WheelBuffer[0];
				WheelBuffer[0] = TPadDetVal[0];			
				
				B_Key = Lpm_IsWheelRightKey();
				if(B_Key != Key_None)
				{
					Lpm_TouchPadKeyProcess(B_Key); 
					return;
				}	
				B_Key = Lpm_IsWheelLeftKey();
				if(B_Key != Key_None)
				{
					Lpm_TouchPadKeyProcess(B_Key);
					return;
				}
			}

			if(TPadDetVal[1])
			{
				if (TPadDetVal[1] == TOUCHKEY_EXIT)
					Lpm_TouchPadKeyProcess(Key_KEY_PHYSICAL_EXIT);	
				else if (TPadDetVal[1] == TOUCHKEY_MENU) 
					Lpm_TouchPadKeyProcess(Key_KEY_PHYSICAL_MENU);	
				else if (TPadDetVal[1] == TOUCHKEY_OK) 			
					Lpm_TouchPadKeyProcess(Key_KEY_PHYSICAL_OK);	
				return;
			}

			//Important : 	Key_None is produce a key to reconigize "stop" signal onWB 
			Lpm_TouchPadKeyProcess(Key_None);	

			Lpm_TouchPadWriteByte(TPAD_INT_STA, 0x04);	//Clear the touch-key interrupt status
		}
		
		//gm_WriteRegWord(LPM_IRQ_STATUS, LPM_EXTINT_STATUS);
		TouchKeyIntFlag = 0;
	}

#elif (TOUCHPAD_CONTROLLER == TPAD_STMPE1208S)

	BYTE HighValue,LowValue;
	KEY_INDEX B_Key;

/*
	if (gm_ScanAdcKey(1)==0xdf)
		msg("-----debug=0x%x",Global_debug);
		
 	msg("-----debug=0x%d",gm_W_GetSystemTime());		
 */
 
	// STMPE1208S generate a High TINT(Touch Interrupt) to notice system
	// touch pad detect finger touch, 
	// After Read TOUCH_BYTE , TINT back to Low 
	// LowValue is 4 key wheel , HighValue is 8 key wheel 
	// 
 	if(Lpm_IoExpanderGetBitWord(IO_EXP_PCF8575, IO_EXP_P01) == 0x2)
   	{
		LowValue  = Lpm_TouchPadReadByte(TOUCH_BYTE);
		HighValue = Lpm_TouchPadReadByte(TOUCH_BYTE+1);		

		if(LowValue)
		{
			WheelBuffer[2] = WheelBuffer[1];
			WheelBuffer[1] = WheelBuffer[0];
			WheelBuffer[0] = LowValue;			
			
			B_Key = Lpm_IsWheelRightKey();
			if(B_Key != Key_None)
			{
				Lpm_TouchPadKeyProcess(B_Key); 
				return;				
			}	
			B_Key = Lpm_IsWheelLeftKey();
			if(B_Key != Key_None)
			{
				Lpm_TouchPadKeyProcess(B_Key); 
				return;
			}
		}

		if(HighValue)
		{
			if (HighValue == TOUCHKEY_EXIT)
				Lpm_TouchPadKeyProcess(Key_KEY_PHYSICAL_EXIT);	
			else if (HighValue == TOUCHKEY_MENU) 
				Lpm_TouchPadKeyProcess(Key_KEY_PHYSICAL_MENU);	
			else if (HighValue == TOUCHKEY_OK) 			
				Lpm_TouchPadKeyProcess(Key_KEY_PHYSICAL_OK);	
			return;
		}

		//Important : 	Key_None is produce a key to reconigize "stop" signal onWB 
		Lpm_TouchPadKeyProcess(Key_None);	
	}

#endif	
}

void Lpm_TouchPadKeyProcess(KEY_EVENTS KeyEvent)
{
	if (IsMissionPoweredUp())
	{
		gm_MessageSend(gmd_MESSAGE_TOUCHKEY, &KeyEvent, 1);
	}
	#ifdef POWER_MODE_HANDLE
	else	//Mission power down
	{
		if(KeyEvent == Key_KEY_PHYSICAL_POWER)
		{		
			powerUpRequest = TRUE;
			Lpm_PowerModeSetEvent2PwrHdr(LPM_EVNT2PWR_PWRKEY);
		}
	}
	#endif
}

/*WORD Lpm_IoExpanderGetBitWord(BYTE B_Addr, WORD W_Bit)
{
	WORD* pw_data = &(W_TPadData[(B_Addr & 0x0E)>>1]);
	
	*pw_data |= W_Bit;
	if (Lpm_I2CMasterWriteBlock(0, (B_Addr&0xFE) , (BYTE *)pw_data, 2, TRUE) != gmd_PASS)
		msg("Fail to Write - IO - Addr 0x%x",0);
	if (Lpm_I2CMasterReadBlock(0, (B_Addr&0xFE), (BYTE *)pw_data, 2, TRUE) != gmd_PASS)
		msg("Fail to Read - IO - Addr 0x%x",0);
	return ((*pw_data) & W_Bit);
}*/

#endif
//**********************************  END  ************************************* 
