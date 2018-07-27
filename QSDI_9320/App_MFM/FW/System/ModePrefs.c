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
// MODULE:		ModePrefs.c
//
//******************************************************************************

//******************************************************************************
//  I N C L U D E    F I L E S
//******************************************************************************

#include <math.h>
#include "System\All.h"




//******************************************************************************
//  D E B U G
//******************************************************************************

//#define _DEBUG_MSG
#ifdef _DEBUG_MSG
#define msg(x,y) gm_Print(x,y)
#else
#define msg(x,y)
#endif


//******************************************************************************
//  E X T E R N A L    F U N C T I O N S
//******************************************************************************

extern void LetterBoxSetup(gmt_PHY_CH B_Channel);


//******************************************************************************
//  D E F I N E S
//******************************************************************************

#define MIN_SIZE(X,Y) ((X)<(Y)?(X):(Y))

//******************************************************************************
//  E X T E R N A L    V A R I A B L E S
//******************************************************************************
BYTE B_DisplayPort_ColorSpace;

//*****************************************************************************
//  C O D E
//*****************************************************************************

//******************************************************************************
//
// FUNCTION       : void SaveOriginalValue(gmt_PHY_CH B_Channel)
//
// USAGE          : Saves the active values into 2nd set of variables.
//                  To be used by aspect ratio changes
//
// INPUT          : B_Channel - CH_A or CH_B
//                     Note : The value of CH_A or CH_B is stored in
//                     gmvb_MainChannel and gmvb_PipChannel
//
// OUTPUT         : None
//
// GLOBALS        : gmvw_InputHActive		(R)
//                  gmvw_InputVActive		(R)
//                  gmvw_InputHTotal		(R)
//                  gmvw_InputVTotal		(R)
//                  gmvw_InputHStart		(R)
//                  gmvw_InputVStart		(R)
//                  gmvw_InputHActiveOrg	(W)
//                  gmvw_InputVActiveOrg	(W)
//                  gmvw_InputHTotalOrg		(W)
//                  gmvw_InputVTotalOrg		(W)
//                  gmvw_InputHStartOrg		(W)
//                  gmvw_InputVStartOrg		(W)
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void SaveOriginalValue(gmt_PHY_CH B_Channel)
{
   gmvw_InputHTotalOrg[B_Channel]	= stInput[B_Channel].Htotal;
   gmvw_InputVTotalOrg[B_Channel]	= stInput[B_Channel].Vtotal;
   gmvw_InputHActiveOrg[B_Channel]	= stInput[B_Channel].Hactive;
   gmvw_InputVActiveOrg[B_Channel]	= stInput[B_Channel].Vactive;
   gmvw_InputHStartOrg[B_Channel]	= stInput[B_Channel].Hstart;
   gmvw_InputVStartOrg[B_Channel]	= stInput[B_Channel].Vstart;
}


//******************************************************************************
//
// FUNCTION       : static void ModeSetCapturePrefs(gmt_PHY_CH B_Channel)
//
// USAGE          : Sets the input capture based on the current mode and user preferences
//
// INPUT          : B_Channel - CH_A or CH_B
//
// OUTPUT         : None
//
// GLOBALS        : B_AspectMethod		(W)
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
static void ModeSetCapturePrefs(gmt_PHY_CH B_Channel)
{
   if (B_Channel == gmvb_MainChannel)
      SelectAspectMain();
   else
   {
      SelectAspectPip();
   }
   SetCapture(B_Channel);

}

