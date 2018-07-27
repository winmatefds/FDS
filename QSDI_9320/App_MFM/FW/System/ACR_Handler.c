/*
	$Workfile:   ACR_Handler.c  $
	$Revision: 1.24 $
	$Date: 2013/07/08 06:30:35 $
*/

//*****************************************************************************************
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
//
//		---
// 		ACR functioms 20100526
// 		Danymic Contrast ratio
//		 Default: Get Luminance data from Pixel grab.
// 		Second solution: Use ACC, if pixel grab used for VWD.
// 		Limitation: If system can not use ACC and pixel grab already used for VWD, it will no ACR anymore.
//
//*****************************************************************************************

//
//******************************************************************************
//******************************************************************************
//  I N C L U D E    F I L E S
//******************************************************************************
#include "System\All.h"
#include "System\ACR_Handler.h"

#if (FEATURE_PIXEL_CRUNCH == ENABLE)
#include "stdlibinc\PIXCRMain.h"
#endif

//******************************************************************************
//  D E F I N E S
//******************************************************************************
//#define _DEBUG
//#define _DEBUG_EXT

#ifdef _DEBUG
#define  msg(a,b)       gm_Print(a,b)
#else
#define  msg(a,b)
#endif

#ifdef _DEBUG_EXT
#define  msg_i(a,b)     gm_Print(a,b)
#else
#define  msg_i(a,b)
#endif

// Full White:   255
// Middle White: 128
// Full Black:   0
#define ACR_PWM_TARGET_0      0
#define ACR_LEVEL_1_THRESHOLD    50
#define ACR_PWM_TARGET_1      30
#define ACR_LEVEL_2_THRESHOLD    100
#define ACR_PWM_TARGET_2      70
#define ACR_LEVEL_3_THRESHOLD    150
#define ACR_PWM_TARGET_3      90
#define ACR_LEVEL_4_THRESHOLD    200
#define ACR_PWM_TARGET_4      100

#define ACR_EXECUTE_INTERVAL     100   // Unit: ms
#define ACR_PWM_ADJ_INTERVAL     20    // Unit: ms

#define ACR_LUM_DIFFERENCE       5
#define ACR_BACKLIGHT_STEP       1     // Backlight transition step in percentage
#define ACR_ACTION_LAG           5     // Unit: 100ms

#if (FEATURE_ACR == ENABLE)
extern BOOL gm_IsAfrSet(gmt_PHY_CH Channel);

//******************************************************************************
//  G L O B A L    V A R I A B L E S
//******************************************************************************
static BOOL ACR_ENABLE = FALSE;
static WORD PrevAvgINTXY = INVALID_RESULT;
static BYTE PrevACRPwmTarget = 0xFF;
static BYTE CurACRPwmTarget = 0xFF;
static BYTE AdjustBacklightVal = DEFAULT_BACKLIGHT_VAL;

//******************************************************************************
//  C O D E
//******************************************************************************
WORD GetCurrentAcrResult(void)
{
   return PrevAvgINTXY;
}

//******************************************************************************
// USAGE:   IsYuvInputMain
// INPUT:   None
// OUTPUT:	BOOL status of Main input color domain
// GLOBALS:
//******************************************************************************
BOOL IsYuvInputMain(void)
{
   return (gm_ReadRegWord(IMP_CONTROL) & IMP_YUV_EN);
}

//******************************************************************************
// USAGE:   IsACREnabled
// INPUT:   None
// OUTPUT:	BOOL status of ACR
// GLOBALS:
//******************************************************************************
BOOL IsACREnabled(void)
{
   return ACR_ENABLE;
}

//******************************************************************************
// USAGE:   IsPixCrAnalyzeACR
// INPUT:   None
// OUTPUT:	BOOL ACR using pixel cruncher
// GLOBALS:
//******************************************************************************
BOOL IsPixCrAnalyzeACR(void)
{
   if (UserPrefVwdFlag == OFF)
      return TRUE;
   else
      return FALSE;
}

//******************************************************************************
// USAGE:   ACR_Init
// INPUT:   None
// OUTPUT:	None
// GLOBALS:
//******************************************************************************
void ACR_Init(void)
{
   AdjustBacklightVal = DEFAULT_BACKLIGHT_VAL;
}

//******************************************************************************
// USAGE:   ACR_Enable
// INPUT:   None
// OUTPUT:	None
// GLOBALS:
//******************************************************************************
void ACR_Enable(void)
{
   msg_i("ACR Enable",0);
   ACR_ENABLE = TRUE;
   if ((gm_IsACCEnabled() == FALSE) && IsPixCrAnalyzeACR() == FALSE)
   {
      // Enable ACC for average brightness
      UserPrefImageScheme = VIVID;  // Test only. Default scheme should be used.
      AdjustImageScheme();
   }
}

