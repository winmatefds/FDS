/*
	$Workfile:   adj_inputCapture.c  $
	$Revision: 1.19 $
	$Date: 2013/05/23 09:41:34 $
*/
#define __ADJ_INPUTCAPTURE_C__
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
// MODULE:     	adj_InputCapture.c
//
//******************************************************************************

//******************************************************************************
//  I N C L U D E    F I L E S
//******************************************************************************

#include "System\All.h"
#include "Board\PortAttrib.h"


//******************************************************************************
//  D E F I N E S
//******************************************************************************

#define _DEBUG
#ifdef _DEBUG
#define msg(string)         gm_Print(string,0)
#define msg_i(string,value) gm_Print(string,value)
#else
#define msg(string)
#define msg_i(string,value)
#endif

// Choose if PIP cropping is done on Cortez or Hudson
#define CROP_PIP_ON_CORTEZ		TRUE

#define PERCENT(X)		(WORD)((0xffffL * (DWORD)X) / 100)


//*****************************************************************************
//  S T R U C T S
//*****************************************************************************

typedef struct AspectConversion_t
{
   WORD    InputHStart;
   WORD    InputVStart;
   WORD    InputHActive;
   WORD    InputVActive;
   WORD    OutputHStart;
   WORD    OutputVStart;
   WORD    OutputHActive;
   WORD    OutputVActive;
} AspectConversion_t;


//*****************************************************************************
//  E X T E R N A L   V A R I A B L E S
//*****************************************************************************

//*****************************************************************************
//  L O C A L S
//*****************************************************************************
//Due to internal filter/design changes, the delay is different for different Chips. The values can be changed in
//order to move the image horizontally. The values can only be fine tuned for NTSC timing or PAL timing,we cannot
//fine tune the the values for indivual modes.

#define H_ACT_OFFSET_NTSC_CVBS             18
#define H_ACT_OFFSET_PAL_CVBS               21
#define H_ACT_OFFSET_NTSC_SVIDEO          24
#define H_ACT_OFFSET_PAL_SVIDEO            21


//
// Aspect Ratios
//
AspectConversion_t ROM W_AspectConversion[NUM_ASPECT_RATIOS] =
{
   // iHStart			iVStart			iHActive		iVActive
   // oHStart			oVStart			oHActive		oVActive

   // Fullscreen
   { PERCENT(0),		PERCENT(0),		PERCENT(100),	PERCENT(100),
      PERCENT(0),		PERCENT(0),		PERCENT(100),	PERCENT(100), },
   // Letterbox
   { PERCENT(0),		PERCENT(10),	PERCENT(100),	PERCENT(80),
     PERCENT(0),		PERCENT(0),		PERCENT(100),	PERCENT(100), },
   // Pillarbox
   { PERCENT(0),		PERCENT(0),		PERCENT(100),	PERCENT(100),
     PERCENT(10),		PERCENT(0),		PERCENT(80),	PERCENT(100), },
   // Pillarbox Expand
   { PERCENT(10),		PERCENT(0),		PERCENT(80),	PERCENT(100),
     PERCENT(0),		PERCENT(0),		PERCENT(100),	PERCENT(100), },
   // Letterbox Expand
   { PERCENT(0),		PERCENT(10),	PERCENT(100),	PERCENT(80),
     PERCENT(0),		PERCENT(0),		PERCENT(100),	PERCENT(100), },
   // Waterglass
   { PERCENT(0),		PERCENT(0),		PERCENT(100),	PERCENT(100),
     PERCENT(10),		PERCENT(0),		PERCENT(80),	PERCENT(100), },
   // Panoramic
   { PERCENT(0),		PERCENT(0),		PERCENT(100),	PERCENT(100),
     PERCENT(0),		PERCENT(0),		PERCENT(100),	PERCENT(100), },
       // Panoramic
   { PERCENT(0),		PERCENT(0),		PERCENT(100),	PERCENT(100),
     PERCENT(0),		PERCENT(0),		PERCENT(100),	PERCENT(100), },

};

static gmt_WindowStruct PipWindowLimit; // Flexible PIP window limit
static gmt_WindowStruct PipWindow;      // Flexible PIP window

static void SetDisplayMainSize(void);
static void SetDisplayPipSize(void);

static void GetRecessiveWinSize(WORD W_DominantInputHActive, WORD W_DominantInputVActive,
                                WORD W_RecessiveInputHActive, WORD W_RecessiveInputVActive,
                                gmt_WindowStruct* pRecessiveOutputWin, BOOL Bt_IsMainWindow);
static void GetDominantWinSize(WORD W_InputHActive, WORD W_InputVActive,
                               gmt_WindowStruct* pOutputWin, BOOL Bt_IsMainWindow);

//*****************************************************************************
//  C O D E
//*****************************************************************************

