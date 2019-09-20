#ifndef __LPM_TOUCHPAD_H__
#define __LPM_TOUCHPAD_H__
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
//==============================================================================
//
// MODULE:      Lpm_TouchPad.h
//
// USAGE:       
//
//******************************************************************************

#ifdef TOUCHPAD_SUPPORT

//******************************************************************************
//  G L O B A L    D E F I N I T I O N S
//******************************************************************************
/*
#define IO_EXP_PCF8575		0x48
#define IO_EXP_PCF8575_4C	0x4C

#define IO_EXP_P00  BIT0
#define IO_EXP_P01  BIT1
#define IO_EXP_P02  BIT2
#define IO_EXP_P03  BIT3
#define IO_EXP_P04  BIT4
#define IO_EXP_P05  BIT5
#define IO_EXP_P06  BIT6
#define IO_EXP_P07  BIT7

#define IO_EXP_P10  BIT8
#define IO_EXP_P11  BIT9
#define IO_EXP_P12  BIT10
#define IO_EXP_P13  BIT11
#define IO_EXP_P14  BIT12
#define IO_EXP_P15  BIT13
#define IO_EXP_P16  BIT14
#define IO_EXP_P17  BIT15
*/

#define TOUCH_PAD_DEVICE		0xb0        // (0x58<<1), I2C device address: 0x58-0x5B 
#define DEVICE_CHECK_LIMIT		255		// number of times to check

#define TOUCHKEY_EXIT		0x01
#define TOUCHKEY_MENU		0x02
#define TOUCHKEY_OK		0x04


typedef BYTE KEY_INDEX;
typedef WORD GPIO_KEY_VALUE;
typedef WORD KEY_VALUE;

typedef enum 
{
	Key_None,
	Key_KEY_PHYSICAL_CHANNEL_UP,
	Key_KEY_PHYSICAL_CHANNEL_DOWN,
	Key_KEY_PHYSICAL_SOURCE,
	Key_KEY_PHYSICAL_PIP_SOURCE,
	Key_KEY_PHYSICAL_SWAP,
	Key_KEY_PHYSICAL_TTX_CC,
	Key_KEY_PHYSICAL_POWER,
	Key_KEY_PHYSICAL_INFO,
	Key_KEY_PHYSICAL_MUTE,
	Key_KEY_PHYSICAL_FREEZE,//10
	Key_ADC_1_AUTO,
	Key_ADC_1_KEYSTONE,
	Key_KEY_PHYSICAL_MENU,//13
	Key_KEY_PHYSICAL_SCAN,
	Key_KEY_PHYSICAL_UP,
	Key_KEY_PHYSICAL_DOWN,
	Key_KEY_PHYSICAL_LEFT,
	Key_KEY_PHYSICAL_RIGHT,
	Key_KEY_PHYSICAL_OK,
	Key_KEY_PHYSICAL_SLEEP,//20
	Key_KEY_PHYSICAL_EXIT,
	Key_KEY_PHYSICAL_TTX_RED,
	Key_KEY_PHYSICAL_TTX_YELLOW,
	Key_KEY_PHYSICAL_TTX_CYAN,
	Key_KEY_PHYSICAL_SPLIT,
	Key_KEY_PHYSICAL_PIP,
	Key_KEY_PHYSICAL_PIP_POSITION,
	Key_KEY_PHYSICAL_PIP_SIZE,
	Key_KEY_PHYSICAL_ZERO,
	Key_KEY_PHYSICAL_ONE,//30
	Key_KEY_PHYSICAL_TWO,
	Key_KEY_PHYSICAL_THREE,
	Key_KEY_PHYSICAL_FOUR,
	Key_KEY_PHYSICAL_FIVE,
	Key_KEY_PHYSICAL_SIX,
	Key_KEY_PHYSICAL_SEVEN,
	Key_KEY_PHYSICAL_EIGHT,
	Key_KEY_PHYSICAL_NINE,
	Key_KEY_PHYSICAL_ENTER_JUMP,
	Key_KEY_PHYSICAL_ASPECT,//40
	Key_KEY_PHYSICAL_PIP_ASPECTRATIO,
	Key_KEY_PHYSICAL_VOLUME_UP,
	Key_KEY_PHYSICAL_VOLUME_DOWN,
	Key_KEY_PHYSICAL_EFFECT,
	Key_KEY_PHYSICAL_TTX_GREEN,
	Key_KEY_PHYSICAL_INDEX,
	Key_KEY_PHYSICAL_SUB,
	Key_KEY_PHYSICAL_SIZE,
	Key_KEY_PHYSICAL_REVEAL,
	Key_KEY_PHYSICAL_HOLD,
	Key_KEY_PHYSICAL_CANCEL,
	Key_KEY_PHYSICAL_MODE,
	Key_KEY_PHYSICAL_AUDIO,//53
	Key_KEY_NUMBER
} KEY_EVENTS;

typedef enum 
{
	TPAD_STMPE1208S,
	TPAD_STMPE16M31
}Touchpad_Controller;

//******************************************************************************
//	G L O B A L    F U N C T I O N    P R O T O T Y P E S
//******************************************************************************
gmt_RET_STAT Lpm_TouchPadCheckDevice(void);
BYTE	 	 Lpm_TouchPadReadByte(WORD W_RegAddr);
gmt_RET_STAT Lpm_TouchPadWriteByte(WORD W_RegAddr, BYTE B_Data);
BYTE	 	 Lpm_TouchPadReadWord(WORD W_RegAddr);
gmt_RET_STAT Lpm_TouchPadWriteWord(WORD W_RegAddr, WORD W_Data);
void Lpm_TouchPadInit(void);
void Lpm_TouchPadHandler(void);
void Lpm_ChangeTouchPadSliderDirectionToVertical(void);
void Lpm_ChangeTouchPadSliderDirectionToHorizontal(void);
void Lpm_TouchPadSendKeyEvent(KEY_EVENTS KeyEvent);
void Lpm_TouchPadKeyProcess(KEY_EVENTS KeyEvent);
//WORD Lpm_IoExpanderGetBitWord(BYTE B_Addr, WORD W_Bit);
#endif
//**********************************  END  *************************************
#endif //__LPM_TOUCHPAD_H__