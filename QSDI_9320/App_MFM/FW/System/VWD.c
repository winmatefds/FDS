//******************************************************************************
//  I N C L U D E    F I L E S
//******************************************************************************
#include <stdLib.h>
#include <math.h>
#include "System\All.h"
#include "System\VWD.h"
#include "StdLibInc\gm_ACC3.h"
#include "stdlibinc\PIXCRMain.h"

//******************************************************************************
//  E X T E R N A L    F U N C T I O N S
//******************************************************************************

//******************************************************************************
//  E X T E R N A L    V A R I A B L E S
//******************************************************************************

//******************************************************************************
//  D E F I N E S
//******************************************************************************
#define _DEBUG_MSG
#ifdef _DEBUG_MSG
#define msg(x,y) gm_Print(x,y)
#else
#define msg(x,y)
#endif

#if (FEATURE_VWD == ENABLE)

#define VWD_CHANGE_THRD    3

#define VWD_HIGHLIGHT_DEMO

#define BLACK_BORDER_FIX   0        // Fix the detection with black borders in desktop with dither.

//******************************************************************************
//  S T A T I C    V A R I A B L E S
//******************************************************************************
static gmt_PCInputConfig VwdConfig;
static gmt_VideoInfo VwdDetectInfo, VwdCoarseInfo;
static gmt_MultiVideoInfo CoarseMVWInfo;

static BYTE VWDCurrState = UNKNOWN_IMAGE;

static BYTE MVWON;
static BYTE SelectRectext;
static BYTE ExtSelectWR;

static BOOL VwdEnable = FALSE;
static WORD VwdXStart = 0, VwdYStart = 0, VwdXEnd = 0, VwdYEnd = 0;
#ifdef _DEBUG_MSG
static DWORD VwdRunTime;
#endif

#ifdef VWD_HIGHLIGHT_DEMO
static WORD VwdWinTransitCnt = 0;
#endif

#ifndef UserPrefVwdDemo
BYTE VwdDemoTemp = VWD_DEMO_FULL;  // Temp declair
#define UserPrefVwdDemo VwdDemoTemp
#endif

//*****************************************************************************
//  S T A T I C    F U N C T I O N    P R O T O T Y P E S
//*****************************************************************************
extern void PixCrCoarseRunEdge_Test(void);
extern void PixCrCoarseRunAcc_Test(void);
extern void PixCrCoarseRun_Test(void);
extern BOOL gm_IsAfrSet(gmt_PHY_CH Channel);

#if (FEATURE_ACM_3D == ENABLE)
extern void gm_ACM3DInit( void );
#endif

//*****************************************************************************
//  C O D E
//*****************************************************************************

//*****************************************************************************
//  Vwd Initial
//*****************************************************************************
void VWD_Initial(void)
{
   VwdConfig.DEBUGONOFF      = 1;   // 1:use following settings , 0:use default settings

   VwdConfig.BLACKONOFF      = 0;
   VwdConfig.BORDERLESSONOFF = 1 ;   // 0: Border mode , 1: Borderless mode

   VwdConfig.COA_EDGEHWTHR   = 34;
   VwdConfig.COA_EDGEDIVFACT = 20;
   VwdConfig.COA_MOTIONTHR   = 0;
   VwdConfig.COA_MOTIONGATE  = 0;
   VwdConfig.COA_REALTHR     = 70;
   VwdConfig.COA_REALGATE    = 0;
   VwdConfig.COA_BLACKTHR    = 8;
   VwdConfig.COA_FINALGATEMOTION = 0;
   VwdConfig.COA_FINALGATE   = 0;
   VwdConfig.COA_FINALMAPSEL = 0;
   VwdConfig.COA_RECTWIDTH   = 5;
   VwdConfig.COA_RECTHEIGHT  = 5;
   VwdConfig.FIN_MOTIONTHR   = 0;
   VwdConfig.FIN_BLACKTHR    = 12;
   // BLACK_BORDER_FIX
   VwdConfig.FORCE_DITHER = FALSE;
   VwdDetectInfo.VideoType = UNKNOWN_IMAGE;
   
   SelectRectext = 0;
   ExtSelectWR   = 0;

   gm_Print("VWD Gen3: May, 2012",0);
}

