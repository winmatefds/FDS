/*
	$Workfile:   T315XW01.h  $
	$Revision: 1.3 $
	$Date: 2011/05/11 06:36:09 $
*/


	#if (DEFAULT_PANEL == AUO_WXGA_T315XW01) 

#include "..\..\Panel\Wxga\AUO\T315XW01\OverdriveLut.h"
//#include "..\..\Panel\Wxga\AUO\T315XW01\GammaLut.h"
#include "..\..\Panel\GammaLut.h"

char ROM PanelName_WXGA_T315XW01[] = "AUO_WXGA_T315XW01";

ROM WORD sRGB2rgb_T315XW01[9] =
{
	0x1000, 0x0000, 0x0000,
	0x0000, 0x1000, 0x0000,
	0x0000, 0x0000, 0x1000
};

ROM gmt_PanelData Panel_AUO_WXGA_T315XW01 =
{
	PanelName_WXGA_T315XW01,			//;
	AUO_WXGA_T315XW01,	// Panel ID (refer to enum defined in panel.h)
	PANEL_WXGA,			//		 	gmd_PanelType PanelType;
	FORMAT_ORG_2D,    // gmd_PanelDataFormat;
	ONE,				//BYTE PixPerClock;
	8,					//BYTE  Depth;
	1368,				//WORD  Width;
	768,				//WORD  Height;
	61,					//BYTE  MaxVFreq;
	59,					//BYTE  MinVFreq;
	85000L,				//DWORD MaxPClk; 		 // KHz
	1414,				//WORD W_MinHTotal
	1560,				//W_TypHTotal
	// TODO: UPDATE 22 and 33 H/V and VTypical totals with spreadsheet
	1560,				//W_22HTotal
	1560,				//W_33HTotal
	10,					//BYTE  MinHSyncWidth;
	28,					//BYTE  MinHSyncBackPorch;
	806,					//W_22VTotal
	806,					//W_33VTotal
	822,				//WORD  MaxVtotal;
	789,					//WORD  MinVtotal;
	806,					//WORD TypVTotal;
	14,					//BYTE  MinVSyncWidth;	
	12,					//BYTE  MinVSyncBackPorch;	
	
	0xCF0C,				//WORD  PowerUpTiming;
	0x9408,				//WORD  PowerDownTiming;
	0xC,					//WORD AccToPixelVal;
	0x0,					//BYTE  SpreadSpectrumCtrl;		// Value for Spread_Spectrum_Control register
	0,					//BYTE  DClkDelay;		 		// DCLK display timing delay adjustment in ns unit.
	Wa_OverDriveLUT_T315XW01,	//	OverDriveLut_T315XW01,
	sRGB2rgb_T315XW01,
	Sta_GammaLUT_M236H5_L02_120,
	0,					//WORD  Invert_DVS 			: 1;
	0,					//WORD  Invert_DHS 			: 1;
	0,					//WORD  Invert_DCLK 			: 1;
	0,					//WORD  Invert_DEN 			: 1;
	TRUE,				//WORD  DportSwap				: 1;
	TRUE,				//WORD  LVDSInterface			: 1;
	SINGLE_BUS,			//WORD  W_LVDS_BusType 			: 1;	
	LVDS_EIGHT_BIT_MODE,		//WORD  LvdsPanelMode	: 2; 
	0xff,				//WORD  Spread_Spect_En 		: 1; // 0xFF : Enable spectrum, 0x00 : Disable
	NO_SWAP,				//WORD	LvdsPosNegSwap			: 1;	
	0,      			//WORD  PadDrive;
	FIVE_PT_SIX_MA,	//WORD  W_LvdsCurrentCtrl		: 2;
	OUT_24_48_BIT,		//WORD  W_BitOutput				: 1;
	STPANEL_LVDS_MAP_STANDARD2,			// WORD W_LVDSSignalMap
	0x0063,				//WORD  W_LvdsClkData
};
extern ROM gmt_PanelData Panel_AUO_WXGA_T315XW01;

#else
	#define Panel_AUO_WXGA_T315XW01	(ROM gmt_PanelData *)PanelArray
	//
	// If panel file is not included in the project, but the panel is selected to be
	// the default panel during bootup, issue warning message
	//
	#if (DEFAULT_PANEL == AUO_WXGA_T315XW01)
		#pragma message "Default panel will not bootup unless INCLUDE_AUO_WXGA_T315XW01 is defined!"
	#endif
#endif
