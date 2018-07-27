/*
   $Workfile:   AppsTest.c  $
    $Revision: 1.252 $
   $Date: 2013/07/31 08:02:03 $
*/

#define __APPSTEST_C__                 
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
// MODULE   :   AppsTest.c
//
// USAGE   :   This module contains all the user AppsTest commands
//
//******************************************************************************

//******************************************************************************
//  I N C L U D E    F I L E S
//******************************************************************************

#include <math.h>
#include <string.h>

#include "System\All.h"
#include "TestHarness\Appstest.h"
#include "StdLibInc\gm_Mcu186.h"
#include "system\Sdram.h"

#if (FEATURE_ACC3 == ENABLE)
#include "StdLibInc\gm_ACC3.h"
#endif

#if (FEATURE_ACM_3D == ENABLE)
#include "StdLibInc\gm_ACM3D.h"
#endif

#if 0// (FEATURE_HDMI==ENABLE)
#include "Board\LoadHdcpKey.h"
#endif

#if FEATURE_HDMI == ENABLE
#include "HdmiDvi\sthdmirx.h"
#endif

#if FEATURE_CEC == ENABLE
#include "CEC\CecHandler.h"
#endif

#if (FEATURE_ACR == ENABLE)
#include "system\ACR_Handler.h"
#endif

#if (FEATURE_PIXEL_CRUNCH == ENABLE)
#include "stdlibinc\PIXCRMain.h"
#endif

#ifdef USER_EXTENDED_GPROBE_CMD
//******************************************************************************
//  D E F I N E S
//******************************************************************************

#define _DEBUG

#define FULL_APPSTEST_CMD        // Full appstest commands used (28.7kBytes)
#define USE_IMP_PIXGRAB_CMD      // Use IMP pixel grab appstest cmd (1kBytes)
#define DP12_BATHTUB_ENABLE      // JC_Athena_BathTub (1kBytes)
#define DP11_BATHTUB_ENABLE      // Bathtub collection for DP2 (CPHY2) // For Stress Test
//#define BATHTUB_COLLECTION       // Old code for for DP
#if ((FEATURE_HDMI==ENABLE) && ((DEBUG_HDMI_CPHY_CAPTURE !=0) ||(DEBUG_HDMI_EQ_BATHTUB !=0))) || defined(ST_4K2K_93xx_BOARD)
#define RELEASE_CUSTOMER_SRAM    // to release SRAM space (5760 bytes)
#endif
#define DP12RX_PRINT_DEBUG_MSG //define for enable/disable DP Lib print messages

#ifdef _DEBUG
#define  msg(a)             gm_Print(a,0)
#define  msg_i(a,b)         gm_Print(a,b)
#else
#define  msg(a)
#define  msg_i(a,b)
#endif

//******************************************************************************
//  E X T E R N A L     V A R I A B L E S
//******************************************************************************
BYTE gBa_AppsTestParam[52];            // input parameters
BYTE gBa_AppsTestParamCount = 0;

BYTE AudioDemoWorkaround = TRUE;


#ifdef PANEL_120_OR_100_HZ
#if (defined(ACC_LUT_METHOD))
extern BYTE Method_ACC_LUT_Enable;
#endif
extern BYTE Method_Black_Frame_Insertion_Enable;
extern BYTE Method_Grey_Frame_Insertion_Enable;
#endif

extern ROM DWORD gmc_MemoryPalAutoInstallSize;

#if FEATURE_CEC == ENABLE
#define PHYS_ADDR_TO_WORD(DIG1,DIG2,DIG3,DIG4)  (WORD)(((((WORD)(DIG1))&0x0F)<<4)|\
                                                        (((WORD)(DIG2))&0x0F)|\
                                                       ((((WORD)(DIG3))&0x0F)<<12)|\
                                                       ((((WORD)(DIG4))&0x0F)<<8))

extern gmt_CEC_DEVICERECORD a_CECDevices[];
#endif // FEATURE_CEC

#ifdef ANX9021_DEV_ID
extern void ModeSetInputAttrPrefs(gmt_PHY_CH);
#endif

//******************************************************************************
//  E X T E R N A L     F U N C T I O N S
//******************************************************************************
extern void StartRScollect             (void);
extern void BypassMemory(void);

extern void  PanelSetAsInputTiming(void); // must before gm_MemorySetup 
// typedef enum gmt_AGCMethod;
//extern void gm_SetDecoderAGCMethod(gmt_AGCMethod AGCMethod);

//
// Checksum calculation
//
WORD far CalculateCodeCrc(void);
WORD GrabPixel(gmt_PHY_CH B_Channel, WORD W_Xcord, WORD W_Ycord, gmt_RGB_COLOR B_Color );

void Set_OCM_Freq(WORD ocm_freq);

#ifndef NO_PIP_SUPPORT
void DebugAdjustPip(void);
#endif

#if (FEATURE_PIXEL_CRUNCH == ENABLE)
// TEST ONLY
void Vwd_Start_Test(BYTE TestMode);
BYTE * GetPixCrBufferAddr(void);
void PixCrCoarseRun_Test(void);

#ifndef UserPrefVwdDemo
extern BYTE VwdDemoTemp;
#define UserPrefVwdDemo VwdDemoTemp
#endif
#define INTL_FILTERAPPSTEP 200
#define INTL_ONTIMEDEFAULT 4000
#endif // FEATURE_PIXEL_CRUNCH

#ifdef DP12_BATHTUB_ENABLE
void DPRxBathTubMeas(BYTE B_Lane, BYTE B_eq, BYTE B_fp);
#endif

#ifdef DP11_BATHTUB_ENABLE
void DPRxBathTubMeas_DP(BYTE B_TestedLane, BYTE B_TestedFp, BYTE B_NumCPT, WORD W_Period10ms, BYTE B_Port);
#endif

#if (FEATURE_PIXEL_CRUNCH == ENABLE)
void TestApplyHighLighWindow(WORD wXStart, WORD wYStart, WORD wWidth, WORD wHeight, BYTE bMode);
#endif

//******************************************************************************
//  L O C A L    D E F I N I T I O N S
//******************************************************************************

//
// enum for appstest functions
//
enum
{
   Func0, Func1, Func2, Func3, Func4, Func5, Func6, Func7, Func8, Func9,
   Func10,Func11,Func12,Func13,Func14,Func15,Func16,Func17,Func18,Func19,
   Func20,Func21,Func22,Func23,Func24,Func25,Func26,Func27,Func28,Func29,
   Func30,Func31,Func32,Func33,Func34,Func35,Func36,Func37,Func38,Func39,
   Func40,Func41,Func42,Func43,Func44,Func45,Func46,Func47,Func48,Func49,
   Func50 = 50,Func51,Func52,Func53,Func54, Func55, Func56,Func57,
   Func60 = 60,Func61,Func62,Func63,Func64,Func65,Func66,Func67,Func68,Func69,
   Func70 = 70,Func71,
   Func80 = 80,Func81,Func82,Func83,
   Func90 = 90,Func91,Func92,Func93,Func94,Func95,Func96,
   Func100=100,Func101,Func102, Func103, Func104, Func106 = 106,
   Func110=110,Func111,Func112,
   Func120=120,Func121,
   Func150=150,Func151,Func152,Func153,Func154,Func155,Func156,Func157,Func158,Func159,
   Func160=160,
   Func170=170,Func171,Func172,Func173,Func174,Func175,Func176,Func177,Func178,Func179,
   Func180=180,Func181,Func182,Func183,Func184,Func185,Func186,
#ifdef OSD_PORT_CHANGE_DIALOG
   Func187=187,
#endif
   Func188=188,
   Func190=190,Func191,Func192,Func193,Func194,
   Func200=200,Func201,Func202,Func203,Func204,Func205,Func206,Func207,Func208,Func209,
   Func210=210,Func211,Func212,Func213,Func214,Func215,Func216,
   Func220=220,Func221, Func222,Func223, Func224, Func225, Func226, Func227, Func228, Func229,
   Func230=230,Func231,Func232,Func233,Func234,Func235,Func236,
   Func240=240,Func241,Func242,Func243,Func244,Func245,Func246,Func247,Func248,Func249,
   Func250=250,Func251,Func252,Func253,Func254,Func255
};

#ifdef NVRAM_DP_EDID_ADDR
BYTE ROM ST_EDID[] =
{
   0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x1D, 0xA3, 0x01, 0x69, 0x65, 0x00, 0x00, 0x00,
   0x2A, 0x11, 0x01, 0x04, 0xA0, 0x25, 0x16, 0x78, 0x02, 0x0D, 0xC9, 0xA0, 0x57, 0x47, 0x98, 0x27,
   0x12, 0x48, 0x4C, 0xEF, 0xEF, 0x00, 0xD1, 0xFC, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
   0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x3A, 0x80, 0x18, 0x71, 0x38, 0x2D, 0x40, 0x58, 0x2C,
   0x45, 0x00, 0x00, 0x00, 0x53, 0x00, 0x00, 0x1E, 0x8C, 0x0A, 0xD0, 0x8A, 0x20, 0xE0, 0x2D, 0x10,
   0x10, 0x3E, 0x96, 0x00, 0xE8, 0x12, 0x11, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x53,
   0x54, 0x4D, 0x20, 0x31, 0x30, 0x38, 0x30, 0x50, 0x20, 0x38, 0x0A, 0x20, 0x00, 0x00, 0x00, 0xFD,
   0x00, 0x31, 0x4B, 0x0F, 0x4B, 0x0F, 0x00, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x01,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};
#endif

#if 0
// ACC3 Test
DWORD const ACC3_Final_LUT[]=
{
   0     ,	   65484 ,	   65488 ,	   65497 ,	   65507 ,	   65518 ,	   65525 ,	   65530 ,	   65532 ,	   65533 ,	   65533 ,	   65533 ,	   65534 ,	   65534 ,	   41    ,	   158   ,
   335   ,	   563   ,	   799   ,	   1013  ,	   1208  ,	   1387  ,	   1547  ,	   1687  ,	   1809  ,	   1915  ,	   1998  ,	   2051  ,	   2077  ,	   2079  ,	   2060  ,	   2022  ,
   1966  ,	   1895  ,	   1815  ,	   1735  ,	   1654  ,	   1572  ,	   1492  ,	   1416  ,	   1344  ,	   1275  ,	   1215  ,	   1169  ,	   1137  ,	   1117  ,	   1109  ,	   1113  ,
   1128  ,	   1153  ,	   1179  ,	   1196  ,	   1205  ,	   1205  ,	   1202  ,	   1198  ,	   1194  ,	   1190  ,	   1189  ,	   1198  ,	   1214  ,	   1238  ,	   1262  ,	   1279  ,
   1289  ,	   1293  ,	   1289  ,	   1278  ,	   1258  ,	   1232  ,	   1202  ,	   1171  ,	   1139  ,	   1106  ,	   1073  ,	   1041  ,	   1009  ,	   976   ,	   945   ,	   914   ,
   884   ,	   854   ,	   825   ,	   797   ,	   770   ,	   743   ,	   717   ,	   692   ,	   666   ,	   642   ,	   617   ,	   593   ,	   569   ,	   545   ,	   522   ,	   499   ,
   476   ,	   454   ,	   432   ,	   411   ,	   390   ,	   369   ,	   348   ,	   328   ,	   308   ,	   289   ,	   270   ,	   251   ,	   233   ,	   215   ,	   198   ,	   181   ,
   164   ,	   147   ,	   131   ,	   115   ,	   100   ,	   85    ,	   70    ,	   56    ,	   42    ,	   28    ,	   15    ,	   2     ,	   65526 ,	   65514 ,	   65504 ,	   65499 ,
   65499 ,	   65502 ,	   65510 ,	   65521 ,	   1     ,	   20    ,	   39    ,	   54    ,	   65    ,	   71    ,	   76    ,	   80    ,	   85    ,	   89    ,	   95    ,	   105   ,
   118   ,	   135   ,	   151   ,	   163   ,	   172   ,	   177   ,	   178   ,	   175   ,	   169   ,	   159   ,	   148   ,	   136   ,	   125   ,	   114   ,	   103   ,	   95    ,
   87    ,	   81    ,	   78    ,	   79    ,	   86    ,	   96    ,	   110   ,	   126   ,	   143   ,	   162   ,	   181   ,	   196   ,	   207   ,	   215   ,	   222   ,	   229   ,
   238   ,	   247   ,	   257   ,	   269   ,	   282   ,	   297   ,	   310   ,	   320   ,	   326   ,	   327   ,	   326   ,	   322   ,	   315   ,	   306   ,	   295   ,	   285   ,
   274   ,	   264   ,	   254   ,	   244   ,	   233   ,	   223   ,	   213   ,	   204   ,	   194   ,	   184   ,	   174   ,	   165   ,	   155   ,	   146   ,	   137   ,	   127   ,
   118   ,	   109   ,	   100   ,	   91    ,	   83    ,	   74    ,	   65    ,	   57    ,	   48    ,	   40    ,	   32    ,	   24    ,	   16    ,	   8     ,	   0     ,	   65528 ,
   65520 ,	   65512 ,	   65506 ,	   65500 ,	   65497 ,	   65495 ,	   65496 ,	   65500 ,	   65507 ,	   65517 ,	   65527 ,	   65535 ,	   3     ,	   4     ,	   4     ,	   4     ,
   4     ,	   4     ,	   4     ,	   4     ,	   4     ,	   4     ,	   4     ,	   4     ,	   4     ,	   4     ,	   4     ,	   4     ,	   4     ,	   4     ,	   4     ,	   4     ,
   1     ,   1     ,
};
#endif

#if (FEATURE_ACC3 == ENABLE)
ROM SDWORD SDW_SLOP_FIX[]=
{
0x0BEB,	0x0BE0,	0x0CB0,	0x0D92,	0x0CC4,	0x0BD5,	0x0BA3,	0x0B8B,	0x0B9A,	0x0BBC,	0x0BE8,	0x0C15,	0x0C70,	0x0D46,	0x0ED1,	0x10A0,
0x1172,	0x10F0,	0x10B8,	0x11C5,	0x133F,	0x1513,	0x1706,	0x17EB,	0x1800,	0x1800,	0x1800,	0x1800,	0x1800,	0x1800,	0x1800,	0x1800,
0x17DB,	0x164C,	0x130C,	0x1037,	0x0EA1,	0x0DE1,	0x0D91,	0x0D36,	0x0CD6,	0x0C8A,	0x0C76,	0x0C73,	0x0C57,	0x0C48,	0x0C5E,	0x0C92,
0x0CB6,	0x0CCA,	0x0CF7,	0x0D38,	0x0D8B,	0x0E34,	0x0F48,	0x102B,	0x10A2,	0x12B8,	0x14AD,	0x122A,	0x0E5B,	0x0CC5,	0x0C60,	0x0C5D,
};
#endif

#ifdef VIDEO_CAPTURE
BYTE CDATA_Flag = 0, Frame_num = 0, Display_Time = 30;
DWORD CDATA_Line = 0;
DWORD FrameMainStart = 0, FramePipStart= 0, FrameMainEnd = 0, FramePipEnd = 0;
DWORD FrameStoreStart[8], FrameStoreEnd[8], Photo_Index = 0, Photo_Select = 0;
void Capture_Video_Handler(void);
#endif

//******************************************************************************
//  S T A T I C    F U N C T I O N S
//******************************************************************************

static void ShowNvramMap(void);
//static void ShowSdramMap(void);
static void EnableDisableInput(void);
static void DebugSelectInput(void);
static void ChangeOutputSize(void);
static void ChangeOutputPosition(void);
static void ChangeCaptureMain(void);
static void ChangePipBlending(void);
static void ChangeFrameLockMode(void);
static void SelectVideoFilter(void);
static void DebugModeSetup(void);
static void DisplayNativeMode(void);
static void DebugScalingAndAfmWindow(void);
static void Pixel_Scan(void);
static void SelectPanel(void);
static void EnableDisableDetection(void);
static void DebugZoom(void);
static void DebugAdjustGamma(void);
static void ReadOSDTileInfo(void);
static void SetOSDTilePosition(void);
static void OverDriveTransition(void);
static void TestVideoProcessing(void);
static void TestEngineeringAdjusters(BYTE B_Adjuster, WORD W_AdjValue);
static void SetupPatGen (gmt_PHY_CH B_Channel, BYTE B_PatNum);
static void PatGenOverride (void);
//static void SetupInputOutputForTPG (void);

#ifdef VPX322xF_DEV_ID
static void VpxReadWriteI2CReg         (void);
static void VpxReadWriteFPReg         (void);
static void VpxSetupMainPip            (void);
#endif   // #ifdef VPX322xF_DEV_ID

#ifdef PANEL_120_OR_100_HZ
// Which 100Hz/120Hz improvement method is enabled.
static void Force_Display_Window_With_Black_Color(void);
static void Force_Display_Window_With_Grey_Color(void);
static void Load_ACC_High_And_Low_Luma_Luts (void);
#endif

void MiscTest(BYTE index);


#ifdef ENABLE_RECORD_DATA
extern BYTE Lost_Count;
extern BYTE Lost_Data[10][24];
#endif

#ifndef RELEASE_CUSTOMER_SRAM
BYTE TEST_BUFFER[5760]; // Reserved for customer use
#endif

#if (DDR_Timing_DBG == ENABLE)
extern WORD Base_Addr_Lo, Base_Addr_Hi, Addr_Mask_Lo, Addr_Mask_Hi;
extern WORD LUT_Hi, LUT_Lo, TCL1;
#endif

#ifndef NO_PIP_SUPPORT
extern void PipDemoFunc(BYTE Param0, BYTE Param1);
extern void PIP_Demo_Handler(void);
void InputCaptureTest_PIP(void);

extern BYTE PIP_Index;
extern BYTE PIP_sub;
extern BYTE PIP_Oder;
#endif

void InputCaptureTest(void);

#if (FEATURE_OVERDRIVE == ENABLE)
extern WORD const OD_LUT1[];
extern WORD const OD_LUT2[];
#endif

//******************************************************************************
//  C O D E
//******************************************************************************

void DpRxGetTimeSlot(void)
{
	DWORD dwTemp = 0 ; 
	BYTE cCount = 0; 

	for (cCount=0; cCount<16;cCount++) 
	{
		dwTemp = gm_ReadRegDWord(DP12RX_PLNUM_TSLOT_0 + (4*cCount));
		gm_Printf("PLNUM_TIMESLOT_%d: %x%x",cCount*4 ,(WORD)(dwTemp>>16),(WORD)dwTemp);
	}
		 
	gm_Print("=====PAY LOAD ASSIGNMENT====",0);
	for (cCount=0; cCount<16;cCount++) 
	{
		dwTemp = gm_ReadRegDWord(DP12RX_PLNUM_PL1 + (4*cCount));
		gm_Printf("PAYLOAD %d:%x%x",cCount+1 ,(WORD)(dwTemp>>16),(WORD)dwTemp);
	}
		 
	gm_Print("=====MST ECF=====", 0); 
	gm_Print("MST_ECF_0:%x", gm_ReadRegDWord(DP12RX0_MST_ECF_0)); 
	gm_Print("MST_ECF_1:%x", gm_ReadRegDWord(DP12RX0_MST_ECF_1)); 

	gm_Print("====LINK STATUS===",0) ; 
	gm_Print("LINK STATUS:%x", gm_ReadRegDWord(DP12RX0_DPCD_STATUS)) ; 
	gm_Print("LINK TRAINING STATUS:%x",gm_ReadRegDWord(DP12RX0_LT_STATUS)); 
}

#ifdef BATHTUB_COLLECTION
void DPRxMeasEye_Symbol( BYTE B_Lane, BYTE B_MeasTime )
{
   WORD W_CPTrim, W_Ctrl1, W_Ctrl2;
   BYTE B_CPCount = 0;
   WORD W_ErrorCount;

   //UNUSED_VAR( B_MeasType );

   // Check inputs
   if ( B_Lane > 3 )
   {
      msg_i("+++Lane number OOR. Setting to 0.", 0);
      B_Lane = 0;
   }

   //msg("Bathtub curve for Lane %d", B_Lane);
   //msg("Time %dms", B_MeasTime*10);  //paul
   msg_i("Lane%d", B_Lane);  //paul

   // Initialize register addresses based on lane number
   switch ( B_Lane )
   {
      case 0:
         W_CPTrim = DPRX_L0_CPTRIM;
         W_Ctrl1 = DPRX_L0_CTRL_1;
         W_Ctrl2 = DPRX_L0_CTRL_2;
         break;

      case 1:
         W_CPTrim = DPRX_L1_CPTRIM;
         W_Ctrl1 = DPRX_L1_CTRL_1;
         W_Ctrl2 = DPRX_L1_CTRL_2;
         break;

      case 2:
         W_CPTrim = DPRX_L2_CPTRIM;
         W_Ctrl1 = DPRX_L2_CTRL_1;
         W_Ctrl2 = DPRX_L2_CTRL_2;
         break;

      case 3:
         W_CPTrim = DPRX_L3_CPTRIM;
         W_Ctrl1 = DPRX_L3_CTRL_1;
         W_Ctrl2 = DPRX_L3_CTRL_2;
         break;

      default:       // Set default to Lane 0
         W_CPTrim = DPRX_L0_CPTRIM;
         W_Ctrl1 = DPRX_L0_CTRL_1;
         W_Ctrl2 = DPRX_L0_CTRL_2;
         break;
   }

   //msg("Current CPTrim 0x%x", gm_ReadRegByte( W_CPTrim ) );
   msg_i("%x", gm_ReadRegByte( W_CPTrim ) );

   // Set up for manual CPTrim adjustment and SATA mode
#if 1	// For Chandler, DPRX_L0_DIS_TRIM_CAL controls all 4 lanes
   gm_SetRegBitsByte(DPRX_L0_CTRL_2, DPRX_L0_DIS_TRIM_CAL);
   gm_SetRegBitsByte(W_Ctrl2, DPRX_L0_FD_TEST);
#else
   gm_SetRegBitsByte( W_Ctrl2, (DPRX_L0_FD_TEST | DPRX_L0_DIS_TRIM_CAL) );
#endif
   // Select lane to take measurements on
   gm_ClearAndSetBitsByte( DPRX_ERR_CNT_CTRL, DPRX_ERR_CNT_SEL, (B_Lane << 1/*DPRX_ERR_CNT_SEL_SHIFT*/) );
   // Enable PRBS error testing use Symbol Error counter
   //gm_SetRegBitsByte( DPRX_LINK_CONTROL_1, DPRX_PRBS_EN );
   // Clear error counter to begin with
   gm_ClearRegBitsByte( DPRX_ERR_CNT_CTRL, DPRX_ERR_CNT_EN );

   // Perform the measurement
   while ( B_CPCount < 64 )
   {
      gm_ClearAndSetBitsByte( W_Ctrl1, DPRX_L0_CPTIN, B_CPCount );
      gm_ClearRegBitsByte( DPRX_ERR_CNT_CTRL, DPRX_ERR_CNT_EN );
      gm_SetRegBitsByte( DPRX_ERR_CNT_CTRL, DPRX_ERR_CNT_EN );
      gm_Delay10ms( B_MeasTime );        // Delay for 0.5 sec
      W_ErrorCount = gm_ReadRegWord( DPRX_ERROR_CNT_0 );
      //msg(" W_ErrorCount 0x%x", W_ErrorCount );
      if ( W_ErrorCount & 0x8000)
      {
         msg_i(" %d", (W_ErrorCount & 0x7FFF) );
      }
      else
      {
         msg_i("+++Error count is invalid", 0);
      }
      B_CPCount++;
   }

#if 1	// Set up to Auto CPTrim
   gm_ClearRegBitsByte(DPRX_L0_CTRL_2, DPRX_L0_DIS_TRIM_CAL);
#endif
}
#endif	// BATHTUB_COLLECTION

extern void gm_AUDIO_Appstest(void);
BYTE far WriteLogicalPortBksvToOtp(BYTE far *Bp_Data, BYTE type);
BOOL far GetLogicalPortBksv(BYTE far * Bksv_p8);

//******************************************************************************
//
// FUNCTION       : void UserExtendableCmd(HANDLER_USE_t B_whichHandler)
//
// USAGE          : User definable extension commands from gProbe
//
// INPUT          : B_whichHandler - Communication channel being used:
//                     SERIAL_USE (serial port),
//                     DDC2BI_USE (DC2bi port)
//
// OUTPUT         : None
//
// GLOBALS        : gExtDebugCmdPtr  (R)
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void UserExtendableCmd(HANDLER_USE_t B_whichHandler, BYTE B_TestNum)
{
   B_whichHandler = B_whichHandler;

   switch (B_TestNum)
   {
      case Func0:
         if (GetParam(0) == 1)
            InitializeNvram();      // Re-initialize NVRAM
         else
            gm_Print("'appstest 0 1' to init Nvram",0);
         break;

#ifdef FULL_APPSTEST_CMD
      case Func1:
         EnableDisableInput();   // Enable/disbale input valid mode
         break;
#endif
      case Func2:
         DebugSelectInput();      // This supports choosing input
         break;

#ifdef FULL_APPSTEST_CMD
      case Func3:
         ChangeOutputSize();      // Vary the size of main and pip channels
         break;

      case Func4:
         ChangeOutputPosition();   // Vary the position of main and pip
         break;

      case Func5:
         ChangeCaptureMain();   // Change Capture Main
         break;
#endif

      case Func6:
      #if FEATURE_AIP == ENABLE
         if (GetChannelConnType(gmvb_MainChannel) == COMPONENT)
         {
            UserPrefADCCalibrationMethod_Comp = (BYTE)GetParam(0);
            gm_Print("Component Cal Method = %d",UserPrefADCCalibrationMethod_Comp);
         }
         else
         {
            UserPrefADCCalibrationMethod_VGA = (BYTE)GetParam(0);
            UserPrefADCCalibrationMethod_Comp = (BYTE)GetParam(0);
            gm_Print("VGA Cal Method = %d",UserPrefADCCalibrationMethod_VGA);
         }
         PerformAdcCalibration();
      #endif
         break;
         
#ifdef FULL_APPSTEST_CMD
      case Func7:
         EnableDisableDetection();
         break;

      case Func8:
         ShowNvramMap();         // Show NVRAM mapping
         break;

 #ifndef NO_PIP_SUPPORT
      case Func9:
         ChangePipBlending();   // Level of blending in case of single pip
         break;
 #endif

      case Func10:
         ChangeFrameLockMode();   // Frame Lock Mode
         break;

      case Func11:
         ShowSdramMap();         // Show SDRAM mapping
         break;

      case Func12:
         SelectVideoFilter();   // Select which H+V filters to load
         break;

      case Func13:
         break;

      case Func15:
         DebugZoom();
         break;
#endif // FULL_APPSTEST_CMD

      case Func14:
#if (FEATURE_HDMI==ENABLE) && defined(STHDMIRX_HDCP_ENABLE_RAM_LOG)
		gm_Print("Hdcp Ram Log",0);
		{
			U8 i;
			extern U8 DebugHdcpRep_a8[15];
			for (i=0;i<150;i++)
			{
				gm_Print("%x",DebugHdcpRep_a8[i]);
			}
		}
#endif
	  	break;

      case Func16:	//Reset 0/1 command: force reset and stay in IROM mode
         {
            WORD Value = 0;

            gm_Delay1ms(5);
            if ((BYTE)GetParam(0) == 0)  Value = 0x1234;
            gm_WriteRegWord(OCM_NR_SCRATCHPAD, Value);

            gm_ClearRegBitsWord(CLOCK_CONFIG1, OCM_CLK_SEL);
            gm_WriteRegWord(HARD_RESETS_2, 0xFFFF);
            gm_WriteRegWord(AUDIO_HARD_RESETS, 0xFFFF);
            //Reset LVDS to avoid DCLK from PLL is gated off inside LVDS loop that kill ODP source clock.
            gm_WriteRegWord(LVDS_RESET_CTRL, LVDSTX_HARD_RESET);
            gm_WriteRegWord(HARD_RESETS, 0xFFFF);
            break;
         }

#ifdef FULL_APPSTEST_CMD
      case Func17:
         DebugAdjustGamma();
         break;

      case Func18:
         MiscTest((BYTE)GetParam(0));
         break;

      case Func19:
         TestVideoProcessing();
         break;

#ifdef ISR_MONITOR_R_S_VALUES
      case Func20:
         StartRScollect();      // Start RS values collection into buffer
         break;
#endif

      case Func21:
         DebugScalingAndAfmWindow();
         break;

      case Func22:
         DebugModeSetup();      // Debug Mode Setup
         break;

      case Func23:
         ModeSetupUserPref(CH_A); // Perform Mode Setup User Pref
         break;

      case Func24:
         DisplayNativeMode();   // Centers image on display (native resolution)
         break;
         
 #ifndef NO_PIP_SUPPORT
      case Func25:
         DebugAdjustPip();
         break;

      case Func26:
         SwapMainPip();
         break;
 #endif
#endif // FULL_APPSTEST_CMD

		case Func27:
         #ifdef ST_4K2K_93xx_BOARD
            if(GetChipID_4K2K()==CHIPID_4K2K_R)
               {
                  gm_Delay1ms(10); //Add by ChengLin for avoid GProbe hang up after UART Port switch
                  External_UART_Switch((BYTE)GetParam(0));
               }
		 #endif
		 	break;
		case Func28:
         #ifdef ST_4K2K_93xx_BOARD
            if(GetChipID_4K2K()==CHIPID_4K2K_R)
			{
				UserPrefPBPMode = (BYTE)GetParam(0);
        		AdjustPBP();
			}
      	#endif
         break;
		

#if (FEATURE_DDR_EXECUTION == ENABLE) && (DDR_TEST_CODE == ENABLE)
      case Func30:
         {
            BYTE B_Flag;

            switch (GetParam(0))
            {
               case 0:
                  SetDdrPhyDly_Handler((BYTE)GetParam(1), (BYTE)GetParam(2), (BYTE)GetParam(3), (BYTE)GetParam(4));
                  break;

               case 10:	// Initiate Memory Interface
                  InitMemIF();
                  break;

               case 30:	// PCTL register programming
                  PctlRegProgramming((WORD)GetParam(1), GetParam(2));
                  break;

               case 66:
                  MemIFTest(DLY_QUALIFY, (BYTE)GetParam(1), 0, 0);
                  break;

					case 68:
						MemIFTest(DLY_DataBits, (BYTE)GetParam(1), 0, 0);
                  break;

               case 101:
                  DdrTest_DataBit((BYTE)GetParam(1), (BYTE)GetParam(2), (BYTE)GetParam(3));
                  break;

               case 108:
                  DdrTest_LongHours(GetParam(1));
                  break;

               case 110:
                  DDR_RW_Flag = (BYTE)GetParam(1);
                  break;
            }
         }
         break;
#endif

#ifdef FULL_APPSTEST_CMD
      case Func31:
         ReadOSDTileInfo();
         break;

      case Func32:
         SetOSDTilePosition();
         break;

      case Func33:
         {
            BYTE B_Data = (BYTE)GetParam(0);

            if (B_Data == CSC_FULL_EXPANSION)
               msg("CSC Full Expansion");
            else if (B_Data == CSC_LINEAR_EXPANSION)
               msg("CSC Linear Expansion");
            else if (B_Data == CSC_PARTIAL_EXPANSION)
               msg("CSC Partial Expansion");
            else
               msg("CSC - Invalid selection");
            SelectCscMatrix(B_Data);
         }
         break;

      case Func34:
#if (FEATURE_OVERDRIVE == ENABLE)
#if 1
         {
#define RED_LUT1_MEM_START 0x20000L
#define RED_LUT2_MEM_START 0x21000L
#define GRN_LUT1_MEM_START 0x22000L
#define GRN_LUT2_MEM_START 0x23000L
#define BLU_LUT1_MEM_START 0x24000L
#define BLU_LUT2_MEM_START 0x25000L
            //#define SEGBIT2SHIFT 4 // 20-bit mode
#define SEGBIT2SHIFT 8 // 24-bit mode

#define LINEAR_TO_FPTR_B(Linear_Address) ((BYTE far *)\
				((void _seg *)(((WORD)(Linear_Address >> 16)) << (16 - SEGBIT2SHIFT)) +\
				(void __near *)((WORD)Linear_Address)))


#define LINEAR_TO_FPTR_W(Linear_Address) ((WORD far *)\
				((void _seg *)(((WORD)(Linear_Address >> 16)) << (16 - SEGBIT2SHIFT)) +\
				(void __near *)((WORD)Linear_Address)))

            if (GetParam(0) != OD_LOAD_TBL)
            {
               gm_Printf("GetParam(0) = %d", GetParam(0));
               gm_UpdateOverDrive((BYTE)GetParam(0), 0, 0);
            }
            else
            {
               WORD far *Wp_Ptr = LINEAR_TO_FPTR_W(RED_LUT1_MEM_START);
               WORD W_Count,W_Index;
               gm_Printf("Load OD Table to memory", 0);

               for ( W_Count = 0,W_Index=0; W_Count < 0x1000/2 ; )
               {
                  Wp_Ptr[W_Count] = OD_LUT1[W_Index];
                  W_Count++;
                  W_Index++;
                  if ((W_Count % 8) == 5)
                     W_Count+=3;
               }

               Wp_Ptr = LINEAR_TO_FPTR_W(GRN_LUT1_MEM_START);

               for ( W_Count = 0,W_Index=0; W_Count < 0x1000/2 ; )
               {
                  Wp_Ptr[W_Count] = OD_LUT1[W_Index];
                  W_Count++;
                  W_Index++;
                  if ((W_Count % 8) == 5)
                     W_Count+=3;
               }

               Wp_Ptr = LINEAR_TO_FPTR_W(BLU_LUT1_MEM_START);

               for ( W_Count = 0,W_Index=0; W_Count < 0x1000/2 ; )
               {
                  Wp_Ptr[W_Count] = OD_LUT1[W_Index];
                  W_Count++;
                  W_Index++;
                  if ((W_Count % 8) == 5)
                     W_Count+=3;
               }

               Wp_Ptr = LINEAR_TO_FPTR_W(RED_LUT2_MEM_START);

               for ( W_Count = 0,W_Index=0; W_Count < 0x1000/2 ; )
               {
                  Wp_Ptr[W_Count] = OD_LUT2[W_Index];
                  W_Count++;
                  W_Index++;
                  if ((W_Count % 8) == 3)
                     W_Count+=5;
               }

               Wp_Ptr = LINEAR_TO_FPTR_W(GRN_LUT2_MEM_START);

               for ( W_Count = 0,W_Index=0; W_Count < 0x1000/2 ; )
               {
                  Wp_Ptr[W_Count] = OD_LUT2[W_Index];
                  W_Count++;
                  W_Index++;
                  if ((W_Count % 8) == 3)
                     W_Count+=5;
               }

               Wp_Ptr = LINEAR_TO_FPTR_W(BLU_LUT2_MEM_START);

               for ( W_Count = 0,W_Index=0; W_Count < 0x1000/2 ; )
               {
                  Wp_Ptr[W_Count] = OD_LUT2[W_Index];
                  W_Count++;
                  W_Index++;
                  if ((W_Count % 8) == 3)
                     W_Count+=5;
               }

               gm_Printf(" OD Table Loaded ...", 0);
            }
         }
#else
         OverDriveTransition();
#endif // 1
#endif // (FEATURE_OVERDRIVE == ENABLE)
         break;

      case Func35:   //DUM test
         {
            WORD DUM_XGRID_SIZE1, DUM_YGRID_SIZE1, DUM_NUM_XGRIDS1, DUM_NUM_YGRIDS1, DUM_RSTART_XGRID1;
            DWORD DUM_XGRID_IINC1, DUM_YGRID_IINC1;

            gm_ClearRegBitsWord(C9VODP_DUM_CONTROL_0, DUM_ENABLE);
            _fmemset(LINEAR_TO_FPTR(0x2E000L),0,0x2000);
            gm_WriteRegWord(IMP_PATGEN_CONTROL, 0xE0);
            gm_WriteRegWord(IMP_PATGEN_RED, 0xFF);
            gm_WriteRegWord(IMP_PATGEN_GRN, 0xFF);
            gm_WriteRegWord(IMP_PATGEN_BLUE, 0xFF);

#if 0 // 1 // Set Size
            DUM_XGRID_SIZE1 = (WORD)GetParam(0); //16;
            DUM_NUM_XGRIDS1 = (PanelWidth / DUM_XGRID_SIZE1)+1;
            DUM_YGRID_SIZE1 = (WORD)GetParam(1); //16;
            DUM_NUM_YGRIDS1 = (PanelHeight / DUM_YGRID_SIZE1)+1;
#else // Set Grid
            DUM_NUM_XGRIDS1 = (WORD)GetParam(0); //16;
            DUM_NUM_YGRIDS1 = (WORD)GetParam(1); //16;

            DUM_XGRID_SIZE1 = PanelWidth / (DUM_NUM_XGRIDS1 - 1);
            DUM_YGRID_SIZE1 = PanelHeight / (DUM_NUM_YGRIDS1 - 1);           
#endif

            gm_Printf("x grid : %d", DUM_NUM_XGRIDS1);
            gm_Printf("y grid : %d", DUM_NUM_YGRIDS1);
            gm_Printf("x size : %d", DUM_XGRID_SIZE1);
            gm_Printf("y size : %d", DUM_YGRID_SIZE1);


            DUM_XGRID_IINC1 = (DWORD)(524288 / DUM_XGRID_SIZE1);
            DUM_YGRID_IINC1 = (DWORD)(524288 / DUM_YGRID_SIZE1);

            DUM_RSTART_XGRID1 = (DUM_NUM_XGRIDS1 / 2);

            if ((BYTE)GetParam(2))
               gm_WriteRegWord(C9VODP_DUM_CONTROL_0, (DUM_ENABLE|DUM_GRID_DOTS|DUM_3_COLOR));
            else
               gm_WriteRegWord(C9VODP_DUM_CONTROL_0, (DUM_ENABLE|DUM_GRID_DOTS));

            // Also can enable (DUM_GRID_DOTS | DUM_3_COLOR for test Dots and color)

            gm_WriteRegDWord_LE(C9VODP_DUM_NUM_GRIDS, ((DWORD)DUM_NUM_YGRIDS1 << 9)|DUM_NUM_XGRIDS1);
            gm_WriteRegDWord_LE(C9VODP_DUM_GRID_SZ, ((DWORD)DUM_YGRID_SIZE1 << 12)|(DUM_XGRID_SIZE1));
            gm_WriteRegWord(C9VODP_DUM_GLOBAL0, 0xFFFF);
            gm_WriteRegWord(C9VODP_DUM_GLOBAL0 + 2, 0x03FF);
            gm_WriteRegWord(C9VODP_DUM_GLOBAL1, 0x1FFF);
            gm_WriteRegDWord_LE(C9VODP_DUM_XGRID_IINC, DUM_XGRID_IINC1);
            gm_WriteRegDWord_LE(C9VODP_DUM_YGRID_IINC, DUM_YGRID_IINC1);
            gm_WriteRegWord(C9VODP_DUM_RSTART, DUM_RSTART_XGRID1);
         }
         break;


      case Func36:   //invalid value set
         // 20110718
         if (((GetChannelConnType(CH_A) == HDMI) && (gm_IsHdmiTiming(gB_CurrentInputConnector[CH_A])))
               || (GetChannelConnType(CH_A) == CT_DISPLAY_PORT))
         {
            gm_Print("HDMI and DP port get input color domain by Info frame", 0);
            return;
         }

         if (((BYTE)GetParam(0) > 2) || ((BYTE)GetParam(1) > 4))
         {
            gm_Print("BYTE 0 --> Input color domain select to 0: Default 1: RGB, 2: YUV", 0);
            gm_Print("BYTE 1 --> Output process type set to 0: Default 1: Graphice, 2: SD Video, 3: HD Video, 4: ED Video", 0);
         }
         else
         {
            Input_Color_Domain = (BYTE)GetParam(0);
            Output_Color_type = (BYTE)GetParam(1);

            ModeSetupRequest(gmvb_MainChannel, REQ_MS_RESTART, PARAM_NONE, 100);
         }
         break;

      case Func37:
#if (FEATURE_AUDIO == ENABLE)
         gm_AUDIO_Appstest();
#endif
         break;

      case Func38:
         BypassMemory();
         break;

#ifndef NO_PIP_SUPPORT
      case Func39:
         PipDemoFunc((BYTE)GetParam(0),(BYTE)GetParam(1));
         break;
#endif // NO_PIP_SUPPORT

      case Func40:   // VWD control
#if (FEATURE_VWD == ENABLE)
         UserPrefVwdFlag = (BYTE)GetParam(0);
         if (UserPrefVwdFlag == ON)
            VWD_Start();
         else
            VWD_Stop();
#endif
         break;

      case Func41:   // Pixel crunch test
#if (FEATURE_VWD == ENABLE)
         Vwd_Start_Test((BYTE)GetParam(0));
#endif
         break;

      case Func42:
#if (FEATURE_VWD == ENABLE)
         UserPrefVwdDemo = (BYTE)GetParam(0);
         gm_Print("Set VWD Demo Mode - %d",UserPrefVwdDemo);
         VWD_ApplyHighLightWindow();
#endif
         break;

      case Func43:
#if (FEATURE_ACR == ENABLE)
      UserPrefDCR = (BYTE)GetParam(0);
      if (UserPrefDCR == ON)
         ACR_Enable();
      else
         ACR_Disable();
#endif
         break;

      case Func44:
         {
            BYTE ch;
            ch=(BYTE)GetParam(0);
            gm_Print("LBADC %d = %d",ch,gm_ScanAdcKey(ch));
         }
         break;
#endif // FULL_APPSTEST_CMD

      case Func45:
         {
            CableInfo CableUpdate;
            Get_CableStatus(&CableUpdate);
            gm_Print("PHY1",0);
            gm_Print("  Cable Status = %d", CableUpdate.PHY1_CableStatus);
            gm_Print("  Cable Source power  = %d", CableUpdate.PHY1_SourcePowerStatus);
            gm_Print("PHY2",0);
            gm_Print("  Cable Status = %d", CableUpdate.PHY2_CableStatus);
            gm_Print("  Cable Source power = %d", CableUpdate.PHY2_SourcePowerStatus);
            gm_Print("PHY3",0);
            gm_Print("  Cable Status = %d", CableUpdate.PHY3_CableStatus);
            gm_Print("  Cable Source power = %d", CableUpdate.PHY3_SourcePowerStatus);
            gm_Print("VGA",0);
            gm_Print("  Cable Status = %d", CableUpdate.VGA_CableStatus);
            gm_Print("  Cable Source  power= %d", CableUpdate.VGA_SourcePowerStatus);
            #ifdef ST_4K2K_93xx_BOARD
            if(GetChipID_4K2K()==CHIPID_4K2K_R)
            {
            gm_Print("MiniDp",0);
            gm_Print("  Cable Status = %d", CableUpdate.MiniDP_CableStatus);
            gm_Print("  Cable Source  power= %d", CableUpdate.MiniDP_SourcePowerStatus);
            }
            #endif
            
            
         }
         break;
      case Func46:
         // For 3D debug and testing only
#if (FEATURE_3D == ENABLE)
         gm_3D_Appstest();
#endif
         break;

      case Func47:
         gm_Printf("Calculating checksum...this process may take around 40 seconds");
         gm_Printf("Checksum = %d",  CalculateCodeCrc());
         break;

      case Func48:
         ModeSetupRequest(gmvb_MainChannel, REQ_MS_RESTART, PARAM_NONE, 0);
         gm_Print("REQ_MS_RESTART",0);
         break;

#if (FEATURE_HDMI == ENABLE)
      case Func49:
               {
                  HdmiDviHpdControl(UserPrefCurrentInputMain, (BYTE)GetParam(0));
               }
               break;

      case Func50:
         if ((GetPortConnType(UserPrefCurrentInputMain)==HDMI)||(GetPortConnType(UserPrefCurrentInputMain)==DVI))
         {
            gm_Print("=====Main Channel=====",0);
            #if (LINK_CLOCK_CHECK_SUPPORT ==1)
            gm_Print("===HdmiDviLinkClockStatus    :(%d)===",UserPrefCurrentInputMain);
            gm_Print("LinkClockPsent:%d",HdmiDviIsLinkClockPresent(UserPrefCurrentInputMain));
            #endif
            HdmiDviPrintSignalProperty(UserPrefCurrentInputMain);
            HdmiDviPrintVideoProperty(UserPrefCurrentInputMain);
            HdmiDviPrintAudioProperty(UserPrefCurrentInputMain);
         }
         if ((GetPortConnType(UserPrefCurrentInputPip)==HDMI)||(GetPortConnType(UserPrefCurrentInputPip)==DVI))
         {
            gm_Print("=====Pip Channel=====",0);
            #if (LINK_CLOCK_CHECK_SUPPORT ==1)
            gm_Print("===HdmiDviLinkClockStatus    :(%d)===",UserPrefCurrentInputPip);
            gm_Print("LinkClockPsent:%d",HdmiDviIsLinkClockPresent(UserPrefCurrentInputPip));            
            #endif
            HdmiDviPrintSignalProperty(UserPrefCurrentInputPip);
            HdmiDviPrintVideoProperty(UserPrefCurrentInputPip);
            HdmiDviPrintAudioProperty(UserPrefCurrentInputPip);
         }
         break;
#endif

#if (FEATURE_DISPLAY_PORT == ENABLE)
      case Func51:
	#ifdef ST_4K2K_93xx_BOARD
         gm_Print("=====Port 10======",0);
         DpRxPrintSignalProperty(ALL_BOARDS_DP1);
         DpRxPrintAudioProperty(ALL_BOARDS_DP1);

         gm_Print("=====Port 11======",0);
         DpRxPrintSignalProperty(ALL_BOARDS_DP2);
         DpRxPrintAudioProperty(ALL_BOARDS_DP2);
	#else
         if (GetPortConnType(UserPrefCurrentInputMain)==CT_DISPLAY_PORT)
         {
            gm_Print("=====Main Channel=====",0);
            DpRxPrintSignalProperty(UserPrefCurrentInputMain);
            DpRxPrintAudioProperty(UserPrefCurrentInputMain);
         }
         if (GetPortConnType(UserPrefCurrentInputPip)==CT_DISPLAY_PORT)
         {
            gm_Print("=====Pip Channel=====",0);
            DpRxPrintSignalProperty(UserPrefCurrentInputPip);
            DpRxPrintAudioProperty(UserPrefCurrentInputPip);
         }

		 if((gm_ReadRegDWord(DP12RX_RL_CONFIG)& DPRX_TYPE) && \
			((gm_GetDpDigitalID(gB_CurrentInputConnector[gmvb_MainChannel])==DID_DPRX0)||(gm_GetDpDigitalID(gB_CurrentInputConnector[gmvb_PipChannel])==DID_DPRX0)))
		 {
			 gm_Print("=====TIME SLOT ASSIGNMENT====",0); 
			 DpRxGetTimeSlot(); 
		 }
	#endif		 
         break;

 #if ENABLE_HPD_OVERRIDED
      case Func52:
         if (GetPortConnType(UserPrefCurrentInputMain)==CT_DISPLAY_PORT)
         {
            BYTE B_Param0 = (BYTE)GetParam(0);
            gmt_DPRX_DID b_DID=gm_GetDpDigitalID(UserPrefCurrentInputMain);

            switch(B_Param0)
            {
               case 0:
                  ForceDpHpd2Low(b_DID,TRUE);
                  gm_Print("Set RX%d HPD LOW",b_DID);                  
                  break;

               case 1:
                  ForceDpHpd2Low(b_DID,FALSE);
                  gm_Print("Set RX%d HPD HIGH",b_DID);                                    
                  break;

               default:
                  gm_Print("Set RX%d HPD LONG PULSE!",b_DID);
                  ForceDpHpd2Low(b_DID,TRUE);
                  SetForceHpdLowFlag(b_DID, TRUE);
                  if(b_DID==DID_DPRX0)
                     gm_TimerStart(RX0_HPD_OVERRIDED_10ms_TMR,HPD_OVERRIDED_TIMEx10ms);
                  else if(b_DID==DID_DPRX1)
                     gm_TimerStart(RX1_HPD_OVERRIDED_10ms_TMR,HPD_OVERRIDED_TIMEx10ms);
            }
         }
         break;
 #endif

#if (SELECT_DPTX_TYPE==USE_DPTX_DYNAMIC_SWITCH) || (SELECT_DPTX_TYPE==USE_DPTX_MANUAL_SWITCH)
      case Func53:
         {
            BYTE B_Param0 = (BYTE)GetParam(0);
            switch(B_Param0)
            {
               case 1:
               gm_Print("DPTX switch to ODP path (SstTx)",0);
               DPTxPortSwitch (MstTxPort, iDpTxPort);
               break;

               case 2:
               gm_Print("DPTX switch to Routing Logic path (MstTx)",0);
               DPTxPortSwitch (iDpTxPort,MstTxPort);
               break;

               case 3:
               gm_Print("Active ODP path (SstTx)",0);               
               DP12_ActivateSstTxPort(iDpTxPort);
               break;

               case 4: 
               gm_Print("Deactive ODP path (SstTx)",0);               
               DP12_DeactivateSstTxPort(iDpTxPort);
               break;

               case 5:
               DP12_DeactivateMstSstTxPorts(MstTxPort);
               DP12_DeactivateMstSstTxPorts(iDpTxPort); 
               Reset_Pre_PortIdToActivate();
               gm_Print("Deactive mst and sst",0);
               break;
                                   case 6://test0106
                                   UserPrefDPSWitch = 0;
                                        AdjustDPSwitch();
                                           break;

                                   case 7:
                                   UserPrefDPSWitch = 1;
                                        AdjustDPSwitch();
                                           break;//test0106


               default:
               gm_Print("======Menu of DPTX switch======",0);
               gm_Print("1: Switch to ODP path (SstTx)",0);
               gm_Print("2: Switch to RL  path (MstTx)",0);
               gm_Print("3: Active ODP path (SstTx)",0); 
               gm_Print("4: Deactive ODP path (SstTx)",0);  
               gm_Print("5: Deactive mst and sst",0);
            }
         }
         break;
#endif
      case Func54:
         {
            BYTE B_Param0 = (BYTE)GetParam(0);
            if (B_Param0==0)
            {
               PrintDpPwrCheck(DID_DPRX0);
            }
            else
            {
               PrintDpPwrCheck(DID_DPRX1);
            }
            break;
         }


      case Func55:
         {
            DP_SendVSCPacket(iDpTxPort, (BYTE)GetParam(0));
            DP_SetLeftRight(iDpTxPort, (BYTE)GetParam(1));
            DPTX_EnableVideo(iDpTxPort);
            gm_Print("ThreeDFormat=%d",(BYTE)GetParam(0));
            gm_Print("ThreeDView=%d",(BYTE)GetParam(1));
         }
         break;
#endif // (FEATURE_DISPLAY_PORT == ENABLE)
   case Func56:
      if((BYTE)GetParam(0)==0)
      {
         gm_Print("Disable Temporary Ignore Sleep",0);
         SystemFlags.sys.IgnoreSleep = FALSE;
         OSDRequestMsg(gmvb_MainChannel, MTO_ExitPowerDeepSleep, REQUEST_NOW);
      }
      else
      {
         gm_Print("Enable Temporary Ignore Sleep",0);
         SystemFlags.sys.IgnoreSleep = TRUE;
      }
      break;

#ifdef FULL_APPSTEST_CMD
 #ifdef SPIFLASH_CTRL_DRV
      case Func57:
      {
         SerialFlashInit();
         Flash_EraseSector(GetParam(0),(WORD)GetParam(1),(BYTE)GetParam(2));
         break;
      }
 #endif

#if (FEATURE_HDMI==ENABLE) 
   #if (DEBUG_HDMI_CPHY_CAPTURE !=0)
      case 58:
         PrintCphyRegs();
         break;
   #endif

   #if (DEBUG_HDMI_EQ_BATHTUB !=0)
      case 59:
      {
         //BYTE B_Param0 = (BYTE)GetParam(0);
         if(GetPortSignalType(gmvb_InputConnector[gmvb_MainChannel])==SIG_DVI)
         {
            Hdmi_RunEQ_Bathtub(gmvb_InputConnector[gmvb_MainChannel]);
            gm_Print("Finish!",0);
         }
      }
      break;
   #endif
         
#endif         

      case Func60:
         {
           BYTE B_Param0 = (BYTE)GetParam(0);
           BYTE Addr = (BYTE)GetParam(1);

                  if (gm_Write2WireBlock(1, B_Param0, (BYTE*) &Addr, 1, FALSE) == gmd_PASS)
                  {
                     gm_Printf("Write to 0x%x at offset 0x%x succeeded",B_Param0, Addr);
                  }
                  else
                  {
                     gm_Printf("Write to 0x%x at offset 0x%x failed",B_Param0, Addr);
                  }
                  if (gm_Read2WireBlock(1, B_Param0, (BYTE*) &Addr, 1, TRUE) == gmd_PASS)
                  {
                     gm_Printf("read from 0x%x succeeded, result=0x%x",B_Param0,Addr);
                  }
                  else
                  {
                     gm_Printf("read from 0x%x failed",B_Param0);
                  }         
            break;
         }

      case Func61:
         {
            BYTE index=0;
            BYTE Buffer = 1;

            while (index < 128)
            {
               if (gm_Write2WireBlock(0, index<<1, (BYTE*) &Buffer, 1, FALSE) == gmd_PASS)
               {
                  gm_Printf("I2C write responds addresses = 0x%x",index<<1);
               }
               if (gm_Read2WireBlock(0, index<<1, (BYTE*) &Buffer, 1, TRUE) == gmd_PASS)
               {
                  gm_Printf("I2C read responds addresses = 0x%x",index<<1);
               }
               index++;
            }
            break;
         }

      case Func62:
         {
            BYTE B_Param0 = (BYTE)GetParam(0);
            BYTE Buffer[2];

            Buffer[0] = (BYTE)GetParam(1);
            Buffer[1] = (BYTE)GetParam(2);

            if (gm_Write2WireBlock(1, B_Param0, (BYTE*)Buffer, 2, TRUE) == gmd_PASS)
            {
               gm_Printf("Write to 0x%x at offset 0x%x to value %x succeeded",B_Param0, Buffer[0], Buffer[1]);
            }
            else
            {
               gm_Printf("Write to 0x%x at offset 0x%x failed",B_Param0, Buffer[0]);
            }
            break;
         }

#endif // FULL_APPSTEST_CMD

#ifdef NVRAM_DP_EDID_ADDR
      case Func63:
         {
            WORD W_Counter;
            WORD W_Addr;
            WORD W_Size;
            BYTE B_Data[NVRAM_DP_EDID_SIZE];

            W_Addr = NVRAM_DP_EDID_ADDR;
            W_Size = NVRAM_DP_EDID_SIZE;

            for (W_Counter = 0; W_Counter < W_Size; W_Counter++)
               B_Data[W_Counter] = 0;

            gm_Printf("Reading address 0x%X", W_Addr);
            gm_Printf("Reading size %d", W_Size);

            NVRam_ReadBuffer(W_Addr, B_Data, W_Size);

            for (W_Counter = 0; W_Counter < W_Size; W_Counter++)
            {
               gm_Printf("Read data[%d] = 0x%X",W_Counter, B_Data[W_Counter] );
            }
         }
         break;

      case Func64:
         {
            WORD W_Counter;
            WORD W_Addr;
            WORD W_Size;
            WORD W_Command;
            W_Command = (WORD)GetParam(0);

            W_Addr = NVRAM_DP_EDID_ADDR;
            W_Size = NVRAM_DP_EDID_SIZE;

            gm_Printf("Writing address 0x%X", W_Addr);
            gm_Printf("Writing size %d", W_Size);
            
            if(W_Command !=1)
               _fmemcpy(TempBuffer,ST_EDID, sizeof(ST_EDID));

            NVRam_WriteBuffer(W_Addr, TempBuffer, W_Size);
         }
         break;
#endif

#ifdef FULL_APPSTEST_CMD
      case Func65:
         {
            Set_OCM_Freq((WORD)GetParam(0));
         }
         break;

 #if 1 // def OTP_APPS_TEST
      case Func66:
         {

         }
         break;

      case Func67:
         {
            gmt_RET_STAT W_Result;
            BOOL B_Result;
            B_Result = gm_OtpSramInit();
            if (B_Result == FALSE)
               gm_Printf("Something wrong in sram init!", 0);
            else
               gm_Printf("Reading sram data successfully", 0);
            W_Result = LoadHdcpKey();
            if (W_Result != OK)
               gm_Printf("Something wrong in Loading Hdcp key!", 0);
            else
               gm_Printf("Loading key successfully", 0);
         }
         break;

      case Func68:
         {

         }
         break;
 #endif //OTP_APPS_TEST

#if (FEATURE_HDMI == ENABLE)
      case Func69:
         // DrvId  : BIT0: Core , BIT1: Video , BIT2: Audio , BIT3: EQ
         // MsgType: 0: Info , 1: Warning , 2: Error >> If MsgType is ERROR, all ERROR msg will be printed without checking DrvId and MsgLevel.
         gm_Print("Hdmi Lib Debug Msg Control: DrvID[0x%X]  MsgType[%d]  MsgLevel[%d]",(BYTE)GetParam(0),(BYTE)GetParam(1),(BYTE)GetParam(2));
         STHDMIRX_DebugMsgControl((BYTE)GetParam(0), (BYTE)GetParam(1), (BYTE)GetParam(2));
         break;
#endif
         
         // Interlaced output control
         // Note: this code is explicitly written for YUV colorspace output through TTL
      case Func70:
         {
            BOOL Bt_Enable = (BOOL)GetParam(0)?1:0;
            BOOL Bt_FreeRunODD = (BOOL)GetParam(1)?1:0;
            BOOL Bt_InvertInternalODD = (BOOL)GetParam(2)?1:0;
            BOOL Bt_InvertOutputODD = (BOOL)GetParam(3)?1:0;

            gmt_PHY_CH B_Channel = CH_A;
            gmt_WindowStruct winDim;

            SelectCscMatrix(CSC_UNITY_MATRIX);
            SetupColorSpace(B_Channel);
            gm_ClearRegBitsWord(DISPLAY_CONTROL, INTERLACED_TIMING);
#if 0 // no need ..
            gm_DisplaySetupColorSpace(B_Channel, INSIDE_WINDOW_MULT_ENABLE
                                      |OUTSIDE_WINDOW_MULT_ENABLE
                                      |INSIDE_WINDOW_COLOR_YUV
                                      |OUTSIDE_WINDOW_COLOR_YUV);
#endif

            if (gm_IsGraphics(B_Channel))
               gm_SetRegBitsWord(OVP_CTRL, RGB2YUV_WIN_CTRL);
            else
               gm_ClearRegBitsWord(OVP_CTRL, RGB2YUV_WIN_CTRL);

            //This following code does not seem to have any effects
            /*winDim.W_Hstart = 10;
            winDim.W_Vstart = 1;
            winDim.W_Width = PanelWidth - winDim.W_Hstart;
            winDim.W_Height = PanelHeight - winDim.W_Vstart;
            gm_DisplayColorWindowSetup(&winDim, RGB_565(RGB(128,0,128)));
            gm_DisplaySetupColorSpace(B_Channel, INSIDE_WINDOW_MULT_ENABLE
                              |OUTSIDE_WINDOW_MULT_ENABLE
                              |INSIDE_WINDOW_COLOR_YUV
                              |OUTSIDE_WINDOW_COLOR_YUV);
            gm_SetRegBitsWord(DISPLAY_CONTROL, COLOR_WINDOW_EN);*/

            gm_WriteRegWord(INTERLACE_CTRL, (Bt_InvertOutputODD<<3) | (Bt_InvertInternalODD<<2) | (Bt_FreeRunODD<<1) | Bt_Enable);

            gm_SyncUpdateWait(B_Channel, INPUT_OUTPUT_CLOCK);
            gm_SetDisplayFreeRun();
            break;
         }

      case Func71:
         //STHdmiRx_SetRawSyncForInterlaced((BYTE)GetParam(0));
         //gm_Print("HDMI RawSyncStatus = %d",STHdmiRx_GetRawSyncForInterlaced());
         break;
         
#endif // FULL_APPSTEST_CMD

#ifdef SMART_ISP
      case 73:
         {
           // BYTE fBuf[PIX_BUF_SIZE+22];
           #define CODE_MAPPING_ADDRESS    0x400000UL
            #define BANK_SIZE_ADDR         0x410036UL
            DWORD Addr,codezide;
            DWORD count=0;
            BYTE PLbuf[BIN_BUF_SIZE+2];
            IspInfoStruct IspInfo;
            SmartIspFlashDriver FlashDrv;
            FlashDrv.FlashInit   = SerialFlashInit;
            FlashDrv.FlashErSectr= Flash_EraseSector;
            FlashDrv.WrBuf2Flash = WriteBuf2Flash;
            FlashDrv.GetFlashInfo= GetSpiFlashInfo;

            codezide= (*LINEAR_TO_FPTR_DW(BANK_SIZE_ADDR));
            //gm_Print("Code Size=0x%LX",codezide); 
            Addr=codezide+CODE_MAPPING_ADDRESS-1;
            //gm_Print("Addr=0x%LX",Addr);
          
            while((*LINEAR_TO_FPTR_B(Addr))==0xFF)
            {
               count++;
               Addr--;
            }
            if(count>=16)
               count-=16;
            else
               count=0;

            gm_Print("0xFF Skip Count=0x%LX",count);    

            IspInfo.OsdWirtePercentage=NULL_PTR;
            IspInfo.FmBinBufStart=gm_MemoryCodeStartAddress()+gmc_MemoryDataSize;
            IspInfo.LLNo=1;
            IspInfo.Loc[0].LocAddr=0x10000UL;
            IspInfo.Loc[0].LocLength=codezide-IspInfo.Loc[0].LocAddr-count;
            IspInfo.FwBinTotal=IspInfo.Loc[0].LocLength;
            if (Isp_LoadFlashDrv(&FlashDrv)==FALSE)
            {
               gm_Print("Load Flash Driver Failed!",0); 
               break;
            }
            if (Isp_FlashErase(0,0,SECT_ER_ALL, SISP_DUALFLASH_DISABLE)==FALSE) 
            {
               gm_Print("Flash Erase Failed!",0);
               break;
            }
            gm_Print("Flash Erase Success",0);

            if (Isp_FlashWrite(&IspInfo, PLbuf)==FALSE)
            {
               gm_Print("Flash Write Failed!",0);
               break;
            }

            EnableWriteProtection();
         }
         break;
         
      case 74:
         {
            BYTE fBuf[PIX_BUF_SIZE+22];
            IspInfoStruct IspInfo;
            WORD x,y,len;
            BYTE PrnMode;
            WORD MaxRdPixPerTime;

            PrnMode=(BYTE)GetParam(0);
            x=(WORD)GetParam(1);
            y=(WORD)GetParam(2);
            len=(WORD)GetParam(3);

            IspInfo.FrameWidth=stInput[gmvb_MainChannel].Hactive;
            IspInfo.FrameHeight=stInput[gmvb_MainChannel].Vactive;
            IspInfo.BurstAdjust=(BYTE)((BURST_SIZE-(IspInfo.FrameWidth*gmvb_MemoryBpp[gmvb_MainChannel]/BIT_PER_BYTE)%BURST_SIZE)& 0x3F);
            IspInfo.FrameBufStart=gm_MemoryGetInfo(BUF_MAIN, BUF_START_ADDRESS)* BURST_SIZE;
            IspInfo.BitsPerPix=gmvb_MemoryBpp[gmvb_MainChannel];
            MaxRdPixPerTime=PIX_BUF_SIZE/IspInfo.BitsPerPix*BIT_PER_BYTE;

            //gm_SetRegBitsWord(HOST_CONTROL,IMP_FORCE_UPDATE|ODP_FORCE_UPDATE);
            gm_Printf ("X=%d   Y=%d   len=%d",x,y,len);
            gm_Printf ("Main_Channel  : %d",gmvb_MainChannel);
            gm_Printf ("MemoryNumBuf  : %d",gmvb_MemoryNumBuffers[gmvb_MainChannel]);
            gm_Printf ("MemoryBpp     : %d",gmvb_MemoryBpp[gmvb_MainChannel]);
            gm_Printf ("FrameBufStart : 0x%lX (0x%lX)",IspInfo.FrameBufStart, IspInfo.FrameBufStart >> BURST_SIZE_SHIFT);
            gm_Printf ("Input Active  : %d x %d", IspInfo.FrameWidth, IspInfo.FrameHeight);
            gm_Printf ("BurstAdjust   : %d",IspInfo.BurstAdjust);
            gm_Printf ("BitsPerPix    : %d",IspInfo.BitsPerPix);

            if (PrnMode==PRINT_RAW)
               gm_Printf("<<Print Raw Data>>",0);
            else if (PrnMode==PRINT_HI_NIBBLE_COMBINE)
               gm_Printf("<<PRINT_HI_NIBBLE_COMBINE>>",0);
            else if ((PrnMode==PRINT_8BIT) || ((PrnMode==PRINT_BY_IN_DEPTH) && (IspInfo.BitsPerPix==24)))
               gm_Printf("<<PRINT_8BIT>>",0);
            else if (PrnMode == PRINT_YUV_20BIT)
            {
               gm_Printf("<<PRINT_YUV_20BIT>>",0);
               if (x & 0x01)
               {
                  gm_Printf("  x is not even number, decrease 1",0);
                  x--;
               }
               if (len > ((sizeof(fBuf)-22)*2/5))
               {
                  len = (sizeof(fBuf)-22)*2/5;
                  gm_Printf("  Length is over buffer size. Change to: %d",len);
               }
            }
            else
               gm_Printf("<<PRINT_10BIT>>",0);

            Isp_FrameCatch(&IspInfo, FCATCH_START);
            while (len)
            {
               WORD RdLen;
               if (len>MaxRdPixPerTime)
                  RdLen=MaxRdPixPerTime;
               else
                  RdLen=len;
               gm_Print("Addr---X(%d)",x);
               PrintDisplayPixelData(&IspInfo,PrnMode, x, y, RdLen, fBuf);
               gm_Delay10ms(10);
               len-=RdLen;
               x+=RdLen;
            }

            Isp_FrameCatch(&IspInfo, FCATCH_END);
            gm_Print("End",0);

         }
         break;

      case 75:
         IspThruDisplay(ISP_EXECUTE_DIRECT);
         break;

 #ifdef SMART_ISP_AUTO_DETECTION
      case 76:
         if ((BYTE)GetParam(0)==FALSE)
            EnableSmartIspDetect(FALSE);
         else
            EnableSmartIspDetect(TRUE);
         break;
 #endif
#endif // SMART_ISP

#ifdef FULL_APPSTEST_CMD
 #ifdef VIDEO_CAPTURE
      case 77:
         Freeze(gmvb_MainChannel, TRUE);
         CDATA_Flag = (WORD)GetParam(0);
         Photo_Select = (WORD)GetParam(1);
         break;
 #endif

 #ifndef NO_PIP_SUPPORT
      case 79:
         InputCaptureTest_PIP();
         break;   
 #endif

      case Func80:
         InputCaptureTest();
         break;

 #if (FEATURE_ACC3 == ENABLE)
      case Func81:
         {
            //gmt_ACC3Options op;
            //gmt_ACC3Params accp;
            switch ((BYTE)GetParam(0))
            {
               case 0: // Check ACC3 procssing tatus
                  gm_Printf(gm_ACC3_IsProcessingEnabled(TRUE) ? "ACC processingis running": "ACC proessing is disabled");
                  break;
               case 1: // turn on/off ACC
                  gm_Printf("Update ACC %d",GetParam(1));
                  gm_UpdateAcc((BYTE)GetParam(1));
                  break;
               case 2: // force ACC3 processing
                  gm_Printf(" Acc Process Frame",0);
                  gm_ACC3ProcessFrame();
                  break;
               case 3: // Init ACC3
                  gm_Printf(" Acc Init",0);
                  gm_ACC3Init();
                  break;
               case 4: // Enavle/desable continuous ACC3 procssing
                  {
                     gmt_ACC3ModeCtr Mc;
                     gm_ACC3GetControlParam(&Mc);

                     Mc.ProcessingEnable = GetParam(1) ? 1 : 0;
                     gm_ACC3SetParams(AccModeControlsSelect, (void far *)&Mc, sizeof(gmt_ACC3CtrlParamSelector));
                     gm_Printf("ProcessingEnable = %d", (WORD)Mc.ProcessingEnable);
                     break;
                  }
               case 5:
                  {
                     DWORD DW_Counter = 0;

                     gm_SetRegBitsWord(OVP_IRQ_MASK, OVP_MEOF_MASK);
                     gm_SetRegBitsWord(OVP_STATUS, OVP_MEOF_STATUS);

                     while (!(gm_ReadRegWord(OVP_STATUS)&OVP_MEOF_STATUS))
                     {
                        DW_Counter++;
                        gm_Delay1ms(1);
                     }
                     gm_Printf("OVP_MEOF 0x%x", DW_Counter);

                     DW_Counter = 0;
                     gm_SetRegBitsWord(OVP_IRQ_MASK, OVP_HISTOGRAM_READY_MASK);
                     gm_SetRegBitsWord(OVP_STATUS, OVP_HISTOGRAM_READY_STATUS);
                     while (!(gm_ReadRegWord(OVP_STATUS)&OVP_HISTOGRAM_READY_STATUS))
                     {
                        DW_Counter++;
                        gm_Delay1ms(1);
                     }
                     gm_Printf("OVP_HISTOGRAM_READY 0x%x", DW_Counter);

                  }
                  break;

               case 6:
                  {
#define ACC3_SLOPES_0_MEM_START 0x32C00L
                     //extern SDWORD SDW_SLOP[];
                     WORD W_Bin;

                     for (W_Bin=0 ; W_Bin < NUMBER_OF_BINS; W_Bin++)
                     {
                        DWORD DW_TempAddr = (DWORD)ACC3_SLOPES_0_MEM_START + W_Bin * 4;

                        /*
                        if(RenormSlope[W_Bin] != INTHIST_AVG_VAL)
                        	RenormSlope[W_Bin] = RenormSlope[W_Bin]<<4;
                        else
                        	RenormSlope[W_Bin] = RenormSlope[W_Bin]<<2;
                        */
 #if 1 // 1
                        *(LINEAR_TO_FPTR_DW(DW_TempAddr)) = SDW_SLOP_FIX[W_Bin];
 #else
                        *(LINEAR_TO_FPTR_DW(DW_TempAddr)) = RenormSlope[W_Bin];
 #endif
                     }
                  }
                  break;

               case 7:
                  {
#define ACC3_SLOPES_0_MEM_START 0x32C00L
                     //extern SDWORD SDW_SLOP[];
                     WORD W_Bin;

                     for (W_Bin=0 ; W_Bin < NUMBER_OF_BINS; W_Bin++)
                     {
                        DWORD DW_TempAddr = (DWORD)ACC3_SLOPES_0_MEM_START + W_Bin * 4;

                        /*
                        if(RenormSlope[W_Bin] != INTHIST_AVG_VAL)
                        	RenormSlope[W_Bin] = RenormSlope[W_Bin]<<4;
                        else
                        	RenormSlope[W_Bin] = RenormSlope[W_Bin]<<2;
                        */
 #if 1 // 1
                        if (GetParam(1) == 1) // Write
                           *(LINEAR_TO_FPTR_DW(DW_TempAddr)) = W_Bin; // SDW_SLOP_FIX[W_Bin];
                        else
                        {
                           DWORD DW_TempValue = *(LINEAR_TO_FPTR_DW(DW_TempAddr));

                           if (DW_TempValue != W_Bin)
                           {
                              gm_Print("Addr = 0x%lX", DW_TempAddr);
                              gm_Print("Value = 0x%lX", DW_TempValue);
                              gm_Print("Index = %d", W_Bin);
                           }
                        }
 #else
                        *(LINEAR_TO_FPTR_DW(DW_TempAddr)) = RenormSlope[W_Bin];
 #endif
                     }
                  }
                  break;

               case 8:
                  {
#define ACC3_SLOPES_0_MEM_START 0x32C00L
                     //extern SDWORD SDW_SLOP[];
                     WORD W_Bin;

                     for (W_Bin=0 ; W_Bin < NUMBER_OF_BINS; W_Bin++)
                     {
                        DWORD DW_TempAddr = (DWORD)ACC3_SLOPES_0_MEM_START + W_Bin * 4;

                        /*
                        if(RenormSlope[W_Bin] != INTHIST_AVG_VAL)
                        	RenormSlope[W_Bin] = RenormSlope[W_Bin]<<4;
                        else
                        	RenormSlope[W_Bin] = RenormSlope[W_Bin]<<2;
                        */
 #if 1 // 1
                        if (GetParam(1) == 1) // Write
                           *(LINEAR_TO_FPTR_DW(DW_TempAddr)) = BIT0 << (W_Bin%16); // SDW_SLOP_FIX[W_Bin];
                        else
                        {
                           DWORD DW_TempValue = *(LINEAR_TO_FPTR_DW(DW_TempAddr));
                           DWORD DW_RefValue = ((DWORD)BIT0 << (W_Bin%16));

                           //gm_Printf(" I %d ", (W_Bin%16));
                           //gm_Printf(" R 0x%lX ", DW_TempValue);
                           //gm_Printf(" V 0x%lX ", DW_RefValue);

                           if (DW_TempValue != DW_RefValue)
                           {
                              gm_Print("Addr = 0x%lX", DW_TempAddr);
                              gm_Print("Value = 0x%lX", DW_TempValue);
                              gm_Print("Ref = 0x%lX", DW_RefValue);
                              gm_Print("Index = %d", W_Bin);
                           }
                        }
 #else
                        *(LINEAR_TO_FPTR_DW(DW_TempAddr)) = RenormSlope[W_Bin];
 #endif
                     }

                     if (GetParam(1) == 0)
                        gm_Printf("Check Done", 0);
                  }
                  break;

               case 9:
                  {
#define ACC3_SLOPES_0_MEM_START 0x32C00L
                     //extern SDWORD SDW_SLOP[];
                     WORD W_Bin;

                     for (W_Bin=0 ; W_Bin < NUMBER_OF_BINS; W_Bin++)
                     {
                        DWORD DW_TempAddr = (DWORD)ACC3_SLOPES_0_MEM_START + W_Bin * 4;

                        /*
                        if(RenormSlope[W_Bin] != INTHIST_AVG_VAL)
                           RenormSlope[W_Bin] = RenormSlope[W_Bin]<<4;
                        else
                           RenormSlope[W_Bin] = RenormSlope[W_Bin]<<2;
                        */
 #if 1 // 1
                        if (GetParam(1) == 1) // Write
                           *(LINEAR_TO_FPTR_DW(DW_TempAddr)) = ~(BIT0 << (W_Bin%16)); // SDW_SLOP_FIX[W_Bin];
                        else
                        {
                           DWORD DW_TempValue = *(LINEAR_TO_FPTR_DW(DW_TempAddr));
                           DWORD DW_RefValue = (DWORD)(~((WORD)BIT0 << (W_Bin%16)));

                           //gm_Printf(" I %d ", (W_Bin%16));
                           //gm_Printf(" R 0x%lX ", DW_TempValue);
                           //gm_Printf(" V 0x%lX ", DW_RefValue);

                           if (DW_TempValue != DW_RefValue)
                           {
                              gm_Print("Addr = 0x%lX", DW_TempAddr);
                              gm_Print("Value = 0x%lX", DW_TempValue);
                              gm_Print("Ref = 0x%lX", DW_RefValue);
                              gm_Print("Index = %d", W_Bin);
                           }
                        }
 #else
                        *(LINEAR_TO_FPTR_DW(DW_TempAddr)) = RenormSlope[W_Bin];
 #endif
                     }

                     if (GetParam(1) == 0)
                        gm_Printf("Check Done", 0);

                  }
                  break;


            }
            break;
         }
#endif // (FEATURE_ACC3 == ENABLE)

      case Func82:
         {
#ifdef USE_NTP3K
            switch ((BYTE)GetParam(0))
            {
               case 0:
                  gm_Printf("Write table %d to NTP3000",(WORD)GetParam(1));
                  gm_Printf(NTP3K_WriteBlock((tSoundCtrl)GetParam(1)) ? "OK" : "Failed" );// 0 - sound off; 1 - sound on; 2 - Init sequence;
                  break;
               case 1:
                  {
                     BYTE B_Bf[2];
                     B_Bf[0] = (BYTE)GetParam(1);
                     B_Bf[1] = (BYTE)GetParam(2);
                     gm_Printf("Write 0x%X(%d) to register 0x%x",(WORD)B_Bf[1],(WORD)B_Bf[1],(WORD)B_Bf[0]);
                     gm_Printf(NTP3K_RegWrite((BYTE far *)B_Bf, sizeof(B_Bf)) ? "OK" : "Failed" );
                     break;
                  }

               case 2:
                  {
                     DWORD DW_Bf = 0;
                     if (NTP3K_Read((t_NTP3K_RegId)GetParam(1), (BYTE *)&DW_Bf))
                        gm_Printf("NTP3000 register 0x%X(%d) = 0x%lX", (WORD)GetParam(1), (WORD)GetParam(1), DW_Bf);
                     else
                        gm_Printf("Failed reading of NTP3000 register 0x%X(%d)", (WORD)GetParam(1), (WORD)GetParam(1), DW_Bf);
                     break;
                  }
               case 3:
                  NTP3K_Init();
                  NTP3K_Turn(SOUND_ON);
                  break;
            }
            gm_Printf("");
#else
            gm_Printf("No NTP3000 chip in the system");
#endif
            break;
         }
      case Func83:
         break;

      case Func90:
         TestEngineeringAdjusters((BYTE)GetParam(0), (WORD)GetParam(1));
         break;
#endif // FULL_APPSTEST_CMD

      case Func91:
         UserPrefSharpness = (BYTE)GetParam(0);
         AdjustSharpness();
         break;

      case Func92:
         UserPrefSharpnessRGB = (BYTE)GetParam(0);
#ifdef ADJUSTER_SHARPNESSRGB_DECLARED
         AdjustSharpnessRGB();
#endif
         break;

#ifdef FULL_APPSTEST_CMD
      case Func93:
         // TODO: update Decoder library
         // gm_SetDecoderAGCMethod( (BYTE)GetParam(0) );
         break;

      case Func94:
#if (FEATURE_TWO_STAGE_PROCESSING_MODE == ENABLE)
         if (GetParam(0) == 0)
         {
            ExitTwoStageProcessing();
            ModeSetupBlender(CH_A, DISPLAY_DEFER_TMO_NORMAL);
            ModeSetupBlender(CH_B, DISPLAY_DEFER_TMO_NORMAL);
            gm_Printf("!!!Exited two-stage!!!",0);
         }
         else
         {
            if (IsTwoStageAllowed())
            {
               EnterTwoStageProcessing();
               ModeSetupBlender(CH_A, DISPLAY_DEFER_TMO_RF);
            }
            else
            {
               gm_Printf("!!!two-stage not executed since signal is progressive or RGB!!!",0);
            }
         }
#endif
         break;
#endif // FULL_APPSTEST_CMD

      case Func95: //test dram code execution functionality
#if (FEATURE_DDR_EXECUTION == ENABLE) && (defined(DRAM_CODE_EXECUTION) || (DATA_DRAM_SIZE > 0))
         switch (GetParam(0))
         {
            case 1: //switch execution to DRAM
               #if (DRAM_CODE_EXECUTION_MODE == 1)
               if(SystemFlags.sys.DramCodeExecution != TRUE)
               {
                  gm_MemoryCodeDownload();
                  gm_MemoryCodeAdjust(DATA_DRAM_SIZE+CODE_SIZE);
                  SystemFlags.sys.DramCodeExecution = TRUE;
               }
               #endif
               gm_MemoryCodeStart();
               break;
            case 2: //switch execution from DRAM
               gm_MemoryCodeStop();
               break;
            case 3: //switch execution from DRAM
               gm_MemoryCodeDownload();
               break;
            default: //verification of DRAM loaded code.
               gm_Printf("DRAM code verify: %s", (gm_MemoryCodeVerify() ? "OK" : "Fail"));
               break;
         }
#endif //DRAM_CODE_EXECUTION
         break;

#ifdef FULL_APPSTEST_CMD
      case Func96:

         {
            gmt_WindowStruct InputWinDim, OutputWinDim;
            gmt_WindowStruct PrevInputDim;

            PrevInputDim.W_Hstart = stInput[gmvb_MainChannel].Hstart;
            PrevInputDim.W_Width = stInput[gmvb_MainChannel].Hactive;
            PrevInputDim.W_Vstart = stInput[gmvb_MainChannel].Vstart;
            PrevInputDim.W_Height = stInput[gmvb_MainChannel].Vactive;

            MultiDisplaySetup((WORD)GetParam(1), (WORD)GetParam(2), (WORD)GetParam(3), (WORD)GetParam(4),
                              (WORD)GetParam(5), (WORD)GetParam(6), (WORD)GetParam(7),
                              (WORD)GetParam(8), (WORD)GetParam(9), (WORD)GetParam(10));

            stInput[gmvb_MainChannel].Vactive = gmvw_InputVActiveOrg[gmvb_MainChannel];
            stInput[gmvb_MainChannel].Hactive = gmvw_InputHActiveOrg[gmvb_MainChannel];
            stInput[gmvb_MainChannel].Hstart = gmvw_InputHStartOrg[gmvb_MainChannel];
            stInput[gmvb_MainChannel].Vstart = gmvw_InputVStartOrg[gmvb_MainChannel];
            InputCaptureCrop(gmvb_MainChannel);
            InputWinDim.W_Height = stInput[gmvb_MainChannel].Vactive;
            InputWinDim.W_Width = stInput[gmvb_MainChannel].Hactive;
            InputWinDim.W_Hstart = stInput[gmvb_MainChannel].Hstart;
            InputWinDim.W_Vstart = stInput[gmvb_MainChannel].Vstart;

            OutputWinDim.W_Height = gmvw_OutputVActive[gmvb_MainChannel];
            OutputWinDim.W_Width= gmvw_OutputHActive[gmvb_MainChannel];
            OutputWinDim.W_Hstart= gmvw_OutputHStart[gmvb_MainChannel];
            OutputWinDim.W_Vstart= gmvw_OutputVStart[gmvb_MainChannel];

            GetMultiDisplayWinAttrib((WORD)GetParam(11), &InputWinDim, &OutputWinDim);

            stInput[gmvb_MainChannel].Vactive = InputWinDim.W_Height;
            stInput[gmvb_MainChannel].Hactive = InputWinDim.W_Width;
            stInput[gmvb_MainChannel].Hstart = InputWinDim.W_Hstart;
            stInput[gmvb_MainChannel].Vstart = InputWinDim.W_Vstart;

            /****** ZOOM STARTS ******/
            //This means no image should be shown in this panel
            if ((stInput[gmvb_MainChannel].Hactive == 0)
                  ||(stInput[gmvb_MainChannel].Vactive == 0))
            {
               SetDisplayOff(gmvb_MainChannel, BACKGROUND_ONLY);
               return;
            }

            RelocateInputCaptureWindow(gmvb_MainChannel, &PrevInputDim,
                                       &InputWinDim);
            /****** ZOOM ENDS ******/
            /****** DYNAMIC SCALING STARTS ******/
            SetDynamicScalerSizeBegin(gmvb_MainChannel);
            gmvw_OutputVActive[gmvb_MainChannel] = OutputWinDim.W_Height;
            gmvw_OutputHActive[gmvb_MainChannel] = OutputWinDim.W_Width;
            gmvw_OutputHStart[gmvb_MainChannel] = OutputWinDim.W_Hstart;
            gmvw_OutputVStart[gmvb_MainChannel] = OutputWinDim.W_Vstart;

            // Setup Display
            SetDynamicScalerStep(gmvb_MainChannel, 1);
            SetDynamicScalerStep(gmvb_PipChannel, 0);   // turn off pip scaling

            SetDynamicScalerSizeEnd(gmvb_MainChannel);

            DynamicScalerStart(gmvb_MainChannel);
            DynamicScalerProcess(FALSE);
            DynamicScalerStop();

            if (gmvb_MainChannel == CH_A)
            {
               //
               // to disable TNR (TNR_PROGRESSIVE) bits
               // in case of Vertical Shrink
               //
               if (stInput[CH_A].Vactive > gmvw_OutputVActive[CH_A])
               {
                  gm_VideoProcessingDisable(VIDEO_TNR);
               }
            }

            // Request Blenders ON and User Prefs
            ModeSetupBlender(gmvb_MainChannel, DISPLAY_DEFER_TMO_NORMAL);
         }
         break;

         //Test the accurance of 100ms delay by using different timer base
         //Please use scope to measure the GPIO pulse width (timer interval)
      case Func100 :
         {
            //#define pcbReg(addr) (*(volatile WORD pcbreg* const)(addr))
            BYTE B_Cnt, B_ErrMsg=0;
            BYTE DelaySel = (BYTE)GetParam(0);
            BYTE DelayNum = (BYTE)GetParam(1);

            //gm_TimerInitStd(gmc_OCLK_FREQ);
            //UserTimerInit();

            if (DelayNum==0)	DelayNum=1;

            for (B_Cnt=0; B_Cnt<10; B_Cnt++)
            {
               if (gm_ReadRegWord(GPOUTPUT4) & BIT15)	//GPIO79, LED Green
                  gm_ClearRegBitsWord(GPOUTPUT4, BIT15);
               else
                  gm_SetRegBitsWord(GPOUTPUT4, BIT15);

               switch (DelaySel)
               {
                  case 0:
                     gm_Delay1us(DelayNum);
                     break;
                  case 1:
                     gm_Delay1ms(DelayNum);
                     break;
                  case 10:
                     gm_Delay10ms(DelayNum);
                     break;
                  case 100:
                     gm_Delay100ms(DelayNum);
                     break;
                  default:
                     B_ErrMsg = 1;
                     break;
               }

               if (B_ErrMsg == 1)
               {
                  gm_Print("Wrong Timer Select! Please select 0(1us), 1(ms), 10(ms), 100(ms)",0);
                  break;
               }
            }
         }
         break;

         //Test system timer, gsel timer, user timer and their accurance
         //Please use scope to measure the GPIO pulse width (timer interval)
      case Func101 :
         {
            //#define pcbReg(addr) (*(volatile WORD pcbreg* const)(addr))
            BYTE B_Cnt, B_Timer, B_ErrMsg=0;
            BYTE TimerType = (BYTE)GetParam(0);
            WORD TimerSel = (WORD)GetParam(1);
            BYTE TimeNum = (BYTE)GetParam(2);

            //gm_TimerInitStd(gmc_OCLK_FREQ);
            //UserTimerInit();

            if (TimeNum==0)	TimeNum=1;

            for (B_Cnt=0; B_Cnt<10; B_Cnt++)
            {
               if (gm_ReadRegWord(GPOUTPUT4) & BIT15)	//GPIO79, LED Green
                  gm_ClearRegBitsWord(GPOUTPUT4, BIT15);
               else
                  gm_SetRegBitsWord(GPOUTPUT4, BIT15);

               switch (TimerType)
               {
                  case 0:	//System Timer
                     if (TimerSel == 1)
                        B_Timer = START_1ms;
                     else if (TimerSel == 10)
                        B_Timer = START_10ms;
                     else if (TimerSel == 100)
                        B_Timer = START_100ms;
                     else if (TimerSel == 1000)
                        B_ErrMsg = 3;
                     else
                        B_ErrMsg = 1;
                     break;

                  case 1:	//Gsel Timer
                     if (TimerSel == 1)
                        B_Timer = GSEL_START_1ms;
                     else if (TimerSel == 10)
                        B_Timer = GSEL_START_10ms;
                     else if (TimerSel == 100)
                        B_Timer = GSEL_START_100ms;
                     else if (TimerSel == 1000)
                        B_Timer = GSEL_START_1s;
                     else
                        B_ErrMsg = 1;
                     break;

                  case 2: //User Timer
                     if (TimerSel == 1)
                        B_Timer = USER_START_1ms;
                     else if (TimerSel == 10)
                        B_Timer = USER_START_10ms;
                     else if (TimerSel == 100)
                        B_Timer = USER_START_100ms;
                     else if (TimerSel == 1000)
                        B_Timer = USER_START_1s;
                     else
                        B_ErrMsg = 1;
                     break;

                  default:
                     B_ErrMsg = 2;
                     break;

               }

               if (B_ErrMsg == 1)
               {
                  gm_Print("Wrong Timer Count! Please select 1, 10, 100(ms)",0);
                  break;
               }
               else if (B_ErrMsg == 2)
               {
                  gm_Print("Wrong Timer Select! Please select 0(SystemTimer), 1(GselTimer), 2(UserTimer)",0);
                  break;
               }
               else if (B_ErrMsg == 3)
               {
                  gm_Print("No 1 sec timer count in system timer!",0);
                  break;
               }

               gm_TimerStart(B_Timer, TimeNum); // setup a timeout
               while (1) //wait for it to complete
               {
                  gmt_TimerStatus TimerStatus = gm_TimerCheck(B_Timer);
                  if (TimerStatus == TIMER_TMO)
                     break;
                  else if (TimerStatus == 3)
                  {
                     gm_Print("Timer Status = %d", TimerStatus);
                     break;
                  }
               }
               gm_TimerStop(B_Timer);
            }
         }
         break;

#if (FEATURE_PIXEL_CRUNCH == ENABLE)
      case Func103:
      {
         WORD tprevCoarseXStart = 0, tprevCoarseYStart = 0, tprevCoarseWidth = 0, tprevCoarseHeight = 0;
         WORD prevXStart=0, prevYStart=0, prevWidth=0, prevHeight=0;
         WORD XStart, YStart, Width, Height;
         WORD CoarseXStart, CoarseYStart,CoarseWidth,CoarseHeight;
         gmt_VideoInfo VwdDetectInfoTest, VwdCoarseInfoTest;
         gmt_PCInputConfig VwdConfigTest;
         gmt_MultiVideoInfo CoarseMVWInfoTest;
         
         DWORD TimeStrt,TimeEd,TimeEvent,TimeEventCo;
         BYTE VWDStateCurrTest, tprevVwdState;
         BYTE MVWONTest, tsloop, tSelectRectext;
         BYTE tExtSelectWR = 0, Rectcount = 0, prevVideoType = 6;

         WORD CXStart, CYStart, CXEnd, CYEnd;
         BYTE Timing_ON, Enter_Cnd;
         BOOL paramzero = 0;
         DWORD count = 0;
         WORD LoopEnd;
         BYTE alreadyclear = 1;
         
         WORD i, Savei;

         UserPrefVwdFlag = ON;
         
         VwdConfigTest.DEBUGONOFF = (BYTE)GetParam(1);
         VwdConfigTest.BLACKONOFF = (BYTE)GetParam(2);
         VwdConfigTest.BORDERLESSONOFF = (BYTE)GetParam(3);
         Timing_ON = (BYTE)GetParam(4);
         CXStart   = (WORD)GetParam(5);
         CXEnd     = (WORD)GetParam(6);
         CYStart   = (WORD)GetParam(7);
         CYEnd     = (WORD)GetParam(8);
         VwdConfigTest.COA_EDGEHWTHR = (BYTE)GetParam(9);
         VwdConfigTest.COA_EDGEDIVFACT = (BYTE)GetParam(10);
         VwdConfigTest.COA_MOTIONTHR = (BYTE)GetParam(11);
         VwdConfigTest.COA_MOTIONGATE = (BYTE)GetParam(12);
         VwdConfigTest.COA_REALTHR = (BYTE)GetParam(13);
         VwdConfigTest.COA_REALGATE = (BYTE)GetParam(14);
         VwdConfigTest.COA_BLACKTHR = (BYTE)GetParam(15);
         VwdConfigTest.COA_FINALGATEMOTION = (BYTE)GetParam(16);
         VwdConfigTest.COA_FINALGATE = (BYTE)GetParam(17);
         VwdConfigTest.COA_FINALMAPSEL = (BYTE)GetParam(18);
         VwdConfigTest.COA_RECTWIDTH = (BYTE)GetParam(19);
         VwdConfigTest.COA_RECTHEIGHT = (BYTE)GetParam(20);
         VwdConfigTest.FIN_MOTIONTHR = (BYTE)GetParam(21);
         VwdConfigTest.FIN_BLACKTHR = (BYTE)GetParam(22);

         tSelectRectext = 0;

         gm_Print("VWD - Start",0);
         PixCrAnalyzeImage(&VwdDetectInfoTest, &VwdCoarseInfoTest,
                     START_ANALYZE, VwdConfigTest,
                     &VWDStateCurrTest, &CoarseMVWInfoTest, &MVWONTest, tSelectRectext, tExtSelectWR);

         tExtSelectWR = 0;
         TimeEvent = 0;
         TimeStrt = gm_GetSystemTime();

         if (GetParam(0) == 0)
         {
            paramzero = 1; 
            LoopEnd = 1;
         }
         else
            LoopEnd = (WORD)GetParam(0);

         gm_Print("LoopEnd=%d\n",LoopEnd);
         for (i=0;i<(WORD)LoopEnd;i++)
         {
            if (paramzero == 1)
		         i--;
            tprevVwdState = VWDStateCurrTest;
            
            PixCrAnalyzeImage(&VwdDetectInfoTest, &VwdCoarseInfoTest,
                     POLL_ANALYZE, VwdConfigTest,
                     &VWDStateCurrTest, &CoarseMVWInfoTest, &MVWONTest, tSelectRectext, tExtSelectWR);

            Rectcount = 0;

            if ((tprevVwdState == COARSE_DETECTION) && (VWDStateCurrTest == COARSE_MVWSELECT) && (MVWONTest == 1))
            {  
               for (tsloop = 4;tsloop > 0;tsloop--) 
               {
                  if (((CoarseMVWInfoTest.Rect[tsloop-1].VideoType == PERFECT_RECTFOUND) || (CoarseMVWInfoTest.Rect[tsloop-1].VideoType == CUT_RECTFOUND)))	
                  {
                     Rectcount++;
                     if (Timing_ON == 0)
                     { 
                        gm_Print("Highlight = %d",tsloop-1);
                        CoarseXStart = (WORD)(((DWORD)CoarseMVWInfoTest.Rect[tsloop-1].XStart * (DWORD)gmvw_OutputHActive[gmvb_MainChannel] + (DWORD)(stInput[gmvb_MainChannel].Hactive/2))/(DWORD)stInput[gmvb_MainChannel].Hactive);
                        CoarseWidth = (WORD) (((DWORD)CoarseMVWInfoTest.Rect[tsloop-1].XEnd  * (DWORD)gmvw_OutputHActive[gmvb_MainChannel] + (DWORD)(stInput[gmvb_MainChannel].Hactive/2))/(DWORD)stInput[gmvb_MainChannel].Hactive) - CoarseXStart + 1;
                        CoarseYStart = (WORD)(((DWORD)CoarseMVWInfoTest.Rect[tsloop-1].YStart * (DWORD)gmvw_OutputVActive[gmvb_MainChannel] + (DWORD)(stInput[gmvb_MainChannel].Vactive/2))/(DWORD)stInput[gmvb_MainChannel].Vactive);
                        CoarseHeight = (WORD)(((DWORD)CoarseMVWInfoTest.Rect[tsloop-1].YEnd * (DWORD)gmvw_OutputVActive[gmvb_MainChannel] + (DWORD)(stInput[gmvb_MainChannel].Vactive/2))/(DWORD)stInput[gmvb_MainChannel].Vactive) - CoarseYStart + 1;

                        TestApplyHighLighWindow(CoarseXStart,CoarseYStart,CoarseWidth,CoarseHeight, 0);

                        gm_Delay10ms(300);
                     }
                     tSelectRectext = tsloop - 1;
                  }
               }
            }

            if (Timing_ON == 0)
            {
               if (Rectcount != 0) //Clear the Coarse window
               {
                  VWD_ClearHighlighWindow();
               }
            }

            if (Rectcount != 0)
               tExtSelectWR = 1;
            else
               tExtSelectWR = 0;

            CoarseXStart = (WORD)(((DWORD)VwdCoarseInfoTest.XStart * (DWORD)gmvw_OutputHActive[gmvb_MainChannel] + (DWORD)(stInput[gmvb_MainChannel].Hactive/2))/(DWORD)stInput[gmvb_MainChannel].Hactive);
            if (VwdCoarseInfoTest.XEnd > VwdCoarseInfoTest.XStart)
               CoarseWidth = (WORD)(((DWORD)VwdCoarseInfoTest.XEnd * (DWORD)gmvw_OutputHActive[gmvb_MainChannel] + (DWORD)(stInput[gmvb_MainChannel].Hactive/2))/(DWORD)stInput[gmvb_MainChannel].Hactive) - CoarseXStart + 1;
            CoarseYStart = (WORD)(((DWORD)VwdCoarseInfoTest.YStart * (DWORD)gmvw_OutputVActive[gmvb_MainChannel] + (DWORD)(stInput[gmvb_MainChannel].Vactive/2))/(DWORD)stInput[gmvb_MainChannel].Vactive);
            if (VwdCoarseInfoTest.YEnd > VwdCoarseInfoTest.YStart)
               CoarseHeight = (WORD)(((DWORD)VwdCoarseInfoTest.YEnd * (DWORD)gmvw_OutputVActive[gmvb_MainChannel] + (DWORD)(stInput[gmvb_MainChannel].Vactive/2))/(DWORD)stInput[gmvb_MainChannel].Vactive) - CoarseYStart + 1;

            if (((tprevCoarseXStart != CoarseXStart) || (tprevCoarseWidth != CoarseWidth) || (tprevCoarseYStart !=CoarseYStart) || (tprevCoarseHeight != CoarseHeight)) || ((VwdCoarseInfoTest.VideoType==FULL_SCREEN) && (alreadyclear==1)))
            {
               tprevCoarseXStart = CoarseXStart;
               tprevCoarseWidth  = CoarseWidth;
               tprevCoarseYStart = CoarseYStart;
               tprevCoarseHeight = CoarseHeight;
               
               if (Timing_ON == 1) 
               {
                  TimeEventCo = gm_GetSystemTime();
                  gm_Print("Coarse APPTime = %d",(TimeEventCo - TimeStrt));
               }
               
               if (VwdCoarseInfoTest.VideoType!=FULL_SCREEN)
               {
                  gm_Print("CoarseXStart = %d",CoarseXStart);
                  gm_Print("CoarseWidth = %d",CoarseWidth);
                  gm_Print("CoarseYStart = %d",CoarseYStart);
                  gm_Print("CoarseHeight = %d",CoarseHeight);
               }
               
               //Uncomment for coarse window
               /*if (Timing_ON == 0)
               {
                  if(VwdCoarseInfoTest.VideoType!= FULL_TEXT){
                     if (VwdCoarseInfoTest.VideoType==FULL_SCREEN)
                     {
                        TestApplyHighLighWindow(CoarseXStart,CoarseYStart,CoarseWidth,CoarseHeight, 0);//Final code make it 1
                        gm_Print("App FullScreen\n");
                     }
                     else
                        TestApplyHighLighWindow(CoarseXStart,CoarseYStart,CoarseWidth,CoarseHeight, 0);
                     alreadyclear = 0;
                  }
               }*/
            }
            
            //uncomment for coarse window
            /*if (((VwdCoarseInfoTest.VideoType == FULL_TEXT) ||(VwdCoarseInfoTest.VideoType == LOST_TYPE)||(tprevVwdState == COARSE_DETECTION) && (VWDStateCurrTest == COARSE_MVWSELECT)) && (alreadyclear == 0))
            {
               alreadyclear = 1;
               gm_Print("Clear WIndow",0);
               VWD_ClearHighlighWindow();
            }*/

            XStart = (WORD)(((DWORD)VwdDetectInfoTest.XStart * (DWORD)gmvw_OutputHActive[gmvb_MainChannel] + (DWORD)(stInput[gmvb_MainChannel].Hactive/2))/(DWORD)stInput[gmvb_MainChannel].Hactive);
            if (VwdDetectInfoTest.XEnd > VwdDetectInfoTest.XStart)
               Width = (WORD)(((DWORD)VwdDetectInfoTest.XEnd * (DWORD)gmvw_OutputHActive[gmvb_MainChannel] + (DWORD)(stInput[gmvb_MainChannel].Hactive/2))/(DWORD)stInput[gmvb_MainChannel].Hactive) - XStart; // + 1;
            YStart = (WORD)(((DWORD)VwdDetectInfoTest.YStart * (DWORD)gmvw_OutputVActive[gmvb_MainChannel] + (DWORD)(stInput[gmvb_MainChannel].Vactive/2))/(DWORD)stInput[gmvb_MainChannel].Vactive);
            if (VwdDetectInfoTest.YEnd > VwdDetectInfoTest.YStart)
               Height = (WORD)(((DWORD)VwdDetectInfoTest.YEnd * (DWORD)gmvw_OutputVActive[gmvb_MainChannel] + (DWORD)(stInput[gmvb_MainChannel].Vactive/2))/(DWORD)stInput[gmvb_MainChannel].Vactive) - YStart; // + 1;

            Enter_Cnd = 0;
            if (Timing_ON == 1) 
            {
               if ((XStart >= CXStart) && (XStart < (CXStart + 20)) 
                     && (YStart >= CYStart) && (YStart < (CYStart + 20))
                     && ((YStart+Height) >= CYEnd) && ((YStart+Height) < (CYEnd + 20))  
                     && ((XStart+Width) >= CXEnd) && ((XStart+Width) < (CXEnd + 20)) 
                     && (TimeEvent == 0)) //Uncomment for Timing
                  Enter_Cnd = 1;
            }
            else
            {
               if ((prevXStart != XStart) || (prevWidth != Width) || (prevYStart != YStart) || (prevHeight != Height) ||(prevVideoType != VwdDetectInfoTest.VideoType))
                  Enter_Cnd = 1;
            }

            if (Enter_Cnd == 1)
            {
               prevXStart = XStart;
               prevWidth = Width;
               prevYStart = YStart;
               prevHeight = Height;
               prevVideoType = VwdDetectInfoTest.VideoType;
               Savei=i;
               TimeEvent = gm_GetSystemTime();

               if (Timing_ON == 1) 
               {
                  gm_Print("SelectRect=%d\n",tSelectRectext);
                  gm_Print("Total APPTime = %d\n",(TimeEvent -TimeStrt));
                  if (CXStart > XStart)
                     gm_Print("XStart Error = %d\n",abs(CXStart -XStart));
                  else
                     gm_Print("XStart Error = -%d\n",abs(XStart - CXStart));

                  if (CXEnd < (XStart+Width)) 
                     gm_Print("XEnd Error = %d\n",abs((XStart+Width) -CXEnd));
                  else
                     gm_Print("XEnd Error = -%d\n",abs(CXEnd - (XStart+Width)));

                  if (CYStart > YStart)
                     gm_Print("YStart Error = %d\n",abs(CYStart -YStart));
                  else
                     gm_Print("YStart Error = -%d\n",abs(YStart -CYStart));

                  if (CYEnd > (YStart+Height)) 
                     gm_Print("YEnd Error = -%d\n",abs(CYEnd -(YStart+Height)));
                  else
                     gm_Print("YEnd Error = %d\n",abs((YStart+Height)-CYEnd));
               }
               else
               {
                  gm_Print("XStart = %d",XStart);
                  gm_Print("XEnd = %d",(XStart+Width));
                  gm_Print("YStart = %d",YStart);
                  gm_Print("YEnd = %d",(YStart+Height));
                  gm_Print("Type = %d",VwdDetectInfoTest.VideoType);
                  //gm_Print("IP YStart = %d\n",videoInfo.YStart);
                  //gm_Print("IP YEnd = %d\n",videoInfo.YEnd);
               }

               if ((VwdDetectInfoTest.VideoType != FULL_TEXT)) //for real 
               {  
                //if(VwdDetectInfoTest.VideoType!=FULL_SCREEN)//for debug
	       //{
                  gm_Print("Not Full Text ======== %d",VwdDetectInfoTest.VideoType);
                  TestApplyHighLighWindow(XStart,YStart,Width,Height, 1);
               //}
               }
               else
               {
                  //gm_Delay1ms(2000);
                  gm_Print("Clear WIndow",0);
                  VWD_ClearHighlighWindow();
               }
               
               if (VwdDetectInfoTest.VideoType == LETTER_BOX)
               {  
                  if(Width > 300)
                     Width = 250;
               }
               else if (VwdDetectInfoTest.VideoType==PILLAR_BOX)
               {
                  if (Height > 300)
                     Height = 250;
               }
               else
               {
                  if(Width > 500)
                     Width = 250;
                  if (Height > 500)
                     Height = 250;
               }
               gm_Print("Draws Rectangle--------------------",0);
            }

            if (Timing_ON == 1)
            {
               if (TimeEvent != 0)
                  i = Savei;
               
               TimeEd = gm_GetSystemTime();
               if ((TimeEd - TimeStrt) >= 10000)
               {
                  if (TimeEvent == 0)
                  {
                     gm_Print("MaxRectCount = 0",0);
                     gm_Print("SelectRect=%d\n",tSelectRectext);
                     gm_Print("Total APPTime = 10000",0);
                     if (CXStart > XStart)
                        gm_Print("XStart Error = %d\n",abs(CXStart -XStart));
                     else
                        gm_Print("XStart Error = -%d\n",abs(XStart - CXStart));

                     if (CXEnd < (XStart+Width)) 
                        gm_Print("XEnd Error = %d\n",abs((XStart+Width) -CXEnd));
                     else
                        gm_Print("XEnd Error = -%d\n",abs(CXEnd - (XStart+Width)));

                     if (CYStart > YStart)
                        gm_Print("YStart Error = %d\n",abs(CYStart -YStart));
                     else
                        gm_Print("YStart Error = -%d\n",abs(YStart -CYStart));

                     if (CYEnd > (YStart+Height)) 
                        gm_Print("YEnd Error = -%d\n",abs(CYEnd -(YStart+Height)));
                     else
                        gm_Print("YEnd Error = %d\n",abs((YStart+Height)-CYEnd));
                     //gm_Print("XStart Error = %d",abs(CXStart -XStart));
                     //gm_Print("XEnd Error = %d",abs(CXEnd -(XStart+Width)));
                     //gm_Print("YStart Error = %d",abs(CYStart -YStart));
                     //gm_Print("YEnd Error = %d",abs(CYEnd -(YStart+Height)));
                  }
                  
                  PixCrAnalyzeImage(&VwdDetectInfoTest, &VwdCoarseInfoTest,
                     STOP_ANALYZE, VwdConfigTest,
                     &VWDStateCurrTest, &CoarseMVWInfoTest, &MVWONTest, tSelectRectext, tExtSelectWR);
                  
                  PixCrAnalyzeImage(&VwdDetectInfoTest, &VwdCoarseInfoTest,
                     START_ANALYZE, VwdConfigTest,
                     &VWDStateCurrTest, &CoarseMVWInfoTest, &MVWONTest, tSelectRectext, tExtSelectWR);
                  
                  prevXStart = XStart =0;
                  prevYStart = YStart = 0;
                  prevWidth = Width = gmvw_OutputHActive[gmvb_MainChannel];		//to avoid first time rectangle draw for instrumentation
                  prevHeight = Height = gmvw_OutputVActive[gmvb_MainChannel];		//to avoid first time rectangle draw for instrumentation
                  prevVideoType = 6;
                  tprevCoarseXStart=0; 
                  tprevCoarseYStart=0; 
                  tprevCoarseWidth=0; 
                  tprevCoarseHeight=0;

                  count = count + ((TimeEd - TimeStrt)/1000);
                  gm_Print("Total Running Time = %d sec\n",count);
                  TimeStrt = gm_GetSystemTime();
                  TimeEvent = 0;
               }
            }   
         }

         
                  
         gm_Print("VWD - Stop",0);
         PixCrAnalyzeImage(&VwdDetectInfoTest, &VwdCoarseInfoTest,
                     STOP_ANALYZE, VwdConfigTest,
                     &VWDStateCurrTest, &CoarseMVWInfoTest, &MVWONTest, tSelectRectext, tExtSelectWR);

         VWD_ClearHighlighWindow();

         UserPrefVwdFlag = OFF;
      } // Func103 - VWD Test
         break;
  #ifdef SINGLE_FIELD_DEINT_FIXED
      case Func104: //Interlace problem detect
      {
         WORD tprevCoarseXStart = 0, tprevCoarseYStart = 0, tprevCoarseWidth = 0, tprevCoarseHeight = 0;
         WORD CoarseXStart,CoarseYStart,CoarseWidth,CoarseHeight;
         gmt_VideoInfo VwdDetectInfoTest, VwdCoarseInfoTest;
         gmt_PCInputConfig VwdConfigTest;
         gmt_MultiVideoInfo CoarseMVWInfoTest;
         BYTE VWDStateCurrTest;
         //BYTE tprevVwdState;
         BYTE MVWONTest, tsloop, tSelectRectext;
         BYTE tExtSelectWR = 0;
         //BYTE Rectcount = 0, prevVideoType = 6;
         BOOL paramzero = 0;
         WORD LoopEnd;
         WORD i;
         DWORD TimeStrt,TimeEd,TimeStrt_act,diffTime,diffTime_step,TimeStrt_step;
         BYTE FlagToggle,RampUpDown;
         WORD OnTimeWin;
         BYTE PosChange,OnTimeCount;

         if (GetParam(0) == 0)
         {   
            paramzero = 1; 
            LoopEnd = 1;
         }
         else if (GetParam(0) >1)
            LoopEnd = (WORD)GetParam(0);
         else
            LoopEnd = 0;

         if (GetParam(1) == 1)
            gm_SetScalerSharpness(CH_A,25);//0.45
         else if (GetParam(1)==2)
            gm_SetScalerSharpness(CH_A,29);//0.45

         tSelectRectext = 0;
         //Start analysize for intialization of variables
         PixCrAnalyzeImage(&VwdDetectInfoTest, &VwdCoarseInfoTest,
         START_ANALYZE, VwdConfigTest,
         &VWDStateCurrTest, &CoarseMVWInfoTest, &MVWONTest, tSelectRectext, tExtSelectWR);
         gm_Print("LoopEnd=%d\n",LoopEnd);
         FlagToggle=0;
         RampUpDown=29;
         OnTimeWin = INTL_ONTIMEDEFAULT;
         OnTimeCount =0;
         
         for (i=0;i < (WORD)LoopEnd;i++)
         {
            if(paramzero ==1)
               i--;
            //tprevVwdState = VWDStateCurrTest;
            //For Intl_Analysis -only for Interlace input
            PixCrAnalyzeImage(&VwdDetectInfoTest, &VwdCoarseInfoTest,
                  INTL_ANALYZE, VwdConfigTest,
                  &VWDStateCurrTest, &CoarseMVWInfoTest, &MVWONTest, tSelectRectext, tExtSelectWR);

            CoarseXStart = (WORD)(((DWORD)VwdDetectInfoTest.XStart * (DWORD)gmvw_OutputHActive[gmvb_MainChannel] + (DWORD)(stInput[gmvb_MainChannel].Hactive/2))/(DWORD)stInput[gmvb_MainChannel].Hactive);
            CoarseWidth = (WORD) (((DWORD)VwdDetectInfoTest.XEnd  * (DWORD)gmvw_OutputHActive[gmvb_MainChannel] + (DWORD)(stInput[gmvb_MainChannel].Hactive/2))/(DWORD)stInput[gmvb_MainChannel].Hactive) - CoarseXStart + 1;
            CoarseYStart = (WORD)(((DWORD)VwdDetectInfoTest.YStart * (DWORD)gmvw_OutputVActive[gmvb_MainChannel] + (DWORD)(stInput[gmvb_MainChannel].Vactive/2))/(DWORD)stInput[gmvb_MainChannel].Vactive);
            CoarseHeight = (WORD)(((DWORD)VwdDetectInfoTest.YEnd * (DWORD)gmvw_OutputVActive[gmvb_MainChannel] + (DWORD)(stInput[gmvb_MainChannel].Vactive/2))/(DWORD)stInput[gmvb_MainChannel].Vactive) - CoarseYStart + 1;

            if (((tprevCoarseXStart != CoarseXStart) || (tprevCoarseWidth != CoarseWidth) || (tprevCoarseYStart !=CoarseYStart) || (tprevCoarseHeight != CoarseHeight)))
            {
               tprevCoarseXStart = CoarseXStart;
               tprevCoarseWidth  = CoarseWidth;
               tprevCoarseYStart = CoarseYStart;
               tprevCoarseHeight = CoarseHeight;
               PosChange = 1;
               //VWD_ClearHighlighWindow();
               if ((VwdDetectInfoTest.VideoType==ARTIFACT) || (RampUpDown!=29))
                  TestApplyHighLighWindow(CoarseXStart,CoarseYStart,CoarseWidth,CoarseHeight, 0);//To Highlight the Coarse detected region Internal Debug
               else 
                  TestApplyHighLighWindow(CoarseXStart,CoarseYStart,CoarseWidth,CoarseHeight, 2);//To Highlight the Coarse detected region Internal Debug
            }
            //gm_Delay10ms(16);
            //gm_Print("Problemfound_app=%d\n",VwdDetectInfoTest.VideoType);
            TimeEd = gm_GetSystemTime(); //Timer is used Vsync Interrupt can also be used instead
            if (TimeEd > TimeStrt_act)
               diffTime = TimeEd - TimeStrt_act;
            else
               diffTime = TimeStrt_act-TimeEd;

            if (TimeEd > TimeStrt_step)
               diffTime_step = TimeEd - TimeStrt_step;
            else
               diffTime_step = TimeStrt_step-TimeEd;
            //OnTimeWin is the time untill which artifact reduction filter is applied
            //OnTimeWin is adaptive based on amount of detection,More the detection the window increase
            //By default OnTimeWin is four sec
            //For every detection OnTimeWin inc by 1 sec
            //For every Forth Not detection OnTimeWin is dec by 1sec
            //The residue can buildup which is dec by 1sec above 10sec buildup and dec by.5sec otherwise 
            //On First Detection sharpness filter is made soft in four steps
            //Once outside the OnTimeWin sharpness filter is made sharp in four steps
            if ((VwdDetectInfoTest.VideoType==ARTIFACT) || (diffTime < OnTimeWin))
            {
               if ((PosChange == 1) && (VwdDetectInfoTest.VideoType==ARTIFACT))
               {
                  if (OnTimeCount < 16)
                  {
                     if(OnTimeWin < 20000)
                        OnTimeWin = OnTimeWin + 1000;
                     OnTimeCount++;
                  }
                  gm_Print("Window Start=%d\n",OnTimeWin);
               }
               else if ((PosChange == 1) && (VwdDetectInfoTest.VideoType==NO_ARTIFACT)) 
               {
                  if((OnTimeCount==0) && (OnTimeWin > 10000))
                     OnTimeWin = OnTimeWin - 1000;
                  else if((OnTimeCount==0) && (OnTimeWin > INTL_ONTIMEDEFAULT))
                     OnTimeWin = OnTimeWin - 500;

                  if (OnTimeCount>0)
                  {
                     OnTimeCount--; 
                     if ((OnTimeCount & 0x03)==0x03)
                        OnTimeWin = OnTimeWin - 1000;
                  }

                  gm_Print("Window End=%d\n",OnTimeWin);
               }

               if (FlagToggle==1)
               {
                  FlagToggle=2;
                  TimeStrt_act = gm_GetSystemTime();
                  TimeStrt_step = gm_GetSystemTime();
               }

               if ((VwdDetectInfoTest.VideoType==ARTIFACT) && (FlagToggle==0))
               {
                  TimeStrt_act =gm_GetSystemTime();
               }

               if(FlagToggle==2)
               {
                  if (diffTime_step > INTL_FILTERAPPSTEP)
                  {
                     if(RampUpDown > 25)
                     {
                        TimeStrt_step = gm_GetSystemTime();
                        RampUpDown--;
                        gm_SetScalerSharpness(CH_A,RampUpDown);//0.45
                        if (RampUpDown==25)
                        {		 
                           FlagToggle=0; 
                           gm_Print("Update",0);
                        }
                     }
                  }
               }
            }
            else {
               OnTimeWin=INTL_ONTIMEDEFAULT;
               OnTimeCount =0;
               if (FlagToggle==0)
               {
                  FlagToggle=1;
                  TimeStrt_step = gm_GetSystemTime();
               }

               if (diffTime_step > INTL_FILTERAPPSTEP)
               {
                  if (RampUpDown < 29)
                  {
                     TimeStrt_step = gm_GetSystemTime();
                     RampUpDown++;
                     gm_SetScalerSharpness(CH_A,RampUpDown);//0.5
                     gm_Print("Came Out=%d\n",RampUpDown);
                  }
               }
            }
            PosChange = 0;

         }
         
         //Stop the analysis
         PixCrAnalyzeImage(&VwdDetectInfoTest, &VwdCoarseInfoTest,
               STOP_ANALYZE, VwdConfigTest,
               &VWDStateCurrTest, &CoarseMVWInfoTest, &MVWONTest, tSelectRectext, tExtSelectWR);
      }
      break;
  #endif // SINGLE_FIELD_DEINT_FIXED
#endif

      case Func106:
      #ifdef ST_4K2K_93xx_BOARD
         {
            BYTE bId=GetChipID_4K2K();
            if(bId==CHIPID_4K2K_FE)
               gm_Print("ChipID_4K2K_FE",0);
            else if(bId==CHIPID_4K2K_L)
               gm_Print("ChipID_4K2K_L",0);
            else if(bId==CHIPID_4K2K_R)
               gm_Print("ChipID_4K2K_R",0);
            else
               gm_Print("CHIPID_4K2K_NONE",0);
          }
      #endif   
         break;

      case Func110:
         {
            static BOOL osd_zoom = 0;
            BYTE i;
            WORD xpos, ypos;

            //turn osd zoom on
            if (((BYTE)GetParam(0)) && !osd_zoom)
            {
               gm_SetRegBitsWord(OSD_CONTROL, OSD_ZOOM);
               gm_WriteRegWord(OSD_H_ACTIVE, (gm_ReadRegWord(OSD_H_ACTIVE) >> 1));
               gm_WriteRegWord(OSD_V_ACTIVE, (gm_ReadRegWord(OSD_V_ACTIVE) >> 1));
               gm_SetRegBitsByte(HOST_CONTROL, ODP_FORCE_UPDATE);
               osd_zoom = TRUE;

               for (i = 0 ; i < MAX_NUMBER_OF_TILES_USED ; i++)
               {
                  if ( OK == gm_OsdGetTilePosition(i, &xpos, &ypos))
                  {
                     gm_OsdSetTilePosition(i, xpos >> 1, ypos >> 1);
                  }
               }
               ReverseTileSet();
               gm_SetRegBitsByte(HOST_CONTROL, ODP_FORCE_UPDATE);
               for (i = 0 ; i < MAX_NUMBER_OF_TILES_USED ; i++)
               {
                  if ( OK == gm_OsdGetTilePosition(i, &xpos, &ypos))
                  {
                     gm_OsdSetTilePosition(i, xpos >> 1, ypos >> 1);
                  }
               }
            }
            //turn osd zoom off
            else if (!((BYTE)GetParam(0)) && osd_zoom)
            {
               gm_ClearRegBitsWord(OSD_CONTROL, OSD_ZOOM);
               gm_WriteRegWord(OSD_H_ACTIVE, (gm_ReadRegWord(OSD_H_ACTIVE) << 1));
               gm_WriteRegWord(OSD_V_ACTIVE, (gm_ReadRegWord(OSD_V_ACTIVE) << 1));
               gm_SetRegBitsByte(HOST_CONTROL, ODP_FORCE_UPDATE);
               osd_zoom = FALSE;

               for (i = 0 ; i < MAX_NUMBER_OF_TILES_USED ; i++)
               {
                  if ( OK == gm_OsdGetTilePosition(i, &xpos, &ypos))
                  {
                     gm_OsdSetTilePosition(i, xpos << 1, ypos << 1);
                  }
               }
               ReverseTileSet();
               gm_SetRegBitsByte(HOST_CONTROL, ODP_FORCE_UPDATE);
               for (i = 0 ; i < MAX_NUMBER_OF_TILES_USED ; i++)
               {
                  if ( OK == gm_OsdGetTilePosition(i, &xpos, &ypos))
                  {
                     gm_OsdSetTilePosition(i, xpos << 1, ypos << 1);
                  }
               }
            }
            break;
         }

      case Func111:
         {
            switch ((BYTE)GetParam(0))
            {
               case OSD_NON_ROTATED:
                  UserPrefOsdRotation = OSD_NON_ROTATED;
                  break;
               case OSD_ROTATED_90:
                  UserPrefOsdRotation = OSD_ROTATED_90;
                  break;
               case OSD_ROTATED_180:
                  UserPrefOsdRotation = OSD_ROTATED_180;
                  break;
               case OSD_ROTATED_270:
                  UserPrefOsdRotation = OSD_ROTATED_270;
                  break;
               default:
                  UserPrefOsdRotation = OSD_NON_ROTATED;
                  break;
            }
            AdjustOsdRotation();
            //gmvb_OsdRotation = UserPrefOsdRotation;
            break;
         }
    #if (defined(DEBUG_HANDLER) & defined(MONITOR_MAIN_LOOP_TIME))
      case Func120:
         gm_Print("Clear Main Loop Max Time record",0);
         ClearDebugMaxLoopTime();
         break;
    #endif

    #if defined(DEBUG_HANDLER) && defined(MONITOR_STACK_USAGE)
      case Func121:
         gm_Print("Available Stack size = %d",(W_StackStart - W_StackEnd));
         gm_Print("Max Used Stack size  = %d",(W_StackStart - W_StackMark));
         gm_Print("Stack Start Addr = 0x%X",W_StackStart);
         gm_Print("Stack End   Addr = 0x%X",W_StackEnd);
         break;
    #endif
    
#endif // FULL_APPSTEST_CMD

#if (FEATURE_ODP_PIXEL_GRAB == ENABLE)
      case Func185:
         {
            // GetParam(0) - dummy because WORD is needed for all parameters
            //
            gmt_PixelGrab OdpPixelGrabValues;
            gm_OdpPixelGrab ((WORD) GetParam(0), (WORD) GetParam(1), &OdpPixelGrabValues);
            msg_i("  ODP Pixel Grab - R = %d", OdpPixelGrabValues.W_PixelGrabRed);
            msg_i("  ODP Pixel Grab - G = %d", OdpPixelGrabValues.W_PixelGrabGreen);
            msg_i("  ODP Pixel Grab - B = %d", OdpPixelGrabValues.W_PixelGrabBlue);
         }
         break;

      case Func186:
         {
            // GetParam(0) - dummy because WORD is needed for all parameters
            //
            gmt_PixelGrab OdpPixelGrabValues;
            WORD i;
            for (i=(WORD)GetParam(0);i<=(WORD) GetParam(1);i++)
            {
               gm_OdpPixelGrab (i, (WORD) GetParam(2), &OdpPixelGrabValues);
               if ((BYTE)GetParam(3) == 0)
                  msg_i("%d",OdpPixelGrabValues.W_PixelGrabRed);
               else if ((BYTE)GetParam(3) == 1)
                  msg_i("%d",OdpPixelGrabValues.W_PixelGrabGreen);
               else if ((BYTE)GetParam(3) == 2)
                  msg_i("%d",OdpPixelGrabValues.W_PixelGrabBlue);
               else//if ((BYTE)GetParam(3) == 3)
               {
                  msg_i(" %d", OdpPixelGrabValues.W_PixelGrabRed);
                  msg_i("   %d", OdpPixelGrabValues.W_PixelGrabGreen);
                  msg_i("     %d", OdpPixelGrabValues.W_PixelGrabBlue);
               }
            }
         }
         break;
#endif // FEATURE_ODP_PIXEL_GRAB == ENABLE

#ifdef FULL_APPSTEST_CMD
 #ifdef OSD_PORT_CHANGE_DIALOG
      case Func187:
         ToggleMenuStyle();

         if (UserPrefOldOrNewPortChange)
            msg("/tNew Menu Style is in use");
         else
            msg("/tOld Menu Style is in use");
         break;
 #endif
      case Func188:
 #ifdef VIRTUAL_GPIO_KEY
         SetVirtualGPINPUT((WORD)GetParam(0));
 #endif
         break;

         // Set Height for overlapped modes for SOG
      case Func190:
         B_Overlap_Height = (BYTE) GetParam(0);

         if (B_Overlap_Height == OVERLAP_GR350)
            gm_Print("Set Height of overlapped mode to 350",0);
         if (B_Overlap_Height == OVERLAP_GR400)
            gm_Print("Set Height of overlapped mode to 400",0);
         break;

      case Func191:
 #ifdef BLACK_FRAME_INSERTION_METHOD
         Force_Display_Window_With_Black_Color();
 #endif
         break;

      case Func192:
 #ifdef GREY_FRAME_INSERTION_METHOD
         Force_Display_Window_With_Grey_Color();
 #endif
         break;

      case Func193:
 #if (defined(ACC_LUT_METHOD))
         //
         // Load ACC table:
         //
         Load_ACC_High_And_Low_Luma_Luts ();
 #endif
         break;

      case Func194:
         {
            WORD B_Buf[528];
            WORD W_Size;
            WORD i;
            WORD W_Offset;
            BYTE B_VH;


            if(GetParam(0) < 4)
            {
               B_VH = gmd_TAB_MVF;
               W_Size = 528; // 1056 bytes
            }
            else
            {
               B_VH = gmd_TAB_MHF;
               W_Size = 288; // 576 bytes
            }

            if(GetParam(0) == 0)
            {
               W_Offset = UV_MVF_START_ADDR;               
            }
            else if(GetParam(0) == 1)
            {
               W_Offset = Y_MVF_START_ADDR;               
            }
            else if(GetParam(0) == 2)
            {
               W_Offset = UVPK_MVF_START_ADDR;               
            }            
            else if(GetParam(0) == 3)
            {
               W_Offset = YPK_MVF_START_ADDR;               
            }            
            else if(GetParam(0) == 4)
            {
               W_Offset = UV_MHF_START_ADDR;               
            }            
            else if(GetParam(0) == 5)
            {
               W_Offset = Y_MHF_START_ADDR;               
            }            
            else if(GetParam(0) == 6)
            {
               W_Offset = UVPK_MHF_START_ADDR;               
            }            
            else if(GetParam(0) == 7)
            {
               W_Offset = YPK_MHF_START_ADDR;               
            }            

            gm_Printf(" Index : %d ", (WORD)GetParam(0));

            gm_ReadSRAMBlock(B_VH, gmd_HIGH_PRIORITY_MODE, (WORD far*)&B_Buf, W_Offset, W_Size*2);

            for(i = 0; i < W_Size; i++)
            {
               gm_Delay1ms(1);
               gm_Printf(" %d 0x%x ", i, B_Buf[i]);
            }
            gm_Printf("Done", 0);
         }
         break;
         
 #ifdef EXTERNAL_DECODER
      case Func200:
         {
  #ifdef ANX9021_DEV_ID
            //BYTE Anx9021_Port = 0;
            BYTE Anx9021_Sel = (BYTE)GetParam(0);
            BYTE Daughter_board_out_mode = (BYTE)GetParam(1);
            BYTE Daughter_board_pin_map = (BYTE)GetParam(2);

            if (Anx9021_Sel == 0)
            {
               gm_Print("appstest 200 0x1 0xC0 0x5", 0);
               gm_Print("0x1  stands for output setting of anx9021", 0);
               gm_Print("0xC0 stands for YCbCr422 in anx9021", 0);
               gm_Print("0x5  stands for DIP_BGR_UYV in DIP's pin map", 0);
               gm_Print("Make sync signal stop a while and then re-input sync,", 0);
               gm_Print("so that the image becomes normal ", 0);
            }
            else if (Anx9021_Sel == 1)
            {
               ANX9021_SetBoardOutputFormat(Daughter_board_out_mode);
               gm_DipConfig(0, 6, Daughter_board_pin_map);
            }
            else {}
  #endif
         }
         break;

      case Func201:
         {
  #ifdef ANX9021_DEV_ID
            BYTE Daughter_board_out_mode;
            Daughter_board_out_mode = (BYTE)GetParam(0);

            ANX9021_SetBoardOutputFormat(Daughter_board_out_mode);
            ModeSetInputAttrPrefs(0);
            ModeDetermine(0);
  #endif
         }
         break;
      case Func202:
         {
            BYTE Daughter_board_pin_map;
            Daughter_board_pin_map = (BYTE)GetParam(0);

            gm_Print("Daughter_board_pin_map = %d", Daughter_board_pin_map);
            gm_DipConfig(0, 6, Daughter_board_pin_map);

         }
         break;
 #endif // EXTERNAL_DECODER

      case Func203:
        {
	    gm_Print("Game mode ...", 0);
	    UserPrefGameMode = (BYTE)GetParam(0);
	    AdjustGameMode();
	}
	break;
 #if 0
      case Func210:
         {
            BYTE TempVariable;
            gm_DecoderWriteRegisterByte( 0, SIGPROC_INDEX_ADDR_REG , 0x86 );
            gm_Delay1ms(10);
            TempVariable = gm_DecoderReadRegisterByte(0 ,SIGPROC_INDEX_DATA_REG );
            if ( GetParam(0) == 1 )
            {
               TempVariable = TempVariable | 0x02;
            }
            else
            {
               TempVariable = TempVariable & 0xFD;
            }

            gm_DecoderWriteRegisterByte( 0 ,SIGPROC_INDEX_DATA_REG , TempVariable );

         }
         break;

      case Func211 :
         {
            BYTE TempVariable;
            gm_DecoderWriteRegisterByte( 0, SIGPROC_INDEX_ADDR_REG , 0x02 );
            gm_Delay1ms(10);
            TempVariable = gm_DecoderReadRegisterByte(0 ,SIGPROC_INDEX_DATA_REG );
            if ( GetParam(0) == 1 )
            {
               TempVariable = TempVariable | 0x80;
            }
            else
            {
               TempVariable = TempVariable & 0x7F;
            }

            gm_DecoderWriteRegisterByte( 0 ,SIGPROC_INDEX_DATA_REG , TempVariable );
         }
         break;
 #endif

      case Func212:
         {
            BYTE i;
            //WORD W_RESTORE_PB_BKGND;

				gm_ClearAndSetBitsWord(IMP_CONTROL,IMP_EXT_DV_EN,IMP_RUN_EN);
	    		gm_FifoControl(TRUE, MC_FIFO_MAIN_WR);
            //W_RESTORE_PB_BKGND = gm_ReadRegWord(PB_BKGND);
            gm_WriteRegWord (AUTO_BKGND_CONTROL, 0);
            //gm_WriteRegWord (CLOCK_CONFIG2, 0x91A);

            gm_WriteRegWord (CLOCK_CONFIG2, 0x50AA); //0x199A);
            //	 gm_WriteRegWord (CLOCK_CONFIG2, 0x222A);

            gm_WriteRegWord(AFR_CONTROL, 0);
            gm_SetRegBitsWord(DISPLAY_DEC_STATUS, IMP_AFR_DETECT);

            SetupInputOutputForTPG();

            PatGenOverride();

            gm_OsdHide();
            gm_ClearRegBitsWord(OSD_CONTROL, OSD_ENABLE);
           while(1)
            {
		UserPrefBackLight++;
	//	if(UserPrefBackLight>=30)
	//		UserPrefBackLight=0;
               SetupPatGen(gmvb_MainChannel, 16);
               gm_Delay10ms(50);//0);
               gm_Printf("Main Pattern no.: %d", UserPrefBackLight);
           	}
#if 0
           while(1)
            {
            i++;
	if(i>=30)
		i=0;
               SetupPatGen(gmvb_MainChannel, (BYTE)i);
               gm_Delay10ms(50);//0);
               gm_Printf("Main Pattern no.: %d", i);
            }
#else
            SetupPatGen(gmvb_MainChannel, 0x14);
#endif

#if 0
            SetupPatGen(gmvb_MainChannel, 0);
            // phyang 07/26/06: Why back to RF1? Can't remain at the same port?
            //UserPrefCurrentInputMain = ALL_BOARDS_RF1;
            //SelectInput(gmvb_MainChannel, UserPrefCurrentInputMain, NORMAL_MODE);
            //SCR2454
            //Restore original background color
            gm_WriteRegWord(PB_BKGND, W_RESTORE_PB_BKGND);
            //connect given input to Main or Pip Channel
            SelectInput(gmvb_MainChannel, UserPrefCurrentInputMain, NORMAL_MODE);
            SelectInput(gmvb_PipChannel, UserPrefCurrentInputPip, NORMAL_MODE);
#endif
         }

         //Freeze(gmvb_MainChannel, TRUE);
         break;

      case Func213:
         {
            BYTE i;

            SetupPatGen(gmvb_PipChannel, 0);
            gm_Delay10ms(100);

            gm_Printf("Please make sure Pip is on before running this appstest",0);

            for (i = 0; i<31; i++)
            {
               SetupPatGen(gmvb_PipChannel, (BYTE)i);
               gm_Delay10ms(100);
               gm_Printf("Pip Pattern no.: %d", i);
            }
            SetupPatGen(gmvb_PipChannel, 0);
            SelectInput(gmvb_MainChannel, UserPrefCurrentInputMain, NORMAL_MODE);
            SelectInput(gmvb_PipChannel, UserPrefCurrentInputPip, NORMAL_MODE);
         }
         break;

      case Func220:
         {
            BYTE B_Input = (BYTE) GetParam(0);
            if ( B_Input == VQLAB_MAY07_METHOD)
            {
               gm_Printf("Selecting VQLAB_MAY07_METHOD");
               gmvb_AdaptiveTNRMethod = VQLAB_MAY07_METHOD;
            }
            else if ( B_Input == CORTEZ27REL_METHOD)
            {
               gm_Printf("Selecting CORTEZ27REL_METHOD");
               gmvb_AdaptiveTNRMethod = CORTEZ27REL_METHOD;
            }
            if (UserPrefTNR != DISABLE)
            {
               gm_VideoProcessingEnable(VIDEO_TNR);
               gm_VideoProcessingAdjustment(VIDEO_TNR, UserPrefTNR);
            }
            else
            {
               gm_VideoProcessingDisable(VIDEO_TNR);
            }
         }
         break;
#endif // FULL_APPSTEST_CMD
#if (FEATURE_DISPLAY_PORT == ENABLE)
#ifdef DP12_BATHTUB_ENABLE //JC_Athena_bathtub
      case Func221:
         {
            if ((BYTE)GetParam(0)==4)
            {
               BYTE tmp;
               for (tmp=0;tmp<4;tmp++)
                  DPRxBathTubMeas(tmp, (BYTE)GetParam(1),(BYTE)GetParam(2) );
            }
            else
               DPRxBathTubMeas((BYTE)GetParam(0), (BYTE)GetParam(1),(BYTE)GetParam(2));
            break;
         }
#endif

	#ifdef DP11_BATHTUB_ENABLE
	case Func222:
		DPRxBathTubMeas_DP((BYTE)GetParam(0), (BYTE)GetParam(1), (BYTE)GetParam(2), (WORD)GetParam(3), (BYTE)GetParam(4));
		break;
	#endif

#ifdef DP12RX_PRINT_DEBUG_MSG
	case Func223:
		if ((BYTE)GetParam(0)== 1)
		{
			DP_EnablePrints(TRUE);
			gm_Print("\n\r DP Lib Enable Prints.\n", 0);			
		}
		else
		{
			DP_EnablePrints(FALSE);
			gm_Print("\n\r DP Lib Disable Prints.\n", 0);			
		}
		break;
#endif
#endif
#ifdef FULL_APPSTEST_CMD
      case Func230:                 //window size and place
#ifdef ENABLE_RECORD_DATA
         {
            BYTE Counter= 0, i= 0, j=0;
            Counter = (BYTE)GetParam(0);

            if (Counter == 99)
            {
               Lost_Count = 0;
               for (j=0; j<10; j++)
               {
                  for (i=0; i<23; i++)
                  {
                     Lost_Data[j][i] = 0;
                  }
               }
               gm_Print("Clear lost counter %d", Lost_Count);
            }
            else
            {
               gm_Print("Total lost %d", Lost_Count);

               gm_Printf("Lost %d,  100H = %x %x %x %x %x %x %x %x",  Counter, Lost_Data[Counter][0], Lost_Data[Counter][1], Lost_Data[Counter][2],
                         Lost_Data[Counter][3],Lost_Data[Counter][4], Lost_Data[Counter][5], Lost_Data[Counter][6], Lost_Data[Counter][7]);

               gm_Printf("Lost %d,  200H = %x %x %x %x %x %x %x %x", Counter, Lost_Data[Counter][8], Lost_Data[Counter][9], Lost_Data[Counter][10],
                         Lost_Data[Counter][11],Lost_Data[Counter][12], Lost_Data[Counter][13], Lost_Data[Counter][14], Lost_Data[Counter][15]);

               gm_Printf("Lost %d, %x %x %x %x %x %x Time = %d", Counter, Lost_Data[Counter][16], Lost_Data[Counter][17], Lost_Data[Counter][18],
                         Lost_Data[Counter][19],Lost_Data[Counter][20], Lost_Data[Counter][21], (WORD)((Lost_Data[Counter][22] << 8) & 0xFF00) | Lost_Data[Counter][23]);
            }
         }
#endif

         break;
#endif // FULL_APPSTEST_CMD

#ifdef USE_IMP_PIXGRAB_CMD
      case Func240:
         {
            WORD i = (WORD)GetParam(3);
            WORD dwPixGbHStart = gm_ReadRegWord(IMP_IBD_HSTART) + (WORD)GetParam(1);
            WORD dwPixGbVStart = gm_ReadRegWord(IMP_IBD_VSTART) + (WORD)GetParam(2);

            if ((BYTE)GetParam(0) > 3)
            {
               gm_Print("appstest 240 [COLOR] [X] [Y] [COUNTER]",0);
               break;
            }

            do
            {
               if ((BYTE)GetParam(0) < 3)
                  gm_Print("%d",GrabPixel(gmvb_MainChannel, dwPixGbHStart, dwPixGbVStart, (gmt_RGB_COLOR)GetParam(0)));
               else
               {
                  gm_Print(" R  %d",GrabPixel(gmvb_MainChannel, dwPixGbHStart, dwPixGbVStart, 0));
                  gm_Print(" G  %d",GrabPixel(gmvb_MainChannel, dwPixGbHStart, dwPixGbVStart, 1));
                  gm_Print(" B  %d",GrabPixel(gmvb_MainChannel, dwPixGbHStart, dwPixGbVStart, 2));
                  gm_Print("",0);
               }
            } while (i--);
            break;
         }
#endif // USE_IMP_PIXGRAB_CMD

#ifdef FULL_APPSTEST_CMD
      case Func241:
         {
            BYTE type; // 0: bksv block, 1: only bksv
            BYTE status;
            type = (BYTE)GetParam(0);
            if((status=WriteLogicalPortBksvToOtp(TempBuffer,type)) != 0)
            {
               gm_Print("WriteLogicalPortBKSVToOtp fail ! Status =%d",status);
               gm_Print("Status: ");
               gm_Print("   0: OK");
               gm_Print("   1: init fail");
               gm_Print("   2: write fail"); 
               gm_Print("   3: OTP already have LogicalPortBKSV"); 
               gm_Print("   4: BKSV block incorrect");
               gm_Print("   5: BKSV key incorrect");
            }
         }
         break;

      case Func242:
         {
            BYTE readbuf[5];
            if(GetLogicalPortBksv(readbuf))
            {
               gm_Print("BKSV[0] = 0x%X",readbuf[0]);
               gm_Print("BKSV[1] = 0x%X",readbuf[1]);
               gm_Print("BKSV[2] = 0x%X",readbuf[2]);
               gm_Print("BKSV[3] = 0x%X",readbuf[3]);
               gm_Print("BKSV[4] = 0x%X",readbuf[4]);
            }
            else
               gm_Print("Can't find 2nd BKSV !");
         }
         break;

      case Func243:
         break;

      case 244:
#if (FEATURE_CEC == ENABLE)
         a_CECDevices[1].PhysicalAddr = PHYS_ADDR_TO_WORD(1, 0, 0, 0);
         a_CECDevices[2].PhysicalAddr = PHYS_ADDR_TO_WORD(2, 0, 0, 0);
         a_CECDevices[3].PhysicalAddr = PHYS_ADDR_TO_WORD(1, 1, 0, 0);
         a_CECDevices[4].PhysicalAddr = PHYS_ADDR_TO_WORD(1, 2, 0, 0);
         a_CECDevices[5].PhysicalAddr = PHYS_ADDR_TO_WORD(1, 3, 0, 0);
         a_CECDevices[6].PhysicalAddr = PHYS_ADDR_TO_WORD(3, 1, 1, 1);
         a_CECDevices[7].PhysicalAddr = PHYS_ADDR_TO_WORD(3, 1, 1, 2);
         a_CECDevices[8].PhysicalAddr = PHYS_ADDR_TO_WORD(3, 1, 1, 3);
         a_CECDevices[9].PhysicalAddr = PHYS_ADDR_TO_WORD(1, 1, 1, 4);
 #if 0
  #if (FEATURE_CEC == ENABLE)
         Appstest_Cec();
  #endif
 #endif
#endif // FEATURE_CEC
         break;

      case Func245:
         SelectPanel();
         break;

      case Func246:
	  	#ifdef ST_4K2K_93xx_BOARD 
			if ((BYTE)GetParam(0) == 1)
				{
					gm_Print("Panel Power on", 0);
					PanelPowerOn4k2k();
				}
			else if ((BYTE)GetParam(0) == 0)
				{
					gm_Print("Panel Power off", 0);
					PanelPowerOff4k2k();
				}
		 #endif
         break;

      case Func247:
         {
            gmt_BusOwn HostBusOwner;
            BYTE HostBusOwnSel = (BYTE)GetParam(0);

            if (HostBusOwnSel == 0)
            {
               HostBusOwner = BUS_MISSION;
               if (gm_MessageSend(gmd_MESSAGE_HOSTBUS, &HostBusOwner, sizeof(HostBusOwner))==FALSE)
                  gm_Print("---FAIL to switch shared host bus to Mission",0);
               else
                  gm_Print("---OK to switch shared host bus to Mission",0);
            }
            else// if(HostBusOwnSel == 1)
            {
               HostBusOwner = BUS_LPM;
               if (gm_MessageSend(gmd_MESSAGE_HOSTBUS, &HostBusOwner, sizeof(HostBusOwner))==FALSE)
                  gm_Print("---FAIL to switch shared host bus to LPM",0);
               else
                  gm_Print("---OK to switch shared host bus to LPM",0);
            }
            break;
         }

      case Func248:
         {
            WORD Power_state[1];
            Power_state[0] = Pwr_Sleep;
            gm_MessageSend(gmd_MESSAGE_POWER, &Power_state[0], sizeof(Power_state)); // Replace by gmd_MESSAGE_M2L later!
         }
         break;

      case Func249:
         {
            WORD command[3];
            WORD back = 0;

            command[0]= (WORD)GetParam(0);   //0:read   1:write
            command[1]= (WORD)GetParam(1);   //address
            command[2]= (WORD)GetParam(2);   //value

            if (gm_MessageSend(gmd_MESSAGE_DEBUG, &command, sizeof(command)) == FALSE)
               gm_Print("---Fail to send debug request to LPM",0);
            else
            {
               gm_Print("---OK to send debug request to LPM",0);

               if (command[0] == 0) //read, wait for receiving return value
               {
                  while (1)
                  {
                     if (gm_MessageReceived(gmd_MESSAGE_DEBUG, &back, sizeof(back) ) )
                     {
                        gm_Printf("*** Addr 0x%x = 0x%x",command[1], back);
                        break;
                     }
                  }
               }
               else if (command[0] == 1) //write
               {
                  gm_Printf("*** Write 0x%x to addr 0x%x",command[2], command[1]);
               }
            }
            break;
         }

      case Func250:
         gm_Print ("ACC Dump %d", GetParam(0));
 #if (FEATURE_ACC3 == ENABLE)
         gm_ACCDump((BYTE)GetParam(1), (BYTE)GetParam(2));
 #endif //(FEATURE_ACC3 == ENABLE)
         break;
#endif // FULL_APPSTEST_CMD

      case Func251:
      #if FEATURE_AIP == ENABLE
         {
            BYTE B_Param1 = (BYTE)GetParam(0);
            BYTE B_Param2 = (BYTE)GetParam(1);

            gm_AutoGeometry(B_Param1, B_Param2);
            msg("AutoGeometry End");            
         }
      #endif
         break;
         
#ifdef FULL_APPSTEST_CMD
      case Func252:
         gm_EnablePrints(TRUE);
         if(GetParam(0))
         {
            gm_Print("Enable UART Prints.");
         }
         else
         {
            gm_Print("Disable UART Prints.");
            gm_EnablePrints(FALSE);
         }
         break;
#endif

#ifdef USE_IMP_PIXGRAB_CMD
      case Func253:
         Pixel_Scan();
         break;
#endif

      case Func254:
#if (FEATURE_SOCIPC == ENABLE)
		/*ChengLin 20120117: SOCIPC Test Command Sets*/
		SocIPCAppsTestHandler();
#endif	
         break;

         //
         // Menu
         //
      default:
#ifdef FULL_APPSTEST_CMD
         msg("AppsTest");
         msg("Choose one of the following options:");
         msg(" 0 = Initialize NVRam");
         msg(" 1 = Enable/disbale input valid mode");
         msg(" 2 = Select Input Port");
         msg(" 3 = Change Output Size");
         msg(" 4 = Change Output Position");
         msg(" 5 = Change Main Aspect Ratio");
         msg(" 6 = ADC Calibration");
         msg(" 7 = Enable Disable mode detection");
         msg(" 8 = Show NVRAM map");
         msg(" 9 = PIP Blending");
         msg("10 = Frame Lock Mode");
         msg("11 = Show SDRAM mapping");
         msg("12 = Select which H+V filters");
         msg("15 = Zoom Main");
         msg("16 = Reset 0 command");
         msg("17 = Adjust Gamma");
         msg("18 = Misc test");
         msg("19 = Video Processing");
         msg("22 = Debug Modesetup 0=All 1=Processing");
         msg("23 = Mode Setup User Pref");
         msg("24 = Display Native Mode");
         msg("25 = Select Pip channel port");
         msg("26 = Swap Main/Pip");
         msg("30 = DDR Test");
         msg("33 = 3x3 matrix selection");
         msg("39 = Main/Pip video demo function");
         msg("43 = Main/Pip audio demo function");
         msg("60 = I2C read test(syntax: appstest 60 deviceAddr offsetToRead)");
         msg("61 = I2C response test (addresses of responding devices will be listed)");
         msg("62 = I2C write test(syntax: appstest 62 deviceAddr offsetToWrite valueToWrite)");
         msg("70 = Interlace output control (syntax (all boolean): appstest 70 Enable FreeRunODD InvertInternalODD InvertOutputODD)");
         msg("96 = Multi-display feature");
         msg("185 = Display ODP pixel grab");
         msg("186 = Display ODP pixel grab values");
         msg("220 = Select adaptive TNR method");
         msg("245 = Panel Selection");
         msg("250 = ACC Dump Buffer, Mode (0=off, 1=in hist, 2=out hist, 3=both hist), Frame Count");
         msg("251 = AutoGeometry");
         msg("252 = Enable/disable UART print messages");
#if (FEATURE_SOCIPC == ENABLE)
         msg("254 = SoCIPC Test Commands");
#endif
#else
         msg("Simplified Appstest Cmd");
#endif
         break;
   }
}

//******************************************************************************
// Initialize NVRAM
// No Parameters
//******************************************************************************
//******************************************************************************
//
// FUNCTION       : void InitializeNvram(void)
//
// USAGE          :
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        :
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void InitializeNvram(void)
{
   BYTE i;

#if FEATURE_AIP == ENABLE    
   //
   // Initialize ADC setting
   //
   gmt_DecoderTbcSettingsType DecoderTbcSettings;
   gmt_ADC_SETTING AdcSettings;

   // Init ADC Bandgap
   i = 0xFF;
   SaveBandgapSetting(&i);
   for (i = 0; i < NumberOfAdcSetting; i++)
   {
      AdcSettings = AdcDefaultSetting[i];
      SaveAdcSettings(0, (BYTE *)&AdcSettings, i);
   }
#endif

   //
   // Change checksum value
   //
   i = NVRam_ReadByte (ModeIndependentChecksumStart);
   i--;
   NVRam_WriteByte (ModeIndependentChecksumStart,i);


   InitNvram();

   msg("NVRAM will initialize on next reboot");
}

//******************************************************************************
// Show NVRAM mapping
// No Parameters
//******************************************************************************
//******************************************************************************
//
// FUNCTION       : void ShowNvramMap(void)
//
// USAGE          :
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        :
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
#ifdef FULL_APPSTEST_CMD
void ShowNvramMap(void)
{
   msg("NVRAM mapping : ");
   msg_i("  ADC calibration setting START - %d", AdcSettingsStart);
   msg_i("  ADC calibration setting size    - %d", AdcSettingsSize +
         ChecksumSize);
   msg_i("  ADC calibration setting num     - %d", NumberOfAdcSetting);

   msg_i("  Mode Dependent preset START   - %d", ModeDependentPresetEntryStart);
   msg_i("  Mode Dependent preset size      - %d", ModeDependentUserPrefSize +
         //sizeof(B_AutoAdjustCount)+
         sizeof(BYTE)+
         ChecksumSize);
   msg_i("  Mode Dependent preset num       - %d", NumOfGraphicsPresetMode);

   msg_i("  Mode Dependent lookup START   - %d", ModeDependentUserLookUpStart);
   msg_i("  Mode Dependent lookup size      - %d", ModeDependentLookUpSize +
         ChecksumSize);
   msg_i("  Mode Dependent lookup num       - %d", MAX_USER_MODES_SAVED);

   msg_i("  Mode Dependent user START     - %d", ModeDependentUserEntryStart);
   msg_i("  Mode Dependent user size        - %d", ModeDependentUserPrefSize +
         //sizeof(B_AutoAdjustCount)+
         sizeof(BYTE)+
         ChecksumSize);
   msg_i("  Mode Dependent user num         - %d", MAX_USER_MODES_SAVED);

#ifdef CVT_MODES_SUPPORT
   msg_i("  Mode Dependent CVT lookup START   - %d", ModeDependentCvtLookUpStart);
   msg_i("  Mode Dependent CVT lookup size      - %d", ModeDependentLookUpSize +
         ChecksumSize);
   msg_i("  Mode Dependent CVT lookup num       - %d", MAX_CVT_MODES_SAVED);

   msg_i("  Mode Dependent CVT entry START     - %d", ModeDependentCvtEntryStart);
   msg_i("  Mode Dependent CVT entry size        - %d", ModeDependentUserPrefSize +
         //sizeof(B_AutoAdjustCount)+
         sizeof(BYTE)+
         ChecksumSize);
   msg_i("  Mode Dependent CVT entry num         - %d", MAX_CVT_MODES_SAVED);
#endif

   msg_i("  Port Dependent setting START  - %d", PortSchemeDependentIndexStart); // PSD
   msg_i("  Port Dependent setting size     - %d", PortSchemeDependentUserPrefSize +
         ChecksumSize);
   msg_i("  Port Dependent setting num      - %d", NUM_OF_CONNECTORS);
   msg_i("  Mode Independent start        - %d", ModeIndependentStart);
   msg_i("  Mode Independent size         - %d", ModeIndependentUserPrefSize);

   //msg_i("NVRAM end - %d", ModeIndependentEnd);

   msg_i("  Auto Scan Input Start START     - %d", AutoScanInputStart);
   msg_i("  Auto Scan Input size           - %d", AutoScanInputEnd - AutoScanInputStart);
   msg_i("NVRAM end - %d", AutoScanInputEnd);
}

//******************************************************************************
// Show SDRAM mapping
// No Parameters
//******************************************************************************
//******************************************************************************
//
// FUNCTION       : void ShowSdramMap(void)
//
// USAGE          :
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        :
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void ShowSdramMap(void)
{
   // SDRAM mapping
   gm_Printf("SDRAM MAPPING: (1 burst = %d bit)", SDRAM_MEM_BURST);
   gm_Printf("--------------");

   gm_Printf("OSD:               0x%lX - 0x%lX : %ld",
             gm_MemoryGetInfo(BUF_OSD, BUF_START_ADDRESS),
             gm_MemoryGetInfo(BUF_OSD, BUF_END_ADDRESS),
             gm_MemoryGetInfo(BUF_OSD, BUF_SIZE));

#if ((FEATURE_DDR_EXECUTION == ENABLE) && ((defined DRAM_CODE_EXECUTION)||(DATA_DRAM_SIZE > 0)))
   gm_Printf("OCM Mapped:        0x%lX - 0x%lX : %ld",
             gm_MemoryCodeStartAddressInBursts(),
             gm_MemoryCodeEndAddressInBursts(),
             gm_MemoryCodeSizeInBursts());
#endif

#if (FEATURE_OVERDRIVE == ENABLE)
   gm_Printf("LCD Overdrive:     0x%lX - 0x%lX : %ld",
             gm_MemoryGetInfo(BUF_LCD_OVERDRIVE, BUF_START_ADDRESS),
             gm_MemoryGetInfo(BUF_LCD_OVERDRIVE, BUF_END_ADDRESS),
             gm_MemoryGetInfo(BUF_LCD_OVERDRIVE, BUF_SIZE));
#endif

   gm_Printf("AVS:               0x%lX - 0x%lX : %ld",
             gm_MemoryGetInfo(BUF_AVS, BUF_START_ADDRESS),
             gm_MemoryGetInfo(BUF_AVS, BUF_END_ADDRESS),
             gm_MemoryGetInfo(BUF_AVS, BUF_SIZE));

#if 0
   gm_Printf("VBI                0x%lX - 0x%lX : %ld",
             gm_MemoryGetInfo(BUF_VBI, BUF_START_ADDRESS),
             gm_MemoryGetInfo(BUF_VBI, BUF_END_ADDRESS),
             gm_MemoryGetInfo(BUF_VBI, BUF_SIZE));

   gm_Printf("Teletext           0x%lX - 0x%lX : %ld",
             gm_MemoryGetInfo(BUF_TELETEXT, BUF_START_ADDRESS),
             gm_MemoryGetInfo(BUF_TELETEXT, BUF_END_ADDRESS),
             gm_MemoryGetInfo(BUF_TELETEXT, BUF_SIZE));


   // Main video - CCS memory buffer
   gm_Printf("CCS                0x%lX - 0x%lX : %ld",
             gm_MemoryGetInfo(BUF_CCS, BUF_START_ADDRESS),
             gm_MemoryGetInfo(BUF_CCS, BUF_END_ADDRESS),
             gm_MemoryGetInfo(BUF_CCS, BUF_SIZE));
#endif

   // Main video - Motion vector memory buffer
   gm_Printf("Motion Vector:     0x%lX - 0x%lX : %ld",
             gm_MemoryGetInfo(BUF_MOTION_VECTORS, BUF_START_ADDRESS),
             gm_MemoryGetInfo(BUF_MOTION_VECTORS, BUF_END_ADDRESS),
             gm_MemoryGetInfo(BUF_MOTION_VECTORS, BUF_SIZE));

   // Main channel buffer
   gm_Printf("MAIN %d buffers:    0x%lX - 0x%lX : %ld",
             gmvb_MemoryNumBuffers[CH_A],
             gm_MemoryGetInfo(BUF_MAIN, BUF_START_ADDRESS),
             gm_MemoryGetInfo(BUF_MAIN, BUF_END_ADDRESS),
             gm_MemoryGetInfo(BUF_MAIN, BUF_SIZE));

   // Main channel bpp
   gm_Printf("Main %d bpp", gmvb_MemoryBpp[CH_A] );

   // PIP channel buffer
   gm_Printf("PIP %d buffers:     0x%lX - 0x%lX : %ld",
             gmvb_MemoryNumBuffers[CH_B],
             gm_MemoryGetInfo(BUF_PIP, BUF_START_ADDRESS),
             gm_MemoryGetInfo(BUF_PIP, BUF_END_ADDRESS),
             gm_MemoryGetInfo(BUF_PIP, BUF_SIZE));

   // Pip channel bpp
   gm_Printf("Pip %d bpp", gmvb_MemoryBpp[CH_B] );

   // Total end address
   gm_Printf("--------------");
   gm_Printf("Memory Total Size: 0x%lX bursts = %ld Mbits", SDRAM_MEM_SIZE, (SDRAM_MEM_SIZE * SDRAM_MEM_BURST) >> 20);

   {
      SDWORD SD_MemFree = MemoryCalculateRemainingSDRAM();
      gm_Printf("Total Memory Free: 0x%lX bursts = %ld Mbits", SD_MemFree,
                (SD_MemFree * SDRAM_MEM_BURST) >> 20);
   }
}

//******************************************************************************
// Enable / Disable mode detection
// param 0 - "0" - disable "1" - enable
//******************************************************************************
//******************************************************************************
//
// FUNCTION       : void EnableDisableDetection(void)
//
// USAGE          :
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        :
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void EnableDisableDetection(void)
{
   if (GetParam(0) == 0)
   {
      CLRBIT(gmvw_HwUpdateCtrl,INPUT_DETECT_HDLR);
      CLRBIT(gmvw_HwUpdateCtrl,DECODER_UPDATE_HDLR);
      CLRBIT(gmvw_HwUpdateCtrl,DECODER_NOISE_FILETR);
      msg("Disable Mode Detection");
   }
   else
   {
      SETBIT(gmvw_HwUpdateCtrl,INPUT_DETECT_HDLR);
      SETBIT(gmvw_HwUpdateCtrl,DECODER_UPDATE_HDLR);
      SETBIT(gmvw_HwUpdateCtrl,DECODER_NOISE_FILETR);
      msg("Re-Enable Mode Detection");
   }
}

//******************************************************************************
// Enable / Disable input signal
// param 0 - channel number
// param 1 - "0" = invalid mode      "1" = valid mode
//******************************************************************************
//******************************************************************************
//
// FUNCTION       : void EnableDisableInput(void)
//
// USAGE          :
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        :
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void EnableDisableInput(void)
{
   WORD W_Param0 = (BYTE)GetParam(0);
   WORD W_Param1 = (WORD)GetParam(1);

   if ((W_Param0 > 1) || (W_Param1 > 1))
   {
      msg("Syntax: appstest 1 <channel> <state>");
      msg(" <state 0> = Signal Loss");
      msg(" <state 1> = Signal Stable");
   }
   else
   {
      switch (W_Param1)
      {
         case 0:
            CLRBIT(stInput[W_Param0].ModeFlags, gmd_MODE_VALID);
            ModeSetupFreerun(W_Param0);
            msg_i("Signal Lost CH(%d)", W_Param0);
            break;

         case 1:
            SETBIT(stInput[W_Param0].ModeFlags, gmd_MODE_VALID);
            // Add short pulse for DP later.
            ModeSetupFullSetup(W_Param0);
            msg_i("Signal Stable CH(%d)", W_Param0);
            break;
      }
   }
}

#ifndef NO_PIP_SUPPORT
void DebugAdjustPip(void)
{
   BYTE B_PipMode = (BYTE)GetParam(0);
   BYTE B_Port  = (BYTE)GetParam(1);

   if ((BYTE)GetParam(0) >= NUM_PIP_MODES)
   {
      gm_Print("UserPrefCurrentInputPip=%d",UserPrefCurrentInputPip);
      gm_Print("UserPrefPipMode=%d",UserPrefPipMode);
      return;
   }

   if (B_PipMode > SIDE_BY_SIDE_TALL)
      B_PipMode = SIDE_BY_SIDE_TALL;
   UserPrefPipMode=B_PipMode;
   gm_Print("UserPrefPipMode=%d",UserPrefPipMode);

   if (B_Port >= NUM_MASTER_LIST)
      B_Port = ALL_BOARDS_VGA1;
   UserPrefCurrentInputPip= B_Port;
   gm_Print("UserPrefCurrentInputPip=%d",UserPrefCurrentInputPip);
   AdjustCurrentInputPip();
   AdjustPipMode();
}
#endif // NO_PIP_SUPPORT
#endif // FULL_APPSTEST_CMD

//******************************************************************************
// Select input port on Main channel
// param 0 - port number
//******************************************************************************
//******************************************************************************
//
// FUNCTION       : void DebugSelectInput(void)
//
// USAGE          :
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        :
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void DebugSelectInput(void)
{
   BYTE B_Param = (BYTE)GetParam(0);

   switch (B_Param)
   {
         // EV1,EV2,RD1
      case ALL_BOARDS_CVBS1: // 0
         gm_Print("Select ALL_BOARDS_CVBS1", 0);
         break;
      case ALL_BOARDS_SVIDEO1:	// 1
         gm_Print("Select ALL_BOARDS_SVIDEO1", 0);
         break;
      case ALL_BOARDS_COMP1: // 2
         gm_Print("Select ALL_BOARDS_COMP1", 0);
         break;
      case ALL_BOARDS_VGA1: // 3
         gm_Print("Select ALL_BOARDS_VGA1", 0);
         break;
      case ALL_BOARDS_DVI1: // 4
         gm_Print("Select ALL_BOARDS_DVI1", 0);
         break;
      case ALL_BOARDS_DVI2: // 5
         gm_Print("Select ALL_BOARDS_DVI2", 0);
         break;
      case ALL_BOARDS_DIP1: // 6
         gm_Print("Select ALL_BOARDS_DIP1", 0);
         break;
      case ALL_BOARDS_HDMI: // 7
         gm_Print("Select ALL_BOARDS_HDMI", 0);
         break;
      case ALL_BOARDS_HDMI2: // 8
         gm_Print("Select ALL_BOARDS_HDMI2", 0);
         break;
      case ALL_BOARDS_HDMI3: // 9
         gm_Print("Select ALL_BOARDS_HDMI3", 0);
         break;
      case ALL_BOARDS_DP1: // 10
         gm_Print("Select ALL_BOARDS_DP1", 0);
         break;
      case ALL_BOARDS_DP2: // 11
         gm_Print("Select ALL_BOARDS_DP2", 0);
         break;
#ifdef ST_4K2K_93xx_BOARD
      case ALL_BOARDS_DP3: // 12
         gm_Print("Select ALL_BOARDS_DP3", 0);
         break;
#endif         
      default:
         gm_Print("Select Main Input:", 0);
         gm_Print("   appstest 2 <port>", 0);
         gm_Print("    port: 0 - ALL_BOARDS_CVBS1", 0);
         gm_Print("          1 - ALL_BOARDS_SVIDEO1", 0);
         gm_Print("          2 - ALL_BOARDS_COMP1", 0);
         gm_Print("          3 - ALL_BOARDS_VGA1", 0);
         gm_Print("          4 - ALL_BOARDS_DVI1", 0);
         gm_Print("          5 - ALL_BOARDS_DVI2", 0);
         gm_Print("          6 - ALL_BOARDS_DIP1", 0);
         gm_Print("          7 - ALL_BOARDS_HDMI", 0);
         gm_Print("          8 - ALL_BOARDS_HDMI2", 0);
         gm_Print("          9 - ALL_BOARDS_HDMI3", 0);
         gm_Print("         10 - ALL_BOARDS_DP1", 0);
         gm_Print("         11 - ALL_BOARDS_DP2", 0);
         gm_Print("   CurrentInput is %d", UserPrefCurrentInputMain);
         return;
   }

	if((BYTE)GetParam(1))
	{
   	UserPrefCurrentInputPip = B_Param;
   	AdjustCurrentInputPip();

		msg_i("Pip Select Input Port %d", B_Param);
	}
	else
	{
		UserPrefCurrentInputMain = B_Param;
   	AdjustCurrentInputMain();

		msg_i("Main Select Input Port %d", B_Param);
	}

   
}

//******************************************************************************
// Change output size
// param 0 - Channel number - "0" = Main  "1" = Pip
// param 1 - gmvw_OutputHActive
// param 2 - gmvw_OutputVActive
//******************************************************************************
//******************************************************************************
//
// FUNCTION       : void ChangeOutputSize(void)
//
// USAGE          :
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        :
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
#ifdef FULL_APPSTEST_CMD
void ChangeOutputSize(void)
{
   WORD B_Channel = (BYTE)GetParam(0);
   WORD W_HActive = (WORD)GetParam(1);
   WORD W_VActive = (WORD)GetParam(2);

   if (B_Channel == 0)
      B_Channel = gmvb_MainChannel;
   else
      B_Channel = gmvb_PipChannel;

   gmvw_OutputHActive[B_Channel] = W_HActive;
   gmvw_OutputVActive[B_Channel] = W_VActive;

   gmvw_OutputHStart[B_Channel] = gmvw_OutputHActive[CH_A] - W_HActive - 160; // 0;
   gmvw_OutputVStart[B_Channel] = gmvw_OutputVActive[CH_A] - W_VActive - 160;	// 0;

   SetDynamicScalerSizeBegin(B_Channel);

   // Setup Display
   if (B_Channel == gmvb_MainChannel)
   {
      SetDynamicScalerStep(gmvb_MainChannel, 4);
   #ifndef NO_PIP_SUPPORT
      SetDynamicScalerStep(gmvb_PipChannel, 0); // turn off pip scaling
   #endif
      gm_AfmSetup(W_OverscanEnabled[gmvb_MainChannel]);
   }
 #ifndef NO_PIP_SUPPORT
   else
   {
      SetDynamicScalerStep(gmvb_MainChannel, 0); // turn off Main scaling
      SetDynamicScalerStep(gmvb_PipChannel, 5);
   }
 #endif
 
   //
   // Setup Dynamic scaling size end
   //
   SetDynamicScalerSizeEnd(B_Channel);
   SetDisplayOff(B_Channel, BACKGROUND_ONLY);
   gm_ScalerDynamicEnable(B_Channel, FALSE);
   gm_DisplaySetSize(B_Channel);
   gm_DisplaySetPosition(B_Channel);
   gm_ScalerSetup(B_Channel);
   gm_MemorySizeAdjust(B_Channel);
   gm_SyncUpdateWait(B_Channel, INPUT_OUTPUT_CLOCK);

   ModeSetupBlender(B_Channel, DISPLAY_DEFER_TMO_NORMAL);


   msg_i("\n Window width = %d", W_HActive);
   msg_i("\n Window height = %d", W_VActive);
}

//******************************************************************************
// Change output position
// param 0 - Channel number - "0" = Main  "1" = Pip
// param 1 - gmvw_OutputHStart
// param 2 - gmvw_OutputVStart
//******************************************************************************
//******************************************************************************
//
// FUNCTION       : void ChangeOutputPosition(void)
//
// USAGE          :
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        :
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void ChangeOutputPosition(void)
{
   WORD W_WindowWidth;
   WORD W_BkgndWidth;
   WORD W_WindowHeight;
   WORD W_BkgndHeight;

   WORD W_Param0 = (BYTE) GetParam(0);
   WORD W_Param1 = (WORD) GetParam(1);
   WORD W_Param2 = (WORD) GetParam(2);

   if (W_Param0 == 0)
      W_Param0 = gmvb_MainChannel;
   else
      W_Param0 = gmvb_PipChannel;

   W_WindowWidth = gmvw_OutputHActive[W_Param0];
   W_BkgndWidth = gm_ReadRegWord(PB_BKGND_WIDTH);
   W_WindowHeight = gmvw_OutputVActive[W_Param0];
   W_BkgndHeight = gm_ReadRegWord(PB_BKGND_HEIGHT);

   if (W_BkgndWidth < (W_WindowWidth + W_Param1))
   {
      msg_i("\n ERROR: Window horizontal start must be equal to or lesser than %d",
            (W_BkgndWidth - W_WindowWidth));
      return;
   }
   else
   {
      gmvw_OutputHStart[W_Param0] = W_Param1;
   }
   if (W_BkgndHeight < (W_WindowHeight + W_Param2))
   {
      msg_i("\n ERROR: Window vertical start must be equal to or lesser than %d",
            (W_BkgndHeight - W_WindowHeight));
      return;
   }
   else
   {
      gmvw_OutputVStart[W_Param0] = W_Param2;
   }

   ModeSetupDisplayChange(W_Param0, 0);

   msg_i("\n Window horizontal start = %d",W_Param1);
   msg_i("\n Window vertical start = %d",W_Param2);
}

//******************************************************************************
// Change Capture Main
// param 0 - UserPrefAspectRatioMain
//******************************************************************************
//******************************************************************************
//
// FUNCTION       : void ChangeCaptureMain(void)
//
// USAGE          :
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        :
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void ChangeCaptureMain(void)
{
   BYTE B_Param = (BYTE)GetParam(0);

   if (B_Param < 8)
   {
      UserPrefAspectRatioMain = B_Param;
      ModeSetupInputChange(gmvb_MainChannel);
      ModeSetupDisplayChange(gmvb_MainChannel, 0);

      msg_i("Capture Main : %d",B_Param);
   }
   else
   {
      msg("Capture Main:");
      msg(" 0 = Auto");
      msg(" 1 = Full Screen");
      msg(" 2 = Letter Box");
      msg(" 3 = Pillar Box");
      msg(" 4 = Pillar Box Expand");
      msg(" 5 = Letter Box Expand");
      msg(" 6 = Water Glass");
      msg(" 7 = Panaoramic");
   }
}

//******************************************************************************
// Change single Pip blending level
// param 0 - blending level
//******************************************************************************
//******************************************************************************
//
// FUNCTION       : void ChangePipBlending(void)
//
// USAGE          :
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        :
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
#ifndef NO_PIP_SUPPORT
void ChangePipBlending(void)
{
   BYTE B_Param0 = (BYTE)GetParam(0);

   gm_BlenderSetLevel(B_Param0);
   gm_SyncUpdate(gmvb_MainChannel, OUTPUT_CLOCK);

   msg_i("Change single Pip blending level - %d",B_Param0);
}
#endif

//******************************************************************************
// Change frame lock mode
// param 0 - "0" = DDDS close loop  "1" = DFL
//******************************************************************************
//******************************************************************************
//
// FUNCTION       : void ChangeFrameLockMode(void)
//
// USAGE          :
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        :
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void ChangeFrameLockMode(void)
{
   BYTE B_Param0 = (BYTE)GetParam(0);

   switch (B_Param0)
   {
      case 0:
      case 1:
         B_DisplayLockMethod = B_Param0;
         ModeSetupFullSetup(CH_A);
         //msg("Frame Lock: DDS Close Loop");
         break;

      default:
         msg("Frame Lock:");
         msg(" 0 = DDDS Close Loop");
         msg(" 1 = DFL");
         msg(" 2 = FreeRun DDDS Open Loop");
         break;
   }
}

//******************************************************************************
// Select and load video coefficient filter table
// param 0 - Horizontal Filter number
// param 1 - vertical Filter number
//******************************************************************************
#if 0
extern BYTE ROM * ROM Ba_VfTablePtr[];   // External vertical filters
#if (FEATURE_SHARPNESS != REV1)
extern BYTE ROM * ROM Ba_HfTablePtr[];   // External vhorizontal filters
#endif
#endif
//******************************************************************************
//
// FUNCTION       : void SelectVideoFilter(void)
//
// USAGE          :
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        :
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void SelectVideoFilter(void)
{
}

//******************************************************************************
// Debug Mode Setup handler
// param 0 - event debug parameter
//******************************************************************************
//******************************************************************************
//
// FUNCTION       : void DebugModeSetup(void)
//
// USAGE          :
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        :
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void DebugModeSetup(void)
{
   BYTE B_Param = (BYTE)GetParam(0);

   if (B_Param < 2)
   {
   	#ifdef DEBUG_MODE_SETUP
      if(B_Param == PARAM_DEBUG_ALL)
         SystemFlags.sys.PrintDebug = TRUE;
      ModeSetupRequest(CH_A, REQ_DEBUG, B_Param, REQUEST_NOW);
      #endif
      msg_i("Debug ModeSetup,%d", B_Param);
   }
   else
   {
      msg("Debug ModeSetup:");
      msg(" 0 = All");
      msg(" 1 = Processing");
   }
}

//******************************************************************************
// DisplayNativeMode - no scaling, cropping, image centered on display
//******************************************************************************
//******************************************************************************
//
// FUNCTION       : void DisplayNativeMode(void)
//
// USAGE          :
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        :
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void DisplayNativeMode(void)
{
   BOOL Bt_IsMainCH_A = (gmvb_MainChannel == CH_A);
   //
   // Setup output width/height based on Input width/height
   //
   gmvw_OutputHActive[gmvb_MainChannel]    = gm_ReadRegWord (Bt_IsMainCH_A?IMP_H_ACT_WIDTH:IPP_H_ACT_WIDTH);

   if ((gm_ReadRegWord(Bt_IsMainCH_A?IMP_CONTROL:IPP_CONTROL)) & (Bt_IsMainCH_A?IMP_INTLC_EN:IPP_INTLC_EN))
      gmvw_OutputVActive[gmvb_MainChannel] = gm_ReadRegWord(Bt_IsMainCH_A?IMP_V_ACT_LENGTH:IPP_V_ACT_LENGTH) * 2;
   else
      gmvw_OutputVActive[gmvb_MainChannel] = gm_ReadRegWord(Bt_IsMainCH_A?IMP_V_ACT_LENGTH:IPP_V_ACT_LENGTH);
   // do not crop input
   B_AspectMethod[gmvb_MainChannel] = AR_FULL_SCREEN;
   if (Bt_IsMainCH_A)
   {
      gm_SetupInputMain();
      gm_AfmSetup(0);
   }
 #ifndef NO_PIP_SUPPORT
   else
      gm_SetupInputPip();
 #endif
   //
   // Setup Output to be centered on Panel
   //
   gmvw_OutputHStart[gmvb_MainChannel] = (PanelWidth/2) -(gmvw_OutputHActive[gmvb_MainChannel]/2);
   gmvw_OutputVStart[gmvb_MainChannel] = (PanelHeight/2) -(gmvw_OutputVActive[gmvb_MainChannel]/2);
   gm_ScalerDynamicEnable(gmvb_MainChannel, FALSE);
   gm_DisplaySetSize(gmvb_MainChannel);
   gm_DisplaySetPosition(gmvb_MainChannel);
   gm_ScalerSetup(gmvb_MainChannel);
   gm_MemorySizeAdjust(gmvb_MainChannel);
   gm_SyncUpdate(gmvb_MainChannel, INPUT_OUTPUT_CLOCK);
}

//******************************************************************************
// Select panel structure
// Param 0 - panel number
//******************************************************************************
//******************************************************************************
//
// FUNCTION       : void SelectPanel(void)
//
// USAGE          :
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        :
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void SelectPanel(void)
{
   char temp[30];
   BYTE i;
   BYTE B_Param = (BYTE) GetParam(0);
   //
   // Force Reinitialization of NVRAM so that new panel boots up with known state
   //
   InitNvram();
   msg ("NVRAM re-initialized to default.");
   //
   // Now process panel update request and save to NVRAM
   //
   UserPrefPanelSelect = B_Param;
   SaveModeIndependentSettings();
   //
   // Search for panel specified
   //
   for (i=1; PanelArray[i] != NULL_PTR; i++)
   {
      if (PanelArray[i] == PanelArray)
      {
         continue;
      }
      //
      // For manually modified panel, search for panel ID based on userpreference
      //
      if (B_Param)
      {
         if (PanelArray[i]->ID == B_Param)
         {
            msg_i("Panel chosen was No.%d", B_Param);
            msg ("Panel ID found");
#if (NON_SCALING_OUTPUT == 1)
            CurrentPanelData =(gmt_PanelData far *) PanelArray[i];
#else
            CurrentPanelData = (gmt_PanelData far *)PanelArray[i];
#endif
            _fstrcpy (temp, PanelArray[i]->C_Name);
            msg (temp);
            msg ("Please make sure connections are correct, and power cycle the board");
            return;
         }
      }
      //
      // Otherwise, use default panel
      //
      else
      {
         if (PanelArray[i]->ID == DEFAULT_PANEL)
         {
#if (NON_SCALING_OUTPUT == 1)
            CurrentPanelData =(gmt_PanelData far *) PanelArray[i];
#else
            CurrentPanelData = (gmt_PanelData far *)PanelArray[i];
#endif
            msg ("Using DEFAULT_PANEL defined in <<panel.h>>");
            msg_i ("Panel ID = %d", CurrentPanelData->ID);
            _fstrcpy (temp, CurrentPanelData->C_Name);
            msg (temp);
            msg ("Please make sure connections are correct, and power cycle the board");
            return;
         }
      }
   }
   msg ("Panel ID given is not supported");
}

//******************************************************************************
// Image scaling and AFM window & registers compensation
// Param(0) - Dummy because the first argument is of the type "BYTE".
// Param(1) - Desired horizontal size
// Param(2) - Desired vertical size
//******************************************************************************
//******************************************************************************
//
// FUNCTION       : void DebugScalingAndAfmWindow(void)
//
// USAGE          :
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        :
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void DebugScalingAndAfmWindow(void)
{
   // Example:    appstest 22 0 720 480
   // It displays in SDTV native resolution

   BYTE dummy = (BYTE)GetParam(0); // Dummy because the first argument is of the type "BYTE".
   WORD H_Size = (WORD)GetParam(1);// Desired horizontal size
   WORD V_Size = (WORD)GetParam(2);// Desired vertical size

   //The following variables are used to adjust the AFM window size based on the selected output size (Avoid AFM mis-detection)
   DWORD AFM_Win_HStart, AFM_Win_HCount, AFM_Win_VStart, AFM_Win_VCount;
   WORD H_Active, V_Active;
   DWORD H_Shrink_Factor, V_Shrink_Factor;
   DWORD Ref_Total_Pixels = 140000; //Reference: 700 x 200 = 14000
   DWORD AFM_Scaling_Factor = 1; //To be used to store the registers scaling factor
   DWORD Temp_hi,Temp_lo; //To read 32-bit registers high and low words
   DWORD Reg_Temp=0;

   AFM_Win_HStart=3; // <--This number is up to the customer. It must be GREATER than 2, otherwise the AFM WON'T WORK!!!
   AFM_Win_VStart=3;
   H_Active = gm_ReadRegWord(IMP_H_ACT_WIDTH);   // Get the current active dimensions
   V_Active = gm_ReadRegWord(IMP_V_ACT_LENGTH);
   H_Shrink_Factor =   (H_Active*1048576)/H_Size; //Compute the horizontal and vertical shrink factors
   V_Shrink_Factor =   (V_Active*262144)/V_Size;

   msg_i ("First Parameter =%d is unused.", dummy);
   //********************************************************************************************************
   //Set up the scaling registers
   //********************************************************************************************************
   gm_WriteRegWord(MHF_SCALE_VALUE_hi, H_Shrink_Factor>>16);  //
   gm_WriteRegWord(MHF_SCALE_VALUE_lo, H_Shrink_Factor);

   gm_WriteRegWord(MVF_SV_hi, V_Shrink_Factor>>16);
   gm_WriteRegWord(MVF_SV_lo, V_Shrink_Factor);

   //********************************************************************************************************
   // There are four scaling cases. Depending upon the case, the scaler and the AFM window has to be adjusted:
   //********************************************************************************************************

   if ((H_Size >= H_Active) && (V_Size >= V_Active)) // HZoom, VZoom
   {
      gm_WriteRegWord(MFRC_PIXELS_OUT, H_Active); //Set up MFRC to scale image
      gm_WriteRegWord(MFRC_LINES_OUT , V_Active);
      gm_WriteRegWord(MAIN_SCALING_FILTER_CTRL , 0x3F); //Set up the scaling control
      // Adjust the AFM window accordingly. This is the correct way of computing the AFM window
      AFM_Win_HCount = H_Active - 2*(AFM_Win_HStart-1);
      AFM_Win_VCount = V_Active - 2*(AFM_Win_VStart-1);
      msg("HZoom, VZoom");
   }

   if ((H_Size < H_Active) && (V_Size < V_Active)) // HShrink, VShrink
   {
      gm_WriteRegWord(MFRC_PIXELS_OUT, H_Size);
      gm_WriteRegWord(MFRC_LINES_OUT , V_Size);
      gm_WriteRegWord(MAIN_SCALING_FILTER_CTRL , 0x35);
      // Adjust the AFM window accordingly. This is the correct way of computing the AFM window
      AFM_Win_HCount = H_Size - 2*(AFM_Win_HStart-1);
      AFM_Win_VCount = V_Size - 2*(AFM_Win_VStart-1);;
      msg("HShrink, VShrink");
   }

   if ((H_Size < H_Active) && (V_Size >= V_Active)) // HShrink, VZoom
   {
      gm_WriteRegWord(MFRC_PIXELS_OUT, H_Size);
      gm_WriteRegWord(MFRC_LINES_OUT , V_Active);
      gm_WriteRegWord(MAIN_SCALING_FILTER_CTRL , 0x37);
      AFM_Win_HCount = H_Size - 2*(AFM_Win_HStart-1);
      AFM_Win_VCount = V_Active - 2*(AFM_Win_VStart-1);;
      msg("HShrink, VZoom");
   }

   if ((H_Size >= H_Active) && (V_Size < V_Active)) // HZoom, VShrink
   {
      gm_WriteRegWord(MFRC_PIXELS_OUT, H_Active );
      gm_WriteRegWord(MFRC_LINES_OUT , V_Size);
      gm_WriteRegWord(MAIN_SCALING_FILTER_CTRL , 0x3D);
      AFM_Win_HCount = H_Active - 2*(AFM_Win_HStart-1);
      AFM_Win_VCount = V_Size - 2*(AFM_Win_VStart-1);;
      msg("HZoom, VShrink");
   }
   //********************************************************************************************************
   // Update the AFM window and compensate registers:
   //********************************************************************************************************
   gm_WriteRegWord(AFM_HSTART, AFM_Win_HStart);
   gm_WriteRegWord(AFM_HCOUNT, AFM_Win_HCount);
   gm_WriteRegWord(AFM_VSTART, AFM_Win_VStart);
   gm_WriteRegWord(AFM_VCOUNT, AFM_Win_VCount);
   // Compute the AFM registers scaling factor
   AFM_Scaling_Factor = ((AFM_Win_HCount * AFM_Win_VCount) << 8) / Ref_Total_Pixels;    // << 8 in order to have 8 bits resolution for fractional part

   // Adjust AFM thresholds (Compensation due to scaling changes)
   Temp_hi = gm_ReadRegWord(0x82);
   Temp_lo = gm_ReadRegWord(0x80);
   Reg_Temp = (Temp_hi << 16)|(Temp_lo); //Get the 32-bit AFM register
   Reg_Temp = (Reg_Temp * AFM_Scaling_Factor) >> 8;  // <------Scale down
   gm_WriteRegWord(0x82, (WORD)(Reg_Temp>>16) ); //AFM_SM_MAX_R_hi
   gm_WriteRegWord(0x80, (WORD)Reg_Temp ); //AFM_SM_MAX_R_hlo

   Temp_hi = gm_ReadRegWord(0x8E);
   Temp_lo = gm_ReadRegWord(0x8C);
   Reg_Temp = (Temp_hi << 16)|(Temp_lo); //Get the 32-bit AFM register
   Reg_Temp = (Reg_Temp * AFM_Scaling_Factor) >> 8;  // <------Scale down
   gm_WriteRegWord(0x8E, (WORD)(Reg_Temp>>16) ); //AFM_MIN_SCENE_CHANGE_hi
   gm_WriteRegWord(0x8C, (WORD)Reg_Temp ); //AFM_MIN_SCENE_CHANGE_lo

   Temp_hi = gm_ReadRegWord(0x92);
   Temp_lo = gm_ReadRegWord(0x90);
   Reg_Temp = (Temp_hi << 16)|(Temp_lo); //Get the 32-bit AFM register
   Reg_Temp = (Reg_Temp * AFM_Scaling_Factor) >> 8;  // <------Scale down
   gm_WriteRegWord(0x92, (WORD)(Reg_Temp>>16) ); //AFM_GFX_MAX_LOW_S_hi
   gm_WriteRegWord(0x90, (WORD)Reg_Temp ); //AFM_GFX_MAX_LOW_S_lo

   Temp_hi = gm_ReadRegWord(0x96);
   Temp_lo = gm_ReadRegWord(0x94);
   Reg_Temp = (Temp_hi << 16)|(Temp_lo); //Get the 32-bit AFM register
   Reg_Temp = (Reg_Temp * AFM_Scaling_Factor) >> 8;  // <------Scale down
   gm_WriteRegWord(0x96, (WORD)(Reg_Temp>>16) ); //AFM_PHASE_0_MAX_S_hi
   gm_WriteRegWord(0x94, (WORD)Reg_Temp ); //AFM_PHASE_0_MAX_S_lo
   msg("HShrink, VShrink");

   //********************************************************************************************************
   // Update output size according to the user's entered dimensions
   //********************************************************************************************************
   gm_WriteRegWord(PB_MAIN_HWIDTH, H_Size);
   gm_WriteRegWord(PB_MAIN_VHEIGHT, V_Size);
   gm_WriteRegWord(HOST_CONTROL, 0xF0);
}

#endif // FULL_APPSTEST_CMD

//******************************************************************************
//******************************************************************************
#ifdef USE_IMP_PIXGRAB_CMD
WORD GrabPixel(gmt_PHY_CH B_Channel, WORD W_Xcord, WORD W_Ycord, gmt_RGB_COLOR B_Color)
{
   WORD W_Retval;
   WORD W_FlaglineOrg;

   if (B_Channel == CH_A)
   {
      W_FlaglineOrg = gm_ReadRegWord(IMP_FLAGLINE);         // save the current flagline value
      gm_WriteRegWord(IMP_PIXGRAB_V, W_Ycord);            // co-ords of the pixel we want
      gm_WriteRegWord(IMP_PIXGRAB_H, W_Xcord);
      gm_WriteRegWord(IMP_FLAGLINE, (W_Ycord + 1));         // wait for the correct line
      gm_SetRegBitsWord(IMP_CONTROL, IMP_PIXGRAB_EN);         // enable pixel grab

      gm_TimerStart(DELAY_10ms_TMR, 3);      // setup a timeout
      gm_SetRegBitsWord(MAIN_INPUT_STATUS, IMP_LINEFLAG);      // clear IP_LINEFLAG

      if (gm_IsInterlaced(B_Channel))               // the return statuses are all messed up here
      {
         // wait for this row done & an even field
         while (((gm_ReadRegWord(MAIN_INPUT_STATUS) & IMP_LINEFLAG) == 0) || (gm_ReadRegWord(MISC_STATUS) & IMP_ODD_STATUS))
         {
            if (gm_TimerCheck( DELAY_10ms_TMR) == TIMER_TMO)
            {
               break;
            }
         }
      }
      else
      {
         // wait for this row done
         while ((gm_ReadRegWord(MAIN_INPUT_STATUS) & IMP_LINEFLAG) == 0)
         {
            if (gm_TimerCheck(DELAY_10ms_TMR) == TIMER_TMO)
            {
               break;
            }
         }
      }

      if (B_Color == gmd_RED_COMPONENT)
         W_Retval = gm_ReadRegWord(IMP_PIXGRAB_RED);
      else if (B_Color == gmd_GREEN_COMPONENT)
         W_Retval = gm_ReadRegWord(IMP_PIXGRAB_GRN);
      else
         W_Retval = gm_ReadRegWord(IMP_PIXGRAB_BLU);

      // Do not disable pixel grab so we can get other color values after this function returns
      // Note: For FLI8668 disabling pixel grab causes the raw ADC data to be writen into the pixel grab result registers.
//      gm_ClearRegBitsWord(IMP_CONTROL, IMP_PIXGRAB_EN);
      gm_WriteRegWord(IMP_FLAGLINE, W_FlaglineOrg);         // restore the original flagline value
   }
   else
   {
      W_FlaglineOrg = gm_ReadRegWord(IPP_FLAGLINE);         // save the current flagline value
      gm_WriteRegWord(IPP_PIXGRAB_V, W_Ycord);            // co-ords of the pixel we want
      gm_WriteRegWord(IPP_PIXGRAB_H, W_Xcord);            // enable pixel grab
      gm_WriteRegWord(IPP_FLAGLINE, (W_Ycord + 1));         // wait for the correct line
      gm_SetRegBitsWord(IPP_CONTROL, IPP_PIXGRAB_EN);         // enable pixel grab

      gm_TimerStart(DELAY_10ms_TMR, 3);      // setup a timeout
      gm_SetRegBitsWord(PIP_INPUT_STATUS, IPP_LINEFLAG);      // clear IP_LINEFLAG

      if (gm_IsInterlaced(B_Channel))
      {
         // wait for this row done & an even field
         while (((gm_ReadRegWord(PIP_INPUT_STATUS) & IPP_LINEFLAG) == 0) || (gm_ReadRegWord(MISC_STATUS) & IPP_ODD_STATUS))
         {
            if (gm_TimerCheck(DELAY_10ms_TMR) == TIMER_TMO)
            {
               break;
            }
         }
      }
      else
      {
         // wait for this row done
         while ((gm_ReadRegWord(PIP_INPUT_STATUS) & IPP_LINEFLAG) == 0)
         {
            if (gm_TimerCheck(DELAY_10ms_TMR) == TIMER_TMO)
            {
               break;
            }
         }
      }

      if (B_Color == gmd_RED_COMPONENT)
         W_Retval = gm_ReadRegWord(IPP_PIXGRAB_RED);
      else if (B_Color == gmd_GREEN_COMPONENT)
         W_Retval = gm_ReadRegWord(IPP_PIXGRAB_GRN);
      else
         W_Retval = gm_ReadRegWord(IPP_PIXGRAB_BLU);

      // Do not disable pixel grab so we can get other color values after this function returns
      // Note: For FLI8668 disabling pixel grab causes the raw ADC data to be writen into the pixel grab result registers.
//      gm_ClearRegBitsWord(IPP_CONTROL, IPP_PIXGRAB_EN);
      gm_WriteRegWord(IPP_FLAGLINE, W_FlaglineOrg);         // restore the original flagline value
   }

   return W_Retval;
}

//******************************************************************************
// Pixel Scan Testing
// Param(0) - Channel
// Param(1) - X Coordinate
// Param(2) - Y Coordinate
// Param(3) - Color (0 = RED, 1 = GRN, 2 = BLU, 3 = ALL)
//******************************************************************************
void Pixel_Scan(void)
{
   gmt_PHY_CH B_Channel;
   WORD Xpos, Ypos, Phase, StoreR, StoreG, StoreB;
   BYTE Count, Color;

   gm_Print("Pixel Scan Start",0);
   B_Channel = (gmt_PHY_CH)GetParam(0);
   Xpos = (WORD)GetParam(1);
   Ypos = (WORD)GetParam(2);
   Color = (BYTE)GetParam(3);

   gm_Print("CH: %d",B_Channel);
   gm_Print("X,Y: %d , %d",Xpos,Ypos);
   gm_Print("Col: %d",Color);

   if (Color == 3)
   {
      BYTE PixCnt = 2;
      gm_Print("  R     G     B",0);
      gm_Print("------------------",0);
 
      while(PixCnt--)
      {
         for (Phase = 0; Phase < 64; Phase++)
         {
            gm_SetAdcPhase(B_Channel, Phase);
            gm_InputWaitVsync(B_Channel, 1);
            StoreR = StoreG = StoreB = 0;

            for (Count = 0; Count < 16; Count++)
            {
               StoreR += GrabPixel(B_Channel, Xpos, Ypos, gmd_RED_COMPONENT);
               if (B_Channel == CH_A)
               {
                  StoreG += gm_ReadRegWord(IMP_PIXGRAB_GRN);
                  StoreB += gm_ReadRegWord(IMP_PIXGRAB_BLU);
               }
               else
               {
                  StoreG += gm_ReadRegWord(IPP_PIXGRAB_GRN);
                  StoreB += gm_ReadRegWord(IPP_PIXGRAB_BLU);
               }
            }
            //gm_Print("*R%d", StoreR >> 4);
            //gm_Print("*G%d", StoreG >> 4);
            //gm_Print("*B%d", StoreB >> 4);
            gm_Print("%d    %d    %d",(StoreR >> 4), (StoreG >> 4), (StoreB >> 4));
         }
         Xpos++;
      }
      gm_Print("------------------",0);
   }
   else
   {
      for (Phase = 0; Phase < 64; Phase++)
      {
         gm_SetAdcPhase(B_Channel, Phase);
         gm_InputWaitVsync(B_Channel, 1);
         gm_Print("%d", GrabPixel(B_Channel, Xpos, Ypos, Color));
      }
   }
}
#endif // USE_IMP_PIXGRAB_CMD

//******************************************************************************
// Zoom
// Param(0) - %            (eg. 10 == 10%, 0 == Restore)
//******************************************************************************
//******************************************************************************
//
// FUNCTION       : void Zoom(void)
//
// USAGE          :
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        :
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
#ifdef FULL_APPSTEST_CMD
extern void Image_Force_OnOff(BOOL OnOff);
void DebugZoom(void)
   {
      WORD W_Dx;
      WORD W_Dy;
      BYTE B_Direction = (BYTE)GetParam(0); // = gm_OsdGetActionByteParameter();
      BOOL Bt_Dirty = FALSE;
      gmt_WindowStruct WinNewDim;
      gmt_WindowStruct WinPrevDim;
      BYTE Test_ZOOM_PERCENT = 10, Test_MAX_STEPS = 6;

      //Test_MAX_STEPS = 6;
      //Test_ZOOM_PERCENT = (WORD)GetParam(0);

      WinPrevDim.W_Hstart = stInput[gmvb_MainChannel].Hstart;
      WinPrevDim.W_Width = stInput[gmvb_MainChannel].Hactive;
      WinPrevDim.W_Vstart = stInput[gmvb_MainChannel].Vstart;
      WinPrevDim.W_Height = stInput[gmvb_MainChannel].Vactive;
 
      W_Dx = (Test_ZOOM_PERCENT * gmvw_InputHActiveOrg[gmvb_MainChannel]) / 200;
      W_Dy = (Test_ZOOM_PERCENT * gmvw_InputVActiveOrg[gmvb_MainChannel]) / 200;

      if(B_Direction != 0)
         B_Direction = UP;
      
      // Zoom In
      if (B_Direction == UP)
      {
         if ((stInput[gmvb_MainChannel].Hactive - (W_Dx * 2)
               > gmvw_InputHActiveOrg[gmvb_MainChannel] - (W_Dx * Test_MAX_STEPS * 2))
               && (stInput[gmvb_MainChannel].Vactive - (W_Dy * 2)
                   > gmvw_InputVActiveOrg[gmvb_MainChannel] - (W_Dy * Test_MAX_STEPS * 2)))
         {
            stInput[gmvb_MainChannel].Hstart += W_Dx;
            stInput[gmvb_MainChannel].Hactive -= (W_Dx * 2);
            stInput[gmvb_MainChannel].Vstart += W_Dy;
            stInput[gmvb_MainChannel].Vactive -= (W_Dy * 2);
            Bt_Dirty = TRUE;
         }
      }
   
      // Zoom Out
      else
      {
         if ((stInput[gmvb_MainChannel].Hactive + (W_Dx * 2) <= gmvw_InputHActiveOrg[gmvb_MainChannel])
               && (stInput[gmvb_MainChannel].Vactive + (W_Dy * 2) <= gmvw_InputVActiveOrg[gmvb_MainChannel]))
         {
            stInput[gmvb_MainChannel].Hstart -= W_Dx;
            stInput[gmvb_MainChannel].Hactive += (W_Dx * 2);
            stInput[gmvb_MainChannel].Vstart -= W_Dy;
            stInput[gmvb_MainChannel].Vactive += (W_Dy * 2);
            Bt_Dirty = TRUE;
         }
      }
   
      // Reconfig Chip
      if (Bt_Dirty == TRUE)
      {   
         gm_ClearRegBitsWord(PB_CTRL, PB_MAIN_ENABLE);
   	   gm_SetRegBitsWord(PB_CTRL, PB_FORCE_MAIN_BKGND);
   	   gm_ForceUpdate(CH_A , INPUT_OUTPUT_CLOCK);
      
         //save the desired input dimension
         WinNewDim.W_Hstart = stInput[gmvb_MainChannel].Hstart;
         WinNewDim.W_Width = stInput[gmvb_MainChannel].Hactive;
         WinNewDim.W_Vstart = stInput[gmvb_MainChannel].Vstart;
         WinNewDim.W_Height = stInput[gmvb_MainChannel].Vactive;
   
         // 20090618 #1, For DP over Scan.
	   #if ((FEATURE_HDMI == ENABLE) || (FEATURE_DISPLAY_PORT == ENABLE))
         if ((gm_IsInputHdmiDvi(gmvb_MainChannel)) ||(gm_IsInputDP(gmvb_MainChannel)))
         {
				stInput[gmvb_MainChannel].ModeFlags |= gmd_MODE_CAPTURE_CROP;
            
				if ((WinNewDim.W_Width ==  gmvw_InputHActiveOrg[gmvb_MainChannel])&&(WinNewDim.W_Height== gmvw_InputVActiveOrg[gmvb_MainChannel]))
				{
               stInput[gmvb_MainChannel].Hstart = gmvw_InputHStartOrg[gmvb_MainChannel];
               stInput[gmvb_MainChannel].Vstart = gmvw_InputVStartOrg[gmvb_MainChannel];
               stInput[gmvb_MainChannel].ModeFlags &= ~gmd_MODE_CAPTURE_CROP;                  
            }
         }
	   #endif
   
      {  
         DWORD V_StartAddr = gm_ReadRegDWord(MFRC_WBUF_START_lo);

         gm_Print("Main mem allocate V_StartAddr= 0x%X", V_StartAddr);
         //following line is already done by enabling VIDEO_DYN_SCAL
         //gm_VideoProcessingDisable(VIDEO_DCDI_MAIN);
         gm_MemoryAdjust_VFlip(gmvb_MainChannel, V_StartAddr, WinNewDim.W_Width, WinNewDim.W_Height);
      }
      
         Freeze(gmvb_MainChannel, FALSE);
         UserPrefFreeze = 0;                  // System is unfrozen so WB must be notified.

         //Disable ACC3 before zooming to avoid picture flashing
		#if (FEATURE_ACC3 == ENABLE)
         {
            BOOL Bt_Acc3En;
            Bt_Acc3En = gm_ACC3_IsProcessingEnabled(TRUE);
            gm_ACC3StopISR();
		#endif
            RelocateInputCaptureWindow(gmvb_MainChannel, &WinPrevDim, &WinNewDim);

            gm_SetRegBitsWord(PB_CTRL, PB_MAIN_ENABLE);
   	      gm_ClearRegBitsWord(PB_CTRL, PB_FORCE_MAIN_BKGND);
   	      gm_ForceUpdate(CH_A , INPUT_OUTPUT_CLOCK);
		#if (FEATURE_ACC3 == ENABLE)
            if (Bt_Acc3En)
               gm_ACC3StartISR();
         }
		#endif
      }
   }


//******************************************************************************
// Loads the specified gamma table
// Param(0) - Gamma Table (0 = OFF, 1 = 2.2, 2 = 2.4, 3 = 2.6, 4 = 2.8)
// Param(1) - Load Component (0 = R, 1 = G, 2 = B, 3 = RGB)
//******************************************************************************
//******************************************************************************
//
// FUNCTION       : void DebugAdjustGamma(void)
//
// USAGE          :
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        :
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void DebugAdjustGamma(void)
{
#ifdef ADJUSTER_GAMMATABLE_DECLARED
   UserPrefColorSpace = (BYTE)GetParam(0);
   UserPrefColorTemp = (BYTE)GetParam(1);
   if (GetGammaTable())
      gm_Printf("Using Color Space number = %d with color temp. = %d", UserPrefColorSpace,UserPrefColorTemp);
   else
      gm_Printf("Gamma table not found. Vdeo LUT disabled",0);
   AdjustGammaCorrection();
#endif
}

//******************************************************************************
//
// FUNCTION       : void ReadOSDTileInfo (void)
//
// USAGE          :
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        :
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void ReadOSDTileInfo (void)
{
   ST_OSD_TILE St_TileSet[16];
   BYTE B_NumTiles = ((gm_ReadRegWord(OSD_CONTROL) & MAX_NUMBER_TILES) >> 3) + 1;
   BYTE B_ActiveTileset = ((gm_ReadRegWord(OSD_CONTROL) & ACTIVE_BUFF_HIGH) >> 1);
   BYTE i;

   memset(St_TileSet, 0, sizeof(St_TileSet));

   gm_Print ("*************  TileSet %d   ***************", B_ActiveTileset);

   for (i=0; i<B_NumTiles; i++)
   {
      gm_OsdGetTile(i, &St_TileSet[i]);
      gm_Printf ("Tile %d has VSTART = %d", i, St_TileSet[i].W_Ypos);
   }
}

//******************************************************************************
//
// FUNCTION       : void SetOSDTilePosition (void)
//
// USAGE          :
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        :
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void SetOSDTilePosition (void)
{
   WORD W_TileID = (WORD)GetParam(0);
   WORD W_HStart = (WORD)GetParam(1);
   WORD W_VStart = (WORD)GetParam(2);
   gm_OsdSetTilePosition(W_TileID, W_HStart, W_VStart);
   ReverseTileSet();
   gm_OsdSetTilePosition(W_TileID, W_HStart, W_VStart);
   gm_Printf("Changing Tile ID %d, to HSTART = %d, and VSTART = %d", W_TileID, W_HStart, W_VStart);
}

//******************************************************************************
//
// FUNCTION       : void SetupPatGen (void)
//
// USAGE          :
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        :
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void SetupPatGen (gmt_PHY_CH B_Channel, BYTE B_PatNum)
{
   if (B_Channel == gmvb_MainChannel)
   {
      gm_ClearRegBitsWord (IMP_CONTROL, IMP_INTLC_EN); //Output of Patgen is progressive
      if (B_PatNum == 0)
      {
         gm_WriteRegWord (IMP_PATGEN_CONTROL, 0);
         gm_WriteRegWord (IMP_PATGEN_BLUE, 0);
         gm_WriteRegWord (IMP_PATGEN_GRN, 0);
         gm_WriteRegWord (IMP_PATGEN_RED, 0);
      }
      else
      {
         gm_WriteRegWord (IMP_PATGEN_CONTROL, B_PatNum | IMP_TPG_EN);
         gm_WriteRegWord (IMP_PATGEN_BLUE, 0x88);
         gm_WriteRegWord (IMP_PATGEN_GRN, 0xF);
         gm_WriteRegWord (IMP_PATGEN_RED, 0x88);
         gm_ClearRegBitsWord (IMP_CONTROL, IMP_INTLC_EN); //Output of Patgen is progressive
      }
   }
   else
   {
      gm_ClearRegBitsWord (IPP_CONTROL, IPP_INTLC_EN); //Output of Patgen is progressive
      if (B_PatNum == 0)
      {
         gm_WriteRegWord (IPP_PATGEN_CONTROL, 0);
         gm_WriteRegWord (IPP_PATGEN_BLUE, 0);
         gm_WriteRegWord (IPP_PATGEN_GRN, 0);
         gm_WriteRegWord (IPP_PATGEN_RED, 0);
      }
      else
      {
         gm_WriteRegWord (IPP_PATGEN_CONTROL, B_PatNum | IPP_TPG_EN);
         gm_WriteRegWord (IPP_PATGEN_BLUE, 0x88);
         gm_WriteRegWord (IPP_PATGEN_GRN, 0xF);
         gm_WriteRegWord (IPP_PATGEN_RED, 0x88);
         gm_ClearRegBitsWord (IPP_CONTROL, IPP_INTLC_EN); // Output of Patgen is progressive
      }
   }
}

//******************************************************************************
//
// FUNCTION       : static void PatGenOverride (void)
//
// USAGE          :
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        :
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
// phyang 07/26/06: Fixed the ACLK not running issue.
static void PatGenOverride (void)
{

#if 0 // Louis ... To Do --> Add new pattern gen. functions

   gm_SetRegBitsWord(SDDS1_CONTROL, SDDS1_FORCE_OPLOOP);
//   gm_SetRegBitsWord(SDDS2_CONTROL, SDDS2_FORCE_OPLOOP);
   gm_SetRegBitsWord(DDDS_CONTROL, DDDS_FORCE_OPLOOP);
   gm_SetRegBitsWord(SOFT_PD, SDDS1_EN);
//   gm_SetRegBitsWord(SOFT_PD, SDDS2_EN);
   gm_SetRegBitsWord(SOFT_PD, DDDS_EN);
   gm_SetRegBitsWord(SOFT_PD, FDDS_EN);
   gm_SetRegBitsWord(SOFT_PD, ODDS_EN);

   gm_WriteRegWord(CLOCK_CONFIG2, 0x222A);

   gm_WriteRegWord (IMP_SOURCE, 0x6);            // Selects timing from STG
   gm_WriteRegWord (SDDS_INIT, 0x1);            // Reinit Sdds for STG of PatGen

   /* workaround for stuck ACLK ...
   ** - Check we have VSYNC from the AFE
   ** - if not, enable the manual control for manual clock phase selection
   ** - Select the clock phase from 180 degree out of phase
   ** - check for VSYNC again
   ** - if no VSYNC, problem on HW
   */
   gm_SetRegBitsWord (MAIN_INPUT_STATUS, IMP_VACTIVE);
   gm_Delay10ms(5);
   if ((gm_ReadRegWord(MAIN_INPUT_STATUS) & IMP_VACTIVE)==0)
   {
      /* we don't found VSync, ACLK is potentially not running.
      ** let's manually flip the phase control circuit
      */
      if (gm_ReadRegWord (RPLL_TEST_CONTROL) & HS_IPCLK_SCLK_OPT)
         gm_ClearRegBitsWord (RPLL_TEST_CONTROL, HS_IPCLK_SCLK_OPT);
      else
         gm_SetRegBitsWord (RPLL_TEST_CONTROL, HS_IPCLK_SCLK_OPT);


      gm_SetRegBitsWord(SDDS1_TEST_CNTRL1, SDDS1_HS_IPCLK_MANUAL);
      gm_SetRegBitsWord(SDDS1_TEST_CNTRL1, SDDS1_HS_IPCLK_MANUAL);

      gm_Delay10ms(5);
      if (gm_ReadRegWord (MAIN_INPUT_STATUS) & IMP_VACTIVE)
         gm_Print("Info: ACLK recovered", 0);
      else
         gm_Print("Error: ACLK not running", 0);
   }
//#endif
#endif
}

//******************************************************************************
//
// FUNCTION       : void SetupInputOutputForTPG (void)
//
// USAGE          :
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        :
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
// phyang 07/26/06: Modified to work for any panel.
#define POW2(a) (1L << (a))
void SetupInputOutputForTPG (void)
{

   BYTE B_Channel;
PanelSetAsInputTiming(); // must before gm_MemorySetup 
   //
   // Trick FW into thinking valid 480i input is available (also disable mode detection)
   // This is required for setting up the internal test pattern generator AND using the
   // internal source timing generator (STG)
   //
   for (B_Channel = 0; B_Channel <2; B_Channel++)
   {
      stInput[B_Channel].ModeFlags  = 0;             // Clear flags

      SETBIT(stInput[B_Channel].ModeFlags,
             gmd_MODE_VALID | gmd_MODE_GRAPHICS);
      //
      // Program Input
      //
      // 20090924 #1, Test internal pattern wo input signal for Japan customer.
      gmvw_InputSyncFlags[B_Channel] = gmd_SYNC_ODD_INV;
      stInput[B_Channel].Htotal = 0x2F9; //858;
      stInput[B_Channel].Hactive = 640; //720;
      stInput[B_Channel].Vactive = 480; //240;
      stInput[B_Channel].Hstart = 0x2C; //4;
      stInput[B_Channel].Vstart = 0x24; //20;
      SaveOriginalValue(B_Channel);
   }
   //
   // Program Output
   //
   gmvw_OutputHActive[gmvb_MainChannel] = PanelWidth;
   gmvw_OutputVActive[gmvb_MainChannel] = PanelHeight;
   gmvw_OutputHStart[gmvb_MainChannel] = 0;
   gmvw_OutputVStart[gmvb_MainChannel] = 0;

   // 20090924 #1, Test internal pattern wo input signal for Japan customer.
   gm_WriteRegWord(SDDS1_HTOTAL, 0x2F9); //0x35A);
//      gm_WriteRegWord(SDDS2_HTOTAL, 0x35A);
   gm_WriteRegWord(AIP_STG_VTOTAL, 0x1F2); //0x106);    // Based on 480i input (525/2 = 262.5)
//      gm_WriteRegWord(AIP2_STG_VTOTAL, 0x106);    // Based on 480i input (525/2 = 262.5)
   //gm_WriteRegWord(SDDS_INITIAL_FREQ_hi, 0x0E);
   //gm_WriteRegWord(SDDS_INITIAL_FREQ_lo, 0xB4D5);
   gm_WriteRegDWord(SDDS1_INI_FREQ, 0xD2471); //0xEB4D5);
//      gm_WriteRegWord(SDDS2_INITIAL_FREQ_hi, 0x0E);
//      gm_WriteRegWord(SDDS2_INITIAL_FREQ_lo, 0xB4D5);
   gm_WriteRegWord(SDDS1_INIT, 1);
//      gm_WriteRegWord(SDDS2_INIT, 1);
   gm_SetRegBitsWord(SDDS1_CONTROL, 0x90);
//      gm_SetRegBitsWord(SDDS2_CONTROL, 0x90);


   /* IPG is RGB, so we need to enable backend for
   ** ... 444 process
   ** ... then convert RGB to YUV_444
   ** ... then convert YUV_444 to YUV_422
   */
   gm_WriteRegDWord(IMP_CONTROL, IMP_RUN_EN | IMP_444_EN);

   // 20090924 #1, Test internal pattern wo input signal for Japan customer.
   //gm_SetRegBitsWord(IMP_CONTROL, IMP_YUV_EN);
   //gm_ClearRegBitsWord(ODP_CTRL, MAIN_RGB_EN);
   gm_SetRegBitsWord(HOST_CONTROL, IMP_FORCE_UPDATE);


   //gm_SetRegBitsWord(IPP_CONTROL, IPP_444_EN);

   //gm_SetRegBitsWord(IPP_CONTROL, IPP_YUV_EN);
   //gm_ClearRegBitsWord(ODP_CTRL, PIP_RGB_EN);
   //gm_SetRegBitsWord(HOST_CONTROL, IPP_FORCE_UPDATE);

   // phyang 07/26/06: Make it generic. Not just for WXGA panel.
   {
      DWORD D_Frequency;
      //gm_WriteRegWord(DDDS_INITIAL_FREQ_hi, 0x47);
      //gm_WriteRegWord(DDDS_INITIAL_FREQ_lo, 0x7911);
      D_Frequency = (DWORD)gmv_OUTPUT_HTOTAL * (DWORD)gmv_OUTPUT_VTOTAL * (DWORD)gmvb_OutputFrameRate;
      gm_DdsSetInitFreq(CLK_DDDS, D_Frequency);
   }

   gm_WriteRegWord(DDDS1_INIT, 1);
   gm_WriteRegWord(DDDS1_CONTROL, 0x90);
   gm_WriteRegWord(DIP_CONFIG_CTRL, 0x0);
   gm_WriteRegWord(IMP_SOURCE, 0x6);
   //gm_WriteRegWord(IPP_SOURCE, 0x6);

   // 20090924 #1, Test internal pattern wo input signal for Japan customer.
#if 0
   /* 480i input, initiate Main Capture windows size */
   gm_WriteRegWord(IMP_H_ACT_START, 0x2C); //0x1C);
   gm_WriteRegWord(IMP_H_ACT_WIDTH, 0x280); //0x2D0);     // 720 pixels
   gm_WriteRegWord(IMP_V_ACT_LENGTH, 0x1E0); //0xF0);     // 240 lines
   gm_WriteRegWord(IMP_V_ACT_START_ODD, 0x24); //0x1D);
   gm_WriteRegWord(IMP_V_ACT_START_EVEN, 0x24); //0x1D);
   gm_WriteRegWord(IMP_VLOCK_EVENT, 0x4);
   gm_WriteRegWord(IMP_FRAME_RESET_LINE, 0x1B);
   gm_SetRegBitsWord(IMP_CONTROL, IMP_RUN_EN);
#endif
#if 1
   gm_WriteRegWord(IPP_H_ACT_START, 0x1C);
   gm_WriteRegWord(IPP_H_ACT_WIDTH, 0x2D0);     // 720 pixels
   gm_WriteRegWord(IPP_V_ACT_LENGTH, 0xF0);     // 240 lines
   gm_WriteRegWord(IPP_V_ACT_START_ODD, 0x1D);
   gm_WriteRegWord(IPP_V_ACT_START_EVEN, 0x1D);
   gm_WriteRegWord(IPP_VLOCK_EVENT, 0x4);
   gm_WriteRegWord(IPP_FRAME_RESET_LINE, 0x1B);
   gm_ClearRegBitsWord(IPP_CONTROL, IPP_RUN_EN);
#endif
   // 20090924 #1, Test internal pattern wo input signal for Japan customer.
   gm_WriteRegWord(MC_MEMORY_CONFIG, 0x0);

   gm_WriteRegWord(MFRC_CTRL, 0x20);
   /*
   ** Just in case we want to test TNR, CCS, MADi enabling,
   ** we need to enable the main buffer for 5 buffers
   gm_WriteRegWord(MFRC_CTRL, 0x40);
   */

   // 20090924 #1, Test internal pattern wo input signal for Japan customer.
   // Main buffer for 480i input
   gm_WriteRegWord(MFRC_BUF_WIDTH, 0x26); //0x1D); // (720*20)/512
   gm_WriteRegWord(MFRC_WR_WIDTH, 0x26); //0x1D);
   gm_WriteRegWord(MFRC_RD_WIDTH, 0x26); //0x1D);
   gm_WriteRegWord(MFRC_BUF_STRIDE_hi, 0x0);
   gm_WriteRegWord(MFRC_BUF_STRIDE_lo, 0x4740); //0x1B30);
   {
      DWORD thisAddr;
      thisAddr = ((DWORD)gm_ReadRegWord(MFRC_RBUF_START_hi)) << 16;
      thisAddr |= gm_ReadRegWord(MFRC_RBUF_START_lo);
      thisAddr += (0x1B30 - 0x1D);
      gm_WriteRegWord(MFRC_RBUF_END_hi, (WORD)(thisAddr>>16));
      gm_WriteRegWord(MFRC_WBUF_END_hi, (WORD)(thisAddr>>16));
      gm_WriteRegWord(MFRC_RBUF_END_lo, (WORD)thisAddr);
      gm_WriteRegWord(MFRC_WBUF_END_lo, (WORD)thisAddr);
   }
   gm_WriteRegWord(MFRC_PIXELS_OUT, 0x280); //0x2D0);
   gm_WriteRegWord(MFRC_LINES_OUT, 0x1E0); //0xF0);


   gm_WriteRegWord(PB_MAIN_HSTART, 0x0);
   gm_WriteRegWord(PB_MAIN_HWIDTH, PanelWidth);   // 1280 pixel for WXGA
   gm_WriteRegWord(PB_MAIN_VSTART, 0x0);
   gm_WriteRegWord(PB_MAIN_VHEIGHT, PanelHeight);
   gm_WriteRegWord(PB_BKGND, 0xF9E7);
   gm_WriteRegWord(TIMING_CONFIG, 0x1);
   gm_WriteRegWord(PB_CTRL, 0x1);
   gm_WriteRegWord(PB_BKGND_WIDTH, PanelWidth);
   gm_WriteRegWord(PB_BKGND_HEIGHT, PanelHeight);


   // for 480i input to WXGA output
   /* To convert 480i to WXGA panel, enable the Horizontal & Vertical Zoom
   ** 720 -> 1280: scale value = 720/1280 * 2^20
   ** ... and we enable the HW filter
   ** 240 -> 768: scale value = 240/768 * 2^18
   ** ... and enable the HW filer, two fields process bit
   */
   {
      DWORD D_ScaleFactor;

      // Write the horizontal scale value
      gm_WriteRegWord(MAIN_SCALING_FILTER_CTRL, 0x3F); //0x0F);

      // phyang 07/26/06: Make it generic.  Not just for WXGA panel.
      //gm_WriteRegWord(MHF_SCALE_VALUE_hi, 0x09);
      //gm_WriteRegWord(MHF_SCALE_VALUE_lo, 0x0000);
      // 20090924 #1, Test internal pattern wo input signal for Japan customer.
      //D_ScaleFactor = ((DWORD)720 * POW2(20)) / PanelWidth;
		D_ScaleFactor = ((DWORD)640 * POW2(20)) / PanelWidth;

      gm_WriteRegWord(MHF_SCALE_VALUE_lo, (WORD)D_ScaleFactor);
      gm_WriteRegWord(MHF_SCALE_VALUE_hi, (WORD)(D_ScaleFactor  >> 16));

      // phyang 07/26/06: Make it generic.  Not just for WXGA panel.
      //gm_WriteRegWord(MVF_SV_hi, 1);
      //gm_WriteRegWord(MVF_SV_lo, 0x4000);
      // 20090924 #1, Test internal pattern wo input signal for Japan customer.
      //D_ScaleFactor = ((DWORD)240 * POW2(18)) / PanelHeight;
		D_ScaleFactor = ((DWORD)480 * POW2(18)) / PanelHeight;

      gm_WriteRegWord(MVF_SV_lo, ((WORD)D_ScaleFactor));
      gm_WriteRegWord(MVF_SV_hi, ((WORD)(D_ScaleFactor >> 16)));

      gm_ClearRegBitsWord(MVF_CTRL, MVF_SPATIAL_EN); // enable 2 fields process
      gm_SetRegBitsWord(MVF_CTRL, MVF_INT_COEF_EN);   // use internal coeff.
      gm_SetRegBitsWord(MHF_CTRL, MHF_INT_COEF_EN);   // use internal coeff.
   }

   gm_WriteRegWord(SYNC_CONTROL, 0x2);

   gm_WriteRegWord(HOST_CONTROL, 0xF0);

   gm_SetRegBitsWord(SDDS1_CONTROL, SDDS1_FORCE_OPLOOP);
//      gm_SetRegBitsWord(SDDS2_CONTROL, SDDS1_FORCE_OPLOOP);
   gm_SetRegBitsWord(DDDS1_CONTROL, DDDS1_FORCE_OPLOOP);
   //gm_SetRegBitsWord(SOFT_PD, SDDS1_EN);
//      gm_SetRegBitsWord(SOFT_PD, SDDS2_EN);
   //gm_SetRegBitsWord(SOFT_PD, DDDS_EN);
   //gm_SetRegBitsWord(SOFT_PD, FDDS_EN);
   //gm_SetRegBitsWord(SOFT_PD, ODDS_EN);
   gm_WriteRegWord(SDDS1_INIT, 0x1);
//      gm_WriteRegWord(SDDS2_INIT, 0x1);

   gm_WriteRegWord(DISPLAY_CONTROL, 0x7);
}

#if (FEATURE_OVERDRIVE == ENABLE)
//******************************************************************************
//
// FUNCTION       : static void OverDriveTransition(void)
//
// USAGE          :
//
// INPUT          : None
//
// OUTPUT         :
//
// GLOBALS        :
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
static void OverDriveTransition(void)
{
   ST_OSD_TILE St_Tile;
   BYTE Ba_Colour[4];
   BYTE B_GpioFlag;
   BYTE B_SpecialAction;

   Ba_Colour[0] = (BYTE)GetParam(0);      //red
   Ba_Colour[1] = 0x00;               //reserved
   Ba_Colour[2] = (BYTE)GetParam(2);      //blue
   Ba_Colour[3] = (BYTE)GetParam(1);      //green
   B_GpioFlag = (BYTE)GetParam(3);
   B_SpecialAction = (BYTE)GetParam(4);

   gm_Printf("OD Transistion:  Red = %d, Green = %d, Blue = %d", Ba_Colour[0], Ba_Colour[1], Ba_Colour[2]);

   if (!B_SpecialAction)
   {
      gmvb_DebugOnly = TRUE;
   }
   else
   {
      gmvb_DebugOnly = FALSE;
   }

   if (B_GpioFlag)
   {
      gm_Delay10ms(5);
   }

   // Initialize the fields in St_Tile from the TILE_CTRL structure
   St_Tile.B_Attr = 5;
   St_Tile.W_Xpos = 0;
   St_Tile.W_Ypos = 0;
   St_Tile.W_Width = PanelWidth;
   St_Tile.W_Height = PanelHeight;
   St_Tile.W_Color = 0;

   gm_OsdSetTile(0, (ST_OSD_TILE ROM *)(&St_Tile), 1);
   ReverseTileSet();
   gm_OsdSetTile(0, (ST_OSD_TILE ROM *)(&St_Tile), 0);
   gm_OsdLoadClutByOcmPolling(0, 1, (WORD far *)Ba_Colour);
   gm_OsdShow();
}
#endif

#define CHIP_BASED_TEST 1

//
// appstest 19 5 - to print out MVF LUT
// appstest 19 6 - to print out MHF LUT
// appstest 19 7 - to print out PVF LUT
// appstest 19 8 - to print out MVF LUT
//
extern BYTE B_SegBit2Shift;
typedef struct
{
   WORD ctrlAddr; //address of crl host register TABLE_ACCESS_BRIDGE_CTRL1 or TABLE_ACCESS_BRIDGE_CTRL2
   WORD ctrlMask; //bit mask of apropriate table inside ctrl register
   WORD ctrlMode[gmd_MAX_NUMBER_TABLE_TRANSFER_MODES];
   WORD ctrlAccs[gmd_MAX_NUMBER_TABLE_ACCESS_MODES];
   WORD status; //apropriate bit in status register
   WORD request; //mask in DMA_REQUEST_CTRL register
   DWORD memAddr; //address in memory space reserved for that table
   WORD tabSize; //size of table
} TTableLoadParamsA;

TTableLoadParamsA const far TableLoadingParamsA[gmd_TAB_SIZE] =
{
   //     ctrlAddr                     ctrlMask      dma poll   stall    safe hi-prior  status request mAdLo  tabSize
   { TABLE_ACCESS_BRIDGE_CTRL1, BIT0 |BIT1 |BIT2,  { 0, BIT0,  BIT1  }, {0, BIT2  }, BIT0, 0x0040, 0x18000L, 0x1800}, //VLUT
   { TABLE_ACCESS_BRIDGE_CTRL1, BIT3 |BIT4 |BIT5,  { 0, BIT3,  BIT4  }, {0, BIT5  }, BIT1, 0x00A0, 0x17000L, 0x1000}, //ACC
   { TABLE_ACCESS_BRIDGE_CTRL1, BIT9 |BIT10|BIT11, { 0, BIT9,  BIT10 }, {0, BIT11 }, BIT3, 0x0050, 0x0F000L, 0x0480}, //OSD
   { TABLE_ACCESS_BRIDGE_CTRL1, BIT12|BIT13|BIT14, { 0, BIT12, BIT13 }, {0, BIT14 }, BIT4, 0x00B0, 0x16400L, 0x0400}, //ACM
   { TABLE_ACCESS_BRIDGE_CTRL2, BIT0 |BIT1 |BIT2,  { 0, BIT0,  BIT1  }, {0, BIT2  }, BIT5, 0x0060, 0x12000L, 0x1C20}, //MVF
   { TABLE_ACCESS_BRIDGE_CTRL2, BIT3 |BIT4 |BIT5,  { 0, BIT3,  BIT4  }, {0, BIT5  }, BIT6, 0x0080, 0x14000L, 0x0E40}, //MHF
   { TABLE_ACCESS_BRIDGE_CTRL2, BIT6 |BIT7 |BIT8,  { 0, BIT6,  BIT7  }, {0, BIT8  }, BIT7, 0x0070, 0x16000L, 0x0388}, //PVF
   { TABLE_ACCESS_BRIDGE_CTRL2, BIT9 |BIT10|BIT11, { 0, BIT9,  BIT10 }, {0, BIT11 }, BIT8, 0x0090, 0x15000L, 0x0E40}, //PHF
};

//******************************************************************************
//
// FUNCTION       : static void TestVideoProcessing(void)
//
// USAGE          :
//
// INPUT          : None
//
// OUTPUT         :
//
// GLOBALS        :
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
static void TestVideoProcessing(void)
{
   BYTE B_MADI = (BYTE)GetParam(0);
   BYTE B_TNR = (BYTE)GetParam(1);
   BYTE B_CCS = (BYTE)GetParam(2);
   BYTE B_Noise_Monitor = (BYTE)GetParam(3);
   BYTE B_RS_Monitor = (BYTE)GetParam(4);
   BYTE B_Region_Monitor = (BYTE)GetParam(5);
#if (FEATURE_VIDEO_SHARPNOISE == REV2)
   BYTE B_SharpNoise =  (BYTE)GetParam(6);
#endif


   //
   // to read LUT tables and print all Video Processing tables
   //

   BYTE B_LUT = B_MADI;

   if ( B_LUT  == 5 ||
         B_LUT  == 6 ||
         B_LUT  == 7 ||
         B_LUT  == 8 )
   {
      gmt_TAB_TRANSFER_MODE tmode;
      gmt_TAB_ACCESS_MODE taccess;
      gmt_TAB_TABLE tab;
      BYTE hole;
      WORD tabsize;
      WORD W_tempReg = gm_ReadRegWord(TABLE_ACCESS_BRIDGE_CTRL2);

      switch ( B_LUT )
      {
         case 5:
            {
               tab = gmd_TAB_MVF;
               break;
            }
         case 6:
            {
               tab = gmd_TAB_MHF;
               break;
            }
         case 7:
            {
               tab = gmd_TAB_PVF;
               break;
            }
         case 8:
            {
               tab = gmd_TAB_PHF;
               break;
            }
      }


      for (hole = 0; hole < 4; hole ++) //for memory with holes
      {
         TTableLoadParamsA params = TableLoadingParamsA[tab];
         // B_SegBit2Shift == 4 20bit, ==8 24 bit
         WORD far *pt1 = (WORD far *)(params.memAddr << (16 - B_SegBit2Shift)) ;

         //gm_ClearAndSetBitsWord(params.ctrlAddr, params.ctrlMask,
         //   (params.ctrlMode[gmd_OCM_POLLING_MODE]) | (params.ctrlAccs[gmd_HIGH_PRIORITY_MODE]));

         // Configure the Table Access Bridge
         gm_WriteRegWord(TABLE_ACCESS_BRIDGE_CTRL2 , 0x0DB6);   //  Stalling Mode, Immediate Access for all Filter LUTs

         gm_Print("////////////////",0);
         switch ( tab )
         {
            case gmd_TAB_MVF:
               {
                  gm_Print(" -+= MVF coefficients =+-",0);
                  tabsize = 0x420/sizeof(WORD);
                  pt1 = pt1 + hole*0x800/sizeof(WORD);
                  break;
               }
            case gmd_TAB_MHF:
               {
                  gm_Print(" -+= MHF coefficients =+-",0);
                  tabsize = 0x240/sizeof(WORD);
                  pt1 = pt1 + hole*0x400/sizeof(WORD);
                  break;
               }
            case gmd_TAB_PVF:
               {
                  gm_Print(" -+= PVF coefficients =+-",0);
                  tabsize = 0x88/sizeof(WORD);
                  pt1 = pt1 + hole*0x100/sizeof(WORD);
                  break;
               }
            case gmd_TAB_PHF:
               {
                  gm_Print(" -+= PHF coefficients =+-",0);
                  tabsize = 0x240/sizeof(WORD);
                  pt1 = pt1 + hole*0x400/sizeof(WORD);
                  break;
               }
         }
         switch ( hole )
         {
            case 0:
               gm_Print(" = UV =",0);
               break;
            case 1:
               gm_Print(" = Y =",0);
               break;
            case 2:
               gm_Print(" = Ypk =",0);
               break;
            case 3:
               gm_Print(" = UVpk =",0);
               break;
         }

         for (; tabsize; pt1++, tabsize--)
         {
            gm_Print("0x%x", *pt1);
            gm_Delay1ms(1);
         }
         gm_Print("////////////////",0);
         gm_Delay100ms( 1);
      }
      gm_WriteRegWord(TABLE_ACCESS_BRIDGE_CTRL2 , W_tempReg );
      return;
   }

   //
   // setting Video Processing modes
   //

   if ((B_TNR != TNR_OFF) &&
         (B_TNR != TNR_LOW) &&
         (B_TNR != TNR_MEDIUM) &&
         (B_TNR != TNR_HIGH) &&
         (B_TNR != TNR_ADAPTIVE))
   {
      gm_Print("Parameter TNR mode is out of range.",0);
      return;
   }

   if ((B_MADI != MADI_OFF) &&
         (B_MADI != MADI_STANDARD) &&
         (B_MADI != MADI_ADAPTIVE))
   {
      gm_Print("Parameter MADI mode is out of range.",0);
      return;
   }

   if ((B_CCS != CCS_OFF) &&
         (B_CCS != CCS_STANDARD) &&
         (B_CCS != CCS_ADAPTIVE))
   {
      gm_Print("Parameter CCS mode is out of range.",0);
      return;
   }
#if (FEATURE_VIDEO_SHARPNOISE == REV2)
   if ((B_SharpNoise != SHARPNOISE_OFF) &&
         (B_SharpNoise != SHARPNOISE_ADAPTIVE) &&
         (B_SharpNoise != SHARPNOISE_LOW) &&
         (B_SharpNoise != SHARPNOISE_MEDIUM) &&
         (B_SharpNoise != SHARPNOISE_HIGH))
   {
      gm_Print("Parameter Sharpness Noise Coring mode is out of range.",0);
      return;
   }
#endif
   // to save new settings in NVRAM
   UserPrefMADI = B_MADI;
   UserPrefCCS = B_CCS;
   UserPrefTNR = B_TNR;
#if (FEATURE_VIDEO_SHARPNOISE == REV2)
   UserPrefSharpNoise = B_SharpNoise;
#endif

   // this call is ONLY to turn on/off debug messages
   gm_AdaptiveGlobalMotion(B_MADI, B_TNR, B_CCS,
                           B_Noise_Monitor,B_RS_Monitor,B_Region_Monitor);

   // call OSD adjusters while OSD is not implemented yet
   AdjustMADI();
   AdjustCCS();
   AdjustTNR();
#if (FEATURE_VIDEO_SHARPNOISE == REV2)
   AdjustSharpNoise();
#endif

}

//******************************************************************************
//
// FUNCTION       : static void TestEngineeringAdjusters(BYTE B_Adjuster, WORD W_AdjValue)
//
// USAGE          :
//
// INPUT          : B_Adjuster
//                  W_AdjValue
//
// OUTPUT         :
//
// GLOBALS        :
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
static void TestEngineeringAdjusters(BYTE B_Adjuster, WORD W_AdjValue)
{
   B_Adjuster = B_Adjuster;
   W_AdjValue = W_AdjValue;
#ifdef ADJUSTER_REG_MVF_SHARPNESS_LUMA_DECLARED
   switch ( B_Adjuster )
   {
         // Main Advanced Sharpness
      case 1:
         UserPrefReg_MVF_SHARPNESS_LUMA = W_AdjValue;
         AdjustMVF_SHARPNESS_LUMA();
         break;
      case 2:
         UserPrefReg_MVF_SHARPNESS_CHROMA = W_AdjValue;
         AdjustMVF_SHARPNESS_CHROMA();
         break;
      case 3:
         UserPrefReg_MVF_NOISE_CORING_THRESH = W_AdjValue;
         AdjustMVF_NOISE_CORING_THRESH();
         break;
      case 4:
         UserPrefReg_MVF_NONLINEAR_THRESH1 = W_AdjValue;
         AdjustMVF_NONLINEAR_THRESH1();
         break;
      case 5:
         UserPrefReg_MVF_NONLINEAR_GAIN1 = W_AdjValue;
         AdjustMVF_NONLINEAR_GAIN1();
         break;
      case 6:
         UserPrefReg_MVF_NONLINEAR_THRESH2 = W_AdjValue;
         AdjustMVF_NONLINEAR_THRESH2();
         break;
      case 7:
         UserPrefReg_MVF_NONLINEAR_GAIN2 = W_AdjValue;
         AdjustMVF_NONLINEAR_GAIN2();
         break;
      case 8:
         UserPrefReg_MHF_SHARPNESS_LUMA = W_AdjValue;
         AdjustMHF_SHARPNESS_LUMA();
         break;
      case 9:
         UserPrefReg_MHF_SHARPNESS_CHROMA = W_AdjValue;
         AdjustMHF_SHARPNESS_CHROMA();
         break;
      case 10:
         UserPrefReg_MHF_NOISE_CORING_THRESH = W_AdjValue;
         AdjustMHF_NOISE_CORING_THRESH();
         break;
      case 11:
         UserPrefReg_MHF_NONLINEAR_THRESH1 = W_AdjValue;
         AdjustMHF_NONLINEAR_THRESH1();
         break;
      case 12:
         UserPrefReg_MHF_NONLINEAR_GAIN1 = W_AdjValue;
         AdjustMHF_NONLINEAR_GAIN1();
         break;
      case 13:
         UserPrefReg_MHF_NONLINEAR_THRESH2 = W_AdjValue;
         AdjustMHF_NONLINEAR_THRESH2();
         break;
      case 14:
         UserPrefReg_MHF_NONLINEAR_GAIN2 = W_AdjValue;
         AdjustMHF_NONLINEAR_GAIN2();
         break;
      case 15:
         // PIP Advanced Sharpness
         UserPrefReg_PVF_SHARPNESS_LUMA = W_AdjValue;
         AdjustPVF_SHARPNESS_LUMA();
         break;
      case 16:
         UserPrefReg_PVF_SHARPNESS_CHROMA = W_AdjValue;
         AdjustPVF_SHARPNESS_CHROMA();
         break;
      case 17:
         UserPrefReg_PVF_NOISE_CORING_THRESH = W_AdjValue;
         AdjustPVF_NOISE_CORING_THRESH();
         break;
      case 18:
         UserPrefReg_PVF_NONLINEAR_THRESH1 = W_AdjValue;
         AdjustPVF_NONLINEAR_THRESH1();
         break;
      case 19:
         UserPrefReg_PVF_NONLINEAR_GAIN1 = W_AdjValue;
         AdjustPVF_NONLINEAR_GAIN1();
         break;
      case 20:
         UserPrefReg_PVF_NONLINEAR_THRESH2 = W_AdjValue;
         AdjustPVF_NONLINEAR_THRESH2();
         break;
      case 21:
         UserPrefReg_PVF_NONLINEAR_GAIN2 = W_AdjValue;
         AdjustPVF_NONLINEAR_GAIN2();
         break;
      case 22:
         UserPrefReg_PHF_SHARPNESS_LUMA = W_AdjValue;
         AdjustPHF_SHARPNESS_LUMA();
         break;
      case 23:
         UserPrefReg_PHF_SHARPNESS_CHROMA = W_AdjValue;
         AdjustPHF_SHARPNESS_CHROMA();
         break;
      case 24:
         UserPrefReg_PHF_NOISE_CORING_THRESH = W_AdjValue;
         AdjustPHF_NOISE_CORING_THRESH();
         break;
      case 25:
         UserPrefReg_PHF_NONLINEAR_THRESH1 = W_AdjValue;
         AdjustPHF_NONLINEAR_THRESH1();
         break;
      case 26:
         UserPrefReg_PHF_NONLINEAR_GAIN1 = W_AdjValue;
         AdjustPHF_NONLINEAR_GAIN1();
         break;
      case 27:
         UserPrefReg_PHF_NONLINEAR_THRESH2 = W_AdjValue;
         AdjustPHF_NONLINEAR_THRESH2();
         break;
      case 28:
         UserPrefReg_PHF_NONLINEAR_GAIN2 = W_AdjValue;
         AdjustPHF_NONLINEAR_GAIN2();
         break;

         /*	// ENH are removed.
               case 29:
                  // Enhancer
                  UserPrefReg_HEM_HLE_THR = W_AdjValue;
                  AdjustHEM_HLE_THR();
                  break;
               case 30:
                  UserPrefReg_HEM_HLE_GAIN = W_AdjValue;
                  AdjustHEM_HLE_GAIN();
                  break;
               case 31:
                  UserPrefReg_HEM_HDP_THR = W_AdjValue;
                  AdjustHEM_HDP_THR();
                  break;
               case 32:
                  UserPrefReg_HEM_HDP_GAIN = W_AdjValue;
                  AdjustHEM_HDP_GAIN();
                  break;
               case 33:
                  UserPrefReg_HEM_CE_THR = W_AdjValue;
                  AdjustHEM_CE_THR();
                  break;
               case 34:
                  UserPrefReg_HEM_CE_GAIN = W_AdjValue;
                  AdjustHEM_CE_GAIN();
                  break;
         */
      case 35:
         UserPrefReg_VDP_THRESHOLD = W_AdjValue;
         AdjustVDP_THRESHOLD();
         break;
      case 36:
         UserPrefReg_VDP_GAIN = W_AdjValue;
         AdjustVDP_GAIN();
         break;
#if 0
      case 37:
         UserPrefReg_ = W_AdjValue;
         AdjustMPEGNRMode();
         break;
      case 38:
         UserPrefReg_ = W_AdjValue;
         AdjustMPEGNRThresh();
         break;
      case 39:
         UserPrefReg_ = W_AdjValue;
         AdjustACCScheme();
         break;
      case 40:
         UserPrefReg_ = W_AdjValue;
         AdjustACMScheme();
         break;
      case 41:
         UserPrefReg_ = W_AdjValue;
         AdjustACCACMScheme();
         break;
#endif

#if (FEATURE_ACM_3D == ENABLE)
      case 50:
         // setup default regions and correction params, but do not enable
         LLD_ACMSetParameters((VIP_ACMParameters_t far *)&ACM_Params_Default.ACMParams);
      	gm_ACM3DSetRegionDefinitionTablePtr((VIP_ACMRegionDefinitions_t far *)&ACM_Params_Default.ACMRegionDefs);
      	gm_ACM3DSetCorrectionParamsTablePtr((VIP_ACMRegionCorrectionParameters_t far *)&ACM_Params_Default.ACMRegionCorrectionParams);         
         gm_ACM3DInit();
         break;
      case 51:
         UserPrefAcm3RedHue = W_AdjValue-50;
         AdjustAcmRedHue();
         break;
      case 52:
         UserPrefAcm3RedSat = W_AdjValue;
         AdjustAcmRedSat();
         break;
      case 53:
         UserPrefAcm3RedLuma = W_AdjValue;
         AdjustAcmRedLuma();
         break;
      case 54:
         //UserPrefAcmRedLumaOffs = W_AdjValue;
         //AdjustAcmRedLumaOffs();
         break;

      case 61:
         UserPrefAcm3GreenHue = W_AdjValue-50;
         AdjustAcmGreenHue();
         break;
      case 62:
         UserPrefAcm3GreenSat = W_AdjValue;
         AdjustAcmGreenSat();
         break;
      case 63:
         UserPrefAcm3GreenLuma = W_AdjValue;
         AdjustAcmGreenLuma();
         break;
      case 64:
         //UserPrefAcmGreenLumaOffs = W_AdjValue;
         //AdjustAcmGreenLumaOffs();
         break;

      case 71:
         UserPrefAcm3BlueHue = W_AdjValue-50;
         AdjustAcmBlueHue();
         break;
      case 72:
         UserPrefAcm3BlueSat = W_AdjValue;
         AdjustAcmBlueSat();
         break;
      case 73:
         UserPrefAcm3BlueLuma = W_AdjValue;
         AdjustAcmBlueLuma();
         break;
      case 74:
         //UserPrefAcmBlueLumaOffs = W_AdjValue;
         //AdjustAcmBlueLumaOffs();
         break;

      case 81:
         UserPrefAcm3MagentaHue = W_AdjValue-50;
         AdjustAcmMagentaHue();
         break;
      case 82:
         UserPrefAcm3MagentaSat = W_AdjValue;
         AdjustAcmMagentaSat();
         break;
      case 83:
         UserPrefAcm3MagentaLuma = W_AdjValue;
         AdjustAcmMagentaLuma();
         break;
      case 84:
         //UserPrefAcmMagentaLumaOffs = W_AdjValue;
         //AdjustAcmMagentaLumaOffs();
         break;

      case 91:
         UserPrefAcm3YellowHue = W_AdjValue-50;
         AdjustAcmYellowHue();
         break;
      case 92:
         UserPrefAcm3YellowSat = W_AdjValue;
         AdjustAcmYellowSat();
         break;
      case 93:
         UserPrefAcm3YellowLuma = W_AdjValue;
         AdjustAcmYellowLuma();
         break;
      case 94:
         //UserPrefAcmYellowLumaOffs = W_AdjValue;
         //AdjustAcmYellowLumaOffs();
         break;

      case 101:
         UserPrefAcm3CyanHue = W_AdjValue-50;
         AdjustAcmCyanHue();
         break;
      case 102:
         UserPrefAcm3CyanSat = W_AdjValue;
         AdjustAcmCyanSat();
         break;
      case 103:
         UserPrefAcm3CyanLuma = W_AdjValue;
         AdjustAcmCyanLuma();
         break;
      case 104:
         //UserPrefAcmCyanLumaOffs = W_AdjValue;
         //AdjustAcmCyanLumaOffs();
         break;

      case 120:
#ifdef Action_AdjustACM3DColorScheme_WB_Used
         UserPrefACM3DColorScheme = W_AdjValue;
         AdjustACM3DColorScheme();
#endif
#endif

      default:
         break;
   }
#endif   // ADJUSTER_REG_MVF_SHARPNESS_LUMA_DECLARED
}

#ifdef VPX322xF_DEV_ID

//******************************************************************************
//
// FUNCTION       : void VpxReadWriteI2CReg (void)
//
// USAGE          :
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        :
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void VpxReadWriteI2CReg (void)
{
   BYTE B_Func = (BYTE)GetParam(0);   // read or write
   BYTE B_Addr = (BYTE)GetParam(1);   // I2C address
   BYTE B_Data = (BYTE)GetParam(2);   // Value
   BYTE i;
   switch (B_Func)
   {
      case 0:
         B_Data = dev_vpx32xxByteRead   (VPX322xF_DEV_ID,B_Addr);
         gm_Printf("Read I2C REG 0X%X = %x",B_Addr,B_Data);
         break;

      case 1:
         dev_vpx32xxByteWrite   (VPX322xF_DEV_ID, B_Addr, B_Data);
         gm_Print("Writing to I2C Reg 0x%x",B_Addr);
         break;

   }
}

//******************************************************************************
//
// FUNCTION       : void VpxReadWriteFPReg (void)
//
// USAGE          :
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        :
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void VpxReadWriteFPReg (void)
{
   BYTE B_Func = (BYTE)GetParam(0);   // read or write
   WORD W_Addr = (WORD)GetParam(1);   // I2C address
   WORD W_Data = (WORD)GetParam(2);   // Value
   BYTE i;

   switch (B_Func)
   {
      case 0:
         W_Data = dev_vpx32xxFPRead   (VPX322xF_DEV_ID,(WORD)(W_Addr));
         if (W_Data>>8)
            gm_Printf("Read FP REG 0X%X = %x%x%x",W_Addr,(BYTE)(W_Data>>8),(BYTE)(0xff00&W_Data),(BYTE)(0x00ff&W_Data));
         else
            gm_Printf("Read FP REG 0X%X = %x%x%x",W_Addr,(BYTE)(W_Data>>8),(BYTE)(0xff00&W_Data),(BYTE)(0x00ff&W_Data));

         gm_Printf("Read FP REG 0X%X = %x",W_Addr,W_Data);
         break;

      case 1:
         dev_vpx32xxFPWrite   (VPX322xF_DEV_ID, W_Addr, (WORD)W_Data);
         gm_Print("Writing to FP REG",0);
         break;
   }
}


//
// NOTE THIS APPSTEST CONFIGURES THE DISPLAY TO USE THE VPX DECODER.
// BOTH MAIN AND PIP SHOULD BE ENABLED AND SETUP FOR ANY INPUT
// AS LONG AS A STABLE IMAGE IS SEEN *BEFORE* RUNNING THIS APPSTEST.
// THIS IS ONLY A PROOF OF CONCEPT THAT VIDEO CAN BE SEEN WITH CORTEZ.
// LITTLE TIME HAS BEEN INVESTED ON THE VIDEO ACQUISTION + SETUP LOGIC.
//
// THE FOCUS WAS ON TELETEXT THROUGH THE VPX DECODER.
//
//******************************************************************************
//
// FUNCTION       : void VpxSetupMainPip(void)
//
// USAGE          :
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        :
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void VpxSetupMainPip(void)
{
   VpxInit();   // VPX initialization for PAL + VBI window
   //
   // Cortez Specific Registers - PIP Configuration
   //
   gm_WriteRegWord (DIP_PIP_CTRL, 0x108);
   gm_WriteRegWord (IPP_SOURCE, 0x0);
   //
   // Main Configuration
   //
   gm_WriteRegWord (DIP_MAIN_CTRL, 0x108);
   gm_WriteRegWord (IMP_SOURCE, 0x0);
   //
   // Choose Appropriate Clock to route to main and pip
   // NOTE: This register may need to be altered based on the board
   // configuration. If there are problems try changing IMP_CLK_SEL and IPP_CLK_SEL
   // such that it chooses the valid input clock from the digital port (there are 4 operational clocks)
   //
   gm_WriteRegWord (CLOCK_CONFIG2, 0x124A);
   //
   // Update Registers
   //
   gm_WriteRegWord (HOST_CONTROL, 0x70);
   msg ("VPX Initialized for Pal Input");
}
#endif // #ifdef VPX322xF_DEV_ID

#ifdef BLACK_FRAME_INSERTION_METHOD
static void Force_Display_Window_With_Black_Color(void)
{
   if (Method_Black_Frame_Insertion_Enable)
   {
      Method_Black_Frame_Insertion_Enable = FALSE;
      gm_ClearRegBitsWord(DISPLAY_CONTROL, FORCE_COLOR);
#if (FEATURE_INTERRUPT_SERVICE == REV2)
      INTERRUPT_DISABLE(DISPLAY_DEC_IRQ5MASK, D_VS_EDGE);
#endif
      gm_Print("Disable BLACK Frame Insertion. Using Frame repeated.",0);
   }
   else
   {
      Method_Black_Frame_Insertion_Enable = TRUE;
      Method_Grey_Frame_Insertion_Enable = FALSE;
#if (defined(ACC_LUT_METHOD))
      Method_ACC_LUT_Enable = FALSE;
#endif
   }
}
#endif

#ifdef GREY_FRAME_INSERTION_METHOD
static void Force_Display_Window_With_Grey_Color(void)
{
   if (Method_Grey_Frame_Insertion_Enable)
   {
      Method_Grey_Frame_Insertion_Enable = FALSE;
      gm_ClearRegBitsWord(DISPLAY_CONTROL, FORCE_COLOR);
#if (FEATURE_INTERRUPT_SERVICE == REV2)
      INTERRUPT_DISABLE(DISPLAY_DEC_IRQ5MASK, D_VS_EDGE);
#endif
      gm_Print("Disable GREY Frame Insertion. Using Frame repeated.",0);
   }
   else
   {
      Method_Black_Frame_Insertion_Enable = FALSE;
      Method_Grey_Frame_Insertion_Enable = TRUE;
#if (defined(ACC_LUT_METHOD))
      Method_ACC_LUT_Enable = FALSE;
#endif
   }
}
#endif

#if (defined(ACC_LUT_METHOD))
static void Load_ACC_High_And_Low_Luma_Luts (void)
{
   if (Method_ACC_LUT_Enable)
   {
      Method_ACC_LUT_Enable = FALSE;
      gm_ClearRegBitsWord(ACC_CTRL, ACC_16BIN_EN|ACC_WIN_CTRL);
      gm_WriteRegWord(HOST_CONTROL, ODP_SYNC_UPDATE);
 #if (FEATURE_INTERRUPT_SERVICE == REV2)
      INTERRUPT_DISABLE(DISPLAY_DEC_IRQ5MASK, D_BLANK_MASK);
 #endif
      gm_Print("Disable D_BLANK ISR for ACC LUT method", 0);
   }
   else
   {
      Method_Black_Frame_Insertion_Enable = FALSE;
      Method_Grey_Frame_Insertion_Enable = FALSE;
      Method_ACC_LUT_Enable = TRUE;
   }
}
#endif


/******************************************************************************/
//
// FUNCTION       :static void AdjVCaptureForDecVLineVar(void)
//
// USAGE          : This function is just a prototype of the algorithm.This might be extended for all the cases.
//             The function programs the backend capture depending upon the Line variation(Decoder).
//
//
// INPUT          : none
//
// OUTPUT         : None
//
// GLOBALS        : gmvw_InputVActive[B_Channel]
//
// USED_REGS      : IMP_V_ACT_START_EVEN ,IMP_V_ACT_LENGTH
//
//******************************************************************************/
 #if 0
static void AdjVCaptureForDecVLineVar(void)
{
   WORD W_ReqLinesInAFrame,W_CurrentDecoderLineCount;
   WORD W_CurrentInputVStart,W_CurrentInputVActive;
   BYTE B_Channel = CH_A;

   //If the decoder is not used for the channel then return
   if ( (IsDecoderUsed(B_Channel)== FALSE )
         ||(gm_DecoderGetStatus(gmd_DEC_CORTEZ_ID,gmd_DEC_SIGNAL_PRESENT ) == FALSE )
      )
   {
      gm_Printf("CH_A doesn't have a valid signal:Check the connections");
      return;
   }
   //get the Decoder line count
   W_CurrentDecoderLineCount = gm_DecoderGetStatus( gmd_DEC_CORTEZ_ID, gmd_DEC_LINECOUNT);

   //Get the start and active lenght
   W_CurrentInputVStart = gm_ReadRegWord(IMP_V_ACT_START_EVEN);
   W_CurrentInputVActive =  gm_ReadRegWord(IMP_V_ACT_LENGTH);

//*2 is to converted to lines in a frame.
   W_ReqLinesInAFrame = (W_CurrentInputVStart + W_CurrentInputVActive) * 2 ;


// 3 : bcos if one line is less in a field ,then it corresponds to 3lines less in a frame
   if ( W_CurrentDecoderLineCount <= (W_ReqLinesInAFrame - 3) )
   {
//Convert back to field by diving it by 2.
      W_CurrentDecoderLineCount = (W_ReqLinesInAFrame - W_CurrentDecoderLineCount)/2 ;
//reduce the active lines in order to avoid flickering.
      gm_WriteRegWord(IMP_V_ACT_LENGTH, W_CurrentInputVActive -W_CurrentDecoderLineCount);
      gm_Print("V_ACT_LENGHT programmed = %d",(W_CurrentInputVActive -W_CurrentDecoderLineCount));
   }
   else
   {
//restore the original values,if there is no variation.
      gm_WriteRegWord(IMP_V_ACT_LENGTH, stInput[B_Channel].Vactive );
      gm_Printf("CH_A has no line variation: restore the original values");

   }
}
 #endif // 0
#endif // FULL_APPSTEST_CMD

#ifdef DP12_BATHTUB_ENABLE //JC_Athena_bathtub
void DPRxBathTubMeas(BYTE B_Lane, BYTE B_eq, BYTE B_fp)
{
   WORD W_CPTrim, W_Ctrl, W_EQ;
   WORD B_CPCount = 0, EQ_Test_Start, EQ_Test_End,FP_Test_Start,FP_Test_End,B_Rx;
   WORD W_ErrorCount;
   DWORD B_Ctrl_setting,B_Ctrl_setting1,EQ_Test = 0, FP_Test=0,B_cptrim=0, B_fpval;

   gm_ClearRegBitsDWord( DP12RX0_LINK_IRQ_CTRL, BIT0);  //Disable CR LOST IRQ
   gm_SetRegBitsDWord( DP12RX0_CLK_CTRL, BIT2); //To select LSCLK from CDR
   gm_ClearRegBitsDWord( CPHY0_PHY_CTRL, CPHY0_DP_MODE );
   B_Ctrl_setting = gm_ReadRegDWord(CPHY0_L0_CTRL_0+12*B_Lane);
   B_Ctrl_setting1=gm_ReadRegDWord(CPHY0_L0_CTRL_4+12*B_Lane);
   B_cptrim= gm_ReadRegWord(CPHY0_L0_CPTRIM+12*B_Lane);
   B_fpval=gm_ReadRegByte(CPHY0_L0_CTRL_4+3+12*B_Lane);
   W_EQ=gm_ReadRegByte(CPHY0_L0_CTRL_0+2+12*B_Lane);
   B_Rx= REG16_ADDR32(0x1F206L);

   gm_ClearRegBitsDWord( CPHY0_L0_CTRL_0+12*B_Lane, (BIT2|BIT1|BIT0) );
   gm_ClearRegBitsDWord( CPHY0_L0_CTRL_4+12*B_Lane, (BIT14|BIT13|BIT12|BIT10|BIT8) );


   if (B_Lane > 4)
   {
      gm_Print("+++Lane number OOR. Setting to 0.", 0);
      B_Lane = 0;
   }

   if ((gm_ReadRegDWord(DP12RX0_LINK_CONTROL)&DPRX_PRBS_EN) == DPRX_PRBS_EN)
      gm_Print("#Bathtub curve for Lane %d with PRBS7", B_Lane);
   else
      gm_Print("#Bathtub curve for Lane %d with Video Stream", B_Lane);



   gm_Print("#AdjReg=0x%x /EQ=0x%x/FP=0x%x/CP=0x%x", (WORD)B_Rx,((WORD)W_EQ&0x7), ((WORD)(B_fpval)&0x7),(WORD)(B_cptrim& 0x3F)); //EQ by LT

   if (B_eq > 8)
      B_eq = 8;


   if (B_eq==8)
   {
      EQ_Test_Start = 0;
      EQ_Test_End = 8;
   }
   else
   {
      EQ_Test_Start = B_eq;
      EQ_Test_End = B_eq + 1;
   }

   if (B_fp > 9)
      B_fp = 9;

   if (B_fp==8)
   {
      FP_Test_Start = 0;
      FP_Test_End = 8;
   }
   else if (B_fp==9)
   {
      FP_Test_Start = 4;
      FP_Test_End = 8;        
      }
   else
   {
      FP_Test_Start = B_fp;
      FP_Test_End = B_fp + 1;
   }

   for (FP_Test = FP_Test_Start; FP_Test < FP_Test_End; FP_Test++)
   {
      gm_Print("##FP= 0x%x",FP_Test);
      gm_ClearAndSetBitsDWord(CPHY0_L0_CTRL_4+12*B_Lane, CPHY0_L0_EQ_FPEAK, (DWORD)(FP_Test<<24));
      //    gm_Print("*FPLow: 0x%x",(WORD)(FP_Test & 0x0000FFFFL));
      //    gm_Print("*FPHigh: 0x%x",(WORD)((FP_Test & 0xFFFF0000L)>>16));


      for (EQ_Test = EQ_Test_Start; EQ_Test < EQ_Test_End; EQ_Test++)
      {
         B_CPCount = 0;

         gm_Print("##EQ=0x%x",EQ_Test);
         gm_ClearAndSetBitsDWord(CPHY0_L0_CTRL_0+12*B_Lane, CPHY0_L0_GAIN, (DWORD)(EQ_Test<<16));

         gm_Delay10ms(10);


         gm_SetRegBitsDWord(CPHY0_L0_CTRL_0+12*B_Lane, CPHY0_L0_DIS_TRIM_CAL);
         gm_ClearAndSetBitsByte(DP12RX0_ERR_CNT_CTRL, DPRX_ERR_CNT_SEL, (B_Lane << DPRX_ERR_CNT_SEL_SHIFT));
         gm_ClearRegBitsByte(DP12RX0_ERR_CNT_CTRL, DPRX_ERR_CNT_EN);


         for (B_CPCount = 0; B_CPCount < 0x40; B_CPCount++)
         {
            gm_ClearAndSetBitsDWord(CPHY0_L0_CTRL_0+12*B_Lane, CPHY0_L0_CPTIN, (WORD)((B_CPCount & 0x00FF)<<8));
            gm_SetRegBitsByte(DP12RX0_ERR_CNT_CTRL, DPRX_ERR_CNT_EN);
            gm_Delay10ms(20);     //From 200ms
            W_ErrorCount = gm_ReadRegWord(DP12RX0_ERROR_CNT);
            gm_ClearRegBitsByte(DP12RX0_ERR_CNT_CTRL, DPRX_ERR_CNT_EN);

            if (W_ErrorCount & 0x8000)
               gm_Print(" %d", (W_ErrorCount & 0x7FFF));
            else
               gm_Print("+++Error count is invalid", 0);

            
         }
      }
   }

   gm_WriteRegDWord(CPHY0_L0_CTRL_0+12*B_Lane, B_Ctrl_setting);
   gm_WriteRegDWord(CPHY0_L0_CTRL_4+12*B_Lane, B_Ctrl_setting1);
   gm_ClearRegBitsDWord( CPHY0_L0_CTRL_4+12*B_Lane, (BIT14|BIT13|BIT12|BIT10|BIT8) );

   // gm_WriteRegByte(VDDS1_INIT, 0x11);
   gm_Print("END$$$$$$$$$$$$$$$$$$$$$$$", 0);

}


#endif // DP12_BATHTUB_ENABLE

#ifdef DP11_BATHTUB_ENABLE
#define MeasPORT_DP1	0
#define MeasPORT_DP2	1

void DPRxBathTubMeas_DP(BYTE B_TestedLane, BYTE B_TestedFp, BYTE B_NumCPT, WORD W_Period10ms, BYTE B_Port)
{
   BYTE B_CPTrim;
   BYTE B_LaneStart, B_LaneEnd, B_FpStart, B_FpEnd, B_CPTStart, B_CPTEnd;
   BYTE B_Lane, B_Fp, B_EQ, B_CPT;
	WORD W_Temp, W_Temp1, W_Val;
	WORD W_RegCtrl0, W_RegCtrl4, W_RegCPTrim, W_RegIrqCtrl, W_RegPhyCtrl;
   DWORD DW_Ctrl0, DW_Ctrl4, DW_IrqCtrl;

	if (B_Port == MeasPORT_DP2)
	{
		W_RegIrqCtrl = DP12RX1_LINK_IRQ_CTRL;
		W_Temp = REG16_ADDR32(0x1B206L);
		W_Temp1 = (WORD)gm_ReadRegDWord_LE(DP12RX1_LINK_CONTROL);
		W_RegPhyCtrl = CPHY2_PHY_CTRL;
		W_Val = (WORD)gm_ReadRegDWord_LE(DP12RX1_LT_STATUS);
	}
	else
	{
		W_RegIrqCtrl = DP12RX0_LINK_IRQ_CTRL;
		W_Temp = REG16_ADDR32(0x1F206L);
		W_Temp1 = (WORD)gm_ReadRegDWord_LE(DP12RX0_LINK_CONTROL);
		W_RegPhyCtrl = CPHY0_PHY_CTRL;
		W_Val = (WORD)gm_ReadRegDWord_LE(DP12RX0_LT_STATUS);
	}

	DW_IrqCtrl = gm_ReadRegDWord_LE(W_RegIrqCtrl);
	gm_ClearRegBitsDWord(W_RegIrqCtrl, DPRX_CR_LOST_IRQ_EN);
	gm_Delay10ms(1);
	gm_SetRegBitsDWord((W_RegIrqCtrl - 8), DPRX_LSCLK_135_SRC);

	gm_Printf("LT Info: 0x%x / #AdjReg: 0x%x / Collection Period: %dms", W_Val, W_Temp, W_Period10ms * 10);

   if ((W_Temp1 & DPRX_PRBS_EN) == DPRX_PRBS_EN)
      gm_Print("#Bathtub with PRBS7", W_Temp1);
   else
      gm_Print("#Bathtub with Video Stream", W_Temp1);

   gm_ClearRegBitsDWord(W_RegPhyCtrl, CPHY0_DP_MODE);

	if (B_TestedLane < 4)
	{
		B_LaneStart = B_TestedLane;
		B_LaneEnd = B_LaneStart + 1;
	}
	else
	{
		B_LaneStart = 0;
		B_LaneEnd = 4;
	}

	if (B_TestedFp < 8)
	{
		B_FpStart = B_TestedFp;
		B_FpEnd = B_FpStart + 1;
	}
	else
	{
		B_FpStart = 0;
		B_FpEnd = 8;
	}

	if (B_NumCPT <= 64)
	{
		B_CPTStart = 32 - (B_NumCPT >> 1);
		B_CPTEnd = B_CPTStart + B_NumCPT;
	}
	else
	{
		B_CPTStart = 0;
		B_CPTEnd = 64;
	}
	
	for (B_Lane = B_LaneStart; B_Lane < B_LaneEnd; B_Lane++)
	{
		gm_Print("Lane %d is under testing ......", (WORD)B_Lane);

		if (B_Port == MeasPORT_DP2)
		{
			W_RegCtrl0 = (WORD)B_Lane * 12 + CPHY2_L0_CTRL_0;
			W_Temp = DP12RX1_ERR_CNT_CTRL;
			W_Temp1 = DP12RX1_ERROR_CNT;
		}
		else
		{
			W_RegCtrl0 = (WORD)B_Lane * 12 + CPHY0_L0_CTRL_0;
			W_Temp = DP12RX0_ERR_CNT_CTRL;
			W_Temp1 = DP12RX0_ERROR_CNT;
		}
		
		W_RegCtrl4 = W_RegCtrl0 + 4;
		W_RegCPTrim = W_RegCtrl4 + 4;

		DW_Ctrl0 = gm_ReadRegDWord_LE(W_RegCtrl0);
		DW_Ctrl4 = gm_ReadRegDWord_LE(W_RegCtrl4);
		
		//gm_ClearRegBitsDWord(W_RegCtrl0, (BIT2|BIT1|BIT0));
		gm_ClearRegBitsDWord(W_RegCtrl4, (BIT14|BIT13|BIT12|BIT10|BIT8));

		gm_ClearAndSetBitsByte(W_Temp, DPRX_ERR_CNT_SEL, (B_Lane << DPRX_ERR_CNT_SEL_SHIFT));
		
		for (B_Fp = B_FpStart; B_Fp < B_FpEnd; B_Fp++)
		{
			gm_Print("FP = %d", (WORD)B_Fp);
			gm_ClearAndSetBitsDWord(W_RegCtrl4, CPHY0_L0_EQ_FPEAK, ((DWORD)B_Fp << 24));
		
			for (B_EQ = 0; B_EQ < 8; B_EQ++)
			{
				gm_Print("EQ = %d", (WORD)B_EQ);
				gm_ClearAndSetBitsDWord(W_RegCtrl0, CPHY0_L0_GAIN, ((DWORD)B_EQ << 16));
				gm_ClearRegBitsDWord(W_RegCtrl0, CPHY0_L0_DIS_TRIM_CAL);
				gm_Delay1ms(1);
				B_CPTrim = gm_ReadRegByte(W_RegCPTrim);
				B_CPTrim &= 0x3F;
				gm_Print("CPT = 0x%x", (WORD)B_CPTrim);
				gm_SetRegBitsDWord(W_RegCtrl0, CPHY0_L0_DIS_TRIM_CAL);
				
				for (B_CPT = 0; B_CPT < 64; B_CPT++)
				{
					if ((B_CPT >= B_CPTStart) && (B_CPT < B_CPTEnd))
					{
						gm_ClearAndSetBitsDWord(W_RegCtrl0, CPHY0_L0_CPTIN, ((DWORD)B_CPT << 8));
						gm_ClearRegBitsByte(W_Temp, DPRX_ERR_CNT_EN);
						gm_SetRegBitsByte(W_Temp, DPRX_ERR_CNT_EN);
			
						gm_Delay10ms(W_Period10ms);
			
						W_Val = gm_ReadRegWord(W_Temp1);
					
						if (W_Val & 0x8000)
						{
							if (B_CPT == B_CPTrim)
								gm_Print("CPTrim", 0);
							else
								gm_Print(" %d", (W_Val & 0x7FFF));
						}
						else
							gm_Print("+++Error count is invalid", 0);
					}
					else
					{
						gm_Print("Skipped", 0);
					}
				}
			}
		}

		gm_WriteRegDWord_LE(W_RegCtrl0, DW_Ctrl0);
		gm_WriteRegDWord_LE(W_RegCtrl4, DW_Ctrl4);
	}

	gm_WriteRegDWord_LE(W_RegIrqCtrl, DW_IrqCtrl);

   gm_Print("End of the collection", 0);

}
#endif // End of DP11_BATHTUB_ENABLE

#ifdef VIDEO_CAPTURE
void CV_Prepare(void)
{
   //Freeze(gmvb_MainChannel, FALSE);
   //gm_ClearAndSetBitsWord(MFRC_CTRL, (BIT4 |BIT5 | BIT6), (BIT4));
   //gm_ForceUpdate(gmvb_MainChannel, INPUT_OUTPUT_CLOCK);

   FrameMainStart=gm_MemoryGetInfo(BUF_MAIN, BUF_START_ADDRESS)* BURST_SIZE;
   FrameMainEnd=gm_MemoryGetInfo(BUF_MAIN, BUF_END_ADDRESS)* BURST_SIZE;
   FramePipEnd=gm_MemoryGetInfo(BUF_PIP, BUF_END_ADDRESS)* BURST_SIZE;

   CDATA_Line = 0; // (WORD)GetParam(0);

   gm_WaitForDisplaySync(1);
   Freeze(gmvb_MainChannel, TRUE);

   gm_ClearRegBitsWord(MFRC_CTRL, (BIT4 |BIT5 | BIT6));
   gm_WaitForDisplaySync(1);
   gm_ForceUpdate(gmvb_MainChannel, INPUT_OUTPUT_CLOCK);
   Freeze(gmvb_MainChannel, FALSE);
}

#define BUFF_SIZE						1024
#define MAX_WORDS 						32			// For Read or Write to memory
#define BURST_SIZE						64			// 64 Byte bursts
#define BURST_SIZE_SHIFT				6

void Capture_Video_Handler(void)
{
   DWORD Phy_Start, Phy_End, Store_start, Store_End, data_size;
   //WORD i, j, data_index;
   //BYTE CPN[5760], k;

   //FrameMainStart=gm_MemoryGetInfo(BUF_MAIN, BUF_START_ADDRESS)* BURST_SIZE;
   //FramePipStart=gm_MemoryGetInfo(BUF_PIP, BUF_START_ADDRESS)* BURST_SIZE;
   if ((CDATA_Flag == 0))
   {
      return;
   }

   switch (CDATA_Flag)
   {
      case 1:	// Freeze Bufer 1.
         CV_Prepare();
         CDATA_Flag = 0;
         break;

      case 2:	// Load Image to main buffer (0) from buffer 1.

         Freeze(gmvb_MainChannel, FALSE);

         FrameMainStart=gm_MemoryGetInfo(BUF_MAIN, BUF_START_ADDRESS);
         FrameMainEnd=gm_MemoryGetInfo(BUF_MAIN, BUF_END_ADDRESS);
         FramePipEnd=gm_MemoryGetInfo(BUF_PIP, BUF_END_ADDRESS);

         data_size = (0xFFFFF - FramePipEnd) / 0x1A586;

         Store_End = 0xFFFFF - (Photo_Index * 0x1A586);
         Store_start = Store_End - 0x1A586;
         if (Store_start < FramePipEnd)
         {
            gm_Print("Room not enough ... Total Photos %d", Photo_Index);
            CDATA_Flag = 0;
            CDATA_Line = 0; //(WORD)GetParam(0);
            return;
         }
         Photo_Index ++;

         gm_Print("Total rooms are %d ", data_size);
         gm_Print("Empty is %d ", data_size - Photo_Index);
         //Store_start = FrameStoreEnd - ((FrameMainEnd - FrameMainStart) / Frame_num);

         Frame_num = ((gm_ReadRegWord(MFRC_CTRL) & MFRC_NUMBUF_MINUS_ONE) >> 4);
         //gm_Print("Total frame = ... %d", Frame_num);

         gm_ClearRegBitsWord(MFRC_CTRL, MFRC_NUMBUF_MINUS_ONE);
         gm_ForceUpdate(gmvb_MainChannel, INPUT_OUTPUT_CLOCK);

         gm_WriteRegWord(MFRC_WBUF_START_hi, (Store_start >> 16) & 0x00FF);
         gm_WriteRegWord(MFRC_WBUF_START_lo, Store_start & 0x0000FFFF);

         gm_WriteRegWord(MFRC_WBUF_END_hi, (Store_End >> 16) & 0x00FF);
         gm_WriteRegWord(MFRC_WBUF_END_lo, Store_End & 0x0000FFFF);
         gm_ForceUpdate(gmvb_MainChannel, INPUT_OUTPUT_CLOCK);

         gm_WaitForDisplaySync(3);
         Freeze(gmvb_MainChannel, TRUE);

         gm_WriteRegWord(MFRC_WBUF_START_hi, (FrameMainStart >> 16) & 0x00FF);
         gm_WriteRegWord(MFRC_WBUF_START_lo, FrameMainStart & 0x0000FFFF);

         gm_WriteRegWord(MFRC_WBUF_END_hi, (FrameMainEnd >> 16) & 0x00FF);
         gm_WriteRegWord(MFRC_WBUF_END_lo, FrameMainEnd & 0x0000FFFF);
         gm_ForceUpdate(gmvb_MainChannel, INPUT_OUTPUT_CLOCK);

         gm_SetRegBitsWord(MFRC_CTRL, (Frame_num << 4));
         gm_ForceUpdate(gmvb_MainChannel, INPUT_OUTPUT_CLOCK);
         Frame_num = 0;

         gm_WaitForDisplaySync(1);
         Freeze(gmvb_MainChannel, FALSE);

         CDATA_Flag = 0;
         CDATA_Line = 0; //(WORD)GetParam(0);
         break;

      case 3:	// Read image from ext memory to main buffer. (After end of pip buffer).
         if (Photo_Select >= Photo_Index)
         {
            gm_Print("Photos not enough ... Total Photos %d", Photo_Index);
            CDATA_Flag = 0;
            CDATA_Line = 0; //(WORD)GetParam(0);
            return;
         }

         Store_End = 0xFFFFF - (Photo_Select * 0x1A586);
         Store_start = Store_End - 0x1A586;

         gm_ClearRegBitsWord(MFRC_CTRL, MFRC_NUMBUF_MINUS_ONE);
         gm_ForceUpdate(gmvb_MainChannel, INPUT_OUTPUT_CLOCK);

         gm_WriteRegWord(MFRC_RBUF_START_hi, (Store_start >> 16) & 0x00FF);
         gm_WriteRegWord(MFRC_RBUF_START_lo, Store_start & 0x0000FFFF);

         gm_WriteRegWord(MFRC_RBUF_END_hi, (Store_End >> 16) & 0x00FF);
         gm_WriteRegWord(MFRC_RBUF_END_lo, Store_End & 0x0000FFFF);

         gm_ForceUpdate(gmvb_MainChannel, INPUT_OUTPUT_CLOCK);

         //gm_Print("Main Buffer Start Address : 0x%lX", FrameMainStart);
         //gm_Print("Main Buffer End Address : 0x%lX", FrameMainEnd);
         //gm_Print("PIP Buffer End Address : 0x%lX", FramePipEnd);


         CDATA_Flag = 0;
         CDATA_Line = 0; //(WORD)GetParam(0);
         break;

      case 4:	// Read image from ext memory to main buffer. (After end of pip buffer).

         if (Photo_Index == 0)
         {
            gm_Print("No photo be stored ......... %d", 0);
            CDATA_Flag = 0;
            CDATA_Line = 0; //(WORD)GetParam(0);
         }
         else if (gm_TimerCheck(TV_BACKGROUNDAFT_100ms_TMR) == TIMER_TMO)
         {
            Store_End = 0xFFFFF - (Photo_Select * 0x1A586);
            Store_start = Store_End - 0x1A586;

            gm_ClearRegBitsWord(MFRC_CTRL, MFRC_NUMBUF_MINUS_ONE);
            gm_ForceUpdate(gmvb_MainChannel, INPUT_OUTPUT_CLOCK);

            gm_WriteRegWord(MFRC_RBUF_START_hi, (Store_start >> 16) & 0x00FF);
            gm_WriteRegWord(MFRC_RBUF_START_lo, Store_start & 0x0000FFFF);

            gm_WriteRegWord(MFRC_RBUF_END_hi, (Store_End >> 16) & 0x00FF);
            gm_WriteRegWord(MFRC_RBUF_END_lo, Store_End & 0x0000FFFF);

            gm_ForceUpdate(gmvb_MainChannel, INPUT_OUTPUT_CLOCK);

            if (Photo_Index == 1)
            {
               gm_TimerStop(TV_BACKGROUNDAFT_100ms_TMR);
               CDATA_Flag = 0;
               CDATA_Line = 0; //(WORD)GetParam(0);
            }
            else
            {
               if ((Photo_Select + 1) >= Photo_Index)
                  Photo_Select = 0;
               else
                  Photo_Select ++;

               gm_TimerStart(TV_BACKGROUNDAFT_100ms_TMR, Display_Time);
            }
         }

         break;

      case 5:
         Photo_Index = Photo_Select;
         gm_Print("Clear Photos ... For re-load new photos %d", Photo_Select);
         Photo_Select = 0;

         CDATA_Flag = 0;
         CDATA_Line = 0; //(WORD)GetParam(0);
         break;

      case 6:
         Display_Time = (BYTE)Photo_Select;
         gm_Print("Photos display time set to %d s", Photo_Select);
         Photo_Select = 0;

         CDATA_Flag = 0;
         CDATA_Line = 0; //(WORD)GetParam(0);
         break;

      case 7:
         gm_WriteRegWord(AUTO_BKGND_CONTROL, 0);
         gm_WriteRegWord(AFR_CONTROL, 0);
         gm_WriteRegWord(DISPLAY_DEC_STATUS, 0xFFFF);
         gm_WriteRegByte(TDEC_SOFTRESET2, 0xF7);
         gm_WriteRegWord(MC_MEMORY_CONFIG, 0x30);
         gm_WriteRegWord(MFRC_BUF_WIDTH, 0x5A);
         gm_WriteRegWord(MFRC_RD_WIDTH, 0x5A);
         gm_WriteRegWord(MAIN_SCALING_FILTER_CTRL, 0x3A);
         gm_WriteRegWord(DISPLAY_CONTROL, 0x07);
         gm_WriteRegWord(PB_CTRL, 0x01);
         gm_WriteRegWord(PB_BKGND, 0xF9E7);
         gm_WriteRegWord(PB_BKGND_HEIGHT, 0x4B0);
         gm_WriteRegWord(PB_BKGND_WIDTH, 0x780);
         gm_WriteRegWord(PB_MAIN_VHEIGHT, 0x4B0);
         gm_WriteRegWord(PB_MAIN_HWIDTH, 0x780);
         gm_SetRegBitsWord(IMP_CONTROL, IMP_RUN_EN);
         //gm_SetRegBitsWord(ODP_CTRL, MAIN_RGB_EN);
         HideOsd();
         gm_ClearRegBitsWord(OSD_CONTROL, OSD_ENABLE);
         CDATA_Flag = 0;
         CDATA_Line = 0; //(WORD)GetParam(0);
         break;

      case 9:
         CDATA_Flag = 0;
         CDATA_Line = 0;

         gm_WriteRegWord(MFRC_RBUF_START_hi, (FrameMainStart >> 16) & 0x00FF);
         gm_WriteRegWord(MFRC_RBUF_START_lo, FrameMainStart & 0x0000FFFF);

         gm_WriteRegWord(MFRC_RBUF_END_hi, (FrameMainEnd >> 16) & 0x00FF);
         gm_WriteRegWord(MFRC_RBUF_END_lo, FrameMainEnd & 0x0000FFFF);
         gm_ForceUpdate(gmvb_MainChannel, INPUT_OUTPUT_CLOCK);

         gm_SetRegBitsWord(MFRC_CTRL, (Frame_num << 4));
         gm_ForceUpdate(gmvb_MainChannel, INPUT_OUTPUT_CLOCK);
         Frame_num = 0;

         gm_WaitForDisplaySync(1);
         Freeze(gmvb_MainChannel, FALSE);
         break;

   }
}
#endif

#ifdef FULL_APPSTEST_CMD
void DisableSpiFlashCache(BOOL flag)
{
   static WORD save_cache_ctrl2=0;

   if (!save_cache_ctrl2)
      save_cache_ctrl2 = gm_ReadRegWord(CACHE_CTRL2);

   if (flag) // disable SPI-flash cache
   {
      DISABLE_INTERRUPTS;
      gm_WriteRegWord(CACHE_CTRL2, CACHE_PURGE_SERIAL|(1<<CACHE_BURST_SIZE_SERIAL_SHIFT));
      gm_WriteRegWord(CACHE_UPDATE_EN, CACHE_UPDATE_SERIAL_EN);
      ENABLE_INTERRUPTS;
   }
   else
   {
      DISABLE_INTERRUPTS;
      gm_WriteRegWord(CACHE_CTRL2, save_cache_ctrl2);
      gm_ClearRegBitsWord(CACHE_CTRL2,CACHE_PURGE_SERIAL);
      gm_WriteRegWord(CACHE_UPDATE_EN, CACHE_UPDATE_SERIAL_EN);
      ENABLE_INTERRUPTS;
   }

}

#define POWER_TEST_CASE 3 // 1~7
BYTE PowerTestIndex=POWER_TEST_CASE;

void AdjustPowerForTest(void)
{
   switch(PowerTestIndex) 
   {
      case 1:
         gm_WriteRegDWord_LE(DPTX_AUX_PHY_CTRL, 0x66);
         gm_WriteRegDWord_LE(DPTX_PHY_CTRL, 0x808FF);
         gm_SetRegBitsDWord(DPTX_CLK_CTRL, DPTX_TXPLL_PD);
         gm_WriteRegDWord_LE(DPTX_CTL_PLL_FREQ, 1);
         gm_SetRegBitsWord(CLOCK_CONFIG4,DPTX_VCLK_SEL);
         gm_SetRegBitsWord(CLOCK_CONFIG9, DPTX_AUX_CLK_SEL);
         gm_ClearAndSetBitsWord(CLOCK_CONFIG5,7<<6, 1<<6); // DPTX_AUX_CLK_SEL
         gm_WriteRegByte(AFE_DAC_CTRL1,0);
         gm_WriteRegWord(ADC_CONTROL0,0xE);
         gm_WriteRegWord(ASE_CONTROL,0);
         gm_ClearAndSetBitsWord(CLOCK_CONFIG2,SCLK_SEL|HD_G_CLK_SEL,1 | 1<<HD_G_CLK_SEL_SHIFT | IP_REF_CLK_SEL);
         gm_WriteRegDWord_LE(DP12RX0_CONTROL, 0);
         gm_WriteRegDWord_LE(DP12RX0_AUX_CONTROL, 0x6588D0);
         gm_WriteRegDWord_LE(DP12RX1_CONTROL, 0);
         gm_WriteRegDWord_LE(DP12RX1_AUX_CONTROL, 0x6588D0);
         gm_ClearAndSetBitsWord(CLOCK_CONFIG4,DPRX1_VCLK_SEL|DPRX2_VCLK_SEL|LT_CLK_SEL, 1|1<<DPRX2_VCLK_SEL_SHIFT|1<<12); // LT_CLK_SEL
         gm_ClearAndSetBitsWord(CLOCK_CONFIG5,DPRX1_AUCLK_SEL|DPRX2_AUCLK_SEL, 1<<DPRX1_AUCLK_SEL_SHIFT|1<<DPRX2_AUCLK_SEL_SHIFT);
         // For DDVI mode, the clock lane (lane 0) of CPHY2 should be shut off. 
         gm_SetRegBitsDWord(CPHY2_L0_CTRL_0, CPHY2_L0_IDDQEN);
         break;

      case 2:
         gm_ClearAndSetBitsWord(CLOCK_CONFIG2,IPP_CLK_SEL, 1<<11);
         gm_WriteRegByte(AFE_DAC_CTRL1,0);
         gm_WriteRegWord(ADC_CONTROL0,0xE);
         gm_WriteRegWord(ASE_CONTROL,0);
         gm_ClearAndSetBitsWord(CLOCK_CONFIG2,SCLK_SEL|HD_G_CLK_SEL,1 | 1<<HD_G_CLK_SEL_SHIFT | IP_REF_CLK_SEL);
         gm_WriteRegDWord_LE(HDMI_RX_PHY_CTRL,0x3);
         gm_ClearAndSetBitsWord(HDMI_CLK_CFG,HDMI_VCLK_SEL|HDMI_AUCLK_SEL,1|1<<8);
         gm_WriteRegDWord_LE(DVI0_RX_PHY_CTRL, 0x3);
         gm_ClearAndSetBitsWord(DVI_CLK_CFG, DVIA_VCLK_SEL,1);
         gm_WriteRegDWord_LE(DVI1_RX_PHY_CTRL, 0x3);
         gm_ClearAndSetBitsWord(DVI_CLK_CFG, DVIB_VCLK_SEL,1<<4);

         gm_WriteRegDWord_LE(CPHY1_AUX_PHY_CTRL, 0x400066);
         gm_SetRegBitsDWord(CPHY1_L0_CTRL_0, CPHY1_L0_IDDQEN);
         gm_SetRegBitsDWord(CPHY1_L1_CTRL_0, CPHY1_L1_IDDQEN);
         gm_SetRegBitsDWord(CPHY1_L2_CTRL_0, CPHY1_L2_IDDQEN);
         gm_SetRegBitsDWord(CPHY1_L3_CTRL_0, CPHY1_L3_IDDQEN);
         gm_SetRegBitsDWord(CPHY1_TEST_CTRL, CPHY1_IMBGEN_PD|CPHY1_RTERM_ENB);

         gm_WriteRegDWord_LE(CPHY2_AUX_PHY_CTRL, 0x400066);
         gm_SetRegBitsDWord(CPHY2_L0_CTRL_0, CPHY2_L0_IDDQEN);
         gm_SetRegBitsDWord(CPHY2_L1_CTRL_0, CPHY2_L1_IDDQEN);
         gm_SetRegBitsDWord(CPHY2_L2_CTRL_0, CPHY2_L2_IDDQEN);
         gm_SetRegBitsDWord(CPHY2_L3_CTRL_0, CPHY2_L3_IDDQEN);
         gm_SetRegBitsDWord(CPHY2_TEST_CTRL, CPHY2_IMBGEN_PD|CPHY2_RTERM_ENB);
         break;

      case 3:
         gm_WriteRegByte(AFE_DAC_CTRL1,0);
         gm_WriteRegWord(ADC_CONTROL0,0xE);
         gm_WriteRegWord(ASE_CONTROL,0);
         gm_ClearAndSetBitsWord(CLOCK_CONFIG2,SCLK_SEL|HD_G_CLK_SEL,1 | 1<<HD_G_CLK_SEL_SHIFT | IP_REF_CLK_SEL);
         gm_WriteRegDWord_LE(HDMI_RX_PHY_CTRL,0x3);
         gm_ClearAndSetBitsWord(HDMI_CLK_CFG,HDMI_VCLK_SEL|HDMI_AUCLK_SEL,1|1<<8);
         // PIP 8 bit
         gm_SetRegBitsWord(MC_MEMORY_CONFIG,PIP_8BIT_COMPRESS);
         // For DDVI mode, the clock lane (lane 0) of CPHY2 should be shut off. 
         gm_SetRegBitsDWord(CPHY2_L0_CTRL_0, CPHY2_L0_IDDQEN);
         break;

      case 4:
         gm_WriteRegDWord_LE(DPTX_AUX_PHY_CTRL, 0x66);
         gm_WriteRegDWord_LE(DPTX_PHY_CTRL, 0x808FF);
         gm_SetRegBitsDWord(DPTX_CLK_CTRL, DPTX_TXPLL_PD);
         gm_WriteRegDWord_LE(DPTX_CTL_PLL_FREQ, 1);
         gm_SetRegBitsWord(CLOCK_CONFIG4,DPTX_VCLK_SEL);
         gm_SetRegBitsWord(CLOCK_CONFIG9, DPTX_AUX_CLK_SEL);
         gm_ClearAndSetBitsWord(CLOCK_CONFIG5,7<<6, 1<<6); // DPTX_AUX_CLK_SEL
         gm_ClearRegBitsWord(DISPLAY_ERROR,ACC_ACM_CLK_CLK_EN);
         gm_ClearAndSetBitsWord(CLOCK_CONFIG2,IPP_CLK_SEL, 1<<11);

         gm_WriteRegByte(AFE_DAC_CTRL1,0);
         gm_WriteRegWord(ADC_CONTROL0,0xE);
         gm_WriteRegWord(ASE_CONTROL,0);
         gm_ClearAndSetBitsWord(CLOCK_CONFIG2,SCLK_SEL|HD_G_CLK_SEL,1 | 1<<HD_G_CLK_SEL_SHIFT | IP_REF_CLK_SEL);
         gm_WriteRegDWord_LE(HDMI_RX_PHY_CTRL,0x3);
         gm_ClearAndSetBitsWord(HDMI_CLK_CFG,HDMI_VCLK_SEL|HDMI_AUCLK_SEL,1|1<<8);

         gm_WriteRegDWord_LE(DP12RX0_CONTROL, 0);
         gm_WriteRegDWord_LE(DP12RX0_AUX_CONTROL, 0x6588D0);
         gm_WriteRegDWord_LE(DP12RX1_CONTROL, 0);
         gm_WriteRegDWord_LE(DP12RX1_AUX_CONTROL, 0x6588D0);
         gm_WriteRegDWord_LE(CPHY0_AUX_PHY_CTRL, 0x400006);
         gm_SetRegBitsDWord(CPHY0_L0_CTRL_0, CPHY0_L0_IDDQEN);
         gm_SetRegBitsDWord(CPHY0_L1_CTRL_0, CPHY0_L0_IDDQEN);
         gm_SetRegBitsDWord(CPHY0_L2_CTRL_0, CPHY0_L0_IDDQEN);
         gm_SetRegBitsDWord(CPHY0_L3_CTRL_0, CPHY0_L0_IDDQEN);
         gm_SetRegBitsDWord(CPHY0_TEST_CTRL, CPHY0_IMBGEN_PD|CPHY0_RTERM_ENB);
         gm_ClearAndSetBitsWord(CLOCK_CONFIG4,DPRX1_VCLK_SEL|DPRX2_VCLK_SEL|LT_CLK_SEL, 1|1<<DPRX2_VCLK_SEL_SHIFT|1<<12); // LT_CLK_SEL
         gm_ClearAndSetBitsWord(CLOCK_CONFIG5,DPRX1_AUCLK_SEL|DPRX2_AUCLK_SEL, 1<<DPRX1_AUCLK_SEL_SHIFT|1<<DPRX2_AUCLK_SEL_SHIFT);
// LVDS 400MHz
         gm_ClearAndSetBitsWord(CLOCK_CONFIG1,ODP_CLK_SEL, 4<<8);
         gm_ClearAndSetBitsWord(LVDS_DLL_CTRL_FREQ,LVDS_DLL_NDIV, 7<<4);
         gm_ClearAndSetBitsWord(CLOCK_CONFIG1,LVTX_IN_CLK_SEL, 4<<11);
         gm_WriteRegDWord_LE(LVDS_DIGITAL_CTRL,0x630043);
         gm_WriteRegDWord_LE(LVDS_BUS_AB_PAD_CTRL,0x22AA22AA);
         gm_WriteRegDWord_LE(LVDS_BUS_CD_PAD_CTRL,0x22AA22AA);
         gm_WriteRegDWord_LE(LVDS_BIAS_CTRL,0x39);
         // For DDVI mode, the clock lane (lane 0) of CPHY2 should be shut off. 
         gm_SetRegBitsDWord(CPHY2_L0_CTRL_0, CPHY2_L0_IDDQEN);
         break;
      case 5:
         gm_WriteRegDWord_LE(LVDS_BUS_CD_PAD_CTRL, 0);
         gm_WriteRegByte(AFE_DAC_CTRL1,0);
         gm_WriteRegWord(ADC_CONTROL0,0xE);
         gm_WriteRegWord(ASE_CONTROL,0);
         gm_ClearAndSetBitsWord(CLOCK_CONFIG2,SCLK_SEL|HD_G_CLK_SEL,1 | 1<<HD_G_CLK_SEL_SHIFT | IP_REF_CLK_SEL);
         gm_WriteRegDWord_LE(DVI0_RX_PHY_CTRL, 0x3);
         gm_ClearAndSetBitsWord(DVI_CLK_CFG, DVIA_VCLK_SEL,1);
         gm_WriteRegDWord_LE(DVI1_RX_PHY_CTRL, 0x3);
         gm_ClearAndSetBitsWord(DVI_CLK_CFG, DVIB_VCLK_SEL,1<<4);
         gm_WriteRegDWord_LE(CPHY2_AUX_PHY_CTRL, 0x400066);
         gm_SetRegBitsDWord(CPHY2_L0_CTRL_0, CPHY2_L0_IDDQEN);
         gm_SetRegBitsDWord(CPHY2_L1_CTRL_0, CPHY2_L1_IDDQEN);
         gm_SetRegBitsDWord(CPHY2_L2_CTRL_0, CPHY2_L2_IDDQEN);
         gm_SetRegBitsDWord(CPHY2_L3_CTRL_0, CPHY2_L3_IDDQEN);
         gm_SetRegBitsDWord(CPHY2_TEST_CTRL, CPHY2_IMBGEN_PD|CPHY2_RTERM_ENB);
         break;
      case 6:
         gm_WriteRegDWord_LE(LVDS_BUS_CD_PAD_CTRL, 0);
         gm_ClearAndSetBitsWord(CLOCK_CONFIG2,IPP_CLK_SEL, 1<<11);
         gm_WriteRegByte(AFE_DAC_CTRL1,0);
         gm_WriteRegWord(ADC_CONTROL0,0xE);
         gm_WriteRegWord(ASE_CONTROL,0);
         gm_ClearAndSetBitsWord(CLOCK_CONFIG2,SCLK_SEL|HD_G_CLK_SEL,1 | 1<<HD_G_CLK_SEL_SHIFT | IP_REF_CLK_SEL);
         gm_WriteRegDWord_LE(HDMI_RX_PHY_CTRL,0x3);
         gm_ClearAndSetBitsWord(HDMI_CLK_CFG,HDMI_VCLK_SEL|HDMI_AUCLK_SEL,1|1<<8);

         gm_WriteRegDWord_LE(DVI0_RX_PHY_CTRL, 0x3);
         gm_ClearAndSetBitsWord(DVI_CLK_CFG, DVIA_VCLK_SEL,1);
         gm_WriteRegDWord_LE(DVI1_RX_PHY_CTRL, 0x3);
         gm_ClearAndSetBitsWord(DVI_CLK_CFG, DVIB_VCLK_SEL,1<<4);

         gm_WriteRegDWord_LE(CPHY1_AUX_PHY_CTRL, 0x400066);
         gm_SetRegBitsDWord(CPHY1_L0_CTRL_0, CPHY1_L0_IDDQEN);
         gm_SetRegBitsDWord(CPHY1_L1_CTRL_0, CPHY1_L1_IDDQEN);
         gm_SetRegBitsDWord(CPHY1_L2_CTRL_0, CPHY1_L2_IDDQEN);
         gm_SetRegBitsDWord(CPHY1_L3_CTRL_0, CPHY1_L3_IDDQEN);
         gm_SetRegBitsDWord(CPHY1_TEST_CTRL, CPHY1_IMBGEN_PD|CPHY1_RTERM_ENB);

         gm_WriteRegDWord_LE(CPHY2_AUX_PHY_CTRL, 0x400066);
         gm_SetRegBitsDWord(CPHY2_L0_CTRL_0, CPHY2_L0_IDDQEN);
         gm_SetRegBitsDWord(CPHY2_L1_CTRL_0, CPHY2_L1_IDDQEN);
         gm_SetRegBitsDWord(CPHY2_L2_CTRL_0, CPHY2_L2_IDDQEN);
         gm_SetRegBitsDWord(CPHY2_L3_CTRL_0, CPHY2_L3_IDDQEN);
         gm_SetRegBitsDWord(CPHY2_TEST_CTRL, CPHY2_IMBGEN_PD|CPHY2_RTERM_ENB);
         break;
      case 7:
         gm_WriteRegDWord_LE(DPTX_AUX_PHY_CTRL, 0x66);
         gm_WriteRegDWord_LE(DPTX_PHY_CTRL, 0x808FF);
         gm_SetRegBitsDWord(DPTX_CLK_CTRL, DPTX_TXPLL_PD);
         gm_WriteRegDWord_LE(DPTX_CTL_PLL_FREQ, 1);
         gm_SetRegBitsWord(CLOCK_CONFIG4,DPTX_VCLK_SEL);
         gm_SetRegBitsWord(CLOCK_CONFIG9, DPTX_AUX_CLK_SEL);
         gm_ClearAndSetBitsWord(CLOCK_CONFIG5,7<<6, 1<<6); // DPTX_AUX_CLK_SEL

         gm_WriteRegDWord_LE(LVDS_BUS_CD_PAD_CTRL, 0);
         gm_WriteRegByte(AFE_DAC_CTRL1,0);
         gm_WriteRegWord(ADC_CONTROL0,0xE);
         gm_WriteRegWord(ASE_CONTROL,0);
         gm_ClearAndSetBitsWord(CLOCK_CONFIG2,SCLK_SEL|HD_G_CLK_SEL,1 | 1<<HD_G_CLK_SEL_SHIFT | IP_REF_CLK_SEL);

         gm_WriteRegDWord_LE(DVI0_RX_PHY_CTRL, 0x3);
         gm_ClearAndSetBitsWord(DVI_CLK_CFG, DVIA_VCLK_SEL,1);
         gm_WriteRegDWord_LE(DVI1_RX_PHY_CTRL, 0x3);
         gm_ClearAndSetBitsWord(DVI_CLK_CFG, DVIB_VCLK_SEL,1<<4);

         gm_WriteRegDWord_LE(CPHY2_AUX_PHY_CTRL, 0x400066);
         gm_SetRegBitsDWord(CPHY2_L0_CTRL_0, CPHY2_L0_IDDQEN);
         gm_SetRegBitsDWord(CPHY2_L1_CTRL_0, CPHY2_L1_IDDQEN);
         gm_SetRegBitsDWord(CPHY2_L2_CTRL_0, CPHY2_L2_IDDQEN);
         gm_SetRegBitsDWord(CPHY2_L3_CTRL_0, CPHY2_L3_IDDQEN);
         gm_SetRegBitsDWord(CPHY2_TEST_CTRL, CPHY2_IMBGEN_PD|CPHY2_RTERM_ENB);
         break;
      default:
         break;
   }

}

WORD ROM Wa_LpmResetConfig[]=
{
   0xDA00, 0x01,
   0xDA06, 0x02,
   0xDA06, 0x0A,
   0xDA06, 0x06,
   0xDA06, 0x0E,
   0xDA06, 0x02,
   0xDA06, 0x0A,
   0xDA06, 0x06,
   0xDA06, 0x0E,
   0xDA06, 0x02,
   0xDA06, 0x0A,
   0xDA06, 0x02,
   0xDA06, 0x0A,
   0xDA06, 0x06,
   0xDA06, 0x0E,
   0xDA06, 0x02,
   0xDA06, 0x0A,
   0xDA06, 0x02,
   0xDA06, 0x0A,
   0xDA06, 0x06,
   0xDA06, 0x0E,
   0xDA06, 0x02,
   0xDA06, 0x0A,
   0xDA06, 0x06,
   0xDA06, 0x0E,
   0xDA06, 0x02,
   0xDA06, 0x0A,
   0xDA06, 0x06,
   0xDA06, 0x0E,
   0xDA06, 0x02,
   0xDA06, 0x0A,
   0xDA06, 0x06,
   0xDA06, 0x0E,
   0xDA06, 0x02,
   0xDA06, 0x0A,
   0xDA06, 0x06,
   0xDA06, 0x0E,
   0xDA06, 0x02,
   0xDA06, 0x0A,
   0xDA06, 0x02,
   0xDA06, 0x0A,
   0xDA06, 0x06,
   0xDA06, 0x0E,
   0xDA06, 0x06,
   0xDA06, 0x0E,
   0xDA06, 0x06,
   0xDA06, 0x0E,
   0xDA06, 0x02,
   0xDA06, 0x0A, 
   0xDA06, 0x02,
   0xDA04, 0x01,
   NULL_PTR
};

void ResetLpm(void)
{
   WORD command[3];
   WORD index = 0;

   command[0]= 1;
   
   while(Wa_LpmResetConfig[index] != NULL_PTR)
   {
      command[1]= Wa_LpmResetConfig[index];
      command[2]= Wa_LpmResetConfig[index+1];
      if (gm_MessageSend(gmd_MESSAGE_DEBUG, &command, sizeof(command)) == FALSE)
         gm_Print("Send request to LPM fail!",0);
      gm_Delay1ms(1);
      index += 2;
   }
}

void MiscTest(BYTE index)
{
   switch (index)
   {
      case 1:
         DisableSpiFlashCache(TRUE);
         break;
      case 2:
         DisableSpiFlashCache(FALSE);
         break;
      case 3:
         gm_PowerDownAll();
         gm_UartInit(UART0, gmc_TCLK_FREQ, 115200L, &uartData);
         break;
      case 4:
         gm_PowerUpAll();
         gm_UartInit(UART0, gmc_OCLK_FREQ, 115200L, &uartData);
         break;
      case 5:
         AdjustPowerForTest();
         break;
      case 6:
         PowerDeepSleepInRequest();
         break;
      case 7:
         ResetLpm();
         break;
      default:
         gm_Print("Misc test:", 0);
         gm_Print("   appstest 18 <index>", 0);
         gm_Print("     index:", 0);
         gm_Print("          1 - Disable SPI-Flash Cache", 0);
         gm_Print("          2 - Enable SPI-Flash Cache", 0);
         gm_Print("          3 - Run gm_PowerDownAll() function", 0);
         gm_Print("          4 - Run gm_PowerUpAll() function", 0);
         gm_Print("          5 - Run AdjustPowerForTest() function", 0);
         gm_Print("          6 - Run PowerDeepSleepInRequest() function", 0);
         gm_Print("          7 - Run ResetLpm() function", 0);
         break;
   }
}

void Set_OCM_Freq(WORD ocm_freq)     // ocm_freq in unit of MHz
{
   gmt_FS432Clk OcmClkSel;
   gmt_RCLK_FREQ_SEL RclkSel;
   DWORD OcmClk;

   if (ocm_freq == 27)
   {
      OcmClkSel = FSCLK_27M;
      RclkSel= RCLKSEL_378M;
      OcmClk = 27000000L;
   }
   else if (ocm_freq == 100)
   {
      OcmClkSel = FSCLK_100M;
      RclkSel= RCLKSEL_378M;
      OcmClk = 100000000L;
   }
   else if (ocm_freq == 135)
   {
      OcmClkSel = FSCLK_135M;
      RclkSel= RCLKSEL_378M;
      OcmClk = 135000000L;
   }
   else if (ocm_freq == 150)
   {
      OcmClkSel = FSCLK_150M;
      RclkSel= RCLKSEL_378M;
      OcmClk = 150000000L;
   }
   else if (ocm_freq == 177)
   {
      OcmClkSel = FSCLK_176_9M;
      RclkSel= RCLKSEL_378M;
      OcmClk = 176900000L;
   }
   else  if (ocm_freq == 200)
   {
      OcmClkSel = FSCLK_200M;
      RclkSel= RCLKSEL_378M;
      OcmClk = 200000000L;
   }
#if 0 // special case for 148.5 MHz
   else  if (ocm_freq == 149)
   {
      // RCLKSEL_378M;
      OcmClk = 148500000L;
      DISABLE_INTERRUPTS;
      gm_ClearRegBitsWord(CLOCK_CONFIG1, OCM_CLK_SEL);
      gm_WriteRegWord(FS432_3_FREQ_CTRL0, 0x0E93);
      gm_WriteRegWord(FS432_3_FREQ_CTRL1, 0x5175);
      gm_ClearAndSetBitsWord(CLOCK_CONFIG1, OCM_CLK_SEL, OCM_CLK_SEL_FS432_SYNTH_CLK3);
      gm_TimerCpuClock(OcmClk);
      gm_UartInit(UART0, OcmClk, 115200L, &uartData);
      ENABLE_INTERRUPTS;
      gm_Print("Set OCM clock as 148.5 MHz",0);
      return;
   }
#endif
   else
   {
      gm_Print("Only support 27/100/135/150/177/200 MHz OCM frequency!",0);
      return;
   }
   
   //Set up clock frequency for flash BEFORE increasing OCLK 
#if (FEATURE_SERIAL_FLASH == ENABLE)
   if(gmc_FLASH_CLK_DIV_N)
   {
      BYTE FlashClkDivN;
      if(ocm_freq/gmc_FLASH_CLK_DIV_N > 40)
         FlashClkDivN = 5;
      else
         FlashClkDivN = gmc_FLASH_CLK_DIV_N;
      DISABLE_INTERRUPTS;
      //gm_WriteRegWord(SERIAL_FLASH_CTRL2, gmc_FLASH_CLK_DIV_N);
      gm_ClearAndSetBitsWord(SERIAL_FLASH_CTRL2,FLASH_CLK_DIV_N,FlashClkDivN);
      gm_WriteRegWord(CACHE_CTRL2, gm_RegBitShiftValue(CACHE_BURST_SIZE_SERIAL, 4));
      gm_WriteRegWord(CACHE_UPDATE_EN, CACHE_UPDATE_SERIAL_EN);
      ENABLE_INTERRUPTS;
   }
   #ifdef SERIAL_FLASH_CTRL3
   if (gmc_SPI_CE_WIDTH)
   {
      BYTE b = (BYTE)(gmc_OCLK_FREQ/(1000000000L/gmc_SPI_CE_WIDTH));
      gm_WriteRegWord(SERIAL_FLASH_CTRL3, (b>0x1f) ? 0x1f : b);        
   }  
   #endif
#endif 

   DISABLE_INTERRUPTS;
   gm_ProgramRCLKandOCLKandTimer(OcmClkSel, RclkSel, OcmClk);
   gm_UartInit(UART0, OcmClk, 115200L, &uartData);
   ENABLE_INTERRUPTS;

   gm_Print("Set OCM clock as %d MHz",ocm_freq);

}

#endif // FULL_APPSTEST_CMD

//******************************************************************************
//******************************************************************************

#ifdef FULL_APPSTEST_CMD
 #if (FEATURE_PIXEL_CRUNCH == ENABLE)
void TestApplyHighLighWindow(WORD wXStart, WORD wYStart, WORD wWidth, WORD wHeight, BYTE bMode)
{
   UserPrefSaturation = 256;
   CscUpdateColors(gmvb_MainChannel);

 #if (FEATURE_ACM_3D == ENABLE)   
   ACM3DSetColorScheme(VWD_SCHEME);
   gm_ACM3DInit();

   // to inform Library about ACM status
   gm_UpdateACM3DColorScheme(VWD_SCHEME);
 #endif
   
   SetupColorSpace(gmvb_MainChannel);
   gm_CscSetVideoWindow(DEMO_WINDOW_SPLIT);
   
   gm_WriteRegWord(VIDEO_WIN_HSTART, wXStart);
   gm_WriteRegWord(VIDEO_WIN_VSTART, wYStart);
   gm_WriteRegWord(VIDEO_WIN_HWIDTH, wWidth);
   gm_WriteRegWord(VIDEO_WIN_VLENGTH, wHeight);

   gm_SetRegBitsByte(VIDEO_WIN_CTRL, VIDEO_WIN_EN);
   if (bMode == 2)
      gm_ClearAndSetBitsWord(COLOR_MATRIX_CTRL, MAIN_MATRIX_WIN_CTRL, 1 << MAIN_MATRIX_WIN_CTRL_SHIFT);
   else
      gm_ClearAndSetBitsWord(COLOR_MATRIX_CTRL, MAIN_MATRIX_WIN_CTRL, 2 << MAIN_MATRIX_WIN_CTRL_SHIFT);

   if (bMode == 0)
      gm_WriteRegWord(MAIN_S_IN_OFFSET3, 0);
   else
   {
      UserPrefSaturation = 0;
      CscUpdateColors(gmvb_MainChannel);
   }
   
   gm_Delay1ms(10);  // TEST ONLY
   gm_SetRegBitsByte(OVP_UPDATE_CTRL, OVP_FORCE_UPDATE);

 #if (FEATURE_ACC3 == ENABLE)
   if (gm_IsACCEnabled())
      gm_ACC3UpdateModeDependants();
 #endif
}
 #endif // FEATURE_PIXEL_CRUNCH

// Louis 02100723, For Input / Output dynamic capture 
void InputCaptureTest(void)
{
			gmt_WindowStruct InputWinDim, OutputWinDim;
			gmt_WindowStruct PrevInputDim, PrevOutputDim;
			WORD W_OutputLeftBorderWidth;
			WORD W_OutputRightBorderWidth;
			WORD W_OutputTopBorderHeight;
			WORD W_OutputBottomBorderHeight;
			WORD W_PanelWidth = PanelWidth;
			WORD W_PanelHeight = PanelHeight;
			WORD W_PanelBezelWidth = 0;
			WORD W_PanelBezelHeight = 0;

			DWORD DW_TotalOutputW;
			DWORD DW_TotalOutputH;

			//these values are restored at the end of the routine
			ASPECT_RATIOS_MODE TempAspectRatio = UserPrefAspectRatioMain;
			//BYTE TempPCWideMode = UserPrefPCWideMode;

			if((GetParam(0) == 0) || (GetParam(1) == 0) || (GetParam(4) == 0) || (GetParam(5) == 0))
			{
				gm_Print("Wariing ............... Input or Output Width and Height can not set to %d", 0);
				gm_Print("Format: Appstest 8 InputWidth InputHStart InputHeight InputVStart OutputWidth OutputHStart OutputHeight OutputVStart ", 0);
				return;
			}

			#if 0
			if(GetParam(1) < stInput[gmvb_MainChannel].Hstart)
			{
				gm_Print("Wariing ............... H Start should great then %d", stInput[gmvb_MainChannel].Hstart);
				return;
			}

			if(GetParam(3) < stInput[gmvb_MainChannel].Vstart)
			{
				gm_Print("Wariing ............... V Start should great then %d", stInput[gmvb_MainChannel].Vstart);
				return;
			}
			#endif

			OutputWinDim.W_Width = PanelWidth;
			OutputWinDim.W_Hstart = 0;
			OutputWinDim.W_Height = PanelHeight;
			OutputWinDim.W_Vstart = 0;
	#if (FEATURE_PANORAMIC == ENABLE)
			//exit panoramic mode
			if (UserPrefAspectRatioMain == AR_PANORAMIC)
			{

				// RC: Disable blender such that OSD does not disappear when panoramic
				// is disabled.
				gm_BlenderOnOff(gmvb_MainChannel, FALSE);
				gm_ForceUpdate(gmvb_MainChannel, OUTPUT_CLOCK);
				gm_ScalerSetPanoramic(PanelWidth, gmd_PANORAMIC_OFF);
			}
   #endif
			if (!gm_IsValidSignal(gmvb_MainChannel))//bbao July 24, 2006: if no signal ignore the command
				return;
			
			PrevInputDim.W_Hstart = stInput[gmvb_MainChannel].Hstart;
			PrevInputDim.W_Width = stInput[gmvb_MainChannel].Hactive;
			PrevInputDim.W_Vstart = stInput[gmvb_MainChannel].Vstart;
			PrevInputDim.W_Height = stInput[gmvb_MainChannel].Vactive;
			PrevOutputDim.W_Hstart = gmvw_OutputHStart[gmvb_MainChannel];
			PrevOutputDim.W_Width = gmvw_OutputHActive[gmvb_MainChannel];
			PrevOutputDim.W_Vstart = gmvw_OutputVStart[gmvb_MainChannel];
			PrevOutputDim.W_Height = gmvw_OutputVActive[gmvb_MainChannel];
			
			if (gm_IsVideo(gmvb_MainChannel))
			{
				gmt_WindowStruct LocalOutputWin;
				LocalOutputWin.W_Width = PanelWidth;
				LocalOutputWin.W_Height = PanelHeight;

				//following two lines taken from SelectAspectMain()
				B_AspectMethod[gmvb_MainChannel] = UserPrefAspectRatioMain;
				//B_InputAspect[gmvb_MainChannel]  = GetAspectRatio(gmvb_MainChannel);
				SetCapture(gmvb_MainChannel);

				//assuming B_AspectMethod is correct
				//ApplyOutputAspect(&LocalOutputWin, B_AspectMethod[gmvb_MainChannel]);
				ApplyOutputAspect(gmvb_MainChannel, B_AspectMethod[gmvb_MainChannel]);

				gmvw_OutputHActive[gmvb_MainChannel] = LocalOutputWin.W_Width;
				gmvw_OutputVActive[gmvb_MainChannel] = LocalOutputWin.W_Height;
			}
			else
			{
				//UserPrefAspectRatioMain = WMODE_NORMAL;
				//UserPrefPCWideMode = PCMODE_FULL2;//fill the whole screen
				//AdjustPCWideModeInputAspect();
				//AdjustPCWideModeOutputAspect();
				//following two lines taken from SelectAspectMain()
				B_AspectMethod[gmvb_MainChannel] = UserPrefAspectRatioMain;
				//B_InputAspect[gmvb_MainChannel]  = GetAspectRatio(gmvb_MainChannel);
				SetCapture(gmvb_MainChannel);
			}
			InputWinDim.W_Height = stInput[gmvb_MainChannel].Vactive;
			InputWinDim.W_Width = stInput[gmvb_MainChannel].Hactive;
			InputWinDim.W_Hstart = stInput[gmvb_MainChannel].Hstart;
			InputWinDim.W_Vstart = stInput[gmvb_MainChannel].Vstart;
			
			// calculate output size, assuming output window size is determined already
			DW_TotalOutputW = W_PanelWidth + ((DWORD)W_PanelWidth + W_PanelBezelWidth) * (UserPrefXNumber - 1);
			DW_TotalOutputH = W_PanelHeight + ((DWORD)W_PanelHeight + W_PanelBezelHeight) * (UserPrefYNumber - 1);

			//if ((IsCurrentInputPC() && (UserPrefPCWideMode == PCMODE_FULL2))
			//	||(IsCurrentInputVideo() && (UserPrefAspectRatioMain != AR_PILLAR_BOX)))//bbao Aug 19, 2006: only 4:3 widemode displays blank area
			if((gm_IsVideo(gmvb_MainChannel) && (UserPrefAspectRatioMain != AR_PILLAR_BOX)) ||
				(gm_IsGraphics(gmvb_MainChannel) && (UserPrefAspectRatioMain == AR_FULL_SCREEN)))
			{
				// do not care about the aspect ratio and just fill the whole screen
				W_OutputTopBorderHeight = 0;
				W_OutputBottomBorderHeight = 0;
				W_OutputLeftBorderWidth = 0;
				W_OutputRightBorderWidth = 0;
			}
			else if ((DW_TotalOutputW * gmvw_OutputVActive[gmvb_MainChannel]/gmvw_OutputHActive[gmvb_MainChannel])
				> DW_TotalOutputH)
			{	//height is the limiting factor
				W_OutputTopBorderHeight = 0;
				W_OutputBottomBorderHeight = 0;
				W_OutputLeftBorderWidth = (WORD)((DW_TotalOutputW - DW_TotalOutputH * gmvw_OutputHActive[gmvb_MainChannel] / gmvw_OutputVActive[gmvb_MainChannel])/2);
				W_OutputRightBorderWidth = W_OutputLeftBorderWidth;
			}
			else
			{	//width is the limiting factor
				W_OutputLeftBorderWidth = 0;
				W_OutputRightBorderWidth = 0;
				W_OutputTopBorderHeight = (WORD)((DW_TotalOutputH - DW_TotalOutputW * gmvw_OutputVActive[gmvb_MainChannel] / gmvw_OutputHActive[gmvb_MainChannel])/2);
				W_OutputBottomBorderHeight = W_OutputTopBorderHeight;
			}

			#if 0
			MultiDisplaySetup(W_OutputLeftBorderWidth, W_OutputRightBorderWidth, W_OutputTopBorderHeight,
				W_OutputBottomBorderHeight, W_PanelWidth, W_PanelHeight, W_PanelBezelWidth, W_PanelBezelHeight,
				UserPrefXNumber, UserPrefYNumber);
			GetMultiDisplayWinAttrib((UserPrefPublicDisplayNumber-1), &InputWinDim, &OutputWinDim);
			InputWinDim.W_Width = (InputWinDim.W_Width + 1) & ~1;//make sure it is even
			OutputWinDim.W_Hstart &= ~1;//make sure it is even
			OutputWinDim.W_Width = (OutputWinDim.W_Width + 1) & ~1;//make sure it is even
			#endif

			InputWinDim.W_Width = (WORD)GetParam(0);
			InputWinDim.W_Height = (WORD)GetParam(1);
			//InputWinDim.W_Height = (WORD)GetParam(2);
			//InputWinDim.W_Vstart = (WORD)GetParam(3);
			InputWinDim.W_Hstart += (WORD)GetParam(2);
			InputWinDim.W_Vstart += (WORD)GetParam(3);

			OutputWinDim.W_Width = (WORD)GetParam(4);
			OutputWinDim.W_Height = (WORD)GetParam(5);
			OutputWinDim.W_Hstart = (WORD)GetParam(6);
			OutputWinDim.W_Vstart = (WORD)GetParam(7);


			#if 1
			if(UserPrefXNumber > 1)
				{
			InputWinDim.W_Width += 2;
			OutputWinDim.W_Width +=2;
				}
			if(UserPrefYNumber > 1)
				{
			InputWinDim.W_Height+= 1;
			OutputWinDim.W_Height+=2;			
				}
			#endif
			
			// after this line the new input and output dimension is stored in InputWinDim and OutputWinDim
				
/****** ZOOM STARTS ******/
			// This means no image should be shown in this panel
			if ((InputWinDim.W_Width == 0)
				||(InputWinDim.W_Height == 0))
			{
				SetDisplayOff(gmvb_MainChannel, BACKGROUND_ONLY);
				return;
			}

			#if 1
			gm_Print("Input W_Width ........... = %d", InputWinDim.W_Width);
			gm_Print("Input W_Hstart ........... = %d", InputWinDim.W_Hstart);
			gm_Print("Input W_Height ........... = %d", InputWinDim.W_Height);
			gm_Print("Input W_Vstart ........... = %d", InputWinDim.W_Vstart);
			
			gm_Print("Output W_Width ........... = %d", OutputWinDim.W_Width);
			gm_Print("Output W_Hstart ........... = %d", OutputWinDim.W_Hstart);
			gm_Print("Output W_Height ........... = %d", OutputWinDim.W_Height);
			gm_Print("Output W_Vstart ........... = %d", OutputWinDim.W_Vstart);
			
			gm_Print("Prev W_Width ........... = %d", PrevOutputDim.W_Width);
			gm_Print("Prev W_Hstart ........... = %d", PrevOutputDim.W_Hstart);
			gm_Print("Prev W_Height ........... = %d", PrevOutputDim.W_Height);
			gm_Print("Prev W_Vstart ........... = %d", PrevOutputDim.W_Vstart);
			#endif

			// Restore original output size; this is needed for RelocateInputCaptureWindow()
			gmvw_OutputHStart[gmvb_MainChannel] = PrevOutputDim.W_Hstart;
			gmvw_OutputHActive[gmvb_MainChannel] = PrevOutputDim.W_Width;
			gmvw_OutputVStart[gmvb_MainChannel] = PrevOutputDim.W_Vstart;
			gmvw_OutputVActive[gmvb_MainChannel] = PrevOutputDim.W_Height;
			//RelocateInputCaptureWindow(gmvb_MainChannel, &PrevInputDim,
			//				 		&InputWinDim, FALSE);
			RelocateInputCaptureWindow(gmvb_MainChannel, &PrevInputDim,
							 		&InputWinDim);
			
/****** ZOOM ENDS ******/
/****** DYNAMIC SCALING STARTS ******/
			// these 4 lines are needed for programming the scalers
			stInput[gmvb_MainChannel].Vactive = InputWinDim.W_Height;
			stInput[gmvb_MainChannel].Hactive = InputWinDim.W_Width;
			stInput[gmvb_MainChannel].Hstart= InputWinDim.W_Hstart;
			stInput[gmvb_MainChannel].Vstart= InputWinDim.W_Vstart;
			
			// store current output dimension
			SetDynamicScalerSizeBegin(gmvb_MainChannel);

			gmvw_OutputVActive[gmvb_MainChannel] = OutputWinDim.W_Height;
			gmvw_OutputHActive[gmvb_MainChannel] = OutputWinDim.W_Width;
			gmvw_OutputHStart[gmvb_MainChannel] = OutputWinDim.W_Hstart;
			gmvw_OutputVStart[gmvb_MainChannel] = OutputWinDim.W_Vstart;

			//if no change to input-output ratio, skip the second dynamic scaling sequence
			if (((gmvw_OutputHActive[gmvb_MainChannel] * PrevInputDim.W_Width)
				== (stInput[gmvb_MainChannel].Hactive  * PrevOutputDim.W_Width))
				&&
				((gmvw_OutputVActive[gmvb_MainChannel] * PrevInputDim.W_Height)
				== (stInput[gmvb_MainChannel].Vactive  * PrevOutputDim.W_Height))
				&& (gmvw_OutputHStart[gmvb_MainChannel] == PrevOutputDim.W_Hstart)
				&& (gmvw_OutputVStart[gmvb_MainChannel] == PrevOutputDim.W_Vstart)
			)
			{
				DynamicScalerStop();
			}
			else
			{
				// Setup Display
				//using only one step is probably too abrupt for the hardware; artifact is
				//  part of the image go black temporarily
				SetDynamicScalerStep(gmvb_MainChannel, 2);
				SetDynamicScalerStep(gmvb_PipChannel, 0);	// turn off pip scaling
			
				SetDynamicScalerSizeEnd(gmvb_MainChannel);

				DynamicScalerStart(gmvb_MainChannel);
				DynamicScalerProcess(FALSE);		
				DynamicScalerStop();
				
				if (gmvb_MainChannel == CH_A)
				{
				//
				// to disable TNR (TNR_PROGRESSIVE) bits 
				// in case of Vertical Shrink
				//
					if (stInput[gmvb_MainChannel].Vactive > gmvw_OutputVActive[CH_A])	
					{
						gm_VideoProcessingDisable(VIDEO_TNR);
					}
				}
			}
			
			//restore original aspect ratio
			UserPrefAspectRatioMain = TempAspectRatio;
			//UserPrefPCWideMode = TempPCWideMode;

			gm_Print("Input W_Hstart 1 ........... = %d", InputWinDim.W_Hstart);
			gm_WriteRegWord(IMP_H_ACT_START, GetParam(2));
			gm_WriteRegWord(IMP_V_ACT_START_ODD, GetParam(3));
			gm_WriteRegWord(IMP_V_ACT_START_EVEN, GetParam(3) + 1);
			
			// Request Blenders ON and User Prefs
			ModeSetupBlender(gmvb_MainChannel, DISPLAY_DEFER_TMO_NORMAL);

         W_OutputLeftBorderWidth = W_OutputLeftBorderWidth;
			W_OutputRightBorderWidth = W_OutputRightBorderWidth;
			W_OutputTopBorderHeight = W_OutputTopBorderHeight;
			W_OutputBottomBorderHeight = W_OutputBottomBorderHeight;
}

#ifndef NO_PIP_SUPPORT
void InputCaptureTest_PIP(void)
{
			gmt_WindowStruct InputWinDim, OutputWinDim;
			gmt_WindowStruct PrevInputDim, PrevOutputDim;
			WORD W_OutputLeftBorderWidth;
			WORD W_OutputRightBorderWidth;
			WORD W_OutputTopBorderHeight;
			WORD W_OutputBottomBorderHeight;
			WORD W_PanelWidth = PanelWidth;
			WORD W_PanelHeight = PanelHeight;
			WORD W_PanelBezelWidth = 0;
			WORD W_PanelBezelHeight = 0;

			DWORD DW_TotalOutputW;
			DWORD DW_TotalOutputH;

			//these values are restored at the end of the routine
			ASPECT_RATIOS_MODE TempAspectRatio = UserPrefAspectRatioMain;
			//BYTE TempPCWideMode = UserPrefPCWideMode;

			if((GetParam(0) == 0) || (GetParam(1) == 0) || (GetParam(4) == 0) || (GetParam(5) == 0))
			{
				gm_Print("Wariing ............... Input or Output Width and Height can not set to %d", 0);
				gm_Print("Format: Appstest 8 InputWidth InputHStart InputHeight InputVStart OutputWidth OutputHStart OutputHeight OutputVStart ", 0);
				return;
			}

			#if 0
			if(GetParam(1) < stInput[gmvb_MainChannel].Hstart)
			{
				gm_Print("Wariing ............... H Start should great then %d", stInput[gmvb_MainChannel].Hstart);
				return;
			}

			if(GetParam(3) < stInput[gmvb_MainChannel].Vstart)
			{
				gm_Print("Wariing ............... V Start should great then %d", stInput[gmvb_MainChannel].Vstart);
				return;
			}
			#endif

			OutputWinDim.W_Width = PanelWidth;
			OutputWinDim.W_Hstart = 0;
			OutputWinDim.W_Height = PanelHeight;
			OutputWinDim.W_Vstart = 0;
	#if (FEATURE_PANORAMIC == ENABLE)
			//exit panoramic mode
			if (UserPrefAspectRatioMain == AR_PANORAMIC)
			{

				// RC: Disable blender such that OSD does not disappear when panoramic
				// is disabled.
				gm_BlenderOnOff(gmvb_PipChannel, FALSE);
				gm_ForceUpdate(gmvb_PipChannel, OUTPUT_CLOCK);
				gm_ScalerSetPanoramic(PanelWidth, gmd_PANORAMIC_OFF);
			}
   #endif
			if (!gm_IsValidSignal(gmvb_PipChannel))//bbao July 24, 2006: if no signal ignore the command
				return;
			
			PrevInputDim.W_Hstart = stInput[gmvb_PipChannel].Hstart;
			PrevInputDim.W_Width = stInput[gmvb_PipChannel].Hactive;
			PrevInputDim.W_Vstart = stInput[gmvb_PipChannel].Vstart;
			PrevInputDim.W_Height = stInput[gmvb_PipChannel].Vactive;
			PrevOutputDim.W_Hstart = gmvw_OutputHStart[gmvb_PipChannel];
			PrevOutputDim.W_Width = gmvw_OutputHActive[gmvb_PipChannel];
			PrevOutputDim.W_Vstart = gmvw_OutputVStart[gmvb_PipChannel];
			PrevOutputDim.W_Height = gmvw_OutputVActive[gmvb_PipChannel];
			
			if (gm_IsVideo(gmvb_PipChannel))
			{
				gmt_WindowStruct LocalOutputWin;
				LocalOutputWin.W_Width = PanelWidth;
				LocalOutputWin.W_Height = PanelHeight;

				//following two lines taken from SelectAspectMain()
				B_AspectMethod[gmvb_PipChannel] = UserPrefAspectRatioMain;
				//B_InputAspect[gmvb_MainChannel]  = GetAspectRatio(gmvb_MainChannel);
				SetCapture(gmvb_PipChannel);

				//assuming B_AspectMethod is correct
				//ApplyOutputAspect(&LocalOutputWin, B_AspectMethod[gmvb_MainChannel]);
				ApplyOutputAspect(gmvb_PipChannel, B_AspectMethod[gmvb_PipChannel]);

				gmvw_OutputHActive[gmvb_PipChannel] = LocalOutputWin.W_Width;
				gmvw_OutputVActive[gmvb_PipChannel] = LocalOutputWin.W_Height;
			}
			else
			{
				//UserPrefAspectRatioMain = WMODE_NORMAL;
				//UserPrefPCWideMode = PCMODE_FULL2;//fill the whole screen
				//AdjustPCWideModeInputAspect();
				//AdjustPCWideModeOutputAspect();
				//following two lines taken from SelectAspectMain()
				B_AspectMethod[gmvb_PipChannel] = UserPrefAspectRatioMain;
				//B_InputAspect[gmvb_MainChannel]  = GetAspectRatio(gmvb_MainChannel);
				SetCapture(gmvb_PipChannel);
			}
			InputWinDim.W_Height = stInput[gmvb_PipChannel].Vactive;
			InputWinDim.W_Width = stInput[gmvb_PipChannel].Hactive;
			InputWinDim.W_Hstart = stInput[gmvb_PipChannel].Hstart;
			InputWinDim.W_Vstart = stInput[gmvb_PipChannel].Vstart;
			
			// calculate output size, assuming output window size is determined already
			DW_TotalOutputW = W_PanelWidth + ((DWORD)W_PanelWidth + W_PanelBezelWidth) * (UserPrefXNumber - 1);
			DW_TotalOutputH = W_PanelHeight + ((DWORD)W_PanelHeight + W_PanelBezelHeight) * (UserPrefYNumber - 1);

			//if ((IsCurrentInputPC() && (UserPrefPCWideMode == PCMODE_FULL2))
			//	||(IsCurrentInputVideo() && (UserPrefAspectRatioMain != AR_PILLAR_BOX)))//bbao Aug 19, 2006: only 4:3 widemode displays blank area
			if((gm_IsVideo(gmvb_PipChannel) && (UserPrefAspectRatioMain != AR_PILLAR_BOX)) ||
				(gm_IsGraphics(gmvb_PipChannel) && (UserPrefAspectRatioMain == AR_FULL_SCREEN)))
			{
				// do not care about the aspect ratio and just fill the whole screen
				W_OutputTopBorderHeight = 0;
				W_OutputBottomBorderHeight = 0;
				W_OutputLeftBorderWidth = 0;
				W_OutputRightBorderWidth = 0;
			}
			else if ((DW_TotalOutputW * gmvw_OutputVActive[gmvb_PipChannel]/gmvw_OutputHActive[gmvb_PipChannel])
				> DW_TotalOutputH)
			{	//height is the limiting factor
				W_OutputTopBorderHeight = 0;
				W_OutputBottomBorderHeight = 0;
				W_OutputLeftBorderWidth = (WORD)((DW_TotalOutputW - DW_TotalOutputH * gmvw_OutputHActive[gmvb_PipChannel] / gmvw_OutputVActive[gmvb_PipChannel])/2);
				W_OutputRightBorderWidth = W_OutputLeftBorderWidth;
			}
			else
			{	//width is the limiting factor
				W_OutputLeftBorderWidth = 0;
				W_OutputRightBorderWidth = 0;
				W_OutputTopBorderHeight = (WORD)((DW_TotalOutputH - DW_TotalOutputW * gmvw_OutputVActive[gmvb_PipChannel] / gmvw_OutputHActive[gmvb_PipChannel])/2);
				W_OutputBottomBorderHeight = W_OutputTopBorderHeight;
			}

			#if 0
			MultiDisplaySetup(W_OutputLeftBorderWidth, W_OutputRightBorderWidth, W_OutputTopBorderHeight,
				W_OutputBottomBorderHeight, W_PanelWidth, W_PanelHeight, W_PanelBezelWidth, W_PanelBezelHeight,
				UserPrefXNumber, UserPrefYNumber);
			GetMultiDisplayWinAttrib((UserPrefPublicDisplayNumber-1), &InputWinDim, &OutputWinDim);
			InputWinDim.W_Width = (InputWinDim.W_Width + 1) & ~1;//make sure it is even
			OutputWinDim.W_Hstart &= ~1;//make sure it is even
			OutputWinDim.W_Width = (OutputWinDim.W_Width + 1) & ~1;//make sure it is even
			#endif

			InputWinDim.W_Width = (WORD)GetParam(0);
			InputWinDim.W_Height = (WORD)GetParam(1);
			//InputWinDim.W_Height = (WORD)GetParam(2);
			//InputWinDim.W_Vstart = (WORD)GetParam(3);
			InputWinDim.W_Hstart += (WORD)GetParam(2);
			InputWinDim.W_Vstart += (WORD)GetParam(3);

			OutputWinDim.W_Width = (WORD)GetParam(4);
			OutputWinDim.W_Height = (WORD)GetParam(5);
			OutputWinDim.W_Hstart = (WORD)GetParam(6);
			OutputWinDim.W_Vstart = (WORD)GetParam(7);


			#if 1
			if(UserPrefXNumber > 1)
				{
			InputWinDim.W_Width += 2;
			OutputWinDim.W_Width +=2;
				}
			if(UserPrefYNumber > 1)
				{
			InputWinDim.W_Height+= 1;
			OutputWinDim.W_Height+=2;			
				}
			#endif
			
			// after this line the new input and output dimension is stored in InputWinDim and OutputWinDim
				
/****** ZOOM STARTS ******/
			// This means no image should be shown in this panel
			if ((InputWinDim.W_Width == 0)
				||(InputWinDim.W_Height == 0))
			{
				SetDisplayOff(gmvb_PipChannel, BACKGROUND_ONLY);
				return;
			}

			#if 1
			gm_Print("Input W_Width ........... = %d", InputWinDim.W_Width);
			gm_Print("Input W_Hstart ........... = %d", InputWinDim.W_Hstart);
			gm_Print("Input W_Height ........... = %d", InputWinDim.W_Height);
			gm_Print("Input W_Vstart ........... = %d", InputWinDim.W_Vstart);
			
			gm_Print("Output W_Width ........... = %d", OutputWinDim.W_Width);
			gm_Print("Output W_Hstart ........... = %d", OutputWinDim.W_Hstart);
			gm_Print("Output W_Height ........... = %d", OutputWinDim.W_Height);
			gm_Print("Output W_Vstart ........... = %d", OutputWinDim.W_Vstart);
			
			gm_Print("Prev W_Width ........... = %d", PrevOutputDim.W_Width);
			gm_Print("Prev W_Hstart ........... = %d", PrevOutputDim.W_Hstart);
			gm_Print("Prev W_Height ........... = %d", PrevOutputDim.W_Height);
			gm_Print("Prev W_Vstart ........... = %d", PrevOutputDim.W_Vstart);
			#endif

			// Restore original output size; this is needed for RelocateInputCaptureWindow()
			gmvw_OutputHStart[gmvb_PipChannel] = PrevOutputDim.W_Hstart;
			gmvw_OutputHActive[gmvb_PipChannel] = PrevOutputDim.W_Width;
			gmvw_OutputVStart[gmvb_PipChannel] = PrevOutputDim.W_Vstart;
			gmvw_OutputVActive[gmvb_PipChannel] = PrevOutputDim.W_Height;
			//RelocateInputCaptureWindow(gmvb_MainChannel, &PrevInputDim,
			//				 		&InputWinDim, FALSE);
			RelocateInputCaptureWindow(gmvb_PipChannel, &PrevInputDim,
							 		&InputWinDim);
			
/****** ZOOM ENDS ******/
/****** DYNAMIC SCALING STARTS ******/
			// these 4 lines are needed for programming the scalers
			stInput[gmvb_PipChannel].Vactive = InputWinDim.W_Height;
			stInput[gmvb_PipChannel].Hactive = InputWinDim.W_Width;
			stInput[gmvb_PipChannel].Hstart= InputWinDim.W_Hstart;
			stInput[gmvb_PipChannel].Vstart= InputWinDim.W_Vstart;
			
			// store current output dimension
			SetDynamicScalerSizeBegin(gmvb_PipChannel);

			gmvw_OutputVActive[gmvb_PipChannel] = OutputWinDim.W_Height;
			gmvw_OutputHActive[gmvb_PipChannel] = OutputWinDim.W_Width;
			gmvw_OutputHStart[gmvb_PipChannel] = OutputWinDim.W_Hstart;
			gmvw_OutputVStart[gmvb_PipChannel] = OutputWinDim.W_Vstart;

			//if no change to input-output ratio, skip the second dynamic scaling sequence
			if (((gmvw_OutputHActive[gmvb_PipChannel] * PrevInputDim.W_Width)
				== (stInput[gmvb_PipChannel].Hactive  * PrevOutputDim.W_Width))
				&&
				((gmvw_OutputVActive[gmvb_PipChannel] * PrevInputDim.W_Height)
				== (stInput[gmvb_PipChannel].Vactive  * PrevOutputDim.W_Height))
				&& (gmvw_OutputHStart[gmvb_PipChannel] == PrevOutputDim.W_Hstart)
				&& (gmvw_OutputVStart[gmvb_PipChannel] == PrevOutputDim.W_Vstart)
			)
			{
				DynamicScalerStop();
			}
			else
			{
				// Setup Display
				//using only one step is probably too abrupt for the hardware; artifact is
				//  part of the image go black temporarily
				SetDynamicScalerStep(gmvb_PipChannel, 2);
				SetDynamicScalerStep(gmvb_PipChannel, 0);	// turn off pip scaling
			
				SetDynamicScalerSizeEnd(gmvb_PipChannel);

				DynamicScalerStart(gmvb_PipChannel);
				DynamicScalerProcess(FALSE);		
				DynamicScalerStop();
				
				if (gmvb_PipChannel == CH_B)
				{
				//
				// to disable TNR (TNR_PROGRESSIVE) bits 
				// in case of Vertical Shrink
				//
					if (stInput[gmvb_PipChannel].Vactive > gmvw_OutputVActive[CH_B])	
					{
						gm_VideoProcessingDisable(VIDEO_TNR);
					}
				}
			}
			
			//restore original aspect ratio
			UserPrefAspectRatioMain = TempAspectRatio;
			//UserPrefPCWideMode = TempPCWideMode;

			gm_Print("Input W_Hstart 1 ........... = %d", InputWinDim.W_Hstart);
			gm_WriteRegWord(IPP_H_ACT_START, GetParam(2));
			gm_WriteRegWord(IPP_V_ACT_START_ODD, GetParam(3));
			gm_WriteRegWord(IPP_V_ACT_START_EVEN, GetParam(3) + 1);
			
			// Request Blenders ON and User Prefs
			ModeSetupBlender(gmvb_PipChannel, DISPLAY_DEFER_TMO_NORMAL);

         W_OutputLeftBorderWidth = W_OutputLeftBorderWidth;
			W_OutputRightBorderWidth = W_OutputRightBorderWidth;
			W_OutputTopBorderHeight = W_OutputTopBorderHeight;
			W_OutputBottomBorderHeight = W_OutputBottomBorderHeight;	
}

void PipDemoFunc(BYTE Param0, BYTE Param1)
{
   switch (Param0)
   {
      case 1: // position change
         switch (Param1)
         {
            case BOTTOM_RIGHT: // 0
               UserPrefPipPosition = BOTTOM_RIGHT;
               break;
            case TOP_RIGHT: // 1
               UserPrefPipPosition = TOP_RIGHT;
               break;
            case TOP_LEFT: // 2
               UserPrefPipPosition = TOP_LEFT;
               break;
            case BOTTOM_LEFT: // 3
               UserPrefPipPosition = BOTTOM_LEFT;
               break;
            default:
               break;
         }
         AdjustPipMode();
         break;
      case 2: // size change
         switch (Param1)
         {
            case NO_PIP: // 0
               UserPrefPipMode = NO_PIP;
               break;
            case LARGE_PIP: // 1
               UserPrefPipMode = LARGE_PIP;
               break;
            case SMALL_PIP: // 2
               UserPrefPipMode = SMALL_PIP;
               break;
            case SIDE_BY_SIDE:// 3
               UserPrefPipMode = SIDE_BY_SIDE;
               break;
            case SIDE_BY_SIDE_TALL: // 4
               UserPrefPipMode = SIDE_BY_SIDE_TALL;
               break;
            default:
               break;
         }
         AdjustPipMode();
         break;
      case 3: // Alpha blending
         if (Param1>15)
         {
            Param1 = 15;
         }
         gm_BlenderSetLevel(Param1); // PIP_Blend_Level
         gm_SyncUpdate(gmvb_MainChannel, OUTPUT_CLOCK);
         break;
      case 4: // Swap
         SwapMainPip();
         break;
      case 5: // Scaling
         switch (Param1)
         {
            case DOWN:// 0, scaling down
               break;
            case UP: // 1, scaling up
               break;
            case LEFT: // 2, Focus Left Border
               break;
            case RIGHT: // 3, Focus Right Border
               break;
            case SBS_DEFAULT: // 5
               break;
            default:
               break;
         }
         if (UserPrefPipMode != SIDE_BY_SIDE)
         {
            UserPrefPipMode = SIDE_BY_SIDE;
            AdjustPipMode();
         }
         DynamicScalerSideBySide(Param1); // S_Direction
         break;
      case 6: // Auto
         PIP_Index = Param1;
         PIP_Oder = 0;
         gm_TimerStart(BLINKING_LED_TMR, 5);
         PIP_Demo_Handler();
         break;
      case 7:
         SetFlexiblePipWindowLimit(PIP_WINDOW_HORIZONTAL_OFFSET,PIP_WINDOW_VERTICAL_OFFSET,
                                   PanelWidth - PIP_WINDOW_HORIZONTAL_OFFSET,PanelHeight - PIP_WINDOW_VERTICAL_OFFSET);
         SetFlexiblePipWindow((WORD)GetParam(1),(WORD)GetParam(2),(WORD)GetParam(3),(WORD)GetParam(4));
         UserPrefPipMode = FLEXIBLE_PIP;
         AdjustPipMode();
         break;
      case 8:
         PIP_Index = 15;
         PIP_sub = (BYTE)GetParam(1);
         gm_TimerStart(BLINKING_LED_TMR, 5);
         PIP_Demo_Handler();
         break;
      default: // Help
         gm_Print("PIP Demo Function:", 0);
         gm_Print("   appstest 39 <case> <sub-case>", 0);
         gm_Print("    case: ", 0);
         gm_Print("          1 : Position change", 0);
         gm_Print("              0 : BOTTOM_RIGHT", 0);
         gm_Print("              1 : TOP_RIGHT", 0);
         gm_Print("              2 : TOP_LEFT", 0);
         gm_Print("              3 : BOTTOM_LEFT", 0);
         gm_Print("          2 : Size change", 0);
         gm_Print("              0 : NO_PIP", 0);
         gm_Print("              1 : LARGE_PIP", 0);
         gm_Print("              2 : SMALL_PIP", 0);
         gm_Print("              3 : SIDE_BY_SIDE", 0);
         gm_Print("              4 : SIDE_BY_SIDE_TALL", 0);
         gm_Print("          3 : Alpha blending", 0);
         gm_Print("              0 ~ 15 (Blend level)", 0);
         gm_Print("          4 - Swap Main/Pip", 0);
         gm_Print("          5 - Scaling (side by side)", 0);
         gm_Print("              0 : Scaling down", 0);
         gm_Print("              1 : Scaling up", 0);
         gm_Print("              2 : Focus left border", 0);
         gm_Print("              3 : Focus right border", 0);
         gm_Print("          6 : Auto", 0);
         gm_Print("              0 ~ 14 (Start index)", 0);
         gm_Print("          7 : Set Pip position and size", 0);
         gm_Print("              x y width height", 0);
         gm_Print("          8 : Pip Relocating and Resizing", 0);
         gm_Print("              0 : Init", 0);
         gm_Print("              1 : Up", 0);
         gm_Print("              2 : Down", 0);
         gm_Print("              3 : Left", 0);
         gm_Print("              4 : Right", 0);
         gm_Print("              5 : Size up BOTTOM_RIGHT", 0);
         gm_Print("              6 : Size up TOP_LEFT", 0);
         gm_Print("              7 : Size up TOP_RIGHT", 0);
         gm_Print("              8 : Size up BOTTOM_LEFT", 0);
         gm_Print("              9 : Size down BOTTOM_RIGHT", 0);
         gm_Print("             10 : Size down TOP_LEFT", 0);
         gm_Print("             11 : Size down TOP_RIGHT", 0);
         gm_Print("             12 : Size down BOTTOM_LEFT", 0);
         gm_Print("             13 : Stop Relocating and Resizing", 0);
         break;
   }
}

#endif // NO_PIP_SUPPORT

#endif // FULL_APPSTEST_CMD

#else
// Not defined 'USER_EXTENDED_GPROBE_CMD'
// Add these variables to avoid error in Syslib
BYTE gBa_AppsTestParam = 0;
BYTE gBa_AppsTestParamCount = 0;

#endif // USER_EXTENDED_GPROBE_CMD

//******************************************************************************
//  T E M P   D A T A   F O R   O V E R D R I V E
//******************************************************************************

#if (FEATURE_OVERDRIVE == ENABLE)
// Overdrive test
WORD const OD_LUT1[]=
{
0xe010 ,	0x0000 ,	0x1000 ,	0xa198 ,	0x003f ,	0x400a ,	0x0000 ,	0x0100 ,	0x1040 ,	0xfebf ,	0x280a ,	0x0000 ,	0xfd00 ,	0xd007 ,	0xfc7e ,	0x2809 ,
0x0000 ,	0xf500 ,	0x5ff7 ,	0xfb7e ,	0x2409 ,	0x0000 ,	0xee00 ,	0xefd7 ,	0xf97d ,	0x2408 ,	0x0000 ,	0xe900 ,	0x9fbb ,	0xf7bd ,	0x2008 ,	0x0000 ,
0xe500 ,	0x5fa7 ,	0xf67d ,	0x2007 ,	0x0000 ,	0xe000 ,	0x0f97 ,	0xf57d ,	0x1c07 ,	0x0000 ,	0xd700 ,	0x7f83 ,	0xf43c ,	0x1c07 ,	0x0000 ,	0xcf00 ,
0xff5f ,	0xf1fb ,	0x1c06 ,	0x0000 ,	0xc700 ,	0x7f3f ,	0xeffb ,	0x1806 ,	0x0000 ,	0xbf00 ,	0xff1f ,	0xedfa ,	0x1805 ,	0x0000 ,	0xb600 ,	0x6eff ,
0xebfa ,	0x1405 ,	0x0000 ,	0xac00 ,	0xcedb ,	0xe9b9 ,	0x1405 ,	0x0000 ,	0xa100 ,	0x1eb3 ,	0xe739 ,	0x1404 ,	0x0000 ,	0x9400 ,	0x4e87 ,	0xe478 ,
0xd044 ,	0x0101 ,	0x770e ,	0x02cc ,	0x1981 ,	0x1020 ,	0x00a1 ,	0x2004 ,	0x11dc ,	0x0400 ,	0x8011 ,	0x80a0 ,	0x0d02 ,	0xd080 ,	0x007f ,	0x440f ,
0x8090 ,	0x0402 ,	0x5034 ,	0xff7f ,	0x3c0e ,	0x4090 ,	0xfe02 ,	0xe013 ,	0xfd7e ,	0x380d ,	0x4080 ,	0xf902 ,	0x9ffb ,	0xfbbe ,	0x340c ,	0x0080 ,
0xf502 ,	0x5fe7 ,	0xfa7e ,	0x300b ,	0x0070 ,	0xf002 ,	0x0fd7 ,	0xf97e ,	0x2c0a ,	0xc070 ,	0xe701 ,	0x7fc3 ,	0xf83d ,	0x2809 ,	0xc070 ,	0xdf01 ,
0xff9f ,	0xf5fc ,	0x2408 ,	0xc060 ,	0xd701 ,	0x7f7f ,	0xf3fc ,	0x2007 ,	0x8060 ,	0xcf01 ,	0xff5f ,	0xf1fb ,	0x1c07 ,	0x8050 ,	0xc601 ,	0x6f3f ,
0xeffb ,	0x1c06 ,	0x4050 ,	0xbc01 ,	0xcf1b ,	0xedba ,	0x1805 ,	0x4050 ,	0xb101 ,	0x1ef3 ,	0xeb3a ,	0x1404 ,	0x4040 ,	0xa401 ,	0x4ec7 ,	0xe879 ,
0x4868 ,	0x0442 ,	0xa21d ,	0x7338 ,	0x2cc7 ,	0xa044 ,	0x0201 ,	0x6611 ,	0x0288 ,	0x1dc2 ,	0x1030 ,	0x0111 ,	0x3008 ,	0xd198 ,	0x0800 ,	0xc022 ,
0x40f0 ,	0x1404 ,	0x40c0 ,	0x0340 ,	0x8816 ,	0xc0e0 ,	0x0b03 ,	0xe050 ,	0x013f ,	0x5812 ,	0x80d0 ,	0x0603 ,	0x902c ,	0xffbf ,	0x4810 ,	0x40c0 ,
0x0303 ,	0x5018 ,	0xfe7f ,	0x400e ,	0x00b0 ,	0x0003 ,	0x000c ,	0xfd7f ,	0x380e ,	0xc0a0 ,	0xf702 ,	0x7003 ,	0xfc3e ,	0x380c ,	0x8090 ,	0xef02 ,
0xffdf ,	0xf9fd ,	0x300c ,	0x4080 ,	0xe702 ,	0x7fbf ,	0xf7fd ,	0x300b ,	0x0070 ,	0xdf02 ,	0xff9f ,	0xf5fc ,	0x2c0a ,	0xc070 ,	0xd601 ,	0x6f7f ,
0xf3fc ,	0x280a ,	0xc060 ,	0xcc01 ,	0xcf5b ,	0xf1bb ,	0x2808 ,	0x8050 ,	0xc101 ,	0x1f33 ,	0xef3b ,	0x2006 ,	0x4040 ,	0xb401 ,	0x4f07 ,	0xec7a ,
0x9080 ,	0x8682 ,	0xbe24 ,	0x2368 ,	0x338a ,	0x0060 ,	0x0442 ,	0x8f1a ,	0x62f8 ,	0x2886 ,	0x804d ,	0x0301 ,	0x6911 ,	0x023c ,	0x1983 ,	0x3440 ,
0x0221 ,	0x400c ,	0x41a4 ,	0x0c01 ,	0x002d ,	0x8161 ,	0x1f08 ,	0xb100 ,	0x0500 ,	0xb42a ,	0x8120 ,	0x1005 ,	0x607c ,	0x02c0 ,	0xa820 ,	0x8100 ,
0x0c04 ,	0x3040 ,	0x0180 ,	0x801b ,	0x00e0 ,	0x0804 ,	0x0030 ,	0x00c0 ,	0x6c18 ,	0x80e0 ,	0x0303 ,	0x7020 ,	0x003f ,	0x6014 ,	0x80c0 ,	0xff03 ,
0xf00f ,	0xfdfe ,	0x5011 ,	0x00c0 ,	0xf703 ,	0x7fff ,	0xfbfe ,	0x440d ,	0x00b0 ,	0xef03 ,	0xffdf ,	0xf9fd ,	0x340c ,	0xc0a0 ,	0xe602 ,	0x6fbf ,
0xf7fd ,	0x300b ,	0x80a0 ,	0xdc02 ,	0xcf9b ,	0xf5bc ,	0x2c0a ,	0x8080 ,	0xd102 ,	0x1f73 ,	0xf33c ,	0x2809 ,	0x0060 ,	0xc402 ,	0x4f47 ,	0xf07b ,
0xc894 ,	0x0802 ,	0xcd29 ,	0xe390 ,	0x368b ,	0x5078 ,	0x0602 ,	0xaa20 ,	0xf334 ,	0x2f88 ,	0xe068 ,	0x04d1 ,	0x8d18 ,	0x92a8 ,	0x23c6 ,	0xa05b ,
0x4401 ,	0x6e13 ,	0x0234 ,	0x1a44 ,	0x6c50 ,	0x02d1 ,	0x5010 ,	0xf1b8 ,	0x1001 ,	0x4048 ,	0x42a1 ,	0x330b ,	0x0140 ,	0x07c1 ,	0x2041 ,	0x8201 ,
0x1d0a ,	0xc0cc ,	0x0400 ,	0x0438 ,	0x01b1 ,	0x1008 ,	0x8074 ,	0x0300 ,	0xe032 ,	0xc180 ,	0x0b06 ,	0x3040 ,	0x0200 ,	0xc82e ,	0x0140 ,	0x0706 ,
0xf02c ,	0x00ff ,	0xb828 ,	0x0110 ,	0x0305 ,	0x701c ,	0xffff ,	0xa020 ,	0x40d0 ,	0xff04 ,	0xf00f ,	0xfdfe ,	0x8019 ,	0x40c0 ,	0xf603 ,	0x6fff ,
0xfbfe ,	0x6411 ,	0x00b0 ,	0xec03 ,	0xcfdb ,	0xf9bd ,	0x4410 ,	0xc0a0 ,	0xe102 ,	0x1fb3 ,	0xf73d ,	0x400c ,	0x8090 ,	0xd402 ,	0x4f87 ,	0xf47c ,
0xf8a2 ,	0x8942 ,	0xd82c ,	0xd3b4 ,	0x390c ,	0x888a ,	0x0782 ,	0xbe25 ,	0xa360 ,	0x334a ,	0x2880 ,	0x0682 ,	0xa71e ,	0xd2f8 ,	0x2a88 ,	0x0074 ,
0x05b2 ,	0x901a ,	0xe29c ,	0x2346 ,	0xd06a ,	0xc501 ,	0x7816 ,	0x0240 ,	0x1b85 ,	0xa860 ,	0x0481 ,	0x6014 ,	0x31e0 ,	0x1403 ,	0x8059 ,	0x0411 ,
0x4612 ,	0xd180 ,	0x0cc1 ,	0x6454 ,	0x4381 ,	0x2d10 ,	0x0118 ,	0x0741 ,	0x504a ,	0x0321 ,	0x190e ,	0xb0b4 ,	0x0400 ,	0x2846 ,	0x82e1 ,	0x0f0c ,
0x7064 ,	0x02c0 ,	0x183e ,	0x8281 ,	0x0b0b ,	0x303c ,	0x01c0 ,	0xf838 ,	0x0200 ,	0x070a ,	0xf02c ,	0x00ff ,	0xe02d ,	0x0190 ,	0x0308 ,	0x601c ,
0xffff ,	0xb422 ,	0x4110 ,	0xfc06 ,	0xc00f ,	0xfdbe ,	0x881a ,	0x4100 ,	0xf104 ,	0x1ff3 ,	0xfb3e ,	0x6812 ,	0x00c0 ,	0xe404 ,	0x4fc7 ,	0xf87d ,
0x20ac ,	0x8a23 ,	0xe22f ,	0x83d0 ,	0x3b4d ,	0xb09a ,	0x88a2 ,	0xcd28 ,	0xe388 ,	0x360b ,	0x6891 ,	0x8802 ,	0xbc22 ,	0x7334 ,	0x2f8a ,	0x4488 ,
0x0742 ,	0xa920 ,	0x02f0 ,	0x29c9 ,	0x2080 ,	0x06a2 ,	0x961d ,	0x82a4 ,	0x2407 ,	0x0079 ,	0x8602 ,	0x841a ,	0x0258 ,	0x1e06 ,	0xe470 ,	0x0591 ,
0x7018 ,	0x6210 ,	0x1804 ,	0xc069 ,	0x4541 ,	0x5816 ,	0xd1c0 ,	0x1182 ,	0xa463 ,	0x04a1 ,	0x3e15 ,	0x9160 ,	0x0b41 ,	0x8c5e ,	0x8461 ,	0x2312 ,
0xf0f8 ,	0x0640 ,	0x7857 ,	0x83e1 ,	0x1611 ,	0xb08c ,	0x03c0 ,	0x5c4f ,	0x8381 ,	0x0e0f ,	0x7058 ,	0x02c0 ,	0x3c44 ,	0x02d1 ,	0x0a0e ,	0x3038 ,
0x01c0 ,	0x1037 ,	0x4221 ,	0x050b ,	0xc028 ,	0x00ff ,	0xdc2c ,	0x81a0 ,	0x0008 ,	0x1014 ,	0xff3f ,	0xb01a ,	0x8120 ,	0xf406 ,	0x4003 ,	0xfc7e ,
0x3cb6 ,	0x0ac3 ,	0xea32 ,	0x23e4 ,	0x3d0e ,	0xd8a7 ,	0x09a2 ,	0xd92b ,	0xd3a8 ,	0x388c ,	0x9ca0 ,	0x8912 ,	0xcc26 ,	0xc364 ,	0x334b ,	0x809a ,
0x4882 ,	0xbe24 ,	0x9330 ,	0x2f0a ,	0x6893 ,	0x0802 ,	0xb122 ,	0x62f8 ,	0x2a49 ,	0x4c8c ,	0x0792 ,	0xa020 ,	0x42c4 ,	0x2588 ,	0x3088 ,	0x4702 ,
0x911e ,	0x0280 ,	0x2107 ,	0x2080 ,	0x0692 ,	0x801c ,	0x8244 ,	0x1c05 ,	0x007a ,	0x4632 ,	0x681a ,	0xe200 ,	0x1603 ,	0xe874 ,	0xc5e1 ,	0x4f18 ,
0x31a0 ,	0x0f82 ,	0xd06f ,	0x8571 ,	0x3817 ,	0x613c ,	0x08c1 ,	0xbc67 ,	0xc4f1 ,	0x2015 ,	0xe0e0 ,	0x0580 ,	0x9c5b ,	0xc441 ,	0x1213 ,	0xa080 ,
0x0380 ,	0x6c4f ,	0x0371 ,	0x0c11 ,	0x5048 ,	0x0280 ,	0x3c44 ,	0xc2c1 ,	0x070d ,	0x0030 ,	0x0140 ,	0x1028 ,	0x01a1 ,	0x020b ,	0x401c ,	0x003f ,
0x58c0 ,	0xcb63 ,	0xf233 ,	0xa3fc ,	0x3e4e ,	0x00b5 ,	0x8a73 ,	0xe32d ,	0x93c8 ,	0x3a8d ,	0xd4af ,	0xca02 ,	0xd929 ,	0xc38c ,	0x364c ,	0xbcaa ,
0x09a2 ,	0xcf28 ,	0xe364 ,	0x330b ,	0xa8a5 ,	0x8932 ,	0xc426 ,	0x133c ,	0x2f8b ,	0x949f ,	0xc8c2 ,	0xba24 ,	0x0310 ,	0x2c4a ,	0x7c9b ,	0x0882 ,
0xae23 ,	0x12e8 ,	0x2809 ,	0x6c97 ,	0x0802 ,	0x9f22 ,	0x02b8 ,	0x2448 ,	0x5c90 ,	0x07a2 ,	0x9020 ,	0x827c ,	0x2006 ,	0x408a ,	0x8742 ,	0x7c1e ,
0xf240 ,	0x1a04 ,	0x2887 ,	0x06f2 ,	0x661d ,	0x81f0 ,	0x13c3 ,	0x1c7e ,	0xc672 ,	0x4c1b ,	0x0198 ,	0x0e02 ,	0xf874 ,	0xc5b1 ,	0x3119 ,	0x2130 ,
0x0801 ,	0xd06a ,	0xc4f1 ,	0x1916 ,	0xc0c4 ,	0x0480 ,	0xa85f ,	0xc441 ,	0x0e13 ,	0x7064 ,	0x0300 ,	0x7c44 ,	0x0281 ,	0x0911 ,	0x2038 ,	0x01c0 ,
0x70c7 ,	0x8c03 ,	0xf735 ,	0x2438 ,	0x3fcf ,	0x1cc2 ,	0x0b53 ,	0xed30 ,	0x33dc ,	0x3c8e ,	0x08bd ,	0x4af3 ,	0xe42d ,	0x93b4 ,	0x38cd ,	0xf4b9 ,
0xcaa2 ,	0xdc2b ,	0xf390 ,	0x364c ,	0xe4b6 ,	0x8a52 ,	0xd42a ,	0x4370 ,	0x33cc ,	0xd8b0 ,	0x49f2 ,	0xcc29 ,	0xa350 ,	0x310b ,	0xc0ad ,	0xc9b2 ,
0xc527 ,	0xe330 ,	0x2e8a ,	0xb4aa ,	0xc972 ,	0xbb26 ,	0xf314 ,	0x2b89 ,	0xa8a6 ,	0xc902 ,	0xb025 ,	0x02ec ,	0x27c9 ,	0x98a0 ,	0x08a2 ,	0xa024 ,
0xc2c0 ,	0x2407 ,	0x809b ,	0x8872 ,	0x9122 ,	0x6280 ,	0x1f06 ,	0x6c96 ,	0xc7e2 ,	0x7d21 ,	0xc244 ,	0x1984 ,	0x588d ,	0x8742 ,	0x621f ,	0x11f4 ,
0x1303 ,	0x3485 ,	0x06a2 ,	0x431d ,	0x9188 ,	0x0c41 ,	0x1478 ,	0x85f2 ,	0x241a ,	0xe10c ,	0x0640 ,	0xe062 ,	0xc441 ,	0x1017 ,	0x9090 ,	0x0380 ,
0x90d2 ,	0x0c73 ,	0xfd37 ,	0x7478 ,	0x438f ,	0x48cf ,	0xcc23 ,	0xf431 ,	0xd3f4 ,	0x3dce ,	0x3ccb ,	0x8bd3 ,	0xef30 ,	0x43d0 ,	0x3b4e ,	0x2cc7 ,
0x4b93 ,	0xe82f ,	0xc3bc ,	0x390d ,	0x1cc5 ,	0x4b63 ,	0xe22e ,	0x43a0 ,	0x370d ,	0x14c1 ,	0x8b03 ,	0xdb2d ,	0xc388 ,	0x350c ,	0x04bf ,	0x0ad3 ,
0xd62c ,	0x536c ,	0x330c ,	0xfcbc ,	0x4aa2 ,	0xcf2b ,	0xb358 ,	0x314b ,	0xf0b9 ,	0x8a62 ,	0xc62a ,	0x033c ,	0x2ecb ,	0xe4b6 ,	0x8a02 ,	0xbc29 ,
0x0318 ,	0x2c0a ,	0xd8b0 ,	0x09b2 ,	0xb028 ,	0x12f0 ,	0x2809 ,	0xc0ab ,	0xc962 ,	0xa226 ,	0xd2c0 ,	0x2447 ,	0xaca6 ,	0x88d2 ,	0x9225 ,	0x2288 ,
0x1f46 ,	0x989e ,	0x4852 ,	0x7a23 ,	0x3248 ,	0x1884 ,	0x7894 ,	0x4782 ,	0x5a21 ,	0x41e8 ,	0x10c2 ,	0x5084 ,	0x0622 ,	0x351e ,	0x0168 ,	0x0901 ,
0xa0de ,	0x0d23 ,	0x0939 ,	0xd4b9 ,	0x478f ,	0x78db ,	0x8cf3 ,	0xfb34 ,	0x4424 ,	0x3f4f ,	0x6cd8 ,	0xccb3 ,	0xf733 ,	0xf3ec ,	0x3d0e ,	0x60d5 ,
0xcc73 ,	0xf232 ,	0x83dc ,	0x3bce ,	0x54d3 ,	0xcc53 ,	0xee31 ,	0x23c8 ,	0x3a0e ,	0x4cd0 ,	0x4c13 ,	0xe931 ,	0xb3b8 ,	0x388d ,	0x40cf ,	0x4bf3 ,
0xe430 ,	0x63a4 ,	0x36cd ,	0x3ccd ,	0xcbc3 ,	0xdf2f ,	0xf390 ,	0x358c ,	0x34cb ,	0x0b93 ,	0xd92f ,	0x637c ,	0x33cc ,	0x2cc8 ,	0x4b63 ,	0xd22e ,
0xc364 ,	0x318b ,	0x20c5 ,	0x8b03 ,	0xcb2d ,	0x0348 ,	0x2f0b ,	0x14c0 ,	0x0ab3 ,	0xc02c ,	0x232c ,	0x2c0a ,	0x00bb ,	0xca63 ,	0xb42a ,	0x2300 ,
0x2889 ,	0xecb5 ,	0x89e2 ,	0xa329 ,	0xa2d0 ,	0x2487 ,	0xd4ac ,	0x8942 ,	0x9127 ,	0xa28c ,	0x1e85 ,	0xb0a3 ,	0x0842 ,	0x7425 ,	0x5244 ,	0x1683 ,
0xb0e7 ,	0x0de3 ,	0x193a ,	0x94f9 ,	0x4b90 ,	0x9ce5 ,	0x8db3 ,	0x0637 ,	0xb465 ,	0x424f ,	0x94e4 ,	0xcd83 ,	0xfe36 ,	0x7418 ,	0x3ecf ,	0x90e2 ,
0x0d53 ,	0xfb36 ,	0x23f8 ,	0x3dcf ,	0x88e1 ,	0x4d33 ,	0xf835 ,	0xe3ec ,	0x3c8e ,	0x84df ,	0xcd03 ,	0xf534 ,	0x93e0 ,	0x3b8e ,	0x7cde ,	0x0cf3 ,
0xf234 ,	0x43d4 ,	0x3a4e ,	0x78dc ,	0xccd3 ,	0xef33 ,	0xf3c8 ,	0x390d ,	0x70db ,	0x4cb3 ,	0xea33 ,	0x93bc ,	0x37cd ,	0x6cd9 ,	0xcc83 ,	0xe432 ,
0x23a8 ,	0x364d ,	0x64d6 ,	0x0c53 ,	0xde32 ,	0xb390 ,	0x348c ,	0x58d5 ,	0x4c03 ,	0xd831 ,	0x0378 ,	0x32cc ,	0x54d0 ,	0x0bb3 ,	0xd030 ,	0x4360 ,
0x300b ,	0x40cc ,	0xcb53 ,	0xc62e ,	0x3340 ,	0x2d0a ,	0x30c7 ,	0x4ac3 ,	0xba2d ,	0x1318 ,	0x28c9 ,	0x1cbe ,	0x0a33 ,	0xa72b ,	0x42e8 ,	0x2447 ,
0xc8f1 ,	0x0e73 ,	0x293b ,	0x9539 ,	0x4f91 ,	0xc4f1 ,	0xce53 ,	0x1639 ,	0x64a5 ,	0x4650 ,	0xc4f0 ,	0x4e43 ,	0x0d39 ,	0xe459 ,	0x418f ,	0xc0ee ,
0x0e23 ,	0x0739 ,	0xb435 ,	0x3f8f ,	0xb8ed ,	0x8e13 ,	0x0238 ,	0x841d ,	0x3ecf ,	0xb4ed ,	0x4df3 ,	0xfe38 ,	0x5408 ,	0x3e0f ,	0xb4ec ,	0xcde3 ,
0xfc37 ,	0x23f8 ,	0x3d4f ,	0xb0eb ,	0x8dc3 ,	0xfa37 ,	0xf3f0 ,	0x3c8e ,	0xacea ,	0x0db3 ,	0xf737 ,	0xa3e8 ,	0x3bce ,	0xa8e9 ,	0xcd93 ,	0xf536 ,
0x43dc ,	0x3a8e ,	0xa4e7 ,	0x4d63 ,	0xf236 ,	0xe3d4 ,	0x390d ,	0x9ce6 ,	0x8d53 ,	0xed35 ,	0x83c8 ,	0x378d ,	0x98e4 ,	0x4d03 ,	0xe735 ,	0x03b4 ,
0x360d ,	0x90e0 ,	0x0cc3 ,	0xe034 ,	0x639c ,	0x340c ,	0x80dd ,	0x0c73 ,	0xda33 ,	0xa380 ,	0x318b ,	0x74d7 ,	0xcbe3 ,	0xcf31 ,	0x7368 ,	0x2e8a ,
0xf0fc ,	0x8f13 ,	0x393c ,	0x9579 ,	0x5392 ,	0xf0fc ,	0x4f13 ,	0x263c ,	0x64e5 ,	0x4a51 ,	0xf0fb ,	0x4f03 ,	0x1d3c ,	0xd499 ,	0x4590 ,	0xecfb ,
0x0ee3 ,	0x173c ,	0x7475 ,	0x4350 ,	0xecfa ,	0x8ed3 ,	0x123b ,	0x245d ,	0x41d0 ,	0xe8fa ,	0x4ed3 ,	0x0e3b ,	0xe449 ,	0x408f ,	0xe8f9 ,	0x4ec3 ,
0x4ec3 ,	0x0b3b ,	0xc439 ,	0x3f8f ,	0xe4f9 ,	0x0eb3 ,	0x073b ,	0xa42d ,	0x3f0f ,	0xe4f8 ,	0xcea3 ,	0x043a ,	0x741d ,	0x3e8f ,	0xe0f7 ,	0x8e93 ,
0x5411 ,	0x3dcf ,	0xdcf6 ,	0x4e73 ,	0xff3a ,	0x2404 ,	0x3d4f ,	0xd8f5 ,	0xce63 ,	0xfc39 ,	0xd3fc ,	0x3c8e ,	0xd4f4 ,	0x8e43 ,	0xf939 ,	0x73f0 ,
0x3b4e ,	0xd0f3 ,	0x0e03 ,	0xf639 ,	0x03e4 ,	0x39ce ,	0xccf0 ,	0x0dd3 ,	0xf038 ,	0xa3d8 ,	0x380d ,	0xc0ee ,	0x4d73 ,	0xed37 ,	0xf3c0 ,	0x368c ,
0xfcff ,	0x0fc3 ,	0x483f ,	0x95b5 ,	0x5793 ,	0xfcff ,	0x0fc3 ,	0x353f ,	0x6521 ,	0x4e52 ,	0xfcff ,	0x0fb3 ,	0x2c3f ,	0xd4d5 ,	0x4991 ,	0xfcff ,
0xcfb3 ,	0x263e ,	0x74b1 ,	0x4751 ,	0xfcff ,	0xcfa3 ,	0x213e ,	0x2499 ,	0x45d1 ,	0xfcff ,	0x8fa3 ,	0x1d3e ,	0xe485 ,	0x4490 ,	0xfcff ,	0x8f93 ,
0x1a3e ,	0xb475 ,	0x4390 ,	0xfcff ,	0x4f93 ,	0x163e ,	0x7469 ,	0x42d0 ,	0xfcff ,	0x4f83 ,	0x133e ,	0x4459 ,	0x41d0 ,	0xfcff ,	0x0f73 ,	0x103e ,
0x144d ,	0x4110 ,	0xfcff ,	0xcf63 ,	0x0e3d ,	0xf441 ,	0x404f ,	0xfcff ,	0x8f53 ,	0x0b3d ,	0xc439 ,	0x3fcf ,	0xfcff ,	0x4f43 ,	0x083d ,	0x942d ,
0x3f0f ,	0xfcff ,	0x0f33 ,	0x063d ,	0x6421 ,	0x3e4f ,	0xfcff ,	0xcf03 ,	0x043c ,	0x0419 ,	0x3d8f ,	0xfcff ,	0x0ee3 ,	0xff3c ,	0xd410 ,	0x3c0e ,

};

WORD const OD_LUT2[]=
{
0x2008,	0x0000,	0x00  ,	0x2008,	0x0000,	0x00  ,	0x2008,	0x0000,	0x00  ,	0x2008,	0x0000,	0x00  ,	0x2008,	0x0000,	0x00  ,	0x2008,
0x0000,	0x00  ,	0x2008,	0x0000,	0x00  ,	0x2008,	0x0000,	0x00  ,	0x4010,	0x0080,	0x02  ,	0x4010,	0x0080,	0x02  ,	0x4010,	0x0080,
0x02  ,	0x4010,	0x0080,	0x02  ,	0x4010,	0x0080,	0x02  ,	0x4010,	0x0080,	0x02  ,	0x4010,	0x0080,	0x02  ,	0x4010,	0x0080,	0x02  ,
0x6018,	0x0100,	0x04  ,	0x6018,	0x0100,	0x04  ,	0x6018,	0x0100,	0x04  ,	0x6018,	0x0100,	0x04  ,	0x6018,	0x0100,	0x04  ,	0x6018,
0x0100,	0x04  ,	0x6018,	0x0100,	0x04  ,	0x6018,	0x0100,	0x04  ,	0x8020,	0x0180,	0x06  ,	0x8020,	0x0180,	0x06  ,	0x8020,	0x0180,
0x06  ,	0x8020,	0x0180,	0x06  ,	0x8020,	0x0180,	0x06  ,	0x8020,	0x0180,	0x06  ,	0x8020,	0x0180,	0x06  ,	0x8020,	0x0180,	0x06  ,
0xa028,	0x0200,	0x08  ,	0xa028,	0x0200,	0x08  ,	0xa028,	0x0200,	0x08  ,	0xa028,	0x0200,	0x08  ,	0xa028,	0x0200,	0x08  ,	0xa028,
0x0200,	0x08  ,	0xa028,	0x0200,	0x08  ,	0xa028,	0x0200,	0x08  ,	0xc030,	0x0280,	0x0a  ,	0xc030,	0x0280,	0x0a  ,	0xc030,	0x0280,
0x0a  ,	0xc030,	0x0280,	0x0a  ,	0xc030,	0x0280,	0x0a  ,	0xc030,	0x0280,	0x0a  ,	0xc030,	0x0280,	0x0a  ,	0xc030,	0x0280,	0x0a  ,
0xe038,	0x0300,	0x0c  ,	0xe038,	0x0300,	0x0c  ,	0xe038,	0x0300,	0x0c  ,	0xe038,	0x0300,	0x0c  ,	0xe038,	0x0300,	0x0c  ,	0xe038,
0x0300,	0x0c  ,	0xe038,	0x0300,	0x0c  ,	0xe038,	0x0300,	0x0c  ,	0x0040,	0x0381,	0x0e  ,	0x0040,	0x0381,	0x0e  ,	0x0040,	0x0381,
0x0e  ,	0x0040,	0x0381,	0x0e  ,	0x0040,	0x0381,	0x0e  ,	0x0040,	0x0381,	0x0e  ,	0x0040,	0x0381,	0x0e  ,	0x0040,	0x0381,	0x0e  ,
0x2048,	0x0401,	0x10  ,	0x2048,	0x0401,	0x10  ,	0x2048,	0x0401,	0x10  ,	0x2048,	0x0401,	0x10  ,	0x2048,	0x0401,	0x10  ,	0x2048,
0x0401,	0x10  ,	0x2048,	0x0401,	0x10  ,	0x2048,	0x0401,	0x10  ,	0x4050,	0x0481,	0x12  ,	0x4050,	0x0481,	0x12  ,	0x4050,	0x0481,
0x12  ,	0x4050,	0x0481,	0x12  ,	0x4050,	0x0481,	0x12  ,	0x4050,	0x0481,	0x12  ,	0x4050,	0x0481,	0x12  ,	0x4050,	0x0481,	0x12  ,
0x6058,	0x0501,	0x14  ,	0x6058,	0x0501,	0x14  ,	0x6058,	0x0501,	0x14  ,	0x6058,	0x0501,	0x14  ,	0x6058,	0x0501,	0x14  ,	0x6058,
0x0501,	0x14  ,	0x6058,	0x0501,	0x14  ,	0x6058,	0x0501,	0x14  ,	0x8060,	0x0581,	0x16  ,	0x8060,	0x0581,	0x16  ,	0x8060,	0x0581,
0x16  ,	0x8060,	0x0581,	0x16  ,	0x8060,	0x0581,	0x16  ,	0x8060,	0x0581,	0x16  ,	0x8060,	0x0581,	0x16  ,	0x8060,	0x0581,	0x16  ,
0xa068,	0x0601,	0x18  ,	0xa068,	0x0601,	0x18  ,	0xa068,	0x0601,	0x18  ,	0xa068,	0x0601,	0x18  ,	0xa068,	0x0601,	0x18  ,	0xa068,
0x0601,	0x18  ,	0xa068,	0x0601,	0x18  ,	0xa068,	0x0601,	0x18  ,	0xc070,	0x0681,	0x1a  ,	0xc070,	0x0681,	0x1a  ,	0xc070,	0x0681,
0x1a  ,	0xc070,	0x0681,	0x1a  ,	0xc070,	0x0681,	0x1a  ,	0xc070,	0x0681,	0x1a  ,	0xc070,	0x0681,	0x1a  ,	0xc070,	0x0681,	0x1a  ,
0xe078,	0x0701,	0x1c  ,	0xe078,	0x0701,	0x1c  ,	0xe078,	0x0701,	0x1c  ,	0xe078,	0x0701,	0x1c  ,	0xe078,	0x0701,	0x1c  ,	0xe078,
0x0701,	0x1c  ,	0xe078,	0x0701,	0x1c  ,	0xe078,	0x0701,	0x1c  ,	0x0080,	0x0782,	0x1e  ,	0x0080,	0x0782,	0x1e  ,	0x0080,	0x0782,
0x1e  ,	0x0080,	0x0782,	0x1e  ,	0x0080,	0x0782,	0x1e  ,	0x0080,	0x0782,	0x1e  ,	0x0080,	0x0782,	0x1e  ,	0x0080,	0x0782,	0x1e  ,
0x2088,	0x0802,	0x20  ,	0x2088,	0x0802,	0x20  ,	0x2088,	0x0802,	0x20  ,	0x2088,	0x0802,	0x20  ,	0x2088,	0x0802,	0x20  ,	0x2088,
0x0802,	0x20  ,	0x2088,	0x0802,	0x20  ,	0x2088,	0x0802,	0x20  ,	0x4090,	0x0882,	0x22  ,	0x4090,	0x0882,	0x22  ,	0x4090,	0x0882,
0x22  ,	0x4090,	0x0882,	0x22  ,	0x4090,	0x0882,	0x22  ,	0x4090,	0x0882,	0x22  ,	0x4090,	0x0882,	0x22  ,	0x4090,	0x0882,	0x22  ,
0x6098,	0x0902,	0x24  ,	0x6098,	0x0902,	0x24  ,	0x6098,	0x0902,	0x24  ,	0x6098,	0x0902,	0x24  ,	0x6098,	0x0902,	0x24  ,	0x6098,
0x0902,	0x24  ,	0x6098,	0x0902,	0x24  ,	0x6098,	0x0902,	0x24  ,	0x80a0,	0x0982,	0x26  ,	0x80a0,	0x0982,	0x26  ,	0x80a0,	0x0982,
0x26  ,	0x80a0,	0x0982,	0x26  ,	0x80a0,	0x0982,	0x26  ,	0x80a0,	0x0982,	0x26  ,	0x80a0,	0x0982,	0x26  ,	0x80a0,	0x0982,	0x26  ,
0xa0a8,	0x0a02,	0x28  ,	0xa0a8,	0x0a02,	0x28  ,	0xa0a8,	0x0a02,	0x28  ,	0xa0a8,	0x0a02,	0x28  ,	0xa0a8,	0x0a02,	0x28  ,	0xa0a8,
0x0a02,	0x28  ,	0xa0a8,	0x0a02,	0x28  ,	0xa0a8,	0x0a02,	0x28  ,	0xc0b0,	0x0a82,	0x2a  ,	0xc0b0,	0x0a82,	0x2a  ,	0xc0b0,	0x0a82,
0x2a  ,	0xc0b0,	0x0a82,	0x2a  ,	0xc0b0,	0x0a82,	0x2a  ,	0xc0b0,	0x0a82,	0x2a  ,	0xc0b0,	0x0a82,	0x2a  ,	0xc0b0,	0x0a82,	0x2a  ,
0xe0b8,	0x0b02,	0x2c  ,	0xe0b8,	0x0b02,	0x2c  ,	0xe0b8,	0x0b02,	0x2c  ,	0xe0b8,	0x0b02,	0x2c  ,	0xe0b8,	0x0b02,	0x2c  ,	0xe0b8,
0x0b02,	0x2c  ,	0xe0b8,	0x0b02,	0x2c  ,	0xe0b8,	0x0b02,	0x2c  ,	0x00c0,	0x0b83,	0x2e  ,	0x00c0,	0x0b83,	0x2e  ,	0x00c0,	0x0b83,
0x2e  ,	0x00c0,	0x0b83,	0x2e  ,	0x00c0,	0x0b83,	0x2e  ,	0x00c0,	0x0b83,	0x2e  ,	0x00c0,	0x0b83,	0x2e  ,	0x00c0,	0x0b83,	0x2e  ,
0x20c8,	0x0c03,	0x30  ,	0x20c8,	0x0c03,	0x30  ,	0x20c8,	0x0c03,	0x30  ,	0x20c8,	0x0c03,	0x30  ,	0x20c8,	0x0c03,	0x30  ,	0x20c8,
0x0c03,	0x30  ,	0x20c8,	0x0c03,	0x30  ,	0x20c8,	0x0c03,	0x30  ,	0x40d0,	0x0c83,	0x32  ,	0x40d0,	0x0c83,	0x32  ,	0x40d0,	0x0c83,
0x32  ,	0x40d0,	0x0c83,	0x32  ,	0x40d0,	0x0c83,	0x32  ,	0x40d0,	0x0c83,	0x32  ,	0x40d0,	0x0c83,	0x32  ,	0x40d0,	0x0c83,	0x32  ,
0x60d8,	0x0d03,	0x34  ,	0x60d8,	0x0d03,	0x34  ,	0x60d8,	0x0d03,	0x34  ,	0x60d8,	0x0d03,	0x34  ,	0x60d8,	0x0d03,	0x34  ,	0x60d8,
0x0d03,	0x34  ,	0x60d8,	0x0d03,	0x34  ,	0x60d8,	0x0d03,	0x34  ,	0x80e0,	0x0d83,	0x36  ,	0x80e0,	0x0d83,	0x36  ,	0x80e0,	0x0d83,
0x36  ,	0x80e0,	0x0d83,	0x36  ,	0x80e0,	0x0d83,	0x36  ,	0x80e0,	0x0d83,	0x36  ,	0x80e0,	0x0d83,	0x36  ,	0x80e0,	0x0d83,	0x36  ,
0xa0e8,	0x0e03,	0x38  ,	0xa0e8,	0x0e03,	0x38  ,	0xa0e8,	0x0e03,	0x38  ,	0xa0e8,	0x0e03,	0x38  ,	0xa0e8,	0x0e03,	0x38  ,	0xa0e8,
0x0e03,	0x38  ,	0xa0e8,	0x0e03,	0x38  ,	0xa0e8,	0x0e03,	0x38  ,	0xc0f0,	0x0e83,	0x3a  ,	0xc0f0,	0x0e83,	0x3a  ,	0xc0f0,	0x0e83,
0x3a  ,	0xc0f0,	0x0e83,	0x3a  ,	0xc0f0,	0x0e83,	0x3a  ,	0xc0f0,	0x0e83,	0x3a  ,	0xc0f0,	0x0e83,	0x3a  ,	0xc0f0,	0x0e83,	0x3a  ,
0xe0f8,	0x0f03,	0x3c  ,	0xe0f8,	0x0f03,	0x3c  ,	0xe0f8,	0x0f03,	0x3c  ,	0xe0f8,	0x0f03,	0x3c  ,	0xe0f8,	0x0f03,	0x3c  ,	0xe0f8,
0x0f03,	0x3c  ,	0xe0f8,	0x0f03,	0x3c  ,	0xe0f8,	0x0f03,	0x3c  ,	0xfcff,	0x0f83,	0x3e  ,	0xfcff,	0x0f83,	0x3e  ,	0xfcff,	0x0f83,
0x3e  ,	0xfcff,	0x0f83,	0x3e  ,	0xfcff,	0x0f83,	0x3e  ,	0xfcff,	0x0f83,	0x3e  ,	0xfcff,	0x0f83,	0x3e  ,	0xfcff,	0x0f83,	0x3e  ,
};
#endif // (FEATURE_OVERDRIVE == ENABLE)

//*********************************  END  **************************************