//******************************************************************************
//
// FUNCTION       : void InputCaptureCrop(gmt_PHY_CH B_Channel)
//
// USAGE          : Crop the input based on the input mode and/or input port.
//
// INPUT          : B_Channel		 	 	CH_A
//                     CH_B
//
// OUTPUT         : None
//
// GLOBALS        : gmvw_InputHStart		(RW)
//                  gmvw_InputHActive		(RW)
//                  gmvw_InputVStart		(RW)
//                  gmvw_InputVActive		(RW)
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void InputCaptureCrop(gmt_PHY_CH B_Channel)
{
   BYTE B_InputConnector;
   BYTE B_IsVideo;
   gmt_MODE_TYPE B_CurrentModeType;
#if (FEATURE_HDMI == ENABLE)
   WORD W_HOverScan, W_VOverScan;
#endif

   if (B_Channel == gmvb_MainChannel)
   {
      B_InputConnector = UserPrefCurrentInputMain;
      if (gm_IsVideo(gmvb_MainChannel) || gm_IsEmbeddedVideo(gmvb_MainChannel))
         B_IsVideo = TRUE;
      else
         B_IsVideo = FALSE;
   }
   else
   {
      B_InputConnector = UserPrefCurrentInputPip;

      if (gm_IsVideo(gmvb_PipChannel) || gm_IsEmbeddedVideo(gmvb_PipChannel))
         B_IsVideo = TRUE;
      else
         B_IsVideo = FALSE;
   }

// 20110705, No crop in HDMI port
#if (FEATURE_HDMI==ENABLE)
   if (IsInternalHdmiDviPort(B_Channel))
   {
      //if (HdmiCropInput(B_Channel) == FALSE)
      {  
         #ifndef RD2_4K2K_DEMO
         stInput[B_Channel].ModeFlags &= ~(gmd_MODE_CAPTURE_CROP);
         #endif
         return;
      }
   }
   else
#endif
   {
      if ((B_IsVideo == FALSE)||(AppPortAttrib[B_InputConnector].B_InputMeasurementType == gmt_IBDmeasurement))
      {
         return;
      }
   }
 return;
   // 20090618 #1, For DP over Scan. (1 is disable overscan.)
#if 1
   if (gm_IsInputDP(B_Channel))
   {
      #ifdef ST_4K2K_93xx_BOARD
      // Not clear gmd_MODE_CAPTURE_CROP flag
      #else
      stInput[B_Channel].ModeFlags &= ~(gmd_MODE_CAPTURE_CROP);
      #endif
      return;
   }
#endif
   // Only crop video modes

   B_CurrentModeType = gm_GetCurrentInputModeType(B_Channel);

//gm_Printf("B_CurrentModeType=%d",B_CurrentModeType);
#if (FEATURE_HDMI == ENABLE)
   // 20090618 #1, For DP over Scan.
   if ((gm_IsInputHdmiDvi(B_Channel)) ||(gm_IsInputDP(B_Channel)))
   {
      if (( B_CurrentModeType == gmd_NTSC_480I  )
            || (  B_CurrentModeType == gmd_PAL_576I  )
            || ( B_CurrentModeType == gmd_480P )
            ||( B_CurrentModeType == gmd_576P)
            ||(  B_CurrentModeType == gmd_720P  )
            ||(  B_CurrentModeType == gmd_1080I  )
            ||(  ( B_CurrentModeType == gmd_576P ) && ( OVERLAP_1152I == 1188 ) )
            ||(  ( B_CurrentModeType == gmd_576P ) && ( OVERLAP_1152I == 1584 ) )
#if (FEATURE_INPUT_CAPTURE_1080P == ENABLE)
            || ( B_CurrentModeType == gmd_1080P )
#endif
         )
      {
         stInput[B_Channel].ModeFlags |= gmd_MODE_CAPTURE_CROP;
         W_HOverScan = ((stInput[B_Channel].Hactive*5)/100)&0xfffc;
         W_VOverScan = ((stInput[B_Channel].Vactive*5)/100)&0xfffc;
         stInput[B_Channel].Hstart = (W_HOverScan/2);
         stInput[B_Channel].Hactive -= W_HOverScan;
         stInput[B_Channel].Vstart = (W_VOverScan/2);
         stInput[B_Channel].Vactive -= W_VOverScan;
      }
      else
         stInput[B_Channel].ModeFlags &= ~(gmd_MODE_CAPTURE_CROP);
   }
   else
#endif

   {

      //
      // NTSC/480i (input to Cortez)
      //
      if ( B_CurrentModeType == gmd_NTSC_480I  )
      {
         switch (B_InputConnector)
         {
               // TODO: Add more correct cropping values
            case ALL_BOARDS_CVBS1:
             //  stInput[B_Channel].Hstart += H_ACT_OFFSET_NTSC_CVBS;
             //  stInput[B_Channel].Hactive -= 38;
           //    stInput[B_Channel].Vstart += 9;
            //   stInput[B_Channel].Vactive -= 16;
           //    break;
            case ALL_BOARDS_SVIDEO1:
               stInput[B_Channel].Hstart += 0;
               stInput[B_Channel].Hactive -= 0;
               stInput[B_Channel].Vstart += 0;
               stInput[B_Channel].Vactive -= 0;
               break;
            case ALL_BOARDS_COMP1:
               stInput[B_Channel].Hstart += 12;
               stInput[B_Channel].Hactive -= 34;
               stInput[B_Channel].Vstart += 6;
               stInput[B_Channel].Vactive -= 14;
               break;
            case ALL_BOARDS_VGA1:
               stInput[B_Channel].Hstart += 10;
               stInput[B_Channel].Hactive -= 30;
               stInput[B_Channel].Vstart += 6;
               stInput[B_Channel].Vactive -= 14;
               break;
            case ALL_BOARDS_HDMI:
#ifdef ALL_BOARDS_HDMI2
            case ALL_BOARDS_HDMI2:
#endif
#ifdef ALL_BOARDS_HDMI3
            case ALL_BOARDS_HDMI3:
#endif

#ifdef ALL_BOARDS_DVI1
            case ALL_BOARDS_DVI1:
#endif
#ifdef ALL_BOARDS_DVI2
            case ALL_BOARDS_DVI2:
#endif
               stInput[B_Channel].Hstart += 16;
               stInput[B_Channel].Hactive -= 32;
               stInput[B_Channel].Vstart += 8;
               stInput[B_Channel].Vactive -= 16;
               break;
         }
      }

      //
      // PAL/576i (input to Cortez)
      //
      else if (  B_CurrentModeType == gmd_PAL_576I  )
      {
         switch (B_InputConnector)
         {
               // TODO: Add more correct cropping values
            case ALL_BOARDS_CVBS1:
           //    stInput[B_Channel].Hstart += H_ACT_OFFSET_PAL_CVBS;
          //     stInput[B_Channel].Hactive -= 46;
            //   stInput[B_Channel].Vstart += 12;
            //   stInput[B_Channel].Vactive -= 30;
            //   break;
            case ALL_BOARDS_SVIDEO1:
               stInput[B_Channel].Hstart += 0;
               stInput[B_Channel].Hactive -= 0;
               stInput[B_Channel].Vstart += 0;
               stInput[B_Channel].Vactive -= 0;
               break;
            case ALL_BOARDS_COMP1:
               stInput[B_Channel].Hstart += 12;
               stInput[B_Channel].Hactive -= 54;
               stInput[B_Channel].Vstart += 8;
               stInput[B_Channel].Vactive -= 22;
               break;
            case ALL_BOARDS_VGA1:
               stInput[B_Channel].Hstart += 12;
               stInput[B_Channel].Hactive -= 54;
               stInput[B_Channel].Vstart += 8;
               stInput[B_Channel].Vactive -= 30;
               break;
            case ALL_BOARDS_HDMI:
#ifdef ALL_BOARDS_HDMI2
            case ALL_BOARDS_HDMI2:
#endif
#ifdef ALL_BOARDS_HDMI3
            case ALL_BOARDS_HDMI3:
#endif

#ifdef ALL_BOARDS_DVI1
            case ALL_BOARDS_DVI1:
#endif
#ifdef ALL_BOARDS_DVI2
            case ALL_BOARDS_DVI2:
#endif
               stInput[B_Channel].Hstart += 16;
               stInput[B_Channel].Hactive -= 32;
               stInput[B_Channel].Vstart += 8;
               stInput[B_Channel].Vactive -= 16;
               break;
         }
      }
      //
      // 480p (input to Cortez)
      //
      else if ( B_CurrentModeType == gmd_480P )
      {
         switch (B_InputConnector)
         {
            case ALL_BOARDS_COMP1:
            case ALL_BOARDS_VGA1:
               stInput[B_Channel].Hstart += 40;
               stInput[B_Channel].Hactive -= 54;
               stInput[B_Channel].Vstart += 8;
               stInput[B_Channel].Vactive -= 30;
               break;
            case ALL_BOARDS_HDMI:
#ifdef ALL_BOARDS_HDMI2
            case ALL_BOARDS_HDMI2:
#endif
#ifdef ALL_BOARDS_HDMI3
            case ALL_BOARDS_HDMI3:
#endif

#ifdef ALL_BOARDS_DVI1
            case ALL_BOARDS_DVI1:
#endif
#ifdef ALL_BOARDS_DVI2
            case ALL_BOARDS_DVI2:
#endif
               stInput[B_Channel].Hstart += 16;
               stInput[B_Channel].Hactive -= 32;
               stInput[B_Channel].Vstart += 8;
               stInput[B_Channel].Vactive -= 16;
               break;
         }
      }
      //
      // 576p (input to Cortez)
      //
      else if ( ( B_CurrentModeType == gmd_576P) )
      {
         switch (B_InputConnector)
         {
            case ALL_BOARDS_COMP1:
            case ALL_BOARDS_VGA1:
               stInput[B_Channel].Hstart += 0;
               stInput[B_Channel].Hactive -= 40;
               stInput[B_Channel].Vstart += 2;
               stInput[B_Channel].Vactive -= 30;
               break;
            case ALL_BOARDS_HDMI:
#ifdef ALL_BOARDS_HDMI2
            case ALL_BOARDS_HDMI2:
#endif
#ifdef ALL_BOARDS_HDMI3
            case ALL_BOARDS_HDMI3:
#endif

#ifdef ALL_BOARDS_DVI1
            case ALL_BOARDS_DVI1:
#endif
#ifdef ALL_BOARDS_DVI2
            case ALL_BOARDS_DVI2:
#endif
               stInput[B_Channel].Hstart += 16;
               stInput[B_Channel].Hactive -= 32;
               stInput[B_Channel].Vstart += 8;
               stInput[B_Channel].Vactive -= 16;
               break;
         }
      }
      //
      // 720p (input to Cortez)
      //
      else if (  B_CurrentModeType == gmd_720P  )
      {
         switch (B_InputConnector)
         {
            case ALL_BOARDS_COMP1:
            case ALL_BOARDS_VGA1:
               if (stInput[B_Channel].ModeFlags & gmd_MODE_SOG)//YUV
               {
                  stInput[B_Channel].Hstart += 36;
                  stInput[B_Channel].Hactive -= 70;
                  stInput[B_Channel].Vstart += 25;
                  stInput[B_Channel].Vactive -= 50;
               }
               else//RGB
               {
                  stInput[B_Channel].Hstart += 0;
                  stInput[B_Channel].Hactive -= 70;
                  stInput[B_Channel].Vstart += 30;
                  stInput[B_Channel].Vactive -= 50;
               }
               break;
            case ALL_BOARDS_HDMI:
#ifdef ALL_BOARDS_HDMI2
            case ALL_BOARDS_HDMI2:
#endif
#ifdef ALL_BOARDS_HDMI3
            case ALL_BOARDS_HDMI3:
#endif

#ifdef ALL_BOARDS_DVI1
            case ALL_BOARDS_DVI1:
#endif
#ifdef ALL_BOARDS_DVI2
            case ALL_BOARDS_DVI2:
#endif
               stInput[B_Channel].Hstart += 32;
               stInput[B_Channel].Hactive -= 64;
               stInput[B_Channel].Vstart += 16;
               stInput[B_Channel].Vactive -= 32;
               break;
         }
      }
      //
      // 1080i (input to Cortez)
      //
      else if (  B_CurrentModeType == gmd_1080I  )
      {
         switch (B_InputConnector)
         {
            case ALL_BOARDS_COMP1:
            case ALL_BOARDS_VGA1:
               if (stInput[B_Channel].ModeFlags & gmd_MODE_SOG)//YUV
               {
                  stInput[B_Channel].Hstart += 63;
                  stInput[B_Channel].Hactive -= 120;
                  stInput[B_Channel].Vstart += 22;
                  stInput[B_Channel].Vactive -= 42;
               }
               else//RGB
               {
                  stInput[B_Channel].Hstart += 22;
                  stInput[B_Channel].Hactive -= 120;
                  stInput[B_Channel].Vstart += 27;
                  stInput[B_Channel].Vactive -= 42;
               }
               break;
//						to fix SCR #3465 issue (Rogers HD Set-top-box 1080i issue)
//					case ALL_BOARDS_HDMI:
//					case ALL_BOARDS_HDMI2:
//					case ALL_BOARDS_HDMI3:
//						gmvw_InputHStart[B_Channel] += 32;
//						gmvw_InputHActive[B_Channel] -= 64;
//						gmvw_InputVStart[B_Channel] += 24;
//						gmvw_InputVActive[B_Channel] -= 48;
//						break;
         }
      }

      //
      // 1152iLA (input to Cortez)
      //
      else if (  ( B_CurrentModeType == gmd_576P ) && ( OVERLAP_1152I == 1188 ) )
      {
         switch (B_InputConnector)
         {
            case ALL_BOARDS_COMP1:
            case ALL_BOARDS_VGA1:
               if (stInput[B_Channel].ModeFlags & gmd_MODE_SOG)//YUV
               {
                  stInput[B_Channel].Hstart += 63 ; //+63
                  stInput[B_Channel].Hactive -= 120; //-120
                  stInput[B_Channel].Vstart += 22;
                  stInput[B_Channel].Vactive -= 42;
               }
               else//RGB
               {
                  stInput[B_Channel].Hstart += 22;
                  stInput[B_Channel].Hactive -= 120;
                  stInput[B_Channel].Vstart += 27;
                  stInput[B_Channel].Vactive -= 42;
               }
               break;
            case ALL_BOARDS_HDMI:
#ifdef ALL_BOARDS_HDMI2
            case ALL_BOARDS_HDMI2:
#endif
#ifdef ALL_BOARDS_HDMI3
            case ALL_BOARDS_HDMI3:
#endif

#ifdef ALL_BOARDS_DVI1
            case ALL_BOARDS_DVI1:
#endif
#ifdef ALL_BOARDS_DVI2
            case ALL_BOARDS_DVI2:
#endif
               stInput[B_Channel].Hstart += 32;
               stInput[B_Channel].Hactive -= 64;
               stInput[B_Channel].Vstart += 24;
               stInput[B_Channel].Vactive -= 48;
               break;
         }
      }


      //
      // 1152iSH (input to Cortez)
      //
      else if (  ( B_CurrentModeType == gmd_576P ) && ( OVERLAP_1152I == 1584 ) )
      {
         switch (B_InputConnector)
         {
            case ALL_BOARDS_COMP1:
            case ALL_BOARDS_VGA1:
               if (stInput[B_Channel].ModeFlags & gmd_MODE_SOG)//YUV
               {
                  stInput[B_Channel].Hstart += 63 ; //+63
                  stInput[B_Channel].Hactive -= 120; //-120
                  stInput[B_Channel].Vstart += 22;
                  stInput[B_Channel].Vactive -= 42;
               }
               else//RGB
               {
                  stInput[B_Channel].Hstart += 22;
                  stInput[B_Channel].Hactive -= 120;
                  stInput[B_Channel].Vstart += 27;
                  stInput[B_Channel].Vactive -= 42;
               }
               break;
            case ALL_BOARDS_HDMI:
#ifdef ALL_BOARDS_HDMI2
            case ALL_BOARDS_HDMI2:
#endif
#ifdef ALL_BOARDS_HDMI3
            case ALL_BOARDS_HDMI3:
#endif

#ifdef ALL_BOARDS_DVI1
            case ALL_BOARDS_DVI1:
#endif
#ifdef ALL_BOARDS_DVI2
            case ALL_BOARDS_DVI2:
#endif
               stInput[B_Channel].Hstart += 32;
               stInput[B_Channel].Hactive -= 64;
               stInput[B_Channel].Vstart += 24;
               stInput[B_Channel].Vactive -= 48;
               break;
         }
      }

      //
      // 1152iLH (input to Cortez)
      //
      else if (  ( B_CurrentModeType == gmd_576P ) && ( OVERLAP_1152I == 2376 ) )
      {
         switch (B_InputConnector)
         {
            case ALL_BOARDS_COMP1:
            case ALL_BOARDS_VGA1:
               if (stInput[B_Channel].ModeFlags & gmd_MODE_SOG)//YUV
               {
                  stInput[B_Channel].Hstart += 0;
                  stInput[B_Channel].Hactive -= 80;
                  stInput[B_Channel].Vstart += 22;
                  stInput[B_Channel].Vactive -= 42;
               }
               else//RGB
               {
                  stInput[B_Channel].Hstart += 22;
                  stInput[B_Channel].Hactive -= 120;
                  stInput[B_Channel].Vstart += 27;
                  stInput[B_Channel].Vactive -= 42;
               }
               break;
            case ALL_BOARDS_HDMI:
#ifdef ALL_BOARDS_HDMI2
            case ALL_BOARDS_HDMI2:
#endif
#ifdef ALL_BOARDS_HDMI3
            case ALL_BOARDS_HDMI3:
#endif

#ifdef ALL_BOARDS_DVI1
            case ALL_BOARDS_DVI1:
#endif
#ifdef ALL_BOARDS_DVI2
            case ALL_BOARDS_DVI2:
#endif
               stInput[B_Channel].Hstart += 32;
               stInput[B_Channel].Hactive -= 64;
               stInput[B_Channel].Vstart += 24;
               stInput[B_Channel].Vactive -= 48;
               break;
         }
      }

#if (FEATURE_INPUT_CAPTURE_1080P == ENABLE)
      //
      // 1080p (input to Cortez)
      //
      else if ( B_CurrentModeType == gmd_1080P )
      {
         switch (B_InputConnector)
         {
            case ALL_BOARDS_COMP1:
            case ALL_BOARDS_VGA1:
               stInput[B_Channel].Hstart += 46;
               stInput[B_Channel].Hactive -= 94;
               stInput[B_Channel].Vstart += 38;
               stInput[B_Channel].Vactive -= 42;
               break;
            case ALL_BOARDS_HDMI:
#ifdef ALL_BOARDS_HDMI2
            case ALL_BOARDS_HDMI2:
#endif
#ifdef ALL_BOARDS_HDMI3
            case ALL_BOARDS_HDMI3:
#endif

#ifdef ALL_BOARDS_DVI1
            case ALL_BOARDS_DVI1:
#endif
#ifdef ALL_BOARDS_DVI2
            case ALL_BOARDS_DVI2:
#endif
               stInput[B_Channel].Hstart += 32;
               stInput[B_Channel].Hactive -= 64;
               stInput[B_Channel].Vstart += 24;
               stInput[B_Channel].Vactive -= 48;
               break;
         }
      }
#endif
      //
      // All other video modes (input to Cortez)
      //
      else
      {
         stInput[B_Channel].Hstart += (WORD)(((DWORD)PERCENT(4.1) * (DWORD)stInput[B_Channel].Hactive + 0x4000) >> 16);
         stInput[B_Channel].Hactive = (WORD)(((DWORD)PERCENT(91.8) * (DWORD)stInput[B_Channel].Hactive + 0x4000) >> 16);
         stInput[B_Channel].Vstart += (WORD)(((DWORD)PERCENT(4.1) * (DWORD)stInput[B_Channel].Vactive + 0x4000) >> 16);
         stInput[B_Channel].Vactive = (WORD)(((DWORD)PERCENT(91.8) * (DWORD)stInput[B_Channel].Vactive + 0x4000) >> 16);
      }
   }
}

