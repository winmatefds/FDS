/*
	$Workfile:   ADCKeypad.h  $
	$Revision: 1.3 $
	$Date: 2011/12/13 10:32:44 $
*/


#ifndef __ADCKEYPAD_H__
#define __ADCKEYPAD_H__
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
// MODULE:      ADCKeypad.h
//
// USAGE:	Header file for Low-bandwidth ADC Keypad
//
//******************************************************************************
//******************************************************************************
//  G L O B A L    V A R I A B L E S
//******************************************************************************

//******************************************************************************
//	D E F I N I T I O N
//******************************************************************************

#define NUM_KEYPAD_CH	4 // 3
extern BYTE B_AdcKeyData[NUM_KEYPAD_CH];
extern BYTE B_AdcData[NUM_KEYPAD_CH];
//******************************************************************************
//  F U N C T I O N    P R O T O T Y P E S
//******************************************************************************
BYTE ScanNextAdcKey (void);

#endif
typedef enum
{
   KEYPAD_LED_OFF,
   KEYPAD_LED_RED,
   KEYPAD_LED_GREEN,
   KEYPAD_LED_ORANGE,
}gmt_LED_COLOR;

typedef enum
{
   LED_NORMAL,
   LED_BLINKING
}gmt_LED_ATTRIB;

typedef enum
{
   LED_ON,
   LED_OFF
}gmt_LED_STATUS;

void KeypadLightControl(gmt_LED_COLOR B_Color, gmt_LED_ATTRIB B_Attrib);
void BlinkingHandler(void);
//*********************************  END  **************************************


