

#if(DEFAULT_PANEL==LG_XGA_150X06_VGA)
#include "..\..\Panel\Xga\LG\LM150X06_VGA\OverdriveLut.h"
#include "..\..\Panel\GammaLut.h"

//
// NOTE:
// This modification is used by one of our customers with their RD 
// connected with VGA daughter card in order to output XGA signals.
//

char ROM PanelName_XGA_LM150X06_VGA[] = "LG_LM150X06_VGA";

ROM WORD sRGB2rgb_LM150X06_VGA[9] =
{
	0x1000, 0x0000, 0x0000,
	0x0000, 0x1000, 0x0000,
	0x0000, 0x0000, 0x1000
};

/*



ROM gmt_PanelData Panel_LG_XGA_LM150X06_VGA =
{
	PanelName_XGA_LM150X06_VGA,			//;
	LG_XGA_150X06_VGA,	// Panel ID (refer to enum defined in panel.h)	
	PANEL_XGA,			// gmd_PanelType PanelType;
	FORMAT_ORG_2D,    // gmd_PanelDataFormat;
	ONE,				//BYTE PixPerClock;
	6,					//BYTE  Depth;
	800,				//WORD  Width;
	600,					//WORD  Height;
	61,					//BYTE  MaxVFreq;
	58,					//BYTE  MinVFreq;
	85000L,				//DWORD MaxPClk; 		 // KHz
	1096,				//WORD W_MinHTotal
	1344,				//W_TypHTotal - XGA at 60hz
	1626,				//W_22HTotal		
	1112,				//W_33HTotal
	136,				//BYTE  MinHSyncWidth;  
	160,				//BYTE  MinHSyncBackPorch;
	834,				//W_22VTotal				
	813,				//W_33VTotal			
	840,				//WORD  MaxVtotal;
	780,				//WORD  MinVtotal;
	806,				//WORD TypVTotal;
	6,					//BYTE  MinVSyncWidth;	 
	29,					//BYTE  MinVSyncBackPorch;
	
	// TODO: Check the power up and power down timing for this panel!!!
	0x9696,				//WORD  PowerUpTiming;
	0x9696,				//WORD  PowerDownTiming;
	0xC,				//WORD AccToPixelVal;
	0x0,				//BYTE  SpreadSpectrumCtrl;		// Value for Spread_Spectrum_Control register
	0,					//BYTE  DClkDelay;		 		// DCLK display timing delay adjustment in ns unit.
	Wa_OverDriveLUT_LM150X06_VGA,	//OverDriveLut_LM150X06_VGA,
	sRGB2rgb_LM150X06_VGA,
	Sta_GammaLUT_M236H5_L02_120,
	0,					//WORD  Invert_DVS 				: 1;
	0,					//WORD  Invert_DHS 				: 1;
	0,					//WORD  Invert_DCLK 			: 1;
	0,					//WORD  Invert_DEN 				: 1;  
	0xff,				//WORD  DportSwap				: 1;
	TRUE,				//WORD  LVDSInterface			: 1;  
	SINGLE_BUS,			//WORD  W_LVDS_BusType 			: 1;	
	LVDS_EIGHT_BIT_MODE,		//WORD  LvdsPanelMode	: 2; 
	0xff,				//WORD  Spread_Spect_En 		: 1; // 0xFF : Enable spectrum, 0x00 : Disable 
	NO_SWAP,				//WORD	LvdsPosNegSwap			: 1;		
	0,      			//WORD  PadDrive;
	FIVE_PT_SIX_MA,		//WORD  W_LvdsCurrentCtrl		: 2;
	OUT_24_48_BIT,		//WORD  W_BitOutput				: 1;
	STPANEL_LVDS_MAP_STANDARD1,			// WORD W_LVDSSignalMap
	0x0063,				//WORD  W_LvdsClkData
};
*/

