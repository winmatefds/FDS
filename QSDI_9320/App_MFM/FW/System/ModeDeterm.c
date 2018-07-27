
/*
    $Workfile:   ModeDeterm.c  $
    $Revision: 1.106 $
    $Date: 2013/07/26 06:24:31 $
*/
#define __MODEDETERM_C__

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
// MODULE:      ModeDeterm.c
//
//******************************************************************************

//******************************************************************************
//  I N C L U D E    F I L E S
//******************************************************************************

#include <math.h>
#include "System\all.h"
#include "StdLibInc\gm_Register.h"
#include "Board\PortAttrib.h"
#include "StdLibInc\gm_StdLib.h"
#define _DEBUG_MSG

#ifdef _DEBUG_MSG
#define msg(x,y)        gm_Print(x,y)
#define msg_ii(x,y,z)   gm_Print(x,y,z)
#else
#define msg(x,y)
#define msg_ii(x,y,z)
#endif

//******************************************************************************
//  D E F I N E S
//******************************************************************************
#define AustralianModeInterlaceDetect_Enable   1
// Max horizontal and vertical frequencies
#define MAX_HFREQ    940      //in 100Hz units
#define MAX_VFREQ    1200     //in 0.1Hz units

//input H/V freqnecy values extremums
#define MIN_INPUT_H_FREQ    74      //in 100Hz units (74 = 7.4KHz)
#define MAX_INPUT_H_FREQ    1710    //in 100Hz units
#define MIN_INPUT_V_FREQ    230     //in 0.1Hz units (230=23Hz)
#define MAX_INPUT_V_FREQ    2450    //in 0.1Hz units

//******************************************************************************
//  E X T E R N A L    V A R I A B L E S
//******************************************************************************
extern StandardModeStruct ROM * ROM LocalVGAModeTable_SOG[];
extern StandardModeStruct ROM * ROM LocalVGAModeTable_60[];
extern StandardModeStruct ROM * ROM LocalVGAModeTable_75[];
extern StandardModeStruct ROM * ROM LocalVGAModeTable_85[];

//******************************************************************************
//  G L O B A L   V A R I A B L E S
//******************************************************************************
WORD Real_Input_H;
WORD Real_Input_V;
WORD Real_HTotal = 0;
WORD Real_VTotal = 0;
//*****************************************************************************
//  C O D E
//*****************************************************************************

DWORD UpdatePixelClkFromAVDDS(gmt_PHY_CH B_Channel)
{
   WORD reg_AVDDS_CUR_FREQ;
   WORD reg_AVDDS_CONTROL1;
   DWORD dw_Avdds;
#if RCLK_FREQ_ARGS == RCLKSEL_432M
    WORD Factor=1648;                  //Factor= (RCLK/2^24)<<6
#elif RCLK_FREQ_ARGS == RCLKSEL_378M
    WORD Factor=1442;                  
#elif RCLK_FREQ_ARGS == RCLKSEL_270M
    WORD Factor=1030; 
#endif

   if(B_Channel==CH_A)
   {
      reg_AVDDS_CUR_FREQ=AVDDS1_CUR_FREQ;
      reg_AVDDS_CONTROL1=AVDDS1_CONTROL1;
   }
   else 
   {
      reg_AVDDS_CUR_FREQ=AVDDS2_CUR_FREQ;      
      reg_AVDDS_CONTROL1=AVDDS2_CONTROL1;
   }
      
   gm_WriteRegDWord_LE(reg_AVDDS_CUR_FREQ,0xFFFFFFFFUL);
#if (FEATURE_HDMI == ENABLE)
   if((gm_GetHdmiDviDigitalID(gB_CurrentInputConnector[B_Channel])==DID_DVI0)||(gm_GetHdmiDviDigitalID(gB_CurrentInputConnector[B_Channel])==DID_DVI_DUAL))
   {
      dw_Avdds=gm_ReadRegDWord_LE(reg_AVDDS_CONTROL1+4 /*AVDDS1_INI_FREQ*/)>>5;  //DVI block does not actually use AVDDS, but HdmiRxLib will write init freq into AVDDS_INI_FREQ
      if(gm_ReadRegWord(DUAL_DVI0_CTRL)==9)  //Dual DVI signal
         dw_Avdds*=2;
      return (dw_Avdds*Factor); 
   }
#endif
   dw_Avdds=gm_ReadRegDWord_LE(reg_AVDDS_CUR_FREQ)>>6;
   dw_Avdds*=Factor;
  
#if (FEATURE_HDMI == ENABLE)
   if(gm_GetHdmiDviDigitalID(gB_CurrentInputConnector[B_Channel])==DID_HDMI)
   {
      if(gm_IsHdmiTiming(gB_CurrentInputConnector[B_Channel]))
         dw_Avdds/=(gm_HdmiGetAviPixelRepetition(gB_CurrentInputConnector[B_Channel])+1);
   }
#endif
   return dw_Avdds;
}