//******************************************************************************
// USAGE:   ACR_Disable
// INPUT:   None
// OUTPUT:	None
// GLOBALS:
//******************************************************************************
void ACR_Disable(void)
{
   msg_i("ACR Disable",0);
#if (FEATURE_PIXEL_CRUNCH == ENABLE)
   PixCrAnalyzeACRStop();
#endif
   ACR_ENABLE = FALSE;
   PrevAvgINTXY = INVALID_RESULT;
   PrevACRPwmTarget = 0xFF;
   AdjustBacklightVal = DEFAULT_BACKLIGHT_VAL;
   AdjustPanelBacklight(AdjustBacklightVal);
}

//******************************************************************************
// USAGE:   ACR_Running
// INPUT:   None
// OUTPUT:	None
// GLOBALS:
//******************************************************************************
void ACR_Running(void)
{
#if (FEATURE_PIXEL_CRUNCH == ENABLE)
   // ACR only can be worked once the VWD in OSD is disabled.
   if (IsPixCrAnalyzeACR())
   {
      PixCrAnalyzeImageForACR();
   }
#endif   
}

//******************************************************************************
// USAGE:   ACR_GetLumData
// INPUT:   None
// OUTPUT:	WORD Luminance data
// GLOBALS:
//******************************************************************************
#ifdef ST_4K2K_93xx_BOARD
WORD W_PrevLeftDcrResult = INVALID_RESULT;
#endif
WORD ACR_GetLumData(void)
{
#if (FEATURE_PIXEL_CRUNCH == ENABLE)
   if (IsPixCrAnalyzeACR())
   {
    #ifdef ST_4K2K_93xx_BOARD
      if (GetChipID_4K2K() == CHIPID_4K2K_R)
      {
         if (SocIPCGetDcrResult(SOCIPC_DEVICEL_DST_ADDRESS))
         {
            if (gAthena_LEFT_FE_Slave_Parameters.DcrResult != INVALID_RESULT)
               W_PrevLeftDcrResult = gAthena_LEFT_FE_Slave_Parameters.DcrResult;

            return ((W_PrevLeftDcrResult + (WORD)(GetAvgINTXY())) / 2);
         }
         else
         {
            msg(" Error: Read Left DCR data!",0);
            return INVALID_RESULT;
         }
      }
    #endif
      return GetAvgINTXY();
   }
   else
#endif // FEATURE_PIXEL_CRUNCH
   {
      #if 1 // workaround for ACC_AVG_BRIGHTNESS overflow      
      #define ACC3_ACCMEAS_HISTOGRAM_0_MEM_START 0x32E00L // from 0x32E00 ~ 32EFF,  256 bytes, 64x22, 
      DWORD W_AvgBrightness = 0;
      DWORD DW_TotalPixel = 0;
      union
      {
         DWORD  dw;
         BYTE  b[4];
      } DWordVal;
      BYTE i;

      for(i = 0; i < 64; i++)
      {
         DWORD DW_TempAddr = (DWORD)ACC3_ACCMEAS_HISTOGRAM_0_MEM_START + i * 4;         
         
         DWordVal.b[0] = *(LINEAR_TO_FPTR_B(DW_TempAddr));
         DW_TempAddr++;
         DWordVal.b[1] = *(LINEAR_TO_FPTR_B(DW_TempAddr));
         DW_TempAddr++;
         DWordVal.b[2] = *(LINEAR_TO_FPTR_B(DW_TempAddr));
         DW_TempAddr++;
         DWordVal.b[3] = *(LINEAR_TO_FPTR_B(DW_TempAddr));

         DW_TotalPixel += DWordVal.dw;
         W_AvgBrightness += (DWordVal.dw * i); // total brightness
      }
      
      W_AvgBrightness = W_AvgBrightness/(DW_TotalPixel/512); // avg brightness, Max : 512 * 58 = 29696

      // Convert data range to 0~255
      W_AvgBrightness = (25500 / ((DWORD)(29696 - 2048)*100 / (W_AvgBrightness - 2048)));
      
      #else
      WORD W_AvgBrightness = (gm_ReadRegWord(ACC_AVG_BRIGHTNESS) & 0xFFFF) / 2;

      if (W_AvgBrightness > 2000)
         W_AvgBrightness -= 2000;
      else
         W_AvgBrightness = 0;
      #endif

      return (WORD)W_AvgBrightness;
   }
}