//******************************************************************************
//
// FUNCTION       : void ModeSetInputAttrPrefs(gmt_PHY_CH B_Channel)
//
// USAGE          : Sets the input attributes based on the mode information
//
// INPUT          : B_Channel - CH_A or CH_B
//
// OUTPUT         : None
//
// GLOBALS        : gmvw_InputModeAttr		(R)
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void ModeSetInputAttrPrefs(gmt_PHY_CH B_Channel)
{
#if 0//(FEATURE_HDMI==ENABLE) || (FEATURE_DISPLAY_PORT==ENABLE)
   gmt_GammaDescr ROM * ROM *ptr_GammaDescr = GetGammaTable();
#endif
   gm_SetChannelInputAttr(B_Channel, gmd_RGB_RANGE, gmd_FULL_RGB_RANGE);
   gm_SetChannelInputAttr(B_Channel, gmd_RGB_TO_YUV, FALSE);

gm_Printf("GetChannelConnType(B_Channel)=%d",GetChannelConnType(B_Channel));

   switch (GetChannelConnType(B_Channel))
   {
      case HDMI: //=====HDMI color space===============================================
       #if (FEATURE_HDMI==ENABLE)
         if (GetChannelProcType(B_Channel) == INTERNAL)
         {
            if (gm_IsHdmiTiming(gB_CurrentInputConnector[B_Channel]) && gm_HdmiIsStableState(gB_CurrentInputConnector[B_Channel]))
            {
               HDMI_AVI_Y B_HdmiAviY = gm_HdmiGetAviColorSpace(gB_CurrentInputConnector[B_Channel]);

               if (B_HdmiAviY == HDMI_AVI_Y_RGB)
               {
                  gmt_RGB_Quantization B_RGBQuanRange = gm_HdmiGetRGBQuanRange(gB_CurrentInputConnector[B_Channel]);

                  if (B_RGBQuanRange == gmd_RGB_QUANTIZATION_LIMITED)
                     gm_SetChannelInputAttr(B_Channel, gmd_RGB_RANGE, gmd_HDMI_RGB_RANGE);

                  // 20110708, Corrret RGB interlance mode
                  // if (gm_IsVideo(B_Channel))
                  stInput[B_Channel].ColorDomain = Input_RGB;
                  if (gm_IsVideo(B_Channel) || (gm_IsInterlaced(B_Channel)))
                  {
                     gm_SetChannelInputAttr(B_Channel, gmd_COLOR_SPACE, gmd_YUVSD);
                     gm_SetChannelInputAttr(B_Channel, gmd_DATA_WIDTH, gmd_444);
                     gm_SetChannelInputAttr(B_Channel, gmd_RGB_TO_YUV, TRUE);
                     stInput[B_Channel].ModeFlags|= gmd_MODE_SDVIDEO;
                     //gm_Printf("~~~~ 1 RGB to YUV ~~~~", 0);
                  }
                  else
                  {
                  #if 0 // Not to reduce to 16-bit for PIP
                     if (B_Channel != CH_A)
                     {  // PIP must be 16-bit YUV
                        gm_SetChannelInputAttr(B_Channel, gmd_COLOR_SPACE, gmd_YUVSD);
                        gm_SetChannelInputAttr(B_Channel, gmd_DATA_WIDTH, gmd_444);
                        gm_SetChannelInputAttr(B_Channel, gmd_RGB_TO_YUV, TRUE);
                        stInput[B_Channel].ModeFlags|= gmd_MODE_SDVIDEO;
                        //gm_Printf("~~~~ 2 RGB to YUV ~~~~", 0);
                     }
                     else
                  #endif
                     {
                        gm_SetChannelInputAttr(B_Channel, gmd_COLOR_SPACE, gmd_RGB);
                        gm_SetChannelInputAttr(B_Channel, gmd_DATA_WIDTH, gmd_444);
                        gm_SetChannelInputAttr(B_Channel, gmd_RGB_TO_YUV, FALSE);
                     }
                  }
               }
               else
               {
                  gmt_YCC_Quantization B_YCCQuanRange = gm_HdmiGetYCCQuanRange(gB_CurrentInputConnector[B_Channel]);

                  if (B_YCCQuanRange == gmd_YCC_QUANTIZATION_LIMITED)
                     gm_SetChannelInputAttr(B_Channel, gmd_RGB_RANGE, gmd_HDMI_RGB_RANGE);

                  //gm_Printf("~~~~ gm_GetSignalType 0x%x", gm_GetSignalType(B_Channel));
                  //gm_Printf("~~~~ B_HdmiAviY 0x%x", B_HdmiAviY);
                  
                  //if(B_YCCQuanRange == gmd_YCC_QUANTIZATION_FULL)
                  //{
                  //   Input_YUV_Range = Input_YUV_Full;
                  //   gm_Print("Pref Input_YUV_Range = Input_YUV_Full", 0);
                 // }
                  //else
                  //{
                  //   Input_YUV_Range = Input_YUV_Reduce;
                  //   gm_Print("Pref Input_YUV_Range = Input_YUV_Reduce", 0);
                  //}
                  
                  //stInput[B_Channel].ModeFlags|= gmd_MODE_HDVIDEO;
                  
                  gm_SetChannelInputAttr(B_Channel, gmd_COLOR_SPACE, gm_IsHDVideo(B_Channel) ? gmd_YUVHD : gmd_YUVSD);
                  gm_SetChannelInputAttr(B_Channel, gmd_DATA_WIDTH,
                                         (B_HdmiAviY == HDMI_AVI_Y_YUV422) ? gmd_422 :
                                         (B_HdmiAviY == HDMI_AVI_Y_YUV444) ? gmd_444 : 0);
                  gm_SetChannelInputAttr(B_Channel, gmd_RGB_TO_YUV, FALSE);

                #if 1
                  {
                     // Set colorimetry
                     BYTE B_HDMIColorimetry = HdmiGetColorimetry(gB_CurrentInputConnector[B_Channel]);

                     if((B_HDMIColorimetry == STHDMIRX_COLORIMETRY_STD_BT_601) ||
								(B_HDMIColorimetry == STHDMIRX_COLORIMETRY_STD_XVYCC_601) ||
								(B_HDMIColorimetry == STHDMIRX_COLORIMETRY_STD_sYCC_601) ||
								(B_HDMIColorimetry == STHDMIRX_COLORIMETRY_STD_AdobeYCC_601))
                     {
                        stInput[B_Channel].ModeFlags|= gmd_MODE_SDVIDEO;
                     }
                     else if((B_HDMIColorimetry == STHDMIRX_COLORIMETRY_STD_BT_709) ||
										(B_HDMIColorimetry == STHDMIRX_COLORIMETRY_STD_XVYCC_709))
                     {
                        stInput[B_Channel].ModeFlags|= gmd_MODE_HDVIDEO;
                     }
#if 0
                     // Check if gamma table is there or not ...
                     if (ptr_GammaDescr)
                     {
                        if (B_HDMIColorimetry == STHDMIRX_COLORIMETRY_STD_XVYCC_601)
                        {
                           gm_Printf(" STHDMIRX_COLORIMETRY_STD_XVYCC_601 1 ", 0);
                         #if (NON_SCALING_OUTPUT == 0)
                           stInput[B_Channel].Flags |= gmd_INPUT_xvYCC_601;
                           if (B_Channel == gmvb_MainChannel)
                              UserPrefColorSpace = ColorSpace_XvYcc;
                         #endif
                        }
                        else if (B_HDMIColorimetry == STHDMIRX_COLORIMETRY_STD_XVYCC_709)
                        {
                           gm_Printf(" STHDMIRX_COLORIMETRY_STD_XVYCC_709 1 ", 0);
                         #if (NON_SCALING_OUTPUT == 0)
                           stInput[B_Channel].Flags |= gmd_INPUT_xvYCC_709;
                           if (B_Channel == gmvb_MainChannel)
                              UserPrefColorSpace = ColorSpace_XvYcc;
                         #endif
                        }
                        else
                        {
                           //UserPrefColorSpace = ColorSpace_sRGB;
                           gm_Printf(" HDMI Colorimetry = %d ", B_HDMIColorimetry);
                        }
                     }
#endif
                  }
                #endif // 1
               }

               gm_SetChannelInputAttr(B_Channel, gmd_BLACK_OFFSET, FALSE);
               break;
            }
            // else (HDMI port but DVI signal)
            //    use same code in DVI case
         }
         else
            break;
#endif //#if (FEATURE_HDMI==ENABLE)

      case DVI: //=====DVI color space===============================================
         switch (stInput[B_Channel].ModeFlags & gmd_MODE_SIGNAL_MASK)
         {
            case gmd_MODE_GRAPHICS:
               gm_SetChannelInputAttr(B_Channel, gmd_COLOR_SPACE, gmd_RGB);
               gm_SetChannelInputAttr(B_Channel, gmd_BLACK_OFFSET, FALSE);
               //gm_SetChannelInputAttr(B_Channel, gmd_RGB_TO_YUV, FALSE);
               break;

            case gmd_MODE_SDVIDEO:
            case gmd_MODE_EDVIDEO:
               //convert to YUV in case deinterlacing is needed
               gm_SetChannelInputAttr(B_Channel, gmd_COLOR_SPACE, gmd_YUVSD);
               gm_SetChannelInputAttr(B_Channel, gmd_BLACK_OFFSET, FALSE);
               //gm_SetChannelInputAttr(B_Channel, gmd_RGB_TO_YUV, TRUE);
               break;

            case gmd_MODE_HDVIDEO:
            default:
               //convert to YUV in case deinterlacing is needed
               gm_SetChannelInputAttr(B_Channel, gmd_COLOR_SPACE, gmd_YUVHD);
               gm_SetChannelInputAttr(B_Channel, gmd_BLACK_OFFSET, FALSE);
               //gm_SetChannelInputAttr(B_Channel, gmd_RGB_TO_YUV, TRUE);
         }

         // 20110718
         if (stInput[B_Channel].ColorDomain == Input_YUV_Reduce)
         {
            gm_SetChannelInputAttr(B_Channel, gmd_RGB_TO_YUV, FALSE);
            gm_Print("Input color domain set to YUV", 0);
         }
         else
         {
            if ((stInput[B_Channel].ModeFlags & gmd_MODE_SIGNAL_MASK) == gmd_MODE_GRAPHICS)
            {
               gm_SetChannelInputAttr(B_Channel, gmd_RGB_TO_YUV, FALSE);
               //gm_Print("Input color domain set to RGB, output type set to Graphice", 0);
            }
            else
            {
               gm_SetChannelInputAttr(B_Channel, gmd_RGB_TO_YUV, TRUE);
               //gm_Print("Input color domain set to YUV, output type set to Video", 0);
            }
         }
         gm_SetChannelInputAttr(B_Channel, gmd_DATA_WIDTH, gmd_444);
         break;

#if (FEATURE_DISPLAY_PORT == ENABLE)
      case CT_DISPLAY_PORT: //=====CT_DISPLAY_PORT color space===============================================
         {
            {
               // Set colorimetry
               BYTE B_DPRXColorimetry = DpRxGetColorimetry(gB_CurrentInputConnector[B_Channel]);

               //gm_Print("ModeSetInputAttrPrefs B_DPRXColorimetry = .......... %d", B_DPRXColorimetry);

               // 444 or 422
               if ((B_DPRXColorimetry == STDPRX_COLORIMETRY_xvYCC601_422) ||
                     (B_DPRXColorimetry == STDPRX_COLORIMETRY_ITU601_422) ||
                     (B_DPRXColorimetry == STDPRX_COLORIMETRY_xvYCC709_422) ||
                     (B_DPRXColorimetry == STDPRX_COLORIMETRY_ITU709_422))
               {
                  gm_SetChannelInputAttr(B_Channel, gmd_DATA_WIDTH, gmd_422);
               }
               else
               {
                  gm_SetChannelInputAttr(B_Channel, gmd_DATA_WIDTH, gmd_444);
               }

               // Color Space
               if ((B_DPRXColorimetry == STDPRX_COLORIMETRY_xvYCC601_422) ||
                     (B_DPRXColorimetry == STDPRX_COLORIMETRY_ITU601_422) ||
                     (B_DPRXColorimetry == STDPRX_COLORIMETRY_xvYCC709_422) ||
                     (B_DPRXColorimetry == STDPRX_COLORIMETRY_ITU709_422) ||
                     (B_DPRXColorimetry == STDPRX_COLORIMETRY_xvYCC601_444) ||
                     (B_DPRXColorimetry == STDPRX_COLORIMETRY_ITU601_444) ||
                     (B_DPRXColorimetry == STDPRX_COLORIMETRY_xvYCC709_444) ||
                     (B_DPRXColorimetry == STDPRX_COLORIMETRY_ITU709_444) ||
                     (B_DPRXColorimetry == STDPRX_COLORIMETRY_Yonly))
               {
                  #ifdef ST_4K2K_93xx_BOARD // YUV Input Change to RGB via IMP_CSC
                  //gm_Printf("DP IMP CSC Override", 0);
                  if(!gm_IsInterlaced(B_Channel))
                  {
                     gm_SetChannelInputAttr(B_Channel, gmd_DATA_WIDTH, gmd_444);
                     gm_SetRegBitsWord(IMP_CONTROL, IMP_CSC_OVERRIDE);
                     gm_WriteRegWord(IMP_IN_OFFSET2, 0x600);
                     gm_WriteRegWord(IMP_IN_OFFSET3, 0x600);
                     
                     if(
                        (B_DPRXColorimetry == STDPRX_COLORIMETRY_xvYCC601_422) ||
                        (B_DPRXColorimetry == STDPRX_COLORIMETRY_xvYCC601_444) || 
                        (B_DPRXColorimetry == STDPRX_COLORIMETRY_ITU601_422) ||
                        (B_DPRXColorimetry == STDPRX_COLORIMETRY_ITU601_444) ||
                        (B_DPRXColorimetry == STDPRX_COLORIMETRY_Yonly)
                        )
                     {
                        if(
                           (B_DPRXColorimetry == STDPRX_COLORIMETRY_xvYCC601_422) ||
                           (B_DPRXColorimetry == STDPRX_COLORIMETRY_ITU601_422)
                           )
                        {
                           
                           gm_SetRegBitsWord(IMP_CSC_CTRL, IMP_CSC_IN_422TO444_EN);
                           
                        }
                        else
                        {
                           gm_ClearRegBitsWord(IMP_CSC_CTRL, IMP_CSC_IN_422TO444_EN);
                        }                     
   
                        // 601
                        if(stInput[B_Channel].ColorDomain == Input_YUV_Reduce)
                        {
                           gm_WriteRegWord(IMP_IN_OFFSET1, 0x7C0);
                           gm_WriteRegWord(IMP_MULT_COEF11, 0x12A);
                           gm_WriteRegWord(IMP_MULT_COEF12, 0x79C);
                           gm_WriteRegWord(IMP_MULT_COEF13, 0x730);
                           gm_WriteRegWord(IMP_MULT_COEF21, 0x12A);
                           gm_WriteRegWord(IMP_MULT_COEF22, 0x205);
                           gm_WriteRegWord(IMP_MULT_COEF23, 0x000);
                           gm_WriteRegWord(IMP_MULT_COEF31, 0x12A);
                           gm_WriteRegWord(IMP_MULT_COEF32, 0x000);
                           gm_WriteRegWord(IMP_MULT_COEF33, 0x199);                     
                        }
                        else
                        {
                           gm_WriteRegWord(IMP_IN_OFFSET1, 0);
                           gm_WriteRegWord(IMP_MULT_COEF11, 0x100);
                           gm_WriteRegWord(IMP_MULT_COEF12, 0x7AA);
                           gm_WriteRegWord(IMP_MULT_COEF13, 0x74D);
                           gm_WriteRegWord(IMP_MULT_COEF21, 0x100);
                           gm_WriteRegWord(IMP_MULT_COEF22, 0x1BC);
                           gm_WriteRegWord(IMP_MULT_COEF23, 0x000);
                           gm_WriteRegWord(IMP_MULT_COEF31, 0x100);
                           gm_WriteRegWord(IMP_MULT_COEF32, 0x000);
                           gm_WriteRegWord(IMP_MULT_COEF33, 0x15F);                     
                        }
                     }
                     else if(
                        (B_DPRXColorimetry == STDPRX_COLORIMETRY_xvYCC709_422) ||
                        (B_DPRXColorimetry == STDPRX_COLORIMETRY_xvYCC709_444) || 
                        (B_DPRXColorimetry == STDPRX_COLORIMETRY_ITU709_422) ||
                        (B_DPRXColorimetry == STDPRX_COLORIMETRY_ITU709_444)                                    
                        )
                     {
                        if(
                           (B_DPRXColorimetry == STDPRX_COLORIMETRY_xvYCC709_422) ||
                           (B_DPRXColorimetry == STDPRX_COLORIMETRY_ITU709_422)
                           )
                        {
                           gm_SetRegBitsWord(IMP_CSC_CTRL, IMP_CSC_IN_422TO444_EN);
                        }
                        else
                        {
                           gm_ClearRegBitsWord(IMP_CSC_CTRL, IMP_CSC_IN_422TO444_EN);
                        }
   
                        // 709
                        if(stInput[B_Channel].ColorDomain == Input_YUV_Reduce)
                        {
                           gm_WriteRegWord(IMP_IN_OFFSET1, 0x7C0);
                           gm_WriteRegWord(IMP_MULT_COEF11, 0x12A);
                           gm_WriteRegWord(IMP_MULT_COEF12, 0x7C9);
                           gm_WriteRegWord(IMP_MULT_COEF13, 0x777);
                           gm_WriteRegWord(IMP_MULT_COEF21, 0x12A);
                           gm_WriteRegWord(IMP_MULT_COEF22, 0x21D);
                           gm_WriteRegWord(IMP_MULT_COEF23, 0x000);
                           gm_WriteRegWord(IMP_MULT_COEF31, 0x12A);
                           gm_WriteRegWord(IMP_MULT_COEF32, 0x000);
                           gm_WriteRegWord(IMP_MULT_COEF33, 0x1CB);                     
                        }
                        else
                        {
                           gm_WriteRegWord(IMP_IN_OFFSET1, 0);
                           gm_WriteRegWord(IMP_MULT_COEF11, 0x100);
                           gm_WriteRegWord(IMP_MULT_COEF12, 0x7D1);
                           gm_WriteRegWord(IMP_MULT_COEF13, 0x78B);
                           gm_WriteRegWord(IMP_MULT_COEF21, 0x100);
                           gm_WriteRegWord(IMP_MULT_COEF22, 0x1D1);
                           gm_WriteRegWord(IMP_MULT_COEF23, 0x000);
                           gm_WriteRegWord(IMP_MULT_COEF31, 0x100);
                           gm_WriteRegWord(IMP_MULT_COEF32, 0x000);
                           gm_WriteRegWord(IMP_MULT_COEF33, 0x18A);                     
                        }
                     }                  
                     gm_SetChannelInputAttr(B_Channel, gmd_COLOR_SPACE, gmd_RGB);
                     if(stInput[B_Channel].ColorDomain != Input_RGB)
                     {
                        stInput[B_Channel].ColorDomain = Input_RGB;
                        //gm_Printf("4k2k as Graphic Mode always !!!", 0);
                     }
                     gm_SyncUpdate(B_Channel, INPUT_CLOCK);
                  }
                  else                  
                  #endif
                  {
                     if(
                        (B_DPRXColorimetry == STDPRX_COLORIMETRY_xvYCC709_422) ||
                        (B_DPRXColorimetry == STDPRX_COLORIMETRY_xvYCC709_444) || 
                        (B_DPRXColorimetry == STDPRX_COLORIMETRY_ITU709_422) ||
                        (B_DPRXColorimetry == STDPRX_COLORIMETRY_ITU709_444)                                    
                        )
                     {
                        stInput[B_Channel].ModeFlags|= gmd_MODE_HDVIDEO;
                        //gm_Printf("~~~~ DP 709 1 ", 0);
                     }  
						   else
                     {
                        stInput[B_Channel].ModeFlags|= gmd_MODE_SDVIDEO;
                        //gm_Printf("~~~~ DP 601 1 ", 0);
                     }
                     gm_SetChannelInputAttr(B_Channel, gmd_COLOR_SPACE, gm_IsHDVideo(B_Channel) ? gmd_YUVHD : gmd_YUVSD);
                  }
               }
               else
               {
                  #ifdef ST_4K2K_93xx_BOARD // YUV Input Change to RGB via IMP_CSC
                  //gm_Printf("DP RGB", 0);
                  gm_ClearRegBitsWord(IMP_CONTROL, IMP_CSC_OVERRIDE);
                  gm_ClearRegBitsWord(IMP_CSC_CTRL, IMP_CSC_IN_422TO444_EN);
                  gm_SyncUpdate(B_Channel, INPUT_CLOCK);
                  #endif
                  //SWENG_0601: Temporary make it work for Briing up
                  stInput[B_Channel].ModeFlags &=  ~gmd_MODE_SIGNAL_MASK;
                  stInput[B_Channel].ModeFlags |=gmd_MODE_GRAPHICS;
                  gm_SetChannelInputAttr(B_Channel, gmd_COLOR_SPACE, gmd_RGB);
                  //gm_SetChannelInputAttr(B_Channel, gmd_DATA_WIDTH, gmd_444);
                  // 20110708, Corrret RGB interlance mode
                  // if (gm_IsVideo(B_Channel))
                  if (gm_IsInterlaced(B_Channel))
                  {
                     gm_SetChannelInputAttr(B_Channel, gmd_COLOR_SPACE, gmd_YUVSD);
                     gm_SetChannelInputAttr(B_Channel, gmd_RGB_TO_YUV, TRUE);
                     //gm_Printf("~~~~ 1 RGB to YUV ~~~~", 0);
                  }
                  //else
                  //   gm_SetChannelInputAttr(B_Channel, gmd_RGB_TO_YUV, FALSE);

                  stInput[B_Channel].ColorDomain = Input_RGB;
               }
#if 0
               // Check if gamma table is there or not ...
               if (ptr_GammaDescr)
               {
                  if ((B_DPRXColorimetry == STDPRX_COLORIMETRY_xvYCC601_422) ||
                        (B_DPRXColorimetry == STDPRX_COLORIMETRY_xvYCC601_444))
                  {
                     gm_Printf(" gmd_INPUT_xvYCC_601 1 %d", B_DPRXColorimetry);
                     stInput[B_Channel].Flags |= gmd_INPUT_xvYCC_601;
                     if (B_Channel == gmvb_MainChannel)
                        UserPrefColorSpace = ColorSpace_XvYcc;
                  }
                  else if ((B_DPRXColorimetry == STDPRX_COLORIMETRY_xvYCC709_422) ||
                           (B_DPRXColorimetry == STDPRX_COLORIMETRY_xvYCC709_444))
                  {
                     gm_Printf(" STDPRX_COLORIMETRY_xvYCC709_422 1 %d", B_DPRXColorimetry);
                     stInput[B_Channel].Flags |= gmd_INPUT_xvYCC_709;
                     if (B_Channel == gmvb_MainChannel)
                        UserPrefColorSpace = ColorSpace_XvYcc;
                  }
                  else
                  {
                     //UserPrefColorSpace = ColorSpace_sRGB;
                     gm_Printf(" DP RX Colorimetry = %d ", B_DPRXColorimetry);
                  }
               }
#endif 		   
            }
            gm_SetChannelInputAttr(B_Channel, gmd_BLACK_OFFSET, FALSE);

         } // case CT_DISPLAY_PORT
         break;
#endif // (FEATURE_DISPLAY_PORT == ENABLE)
      case DIPX:
         {
#if 0//def ANX9021_DEV_ID
            gmt_HDMI_SIG_FORMAT B_Format = (ANX9021_IsSignalHDMI() && ANX9021_IsSignalStable()) ?
                                           ANX9021_GetBoardOutputFormat() : gmd_HDMI_RGB;
            switch (B_Format)
            {
               case gmd_HDMI_RGB:
                  gm_SetChannelInputAttr(B_Channel, gmd_DATA_WIDTH, gmd_444);
                  stInput[B_Channel].ColorDomain = Input_RGB;
                  if (gm_IsVideo(B_Channel))
                  {
                     gm_SetChannelInputAttr(B_Channel, gmd_COLOR_SPACE, gm_IsHDVideo(B_Channel) ? gmd_YUVHD : gmd_YUVSD);
                     gm_SetChannelInputAttr(B_Channel, gmd_RGB_TO_YUV, TRUE);
                     //gm_SetChannelInputAttr(B_Channel, gmd_RGB_RANGE, gmd_HDMI_RGB_RANGE);
                  }
                  else
                  {
                     gm_SetChannelInputAttr(B_Channel, gmd_COLOR_SPACE, gmd_RGB);
                     gm_SetChannelInputAttr(B_Channel, gmd_RGB_TO_YUV, FALSE);
                  }
                  break;

               case gmd_HDMI_YUV422_SD:
                  stInput[B_Channel].ColorDomain = Input_YUV_Reduce;
                  if (gm_IsInterlaced(B_Channel))
                     stInput[B_Channel].ModeFlags |= gmd_MODE_SDVIDEO;
                  else
                     stInput[B_Channel].ModeFlags |= gmd_MODE_EDVIDEO;
                  gm_SetChannelInputAttr(B_Channel, gmd_COLOR_SPACE, gmd_YUVSD);
                  gm_SetChannelInputAttr(B_Channel, gmd_DATA_WIDTH, gmd_422);
                  gm_SetChannelInputAttr(B_Channel, gmd_RGB_TO_YUV, FALSE);
                  break;

               case gmd_HDMI_YUV422_HD:
                  stInput[B_Channel].ColorDomain = Input_YUV_Reduce;
                  stInput[B_Channel].ModeFlags |= gmd_MODE_HDVIDEO;
                  gm_SetChannelInputAttr(B_Channel, gmd_COLOR_SPACE, gmd_YUVHD);
                  gm_SetChannelInputAttr(B_Channel, gmd_DATA_WIDTH, gmd_422);
                  gm_SetChannelInputAttr(B_Channel, gmd_RGB_TO_YUV, FALSE);
                  break;

               case gmd_HDMI_YUV444_SD:
                  stInput[B_Channel].ColorDomain = Input_YUV_Reduce;
                  if (gm_IsInterlaced(B_Channel))
                     stInput[B_Channel].ModeFlags |= gmd_MODE_SDVIDEO;
                  else
                     stInput[B_Channel].ModeFlags |= gmd_MODE_EDVIDEO;
                  gm_SetChannelInputAttr(B_Channel, gmd_COLOR_SPACE, gmd_YUVSD);
                  gm_SetChannelInputAttr(B_Channel, gmd_DATA_WIDTH, gmd_444);
                  gm_SetChannelInputAttr(B_Channel, gmd_RGB_TO_YUV, FALSE);
                  break;

               case gmd_HDMI_YUV444_HD:
               default:
                  stInput[B_Channel].ColorDomain = Input_YUV_Reduce;
                  stInput[B_Channel].ModeFlags |= gmd_MODE_HDVIDEO;
                  gm_SetChannelInputAttr(B_Channel, gmd_COLOR_SPACE, gmd_YUVHD);
                  gm_SetChannelInputAttr(B_Channel, gmd_DATA_WIDTH, gmd_444);
                  gm_SetChannelInputAttr(B_Channel, gmd_RGB_TO_YUV, FALSE);
                  break;
            }

            gm_SetChannelInputAttr(B_Channel, gmd_BLACK_OFFSET, FALSE);
#else // #ifdef ANX9021_DEV_ID	
/* leo del
            switch (stInput[B_Channel].ModeFlags & gmd_MODE_SIGNAL_MASK)
            {
               case gmd_MODE_SDVIDEO:
               case gmd_MODE_EDVIDEO:
                  stInput[B_Channel].ColorDomain = Input_YUV_Reduce;
                  gm_SetChannelInputAttr(B_Channel, gmd_COLOR_SPACE, gmd_YUVSD);	// Input is YUV SD
                  break;

               case gmd_MODE_HDVIDEO:
                  stInput[B_Channel].ColorDomain = Input_YUV_Reduce;
                  gm_SetChannelInputAttr(B_Channel, gmd_COLOR_SPACE, gmd_YUVHD);	// Input is YUV HD
                  break;
            }
            gm_SetChannelInputAttr(B_Channel, gmd_DATA_WIDTH, gmd_444);
            gm_SetChannelInputAttr(B_Channel, gmd_RGB_TO_YUV, FALSE);
            gm_SetChannelInputAttr(B_Channel, gmd_BLACK_OFFSET, FALSE);
*/
                  stInput[B_Channel].ColorDomain = Input_YUV_Reduce;
                  stInput[B_Channel].ModeFlags |= gmd_MODE_SDVIDEO;
                  gm_SetChannelInputAttr(B_Channel, gmd_COLOR_SPACE, gmd_YUVSD);
                  gm_SetChannelInputAttr(B_Channel, gmd_DATA_WIDTH, gmd_422);
                  gm_SetChannelInputAttr(B_Channel, gmd_RGB_TO_YUV, FALSE);
			
#endif // #ifdef ANX9021_DEV_ID
         }
         break;
         //=====Non-HDMI/DVI/DP/DIPX port========================================================
      default:
         switch (stInput[B_Channel].ModeFlags & gmd_MODE_SIGNAL_MASK)
         {
            case gmd_MODE_GRAPHICS:
               if (B_Channel == CH_A)
               {
                  if (gm_IsInterlaced(B_Channel))
                  {
                     gm_SetChannelInputAttr(B_Channel, gmd_COLOR_SPACE, gmd_YUVHD);  // Input is YUV
                     //gm_SetChannelInputAttr(B_Channel, gmd_RGB_TO_YUV, TRUE);
                  }
                  else
                  {
                     gm_SetChannelInputAttr(B_Channel, gmd_COLOR_SPACE, gmd_RGB);	// Input is RGB
                     //gm_SetChannelInputAttr(B_Channel, gmd_RGB_TO_YUV, FALSE);
                  }
                  gm_SetChannelInputAttr(B_Channel, gmd_BLACK_OFFSET, FALSE);
               }
               else
               {
#if (FEATURE_PIP_RGB_COLORSPACE == DISABLE)
                  // For FLI8532,FLI8538 PIP must be YUV
                  gm_SetChannelInputAttr(B_Channel, gmd_COLOR_SPACE, gmd_YUVSD);
                  //gm_SetChannelInputAttr(B_Channel, gmd_RGB_TO_YUV, TRUE);
#else
                  gm_SetChannelInputAttr(B_Channel, gmd_COLOR_SPACE, gmd_RGB);
                  //gm_SetChannelInputAttr(B_Channel, gmd_RGB_TO_YUV, FALSE);
#endif
                  gm_SetChannelInputAttr(B_Channel, gmd_BLACK_OFFSET, FALSE);
               }
               break;

            case gmd_MODE_SDVIDEO:
            case gmd_MODE_EDVIDEO:
               //VGA port + Video + DSS -> RGB
               if (((GetChannelConnName(B_Channel) == VGA1)
                     &&  !(stInput[B_Channel].ModeFlags & gmd_MODE_SOG))
                     ||(GetChannelConnType(B_Channel) == CT_DISPLAY_PORT))
               {
                  // convert from RGB to YUV so that deinterlacing can be done
                  gm_SetChannelInputAttr(B_Channel, gmd_COLOR_SPACE, gmd_YUVSD);
                  gm_SetChannelInputAttr(B_Channel, gmd_BLACK_OFFSET, FALSE);
                  //gm_SetChannelInputAttr(B_Channel, gmd_RGB_TO_YUV, TRUE);
               }
               else//assumed to be YUV
               {
                  gm_SetChannelInputAttr(B_Channel, gmd_COLOR_SPACE, gmd_YUVSD);	// Input is YUV SD
                  gm_SetChannelInputAttr(B_Channel, gmd_BLACK_OFFSET, TRUE);
                  //gm_SetChannelInputAttr(B_Channel, gmd_RGB_TO_YUV, FALSE);
               }
               break;

            case gmd_MODE_HDVIDEO:
            default:
               //VGA port + Video + DSS -> RGB
               if (((GetChannelConnName(B_Channel) == VGA1)
                     &&  !(stInput[B_Channel].ModeFlags & gmd_MODE_SOG))
                     ||(GetChannelConnType(B_Channel) == CT_DISPLAY_PORT))
               {
                  // convert from RGB to YUV so that deinterlacing can be done
                  gm_SetChannelInputAttr(B_Channel, gmd_COLOR_SPACE, gmd_YUVHD);
                  gm_SetChannelInputAttr(B_Channel, gmd_BLACK_OFFSET, FALSE);
                  //gm_SetChannelInputAttr(B_Channel, gmd_RGB_TO_YUV, TRUE);
               }
               else//assumed to be YUV
               {
                  gm_SetChannelInputAttr(B_Channel, gmd_COLOR_SPACE, gmd_YUVHD);	// Input is YUV HD
                  gm_SetChannelInputAttr(B_Channel, gmd_BLACK_OFFSET, TRUE);
                  //gm_SetChannelInputAttr(B_Channel, gmd_RGB_TO_YUV, FALSE);
               }
         }

         // 20110718
         if(stInput[B_Channel].ColorDomain != Input_RGB)
         {
            gm_SetChannelInputAttr(B_Channel, gmd_RGB_TO_YUV, FALSE);
            gm_Print("Input color domain set to YUV", 0);
         }
         else
         {
            if ((stInput[B_Channel].ModeFlags & gmd_MODE_SIGNAL_MASK) == gmd_MODE_GRAPHICS)
            {
               gm_SetChannelInputAttr(B_Channel, gmd_RGB_TO_YUV, FALSE);
               gm_Print("Input color domain set to RGB, output type set to Graphice", 0);
            }
            else
            {
               gm_SetChannelInputAttr(B_Channel, gmd_RGB_TO_YUV, TRUE);
               gm_Print("Input color domain set to YUV, output type set to Video", 0);
            }
         }
#if 0
         // Special case for COMP signal through decoder (480i and 576i)
         if ((gm_GetComponentRouteMode() == gmd_ComponentThroughDecoder) &&
               (gm_IsSDVideo(B_Channel)) && (stInput[B_Channel].ModeFlags & gmd_MODE_SOG))
            gm_SetChannelInputAttr(B_Channel, gmd_DATA_WIDTH, gmd_444); //gmd_422);
         else
#endif
            gm_SetChannelInputAttr(B_Channel, gmd_DATA_WIDTH,
                                   (gm_IsInputTripleADC(B_Channel)||(GetChannelConnType(B_Channel) == CT_DISPLAY_PORT)) ?
                                   gmd_444 : gmd_422);
   }
}