//******************************************************************************
//
// FUNCTION       : static BOOL OverlapModeValidator(gmt_PHY_CH B_Channel, StandardModeStruct ROM * Stp_ModePtr)
//
// USAGE          : This function takes in a pointer to an overlap mode in the mode table
//                  and attempts to determine if it matches the current mode.
//
// INPUT          : B_Channel - Channel
//                  Stp_ModePtr - pointer to the mode to be validated
//
// OUTPUT         : TRUE/FALSE
//
// GLOBALS        :
//
// USED_REGS      :
//
// PRE_CONDITION  : None
//
// POST_CONDITION : None
//
//******************************************************************************
#if FEATURE_AIP == ENABLE
static BOOL OverlapModeValidator(gmt_PHY_CH B_Channel,
                                 StandardModeStruct ROM* pMode, BOOL Interlace, gmt_InputMeasurementType MeasType,
                                 WORD Hactive, gmt_MODE_FLAGS ModeFlags)
{
   // Check for overlap between 480p & graphics 640x480@60Hz
   if (pMode->W_OverlapType & gmd_MODE_OVERLAPPED_480P)
   {
      if (GetChannelConnType(B_Channel) == VGA)
         return (pMode->W_StdWidth == OVERLAP_480P_VGA60);
      else if (MeasType == gmt_IBDmeasurement)
         return (Hactive == pMode->W_StdWidth);
      else
         return (pMode->W_StdWidth == 720);
   }
   // check for overlap in between 576p, 1080i 50, 1152iLA, 1152iSH and 1152iLH
   else if ( pMode->W_OverlapType & gmd_MODE_OVERLAPPED_1080_576)
   {
      if (MeasType == gmt_IBDmeasurement)
         return (Hactive == pMode->W_StdWidth);
      else
      {   // use the raw measurement, not the re-generated
         // TODO: Temp solution. 576p is non-interlaced.
         if ((!Interlace) && (Hactive < 720))
         {
            return FALSE;
         }
         gm_ClearRegBitsWord(AIP_CSYNC_CONTROL, AIP_IFM_HSYNC_SEL);//SCR#3712,JoeR
         gm_Delay10ms(6);
         gm_SetRegBitsWord(input_status[B_Channel], ip_intlc_err);
         gm_Delay10ms(6);
         if ((gm_ReadRegWord(input_status[B_Channel]) & ip_intlc_err) == 0)
            return (pMode->W_ModeType & gmd_MODE_INTERLACED);
         else if ((pMode->W_StdHTotal == OVERLAP_1152I) ||//1152i
                  (pMode->W_StdHTotal == 864)) //576p
         {
            BOOL res;
            gm_SetRegBitsWord(AIP_CSYNC_CONTROL, AIP_IFM_HSYNC_SEL);//SCR#3893,JoeR
            gm_Delay10ms(6);
            //Set up the AFE for australian mode interlaced detection
            if (B_Channel == CH_A)
            {
               gm_ClearRegBitsWord(MIFM_CTRL, MIFM_FIELD_DETECT_MODE);
               gm_SetupIFMChange (MIFM, 0x422D);// Set MIFM_LOWER_ODD to 2 and MIFM_UPPER_ODD to 4
            }
            else
            {
               gm_ClearRegBitsWord(PIFM_CTRL, MIFM_FIELD_DETECT_MODE);
               gm_SetupIFMChange (PIFM, 0x422D);// Set MIFM_LOWER_ODD to 2 and MIFM_UPPER_ODD to 4
            }
            //set up AFE to 1152i
            // Program mode table value into driver parameters
            stInput[B_Channel].ModeFlags = gmd_MODE_HDVIDEO | gmd_MODE_SOG | gmd_MODE_INTERLACED;
            stInput[B_Channel].Htotal = 1188;
            stInput[B_Channel].Vtotal = 625;
            stInput[B_Channel].Vfrq = 500;
            stInput[B_Channel].Hfrq = 312;
            // Copy Input Detected Syncs
            //gm_AipSetClamping(B_Channel);	// Disabled by Cruise 110607
            gm_ClearRegBitsWord(AIP_CSYNC_CONTROL, AIP_CSYNC_VS_MASK_EN);  //SCR#3712,JoeR
            gm_Delay10ms(6);
            gm_SetRegBitsWord(input_status[B_Channel], ip_intlc_err);
            gm_Delay10ms(6);

            res = (((gm_ReadRegWord(input_status[B_Channel]) & ip_intlc_err) == 0)
                   == (pMode->W_StdHTotal == OVERLAP_1152I)) ? TRUE : FALSE;

            //Reset standard interlaced detection - RemoveAustralianModeInterlaceDetect
            if (B_Channel == CH_A)
            {
               gm_SetRegBitsWord(MIFM_CTRL, MIFM_FIELD_DETECT_MODE);
               gm_SetupIFMChange (MIFM, 0x002D);
            }
            else
            {
               gm_SetRegBitsWord(PIFM_CTRL, MIFM_FIELD_DETECT_MODE);
               gm_SetupIFMChange (PIFM, 0x002D);
            }
            gm_SetRegBitsWord(AIP_CSYNC_CONTROL, AIP_CSYNC_VS_MASK_EN); //SCR#3712,JoeR
            gm_SetRegBitsWord(input_status[B_Channel], ip_intlc_err);
            return res;
         }
         else
            return FALSE;
      }
   }
   // check for overlap between 640x350,720x350,640x400,720x400
   else if (pMode->W_OverlapType & gmd_MODE_OVERLAPPED_GR640_GR720)
   {
      if ((GetChannelConnType(B_Channel) == VGA) ||
            (GetChannelConnType(B_Channel) == DVI) ||
            (GetChannelConnType(B_Channel) == HDMI))
      {
         if (ModeFlags & gmd_MODE_SOG)  // For overlapped SOG signals
         {
            if (((B_Overlap_Height == OVERLAP_GR350) && (pMode->W_StdHeight != 350)) ||
                  ((B_Overlap_Height == OVERLAP_GR400) && (pMode->W_StdHeight != 400)))
               return FALSE;
         }
         else
         {
            if (ModeFlags & gmd_MODE_NEG_VSYNC)
            {
               if (pMode->W_StdHeight != 350) // this mode has a height of 350
                  return FALSE;
            }
            else
            {   // this mode has a height of 400
               if (pMode->W_StdHeight != 400)
                  return FALSE;
            }
         }
         return (pMode->W_StdWidth == OVERLAP_GR640_GR720);
      }
   }
   else if (pMode->W_OverlapType & gmd_MODE_OVERLAPPED_240p_480i)
   {
      if (Interlace)
         return (pMode->W_ModeType & gmd_MODE_INTERLACED);
   }
   return FALSE;
}

#endif // FEATURE_AIP == ENABLE