//******************************************************************************
// USAGE:   ACR_Handler
// INPUT:   None
// OUTPUT:	None
// GLOBALS:
//******************************************************************************
void ACR_Handler(void)
{
   static AcrHandlerState_t AcrHandlerState = ACR_NO_ACTION;
   static DWORD LastAcrExecTime = 0;
   WORD W_AvgLumData;

#ifdef ST_4K2K_93xx_BOARD
   if (GetChipID_4K2K() == CHIPID_4K2K_FE)
      return;
#endif
   if ((UserPrefDCR == OFF) || (!IsACREnabled()) || (gm_IsAfrSet(gmvb_MainChannel)))
      return;   

   if ((gm_GetSystemTime() - LastAcrExecTime) > ACR_EXECUTE_INTERVAL)
   {
      ACR_Running();
      W_AvgLumData = ACR_GetLumData();

      if ((abs(W_AvgLumData - PrevAvgINTXY) > ACR_LUM_DIFFERENCE) && (W_AvgLumData != INVALID_RESULT))
      {
         PrevAvgINTXY = W_AvgLumData;
         msg("Current Lum = %d",PrevAvgINTXY);
        #ifdef ST_4K2K_93xx_BOARD
         if (GetChipID_4K2K() == CHIPID_4K2K_L)
         {
            return;
         }
         else
        #endif
            AcrHandlerState = ACR_CALCULATE_PWM;
      }
   
      LastAcrExecTime = gm_GetSystemTime();
   }
   
   switch (AcrHandlerState)
   {
      case ACR_CALCULATE_PWM:
         if (W_AvgLumData <= ACR_LEVEL_1_THRESHOLD)
            CurACRPwmTarget = ACR_PWM_TARGET_0;
         else if (W_AvgLumData <= ACR_LEVEL_2_THRESHOLD)
            CurACRPwmTarget = ACR_PWM_TARGET_1;
         else if (W_AvgLumData <= ACR_LEVEL_3_THRESHOLD)
            CurACRPwmTarget = ACR_PWM_TARGET_2;
         else if (W_AvgLumData <= ACR_LEVEL_4_THRESHOLD)
            CurACRPwmTarget = ACR_PWM_TARGET_3;
         else
            CurACRPwmTarget = ACR_PWM_TARGET_4;

         if ((CurACRPwmTarget != PrevACRPwmTarget) || (AdjustBacklightVal != CurACRPwmTarget))
         {
            PrevACRPwmTarget = CurACRPwmTarget;
            msg("  ACR: New BL Target = %d",CurACRPwmTarget);
            msg_i("    AdjustBacklightVal = %d",AdjustBacklightVal);
            gm_TimerStart(ACR_ADJUST_100ms_TMR, ACR_ACTION_LAG);
            AcrHandlerState = ACR_ADJUST_PWM;
         }
         else
            AcrHandlerState = ACR_NO_ACTION;
         break;
         
      case ACR_ADJUST_PWM:
         if (gm_TimerCheck(ACR_ADJUST_100ms_TMR) != TIMER_OK)
         {
            static DWORD LastPwmAdjustTime = 0;
            if ((gm_GetSystemTime() - LastPwmAdjustTime) < ACR_PWM_ADJ_INTERVAL)
               break;
            
            if (AdjustBacklightVal != CurACRPwmTarget)
            {
               if (CurACRPwmTarget > AdjustBacklightVal)
                  AdjustBacklightVal = ((AdjustBacklightVal + ACR_BACKLIGHT_STEP) < CurACRPwmTarget)?(AdjustBacklightVal + ACR_BACKLIGHT_STEP):CurACRPwmTarget;
               else
               {
                  if ((AdjustBacklightVal - CurACRPwmTarget) <= ACR_BACKLIGHT_STEP)
                     AdjustBacklightVal = CurACRPwmTarget;
                  else
                     AdjustBacklightVal = ((AdjustBacklightVal - ACR_BACKLIGHT_STEP) > CurACRPwmTarget)?(AdjustBacklightVal - ACR_BACKLIGHT_STEP):CurACRPwmTarget;
               }
               AdjustPanelBacklight(AdjustBacklightVal);
            }
            else
            {
               msg_i("ACR: Backlight Adj Done!",0);
               AcrHandlerState = ACR_NO_ACTION;
            }
            LastPwmAdjustTime = gm_GetSystemTime();
         }
         break;
   }
}

#endif // FEATURE_ACR