//******************************************************************************
//
// FUNCTION       : void ModeSetPrefs(gmt_PHY_CH B_Channel)
//
// USAGE          : Sets the prefered input attributes and input capture window for the
//                  mode.  This function should typically be called after Mode Determination.
//
// INPUT          : B_Channel - CH_A or CH_B
//
// OUTPUT         : None
//
// GLOBALS        : gmvw_InputHStart		(W)
//                  gmvw_InputVStart		(W)
//                  gmvw_InputHTotal		(W)
//                  gmvsw_InputVSyncOffset	(W)
//                  gmvsw_InputHSyncOffset	(W)
//                  gmvb_InputSamplingPhase	(W)
//                  gmvw_InputVFreq			(W)
//                  gmvw_InputHFreq			(W)
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void ModeSetPrefs(gmt_PHY_CH B_Channel)
{
   ModeDependentLookUpType LookUpType;

   // Set the input attributes prefered for the current mode
   ModeSetInputAttrPrefs(B_Channel);

#ifdef CHANNEL_CLONE
//Copy the channel input properties under CLONE condition
   if ((UserPrefCurrentInputMain == UserPrefCurrentInputPip) &&
         gm_IsModeComplete(gm_OtherChannel(B_Channel)))
      gm_CopyChannelInputAttr(gm_OtherChannel(B_Channel), B_Channel);
#endif

#if (FEATURE_WSS == ENABLE)
   // Set WSS measurement
   WssDetectSetup(B_Channel);
#endif

   // Image Correction
   gmvsw_InputVSyncOffset[B_Channel] = 0;
   gmvsw_InputHSyncOffset[B_Channel] = 0;

#if FEATURE_AIP == ENABLE
   // Graphics Mode - User Settings
   if (gm_IsInputTripleADC(B_Channel))
   {
      // Defaults
      UserPrefHStart = stInput[B_Channel].Hstart;
      UserPrefVStart = stInput[B_Channel].Vstart;
      UserPrefHTotal = stInput[B_Channel].Htotal;
      UserPrefHSyncPhase = gmvb_InputSamplingPhase[B_Channel];
      UserPrefHSyncDelay = 0;
      UserPrefVSyncDelay = 0;

      //if (gm_IsGraphics(B_Channel) == TRUE)
      {
         LookUpType.W_Hfreq = stInput[B_Channel].Hfrq;
         LookUpType.W_Vfreq = stInput[B_Channel].Vfrq;
         LookUpType.W_Vtotal = stInput[B_Channel].Vtotal;

         // Retrieve
         RetrieveGraphicsSettings(B_Channel, &LookUpType);

         // Set Globals
         stInput[B_Channel].Hstart = UserPrefHStart;
         stInput[B_Channel].Vstart = UserPrefVStart;
         stInput[B_Channel].Htotal = UserPrefHTotal;
         gmvb_InputSamplingPhase[B_Channel] = UserPrefHSyncPhase;
         gmvsw_InputHSyncOffset[B_Channel] = UserPrefHSyncDelay;
         gmvsw_InputVSyncOffset[B_Channel] = UserPrefVSyncDelay;
      }
   }

#endif // FEATURE_AIP == ENABLE

#ifdef USE_LETTERBOX_DETECTION
   //
   // Setup LetterBox Detection (Can only be call after SaveOriginalValue())
   //
   LetterBoxSetup(B_Channel);
#endif

   //
   // Set the input capture window based on the current mode and the
   // current user preferences
   //
   ModeSetCapturePrefs(B_Channel);

   // Set the memory capture preferences
   MemoryAllocate(B_Channel);

#if (FEATURE_AFP22DETECTVM_PATCH == REV2)
   // Set 50Hz (PAL) flag to select proper AFM VM detection algorithm
   if ( B_Channel == CH_A )
   {
      gm_AfpSetVmPalFlag( ((stInput[CH_A].Vfrq > 480) &&\
                           (stInput[CH_A].Vfrq < 520) )?1:0);
   }
#endif
}
/*********************************  END  **************************************/

