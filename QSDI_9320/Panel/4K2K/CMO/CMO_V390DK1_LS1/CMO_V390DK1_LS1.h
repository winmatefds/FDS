/*
	$Workfile:   VEGA_2K2K.h  $
	$Revision: 1.1 $
	$Date: 2011/12/27 09:42:21 $
*/

//#ifdef INCLUDE_CMO_V390DK1_LS1
#if  (DEFAULT_PANEL == CMO_V390DK1_LS1) 

#include "..\..\Panel\4K2K\CMO\CMO_V390DK1_LS1\OverdriveLut.h"
#include "..\..\Panel\GammaLut.h"

char ROM PanelName_CMO_4K2K[] = "CMO_V390DK1_LS1";

ROM WORD sRGB_CMO_4K2K[9] =
{
	0x1000, 0x0000, 0x0000,
	0x0000, 0x1000, 0x0000,
	0x0000, 0x0000, 0x1000
};

ROM gmt_PanelData Panel_CMO_V390DK1_LS1 =
{
	PanelName_CMO_4K2K,
	CMO_V390DK1_LS1,			// Panel ID (refer to enum defined in panel.h)
	PANEL_2K2K,						// gmd_PanelType PanelType
	FORMAT_ORG_2D,    // gmd_PanelDataFormat;
	FOUR,								// BYTE PixPerClock
	10, //8,									// BYTE  Depth
	2048,//3840, //1920, //2048,								// WORD  Width
	1536,								// WORD  Height
	60, // 29, //60,									// BYTE  MaxVFreq
	60, // 31, //60,									// BYTE  MinVFreq
	259600L, //287800L,								// DWORD MaxPClk (KHz)  
	2560, //2208,								// WORD W_MinHTotal
	2688, //,								// W_TypHTotal
	2688,								// W_22HTotal
	2688,								// W_33HTotal
	32, //32,								// BYTE  MinHSyncWidth
	16, //80, 								// BYTE  MinHSyncBackPorch
	1612, //2173,								// W_22VTotal
	1612, //2173,								// W_33VTotal
	1628, //2173,								// WORD  MaxVtotal
	1547, //2173,								// WORD  MinVtotal
	1612, //2173,								// WORD TypVTotal
	8, //3,									// BYTE  MinVSyncWidth
	16, //7,									// BYTE  MinVSyncBackPorch
	0x1101,								// WORD  PowerUpTiming
	0x1101,								// WORD  PowerDownTiming
	0x21,								// WORD AccToPixelVal
	0x0,								// BYTE  SpreadSpectrumCtrl
	0,									// DCLK display timing delay adjustment in ns unit
	Wa_OverDriveLUT_CMO_4K2K,		// Overdrive look up table
	sRGB_CMO_4K2K,
	Sta_GammaLUT_M236H5_L02_120,
	0,									// WORD  Invert_DVS
	0,									// WORD  Invert_DHS
	0,									// WORD  Invert_DCLK
	0,									// WORD  Invert_DEN
	
	0, // 0xFF,								// WORD  DportSwap
	TRUE,								// WORD  LVDSInterface
	QUAD_BUS,							// WORD  W_LVDS_BusType
	LVDS_EIGHT_BIT_MODE,				// WORD  LvdsPanelMode
	0xFF,								// WORD  Spread_Spect_En
	NO_SWAP,								// WORD	LvdsPosNegSwap
	STPLF_DISPLAY_PAD_DRIVE_1_25,		// WORD PadDrive	
	STPLF_DISPLAY_LVDS_CURRENT_3_5,		// WORD  W_LvdsCurrentCtrl
	OUT_24_48_BIT,						// WORD  W_BitOutput
	STPANEL_LVDS_MAP_STANDARD1,			// WORD W_LVDSSignalMap
	0x0063,								// WORD  W_LvdsClkData
};

extern ROM gmt_PanelData Panel_CMO_V390DK1_LS1;

#else
	#define Panel_CMO_V390DK1_LS1	(ROM gmt_PanelData *)PanelArray
	//
	// If panel file is not included in the project, but the panel is selected to be
	// the default panel during bootup, issue warning message
	//
	#if (DEFAULT_PANEL == CMO_V390DK1_LS1)
		#pragma message "Default panel will not bootup unless INCLUDE_CMO_V390DK1_LS1 is defined!"
	#endif
#endif