//******************************************************************************
//
// FUNCTION       : void ApplyOutputAspect(gmt_PHY_CH B_Channel, BYTE B_Method)
//
// USAGE          : Main()
//                  ModeSetupHdlr()
//                  ModeSetup()
//                  SetCaptureMain()
//                  ApplyOutputAspect()
//                  Called by SetCaptureMain() to apply the Output
//                  Aspect ratio conversion.
//
// INPUT          : B_Channel				CH_A
//                     CH_B
//                  B_Method				AR_FULL_SCREEN
//                     AR_LETTER_BOX
//                     AR_PILLAR_BOX
//                     AR_PILLAR_BOX_EXPAND
//                     AR_LETTER_BOX_EXPAND
//                     AR_WATER_GLASS
//                     AR_PANORAMIC
//
// OUTPUT         : None
//
// GLOBALS        : W_AspectConversion		(RO)
//                  gmvw_OutputHStart		(RW)
//                  gmvw_OutputHActive		(RW)
//                  gmvw_OutputVStart		(RW)
//                  gmvw_OutputVActive		(RW)
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void ApplyOutputAspect(gmt_PHY_CH B_Channel, BYTE B_Method)
{
   WORD W_Data;

   if (B_Method < NUM_ASPECT_RATIOS)
   {
      // Output H
      // + 0x4000 is used for Rounding
      //
      W_Data = (WORD)(((DWORD)W_AspectConversion[B_Method].OutputHStart
                       * (DWORD)gmvw_OutputHActive[B_Channel] + 0x4000) >> 16);
      gmvw_OutputHStart[B_Channel] += W_Data;
      gmvw_OutputHActive[B_Channel] =
         (WORD)(((DWORD)W_AspectConversion[B_Method].OutputHActive
                 * (DWORD)gmvw_OutputHActive[B_Channel] + 0x4000) >> 16);

      //
      // Round off to even number to avoid color swap
      //
      gmvw_OutputHActive[B_Channel] &= ~BIT0;

      // Output V
      W_Data = (WORD)(((DWORD)W_AspectConversion[B_Method].OutputVStart
                       * (DWORD)gmvw_OutputVActive[B_Channel] + 0x4000) >> 16);
      gmvw_OutputVStart[B_Channel] += W_Data;
      gmvw_OutputVActive[B_Channel] =
         (WORD)(((DWORD)W_AspectConversion[B_Method].OutputVActive
                 * (DWORD)gmvw_OutputVActive[B_Channel] + 0x4000) >> 16);
   }
}