//*****************************************************************************
//  Clear VWD window information
//*****************************************************************************
void VWD_ClearWindowsInfo(void)
{
   VwdXStart = 0;
   VwdXEnd   = 0;
   VwdYStart = 0;
   VwdYEnd   = 0;

   VwdDetectInfo.VideoType = UNKNOWN_IMAGE;
}

//*****************************************************************************
//  Is VWD Detected
//*****************************************************************************
BOOL IsVwdDetected(void)
{
   if (VwdDetectInfo.VideoType < UNKNOWN_IMAGE)
      return TRUE;
   else
      return FALSE;
}

//*****************************************************************************
//*****************************************************************************
void Vwd_Start_Test(BYTE TestMode)
{
   PixCrAnalyzeImage(&VwdDetectInfo, &VwdCoarseInfo,
                     START_ANALYZE, VwdConfig,
                     &VWDCurrState, &CoarseMVWInfo,
                     &MVWON, SelectRectext, ExtSelectWR);

   if (TestMode == 0)
      PixCrCoarseRunEdge_Test();      // EDGE
   else if (TestMode == 1)
      PixCrCoarseRun_Test();       // REALNESS
   else // if (TestMode == 2)
      PixCrCoarseRunAcc_Test();
}
#ifdef VWD_HIGHLIGHT_DEMO
//*****************************************************************************
//  Vwd Highlight window
//*****************************************************************************
void VWD_WindowEnhance(void)
{
   WORD wTransVal;

   if ((UserPrefVwdDemo != VWD_DEMO_HIGHLIGHT) || (IsVwdDetected() == FALSE) || (VwdWinTransitCnt == 0))
      return;

   wTransVal = (WORD)((DWORD)VwdWinTransitCnt * 256 / VWD_HIGHLIGHT_DEMO_COUNT);
   UserPrefSaturation = wTransVal;

   CscUpdateColors(gmvb_MainChannel);

   VwdWinTransitCnt--;
}
#endif

//*****************************************************************************
//  Vwd Start
//*****************************************************************************
void VWD_Switch(void)
{
   if (UserPrefVwdFlag == ON)
      VWD_Start();
   else
   {
      VWD_Stop();
      PixCrDisable();
   }
}

//*****************************************************************************
//  Vwd Start
//*****************************************************************************
void VWD_Start(void)
{
#if BLACK_BORDER_FIX
   WORD W_DE_WIDTH = gm_ReadRegWord(IMP_IBD_HWIDTH);
   WORD W_DE_HSTART = gm_ReadRegWord(IMP_IBD_HSTART);
   WORD W_DE_HEIGHT = gm_ReadRegWord(IMP_IBD_VLENGTH);
   WORD W_DE_VSTART = gm_ReadRegWord(IMP_IBD_VSTART);
#endif

   if ((UserPrefVwdFlag == OFF) || (VwdEnable == TRUE))
      return;

#if (FEATURE_ACR == ENABLE)
   if (UserPrefDCR == ON)
      PixCrAnalyzeACRStop();
#endif

   // Only for digital ports
   if (IsInputDigital(gmvb_MainChannel) == FALSE)
      return;

   msg("PixCR Analysis - Start",0);

#if BLACK_BORDER_FIX
   // Check input image with black border or not.
   gm_ClearRegBitsWord(IMP_IBD_CONTROL, IMP_MEASURE_DE_nDATA);
   gm_Delay1ms(50);   
   {
      WORD W_DATA_WIDTH = gm_ReadRegWord(IMP_IBD_HWIDTH);
      WORD W_DATA_HSTART = gm_ReadRegWord(IMP_IBD_HSTART);
      WORD W_DATA_HEIGHT = gm_ReadRegWord(IMP_IBD_VLENGTH);
      WORD W_DATA_VSTART = gm_ReadRegWord(IMP_IBD_VSTART);
      WORD W_HSTART_DIFF = abs(W_DATA_HSTART - W_DE_HSTART);
      WORD W_VSTART_DIFF = abs(W_DATA_VSTART - W_DE_VSTART);

      gm_SetRegBitsWord(IMP_IBD_CONTROL, IMP_MEASURE_DE_nDATA);

      msg("W_DE_WIDTH === %d",W_DE_WIDTH);
      msg("W_DATA_WIDTH === %d",W_DATA_WIDTH);
      
      if (((W_HSTART_DIFF >= (W_DE_WIDTH)/32) && (abs(W_DATA_WIDTH - W_DE_WIDTH) >= (W_HSTART_DIFF * 2)))
            || ((W_VSTART_DIFF >= (W_DE_HEIGHT)/30) && (abs(W_DATA_HEIGHT - W_DE_HEIGHT) >= (W_VSTART_DIFF * 2))))
      {
         msg("  Shrink mode --- Force Dither mode",0);
         VwdConfig.FORCE_DITHER = TRUE;
         gm_TimerStart(VWD_DITHER_CHECK_100ms_TMR, 300); // Force DITHER for 30 sec
      }
      else
         VwdConfig.FORCE_DITHER = FALSE;
   }
#endif
   
   PixCrAnalyzeImage(&VwdDetectInfo, &VwdCoarseInfo,
                     START_ANALYZE, VwdConfig,
                     &VWDCurrState, &CoarseMVWInfo,
                     &MVWON, SelectRectext, ExtSelectWR);
#ifdef _DEBUG_MSG
   VwdRunTime = gm_GetSystemTime();
#endif
   ExtSelectWR = 0;
   VwdEnable = TRUE;
}

