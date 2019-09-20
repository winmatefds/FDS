/*
   $Workfile:   Lpm_Keypad.h  $
   $Revision: 1.9 $
   $Date: 2011/12/15 01:43:01 $
*/
#ifndef __LPM_KEYPAD_H__
#define __LPM_KEYPAD_H__

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
// MODULE:      Lpm_Keypad.h
//
// USAGE:       
//
//******************************************************************************

typedef enum
{
   Key_CH_0,
   Key_CH_1,
   Key_CH_NUM
}gmt_KeyChannel;

typedef enum
{
   Key_NONE,         // 0
   Key_POWER,        // 1
	Key_MENU,         // 2
	Key_ENTER,        // 3
	Key_KEYTONE,      // 4
	Key_AUTO,         // 5
	Key_SWITCH,       // 6
	Key_PIP,          // 7
	Key_SOURCE,       // 8
	Key_UP,           // 9
	Key_DOWN,         // 10
	Key_RIGHT,        // 11
	Key_LEFT,         // 12
	Key_CHANNEL_UP,   // 13
	Key_CHANNEL_DOWN, // 14
	Key_VOLUME_UP,    // 15
	Key_VOLUME_DOWN,  // 16
	Key_NUM           // 17
}gmt_gpADCKeys;


void Lpm_KeypadHandler(void);
void Lpm_KeypadInit(void);
void Lpm_KeypadLbwAdcInit(BYTE timeOut,BYTE treshold, BYTE numChannels, BYTE offChannel, BYTE offValue);
gmt_gpADCKeys Lpm_KeypadGetKey(BYTE AdcCh, BYTE AdcVal);


#endif //__LPM_KEYPAD_H__