//******************************************************************************
//
// FUNCTION       : gmt_ASPECT_RATIO GetAspectRatio(gmt_PHY_CH B_Channel)
//
// USAGE          : Main()
//                  ModeSetupHdlr()
//                  ModeSetup()
//                  SelectCaptureMain()
//                  Called by SelectCaptureMain() to apply determine the
//                  Aspect ratio of the Input.
//
// INPUT          : B_Channel				CH_A
//                     CH_B
//
// OUTPUT         : gmd_ASPECT_16x9
//                  gmd_ASPECT_15x9
//                  gmd_ASPECT_5x4
//                  gmd_ASPECT_4x3
//
// GLOBALS        : gmvw_InputHTotalOrg		(RO)
//                  gmvw_InputVTotalOrg		(RO)
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
gmt_ASPECT_RATIO GetAspectRatio(gmt_PHY_CH B_Channel)
{
   WORD W_Data;

   // Calculate Ratio
   W_Data = gmvw_InputVTotalOrg[B_Channel];
   if (gm_IsInterlaced(B_Channel) == TRUE)
      W_Data *= 2;
   W_Data = (WORD)(((DWORD)gmvw_InputHTotalOrg[B_Channel] * 100) / W_Data);

   // Return Aspect
   if ((W_Data > 177 - 5) && (W_Data < 177 + 5)) 		// 16:9 = 177
      return(gmd_ASPECT_16x9);
   else if ((W_Data > 166 - 5) && (W_Data < 166 + 5))	// 15:9 = 166
      return(gmd_ASPECT_15x9);
   else if ((W_Data > 125 - 5) && (W_Data < 125 +5))	// 5:4 = 125
      return(gmd_ASPECT_5x4);
   else
      return(gmd_ASPECT_4x3);

}


