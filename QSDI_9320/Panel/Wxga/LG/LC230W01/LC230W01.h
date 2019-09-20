/*
	$Workfile:   LC230W01.h  $
	$Revision: 1.4 $
	$Date: 2011/12/15 06:22:27 $
*/

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
// MODULE:      LC230W01.h    
//
// USAGE:       Header file containing the LC230W01 panel specifications
//
//******************************************************************************

#ifdef INCLUDE_LG_WXGA_LC230W01

#include "..\..\Panel\Wxga\LG\LC230W01\OverdriveLut.h"
#include "..\..\Panel\Wxga\LG\LC230W01\GammaLut.h"

char ROM PanelName_WXGA_LC230W01[] = "LG_LC230_W01";

ROM WORD sRGB2rgb_LC230W01[9] =
{
	0x1000, 0x0000, 0x0000,
	0x0000, 0x1000, 0x0000,
	0x0000, 0x0000, 0x1000
};

ROM gmt_PanelData Panel_LG_WXGA_LC230W01 =
{
	PanelName_WXGA_LC230W01,			//;
	LG_WXGA_LC230W01,	// Panel ID (refer to enum defined in panel.h)	
	PANEL_WXGA,			// gmd_PanelType PanelType;
	FORMAT_ORG_2D,    // gmd_PanelDataFormat;
	ONE,				//BYTE PixPerClock;
	8,					//BYTE  Depth;
	1280,				//WORD  Width;
	768,					//WORD  Height;
	63,					//BYTE  MaxVFreq;
	47,					//BYTE  MinVFreq;
	82000L,				//DWORD MaxPClk; 		 // KHz
	1336,				//WORD W_MinHTotal
	1602,				//W_TypHTotal
	// TODO: UPDATE 22 and 33 H/V and VTypical totals with spreadsheet
	1958,				//W_22HTotal		
	1370,				//W_33HTotal
	8,					//BYTE  MinHSyncWidth;
	8,					//BYTE  MinHSyncBackPorch;
	848,					//W_22VTotal				
	808,					//W_33VTotal	
	846,				//WORD  MaxVtotal;
	788,					//WORD  MinVtotal;
	812,					//WORD TypVTotal;
	2,					//BYTE  MinVSyncWidth;	
	35,					//BYTE  MinVSyncBackPorch;	
	
	0x9696,				//WORD  PowerUpTiming;
	0xffff,				//WORD  PowerDownTiming;
	0xC,					//WORD AccToPixelVal;
	0x0,					//BYTE  SpreadSpectrumCtrl;		// Value for Spread_Spectrum_Control register
	0,					//BYTE  DClkDelay;		 		// DCLK display timing delay adjustment in ns unit.
	Wa_OverDriveLUT_LC230W01, //OverDriveLut_LC230W01,
	sRGB2rgb_LC230W01,
	Sta_GammaLUT_LC230W01,
	0,					//WORD  Invert_DVS 				: 1;
	0,					//WORD  Invert_DHS 				: 1;
	0,					//WORD  Invert_DCLK 			: 1;
	0,					//WORD  Invert_DEN 				: 1;
	
	0xff,				//WORD  DportSwap				: 1;
	TRUE,				//WORD  LVDSInterface			: 1;
	SINGLE_BUS,			//WORD  W_LVDS_BusType 			: 1;	
	LVDS_EIGHT_BIT_MODE,		//WORD  LvdsPanelMode	: 2; 
	0xff,				//WORD  Spread_Spect_En 		: 1; // 0xFF : Enable spectrum, 0x00 : Disable 
	SWAP,				//WORD	LvdsPosNegSwap			: 1;
	0,      			//WORD  PadDrive;
	FIVE_PT_SIX_MA,		//WORD  W_LvdsCurrentCtrl		: 2;			
	OUT_24_48_BIT,		//WORD  W_BitOutput				: 1;
	STPANEL_LVDS_MAP_STANDARD1,			// WORD W_LVDSSignalMap
	0x0063,				//WORD  W_LvdsClkData
};
extern ROM gmt_PanelData Panel_LG_WXGA_LC230W01;

#else
	#define Panel_LG_WXGA_LC230W01	(ROM gmt_PanelData *)PanelArray
	//
	// If panel file is not included in the project, but the panel is selected to be
	// the default panel during bootup, issue warning message
	//
	#if (DEFAULT_PANEL == LG_WXGA_LC230W01)
		#pragma message "Default panel will not bootup unless INCLUDE_LG_WXGA_LC230W01 is defined!"
	#endif
#endif