//*****************************************************************************
//  VWD Stop
//*****************************************************************************
void VWD_Stop(void)
{
   if (VwdEnable == FALSE)
      return;

   msg("PixCR Analysis - Stop",0);

   VWD_ClearHighlighWindow();
   VWD_ClearWindowsInfo();

#if BLACK_BORDER_FIX
   VwdConfig.FORCE_DITHER = FALSE;
   gm_TimerStop(VWD_DITHER_CHECK_100ms_TMR);
#endif

   PixCrAnalyzeImage(&VwdDetectInfo, &VwdCoarseInfo,
                     STOP_ANALYZE, VwdConfig,
                     &VWDCurrState, &CoarseMVWInfo,
                     &MVWON, SelectRectext, ExtSelectWR);

   VwdEnable = FALSE;
}

//*****************************************************************************
//  Is VWD Enabled
//*****************************************************************************
BOOL IsVWDEnabled(void)
{
   return VwdEnable;
}

//*****************************************************************************
//  VWD_Handler
//*****************************************************************************
void VWD_Handler(void)
{
#if (FEATURE_AUDIO == ENABLE)
   if (SYS_API_AUDIO_Is_DAC_VolumeSetupDone() != TRUE)
   {
      // let audio volume control is finished and then start to deal with VWD feature
      return;
   }
#endif

   if ((UserPrefVwdFlag == OFF) || (!IsVWDEnabled()))
      return;

   if (gm_IsAfrSet(gmvb_MainChannel))
      return;

   VWD_Running();

#if BLACK_BORDER_FIX
   if (gm_TimerCheck(VWD_DITHER_CHECK_100ms_TMR) == TIMER_TMO)
   {
      if (IsVwdDetected() == FALSE)
      {
         VwdConfig.FORCE_DITHER = FALSE;
         //msg("  FORCE_DITHER -- OFF",0);
      }
   }
#endif

#ifdef VWD_HIGHLIGHT_DEMO
   VWD_WindowEnhance();
#endif
}