//******************************************************************************
//
// FUNCTION       : void SelectAspectMain(void)
//
// USAGE          : Determines the Aspect Correction for Main.
//
// INPUT          :
//
// OUTPUT         : None
//
// GLOBALS        : UserPrefAspectRatioMain	(RW)
//                  B_AspectMethod			(WO)
//                  B_InputAspect			(WO)
//                  gmvb_MainChannel		(RO)
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void SelectAspectMain(void)
{
   BOOL Bt_IsVideo = gm_IsVideo(gmvb_MainChannel) || gm_IsEmbeddedVideo(gmvb_MainChannel);
   static BYTE MainPanoramic= FALSE;

   // May need to change the aspect selection for certain inputs
   switch (UserPrefAspectRatioMain)
   {
      case AR_FULL_SCREEN:
         if (Bt_IsVideo && (stInput[gmvb_MainChannel].Flags & gmd_INPUT_WSS))
            UserPrefAspectRatioMain = AR_LETTER_BOX;
         break;

      case AR_PANORAMIC: 
         if (!Bt_IsVideo || !IsPanoramicAvailable())
            UserPrefAspectRatioMain = AR_FULL_SCREEN;
         break;

      case AR_LETTER_BOX_EXPAND:
         if (!Bt_IsVideo)
            UserPrefAspectRatioMain = AR_FULL_SCREEN;
         break;
   }

   // Turn Panoramic OFF in non Panoramic modes
   if ((UserPrefAspectRatioMain != AR_PANORAMIC) && MainPanoramic)
   {
#if (FEATURE_BLENDER_FAILSAFE == REV_NONE)
      // RC: Disable blender such that OSD does not disappear when panoramic is disabled.
      gm_BlenderOnOff(gmvb_MainChannel, FALSE);
      gm_ForceUpdate(gmvb_MainChannel, OUTPUT_CLOCK);
#endif
		#if (FEATURE_PANORAMIC == ENABLE)
      gm_ScalerSetPanoramic(PanelWidth, gmd_PANORAMIC_OFF);
      MainPanoramic = FALSE;
		#endif
   }
	#if (FEATURE_PANORAMIC == ENABLE)
   else if ((UserPrefAspectRatioMain == AR_PANORAMIC) && !MainPanoramic)
   {
      gm_ScalerSetPanoramic(PanelWidth, gmd_PANORAMIC_ON);
      MainPanoramic=TRUE;
   }
	#endif
   B_AspectMethod[gmvb_MainChannel] = UserPrefAspectRatioMain;
}


//******************************************************************************
//
// FUNCTION       : void SetCaptureMain(void)
//
// USAGE          : Main()
//                  ModeSetupHdlr()
//                  ModeSetup()
//                  SetCaptureMain()
//                  Sets up the Input Capture Window according to the User
//                  Modes Provided.  Designed to be called from either
//                  SetupMain or an OSD Adjuster function.
//
// INPUT          :
//
// OUTPUT         : None
//
// GLOBALS        : gmvb_MainChannel			(R)
//                  W_OverscanEnabled			(R)
//                  W_LetterboxEnabled			(R)
//                  gmvw_InputHStart			(W)
//                  gmvw_InputHActive			(W)
//                  gmvw_InputVStart			(W)
//                  gmvw_InputVActive			(W)
//                  B_InputAspect				(R)
//                  B_OutputAspect				(R)
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void SetCapture(gmt_PHY_CH B_Channel)
{
   // Starting point
   stInput[B_Channel].Hstart = gmvw_InputHStartOrg[B_Channel];
   stInput[B_Channel].Hactive = gmvw_InputHActiveOrg[B_Channel];
   stInput[B_Channel].Vstart = gmvw_InputVStartOrg[B_Channel];
   stInput[B_Channel].Vactive = gmvw_InputVActiveOrg[B_Channel];

   // Main Graphics Image Position
   if (gm_IsInputTripleADC(B_Channel)
         && gm_IsGraphics(B_Channel))
   {
      stInput[B_Channel].Htotal = UserPrefHTotal;
      stInput[B_Channel].Hstart = UserPrefHStart;
      gmvsw_InputHSyncOffset[B_Channel] = UserPrefHSyncDelay;
      stInput[B_Channel].Vstart = UserPrefVStart;
      gmvsw_InputVSyncOffset[B_Channel] = UserPrefVSyncDelay;
      gmvb_InputSamplingPhase[B_Channel] = UserPrefHSyncPhase;
   }

   InputCaptureCrop(B_Channel);
   W_OverscanEnabled[B_Channel] = gm_IsVideo(B_Channel);

   //ApplyInputAspect
   if (B_AspectMethod[B_Channel] < NUM_ASPECT_RATIOS)
   {
      // + 0x4000 is used for Rounding
#if (FEATURE_HDMI==ENABLE)
   #ifndef RD2_4K2K_DEMO
      if (IsInternalHdmiDviPort(B_Channel))
      {
         if (B_AspectMethod[B_Channel]!= AR_FULL_SCREEN)
         {
        //    stInput[B_Channel].ModeFlags |= (gmd_MODE_CAPTURE_CROP);
         }
         else
            stInput[B_Channel].ModeFlags &= ~(gmd_MODE_CAPTURE_CROP);
      }
   #endif
#endif
      stInput[B_Channel].Hstart += (WORD)(((DWORD)W_AspectConversion[B_AspectMethod[B_Channel]].InputHStart
                                           * (DWORD)stInput[B_Channel].Hactive + 0x4000) >> 16);
      stInput[B_Channel].Hactive =
         (WORD)(((DWORD)W_AspectConversion[B_AspectMethod[B_Channel]].InputHActive
                 * (DWORD)stInput[B_Channel].Hactive + 0x4000) >> 16);
      stInput[B_Channel].Vstart += (WORD)(((DWORD)W_AspectConversion[B_AspectMethod[B_Channel]].InputVStart
                                           * (DWORD)stInput[B_Channel].Vactive + 0x4000) >> 16);
      stInput[B_Channel].Vactive =
         (WORD)(((DWORD)W_AspectConversion[B_AspectMethod[B_Channel]].InputVActive
                 * (DWORD)stInput[B_Channel].Vactive + 0x4000) >> 16);
   }
}

