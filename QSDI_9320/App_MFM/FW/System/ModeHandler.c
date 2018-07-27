/*
	$Workfile:   ModeHandler.c  $
	$Revision: 1.97 $
	$Date: 2013/08/16 06:11:33 $
*/
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
// MODULE:      ModeHandler.c
//
// USAGE:       This module contains application level of Mode Handler which
//				resposible for checking input signal stable status,
//				determine input mode, send OSD message and set flag for mode
//				setup handler.
//
//******************************************************************************

//******************************************************************************
//  I N C L U D E    F I L E S
//******************************************************************************

#include <math.h>
#include "System\All.h"
#include "StdLibAdapt\gm_adapt_modedetection.h"

#if (FEATURE_ACR == ENABLE)
#include "system\ACR_Handler.h"
#endif
//******************************************************************************
//  E X T E R N A L    F U N C T I O N S
//******************************************************************************

//extern void VcrModeHandler	(void);
extern void WssStatusHandler(gmt_PHY_CH B_Channel);
extern void LetterBoxDetectHandler	(gmt_PHY_CH B_Channel);

//******************************************************************************
//  E X T E R N A L    V A R I A B L E S
//******************************************************************************
#ifdef ST_4K2K_93xx_BOARD
#if ENABLE_TWO_LOGICAL_PORTS_SLEEP_CHECK  
extern BOOL DP_TWO_LogicalPorts_StartPowerCheck_AthenaR;
#endif
#endif

//******************************************************************************
//  D E F I N E S
//******************************************************************************
#define FREE_RUN_DELAY 			0//750
#define INVALID_MODE_DELAY		3500
#define MODE_MAIN_SETUP_DELAY	50
#define MODE_SETUP_PIP_DELAY	(MODE_MAIN_SETUP_DELAY+50)
#define DETECT_REEVALUATE		1000





#define _DEBUG_MSG

#ifdef _DEBUG_MSG
#define msg(x,y) gm_Print(x,y)
#else
#define msg(x,y)
#endif

//******************************************************************************
//  S T A T I C    V A R I A B L E S
//******************************************************************************
#define INVALID_RECHECK_BY_HDMIDP_TIMING_NOT_MATCH 1		//To give more chance to recheck When HdmiDvi/DP report TimingNotMatch from Mode Determine
#if INVALID_RECHECK_BY_HDMIDP_TIMING_NOT_MATCH
#define MAX_INVALID_CHKCNT    3
static   BYTE InValidCheckCnt[CH_PHY_NUM]={0,0};
#endif


//*****************************************************************************
//  S T A T I C    F U N C T I O N    P R O T O T Y P E S
//*****************************************************************************

static void MainModeHandler		(void);
static void PipModeHandler		(void);
static BOOL ConditionToSwapChannelMapping(void);
static void SwapChannelMapping(void);
static void BandwidthLimitationPipDecoderMemoryTuning(void);

//*****************************************************************************
//  C O D E
//*****************************************************************************

//******************************************************************************
//
// FUNCTION       : void ModeHandlerInit(void)
//
// USAGE          : Mode Handler Initialization - called once in Application
//                  Init.
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        : gmvb_MainChannel				(RO)
//                  gmvb_PipChannel					(RO)
//                  B_ModeHandlerQSize				(WO)
//
// USED_REGS      : None
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void ModeHandlerInit(void)
{
   //VcrModeInit();
   OSDRequestMsg(gmvb_MainChannel, MTO_NoSignal, 2 * NO_SIGNAL_DELAY);
}

//******************************************************************************
//
// FUNCTION       : void ModeHandler(void)
//
// USAGE          : Mode Handler functions for Main and PIP channel.
//
// INPUT          : None
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
void ModeHandler(void)
{
   //
   // Process Main Channel
   //
   MainModeHandler();
   //
   // Process PIP Channel
   //
#ifndef NO_PIP_SUPPORT
   PipModeHandler();
#endif
   //
   // Monitor VCR mode
   //
   //VcrModeHandler();
#if (FEATURE_WSS == ENABLE)
   //
   // Monitor Main WSS status
   //
   WssStatusHandler(gmvb_MainChannel);

   //
   // Monitor PIP WSS status
   //
   WssStatusHandler(gmvb_PipChannel);
#endif

#ifdef USE_LETTERBOX_DETECTION
   //
   // Monitor Main LetterBox status
   //
   LetterBoxDetectHandler(gmvb_MainChannel);
#endif
}