ROM gmt_PanelData Panel_LG_XGA_LM150X06_VGA =
{
	PanelName_XGA_LM150X06_VGA,			//;
	LG_XGA_150X06_VGA,	// Panel ID (refer to enum defined in panel.h)	
	PANEL_XGA,			// gmd_PanelType PanelType;
	FORMAT_ORG_2D,    // gmd_PanelDataFormat;
	ONE,				//BYTE PixPerClock;
	8,					//BYTE  Depth;
	1024,				//WORD  Width;
	768,					//WORD  Height;
	61,					//BYTE  MaxVFreq;
	50,					//BYTE  MinVFreq;
	65000L,				//DWORD MaxPClk; 		 // KHz
	1096,				//WORD W_MinHTotal
	1344,				//W_TypHTotal - XGA at 60hz
	1626,				//W_22HTotal		
	1112,				//W_33HTotal
	136,				//BYTE  MinHSyncWidth;  
	160,				//BYTE  MinHSyncBackPorch;
	834,				//W_22VTotal				
	813,				//W_33VTotal			
	840,				//WORD  MaxVtotal;
	780,				//WORD  MinVtotal;
	806,				//WORD TypVTotal;
	6,					//BYTE  MinVSyncWidth;	 
	29,					//BYTE  MinVSyncBackPorch;
	
	// TODO: Check the power up and power down timing for this panel!!!
	0xCF0C, // 0xFF10,								// WORD  PowerUpTiming
	0x9696,				//WORD  PowerDownTiming;
	0xC,				//WORD AccToPixelVal;
	0x0,				//BYTE  SpreadSpectrumCtrl;		// Value for Spread_Spectrum_Control register
	0,					//BYTE  DClkDelay;		 		// DCLK display timing delay adjustment in ns unit.
	Wa_OverDriveLUT_LM150X06_VGA,	//OverDriveLut_LM150X06_VGA,
	sRGB2rgb_LM150X06_VGA,
	Sta_GammaLUT_M236H5_L02_120,
	0,					//WORD  Invert_DVS 				: 1;
	0,					//WORD  Invert_DHS 				: 1;
	0,					//WORD  Invert_DCLK 			: 1;
	0,					//WORD  Invert_DEN 				: 1;  
	0xff,				//WORD  DportSwap				: 1;
	TRUE,				//WORD  LVDSInterface			: 1;  
	SINGLE_BUS,			//WORD  W_LVDS_BusType 			: 1;	
	LVDS_EIGHT_BIT_MODE,		//WORD  LvdsPanelMode	: 2; 
	0xff,				//WORD  Spread_Spect_En 		: 1; // 0xFF : Enable spectrum, 0x00 : Disable 
	NO_SWAP,				//WORD	LvdsPosNegSwap			: 1;		
	0,      			//WORD  PadDrive;
	FIVE_PT_SIX_MA,		//WORD  W_LvdsCurrentCtrl		: 2;
	OUT_24_48_BIT,		//WORD  W_BitOutput				: 1;
	STPANEL_LVDS_MAP_STANDARD2,			// WORD W_LVDSSignalMap
	0x0063,				//WORD  W_LvdsClkData
};