//******************************************************************************
//
// FUNCTION       : void SetDisplayMain(void)
//
// USAGE          : Main()
//                  ModeSetupHdlr()
//                  ModeSetup()
//                  SetCaptureMain()
//
// INPUT          :
//
// OUTPUT         : None
//
// GLOBALS        : None
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void SetDisplayMain(void)
{
   SetDisplayMainSize();

   // If MAIN is on CH_B, need to set pip to SINGLE_PIP always
   if (gmvb_MainChannel != CH_A)
      gm_PipSetMode(MODE_SINGLE_PIP);

 //  if(stInput[gmvb_MainChannel].Hactive ==674 && stInput[gmvb_MainChannel].Vactive== 258)
 //        gm_WriteRegWord(IMP_FRAME_RESET_LINE, 0x02);
}

//******************************************************************************
//
// FUNCTION       : static void SetDisplayMainSize(void)
//
// USAGE          : Set up MAIN output window size
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        : gmvw_OutputHStart		(WO)
//                  gmvw_OutputVStart		(WO)
//                  gmvw_OutputHActive		(WO)
//                  gmvw_OutputVActive		(WO)
//                  SW_DynScalePosition		(RO)
//                  B_AspectMethod			(RO)
//
// USED_REGS      : None
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
static void SetDisplayMainSize(void)
{
   //
   // Multi PIP has already setup size of main
   {
      //
      // Restore Output
      //
      gmvw_OutputHStart[gmvb_MainChannel] = 0;
      gmvw_OutputVStart[gmvb_MainChannel] = 0;
      gmvw_OutputHActive[gmvb_MainChannel] = PanelWidth;
      gmvw_OutputVActive[gmvb_MainChannel] = PanelHeight;
   }


   //
   // Side by Side (Width is Fixed - Adjust Height to match input aspect)
   //
#ifndef NO_PIP_SUPPORT
   if (IsSideBySidePipMode(UserPrefPipMode) == TRUE)
   {
      gmvw_OutputHActive[gmvb_MainChannel] =
         ((PanelWidth - 3 * PIP_BORDER_WIDTH) / 2 + SW_DynScalePosition);
      gmvw_OutputHStart[gmvb_MainChannel] = PIP_BORDER_WIDTH;

      if (UserPrefPipMode == SIDE_BY_SIDE)
      {
         gmvw_OutputVActive[gmvb_MainChannel] =
            (WORD)(((DWORD)gmvw_OutputHActive[gmvb_MainChannel] * 3) / 4);
      }
      else // SIDE_BY_SIDE_TALL
      {
         gmvw_OutputVActive[gmvb_MainChannel] =
            (PanelHeight - (2 * PIP_BORDER_HEIGHT));
      }
      gmvw_OutputVStart[gmvb_MainChannel] =
         ((PanelHeight - gmvw_OutputVActive[gmvb_MainChannel]) / 2);
   }
#endif
   //
   // Apply Output Aspect
   //
   ApplyOutputAspect(gmvb_MainChannel, B_AspectMethod[gmvb_MainChannel]); 

   //

  if (UserPrefAspectRatioMain==AR_ZOOM_OFF && (UserPrefPipMode==NO_PIP))
	AdjustNativeMode(); 
   // Correct Width for YUV Inputs
   // ceil at even
   //
   gmvw_OutputHActive[gmvb_MainChannel] =
      ((gmvw_OutputHActive[gmvb_MainChannel] + 1) & ~BIT0);

   //Correct the sze of split(demo mode) window

/*
	  
#if defined(UserPrefSplitScreen)
   if (UserPrefSplitScreen)
   {
#if (DEMO_WINDOW_SPLIT == VIDEO_WIN_LEFT)
      gm_CscSetVideoWindowSize(VIDEO_WIN_LEFT);
#elif (DEMO_WINDOW_SPLIT == VIDEO_WIN_RIGHT)
      gm_CscSetVideoWindowSize(VIDEO_WIN_RIGHT);
#elif (DEMO_WINDOW_SPLIT == VIDEO_WIN_UP)
      gm_CscSetVideoWindowSize(VIDEO_WIN_UP);
#elif (DEMO_WINDOW_SPLIT == VIDEO_WIN_DOWN)
      gm_CscSetVideoWindowSize(VIDEO_WIN_DOWN);
#endif
   }
#endif	
*/
}

//******************************************************************************
//
// FUNCTION       : static void GetDominantWinSize(WORD W_InputHActive, WORD W_InputVActive,
//								gmt_WindowStruct* OutputWin, BOOL Bt_IsMainWindow)
//
// USAGE          : Return the largest output window size attainable for the panel with
//					the same aspect ratio as the input.
//
// INPUT          : W_InputHActive - input width
//					W_InputVActive - input height
//					pOutputWin - pointer to a structure that keeps output [H|V]Active and
//								[H|V]Start
//					Bt_IsMainWindow - whether this window is Main or not. This calculation
//									assumes the reference design window assignment.
//
// OUTPUT         : None
//
// GLOBALS        : None
//
// USED_REGS      : None
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
static void GetDominantWinSize(WORD W_InputHActive, WORD W_InputVActive,
                               gmt_WindowStruct* pOutputWin, BOOL Bt_IsMainWindow)
{
   BYTE B_ToleranceH = 2 * PIP_BORDER_WIDTH * 5;//tolerance to decide if win is taking up the whole screen
   //BYTE B_ToleranceV = 2 * PIP_BORDER_HEIGHT * 5;//tolerance to decide if win is taking up the whole screen
   WORD W_MaxOutputHeight = PanelHeight - (2 * PIP_BORDER_HEIGHT);
   WORD W_MaxOutputWidth = PanelWidth - (2 * PIP_BORDER_WIDTH);
   WORD W_Ratio;

   //prevent division by zero
   if ((W_InputHActive == 0)||(W_InputVActive == 0)) return;

   W_Ratio = (WORD)(((DWORD)W_InputHActive * 100) / W_InputVActive);

   //categorize input aspect ratio as 16:9 or 4:3
   if (W_Ratio > 165) // aspect ratio greater than 15:9
      W_InputVActive = (WORD)((DWORD)W_InputHActive * 9/16);//ratio is 16:9
   else
      W_InputVActive = (WORD)((DWORD)W_InputHActive * 3/4);//ratio is 4:3

   pOutputWin->W_Width = (WORD)(W_MaxOutputHeight * (DWORD)W_InputHActive / W_InputVActive);
   pOutputWin->W_Height = (WORD)(W_MaxOutputWidth * (DWORD)W_InputVActive / W_InputHActive);

   //height is the limiting factor
   if (pOutputWin->W_Height > W_MaxOutputHeight)
   {
      pOutputWin->W_Height = W_MaxOutputHeight;
      pOutputWin->W_Width = (WORD)(pOutputWin->W_Height * (DWORD)W_InputHActive / W_InputVActive);
   }
   //width is the limiting factor
   else if (pOutputWin->W_Width > W_MaxOutputWidth)
   {
      pOutputWin->W_Width = W_MaxOutputWidth;
      pOutputWin->W_Height = (WORD)(pOutputWin->W_Width * (DWORD)W_InputVActive / W_InputHActive);
   }

   //if it is taking up almost the whole screen
   if (PanelWidth - pOutputWin->W_Width < B_ToleranceH)
//		&&(PanelHeight - OutputWin->W_Height < B_ToleranceV))
   {
      //assuming width is always the limiting factor
      pOutputWin->W_Width = (WORD)((DWORD)PanelWidth*9/10);
      pOutputWin->W_Height = (WORD)(pOutputWin->W_Width * (DWORD)W_InputVActive / W_InputHActive);
   }

   pOutputWin->W_Vstart = (PanelHeight - pOutputWin->W_Height)/2;

   if (Bt_IsMainWindow)
   {
      pOutputWin->W_Hstart = PIP_BORDER_WIDTH;
   }
   else
   {
      pOutputWin->W_Hstart = PanelWidth - PIP_BORDER_WIDTH - pOutputWin->W_Width;
      pOutputWin->W_Hstart &= ~1;
   }
}