#ifdef DYNAMIC_CHANNEL_MAPPING
//******************************************************************************
//
// FUNCTION		:	static BOOL ConditionToSwapChannelMapping(void)
//
// USAGE		:	Decides if need to swap the physical channels (CH_A and CH_B)
//					that MAIN and PIP are using.
//
// INPUT		:	None
//
// OUTPUT		:	BOOL - Should channels be swapped
//
// GLOBALS		:	gmvb_MainChannel		(R)
//
// USED_REGS	:
//
// PRE_CONDITION :
//
// POST_CONDITION :
//
//******************************************************************************
static BOOL ConditionToSwapChannelMapping(void)
{
   //no swapping when in multipip mode

   if (IsMultiPipOn())
      return FALSE;

   //channel mapping when progressive graphics is on CH_A
   if (((gmvb_MainChannel == CH_A) && gm_IsGraphics(gmvb_MainChannel) && !gm_IsInterlaced(gmvb_MainChannel))
         ||
         ((gmvb_MainChannel == CH_B) && (gm_IsVideo(gmvb_MainChannel) || gm_IsInterlaced(gmvb_MainChannel))))
   {
      return TRUE;
   }
   else
   {
      return FALSE;
   }
}


//******************************************************************************
//
// FUNCTION		:	static void SwapChannelMapping(void)
//
// USAGE		:	Swap the physical channels (CH_A and CH_B) that MAIN and PIP are using.
//
// INPUT		:	None
//
// OUTPUT		:	None
//
// GLOBALS		:	gmvb_MainChannel		(R)
//
// USED_REGS	:
//
// PRE_CONDITION :
//
// POST_CONDITION :
//
//******************************************************************************
static void SwapChannelMapping(void)
{
   SetDisplayOff(CH_A, BLENDER_OFF);
   SetDisplayOff(CH_B, BLENDER_OFF);

   if (gmvb_MainChannel == CH_A)
   {
      gm_SetMainChannel(CH_B);
      gm_SetPipChannel(CH_A);
   }
   else
   {
      gm_SetMainChannel(CH_A);
      gm_SetPipChannel(CH_B);
   }
   B_FocusBorder = gm_OtherChannel(B_FocusBorder);

   gmvb_MemoryBufferTypes[CH_A] = 0;
   gmvb_MemoryBufferTypes[CH_B] = 0;

   MemoryInit();
}
#endif // DYNAMIC_CHANNEL_MAPPING

//******************************************************************************
//
// FUNCTION       : void MainModeHandler(void)
//
// USAGE          : Processes the mode handler for MAIN channel
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        : gmvb_MainChannel		(R)
//                  gmvw_InputModeAttr		(R/W)
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
#ifdef VIDEO_CAPTURE
extern BYTE CDATA_Flag;
#endif
// 20090708 #1, Improved DVI HDCP without HPD.