//*****************************************************************************
//  Vwd Running
//*****************************************************************************
void VWD_Running(void)
{
   static WORD prevCoarseXStart = 0, prevCoarseYStart = 0, prevCoarseXEnd = 0, prevCoarseYEnd = 0;

   gmt_VideoType PrevVideoType;
   BYTE prevVwdState;
   BYTE RectCount = 0;
   BYTE sloop;

   SelectRectext = 0;

   prevVwdState = VWDCurrState;
   PrevVideoType = VwdDetectInfo.VideoType;

   PixCrAnalyzeImage(&VwdDetectInfo, &VwdCoarseInfo, POLL_ANALYZE, VwdConfig,
                     &VWDCurrState, &CoarseMVWInfo, &MVWON, SelectRectext, ExtSelectWR);

   if ((VwdDetectInfo.VideoType >= UNKNOWN_IMAGE) && (PrevVideoType <= SQUARE_IMAGE))
   {
      msg("VWD Lost!",0);
      VWD_ClearWindowsInfo();
      VWD_ClearHighlighWindow();
      
   #if BLACK_BORDER_FIX
      if ((VwdConfig.FORCE_DITHER) && (gm_TimerCheck(VWD_DITHER_CHECK_100ms_TMR) != TIMER_OK))
      {
         VwdConfig.FORCE_DITHER = FALSE;
         msg("  FORCE_DITHER -- OFF",0);
      }
   #endif
   
#ifdef _DEBUG_MSG
      VwdRunTime = gm_GetSystemTime();
#endif
      return;
   }

#if 1
   if ((prevVwdState == COARSE_DETECTION) && (VWDCurrState == COARSE_MVWSELECT) && (MVWON == 1))
   {
      for (sloop = 4; sloop >0; sloop--)
      {
         if (((CoarseMVWInfo.Rect[sloop-1].VideoType == PERFECT_RECTFOUND)
               || (CoarseMVWInfo.Rect[sloop-1].VideoType == CUT_RECTFOUND)))
         {
            RectCount++;
            SelectRectext = sloop - 1;

            msg(" Highlight - %d",SelectRectext);
         }
      }
   }
#endif

   if (RectCount != 0)
      ExtSelectWR = 1;
   else
      ExtSelectWR = 0;

   if (VwdDetectInfo.VideoType >= UNKNOWN_IMAGE)
      return;

   if (((prevCoarseXStart != VwdCoarseInfo.XStart)
         || (prevCoarseXEnd   != VwdCoarseInfo.XEnd)
         || (prevCoarseYStart  != VwdCoarseInfo.YStart)
         || (prevCoarseYEnd != VwdCoarseInfo.YEnd)))
   {
      prevCoarseXStart = VwdCoarseInfo.XStart;
      prevCoarseXEnd  = VwdCoarseInfo.XEnd;
      prevCoarseYStart = VwdCoarseInfo.YStart;
      prevCoarseYEnd = VwdCoarseInfo.YEnd;

      msg("VWD COARSE Detected <%d>",VwdDetectInfo.VideoType);
      msg("  XStart = %d", prevCoarseXStart);
      msg("  XEnd   = %d", prevCoarseXEnd);
      msg("  YStart = %d", prevCoarseYStart);
      msg("  YEnd   = %d", prevCoarseYEnd);
   }

   if ((abs(VwdDetectInfo.XStart - VwdXStart) > VWD_CHANGE_THRD) ||
         (abs(VwdDetectInfo.XEnd - VwdXEnd) > VWD_CHANGE_THRD) ||
         (abs(VwdDetectInfo.YStart - VwdYStart) > VWD_CHANGE_THRD) ||
         (abs(VwdDetectInfo.YEnd - VwdYEnd) > VWD_CHANGE_THRD))
   {
      VwdXStart = VwdDetectInfo.XStart;
      VwdXEnd = VwdDetectInfo.XEnd;
      VwdYStart = VwdDetectInfo.YStart;
      VwdYEnd = VwdDetectInfo.YEnd;
#ifdef _DEBUG_MSG
      msg("VWD FINE Detected <%d>",VwdDetectInfo.VideoType);
      msg("  XStart = %d", VwdXStart);
      msg("  XEnd   = %d", VwdXEnd);
      msg("  YStart = %d", VwdYStart);
      msg("  YEnd   = %d", VwdYEnd);
      msg("< Run Time :: %d ms >", gm_GetSystemTime() - VwdRunTime);
#endif
      VWD_ApplyHighLightWindow();
   }

}