//******************************************************************************
//
// FUNCTION       : static void GetRecessiveWinSize(WORD W_DominantInputHActive,
//						WORD W_DominantInputVActive, WORD W_RecessiveInputHActive,
//						WORD W_RecessiveInputVActive, gmt_WindowStruct* RecessiveOutputWin,
//						BOOL Bt_IsMainWindow)
//
// USAGE          : Return the largest output window size attainable, with the same aspect
//					ratio as the input, in the remaining space after the dimension of the
//					dominant window has been determined in a side-by-side configuration.
//
// INPUT          : W_DominantInputHActive - dominant input width
//					W_DominantInputVActive - dominant input height
//					W_RecessiveInputHActive - recessive input width
//					W_RecessiveInputVActive - recessive input height
//					pRecessiveOutputWin - pointer to a structure that keeps output [H|V]Active
//										and	[H|V]Start
//					Bt_IsMainWindow - whether this window is Main or not. This calculation
//									assumes the reference design window assignment.
//
// OUTPUT         : None
//
// GLOBALS        : None
//
// USED_REGS      : None
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
static void GetRecessiveWinSize(WORD W_DominantInputHActive, WORD W_DominantInputVActive,
                                WORD W_RecessiveInputHActive, WORD W_RecessiveInputVActive,
                                gmt_WindowStruct* pRecessiveOutputWin, BOOL Bt_IsMainWindow)
{
   WORD W_DominantOutputHActive;
   WORD W_DominantOutputVActive;
   gmt_WindowStruct DominantOutputWin;

   //prevent division by zero
   if (W_RecessiveInputHActive == 0) return;

   GetDominantWinSize(W_DominantInputHActive, W_DominantInputVActive,
                      &DominantOutputWin, !Bt_IsMainWindow);

   //assuming width is always the limiting factor
   pRecessiveOutputWin->W_Width = PanelWidth - DominantOutputWin.W_Width - 3 * PIP_BORDER_WIDTH;
   pRecessiveOutputWin->W_Height = (WORD)(pRecessiveOutputWin->W_Width
                                          * (DWORD)W_RecessiveInputVActive / W_RecessiveInputHActive);

   pRecessiveOutputWin->W_Vstart = (PanelHeight - pRecessiveOutputWin->W_Height)/2;

   if (Bt_IsMainWindow)
   {
      pRecessiveOutputWin->W_Hstart = PIP_BORDER_WIDTH;
   }
   else
   {
      pRecessiveOutputWin->W_Hstart = DominantOutputWin.W_Width + 2 * PIP_BORDER_WIDTH;
   }
}

//******************************************************************************
//
// FUNCTION       : void SelectAspectPip(void)
//
// USAGE          : Determines the Aspect Correction for PIP
//
// INPUT          :
//
// OUTPUT         : None
//
// GLOBALS        : gmvb_PipChannel			(RO)
//                  B_AspectMethod			(WO)
//                  st_InputMeasurement.W_DetectFlags	(RO)
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void SelectAspectPip(void)
{
   BOOL Bt_IsVideo = gm_IsVideo(gmvb_PipChannel) || gm_IsEmbeddedVideo(gmvb_PipChannel);
   UserPrefAspectRatioPip=AR_FULL_SCREEN;
   B_AspectMethod[gmvb_PipChannel] = UserPrefAspectRatioPip;

   switch (UserPrefAspectRatioPip)
   {
      case AR_FULL_SCREEN:
         //if (Bt_IsVideo && (stInput[gmvb_PipChannel].Flags & gmd_INPUT_WSS))
            B_AspectMethod[gmvb_PipChannel] = AR_FULL_SCREEN;
         break;

      case AR_PANORAMIC:
      case AR_WATER_GLASS:
         // Panoramic and Waterglass are not supported for PIP
         B_AspectMethod[gmvb_PipChannel] = AR_FULL_SCREEN;
         break;

      case AR_LETTER_BOX_EXPAND:
         if (!Bt_IsVideo)
         {
            UserPrefAspectRatioPip = AR_FULL_SCREEN;
            B_AspectMethod[gmvb_PipChannel] = AR_FULL_SCREEN;
         }
         break;

      default:
         break;
   }
}


//******************************************************************************
//
// FUNCTION       : void SetDisplayPip(void)
//
// USAGE          : Main()
//                  ModeSetupHdlr()
//                  ModeSetup()
//                  SetDisplayPip()
//
// INPUT          :
//
// OUTPUT         : None
//
// GLOBALS        : None
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void SetDisplayPip(void)
{
#ifndef NO_PIP_SUPPORT	
   if (UserPrefPipMode != NO_PIP)
   {
      SetDisplayPipSize();

      // Set the PIP mode to single PIP
      gm_PipSetMode(MODE_SINGLE_PIP);
   }
   else
   {
      gm_PipSetMode(MODE_NO_PIP);
   }
#endif	
}