/*
ROM gmt_PanelData Panel_LG_XGA_LM150X06_VGA =
{
	PanelName_XGA_LM150X06_VGA,			//;
	LG_XGA_150X06_VGA,	// Panel ID (refer to enum defined in panel.h)	
	PANEL_XGA,			// gmd_PanelType PanelType;
	FORMAT_ORG_2D,    // gmd_PanelDataFormat;
	ONE,				//BYTE PixPerClock;
	6,					//BYTE  Depth;
	640,				//WORD  Width;
	480,					//WORD  Height;
	61,					//BYTE  MaxVFreq;
	58,					//BYTE  MinVFreq;
	85000L,				//DWORD MaxPClk; 		 // KHz
	770,				//WORD W_MinHTotal
	800,				//W_TypHTotal - XGA at 60hz
	800,				//W_22HTotal		
	800,				//W_33HTotal
	30,				//BYTE  MinHSyncWidth;  
	100,				//BYTE  MinHSyncBackPorch;
	525,				//W_22VTotal				
	525,				//W_33VTotal			
	560,				//WORD  MaxVtotal;
	520,				//WORD  MinVtotal;
	525,				//WORD TypVTotal;
	5,					//BYTE  MinVSyncWidth;	 
	29,					//BYTE  MinVSyncBackPorch;
	
	// TODO: Check the power up and power down timing for this panel!!!
	0x9696,				//WORD  PowerUpTiming;
	0x9696,				//WORD  PowerDownTiming;
	0xC,				//WORD AccToPixelVal;
	0x0,				//BYTE  SpreadSpectrumCtrl;		// Value for Spread_Spectrum_Control register
	0,					//BYTE  DClkDelay;		 		// DCLK display timing delay adjustment in ns unit.
	Wa_OverDriveLUT_LM150X06_VGA,	//OverDriveLut_LM150X06_VGA,
	sRGB2rgb_LM150X06_VGA,
	Sta_GammaLUT_M236H5_L02_120,
	0,					//WORD  Invert_DVS 				: 1;
	0,					//WORD  Invert_DHS 				: 1;
	0,					//WORD  Invert_DCLK 			: 1;
	0,					//WORD  Invert_DEN 				: 1;  
	0xff,				//WORD  DportSwap				: 1;
	TRUE,				//WORD  LVDSInterface			: 1;  
	SINGLE_BUS,			//WORD  W_LVDS_BusType 			: 1;	
	LVDS_EIGHT_BIT_MODE,		//WORD  LvdsPanelMode	: 2; 
	0xff,				//WORD  Spread_Spect_En 		: 1; // 0xFF : Enable spectrum, 0x00 : Disable 
	NO_SWAP,				//WORD	LvdsPosNegSwap			: 1;		
	0,      			//WORD  PadDrive;
	FIVE_PT_SIX_MA,		//WORD  W_LvdsCurrentCtrl		: 2;
	OUT_24_48_BIT,		//WORD  W_BitOutput				: 1;
	STPANEL_LVDS_MAP_STANDARD2,			// WORD W_LVDSSignalMap
	0x0063,				//WORD  W_LvdsClkData
};



ROM gmt_PanelData Panel_LG_XGA_LM150X06_VGA =
{
	PanelName_XGA_LM150X06_VGA,			//;
	LG_XGA_150X06_VGA,	// Panel ID (refer to enum defined in panel.h)	
	PANEL_XGA,			// gmd_PanelType PanelType;
	FORMAT_ORG_2D,    // gmd_PanelDataFormat;
	ONE,				//BYTE PixPerClock;
	6,					//BYTE  Depth;
	800,				//WORD  Width;
	480,					//WORD  Height;
	51,					//BYTE  MaxVFreq;
	49,					//BYTE  MinVFreq;
	85000L,				//DWORD MaxPClk; 		 // KHz
	900,				//WORD W_MinHTotal
	928,				//W_TypHTotal - XGA at 60hz
	928,				//W_22HTotal		
	928,				//W_33HTotal
	48,				//BYTE  MinHSyncWidth;  
	40,				//BYTE  MinHSyncBackPorch;
	525,				//W_22VTotal				
	525,				//W_33VTotal			
	560,				//WORD  MaxVtotal;
	520,				//WORD  MinVtotal;
	525,				//WORD TypVTotal;
	3,					//BYTE  MinVSyncWidth;	 
	29,					//BYTE  MinVSyncBackPorch;
	
	// TODO: Check the power up and power down timing for this panel!!!
	0x9696,				//WORD  PowerUpTiming;
	0x9696,				//WORD  PowerDownTiming;
	0xC,				//WORD AccToPixelVal;
	0x0,				//BYTE  SpreadSpectrumCtrl;		// Value for Spread_Spectrum_Control register
	0,					//BYTE  DClkDelay;		 		// DCLK display timing delay adjustment in ns unit.
	Wa_OverDriveLUT_LM150X06_VGA,	//OverDriveLut_LM150X06_VGA,
	sRGB2rgb_LM150X06_VGA,
	Sta_GammaLUT_M236H5_L02_120,
	0xff,					//WORD  Invert_DVS 				: 1;
	0xff,					//WORD  Invert_DHS 				: 1;
	0xff,					//WORD  Invert_DCLK 			: 1;
	0,					//WORD  Invert_DEN 				: 1;  
	0xff,				//WORD  DportSwap				: 1;
	TRUE,				//WORD  LVDSInterface			: 1;  
	SINGLE_BUS,			//WORD  W_LVDS_BusType 			: 1;	
	LVDS_EIGHT_BIT_MODE,		//WORD  LvdsPanelMode	: 2; 
	0,				//WORD  Spread_Spect_En 		: 1; // 0xFF : Enable spectrum, 0x00 : Disable 
	NO_SWAP,				//WORD	LvdsPosNegSwap			: 1;		
	0,      			//WORD  PadDrive;
	FIVE_PT_SIX_MA,		//WORD  W_LvdsCurrentCtrl		: 2;
	OUT_24_48_BIT,		//WORD  W_BitOutput				: 1;
	STPANEL_LVDS_MAP_STANDARD1,			// WORD W_LVDSSignalMap
	0x0063,				//WORD  W_LvdsClkData
};

*/
/*

ROM gmt_PanelData Panel_LG_XGA_LM150X06_VGA =
{
	PanelName_XGA_LM150X06_VGA,			//;
	LG_XGA_150X06_VGA,	// Panel ID (refer to enum defined in panel.h)	
	PANEL_XGA,			// gmd_PanelType PanelType;
	FORMAT_ORG_2D,    // gmd_PanelDataFormat;
	ONE,				//BYTE PixPerClock;
	8,					//BYTE  Depth;
	640,				//WORD  Width;
	480,					//WORD  Height;
	61,					//BYTE  MaxVFreq;
	60,					//BYTE  MinVFreq;
	85000L,				//DWORD MaxPClk; 		 // KHz
	770,				//WORD W_MinHTotal
	800,				//W_TypHTotal - XGA at 60hz
	900,				//W_22HTotal		
	900,				//W_33HTotal
	30,				//BYTE  MinHSyncWidth;  
	160,				//BYTE  MinHSyncBackPorch;
	550,				//W_22VTotal				
	550,				//W_33VTotal			
	560,				//WORD  MaxVtotal;
	520,				//WORD  MinVtotal;
	525,				//WORD TypVTotal;
	5,					//BYTE  MinVSyncWidth;	 
	29,					//BYTE  MinVSyncBackPorch;
	
	// TODO: Check the power up and power down timing for this panel!!!
	0x9696,				//WORD  PowerUpTiming;
	0x9696,				//WORD  PowerDownTiming;
	0xC,				//WORD AccToPixelVal;
	0x0,				//BYTE  SpreadSpectrumCtrl;		// Value for Spread_Spectrum_Control register
	0,					//BYTE  DClkDelay;		 		// DCLK display timing delay adjustment in ns unit.
	Wa_OverDriveLUT_LM150X06_VGA,	//OverDriveLut_LM150X06_VGA,
	sRGB2rgb_LM150X06_VGA,
	Sta_GammaLUT_M236H5_L02_120,
	0,					//WORD  Invert_DVS 				: 1;
	0,					//WORD  Invert_DHS 				: 1;
	0,					//WORD  Invert_DCLK 			: 1;
	0,					//WORD  Invert_DEN 				: 1;  
	0xff,				//WORD  DportSwap				: 1;
	TRUE,				//WORD  LVDSInterface			: 1;  
	SINGLE_BUS,			//WORD  W_LVDS_BusType 			: 1;	
	LVDS_EIGHT_BIT_MODE,		//WORD  LvdsPanelMode	: 2; 
	0xff,				//WORD  Spread_Spect_En 		: 1; // 0xFF : Enable spectrum, 0x00 : Disable 
	NO_SWAP,				//WORD	LvdsPosNegSwap			: 1;		
	0,      			//WORD  PadDrive;
	FIVE_PT_SIX_MA,		//WORD  W_LvdsCurrentCtrl		: 2;
	OUT_24_48_BIT,		//WORD  W_BitOutput				: 1;
	STPANEL_LVDS_MAP_STANDARD1,			// WORD W_LVDSSignalMap
	0x0063,				//WORD  W_LvdsClkData
};
*/