//******************************************************************************
//
// FUNCTION       : BOOL ModeDetermine(gmt_PHY_CH B_Channel)
//
// USAGE          : Mode Determination functions, decide what is the mode
//                  base on input sync signal on each channel.
//
// INPUT          : B_Channel - CH_A or CH_B
//                     Note : The value of CH_A or CH_B is stored in
//                     gmvb_MainChannel and gmvb_PipChannel
//
// OUTPUT         : TRUE  - valid timing can be display
//                  FALSE - invalid timing
//
// GLOBALS        : gmvw_InputModeAttr        (W)
//                  gmvw_InputSyncFlags        (W)
//                  st_InputMeasurement.W_HFreq    (R/W)
//                  st_InputMeasurement.W_VFreq    (R/W)
//                  st_InputMeasurement.W_VTotal    (R/W)
//                  Stp_StandardModeTable    (R/W)
//                  gmvw_InputHStart        (W)
//                  gmvw_InputVStart        (W)
//                  gmvw_InputHActive    (W)
//                  gmvw_InputVActive    (W)
//
// USED_REGS      :
//
// PRE_CONDITION  : None
//
// POST_CONDITION : None
//
//******************************************************************************
BOOL ModeDetermine(gmt_PHY_CH B_Channel)
{
   WORD inpHfrq;
   WORD inpVfrq;
   WORD inpVtotal;
   WORD inpVsyncPulse;
   WORD inpHsyncPulse;
   WORD inpHstart;
   WORD inpVstart;
   WORD inpHtotal;
   WORD inpHactive;
   WORD inpVactive;
   BYTE inpModeIndex = 0;
   gmt_MODE_FLAGS inpModeFlags;

   StandardModeStruct ROM * ROM * pModeTable;
   gmt_UserDefConnNames B_Connector = GetChannelConnName(B_Channel);
#if (FEATURE_AIP == ENABLE)
   gmt_ConnTypes B_InputType = GetChannelConnType(B_Channel);
#endif
   gmt_InputMeasurementType inpMeasType = AppPortAttrib[B_Connector].B_InputMeasurementType;

#if (FEATURE_DISPLAY_PORT==ENABLE)||(FEATURE_HDMI==ENABLE)
   gmt_FrontEndSignalTiming_t InTiming;   
#endif   

//   DWORD preT=gm_GetSystemTime();

#if (DEBUG_DISPLAY_TIME_MEASUREMENT == ENABLE)
   dtm_msg("DTM> MDetM_B_T(%Ld)",gm_GetSystemTime());
#endif


   inpModeFlags = gmd_MODE_UNKNOWN;
   gmvw_InputSyncFlags[B_Channel] = AppPortAttrib[B_Connector].B_SyncFlags;

#if (FEATURE_3D == ENABLE)
   gm_3D_CodePatch_In_Start_Of_ModeDetermine();
#endif

   switch (inpMeasType)
   {
#ifdef EXTERNAL_DECODER
      case gmt_ExternalStatus:
         {
            gmt_InputMeasurementStruct ifm;
            gmt_IbdStruct ibd;

#if 1//(DEFAULT_DIP_DAUGHTER_BOARD_OUT != VPC3230_601)

				if(B_Channel == CH_A)
				{
            				gm_SetRegBitsWord(MIFM_CTRL, 0x8000);
					gm_SetRegBitsWord(IMP_IBD_CONTROL, IMP_MEASURE_DE_nDATA);
				}
				else
				{
            				gm_SetRegBitsWord(PIFM_CTRL, 0x8000);
            				gm_SetRegBitsWord(IPP_IBD_CONTROL, IMP_MEASURE_DE_nDATA);
				}

#endif

            gm_IbdConfig(B_Channel,gmd_IBD_THRESHOLD,0x0A);
				if(B_Channel == CH_A)
				{
            	gm_ClearRegBitsWord(IMP_IBD_CONTROL, IMP_IBD_EN);
					gm_SetRegBitsWord(IMP_IBD_CONTROL, IMP_IBD_EN);
				}
				else
				{
            	gm_ClearRegBitsWord(IPP_IBD_CONTROL, IPP_IBD_EN);
            	gm_SetRegBitsWord(IPP_IBD_CONTROL, IPP_IBD_EN);
				}
            gm_WaitForInputBlanking(B_Channel);

            gm_ReadIFMTiming((gmt_PHY_IFM)B_Channel, &ifm);
            inpHfrq = ifm.W_HFreq;
            inpVfrq = ifm.W_VFreq;
            inpVsyncPulse = ifm.W_VsyncPulse;
            inpHsyncPulse = ifm.W_HsyncPulse;
            if (ifm.W_DetectFlags & gmd_INPUT_NEG_HSYNC)
               inpModeFlags |= gmd_MODE_NEG_HSYNC;
            if (ifm.W_DetectFlags & gmd_INPUT_NEG_VSYNC)
               inpModeFlags |= gmd_MODE_NEG_VSYNC;

            gm_InputWaitVsync(B_Channel, 2);// Wait two fields before reading
            gm_IbdRead(B_Channel, &ibd);
            inpHstart = ibd.HStart;
            inpVstart= ibd.VStart;
            inpHactive = ibd.HActive;
            inpVactive =ibd.VActive;
            inpHtotal= ibd.HTotal;
            inpVtotal = ibd.VTotal;
            inpModeFlags |= gmd_MODE_SDVIDEO;
            inpModeFlags &= ~(WORD)gmd_MODE_UNKNOWN;
         }
         break;
#endif
#if (FEATURE_AIP == ENABLE)
      case gmt_IFMmeasurement:
         {
            gmt_InputMeasurementStruct ifm;

            gm_ReadIFMTiming((gmt_PHY_IFM)B_Channel, &ifm);
            inpHfrq = ifm.W_HFreq;
            inpVfrq = ifm.W_VFreq;
            inpVtotal = ifm.W_VTotal;
            inpVsyncPulse = ifm.W_VsyncPulse;
            inpHsyncPulse = ifm.W_HsyncPulse;
            if (ifm.W_DetectFlags & gmd_INPUT_NEG_HSYNC)
               inpModeFlags |= gmd_MODE_NEG_HSYNC;
            if (ifm.W_DetectFlags & gmd_INPUT_NEG_VSYNC)
               inpModeFlags |= gmd_MODE_NEG_VSYNC;
         }
         break;

      case gmt_IBDmeasurement:
         {
            gmt_InputMeasurementStruct ifm;
            gmt_IbdStruct ibd;

            gm_ReadIFMTiming((gmt_PHY_IFM)B_Channel, &ifm);
            inpHfrq = ifm.W_HFreq;
            inpVfrq = ifm.W_VFreq;
            inpVsyncPulse = ifm.W_VsyncPulse;
            inpHsyncPulse = ifm.W_HsyncPulse;
            if (ifm.W_DetectFlags & gmd_INPUT_NEG_HSYNC)
               inpModeFlags |= gmd_MODE_NEG_HSYNC;
            if (ifm.W_DetectFlags & gmd_INPUT_NEG_VSYNC)
               inpModeFlags |= gmd_MODE_NEG_VSYNC;

            gm_InputWaitVsync(B_Channel, 2);// Wait two fields before reading
            gm_IbdRead(B_Channel, &ibd);
            inpHstart = ibd.HStart;
            inpVstart= ibd.VStart;
            inpHactive = ibd.HActive;
            inpVactive =ibd.VActive;
            inpHtotal= ibd.HTotal;
            inpVtotal = ibd.VTotal;

            msg_ii("Input Active: %d x %d",inpHactive, inpVactive);
            msg_ii("Input Total : %d x %d",inpHtotal, inpVtotal);
            msg_ii("Input Start : %d x %d",inpHstart, inpVstart);
            msg_ii("Input Frq   : %d x %d",inpHfrq, inpVfrq);
            //msg_ii("Input SyncPs: %d x %d",inpHsyncPulse,inpVsyncPulse);
         }
         break;
#endif

#if (FEATURE_HDMI == ENABLE)
      case gmt_HdmiDvimeasurement:
         {
            if(gm_HdmiIsTmingMatched(B_Channel,&InTiming)==FALSE)
            {
                  gm_Print("######gm_HdmiIsTmingMatched Failed !",0);
                  return FALSE;
            }
         }
         break;
#endif
#if (FEATURE_DISPLAY_PORT == ENABLE)
      case gmt_DPmeasurement:
         {			
            if(gm_DpRxIsTmingMatched(B_Channel, &InTiming)==FALSE)
            {
                  gm_Print("######gm_HdmiIsTmingMatched Failed !",0);
                  return FALSE;
            }
         }
         break;
#endif //#if (FEATURE_DISPLAY_PORT == ENABLE)
   }

#if (FEATURE_DISPLAY_PORT==ENABLE)||(FEATURE_HDMI==ENABLE)   
   if ((inpMeasType == gmt_HdmiDvimeasurement) || (inpMeasType == gmt_DPmeasurement))
   {
      gmt_IbdStruct ibd;

#if (FEATURE_DISPLAY_PORT == ENABLE)
         if (inpMeasType == gmt_DPmeasurement)		
         {
            gmt_DPRX_DID b_DID = gm_GetDpDigitalID(gmvb_InputConnector[B_Channel]);
            WORD W_IP_Src = gm_ReadRegWord(ip_source[B_Channel]) & ~(IMP_VS_INV | IMP_HS_INV );

            if ((gm_ReadRegWordDP12Rx(b_DID,DP12RX0_MS_HS_WIDTH) & DPRX_MS_HS_POLARITY))
               W_IP_Src |= IMP_HS_INV;

            if ((gm_ReadRegWordDP12Rx(b_DID,DP12RX0_MS_VS_WIDTH) & DPRX_MS_VS_POLARITY))
               W_IP_Src |= IMP_VS_INV;

            gm_WriteRegWord(ip_source[B_Channel], W_IP_Src);
            gm_InputWaitVsync(B_Channel, 3);
         }
#endif

      #ifdef ST_4K2K_93xx_BOARD
      // DP use MS data
      if(inpMeasType == gmt_HdmiDvimeasurement)
      #endif
      	gm_IbdRead(B_Channel, &ibd);

      //gm_Print("@@@RetryConunt=%d",RetryConunt);
      //inpVtotal=ibd.VTotal;
      //inpHtotal=ibd.HTotal;

      #ifdef ST_4K2K_93xx_BOARD
      #if (FEATURE_DISPLAY_PORT == ENABLE)
      if (inpMeasType == gmt_DPmeasurement) // IBD sometimes not stable use DP MS data
      {
         inpVtotal = InTiming.VTotal;
         inpHtotal = InTiming.HTotal;      
         inpHstart = InTiming.HActiveStart - 4;
         inpVstart = InTiming.VActiveStart;
         inpHactive = InTiming.HActive;         
         inpVactive = InTiming.VActive;                           
      }      
      else
      #endif
      #endif
      {
         inpVtotal=ibd.VTotal;
         inpHtotal=ibd.HTotal;               
         inpHactive=ibd.HActive;
         inpVactive=ibd.VActive;
         inpHstart=ibd.HStart;
         inpVstart=ibd.VStart;
      }
      
      InTiming.PixelClockHz=UpdatePixelClkFromAVDDS(B_Channel); //Get more acurate pclk from AVDDS
      msg("Pclk=%Ld",InTiming.PixelClockHz);

      if (InTiming.Interlaced == TRUE)
         inpModeFlags |= gmd_MODE_INTERLACED;
   #if (FEATURE_HDMI==ENABLE) && (AustralianModeInterlaceDetect_Enable != 0)
      else if(inpMeasType == gmt_HdmiDvimeasurement)
      {
         if((inpVactive==576) && (inpHactive==1280 || inpHactive==1920))
         {
            msg("## AUS-1152i detected!, Forced to interlaced mode",0);
            inpModeFlags |= gmd_MODE_INTERLACED;
         }
         else if (inpVactive==540 && inpHactive==1920)
         {
            msg("## AUS-1080i detected!, Forced to interlaced mode",0);
            inpModeFlags |= gmd_MODE_INTERLACED;
         }
      }
   #endif      
         
      if (InTiming.HS_Positive==0)
         inpModeFlags |= gmd_MODE_NEG_HSYNC;

      if (InTiming.VS_Positive==0)
         inpModeFlags |= gmd_MODE_NEG_VSYNC;


      if (InTiming.HTotal != 0)
         inpHfrq = (WORD)((DWORD)(InTiming.PixelClockHz) / InTiming.HTotal / 100); 
      else
         inpHfrq = 1;
         
      if (InTiming.VTotal != 0)
      {
         DWORD tmp;
         tmp = (InTiming.PixelClockHz*10) / InTiming.HTotal;
         tmp *=10;
         if(InTiming.Interlaced == TRUE)
            inpVfrq = (WORD)((tmp<<1) / (((InTiming.VTotal & 0xFFFE)<<1)+1));
         else
            inpVfrq = (WORD)(tmp / InTiming.VTotal);
      }
      else
         inpVfrq = 1;    

      msg("inpVfrq1=%d",inpVfrq);
      if((inpVfrq%10)>=5)
         inpVfrq=(inpVfrq/10)+1;
      else
         inpVfrq=(inpVfrq/10);

      msg("inpVfrq2=%d",inpVfrq);   

      inpModeFlags &= ~(WORD)gmd_MODE_UNKNOWN;
      goto label_mode_found;
   }
#endif


#if (FEATURE_AIP == ENABLE)

   //Check if the input is interlaced using the IFM block
   if ((inpMeasType != gmt_HdmiDvimeasurement) && (inpMeasType != gmt_DPmeasurement))
   {
      if ((gm_ReadRegWord(input_status[B_Channel]) & ip_intlc_err) == 0) //Interlaced input decode error
      {
         // TODO: Temp solution for interalced detection!
         //msg(" ModeDetem ... Interlaced - again!",0);
         gm_SetRegBitsWord(input_status[B_Channel], ip_intlc_err); // Clear status
         gm_InputWaitVsync(B_Channel, 2);
         if ((gm_ReadRegWord(input_status[B_Channel]) & ip_intlc_err) == 0)
         {
            inpModeFlags |= gmd_MODE_INTERLACED;
            msg(" ModeDetem ... Interlaced !!",0);
         }
      }
   }
   else
      gm_SetRegBitsWord(input_status[B_Channel], ip_intlc_err); // Clear status

   //===========Search mode==================================================
   msg_ii(" H-Freq 1 = %d , V-Freq = %d",inpHfrq, inpVfrq);
   msg_ii(" VTotal = %d , ModeFlag = 0x%x", inpVtotal, inpModeFlags);

   //===Non CVT check of frequency parameters================
   if ((inpHfrq < MIN_INPUT_H_FREQ) ||(inpHfrq > MAX_INPUT_H_FREQ) || (inpVfrq < MIN_INPUT_V_FREQ) || (inpVfrq > MAX_INPUT_V_FREQ))
      return FALSE;

   if (inpMeasType == gmt_ExternalStatus)
      goto label_mode_found;

   pModeTable =
      (B_InputType == SLAVE) ? // choose between local and remote table
      AppPortAttrib[B_Connector].RemoteModeTable :
      (B_InputType!= VGA) ?
      AppPortAttrib[B_Connector].LocalModeTable :
#if 1 // Scan video timing table first in SOG		
      (((inpVfrq < 625) || (inpVfrq > 950)) && (gm_GetSyncState(B_Channel) == SOG_STATE)) ?
      LocalVGAModeTable_SOG :
#endif
      (inpVfrq < 625) ?
      LocalVGAModeTable_60 :
      (inpVfrq < 780) ? LocalVGAModeTable_75 : LocalVGAModeTable_85;

   //===No mode table========================================
   if (pModeTable == NULL) // by convention null means no look up process needed
   {
      // set the proper mode flags (not taken from table)
      inpModeFlags &= ~(WORD)gmd_MODE_UNKNOWN;
      goto label_mode_found;
   }

   //===Searches the standard mode table for proper mode match based on HFr, Vfr, Vtotal values===========
   {
      StandardModeStruct ROM * ROM * pModeTableInx;
      StandardModeStruct ROM* pMode;

      for (pModeTableInx = pModeTable; ; pModeTableInx++)
      {
#define PICK_POLICY     PICK_FIRST
         BYTE B_Findings = 0;

         msg(" Searching ModeTable: %d",(pModeTableInx - pModeTable));

         if ((pMode = *pModeTableInx) == NULL)
            break;

         for (inpModeIndex = 0, B_Findings = 0; ; inpModeIndex++,pMode++)
         {    // this function is operating based on the convention that mode table is null element terminated
            if (pMode->W_StdHFreq == 0)
            {
               pMode = 0;
               break;
            }
            // Check H,V frequency, Vtotal is within the standard mode limits
            if ((abs(pMode->W_StdHFreq - inpHfrq) <= pMode->B_DeltaHFreq) && (abs(pMode->W_StdVTotal - inpVtotal) <= pMode->B_DeltaVTotal))
            {
               // if (++B_Findings != PICK_POLICY)
               //    continue;

		    if((UserPrefCurrentInputMain==ALL_BOARDS_CVBS1||UserPrefCurrentInputMain==ALL_BOARDS_SVIDEO1) && inpModeIndex==1 ) //leo add PAL 
		    	{

				gmvw_InputSyncFlags[B_Channel] =(gmd_SYNC_ODD_INV |gmd_SYNC_ODD_TYPE_EXT);
			}


			   
               B_Findings++;//SCR#3712 JoeR
               msg(" Found match timing: %d",inpModeIndex);

               // if the current mode is identified as a overlap mode, then call the overlap validator
               if ((pMode->W_OverlapType & gmd_MODE_OVERLAPPED_GR640_GR720)
                     || (pMode->W_OverlapType & gmd_MODE_OVERLAPPED_480P)
                     || (pMode->W_OverlapType & gmd_MODE_OVERLAPPED_1080_576)
                     || (pMode->W_OverlapType & gmd_MODE_OVERLAPPED_240p_480i))
               {    // if the validator says that this is not the mode, then continue the search

                  if (!OverlapModeValidator(B_Channel, pMode, inpModeFlags & gmd_MODE_INTERLACED, inpMeasType, inpHactive, inpModeFlags))
                  {
                     B_Findings = 0;
                     continue;
                  }
               }
               else
               {
                  if (B_Findings != PICK_POLICY) //SCR#3712 JoeR
                  {
                     B_Findings = 0;
                     continue;
                  }
               }

               if ((pMode->W_OverlapType & gmd_MODE_OVERLAPPED_SYNCTYPE) && (gm_GetSyncState(B_Channel) == DSS_STATE))
               {
                  gm_Print(" Overlap - Input SyncType = 0x%x",inpModeFlags & (gmd_MODE_NEG_HSYNC | gmd_MODE_NEG_VSYNC));
                  gm_Print("                Mode Flag = 0x%X",(pMode->W_ModeType) & (gmd_MODE_NEG_HSYNC | gmd_MODE_NEG_VSYNC));

                  if ((pMode->W_ModeType & (gmd_MODE_NEG_HSYNC | gmd_MODE_NEG_VSYNC)) != (inpModeFlags & (gmd_MODE_NEG_HSYNC | gmd_MODE_NEG_VSYNC)))
                  {
                     B_Findings = 0;
                     continue;
                  }
               }
               
               if (pMode->W_OverlapType & gmd_MODE_OVERLAPPED_VWIDTH)
               {
                  WORD inpVSwidth = ((inpVtotal >> 1) < inpVsyncPulse) ? //Check sync polarity to calculate V width
                                    (inpVtotal - inpVsyncPulse) : //Negative
                                    inpVsyncPulse; //Positive

                  gm_Print("Overlap - VSyncWidth = %d",inpVSwidth);

                  if (inpVSwidth != pMode->B_StdHpulse)
                  {
                     B_Findings = 0;
                     continue;
                  }
               }
               if ((pMode->W_OverlapType & gmd_MODE_OVERLAPPED_HWIDTH) && (gm_GetSyncState(B_Channel) == DSS_STATE))
               {
                  WORD wHSyncWidthPixel;

                  wHSyncWidthPixel = (WORD)(((DWORD)pMode->W_StdHTotal * inpHsyncPulse) / gm_ReadRegWord(MIFM_HS_PERIOD));


                  gm_Print(" Overlap - HSyncWidth = %d",wHSyncWidthPixel);
                  if (abs(wHSyncWidthPixel - pMode->B_StdHpulse) > 8)
                  {
                     B_Findings = 0;
                     continue;
                  }
               }

               msg(" Std Mode Table: %d",(pModeTableInx - pModeTable));
               msg("          Index: %d",inpModeIndex);

               if (*pModeTableInx == StandardModeTable_OOR)
               {
                  gm_Print("Out-Of-Range Timing !",0);
                  stInput[B_Channel].Hactive = pMode->W_StdWidth;
                  stInput[B_Channel].Vactive = pMode->W_StdHeight;
                  stInput[B_Channel].Hfrq = inpHfrq;
                  stInput[B_Channel].Vfrq = pMode->W_StdVFreq;
                  inpModeIndex = 0xFF;
                  return FALSE;
               }

               if (IsInputVGA(B_Channel))    // Only save data for VGA input
               {
                  if (*pModeTableInx == StandardModeTable_70)
                     inpModeIndex += NumOfModeTable70;
                  else if (*pModeTableInx == StandardModeTable_85)
                     inpModeIndex += NumOfModeTable60 + NumOfModeTable70;
                  else if (*pModeTableInx == CompHD_ModeTable)
                     inpModeIndex += NumOfModeTable60 + NumOfModeTable70 + NumOfModeTable85;
               }
               else
                  inpModeIndex = 0xFF;

               msg("   NVRAM Mode Index: %d",inpModeIndex);
               stInput[B_Channel].ModeIndex = inpModeIndex;

               goto label_mode_found_a;
            }
         }
      }

      gm_Print("   Timing Not Found in Std ModeTables!",0);

      //===CVT modes========================================
#ifdef CVT_MODES_SUPPORT
      //only for IFM measurement type, others measurement's mode do not contain sync pulse
      // data, which is needed for CVT calculation
      if ((inpMeasType == gmt_IFMmeasurement) || (inpMeasType == gmt_IBDmeasurement))
      {
         gmt_InputMeasurementStruct ifm;
         ifm.W_HFreq = inpHfrq;
         ifm.W_VFreq = inpVfrq;
         ifm.W_VTotal = inpVtotal;
         ifm.W_VsyncPulse = inpVsyncPulse;
         ifm.W_HsyncPulse = inpHsyncPulse;
         if (inpModeFlags & gmd_MODE_NEG_HSYNC)
            ifm.W_DetectFlags = gmd_INPUT_NEG_HSYNC;
         if (inpModeFlags & gmd_MODE_NEG_VSYNC)
            ifm.W_DetectFlags = gmd_INPUT_NEG_VSYNC;
         if (cvt_IsCVT(&ifm, B_Channel))
         {
            gmt_IbdStruct cvtParams;
            //Calculates the CVT mode parameters based upon the measured input timing.
            //Given the Vtotal and Vpulse, calculates the HActive, VActive, HTotal and H&V Blanking time.
            cvt_CalcTiming(&ifm, &cvtParams);
            // Report 'OUT_OF_RANGE' error if HSync, VSync and Pixel Clock freq are beyond the maximum supported.
            if (((inpHfrq <= MAX_HFREQ) && (inpVfrq <= MAX_VFREQ)) || IsCVTPixelClockRange(&ifm, &cvtParams, B_Channel))
            {
               inpModeFlags &= ~(WORD)gmd_MODE_UNKNOWN;
               inpModeFlags |= gmd_MODE_CVT;
               inpHstart = cvtParams.HStart;
               inpVstart = cvtParams.VStart;
               inpHactive = cvtParams.HActive;
               inpVactive = cvtParams.VActive;
               inpHtotal = cvtParams.HTotal;
               inpVtotal = cvtParams.VTotal;
               gm_Printf("CVT: Hfreq = %d; Vfreq = %d; HActive = %d; VActive = %d",
                         inpHfrq, inpVfrq, inpHactive, inpVactive);
               goto label_mode_found;
            }
         }
      }
#endif //CVT_MODES_SUPPORT

#ifndef ESTIMATED_MODES_SUPPORT
      return FALSE;
#endif

      // === Searches the standard mode table for ESTIMATED mode match based on V-TOTAL ===
      // If no perfect match is found in the SearchModeTable function this function tries
      // to get a mode that has the Vsync value as near to the determined Vsync value.
      {
         BYTE EstModeInx;
         WORD dVTotalMin = 0xFFFF;
         StandardModeStruct ROM* pModeEst = *pModeTable;

         if (pModeEst == NULL_PTR)
         {
            msg("Estimated mode could not be determined",0);
            return FALSE;
         }
         
         for (EstModeInx = 0; EstModeInx < NumOfGraphicsPresetMode; EstModeInx++, pModeEst++)
         {
            if (gm_GetSyncState(B_Channel) != SOG_STATE)
            {
               if ((pModeEst->W_ModeType & gmd_MODE_SIGNAL_MASK) > gmd_MODE_GRAPHICS)
                  continue; // do not allow estimated modes for video modes
               if ((pModeEst->W_ModeType & gmd_MODE_INTERLACED) == gmd_MODE_INTERLACED)
                  continue; // do not allow estimated modes for interlaced modes
            }
            
            if (pModeEst->W_StdHFreq == 0)
               break;
            
            if (inpVtotal > pModeEst->W_StdHeight) //If VTotal is grater than Standard V Active ...
            {
               WORD dVTotal = abs(pModeEst->W_StdVTotal - inpVtotal); // compute the current VTotal offset

               if (dVTotal < dVTotalMin)  // ...and the current offset is smaller than the previous smallest
               {  // This is the right parent for this estimated mode
                  dVTotalMin = dVTotal;
                  pMode = pModeEst;
                  inpModeFlags |= gmd_MODE_ESTIMATED;
                  inpModeIndex = EstModeInx;
                  gm_Print(" Estimated Mode Found: %d",EstModeInx);
                  goto label_mode_found_a;
               }
            }
         }

         if (pModeEst->W_StdHFreq == 0)  //Indicate end of table
         {
            msg(" Estimated mode could not be determined", 0);
            return FALSE;
         }

         inpModeFlags |= gmd_MODE_ESTIMATED;
      }

      //-------------------------------------------
label_mode_found_a:

      inpModeFlags &= ~(WORD)gmd_MODE_UNKNOWN;
      // H / V satrt may not correct from TX, or RX must minus some offset. Read IBD for correct position first.
      // IBD threshold may too low ... sometimes lost or get more 1~2 line. Check later...
      {
         inpModeFlags |= pMode->W_ModeType;
         inpHstart = pMode->W_StdHStart;
         inpVstart = pMode->W_StdVStart;
         inpHactive = pMode->W_StdWidth;
         inpVactive = pMode->W_StdHeight;
         inpHtotal = pMode->W_StdHTotal;
      }

      if ((B_InputType == VGA)|| (B_InputType == COMPONENT))
      {
         inpModeFlags &= ~(gmd_MODE_SOG | gmd_MODE_CSYNC);

         if (gm_GetSyncState(B_Channel) == SOG_STATE)
         {
            inpModeFlags |= gmd_MODE_SOG;

            if ((inpModeFlags & gmd_MODE_SIGNAL_MASK) == gmd_MODE_GRAPHICS)
               gm_IbdConfig(B_Channel,gmd_IBD_RGB_SELECT, gmd_ALL_COMPONENT);
            else
               gm_IbdConfig(B_Channel,gmd_IBD_RGB_SELECT, gmd_GREEN_COMPONENT);
         }
         else if (gm_GetSyncState(B_Channel) == DCS_STATE)
            inpModeFlags |= gmd_MODE_CSYNC;
      }
   }

#endif // FEATURE_AIP == ENABLE

label_mode_found:

   stInput[B_Channel].Hfrq = inpHfrq;
   stInput[B_Channel].Vfrq = inpVfrq;
   stInput[B_Channel].Vtotal = Real_VTotal =inpVtotal;
   stInput[B_Channel].ModeIndex = inpModeIndex;
   stInput[B_Channel].Htotal = Real_HTotal = inpHtotal;
   stInput[B_Channel].ModeFlags = inpModeFlags;
   #ifdef ST_4K2K_93xx_BOARD
   if((UserPrefCurrentInputMain == ALL_BOARDS_DP2) || (GetChipID_4K2K() == CHIPID_4K2K_R)) // HDMI or R
   {
      SETBIT(stInput[gmvb_MainChannel].ModeFlags, gmd_MODE_CAPTURE_CROP);
   }
   #elif defined(RD2_4K2K_DEMO)
   if(GetChipID_4K2K() == CHIPID_4K2K_R)
   {
      //gm_Print("**Crop",0);
      SETBIT(stInput[gmvb_MainChannel].ModeFlags, gmd_MODE_CAPTURE_CROP);
   }
   #endif    
   stInput[B_Channel].Hstart = inpHstart;
   stInput[B_Channel].Vstart = inpVstart;
   stInput[B_Channel].Hactive = Real_Input_H = inpHactive;
   stInput[B_Channel].Vactive = Real_Input_V = inpVactive;

   SaveOriginalValue(B_Channel);

   msg_ii("Input Active: %d x %d",inpHactive, inpVactive);
   msg_ii("Input Total : %d x %d",inpHtotal, inpVtotal);
   msg_ii("Input Start : %d x %d",inpHstart, inpVstart);
   msg_ii("Input Frq   : %d x %d",inpHfrq, inpVfrq);


   if (Input_Color_Domain == Input_Auto)
   {
      //if(((GetChannelConnType(B_Channel) == HDMI) && (gm_IsHdmiTiming(gB_CurrentInputConnector[B_Channel])))
      //   || (GetChannelConnType(B_Channel) == CT_DISPLAY_PORT))
      //{
      //  gm_Print("Info Frame ...", 0);
      //}
      //else
      {
         switch (GetChannelConnType(B_Channel))
         {
#if (FEATURE_DISPLAY_PORT == ENABLE)         
            case CT_DISPLAY_PORT:
                  {
                        BYTE B_DPRXColorimetry = DpRxGetColorimetry(gB_CurrentInputConnector[B_Channel]);
                  		msg("ModeDetermine DP B_DPRXColorimetry = .......... %d", B_DPRXColorimetry);

                     if ((B_DPRXColorimetry & (BIT0 | BIT1)) == 0)
                     {
                        Input_RGB_Range = (B_DPRXColorimetry & BIT2) + 1;
                        msg("DP Input_RGB_Range ... %d", Input_RGB_Range);
                     }
                     else
                     {
                        Input_YUV_Range = B_DPRXColorimetry & BIT2;
                        if(Input_YUV_Range == 0)   // Full
                        {
                           stInput[B_Channel].ColorDomain = Input_YUV_Full;
                           Input_YUV_Range = 1; // Full Range
                           msg("Input YUV Full range ... %d", 0);
                           
                        }
                        else
                        {
                           stInput[B_Channel].ColorDomain = Input_YUV_Reduce;
                           Input_YUV_Range = 0; // Limited Range
                           msg("Input YUV Reduce range ... %d", 0);
                        }
                     }
                  }
               break;
#endif

#if (FEATURE_HDMI == ENABLE)
            case HDMI: // without info frame
            case DVI:
               if(gm_IsHdmiTiming(gB_CurrentInputConnector[B_Channel]))
                  {
                     msg("Info Frame ...", 0);

                     if (gm_HdmiGetAviColorSpace(gB_CurrentInputConnector[B_Channel]) == STHDMIRX_COLOR_FORMAT_RGB)
                     {
                        Input_RGB_Range = gm_HdmiGetRGBQuanRange(gB_CurrentInputConnector[B_Channel]);
                        msg("Input_RGB_Range ... %d", Input_RGB_Range);
                     }
                     else
                     {
                        Input_YUV_Range = gm_HdmiGetYCCQuanRange(gB_CurrentInputConnector[B_Channel]);

                        if(Input_YUV_Range == STHDMIRX_YC_QUANT_RANGE_FULL)
                        {
                           stInput[B_Channel].ColorDomain = Input_YUV_Full;
                           msg("Input YUV Full range ... %d", 0);
                        }
                        else
                        {
                           stInput[B_Channel].ColorDomain = Input_YUV_Reduce;
                           msg("Input YUV Reduce range ... %d", 0);
                        }
                     }
                  }
               else
                  {
                     stInput[B_Channel].ColorDomain = Input_RGB;
                     if ((stInput[B_Channel].ModeFlags & gmd_MODE_INTERLACED) == gmd_MODE_INTERLACED)
                     {            
                        // Output
                        if (inpVtotal <= 625 /2 + 1)
                           stInput[B_Channel].ModeFlags |= gmd_MODE_SDVIDEO;
                        else //if (((0xfffe & inpVactive) == 540) || (abs(inpVactive - 517) <= 1))
                           stInput[B_Channel].ModeFlags |= gmd_MODE_HDVIDEO;
                     }    
                  }
               break;
#endif
            case VGA:
               if (((inpModeFlags & gmd_MODE_SIGNAL_MASK) != gmd_MODE_GRAPHICS)
                     && (gm_GetSyncState(B_Channel) == SOG_STATE))
                  {
                     Input_YUV_Range = Input_YUV_Reduce;
                     stInput[B_Channel].ColorDomain = Input_YUV_Reduce;
                     //gm_Print(" 1 YUV ....... %d", Input_YUV_Reduce);
                  }
               else
                  {
                     Input_RGB_Range = STHDMIRX_RGB_QUANT_RANGE_DEFAULT;
                     stInput[B_Channel].ColorDomain = Input_RGB;
                     //gm_Print(" 1 RGB ....... %d", Input_RGB);
                  }   
               break;


            case COMPONENT:
               if ((inpModeFlags & gmd_MODE_SIGNAL_MASK) == gmd_MODE_GRAPHICS)
               {
                  Input_RGB_Range = STHDMIRX_RGB_QUANT_RANGE_DEFAULT;
                  stInput[B_Channel].ColorDomain = Input_RGB;
               }
               else
               {
                  Input_YUV_Range = Input_YUV_Reduce;
                  stInput[B_Channel].ColorDomain = Input_YUV_Reduce;
               }
               break;

            case DIPX:
               #if 1//ndef ANX9021_DEV_ID
               Input_YUV_Range = Input_YUV_Reduce;
               stInput[B_Channel].ColorDomain = Input_YUV_Reduce;
#endif
               break;

            default:
                 Input_RGB_Range = STHDMIRX_RGB_QUANT_RANGE_DEFAULT;
                  stInput[B_Channel].ColorDomain = Input_RGB;
               break;
         }
      }
   }




   // Force input color domain by appstest 36
      if (Input_Color_Domain == Input_YUV_Reduce)
      {
         stInput[B_Channel].ColorDomain = Input_YUV_Reduce;
         Input_YUV_Range = 0; // Limited Range 
         msg("Input color damain set to Input_YUV_Reduce ... ", 0);
      }
      else if (Input_Color_Domain == Input_YUV_Full)
      {
         stInput[B_Channel].ColorDomain = Input_YUV_Full;
         Input_YUV_Range = 1; // Full Range
         msg("Input color damain set to Input_YUV_Full ... ", 0);
      }
      else if (Input_Color_Domain == Input_RGB)
      {
         // ToDo: Add other range for RGB, if need
         Input_RGB_Range = STHDMIRX_RGB_QUANT_RANGE_DEFAULT;
         stInput[B_Channel].ColorDomain = Input_RGB;
         msg("2 Input color damain set to RGB ... ", 0);
      }




   if (Output_Color_type == Output_Auto)
   {
      msg("Output color damain set to Auto ... ", 0);
   }
   else if (Output_Color_type == Output_RGB)
   {
      stInput[B_Channel].ModeFlags &= ~gmd_MODE_SIGNAL_MASK;
      stInput[B_Channel].ModeFlags |= gmd_MODE_GRAPHICS;
      msg(" 3 Output color process set to Graphics ... ", 0);
   }
   else if (Output_Color_type > Output_RGB)
   {
      if (inpVtotal <= 625 /2 + 1)
         stInput[B_Channel].ModeFlags |= gmd_MODE_SDVIDEO;
      else //if (((0xfffe & inpVactive) == 540) || (abs(inpVactive - 517) <= 1))
         stInput[B_Channel].ModeFlags |= gmd_MODE_HDVIDEO;

      msg(" 3 Output color process set to YUV ... ", 0);

      if (Output_Color_type == Output_YUV_Full)
      {
         // ToDo
      }
      else if (Output_Color_type == Output_YUV_Reduce)
      {
         // ToDo
      }
   }

#if 0
   // ========================================================================
   // 20110718, test for input domain
   if ((((GetChannelConnType(B_Channel) == HDMI) && (gm_IsHdmiTiming(gB_CurrentInputConnector[B_Channel])))
         || (GetChannelConnType(B_Channel) == CT_DISPLAY_PORT)) && (Input_Color_Domain == Input_Auto))
   {
      gm_Print("HDMI and DP port get input color domain by Info frame, Or set to Auto mode", 0);
   }
   else
   {
      // Default rules ... Pls. modify rules by each owner for detail timings or ports
      switch (GetChannelConnType(B_Channel))
      {
         case HDMI: // without info frame
         case DVI:

            stInput[B_Channel].ColorDomain = Input_RGB;

            if ((stInput[B_Channel].ModeFlags & gmd_MODE_INTERLACED) == gmd_MODE_INTERLACED)
            {
               // Output
               if (inpVtotal <= 625 /2 + 1)
                  stInput[B_Channel].ModeFlags |= gmd_MODE_SDVIDEO;
               else if (((0xfffe & inpVactive) == 540) || (abs(inpVactive - 517) <= 1))
                  stInput[B_Channel].ModeFlags |= gmd_MODE_HDVIDEO;
            }
            break;

         case VGA:
            if (((inpModeFlags & gmd_MODE_SIGNAL_MASK) != gmd_MODE_GRAPHICS)
                  && (gm_GetSyncState(B_Channel) == SOG_STATE))
               stInput[B_Channel].ColorDomain = Input_YUV_Reduce;
            else
               stInput[B_Channel].ColorDomain = Input_RGB;
            break;

            //case DVI:
            //   if ((stInput[B_Channel].ModeFlags & gmd_MODE_INTERLACED) == gmd_MODE_INTERLACED)
            //      stInput[B_Channel].ColorDomain = Input_YUV;
            //   else
            //      stInput[B_Channel].ColorDomain = Input_RGB;
            //   break;

         case COMPONENT:
            if ((inpModeFlags & gmd_MODE_SIGNAL_MASK) == gmd_MODE_GRAPHICS)
               stInput[B_Channel].ColorDomain = Input_RGB;
            else
               stInput[B_Channel].ColorDomain = Input_YUV_Reduce;
            break;

         case DIPX:
#ifndef ANX9021_DEV_ID
            stInput[B_Channel].ColorDomain = Input_YUV_Reduce;
#endif
            break;

         default:
            stInput[B_Channel].ColorDomain = Input_RGB;
            break;
      }

      gm_Print("Input %d : ColorDomain = %d",GetChannelConnType(B_Channel),stInput[B_Channel].ColorDomain);

      // Force input color domain by appstest 36
      if (Input_Color_Domain == 2)
      {
         stInput[B_Channel].ColorDomain = Input_YUV_Reduce;
         gm_Print("Input color damain set to YUV ... ", 0);
      }
      else if (Input_Color_Domain == 1)
      {
         stInput[B_Channel].ColorDomain = Input_RGB;
         gm_Print("Input color damain set to RGB ... ", 0);
      }

      if (Output_Color_type == 1)
      {
         stInput[B_Channel].ModeFlags &= ~gmd_MODE_SIGNAL_MASK;
         stInput[B_Channel].ModeFlags |= gmd_MODE_GRAPHICS;
         gm_Print("Output color process set to Graphics ... ", 0);
      }
      else if (Output_Color_type > 1)
      {
         if (inpVtotal <= 625 /2 + 1)
            stInput[B_Channel].ModeFlags |= gmd_MODE_SDVIDEO;
         else if (((0xfffe & inpVactive) == 540) || (abs(inpVactive - 517) <= 1))
            stInput[B_Channel].ModeFlags |= gmd_MODE_HDVIDEO;



         //stInput[B_Channel].ModeFlags &= ~gmd_MODE_SIGNAL_MASK;
         //stInput[B_Channel].ModeFlags |= (Output_Color_type - 1);
         gm_Print("Output color process set to Video ... %d", stInput[B_Channel].ModeFlags);
      }
   }
#endif
   // =========================================================================
  /*ChengLin 20130131: Enable VEGA I2C Control Via Athena FE*/
#ifdef ST_4K2K_93xx_BOARD
#if (FEATURE_HDMI == ENABLE && ENABLE_VEGA_CONTROL_VIA_ATHENA_FE)
	  if(GetChannelConnType(B_Channel) == HDMI)
	  {
	  	  /*ChengLin 20130205: Move to here for avoid VEGA keep black screen*/
		  SOCIPC_VEGA_LVDS_BUS_WIDTH_Via_HDMI_Input();		  
	  }
#endif
#endif

#if (FEATURE_3D == ENABLE)
   gm_3D_CodePatch_In_End_Of_ModeDetermine(B_Channel);
#endif

 //  gm_Print("**D(%d)",(WORD)(gm_GetSystemTime()-preT));
   return TRUE;
}