//******************************************************************************
//
// FUNCTION       : static void SetDisplayPipSize(void)
//
// USAGE          : Set up PIP output window size
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        : gmvw_OutputHStart		(WO)
//                  gmvw_OutputVStart		(WO)
//                  gmvw_OutputHActive		(WO)
//                  gmvw_OutputVActive		(WO)
//                  SW_DynScalePosition		(RO)
//                  B_AspectMethod			(RO)
//
// USED_REGS      : None
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
static void SetDisplayPipSize(void)
{
#ifndef NO_PIP_SUPPORT	
   WORD	W_HActiveMain = ((PanelWidth - (3 * PIP_BORDER_WIDTH)) / 2) + SW_DynScalePosition;
   W_HActiveMain &= ~1;

   {
      switch (UserPrefPipMode)
      {
         case SMALL_PIP:
         case LARGE_PIP:
            if (UserPrefPipMode == SMALL_PIP)
               gmvw_OutputHActive[gmvb_PipChannel] = PanelWidth / 4;
            else
               gmvw_OutputHActive[gmvb_PipChannel] = 3 * (PanelWidth / 8);
            gmvw_OutputVActive[gmvb_PipChannel] = (gmvw_OutputHActive[gmvb_PipChannel] * 3) / 4;
            switch (UserPrefPipPosition)
            {
               case BOTTOM_RIGHT:
                  gmvw_OutputHStart[gmvb_PipChannel] = PanelWidth - gmvw_OutputHActive[gmvb_PipChannel] - PIP_WINDOW_HORIZONTAL_OFFSET;
                  gmvw_OutputVStart[gmvb_PipChannel] = PanelHeight - gmvw_OutputVActive[gmvb_PipChannel] - PIP_WINDOW_VERTICAL_OFFSET;
                  break;
               case TOP_RIGHT:
                  gmvw_OutputHStart[gmvb_PipChannel] = PanelWidth - gmvw_OutputHActive[gmvb_PipChannel] - PIP_WINDOW_HORIZONTAL_OFFSET;
                  gmvw_OutputVStart[gmvb_PipChannel] = PIP_WINDOW_VERTICAL_OFFSET;
                  break;
               case TOP_LEFT:
                  gmvw_OutputHStart[gmvb_PipChannel] = PIP_WINDOW_HORIZONTAL_OFFSET;
                  gmvw_OutputVStart[gmvb_PipChannel] = PIP_WINDOW_VERTICAL_OFFSET;
                  break;
               case BOTTOM_LEFT:
                  gmvw_OutputHStart[gmvb_PipChannel] = PIP_WINDOW_HORIZONTAL_OFFSET;
                  gmvw_OutputVStart[gmvb_PipChannel] = PanelHeight - gmvw_OutputVActive[gmvb_PipChannel] - PIP_WINDOW_VERTICAL_OFFSET;
                  break;
            }

            if (gmvb_PipChannel == CH_A)
            {
               //highlight border is used as PIP border and thus this change
               gmvw_OutputHStart[gmvb_PipChannel] += PIP_BORDER_WIDTH;
               gmvw_OutputVStart[gmvb_PipChannel] += PIP_BORDER_HEIGHT;
            }
            break;
         case FLEXIBLE_PIP:
            gmvw_OutputHActive[gmvb_PipChannel] = PipWindow.W_Width;
            gmvw_OutputVActive[gmvb_PipChannel] = PipWindow.W_Height;
            gmvw_OutputHStart[gmvb_PipChannel] = PipWindowLimit.W_Hstart + PipWindow.W_Hstart;
            gmvw_OutputVStart[gmvb_PipChannel] = PipWindowLimit.W_Vstart + PipWindow.W_Vstart;

            if (gmvb_PipChannel == CH_A)
            {
               //highlight border is used as PIP border and thus this change
               gmvw_OutputHStart[gmvb_PipChannel] += PIP_BORDER_WIDTH;
               gmvw_OutputVStart[gmvb_PipChannel] += PIP_BORDER_HEIGHT;
            }
            break;

         case SIDE_BY_SIDE:
            gmvw_OutputHActive[gmvb_PipChannel] = PanelWidth - (3 * PIP_BORDER_WIDTH) - W_HActiveMain;
            gmvw_OutputVActive[gmvb_PipChannel] = (gmvw_OutputHActive[gmvb_PipChannel] * 3) / 4;
            gmvw_OutputHStart[gmvb_PipChannel] = W_HActiveMain + (2 * PIP_BORDER_WIDTH);
            gmvw_OutputVStart[gmvb_PipChannel] = (PanelHeight - gmvw_OutputVActive[gmvb_PipChannel]) / 2;
            break;

         case SIDE_BY_SIDE_TALL:
            gmvw_OutputHActive[gmvb_PipChannel] = PanelWidth - (3 * PIP_BORDER_WIDTH) - W_HActiveMain;
            gmvw_OutputVActive[gmvb_PipChannel] = PanelHeight - (2 * PIP_BORDER_HEIGHT);
            gmvw_OutputHStart[gmvb_PipChannel] = W_HActiveMain + (2 * PIP_BORDER_WIDTH);
            gmvw_OutputVStart[gmvb_PipChannel] = PIP_BORDER_HEIGHT;
            break;
      }

      //
      // Apply Output Aspect
      //
      ApplyOutputAspect(gmvb_PipChannel, B_AspectMethod[gmvb_PipChannel]);
   }

   //
   // Correct Width for YUV Inputs
   // ceil at even
   //
   gmvw_OutputHActive[gmvb_PipChannel] =
      ((gmvw_OutputHActive[gmvb_PipChannel] + 1) & ~BIT0);
#endif	// NO_PIP_SUPPORT
}

BOOL SetFlexiblePipWindowLimit(WORD Hstart, WORD Vstart, WORD Width, WORD Height)
{
   if (Hstart > PanelWidth || Vstart > PanelHeight || Width > PanelWidth || Height > PanelHeight)
      return FALSE;

   PipWindowLimit.W_Hstart = Hstart; // PIP_WINDOW_HORIZONTAL_OFFSET
   PipWindowLimit.W_Vstart = Vstart; // PIP_WINDOW_VERTICAL_OFFSET
   PipWindowLimit.W_Width  = Width;  // PanelWidth
   PipWindowLimit.W_Height = Height; // PanelHeight
   return TRUE;
}

BOOL SetFlexiblePipWindow(WORD Hstart, WORD Vstart, WORD Width, WORD Height)
{
   if (PipWindowLimit.W_Hstart + Hstart + 2*PIP_BORDER_WIDTH + Width > PipWindowLimit.W_Width)
   {
      // Width = PipWindowLimit.W_Width - PipWindowLimit.W_Hstart - Hstart - 2*PIP_BORDER_WIDTH;
      // msg_i("Adjust Width = %d",Width);
      return FALSE;
   }

   if (PipWindowLimit.W_Vstart + Vstart + 2*PIP_BORDER_HEIGHT + Height > PipWindowLimit.W_Height)
   {
      // Height = PipWindowLimit.W_Height - PipWindowLimit.W_Vstart - Vstart - 2*PIP_BORDER_HEIGHT;
      // msg_i("Adjust Height = %d",Height);
      return FALSE;
   }

   PipWindow.W_Hstart = Hstart;
   PipWindow.W_Vstart = Vstart;
   PipWindow.W_Width = Width;
   PipWindow.W_Height = Height;
   // gm_Print("--- (%d, %d, %d, %d) ---",PipWindow.W_Hstart,PipWindow.W_Vstart,PipWindow.W_Width,PipWindow.W_Height);
   return TRUE;
}

gmt_WindowStruct GetFlexiblePipWindow(void)
{
   return PipWindow;
}
/***********************************  END  ************************************/