/*
ROM gmt_PanelData Panel_LG_XGA_LM150X06_VGA =
{
	PanelName_XGA_LM150X06_VGA,			//;
	LG_XGA_150X06_VGA,	// Panel ID (refer to enum defined in panel.h)	
	PANEL_XGA,			// gmd_PanelType PanelType;
	FORMAT_ORG_2D,    // gmd_PanelDataFormat;
	ONE,				//BYTE PixPerClock;
	8,					//BYTE  Depth;
	640,				//WORD  Width;
	480,					//WORD  Height;
	61,					//BYTE  MaxVFreq;
	60,					//BYTE  MinVFreq;
	85000L,				//DWORD MaxPClk; 		 // KHz
	770,				//WORD W_MinHTotal
	800,				//W_TypHTotal - XGA at 60hz
	800,				//W_22HTotal		
	800,				//W_33HTotal
	30,				//BYTE  MinHSyncWidth;  
	100,				//BYTE  MinHSyncBackPorch;
	525,				//W_22VTotal				
	525,				//W_33VTotal			
	560,				//WORD  MaxVtotal;
	520,				//WORD  MinVtotal;
	525,				//WORD TypVTotal;
	5,					//BYTE  MinVSyncWidth;	 
	29,					//BYTE  MinVSyncBackPorch;
	
	// TODO: Check the power up and power down timing for this panel!!!
	0x9696,				//WORD  PowerUpTiming;
	0x9696,				//WORD  PowerDownTiming;
	0xC,				//WORD AccToPixelVal;
	0x0,				//BYTE  SpreadSpectrumCtrl;		// Value for Spread_Spectrum_Control register
	0,					//BYTE  DClkDelay;		 		// DCLK display timing delay adjustment in ns unit.
	Wa_OverDriveLUT_LM150X06_VGA,	//OverDriveLut_LM150X06_VGA,
	sRGB2rgb_LM150X06_VGA,
	Sta_GammaLUT_M236H5_L02_120,
	0,					//WORD  Invert_DVS 				: 1;
	0,					//WORD  Invert_DHS 				: 1;
	0,					//WORD  Invert_DCLK 			: 1;
	0,					//WORD  Invert_DEN 				: 1;  
	0xff,				//WORD  DportSwap				: 1;
	TRUE,				//WORD  LVDSInterface			: 1;  
	SINGLE_BUS,			//WORD  W_LVDS_BusType 			: 1;	
	LVDS_EIGHT_BIT_MODE,		//WORD  LvdsPanelMode	: 2; 
	0xff,				//WORD  Spread_Spect_En 		: 1; // 0xFF : Enable spectrum, 0x00 : Disable 
	NO_SWAP,				//WORD	LvdsPosNegSwap			: 1;		
	0,      			//WORD  PadDrive;
	FIVE_PT_SIX_MA,		//WORD  W_LvdsCurrentCtrl		: 2;
	OUT_24_48_BIT,		//WORD  W_BitOutput				: 1;
	STPANEL_LVDS_MAP_STANDARD2,			// WORD W_LVDSSignalMap
	0x0063,				//WORD  W_LvdsClkData
};
*/
/*
ROM gmt_PanelData Panel_LG_XGA_LM150X06_VGA =
{
	PanelName_XGA_LM150X06_VGA,			//;
	LG_XGA_150X06_VGA,	// Panel ID (refer to enum defined in panel.h)	
	PANEL_XGA,			// gmd_PanelType PanelType;
	FORMAT_ORG_2D,    // gmd_PanelDataFormat;
	ONE,				//BYTE PixPerClock;
	8,					//BYTE  Depth;
	1024,				//WORD  Width;
	768,					//WORD  Height;
	61,					//BYTE  MaxVFreq;
	59,					//BYTE  MinVFreq;
	70000L,				//DWORD MaxPClk; 		 // KHz
	1096,				//WORD W_MinHTotal
	1344,				//W_TypHTotal - XGA at 60hz
	1344,				//W_22HTotal		
	1344,				//W_33HTotal
	136,				//BYTE  MinHSyncWidth;  
	160,				//BYTE  MinHSyncBackPorch;
	834,				//W_22VTotal				
	813,				//W_33VTotal			
	840,				//WORD  MaxVtotal;
	780,				//WORD  MinVtotal;
	806,				//WORD TypVTotal;
	6,					//BYTE  MinVSyncWidth;	 
	29,					//BYTE  MinVSyncBackPorch;
	
	// TODO: Check the power up and power down timing for this panel!!!
	0xCF0C, // 0xFF10,								// WORD  PowerUpTiming
	0x9696,				//WORD  PowerDownTiming;
	0xC,				//WORD AccToPixelVal;
	0x0,				//BYTE  SpreadSpectrumCtrl;		// Value for Spread_Spectrum_Control register
	0,					//BYTE  DClkDelay;		 		// DCLK display timing delay adjustment in ns unit.
	Wa_OverDriveLUT_LM150X06_VGA,	//OverDriveLut_LM150X06_VGA,
	sRGB2rgb_LM150X06_VGA,
	Sta_GammaLUT_M236H5_L02_120,
	0,					//WORD  Invert_DVS 				: 1;
	0,					//WORD  Invert_DHS 				: 1;
	0,					//WORD  Invert_DCLK 			: 1;
	0,					//WORD  Invert_DEN 				: 1;  
	0xff,				//WORD  DportSwap				: 1;
	TRUE,				//WORD  LVDSInterface			: 1;  
	SINGLE_BUS,			//WORD  W_LVDS_BusType 			: 1;	
	LVDS_EIGHT_BIT_MODE,		//WORD  LvdsPanelMode	: 2; 
	0xff,				//WORD  Spread_Spect_En 		: 1; // 0xFF : Enable spectrum, 0x00 : Disable 
	NO_SWAP,				//WORD	LvdsPosNegSwap			: 1;		
	0,      			//WORD  PadDrive;
	FIVE_PT_SIX_MA,		//WORD  W_LvdsCurrentCtrl		: 2;
	OUT_24_48_BIT,		//WORD  W_BitOutput				: 1;
	STPANEL_LVDS_MAP_STANDARD2,			// WORD W_LVDSSignalMap
	0x0063,				//WORD  W_LvdsClkData
};


ROM gmt_PanelData Panel_LG_XGA_LM150X06_VGA =
{
	PanelName_XGA_LM150X06_VGA,			//;
	LG_XGA_150X06_VGA,	// Panel ID (refer to enum defined in panel.h)	
	PANEL_XGA,			// gmd_PanelType PanelType;
	FORMAT_ORG_2D,    // gmd_PanelDataFormat;
	ONE,				//BYTE PixPerClock;
	8,					//BYTE  Depth;
	1280,				//WORD  Width;
	800,					//WORD  Height;
	61,					//BYTE  MaxVFreq;
	60,					//BYTE  MinVFreq;
	75000L,				//DWORD MaxPClk; 		 // KHz
	1360,				//WORD W_MinHTotal	
	1440,				//W_TypHTotal
	1440,				//W_22HTotal		
	1440,				//W_33HTotal
	8,					//BYTE  MinHSyncWidth;
	32,					//BYTE  MinHSyncBackPorch;
	823,					//W_22VTotal				
	823,					//W_33VTotal		
	840,					//WORD  MaxVtotal;
	808,					//WORD  MinVtotal;
	823,					//WORD TypVTotal;
	2,					//BYTE  MinVSyncWidth;	
	12,					//BYTE  MinVSyncBackPorch;	
	
	0x9408,				//WORD  PowerUpTiming;
	0x9408,				//WORD  PowerDownTiming;
	0xc,					//WORD AccToPixelVal;
	0x00,					//BYTE  SpreadSpectrumCtrl;		// Value for Spread_Spectrum_Control register
	0,					//BYTE  DClkDelay;		 		// DCLK display timing delay adjustment in ns unit.
	Wa_OverDriveLUT_LM150X06_VGA,	//OverDriveLut_LM150X06_VGA,
	sRGB2rgb_LM150X06_VGA,
	Sta_GammaLUT_M236H5_L02_120,
	0,					//WORD  Invert_DVS 				: 1;
	0,					//WORD  Invert_DHS 				: 1;
	0,					//WORD  Invert_DCLK 			: 1;
	0,					//WORD  Invert_DEN 				: 1;
	0xff,				//WORD  DportSwap				: 1;
	TRUE,				//WORD  LVDSInterface			: 1;
	SINGLE_BUS,			//WORD  W_LVDS_BusType 			: 1;	
	LVDS_EIGHT_BIT_MODE,		//WORD  LvdsPanelMode	: 2; 
	0xff,				//WORD  Spread_Spect_En 		: 1; // 0xFF : Enable spectrum, 0x00 : Disable 
	NO_SWAP,				//WORD	LvdsPosNegSwap			: 1;		
	0,      			//WORD  PadDrive;
	FIVE_PT_SIX_MA,		//WORD  W_LvdsCurrentCtrl		: 2;
	OUT_24_48_BIT,		//WORD  W_BitOutput				: 1;
	STPANEL_LVDS_MAP_STANDARD2,			// WORD W_LVDSSignalMap
	0x0063,				//WORD  W_LvdsClkData
};
*/