//*****************************************************************************
//  VWD_ApplyHighLightWindow
//*****************************************************************************
void VWD_ApplyHighLightWindow(void)
{
   WORD wXStart, wWidth, wYStart, wHeight;
   WORD inputWidth, inputHeight, outputWidth, outputHeight;

   if (IsVwdDetected() == FALSE)
      return;
   
   inputWidth   = stInput[gmvb_MainChannel].Hactive;
   inputHeight  = stInput[gmvb_MainChannel].Vactive;
   outputWidth  = gmvw_OutputHActive[gmvb_MainChannel];
   outputHeight = gmvw_OutputVActive[gmvb_MainChannel];

   // Increase by 2 to improve slight extension of VWD on Top & left edge of the window.
   wXStart = (WORD)(((DWORD)(VwdXStart + 2) * (DWORD)outputWidth + (DWORD)(inputWidth/2))/(DWORD)inputWidth);
   wWidth = (WORD)(((DWORD)VwdXEnd * (DWORD)outputWidth + (DWORD)(inputWidth/2))/(DWORD)inputWidth) - wXStart;
   wYStart = (WORD)(((DWORD)(VwdYStart + 2) * (DWORD)outputHeight + (DWORD)(inputHeight/2))/(DWORD)inputHeight);
   wHeight = (WORD)(((DWORD)VwdYEnd * (DWORD)outputHeight + (DWORD)(inputHeight/2))/(DWORD)inputHeight) - wYStart;

   if ((wWidth == 0) || (wHeight == 0))
      return;

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

   gm_Print(" VWD - Applying",0);
   gm_Print("   XStart = %d",wXStart);
   gm_Print("   Width  = %d",wWidth);
   gm_Print("   YStart = %d",wYStart);
   gm_Print("   Height = %d",wHeight);

   if (UserPrefVwdDemo == VWD_DEMO_RIGHT)
   {
      wXStart = wXStart + (wWidth / 2);
      wWidth = wWidth / 2;
   }
   else if (UserPrefVwdDemo == VWD_DEMO_LEFT)
   {
      wWidth = wWidth / 2;
   }
   else //if (UserPrefVwdDemo == VWD_DEMO_FULL)
   {
    #ifdef VWD_HIGHLIGHT_DEMO
      if (UserPrefVwdDemo == VWD_DEMO_HIGHLIGHT)
         VwdWinTransitCnt = VWD_HIGHLIGHT_DEMO_COUNT;
    #endif
   }

   gm_WriteRegWord(VIDEO_WIN_HSTART, wXStart);
   gm_WriteRegWord(VIDEO_WIN_VSTART, wYStart);
   gm_WriteRegWord(VIDEO_WIN_HWIDTH, wWidth);
   gm_WriteRegWord(VIDEO_WIN_VLENGTH, wHeight);

   SetWindowSharpness(0x7F7F, wXStart, (wXStart + wWidth), wYStart, (wYStart + wHeight), FALSE);
   
   gm_SetRegBitsByte(VIDEO_WIN_CTRL, VIDEO_WIN_EN);
   gm_ClearAndSetBitsWord(COLOR_MATRIX_CTRL, MAIN_MATRIX_WIN_CTRL, SEC_IN_PRI_OUT_WIN << MAIN_MATRIX_WIN_CTRL_SHIFT);

   gm_Delay1ms(10);  // TEST ONLY
   gm_SetRegBitsByte(OVP_UPDATE_CTRL, OVP_FORCE_UPDATE);
   gm_SyncUpdateWait(gmvb_MainChannel,INPUT_OUTPUT_CLOCK);

#if (FEATURE_ACC3 == ENABLE)
   if (gm_IsACCEnabled())
      gm_ACC3UpdateModeDependants();
#endif
}

void VWD_ClearHighlighWindow(void)
{
   UserPrefSaturation = 256;
   CscUpdateColors(gmvb_MainChannel);
   gm_CscSetVideoWindow(DEMO_WINDOW_SPLIT);
   gm_ClearAndSetBitsWord(COLOR_MATRIX_CTRL, MAIN_MATRIX_WIN_CTRL, PRI_IN_SEC_OUT_WIN << MAIN_MATRIX_WIN_CTRL_SHIFT);

   gm_WriteRegWord (VIDEO_WIN_CTRL, 0);
   gm_WriteRegWord (VIDEO_WIN_HWIDTH, 0);
   gm_WriteRegWord (VIDEO_WIN_VLENGTH, 0);

   // Clear Demo Sharpness
   gm_ClearRegBitsWord(MHF_DEMOWIND_CTRL_0, WINDOW_SHARP_EN);
   gm_ClearRegBitsWord(MVF_DEMOWIND_CTRL_0, WINDOW_SHARP_EN);
   

   gm_SetRegBitsByte(OVP_UPDATE_CTRL, OVP_FORCE_UPDATE);
   gm_SyncUpdateWait(gmvb_MainChannel, INPUT_OUTPUT_CLOCK);
}
#else
void VWD_Switch(void) {}
void VWD_ApplyHighLightWindow(void) {}

#endif
