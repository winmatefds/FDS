/*
	$Workfile:   ACR_Handler.h  $
	$Revision: 1.8 $
	$Date: 2013/02/05 07:12:46 $
*/

#ifndef __ACR_Handler_H__
#define __ACR_Handler_H__

//*****************************************************************************************
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
//
//	Adaptive Contrast Ratio
// Default: Get Luminance data from Pixel grab.
//	Second solution: Use ACC, if pixel grab used for VWD.
//	Limitation: If system can not use ACC and pixel grab already used for VWD, it will no ACR anymore.
//
//*****************************************************************************************

//******************************************************************************
//  G L O B A L    D E F I N I T I O N S
//******************************************************************************
#define INVALID_RESULT           0xFFFF

//******************************************************************************
//  L O C A L    D E F I N I T I O N S
//******************************************************************************
typedef enum AcrHandlerStateEnum
{
   ACR_CALCULATE_PWM,
   ACR_ADJUST_PWM,
   ACR_NO_ACTION
} AcrHandlerState_t;

//******************************************************************************
//  F U N C T I O N    P R O T O T Y P E S
//******************************************************************************
WORD GetCurrentAcrResult(void);
void ACR_Init(void);
void ACR_Enable(void);
void ACR_Disable(void);
void ACR_Handler(void);

#endif