//BOOL CMI_4K_Flag = 0;
void MainModeHandler(void)
{
   //
   // Check input signal status
   // 1) Is timing stable ?
   // 2) Is timing has changed from unstable to stable ?
   //
   if (gm_adapt_TimingIsStable(gmvb_MainChannel) == TRUE)
   {
#if 0 //(FEATURE_HDMI==ENABLE)
      // HDMI port
      if (IsInternalHdmiDviPort(gmvb_MainChannel))
      {
         if (HdmiStartModeDetection() == FALSE)
            return;
      }
#endif

      if (gm_adapt_InputTimingTransitionToStable(gmvb_MainChannel) == TRUE)
      {
      #if (FEATURE_AUDIO == ENABLE)
         Audio_VideoSyncDetected(gmvb_MainChannel);
      #endif

         stInput[gmvb_MainChannel].Flags= 0; // so far only xvYCC used and clear the flag
         #if (DEBUG_HDMI_PRINT_TIMING_DISPLAY_TIME!=0) || (DEBUG_DP_PRINT_TIMING_DISPLAY_TIME!=0)
             gm_Print(" -->MStb: T(%d)",(WORD)(gm_GetSystemTime()-GetDebugDisplayTime()));
         #endif    
         
         msg("MAIN-Stable",0);
/*
		// Move to intial state later ... must wit for panel initial finish
		 if(PanelID == CMO_V390DK1_LS1)
			{
				if(CMI_4K_Flag == 0) //LEO del
				{
					CMI_4K_Flag = 1;
					ST_4K2K_I2C();
				}
			}
*/
/*ChengLin 20130221: For Handle 4k2k platform Image Sync Handler*/
#ifdef ST_4K2K_93xx_BOARD 
#if ENABLE_ATHENA_R_L_IMAGE_SYNC_HANDLER
		if(GetChipID_4K2K()!=CHIPID_4K2K_FE)
      {    
         if(ImageSyncHandleGetModeStableFlag() == FALSE) //ChengLin 20130401: Add for double check Stable Flag status
         {
            ImageSyncHandleSetModeStableFlag(TRUE); //Stable          
         }
       }
#endif
#endif 
         if (UserPrefPowerState != POWER_STATE_OFF)	// Do not set blinking LEDif power off mode
            KeypadLightControl(KEYPAD_LED_GREEN, LED_NORMAL);
         AutoScanInput_SetDefault( gmvb_MainChannel, UserPrefCurrentInputMain );
         ModeSetupRequestCancel(gmvb_MainChannel, REQ_MS_FREERUN, PARAM_NONE);
         //
         // Stable timing has been detected
         // 1) Search the mode table or use estimate algorithm
         //    to determine this is video/graphic mode
         // 2) Set flag for ModeSetupHandler to setup the system
         //
         if (ModeDetermine(gmvb_MainChannel) == TRUE) // Determine mode
         {
            gm_SetRegBitsWord(MC_FIFO_EN_lo, MC_FIFO_MAIN_WR); //After Mode Determine, enable FIFO
         
            #if (DEBUG_HDMI_PRINT_TIMING_DISPLAY_TIME!=0) || (DEBUG_DP_PRINT_TIMING_DISPLAY_TIME!=0)
             gm_Print(" -->MDet: T(%d)",(WORD)(gm_GetSystemTime()-GetDebugDisplayTime()));
            #endif    
            // Graphic mode and xcYCC color space is set then change back to sRGB as default
            if (gm_IsGraphics(gmvb_MainChannel) && (UserPrefColorSpace == ColorSpace_XvYcc))
            {
               UserPrefColorSpace = ColorSpace_sRGB;
            }
            msg("MAIN-Stable and determined",0);
#ifdef VIDEO_CAPTURE
            if (CDATA_Flag == 4)
            {
               CDATA_Flag = 9;
            }
#endif

            B_SignalStatus = STATUS_SYNC_PRESENT;


#ifdef DYNAMIC_CHANNEL_MAPPING
            if (ConditionToSwapChannelMapping() == TRUE)
            {
               msg("MAIN-Swap channels",0);
               SwapChannelMapping();
               SelectInput(gmvb_MainChannel, UserPrefCurrentInputMain, NORMAL_MODE);
               SelectInput(gmvb_PipChannel, UserPrefCurrentInputPip, NORMAL_MODE);
            }
            else
#endif
            {
               SETBIT(stInput[gmvb_MainChannel].ModeFlags, gmd_MODE_VALID);
               ModeSetPrefs(gmvb_MainChannel);
 

               ModeSetupRequest(gmvb_MainChannel, REQ_MS_SETUP, PARAM_NONE, 0);
               OSDRequestCancelMsg(gmvb_MainChannel, MTO_NoSignal);
#ifdef CHECK_IS_CABLE_CONNECT
               //OSDRequestCancelMsg(gmvb_MainChannel, MTO_NoCable); // SCR2456
               #ifdef ST_4K2K_93xx_BOARD
                  if(GetChipID_4K2K()==CHIPID_4K2K_FE)
                  {
							OSDRequestCancelMsg(gmvb_MainChannel, MTO_NoCable);
						}
					#else
               OSDRequestCancelMsg(gmvb_MainChannel, MTO_NoCable);
					#endif
#endif
               OSDRequestCancelMsg(gmvb_MainChannel, MTO_InvalidMode);
               OSDRequestMsg(gmvb_MainChannel, MTO_ValidMode, REQUEST_NOW);
               msg("MAIN-Mode Setup requested",0);
            }

#if 0//(FEATURE_VWD == ENABLE)
            if (UserPrefVwdFlag == ON)
               VWD_Start();
#endif
         #if INVALID_RECHECK_BY_HDMIDP_TIMING_NOT_MATCH
            InValidCheckCnt[gmvb_MainChannel]=0;
         #endif            
            
         }
         else
         {
         #if INVALID_RECHECK_BY_HDMIDP_TIMING_NOT_MATCH
            if((gm_IsInputDP(gmvb_MainChannel) || gm_IsInputHdmiDvi(gmvb_MainChannel)) && (InValidCheckCnt[gmvb_MainChannel]< MAX_INVALID_CHKCNT))
            {
               //When DP/Hdmi/Dvi mode-determine failed, it should not go to invalid mode,
               //instead, it should re-check the timing again.
               gm_ForceModeChanged(gmvb_MainChannel);
               InValidCheckCnt[gmvb_MainChannel]++;
               
            }
            else
            {
         #endif                           
               B_SignalStatus = STATUS_INVALID_MODE;

#if (NON_SCALING_OUTPUT==1)
               gm_ClkSelect(CLK_ODP, ODP_SEL_DCLK);
               gm_ClkSelect(CLK_LVTX_IN, LVTX_IN_SEL_DDDS);
#endif

#if 1 //def RD1_8028_BOARD // Don't request Mode to resetup, this will cause problem. Queue will full.
               ModeSetupRequest(gmvb_MainChannel, REQ_MS_FREERUN, PARAM_NONE, FREE_RUN_DELAY);
#else
               ModeSetupRequest(gmvb_MainChannel, REQ_MS_RESTART, PARAM_NONE, DETECT_REEVALUATE);
#endif
               OSDRequestCancelMsg(gmvb_MainChannel, MTO_NoSignal);
#ifdef CHECK_IS_CABLE_CONNECT
               OSDRequestCancelMsg(gmvb_MainChannel, MTO_NoCable); // SCR2456
#endif
               //OSDRequestMsg(gmvb_MainChannel, MTO_InvalidMode, INVALID_MODE_DELAY);
   				#ifdef ST_4K2K_93xx_BOARD 
   				   if(GetChipID_4K2K()==CHIPID_4K2K_FE)
   				   {
   						OSDRequestMsg(gmvb_MainChannel, MTO_InvalidMode, INVALID_MODE_DELAY);
   					}
   				#else
                  OSDRequestMsg(gmvb_MainChannel, MTO_InvalidMode, INVALID_MODE_DELAY);
   				#endif
         #if INVALID_RECHECK_BY_HDMIDP_TIMING_NOT_MATCH
               InValidCheckCnt[gmvb_MainChannel]=0;
				}
			#endif	
         }
      }
#if 0 //(FEATURE_DISPLAY_PORT == ENABLE) && (DP_NUM_SST_ONLY_TX == 1) && (NON_SCALING_OUTPUT ==1) 
      else
      {
         if (!(gm_ReadRegWord(DPTX_MS_CTRL0) & DPTX_MS_VIDEO_EN))
         {
            DpTxEnableVideo(DPTX_TIMING_BY_INPUT);
         }
      }
#endif
#ifdef ENABLE_DCR
      DCR_Handler();
#endif
   }
   else
   {
      //
      // Timing not stable or no signal detected
      // 1) Is timing has changed from stable to unstable ?
      // 2) If yes, set flag for ModeSetupHandler to setup the system (freerun)
      // 3) Clear the CC decoder when no signal
      //

      if (gm_adapt_InputTimingTransitionFromStable(gmvb_MainChannel) == TRUE)
      {
      #if (FEATURE_AUDIO == ENABLE)
         Audio_VideoSyncLost(gmvb_MainChannel);
      #endif

         #if (FEATURE_AIP == ENABLE) && (FEATURE_ADC_AOC == ENABLE)
         if (AfeGetChannelPortProc(gmvb_MainChannel) == AIP)
         {
            gm_Print("Disable AOC ......",0);
            gm_AOCEnable(gmd_ADC_0, FALSE);
         }
         #endif
      
      #ifdef ST_4K2K_93xx_BOARD 
			/*ChengLin 20130221: For Handle 4k2k platform Image Sync Handler*/
			#if ENABLE_ATHENA_R_L_IMAGE_SYNC_HANDLER
				if(GetChipID_4K2K()!=CHIPID_4K2K_FE)
            {
               if(ImageSyncHandleGetModeStableFlag() == TRUE) //ChengLin 20130401: Add for double check Stable Flag status
               {
					   ImageSyncHandleSetModeStableFlag(FALSE); //Unstable
					   /*ChengLin 20130408: Once signal lost/change from stable, force image ready pin to Low*/    
                     Image_ReadyFlag(FALSE);	                     
               }
              }
			#endif		

         #ifdef DP_HUB_2_LOG_PORTS
         /*ChengLin 20130604: All sources must to reset Flags for avoid HDMI can't go to deep sleep mode*/
         //if(gm_GetDpDigitalID(gB_CurrentInputConnector[gmvb_MainChannel])==DID_DPRX0)
         {
            ResetDpRx0MstInfoUpdatFlag();
            ResetDpRx0VirtualChannelsAllocated();
         }
         #endif
         
			if(GetChipID_4K2K()==CHIPID_4K2K_L)
         {
   			ImageSync_HStart();
				//gm_Print("IPP HStart = ......... %d", gm_ReadRegWord(IPP_IBD_HSTART));
			}
			
         if(GetChipID_4K2K()==CHIPID_4K2K_FE)
         {
            ImageSync_FE(); 
         }

			if(GetChipID_4K2K()==CHIPID_4K2K_R)
         {
         	//gm_Print("Send Remove OSD event ... ", 0);
            OSDRequestMsg(gmvb_MainChannel, MTO_RemoveOSD, REQUEST_NOW);        
         }
			
         
      #endif
         
#if (NON_SCALING_OUTPUT == 1)
         gm_ClkSelect(CLK_ODP, ODP_SEL_DCLK);
         gm_ClkSelect(CLK_LVTX_IN, LVTX_IN_SEL_DDDS);

   #if (PASS_THRU_MODE == 1) || (DEINTERLACED_OUTPUT == 0)
      #if (FEATURE_HDMI == ENABLE)
         if ((GetPortConnType(gB_CurrentInputConnector[gmvb_MainChannel])==HDMI)||(GetPortConnType(gB_CurrentInputConnector[gmvb_MainChannel])==DVI))
         {
#if 0
            if (gm_GetHdmiDviDigitalID(gB_CurrentInputConnector[gmvb_MainChannel]) == DID_HDMI)
            {
               gm_SetRegBitsWord(HDRX_VID_REGEN_CTRL, HDRX_DVI_VS_REGEN);
            }
            else
            {
               gm_SetRegBitsWord(DVI0_VID_REGEN_CTRL, DVI0_DVI_VS_REGEN);
               gm_SetRegBitsWord(DVI1_VID_REGEN_CTRL, DVI1_DVI_VS_REGEN);
            }
#else
              gm_WriteRegWord(HDRX_VID_REGEN_CTRL, 0x00);
#endif
         }
      #endif
   #endif // (PASS_THRU_MODE == 1) || (DEINTERLACED_OUTPUT == 0)

   #if (PASS_THRU_MODE == 0) && (DEINTERLACED_OUTPUT == 0) && (FEATURE_DISPLAY_PORT == ENABLE)
         INTERRUPT_DISABLE(DISPLAY_DEC_IRQ5MASK, D_VS_MASK);
   #endif
   
   #if (FEATURE_DISPLAY_PORT == ENABLE) && (DP_NUM_SST_ONLY_TX == 1)
      #ifdef ST_4K2K_93xx_BOARD
      if(GetChipID_4K2K()==CHIPID_4K2K_FE)
      #endif
      {
         #ifdef ST_4K2K_93xx_BOARD         
         if(
            (gm_ReadRegWord(DH_DE_END) != (PanelWidth + gm_ReadRegWord(DH_DE_START))) ||
            (gm_ReadRegWord(DV_DE_END) != (PanelHeight + gm_ReadRegWord(DV_DE_START)))
            )
         {
            gm_WriteRegWord(DH_DE_END, PanelWidth + gm_ReadRegWord(DH_DE_START));
            gm_WriteRegWord(DV_DE_END, PanelHeight + gm_ReadRegWord(DV_DE_START));
            //DpTxEnableVideo(DPTX_TIMING_BY_OUTPUT);
         }
         #endif
         DpTxDisableVideo();
      }
   #endif
#endif // (NON_SCALING_OUTPUT == 1)

#if (FEATURE_3D == ENABLE)
         gm_3D_OutputTiming_2D_Mode();
#endif

#if (FEATURE_VWD == ENABLE)
         if (UserPrefVwdFlag == ON)
            VWD_Stop();
#endif

#if (FEATURE_ACR == ENABLE)
         if (UserPrefDCR == ON)
            ACR_Disable();
#endif

         // Disable FIFO WR to avoid FIFO overflow
         gm_ClearRegBitsWord(MC_FIFO_EN_lo, MC_FIFO_MAIN_WR);
         
         #if (ENABLE_DFL == 0)
         gm_SetRegBitsWord(DDDS1_CONTROL, DDDS1_FORCE_OPLOOP); // wait to check
         #endif
         // TODO: Louis ... To Do --> Add HDMI mute function.
         //if((GetChannelConnType(gmvb_MainChannel) == CT_DISPLAY_PORT))
         // gm_DpRxHpdPulse(gmd_DPRX_HPD_PULSE_SHORT);

         debug_ist_start(gmvb_MainChannel);
         CLRBIT(stInput[gmvb_MainChannel].ModeFlags, gmd_MODE_VALID |
                gmd_MODE_COMPLETE);
         //stInput[gmvb_MainChannel].Flags= 0; // so far only xvYCC used and clear the flag
         ModeSetupRequestCancel(gmvb_MainChannel, REQ_MS_SETUP, PARAM_NONE);
         //	The blender needs to be disabled till the Modesetup is completed.
         ModeSetupRequestCancel(gmvb_MainChannel, REQ_MS_BLENDER, PARAM_ANY);
         ModeSetupFreerun(gmvb_MainChannel);
         //	ModeSetupRequest(gmvb_MainChannel, REQ_MS_FREERUN, PARAM_NONE, FREE_RUN_DELAY);

#ifdef CHECK_IS_CABLE_CONNECT
         if (!isCableConnected(UserPrefCurrentInputMain)) // SCR2456
         {

            B_SignalStatus = STATUS_NO_CABLE;
            //OSDRequestMsg(gmvb_MainChannel, MTO_NoCable, NO_SIGNAL_DELAY);	// 20120320 Test
            #ifdef ST_4K2K_93xx_BOARD
               if(GetChipID_4K2K()==CHIPID_4K2K_R)
               {
						OSDRequestMsg(gmvb_MainChannel, MTO_NoCable, NO_SIGNAL_DELAY);
					}
					if(GetChipID_4K2K()==CHIPID_4K2K_FE)
					{
						gm_WriteRegWord(DISPLAY_CONTROL,0x01);
					}
				#else
               OSDRequestMsg(gmvb_MainChannel, MTO_NoCable, NO_SIGNAL_DELAY);
				#endif
         }
         else
#endif
         {

            B_SignalStatus = STATUS_NO_SYNC;
            OSDRequestCancelMsg(gmvb_MainChannel, MTO_NoSignal);
            #ifdef ST_4K2K_93xx_BOARD
               if(GetChipID_4K2K()==CHIPID_4K2K_R)
               {
                  #if (!ENABLE_PBP_ATHENA_L_DEEP_SLEEP_CHECK) //Add by ChengLin 20130506                 
						OSDRequestMsg(gmvb_MainChannel, MTO_NoSignal, NO_SIGNAL_DELAY);
                  #else                
                  if(UserPrefCaptureM == CAPTURE_MODE_HALF_HORIZONTAL)
                  {
						   OSDRequestMsg(gmvb_MainChannel, MTO_NoSignal, NO_SIGNAL_DELAY+1500);                   
                     gm_TimerStart(PBP_SLEEP_CHECK_TIMEOUT_1ms_TMR,PBP_SLEEP_WAIT_TIME_OUT);
                  }
                  else
                  {
						   OSDRequestMsg(gmvb_MainChannel, MTO_NoSignal, NO_SIGNAL_DELAY);  
                  }
                  #endif
                  
                  #if ENABLE_TWO_LOGICAL_PORTS_SLEEP_CHECK
                  if((UserPrefCaptureM != CAPTURE_MODE_HALF_HORIZONTAL) &&(gm_GetDpDigitalID(gB_CurrentInputConnector[gmvb_MainChannel])==DID_DPRX0) && (gm_ReadRegWord(DP12RX_RL_CONFIG) & DPRX_TYPE))
                  {                 
                     gm_TimerStart(DP_TWO_LOGICAL_PORTS_SLEEP_CHECK_TIMEOUT_1ms_TMR, (NO_SIGNAL_DELAY-1000));
                     DP_TWO_LogicalPorts_StartPowerCheck_AthenaR = FALSE;                    
                  }
                  #endif     
               }
					if(GetChipID_4K2K()==CHIPID_4K2K_FE)
					{
						gm_WriteRegWord(DISPLAY_CONTROL,0x01);
					}
				#else
               OSDRequestMsg(gmvb_MainChannel, MTO_NoSignal, NO_SIGNAL_DELAY);
				#endif
         }

         if (UserPrefPowerState != POWER_STATE_OFF)	// Do not set blinking LEDif power off mode
            KeypadLightControl(KEYPAD_LED_GREEN, LED_BLINKING);


#if (FEATURE_DISPLAY_PORT == ENABLE)
         if (gm_IsInputDP(gmvb_MainChannel))
            DpRxAfrEnable(UserPrefCurrentInputMain,FALSE);
#endif


#if (defined(SMART_ISP) && defined(SMART_ISP_AUTO_DETECTION))
         EnableSmartIspDetect(FALSE);
#endif


#if PATCH_OF_DVI_AFB_DIABLE == 1       //Patch for AFB always be triggered if input from DVI front-end.
         gm_SetRegBitsWord(OVP_CTRL,MAIN_AFB_EN);

#endif

#if (FEATURE_AIP == ENABLE)
         gm_WriteRegWord(SDDS1_FREQ_DELTA, 0);
#endif

#if (FEATURE_HDMI == ENABLE) && (USE_HDMIDVI_AFR_DETECT_INTERRUPT!=0)
         if ((GetPortConnType(gB_CurrentInputConnector[gmvb_MainChannel])==HDMI)||(GetPortConnType(gB_CurrentInputConnector[gmvb_MainChannel])==DVI))
            HdmiDviEnableAfrInterrupt(gB_CurrentInputConnector[gmvb_MainChannel], FALSE);
#endif

         #if (FEATURE_OVERDRIVE == ENABLE) // OD Pre-frame workaround
         if(UserPrefLCDOverdrive != 0)
         {
            gm_UpdateOverDrive(OD_OFF, 0, 0);
         }
         #endif   

         msg("MAIN-No Signal",0);
      }
#ifdef CHECK_IS_CABLE_CONNECT
      else
      {
         if (!isCableConnected(UserPrefCurrentInputMain)) // SCR2456
         {
            if (B_SignalStatus == STATUS_NO_SYNC)
            {

               B_SignalStatus = STATUS_NO_CABLE;
               OSDRequestCancelMsg(gmvb_MainChannel, MTO_NoSignal); // SCR2456
               //OSDRequestMsg(gmvb_MainChannel, MTO_NoCable, NO_SIGNAL_DELAY);	// 20120320 Test
               #ifdef ST_4K2K_93xx_BOARD 
                  if(GetChipID_4K2K()==CHIPID_4K2K_FE)
                  {
							OSDRequestMsg(gmvb_MainChannel, MTO_NoCable, NO_SIGNAL_DELAY);
						}
					#else
               OSDRequestMsg(gmvb_MainChannel, MTO_NoCable, NO_SIGNAL_DELAY);
					#endif
            }
         }
         else
         {
            if (B_SignalStatus == STATUS_NO_CABLE)
            {

               B_SignalStatus = STATUS_NO_SYNC;
               OSDRequestCancelMsg(gmvb_MainChannel, MTO_NoCable); // SCR2456		// 20120320 Test
               #ifdef ST_4K2K_93xx_BOARD
                  if(GetChipID_4K2K()==CHIPID_4K2K_FE)
                  {
							OSDRequestMsg(gmvb_MainChannel, MTO_NoSignal, NO_SIGNAL_DELAY);
						}
					#else
               OSDRequestMsg(gmvb_MainChannel, MTO_NoSignal, NO_SIGNAL_DELAY);
					#endif
            }
         }

      }
#endif
#if PATCH_OF_DVI_AFB_DIABLE == 1       //Patch for AFB always be triggered if input from DVI front-end.
      if (!(gm_ReadRegWord(OVP_CTRL) & MAIN_AFB_EN))
         gm_SetRegBitsWord(OVP_CTRL,MAIN_AFB_EN);
#endif

   }
}

