/*
	$Workfile:   CMO_V562D1_L03.h  $
	$Revision: 1.1 $
	$Date: 2012/02/20 01:53:30 $
*/

#ifdef INCLUDE_CMO_2K2K_V562D1_L03

#include "..\..\Panel\2K2K\CMO\CMO_V562D1_L03\OverdriveLut.h"
#include "..\..\Panel\2K2K\CMO\CMO_V562D1_L03\GammaLut.h"


char ROM PanelName_CMO_V562D1_L03[] = "CMO_V562D1_L03";

ROM WORD sRGB2rgb_CMO_V562D1_L03[9] =
{
	0x1000, 0x0000, 0x0000,
	0x0000, 0x1000, 0x0000,
	0x0000, 0x0000, 0x1000
};

ROM gmt_PanelData Panel_2K2K_CMO_V562D1_L03 =
{
	PanelName_CMO_V562D1_L03,
	CMO_V562D1_L03,						// Panel ID (refer to enum defined in panel.h)
	PANEL_2K2K,							// gmd_PanelType PanelType
	FORMAT_ORG_2D,    					// gmd_PanelDataFormat;
	FOUR,								// BYTE PixPerClock
	8,									// BYTE  Depth
	1920,								// WORD  Width
	2160,								// WORD  Height
	60,									// BYTE  MaxVFreq
	57,									// BYTE  MinVFreq
	288000L,							// DWORD MaxPClk (KHz)  
	2200,								// WORD W_MinHTotal
	2200,								// W_TypHTotal
	2200,								// W_22HTotal
	2200,								// W_33HTotal
	32,//40,									// BYTE  MinHSyncWidth
	80,//110, 								// BYTE  MinHSyncBackPorch
	2200,								// W_22VTotal
	2200,								// W_33VTotal
	2300,								// WORD  MaxVtotal
	2200,								// WORD  MinVtotal
	2200,								// WORD TypVTotal
	3,									// BYTE  MinVSyncWidth
	7,									// BYTE  MinVSyncBackPorch
	0x1101,								// WORD  PowerUpTiming
	0x1101,								// WORD  PowerDownTiming
	0x21,								// WORD AccToPixelVal
	0x0,								// BYTE  SpreadSpectrumCtrl
	0,									// DCLK display timing delay adjustment in ns unit
	Wa_OverDriveLUT_CMO_V562D1_L03,		// Overdrive look up table
	sRGB2rgb_CMO_V562D1_L03,
	Sta_GammaLUT_CMO_V562D1_L03,
	0,									// WORD  Invert_DVS
	0,									// WORD  Invert_DHS
	0,									// WORD  Invert_DCLK
	0,									// WORD  Invert_DEN
	
	0xFF,								// WORD  DportSwap
	TRUE,								// WORD  LVDSInterface
	QUAD_BUS,							// WORD  W_LVDS_BusType
	LVDS_EIGHT_BIT_MODE,				// WORD  LvdsPanelMode
	0xFF,								// WORD  Spread_Spect_En
	NO_SWAP,							// WORD	LvdsPosNegSwap
	STPLF_DISPLAY_PAD_DRIVE_1_25,		// WORD PadDrive	
	STPLF_DISPLAY_LVDS_CURRENT_3_5,		// WORD  W_LvdsCurrentCtrl
	OUT_24_48_BIT,						// WORD  W_BitOutput
	STPANEL_LVDS_MAP_STANDARD2,			// WORD W_LVDSSignalMap
	0x0063,								// WORD  W_LvdsClkData
};

extern ROM gmt_PanelData Panel_2K2K_CMO_V562D1_L03;

#else
	#define Panel_2K2K_CMO_V562D1_L03	(ROM gmt_PanelData *)PanelArray
	//
	// If panel file is not included in the project, but the panel is selected to be
	// the default panel during bootup, issue warning message
	//
	#if (DEFAULT_PANEL == CMO_V562D1_L03)
		#pragma message "Default panel will not bootup unless INCLUDE_CMO_2K2K_V562D1_L03 is defined!"
	#endif
#endif