/*

ROM gmt_PanelData Panel_LG_XGA_LM150X06_VGA =
{
	PanelName_XGA_LM150X06_VGA,			//;
	LG_XGA_150X06_VGA,	// Panel ID (refer to enum defined in panel.h)	
	PANEL_XGA,			// gmd_PanelType PanelType;
	FORMAT_ORG_2D,    // gmd_PanelDataFormat;
	ONE,				//BYTE PixPerClock;
	8,					//BYTE  Depth;
	1024,				//WORD  Width;
	768,					//WORD  Height;
	61,					//BYTE  MaxVFreq;
	55,					//BYTE  MinVFreq;
	60000L,				//DWORD MaxPClk; 		 // KHz
	1054,				//WORD W_MinHTotal
	1344,				//W_TypHTotal - XGA at 60hz
	1344,				//W_22HTotal		
	1184,				//W_33HTotal
	136,				//BYTE  MinHSyncWidth;  
	160,				//BYTE  MinHSyncBackPorch;
	806,				//W_22VTotal				
	806,				//W_33VTotal			
	810,				//WORD  MaxVtotal;
	774,				//WORD  MinVtotal;
	806,				//WORD TypVTotal;
	6,					//BYTE  MinVSyncWidth;	 
	29,					//BYTE  MinVSyncBackPorch;
	
	// TODO: Check the power up and power down timing for this panel!!!
	0xCF0C, // 0xFF10,								// WORD  PowerUpTiming
	0x9696,				//WORD  PowerDownTiming;
	0xC,				//WORD AccToPixelVal;
	0x0,				//BYTE  SpreadSpectrumCtrl;		// Value for Spread_Spectrum_Control register
	0,					//BYTE  DClkDelay;		 		// DCLK display timing delay adjustment in ns unit.
	Wa_OverDriveLUT_LM150X06_VGA,	//OverDriveLut_LM150X06_VGA,
	sRGB2rgb_LM150X06_VGA,
	Sta_GammaLUT_M236H5_L02_120,
	0,					//WORD  Invert_DVS 				: 1;
	0,					//WORD  Invert_DHS 				: 1;
	0,					//WORD  Invert_DCLK 			: 1;
	0,					//WORD  Invert_DEN 				: 1;  
	0xff,				//WORD  DportSwap				: 1;
	TRUE,				//WORD  LVDSInterface			: 1;  
	SINGLE_BUS,			//WORD  W_LVDS_BusType 			: 1;	
	LVDS_EIGHT_BIT_MODE,		//WORD  LvdsPanelMode	: 2; 
	0xff,				//WORD  Spread_Spect_En 		: 1; // 0xFF : Enable spectrum, 0x00 : Disable 
	NO_SWAP,				//WORD	LvdsPosNegSwap			: 1;		
	0,      			//WORD  PadDrive;
	FIVE_PT_SIX_MA,		//WORD  W_LvdsCurrentCtrl		: 2;
	OUT_24_48_BIT,		//WORD  W_BitOutput				: 1;
	STPANEL_LVDS_MAP_STANDARD2,			// WORD W_LVDSSignalMap
	0x0063,				//WORD  W_LvdsClkData
};


*/



extern ROM gmt_PanelData Panel_LG_XGA_LM150X06_VGA;

#else
	#define Panel_LG_XGA_LM150X06_VGA	(ROM gmt_PanelData *)PanelArray
	//
	// If panel file is not included in the project, but the panel is selected to be
	// the default panel during bootup, issue warning message
	//
	#if (DEFAULT_PANEL == LG_XGA_150X06_VGA)
		#pragma message "Default panel will not bootup unless INCLUDE_LG_XGA_150X06_VGA is defined!"
	#endif
#endif
