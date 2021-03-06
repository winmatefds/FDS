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
// MODULE:      CUSTOMER_1080I_DVIOUT.h
//
// USAGE:       Header file containing the Customer panel (1920x1080) specifications
//
//******************************************************************************

#ifdef INCLUDE_CUSTOMER_1080I_DVIOUT

#include "..\..\Panel\Interlaced\DVIOut\Customer_1080iDviOut\OverdriveLut.h"
#include "..\..\Panel\Interlaced\DVIOut\Customer_1080iDviOut\GammaLut.h"

char ROM PanelName_CUSTOMER_1080I_DVIOUT[] = "CUSTOMER_1080I_DVIOUT";


ROM WORD sRGB2rgb_CUSTOMER_1080I_DVIOUT[9] =
{
	0x1000, 0x0000, 0x0000,
	0x0000, 0x1000, 0x0000,
	0x0000, 0x0000, 0x1000
};								

ROM gmt_PanelData Panel_CUSTOMER_1080I_DVIOUT =
{
	PanelName_CUSTOMER_1080I_DVIOUT,			//;
	CUSTOMER_1080I_DVIOUT,	// Panel ID (refer to enum defined in panel.h)	
	PANEL_1080I,							// gmd_PanelType PanelType
	FORMAT_ORG_2D,    // gmd_PanelDataFormat;
	ONE,								// BYTE PixPerClock
	8,									// BYTE  Depth
	1920,								// WORD  Width
	1080,								// WORD  Height
	78,									// BYTE  MaxVFreq
	46,									// BYTE  MinVFreq
	80000L,				//DWORD MaxPClk; 		 // KHz
	2000,				//WORD W_MinHTotal
	2200,				//W_TypHTotal
	2200,				//W_22HTotal		
	2200,				//W_33HTotal
	44,  				//BYTE  MinHSyncWidth;
	148, //192,				//BYTE  MinHSyncBackPorch;  vvs
	1125,				//W_22VTotal				
	1125,				//W_33VTotal 
 	1125,				//WORD  MaxVtotal;
	1125,				//WORD  MinVtotal;
	1125,				//WORD TypVTotal;
	10, //5					//BYTE  MinVSyncWidth;	
	30, //15					//BYTE  MinVSyncBackPorch;	
	
	0x9696,				//WORD  PowerUpTiming;
	0xffff,				//WORD  PowerDownTiming;
	0xF,				//WORD AccToPixelVal;
	0x0,								// BYTE  SpreadSpectrumCtrl
	0,									// DCLK display timing delay adjustment in ns unit
	Wa_OverDriveLUT_CUSTOMER_1080I_DVIOUT,	//OverDriveLut_CUSTOMER_1080P_DVIOUT,
	sRGB2rgb_CUSTOMER_1080I_DVIOUT,
	Sta_GammaLUT_CUSTOMER_1080I_DVIOUT,
	0,					//WORD  Invert_DVS 				: 1;
	0,//1,					//WORD  Invert_DHS 				: 1;
	0,					//WORD  Invert_DCLK 			: 1;
	0,					//WORD  Invert_DEN 				: 1;
	0xff,				//WORD  DportSwap				: 1;
	FALSE,				//WORD  LVDSInterface			: 1;
	SINGLE_BUS,			//WORD  W_LVDS_BusType 			: 1;		
	NO_LVDS,			//WORD  LvdsPanelMode				: 2; 
	0xff,				//WORD  Spread_Spect_En 		: 1; // 0xFF : Enable spectrum, 0x00 : Disable 
	SWAP,				//WORD	LvdsPosNegSwap			: 1;
	0,      			//WORD  PadDrive;
	FIVE_PT_SIX_MA,		//WORD  W_LvdsCurrentCtrl		: 2;			
	OUT_24_48_BIT,		//WORD  W_BitOutput				: 1;
	STPANEL_LVDS_MAP_STANDARD1,			// WORD W_LVDSSignalMap
	0x0063,				//WORD  W_LvdsClkData
};
extern ROM gmt_PanelData Panel_CUSTOMER_1080I_DVIOUT;

#else
	#define Panel_CUSTOMER_1080I_DVIOUT (ROM gmt_PanelData *)PanelArray
	//
	// If panel file is not included in the project, but the panel is selected to be
	// the default panel during bootup, issue warning message
	//
	#if (DEFAULT_PANEL == CUSTOMER_1080I_DVIOUT)
		#pragma message "Default panel will not bootup unless INCLUDE_CUSTOMER_1080I_DVIOUT is defined!"
	#endif
#endif
