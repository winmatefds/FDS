/*
	$Workfile:   gm_IFM.h  $
	$Revision: 1.2 $
	$Date: 2011/12/14 09:20:57 $
*/
#ifndef __GM_IFM_H__
#define __GM_IFM_H__
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
// MODULE:		gm_IFM.h
//
// USAGE:		This module contains prototypes and definitions for IFM blocks
//				interface
//
//******************************************************************************

//******************************************************************************
//  E X T E R N S
//******************************************************************************

//******************************************************************************
//  T Y P E D E F S
//******************************************************************************
#ifndef __GM_STDLIB_H__

// NOTE: 	Below structures are currently defined in gm_StdLib.h. In order to 
// 			better keep track of their usage, they are defined here for reference 
//			as well.

//
// IFM enumeration
//
typedef enum
{
	MIFM = 0,					// Main IFM (associated with CH_A)
	PIFM,						// Pip  IFM (associated with CH_B)
#if	(FEATURE_IFMC == ENABLE)	
	IFMC,						// Floating IFM 
#endif	
	IFM_NUM,					// Number Of IFMs
} gmt_PHY_IFM;

//
// IFM Measurement Struct
//
typedef struct
{
	WORD			W_VTotal;
	WORD			W_VsyncPulse;
	WORD			W_HsyncPulse; 
	WORD			W_HFreq;
	WORD			W_VFreq;
	gmt_INPUT_FLAGS	W_DetectFlags;
} gmt_InputMeasurementStruct;

//
// IFM Control Field enum
//
typedef enum
{
	gmd_IFM_ENABLE,
	gmd_IFM_NEW_MEASURE_FROM_NEXT_VS,
	gmd_IFM_OFFSET_HSYNC,
	gmd_IFM_SELECT_FIELD,
	gmd_IFM_MEASURE_SELECTED_FIELD_ONLY,
	gmd_IFM_INT_GEN_ODD_EVEN_FIELD_ID,
	gmd_IFM_CSYNC_MEASUREMENT,
	gmd_IFM_INVERT_ODD_EVEN_FIELD_ID,
	gmd_IFM_FIELD_DETECT_MODE,
	gmd_IFM_MCLK_PER_FRAME_MEASURE,
	gmd_IFM_COUNT_MCLK_BETWEEN_VS,
	gmd_IFM_TOGGLE_ODD_EVERY_FIELD,
	NUM_OF_IFM_BIT_FIELDS,
} gmt_IFMCtrlBitFieldType;

#endif //__GM_STDLIB_H__

//******************************************************************************
//  F U N C T I O N S   P R O T O T Y P E S 
//******************************************************************************

void gm_ReadIFMTiming(gmt_PHY_IFM Ifm, gmt_InputMeasurementStruct* IfmStr);
void gm_SetupIFMCtrl 				(gmt_PHY_IFM Ifm, WORD W_IfmCtrl);
void gm_SetupIFMHLine 				(gmt_PHY_IFM Ifm, WORD W_IfmHLine);
void gm_SetupIFMWatchdog 			(gmt_PHY_IFM Ifm, WORD W_IfmWd);
void gm_SetupIFMChange 				(gmt_PHY_IFM Ifm, WORD W_IfmChange);
void gm_SetInvertIFMOddEven 		(gmt_PHY_IFM Ifm);
void gm_ClearInvertIFMOddEven 		(gmt_PHY_IFM Ifm);
void gm_SetIFMFieldSelect 			(gmt_PHY_IFM Ifm);
void gm_ClearIFMFieldSelect 		(gmt_PHY_IFM Ifm);
void gm_EnableIntGenOddEvenFiledId 	(gmt_PHY_IFM Ifm);
void gm_DisableIntGenOddEvenFiledId (gmt_PHY_IFM Ifm);

void gm_SetIFMCtrlBitField(gmt_PHY_IFM Ifm, gmt_IFMCtrlBitFieldType Type, BOOL Bt_State);
#endif
//*********************************  END  **************************************

