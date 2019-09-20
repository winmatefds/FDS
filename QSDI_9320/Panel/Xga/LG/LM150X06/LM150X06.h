/*
	$Workfile:   LM150X06.h  $
	$Revision: 1.3 $
	$Date: 2011/05/11 06:36:11 $
*/
#if 0//def INCLUDE_LG_XGA_150X06

#include "..\..\Panel\Xga\LG\LM150X06\OverdriveLut.h"
#include "..\..\Panel\Xga\LG\LM150X06\GammaLut.h"

char ROM PanelName_XGA_LM150X06[] = "LG_LM150X06";

ROM WORD sRGB2rgb_LM150X06[9] =
{
	0x1000, 0x0000, 0x0000,
	0x0000, 0x1000, 0x0000,
	0x0000, 0x0000, 0x1000
};

ROM gmt_PanelData Panel_LG_XGA_LM150X06 =
{
	PanelName_XGA_LM150X06,			//;
	LG_XGA_150X06,					// Panel ID (refer to enum defined in panel.h)	
	PANEL_XGA,			// gmd_PanelType PanelType;
	FORMAT_ORG_2D,    // gmd_PanelDataFormat;
	ONE,				//BYTE PixPerClock;
	8,					//BYTE  Depth;
	640,							//WORD  Width;
	480,							//WORD  Height;
	61,								//BYTE  MaxVFreq;
	58,								//BYTE  MinVFreq;
	158500L,						//DWORD MaxPClk;
	770,							//WORD  W_MinHTotal;
	800,							//WORD  W_TypHTotal;
	// TODO: UPDATE 22 and 33 H/V and VTypical totals with spreadsheet
	900,							//WORD  W_22HTotal;	
	900,							//WORD  W_33HTotal;
	8,								//BYTE  MinHSyncWidth; 
	16,								//BYTE  MinHSyncBackPorch;
	510,							//WORD  W_22VTotal;			
	520,							//WORD  W_33VTotal;
	560,							//WORD  MaxVtotal;
	520,							//WORD  MinVtotal;
	525,							//WORD  TypVTotal;
	6,								//BYTE  MinVSyncWidth;	
	25,								//BYTE  MinVSyncBackPorch;	
	
	0xCF0C, // 0xFF10,								// WORD  PowerUpTiming
	0x0CCF, // 0x10FF,								// WORD  PowerDownTiming
	0xF,							//WORD  AccToPixelVal;
	0x0,							//BYTE  SpreadSpectrumCtrl;
	0,								//BYTE  DClkDelay;
	Wa_OverDriveLUT_LM150X06,		//OverDriveLut_LM150X06,
	sRGB2rgb_LM150X06,
	Sta_GammaLUT_LM150X06,
	0,								//WORD  Invert_DVS			: 1;
	0,								//WORD  Invert_DHS			: 1;
	0,								//WORD  Invert_DCLK			: 1;
	0,								//WORD  Invert_DEN			: 1;
	0xff,							//WORD  DportSwap			: 1;
	TRUE,							//WORD  LVDSInterface		: 1;
	SINGLE_BUS,						//WORD  W_LVDS_BusType		: 1;	
	LVDS_EIGHT_BIT_MODE,			//WORD  LvdsPanelMode		: 2;
	0,							//WORD  Spread_Spect_En		: 1;
	NO_SWAP,							//WORD  LvdsPosNegSwap		: 1;
	0,      						//WORD  PadDrive;
	SEVEN_PT_ONE_MA,				//WORD  W_LvdsCurrentCtrl	: 2;			
	OUT_24_48_BIT,					//WORD  W_BitOutput			: 1;
	STPANEL_LVDS_MAP_STANDARD1,			// WORD W_LVDSSignalMap
	0x0063,							//WORD  W_LvdsClkData;
};






extern ROM gmt_PanelData Panel_LG_XGA_LM150X06;

#else
	#define Panel_LG_XGA_LM150X06	(ROM gmt_PanelData *)PanelArray	
	//
	// If panel file is not included in the project, but the panel is selected to be
	// the default panel during bootup, issue warning message
	//
	#if (DEFAULT_PANEL == LG_XGA_150X06)
		#pragma message "Default panel will not bootup unless INCLUDE_LG_XGA_150X06 is defined!"
	#endif
#endif