//******************************************************************************
// USAGE          : Return if the mode is 1080I.
// INPUT          : B_Channel - CH_A or CH_B
// OUTPUT         : TRUE/FALSE
// GLOBALS        :    gmvw_InputHActiveOrg[]    (R)
//                    gmvw_InputVActiveOrg[]    (R)
// USED_REGS      : None
// PRE_CONDITION  : SaveOriginalValue() should have been executed.
// POST_CONDITION :
//******************************************************************************
BOOL IsMode1080i(gmt_PHY_CH B_Channel)
{
   return ((gmvw_InputHActiveOrg[B_Channel] == 1920)
           && (gmvw_InputVActiveOrg[B_Channel] == 540)
           && gm_IsInterlaced(B_Channel));
}

//******************************************************************************
// USAGE          : Return if the mode is NTSC.
// INPUT          : B_Channel - CH_A or CH_B
// OUTPUT         : TRUE/FALSE
// GLOBALS        :    gmvw_InputHActiveOrg[]    (R)
//                gmvw_InputVActiveOrg[]    (R)
//                gmvb_ChannelSource[]        (R)
// PRE_CONDITION  : SaveOriginalValue() should have been executed.
//******************************************************************************
BOOL IsModeNTSC(gmt_PHY_CH B_Channel)
{
   return ((gm_IsInputDecoder(B_Channel))
           && (gmvw_InputHActiveOrg[B_Channel] == 720)
           && (gmvw_InputVActiveOrg[B_Channel] == 240)
           && gm_IsInterlaced(B_Channel));
}