//******************************************************************************
//
// FUNCTION       : void PipModeHandler(void)
//
// USAGE          : Processes the mode handler for PIP channel
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        : gmvb_PipChannel		(R)
//                  gmvw_InputModeAttr	(R/W)
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void PipModeHandler(void)
{
   static BYTE	B_NoSignalStatus = STATUS_NO_SYNC;

#if (FEATURE_3D == ENABLE)
   if (gm_3D_Is3DModeEnabled())
   {
      return;
   }
#endif

   //
   // Check input signal status
   // 1) Is timing stable ?
   // 2) Is timing has changed from unstable to stable ?
   //
#ifdef ST_4K2K_93xx_BOARD
   //Temporary: to avoid Athena_L DIP cannot go to mode determine and mode setup.
   if ((gm_adapt_TimingIsStable(gmvb_PipChannel) == TRUE) && (GetChipID_4K2K()!=CHIPID_4K2K_L))
#else
   if (gm_adapt_TimingIsStable(gmvb_PipChannel) == TRUE)
#endif   
   {
#if 0 //(FEATURE_HDMI==ENABLE)
      // HDMI port
      if (IsInternalHdmiDviPort(gmvb_PipChannel))
      {
         if (HdmiStartModeDetection() == FALSE)
            return;
      }
#endif
      if (gm_adapt_InputTimingTransitionToStable(gmvb_PipChannel) == TRUE)
      {
      #if (FEATURE_AUDIO == ENABLE)
         Audio_VideoSyncDetected(gmvb_PipChannel);
      #endif

         stInput[gmvb_PipChannel].Flags= 0; // so far only xvYCC used and clear the flag
         AutoScanInput_SetDefault( gmvb_PipChannel, UserPrefCurrentInputPip );
         if (UserPrefPipMode != NO_PIP)
            msg("PIP-Stable",0);
         //
         // Stable timing has been detected
         // 1) Search the mode table or use estimate algorithm
         //    to determine this is video/graphic mode
         // 2) Set flag for ModeSetupHandler to setup the system
         //
         if (ModeDetermine(gmvb_PipChannel) == TRUE)	// Determine mode
         {
            gm_SetRegBitsWord(MC_FIFO_EN_lo, MC_FIFO_PIP_WR); //After Mode Determine, enable FIFO        
            SETBIT(stInput[gmvb_PipChannel].ModeFlags, gmd_MODE_VALID);
            ModeSetPrefs(gmvb_PipChannel);

 
            ModeSetupRequest(gmvb_PipChannel, REQ_MS_SETUP, PARAM_NONE, MODE_SETUP_PIP_DELAY);
            OSDRequestCancelMsg(gmvb_PipChannel, MTO_NoSignal);
#ifdef CHECK_IS_CABLE_CONNECT
            OSDRequestCancelMsg(gmvb_PipChannel, MTO_NoCable); // SCR2456
#endif
            OSDRequestCancelMsg(gmvb_PipChannel, MTO_InvalidMode);
            OSDRequestMsg(gmvb_PipChannel, MTO_ValidMode, REQUEST_NOW);
            //AutoScanInput_SetDefault( gmvb_PipChannel, UserPrefCurrentInputPip );

            msg("PIP-Stable and determined",0);
            
            #if INVALID_RECHECK_BY_HDMIDP_TIMING_NOT_MATCH
            InValidCheckCnt[gmvb_PipChannel]=0;
            #endif            
         }
         else
         {
         #if INVALID_RECHECK_BY_HDMIDP_TIMING_NOT_MATCH
            if((gm_IsInputDP(gmvb_PipChannel) || gm_IsInputHdmiDvi(gmvb_PipChannel)) && (InValidCheckCnt[gmvb_PipChannel]< MAX_INVALID_CHKCNT))
            {
               //When DP/Hdmi/Dvi mode-determine failed, it should not go to invalid mode,
               //instead, it should re-check the timing again.
               gm_ForceModeChanged(gmvb_PipChannel);
               InValidCheckCnt[gmvb_PipChannel]++;
               
            }
            else
            {
         #endif                           
               ModeSetupRequest(gmvb_PipChannel, REQ_MS_RESTART, PARAM_NONE, DETECT_REEVALUATE);
               OSDRequestCancelMsg(gmvb_PipChannel, MTO_NoSignal);
               OSDRequestMsg(gmvb_PipChannel, MTO_InvalidMode, INVALID_MODE_DELAY);
         #if INVALID_RECHECK_BY_HDMIDP_TIMING_NOT_MATCH
               InValidCheckCnt[gmvb_PipChannel]=0;
            }
         #endif   
         }
      }
   }
   else
   {
      //
      // Timing not stable or no signal detected
      // 1) Is timing has changed from stable to unstable ?
      // 2) If yes, set flag for ModeSetupHandler to setup the system (freerun)
      //
      if (gm_adapt_InputTimingTransitionFromStable(gmvb_PipChannel) == TRUE)
      {
      #if (FEATURE_AUDIO == ENABLE)
         Audio_VideoSyncLost(gmvb_PipChannel);
      #endif

         debug_ist_start(gmvb_PipChannel);
         CLRBIT(stInput[gmvb_PipChannel].ModeFlags, gmd_MODE_VALID |
                gmd_MODE_COMPLETE);
         //stInput[gmvb_PipChannel].Flags= 0; // so far only xvYCC used and clear the flag
         ModeSetupRequestCancel(gmvb_PipChannel, REQ_MS_SETUP, PARAM_NONE);
         //The blender needs to be disabled till the Modesetup is completed.
         ModeSetupRequestCancel(gmvb_PipChannel, REQ_MS_BLENDER, PARAM_ANY);

         // Disable FIFO WR to avoid FIFO overflow
         gm_ClearRegBitsWord(MC_FIFO_EN_lo, MC_FIFO_PIP_WR);
         
         ModeSetupFreerun(gmvb_PipChannel);
//			ModeSetupRequest(gmvb_PipChannel, REQ_MS_FREERUN, PARAM_NONE, FREE_RUN_DELAY);

         #if (FEATURE_AIP == ENABLE) && (FEATURE_ADC_AOC == ENABLE)
         if (AfeGetChannelPortProc(gmvb_PipChannel) == AIP)
         {
            gm_AOCEnable(gmd_ADC_0, FALSE);
         }
         #endif
         
#ifdef CHECK_IS_CABLE_CONNECT
         if (!isCableConnected(UserPrefCurrentInputPip)) // SCR2456
         {
            B_NoSignalStatus = STATUS_NO_CABLE;
            OSDRequestMsg(gmvb_PipChannel, MTO_NoCable, NO_SIGNAL_DELAY);
         }
         else
#endif				
         {
            B_NoSignalStatus = STATUS_NO_SYNC;

            OSDRequestCancelMsg(gmvb_PipChannel, MTO_NoSignal);

            OSDRequestMsg(gmvb_PipChannel, MTO_NoSignal, NO_SIGNAL_DELAY);
         }


#if (FEATURE_DISPLAY_PORT == ENABLE)
         if (gm_IsInputDP(gmvb_PipChannel))
            DpRxAfrEnable(UserPrefCurrentInputPip,FALSE);
#endif

#if PATCH_OF_DVI_AFB_DIABLE == 1       //Patch for AFB always be triggered if input from DVI front-end.
         gm_SetRegBitsWord(OVP_CTRL,PIP_AFB_EN);
#endif

#if (FEATURE_HDMI == ENABLE) && (USE_HDMIDVI_AFR_DETECT_INTERRUPT!=0)
         if ((GetPortConnType(gB_CurrentInputConnector[gmvb_PipChannel])==HDMI)||(GetPortConnType(gB_CurrentInputConnector[gmvb_PipChannel])==DVI))
            HdmiDviEnableAfrInterrupt(gB_CurrentInputConnector[gmvb_PipChannel], FALSE);
#endif

         if (UserPrefPipMode != NO_PIP)
            msg("PIP-No Signal",0);
      }
#ifdef CHECK_IS_CABLE_CONNECT
      else
      {
         if (!isCableConnected(UserPrefCurrentInputPip)) // SCR2456
         {
            if (B_NoSignalStatus == STATUS_NO_SYNC)
            {
               B_NoSignalStatus = STATUS_NO_CABLE;
               OSDRequestCancelMsg(gmvb_PipChannel, MTO_NoSignal); // SCR2456
               OSDRequestMsg(gmvb_PipChannel, MTO_NoCable, NO_SIGNAL_DELAY);
            }
         }
         else
         {
            if (B_NoSignalStatus == STATUS_NO_CABLE)
            {
               B_NoSignalStatus = STATUS_NO_SYNC;
               OSDRequestCancelMsg(gmvb_PipChannel, MTO_NoCable); // SCR2456
               OSDRequestMsg(gmvb_PipChannel, MTO_NoSignal, NO_SIGNAL_DELAY);
            }
         }
      }
#endif

#if PATCH_OF_DVI_AFB_DIABLE == 1       //Patch for AFB always be triggered if input from DVI front-end.
      if (!(gm_ReadRegWord(OVP_CTRL) & PIP_AFB_EN))
         gm_SetRegBitsWord(OVP_CTRL,PIP_AFB_EN);
#endif

   }
}

 /*********************************  END  **************************************/