//******************************************************************************
//
// FUNCTION       : BOOL IsInputPal(void)
//
// USAGE          : Checks if the input mode is PAL
//
// INPUT          : B_Channel - CH_A or CH_B
//
// OUTPUT         : TRUE or FALSE
//
// GLOBALS        :
//
// USED_REGS      :
//
// PRE_CONDITION  : None
//
// POST_CONDITION : None
//
//******************************************************************************
BOOL IsInputPal(void)
{
   gmt_PHY_CH B_Channel = IsSideBySidePipMode(UserPrefPipMode) ? B_FocusBorder : gmvb_MainChannel;
   gmt_DECODER_ID B_DecId = gmd_DEC_CORTEZ_ID;

   return (gm_IsVideo(B_Channel) && IsDecoderTimingPAL(B_DecId));
}

//******************************************************************************
//
// FUNCTION       : BOOL IsDecoderTimingPAL( gmt_DECODER_ID B_DecoderID )
//
// USAGE          : Checks if decoder input mode is PAL
//
// INPUT          : B_DecoderID - 3D or 2D decoder
//
// OUTPUT         : TRUE or FALSE
//
// GLOBALS        :
//
// USED_REGS      :
//
// PRE_CONDITION  : None
//
// POST_CONDITION : None
//
//******************************************************************************
BOOL IsDecoderTimingPAL( gmt_DECODER_ID B_DecoderID )
{
#if 0
   BYTE B_DecoderMode = gm_DecoderGetVideoMode(B_DecoderID);
   return (((B_DecoderMode == gmd_DEC_MODE_PAL) ||
            (B_DecoderMode == gmd_DEC_MODE_PAL_N) ||
            (B_DecoderMode == gmd_DEC_MODE_PAL_M) ||//if PAL_M mode is to be treated as PAL mode
            (B_DecoderMode == gmd_DEC_MODE_PAL_60) ||
            (B_DecoderMode == gmd_DEC_MODE_SECAM)) ? TRUE : FALSE);
#else
   B_DecoderID = B_DecoderID;
#endif
   return FALSE;
}

//******************************************************************************
//
// FUNCTION       : BOOL IsGraphics(gmt_PHY_CH B_Channel)
//
// USAGE          : Returns whether the given channel is in graphics mode or not. The
//                  main reason for this function is that when Hudson is used, a
//                  different variable (gmvw_InputModeAttr_Hudson) should be checked.
//
// INPUT          : B_Channel - CH_A or CH_B
//                     Note : The value of CH_A or CH_B is stored in
//                     gmvb_MainChannel and gmvb_PipChannel
//
// OUTPUT         : TRUE or FALSE
//
// GLOBALS        : gmvb_PipChannel
//                  Hudson_InputModeAttr
//                  gmvw_InputModeAttr[]
//
// USED_REGS      :
//
// PRE_CONDITION  : None
//
// POST_CONDITION : None
//
//******************************************************************************
BOOL IsGraphics(gmt_PHY_CH B_Channel)
{
   return gm_IsGraphics(B_Channel);
}

/*********************************  END  **************************************/

