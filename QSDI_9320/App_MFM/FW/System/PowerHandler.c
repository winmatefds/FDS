/*
    $Workfile:   PowerHandler.c  $
    $Revision: 1.126 $
    $Date: 2013/07/18 04:02:51 $
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
// MODULE:      PowerHandler.c
//
// USAGE:
//
//******************************************************************************

//******************************************************************************
//  I N C L U D E    F I L E S
//******************************************************************************
#include <string.h>
#include "System\all.h"
#include "StdLibInc\gm_mcu186.h"
#include "StdLibInc\gm_Register.h"
#ifdef LOW_POWER_MONITOR_SUPPORT
#include "StdLibInc\gm_lpm_ipc.h"
#endif
#include "Board\PortAttrib.h"		//

//******************************************************************************
//  L O C A L    D E F I N I T I O N S
//******************************************************************************
//#define _PWR_HDR_DEBUG_MSG
//#define _PWR_HDR_EXT_DEBUG_MSG

#ifdef _PWR_HDR_DEBUG_MSG
#define pwr_msg(x,y)            gm_Print(x,y)
#else
#define pwr_msg(x,y)
#endif


#ifdef _PWR_HDR_EXT_DEBUG_MSG
#define pwr_ext_msg(x,y)            gm_Print(x,y)
#else
#define pwr_ext_msg(x,y)
#endif



#define UART_BAUD_RATE_LOW  115200L // 19200L
#define UART_BAUD_RATE_HIGH 115200L
#define LPM_DETECT_PORT_NUMBER 4
#define LPM_DETECT_PORT_NONE 0xFFFF

//*****************************************************************************
//  G L O B A L    V A R I A B L E S
//*****************************************************************************
gmt_PwrStateType PwrState;
WORD PwrTransitionCount;
BYTE PIP_STATUS=0;
BOOL IsPwrTransitionKeyPressed = FALSE;

#ifdef VIDEO_CAPTURE
extern BYTE CDATA_Flag;
extern DWORD Photo_Index;
#endif

#ifdef ST_4K2K_93xx_BOARD //Add by ChengLin 20130315
extern BOOL AllowKeypadAction;

#if ENABLE_PBP_ATHENA_L_DEEP_SLEEP_CHECK               
extern BOOL AthenaRIgnoreSleep_PBP;             
extern BOOL PBP_AthenaR_ReCheck_AthenaLStatus;
BOOL PBP_StartPowerCheck_AthenaR = FALSE;
#endif

#if ENABLE_TWO_LOGICAL_PORTS_SLEEP_CHECK  
BOOL DP_TWO_LogicalPorts_StartPowerCheck_AthenaR = FALSE;
#endif

#endif

//*****************************************************************************
//  G L O B A L    F U N C T I O N S
//*****************************************************************************
#if (FEATURE_CEC == ENABLE)
BOOL    CEC_EXT_FUNC gm_CecTxFrame(BYTE B_DestLogAddr, BYTE B_DataLen, BYTE CEC_RAM * Bp_Data);
#endif


WORD LpmWakeupPortInfo(gmt_UserDefConnNames B_InputConnector);

//*****************************************************************************
//  C O D E
//*****************************************************************************

//******************************************************************************
//
// FUNCTION       : static void PowerOn(void)
//
// USAGE          : Power Handler functions
//
// INPUT          : None
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
static void PowerOn(void)
{
   // Power Up all blocks
// 20090316 #1, Correct DMA for power up, resolve HDMI hang issue.
   gm_Delay10ms(1); //For DP wake up hange up issue at 0x600=1,
   // 20121106, Turn 0ff LVDS output for power seq.

   gm_PowerUpAll();

   // 20090316 #5, UART baud rate 115200 for oprate mode.
#if (FEATURE_GPROBE_UART == REV2)
   {
#ifndef USE_SECOND_UART
      gm_UartInit(UART0, gmc_OCLK_FREQ, UART_BAUD_RATE_HIGH, &uartData);
#else
      gm_UartInit(UART1, gmc_OCLK_FREQ, UART_BAUD_RATE_HIGH, &uartData);
      gm_SetPrintPort(SERIAL_USE_UART1);
#endif
   }
#endif

   // Change color of keypad LED
   KeypadLightControl(KEYPAD_LED_GREEN, LED_NORMAL);

   //
   // Restart Mode Detection/Setup
   //
   //ModeSetupInit();
   SelectInput(gmvb_MainChannel, UserPrefCurrentInputMain, NORMAL_MODE);

#ifdef PIP_PORT_INDEPENDENCE
   // If there is port collision, disable PIP mode detection, otherwise do normal SelectInput()
   if (!IsMainSourceValid(UserPrefCurrentInputMain))
   {
      DisablePipModeDetection();
   }
   else
#endif
#ifndef NO_PIP_SUPPORT
   {
      SelectInput(gmvb_PipChannel, UserPrefCurrentInputPip, NORMAL_MODE);
   }
#endif

   //
   // Audio
   // AVS control (bypass or delay)
   //

   // Display the pip window if it was on before power off
   if (UserPrefPipMode != NO_PIP)
      ModeSetupRequest(gmvb_PipChannel, REQ_MS_BLENDER, PARAM_NONE, STARTUP_PIP_TMO);
AdjustBackLight(); //LEO 20110728 ADD
   // 20100209 #2, Bypass MPE for Jupiter
   // 20100210 #1, System hang after wake up ...
}

//******************************************************************************
//
// FUNCTION       : static void LPMPowerOff(void)
//
// USAGE          : Power Handler functions
//
// INPUT          : None
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
static void LPMPowerOff(void)
{
#ifdef LOW_POWER_MONITOR_SUPPORT
   WORD Power_state[5];
   BYTE ReTryCounter=10;
   Power_state[0] = Pwr_Off;
   Power_state[1] = LpmWakeupPortInfo(UserPrefCurrentInputMain);
   Power_state[2] = LpmWakeupPortInfo(UserPrefCurrentInputPip);
#if 1 
   INTERRUPT_DISABLE(MAIN_INPUT_IRQ3MASK, 0xFFFF);
   INTERRUPT_DISABLE(PIP_INPUT_IRQ4MASK, 0xFFFF);
   INTERRUPT_DISABLE(DISPLAY_DEC_IRQ5MASK, 0xFFFF);
   INTERRUPT_DISABLE(MISC_OCMMASK, ACC_UPDATE_MASK|RSREADY_MASK);
#else   
   asm cli;//disable all interrupt to prepare for shutdown
   //gm_WriteRegWord(LPM_MISSION_CONTROL, GO_LOW_POWER|MISSION_SHUTDOWN);
#endif

   //Disable i2c slave during the Pbus ownership switching period to avoid hangs up!
   gm_ClearRegBitsWord(I2C_SLAVE1_CTRL1, I2C_SLAVE1_EN);
   gm_ClearRegBitsWord(I2C_SLAVE2_CTRL1, I2C_SLAVE2_EN);
   gm_ClearRegBitsWord(I2C_SLAVE3_CTRL1, I2C_SLAVE3_EN);
   gm_ClearRegBitsWord(I2C_SLAVE4_CTRL1, I2C_SLAVE4_EN);


   pwr_msg("PWR_HDR: LPM Power Off",0);
   do
   {
      gm_MessageSend(gmd_MESSAGE_POWER, &Power_state[0], sizeof(Power_state));
      {   //wait for shut down by LPM
         gmt_STimeEx timer;
         gm_TimerStartEx(&timer, 1000, 0); //timeout = 1s
         while (TRUE)
         {
            if (gm_TimerCheckEx(&timer,0,0) == TIMER_TMO)
            {
               pwr_ext_msg("PWR_HDR: Error! OCM is not being shutdown by LPM",0);
               ReTryCounter--;
               break;
            }
         }
      }
   } while(ReTryCounter);
   pwr_ext_msg("PWR_HDR: Fail to shutdown Mission OCM by LPM",0);
#endif
}

//******************************************************************************
//
// FUNCTION       : static void LPMSleep(void)
//
// USAGE          : Power Handler functions
//
// INPUT          : None
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
static void LPMSleep(void)
{
#ifdef LOW_POWER_MONITOR_SUPPORT
   // To do ... store system parameters for mission power down for system sleep.
   WORD Power_state[5];
   BYTE ReTryCounter=10;
   Power_state[0] = Pwr_Sleep;
   Power_state[1] = LpmWakeupPortInfo(UserPrefCurrentInputMain);
   Power_state[2] = LPM_DETECT_PORT_NONE;
   Power_state[3] = LPM_DETECT_PORT_NONE;
   Power_state[4] = LPM_DETECT_PORT_NONE;

#if defined(AUTO_SCAN_INPUT)
   Power_state[0] |= LPM_DETECT_PORT_NUMBER<<8;
   Power_state[1] = LpmWakeupPortInfo(ASInputRings[0]);
   Power_state[2] = LpmWakeupPortInfo(ASInputRings[1]);
   Power_state[3] = LpmWakeupPortInfo(ASInputRings[2]);
   Power_state[4] = LpmWakeupPortInfo(ASInputRings[3]);
#else
   if (UserPrefPipMode != NO_PIP)
      Power_state[0] |= 2<<8; // Main and PIP channel
   else
      Power_state[0] |= 1<<8; // Main channel only
#endif

   //Disable i2c slave during the Pbus ownership switching period to avoid hangs up!
   gm_ClearRegBitsWord(I2C_SLAVE1_CTRL1, I2C_SLAVE1_EN);
   gm_ClearRegBitsWord(I2C_SLAVE2_CTRL1, I2C_SLAVE2_EN);
   gm_ClearRegBitsWord(I2C_SLAVE3_CTRL1, I2C_SLAVE3_EN);
   gm_ClearRegBitsWord(I2C_SLAVE4_CTRL1, I2C_SLAVE4_EN);

   pwr_msg("PWR_HDR: LPM Sleep",0);
   do
   {
      gm_MessageSend(gmd_MESSAGE_POWER, &Power_state[0], sizeof(Power_state));
      {   //wait for shut down by LPM
         gmt_STimeEx timer;
         gm_TimerStartEx(&timer, 1000, 0); //timeout = 1s
         while (TRUE)
         {
            if (gm_TimerCheckEx(&timer,0,0) == TIMER_TMO)
            {
               pwr_ext_msg("PWR_HDR: Error! OCM is not being shutdown by LPM",0);
               ReTryCounter--;
               break;
            }
         }
      }
   } while(ReTryCounter);
   pwr_ext_msg("PWR_HDR: Fail to shutdown Mission OCM by LPM",0);
#endif
}

//******************************************************************************
//
// FUNCTION       : static void WaitPanelPowerOff(void)
//
// USAGE          : Wait Panel power off sequence finish.
//
// INPUT          : None
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
static void WaitPanelPowerOff(void)
{
   WORD B_DelayCount=0;

   while(1)
   {       
      if(gm_ReadRegByte(PANEL_POWER_STATUS) == 0)
         break;
      gm_Delay1ms(1);
      B_DelayCount++;
      
      if (B_DelayCount >= 1000) // wait 1 sec for panel power down
      {      
         //gm_Printf("PANEL_POWER_STATUS = 0x%x", gm_ReadRegByte(PANEL_POWER_STATUS));
         break;               
      }
   }
}

//******************************************************************************
static void PowerOff(void)
{

   BYTE Ba_Data[2];
#if 0 //(FEATURE_DISPLAY_PORT == ENABLE)
   // Louis ... To Do --> Add Disable all interrut for DPRX function
#endif
   /*ChengLin 20130222: Athena Right and Left Must black screen at the same time*/
#ifdef ST_4K2K_93xx_BOARD 
   #if ENABLE_ATHENA_R_L_IMAGE_SYNC_HANDLER
	if(GetChipID_4K2K()==CHIPID_4K2K_R)
		SOCIPC_SetAthena_Right_Left_PB_Ctrl_Status(FALSE);
   #endif	
#endif

   SetDisplayOff(gmvb_MainChannel, BLENDER_OFF);
   SetDisplayOff(gmvb_PipChannel, BLENDER_OFF);
   ResetTimerUserPrefs();
   ClearOsdKeyQueue();

#if (FEATURE_CEC == ENABLE)
   Ba_Data[0] = CEC_MSG_STANDBY;
   gm_CecTxFrame(CEC_BROADCAST_LOG_ADDR, 1, Ba_Data);
   gm_Delay100ms(10);
#endif

#if (FEATURE_AIP == ENABLE) && (FEATURE_ADC_AOC == ENABLE)
   // Turn Off AOC (automatic offset compensation)
   gm_AOCEnable(gmd_ADC_0, FALSE);
#endif

   // Clean up all scheduler queues
   SchedulerInit();

	#ifdef ST_4K2K_93xx_BOARD 
	if(GetChipID_4K2K()==CHIPID_4K2K_R)
	{
     	PanelPowerOff4k2k();
	}
	#endif
	
   gm_PowerDownAll();

   //UART baud rate 19200 for sleep mode.
#if (FEATURE_GPROBE_UART == REV2)
   {
   #ifndef USE_SECOND_UART
      if ((GetChannelConnType((gmvb_MainChannel) != CT_DISPLAY_PORT)))
         gm_UartInit(UART0, gmc_TCLK_FREQ, UART_BAUD_RATE_LOW, &uartData);
      else
         gm_UartInit(UART0, gmc_OCLK_FREQ, UART_BAUD_RATE_LOW, &uartData);
   #else
      gm_UartInit(UART1, gmc_OCLK_FREQ, UART_BAUD_RATE_LOW, &uartData);
      gm_SetPrintPort(SERIAL_USE_UART1);
   #endif
   }
#endif

}

void PowerRequestOff(void)
{
#if (FEATURE_AUDIO == ENABLE)
   SYS_API_AUDIO_POWER_DOWN();
#endif


#ifdef ST_4K2K_93xx_BOARD 
	if(GetChipID_4K2K()== CHIPID_4K2K_R)
	{	
	   LPM_Set_DCFlag(TRUE);
	}
#endif

   OSDRequestMsg(gmvb_MainChannel, MTO_PowerDownNow, REQUEST_NOW);

#if (FEATURE_LOW_POWER_MONITOR == ENABLE)
   LPM_CalibrateRingOSC(); //Re-calibration before DC OFF
   LPM_SendCalibratedTrim();
#endif

   // We need to check if previous state is deep sleep because if we
   // call PowerOff() twice, gm_PowerDownAll() will incorrectly set
   // the two system clocks causing the system to behave strangely on powerup.
   if (PwrState != Pwr_DeepSleep)
      PowerOff();

   KeypadLightControl(KEYPAD_LED_OFF, LED_NORMAL); // turn off LED
   WaitPanelPowerOff();
   LPMPowerOff();
      

 // BK_OFF();
//   gm_Delay100ms(100);


	
}

void PowerRequestOn(void)
{
   PowerOn();
 //  BK_ON(); //leo add
   PwrState = Pwr_On;
   pwr_msg("PowerRequestOn ",0);
   ResetTimerUserPrefs();
   OSDRequestMsg(gmvb_MainChannel, MTO_PowerUp, REQUEST_NOW);
   OSDRequestMsg(gmvb_MainChannel, MTO_DisplayOn, 2000);    // Wait to call SaveModeIndependentSettings after seeing display
  
}

BOOL Into_Pwr_Transition = FALSE;
void PowerTransition(void)
{
   gm_TimerStop(PWR_TRANSITION_100ms_TMR);
   PwrState = Pwr_Transition;
   IsPwrTransitionKeyPressed = FALSE;
   //KeypadLightControl(KEYPAD_LED_RED, LED_NORMAL);
}

void PowerDeepSleepInRequest(void)
{
#if (FEATURE_LOW_POWER_MONITOR == ENABLE)

if((UserPrefCurrentInputMain == ALL_BOARDS_CVBS1)&& (UserPrefCurrentInputMain == ALL_BOARDS_SVIDEO1))
{
//BK_OFF();
return;

}

   if (PwrState == Pwr_Transition && gm_TimerCheck(PWR_TRANSITION_100ms_TMR) != TIMER_TMO)
   {
      //msg("Pwr_Transition Ignore Pwr_DeepSleep ");
      return;
   }

   /*ChengLin 20130509: If OSD into Deep Sleep Mode, for MST 1 stream on Athena L must to ignore Deep Sleep.*/
   #if 0//def ST_4K2K_93xx_BOARD
   #if ENABLE_TWO_LOGICAL_PORTS_SLEEP_CHECK
   if(GetChipID_4K2K()== CHIPID_4K2K_R)
   {
      if((UserPrefCaptureM != CAPTURE_MODE_HALF_HORIZONTAL) &&(gm_GetDpDigitalID(gB_CurrentInputConnector[gmvb_MainChannel])==DID_DPRX0) && (gm_ReadRegWord(DP12RX_RL_CONFIG) & DPRX_TYPE))
      {
         SocIPCSendCmdPacketParser(SOCIPC_DEVICEL_DST_ADDRESS, SICMD_RO_AthenaL_SyncStatus, NULL);
         if((IsSyncOn() == FALSE) && (gAthena_LEFT_FE_Slave_Parameters.AthenaLSyncStatus==TRUE))
         {
            pwr_ext_msg("PWR_HDR: DP MST and Athena L valid- Ignore Deep Sleep(%d)!!",PwrState);
            if (PwrState != Pwr_Transition)
            {  
               KeypadLightControl(KEYPAD_LED_RED, LED_NORMAL);
               PwrState = Pwr_Transition;
               IsPwrTransitionKeyPressed = TRUE;
               PwrTransitionCount = 50;
               gm_TimerStart(PWR_TRANSITION_100ms_TMR, PwrTransitionCount);
            }
         return;
         }          
   }
   }
   #endif
   #endif
   
   #if defined(DEEP_SLEEP_CONDITION)

//   if((isSourcePowerConnected(UserPrefCurrentInputMain) && isCableConnected(UserPrefCurrentInputMain)) ||
//      ((UserPrefPipMode != NO_PIP) && isSourcePowerConnected(UserPrefCurrentInputPip) && isCableConnected(UserPrefCurrentInputPip)) )
if((UserPrefCurrentInputMain == ALL_BOARDS_CVBS1)||((UserPrefCurrentInputMain == ALL_BOARDS_SVIDEO1)))
   {
      pwr_ext_msg("PWR_HDR: Power/Cable connected",0);
      if (PwrState != Pwr_Transition)
      {
         pwr_msg("PWR_HDR: Wait Pwr_DeepSleep (%d00ms)", DEEP_SLEEP_CONDITION_TIME); 
         KeypadLightControl(KEYPAD_LED_RED, LED_NORMAL);
         PwrState = Pwr_Transition;
         IsPwrTransitionKeyPressed = FALSE;
         PwrTransitionCount = DEEP_SLEEP_CONDITION_TIME;
         gm_TimerStart(PWR_TRANSITION_100ms_TMR, PwrTransitionCount);
			// test 20130411
			#if 0//def ST_4K2K_93xx_BOARD
				if(GetChipID_4K2K()==CHIPID_4K2K_R)
				{
					PanelPowerOff4k2k();
					Into_Pwr_Transition = TRUE;
				}

			#endif
         return;
      }
   }

   #endif

   //pwr_msg("PWR_HDR: PwrDeepSleep",0);
   PwrState = Pwr_DeepSleep;

   if (SystemFlags.sys.IgnoreSleep == TRUE)
   {
      pwr_msg("PWR_HDR: Ignore PwrDeepSleep",0);
      return;
   }


	#if 0//def ST_4K2K_93xx_BOARD
		if(GetChipID_4K2K()!= CHIPID_4K2K_R)
		{
			pwr_ext_msg("PWR_HDR: L/FE Power off by R",0);
			return;
		}
		else
		{
         #if ENABLE_PBP_ATHENA_L_DEEP_SLEEP_CHECK 			
         if(UserPrefCaptureM == CAPTURE_MODE_HALF_HORIZONTAL)
         {    
           pwr_ext_msg("PWR_HDR: Fake Sleep During PBP Mode.",0);
            if (PwrState != Pwr_Transition)
            {
               KeypadLightControl(KEYPAD_LED_RED, LED_NORMAL);
               PwrState = Pwr_Transition;
               IsPwrTransitionKeyPressed = FALSE;
               PwrTransitionCount = 50;
               gm_TimerStart(PWR_TRANSITION_100ms_TMR, PwrTransitionCount);
            }

            #if (FEATURE_AUDIO == ENABLE)
               SYS_API_AUDIO_POWER_DOWN();
            #endif
               // Turn off Panel
               PanelPowerOff4k2k();
               Into_Pwr_Transition = TRUE;
               return;         
         }
         #endif               
		
			if(UserPrefCurrentInputMain == ALL_BOARDS_DP2)
			{      
				pwr_ext_msg("PWR_HDR: R bypass Sleep for HDMI",0);
				goto Bypass_Sleep_Block;
			}
		}
	#endif


#if (FEATURE_DISPLAY_PORT == ENABLE) 
   if((UserPrefPipMode != NO_PIP) && (GetPortConnType(UserPrefCurrentInputPip)==CT_DISPLAY_PORT))
   {
      if(DpRxSleepModeBlocker(UserPrefCurrentInputPip)==TRUE)
      {
         pwr_ext_msg("PWR_HDR: Ignore PwrDeepSleep by DpRxSleepModeBlocker(%d)",UserPrefCurrentInputPip);
         if (PwrState != Pwr_Transition)
         {
            KeypadLightControl(KEYPAD_LED_RED, LED_NORMAL);
            PwrState = Pwr_Transition;
            IsPwrTransitionKeyPressed = FALSE;
            PwrTransitionCount = 50;
            gm_TimerStart(PWR_TRANSITION_100ms_TMR, PwrTransitionCount);
         }
         return;
      }
   }

   if(GetPortConnType(UserPrefCurrentInputMain)==CT_DISPLAY_PORT)
   {
      if(DpRxSleepModeBlocker(UserPrefCurrentInputMain)==TRUE)
      {
         pwr_ext_msg("PWR_HDR: Ignore PwrDeepSleep by DpRxSleepModeBlocker(%d)",UserPrefCurrentInputMain);
         if (PwrState != Pwr_Transition)
         {
            KeypadLightControl(KEYPAD_LED_RED, LED_NORMAL);
            PwrState = Pwr_Transition;
            IsPwrTransitionKeyPressed = FALSE;
            PwrTransitionCount = 50;
            gm_TimerStart(PWR_TRANSITION_100ms_TMR, PwrTransitionCount);
         }
         return;
      }
   }
#endif //(FEATURE_DISPLAY_PORT == ENABLE) 

	// 20130204
	#ifdef ST_4K2K_93xx_BOARD

Bypass_Sleep_Block:

	if(GetChipID_4K2K()==CHIPID_4K2K_R)
	{
   	if(UserPrefCurrentInputMain != ALL_BOARDS_DP2)
   	{
   		LPM_Set_DCFlag(TRUE);
   	}
		else
		{
			LPM_Set_DCFlag(FALSE);
		}
	}
	#endif 

   pwr_msg("PWR_HDR: Deep Sleep",0);
   {
   #if (FEATURE_AUDIO == ENABLE)
      SYS_API_AUDIO_POWER_DOWN();
   #endif

      // Turn off Panel
      #ifdef ST_4K2K_93xx_BOARD
         if(GetChipID_4K2K()==CHIPID_4K2K_R)
         {
	      	PanelPowerOff4k2k();
			}
		#else
			gm_DisplayOnOff(Off);
      	WaitPanelPowerOff();			
      #endif
		
		 
      if (SystemFlags.sys.MissionSleep)
      {
         pwr_msg("PWR_HDR: Mission Sleep",0);

         #if 0 // ToDo: implement power off sequence
         PowerOff(); 
         #endif
         KeypadLightControl(KEYPAD_LED_RED, LED_NORMAL); // turn on amber LED

         #if 0 //(FEATURE_HDMI == ENABLE)
         // 20080930 #4, ... Set DVI to standby in sleep mode in DVI (HDMI) port.
         if (((GetChannelConnType(gmvb_MainChannel) == DVI) ||(GetChannelConnType(gmvb_MainChannel) == HDMI))
               || (((GetChannelConnType(gmvb_PipChannel) == DVI) ||(GetChannelConnType(gmvb_PipChannel) == HDMI)) && (PIP_STATUS)))
         {
            // Louis ... To Do --> Add standby mode for DVI (HDMI).
         }
         #endif
      }
      else
      {
         // Power down CPhy.
         gm_SetRegBitsDWord(CPHY0_L0_CTRL_0, CPHY0_L0_IDDQEN);
         gm_SetRegBitsDWord(CPHY0_L1_CTRL_0, CPHY0_L1_IDDQEN);
         gm_SetRegBitsDWord(CPHY0_L2_CTRL_0, CPHY0_L2_IDDQEN);
         gm_SetRegBitsDWord(CPHY0_L3_CTRL_0, CPHY0_L3_IDDQEN);

         gm_SetRegBitsDWord(CPHY1_L0_CTRL_0, CPHY1_L0_IDDQEN);
         gm_SetRegBitsDWord(CPHY1_L1_CTRL_0, CPHY1_L1_IDDQEN);
         gm_SetRegBitsDWord(CPHY1_L2_CTRL_0, CPHY1_L2_IDDQEN);
         gm_SetRegBitsDWord(CPHY1_L3_CTRL_0, CPHY1_L3_IDDQEN);

         gm_SetRegBitsDWord(CPHY2_L0_CTRL_0, CPHY2_L0_IDDQEN);
         gm_SetRegBitsDWord(CPHY2_L1_CTRL_0, CPHY2_L1_IDDQEN);
         gm_SetRegBitsDWord(CPHY2_L2_CTRL_0, CPHY2_L2_IDDQEN);
         gm_SetRegBitsDWord(CPHY2_L3_CTRL_0, CPHY2_L3_IDDQEN);

         // Shut down mission
         LPMSleep();

         // Power up CPhy. for shut down fail
         gm_ClearRegBitsDWord(CPHY0_L0_CTRL_0, CPHY0_L0_IDDQEN);
         gm_ClearRegBitsDWord(CPHY0_L1_CTRL_0, CPHY0_L1_IDDQEN);
         gm_ClearRegBitsDWord(CPHY0_L2_CTRL_0, CPHY0_L2_IDDQEN);
         gm_ClearRegBitsDWord(CPHY0_L3_CTRL_0, CPHY0_L3_IDDQEN);

         gm_ClearRegBitsDWord(CPHY1_L0_CTRL_0, CPHY1_L0_IDDQEN);
         gm_ClearRegBitsDWord(CPHY1_L1_CTRL_0, CPHY1_L1_IDDQEN);
         gm_ClearRegBitsDWord(CPHY1_L2_CTRL_0, CPHY1_L2_IDDQEN);
         gm_ClearRegBitsDWord(CPHY1_L3_CTRL_0, CPHY1_L3_IDDQEN);

         gm_ClearRegBitsDWord(CPHY2_L0_CTRL_0, CPHY2_L0_IDDQEN);
         gm_ClearRegBitsDWord(CPHY2_L1_CTRL_0, CPHY2_L1_IDDQEN);
         gm_ClearRegBitsDWord(CPHY2_L2_CTRL_0, CPHY2_L2_IDDQEN);
         gm_ClearRegBitsDWord(CPHY2_L3_CTRL_0, CPHY2_L3_IDDQEN);
      }
   }
#endif
}

void PowerDeepSleepOutRequest(void)
{
   if (SystemFlags.sys.MissionSleep)
   {
      // 20100324 #1, Move out from audio enable option
      // 20080930 #7, ... Can not disable this bits. (System will hang).
      //gm_WriteRegWord(AUDIO_SOFT_PD,0x7F);

      #if 0 //(FEATURE_HDMI == ENABLE)
      // 20080930 #8, ... Set DVI PHY Power up for detect in DVI (HDMI) port.
      if (((GetChannelConnType(gmvb_MainChannel) == DVI) ||(GetChannelConnType(gmvb_MainChannel) == HDMI))
            || (((GetChannelConnType(gmvb_PipChannel) == DVI) ||(GetChannelConnType(gmvb_PipChannel) == HDMI)) && (PIP_STATUS)))
      {
         // Louis ... To Do --> Set DVI (HDMI) active function
      }
      #endif

      #if 0 // ToDo: implement power on sequence
      PowerOn(); 
      #else
		   #ifndef ST_4K2K_93xx_BOARD
      	gm_DisplayOnOff(On);
                if(PanelID == CMO_V390DK1_LS1)
                        {
                                        gm_Delay100ms(6);
                                        ST_4K2K_I2C();
                        }

		   #endif
      #endif
   }
   else
   {
		#ifndef ST_4K2K_93xx_BOARD
      	gm_DisplayOnOff(On);
                if(PanelID == CMO_V390DK1_LS1)
                        {
                                        gm_Delay100ms(6);
                                        ST_4K2K_I2C();
                        }

		#endif
   }

   PwrState = Pwr_On;
   OSDRequestMsg(gmvb_MainChannel, MTO_ExitPowerDeepSleep, REQUEST_NOW);
 //BK_ON(); //leo add
	pwr_msg("MTO_ExitPowerDeepSleep 1 ",0);
	// test 20130411
	#ifdef ST_4K2K_93xx_BOARD 
	if(GetChipID_4K2K()==CHIPID_4K2K_R)
	{
		if(Into_Pwr_Transition == TRUE)
		{
			Into_Pwr_Transition = FALSE;
			gm_Print("Panel power up from Pwr_Transition", 0);
			PanelPowerOn4k2k();
		}
	}
	#endif

   pwr_msg("PWR_HDR: Exit from PwrDeepSleep",0);
}

BOOL IsSyncOn(void)
{
//   if (IsVideoInput(UserPrefCurrentInputMain) || (IsGraphicInput(UserPrefCurrentInputMain) && (!IsNoSync())) ||
 //        ((UserPrefPipMode) && ((!IsPipNoSync()) || IsVideoInput(UserPrefCurrentInputPip))) ) // || (GetPortProc(UserPrefCurrentInputMain)==DP_PORT)


   if ((IsVideoInput(UserPrefCurrentInputMain)&& (!IsNoSync())) || (IsGraphicInput(UserPrefCurrentInputMain) && (!IsNoSync()))) //||
  //      ((UserPrefPipMode) && (!IsPipNoSync() ))|| (IsVideoInput(UserPrefCurrentInputPip))&& (!IsPipNoSync()) ) // || (GetPortProc(UserPrefCurrentInputMain)==DP_PORT)
{
      		return TRUE;
   	}
   else
      		return FALSE;
}

void PwrTransitionKeyPressed(BOOL flag)
{
   IsPwrTransitionKeyPressed = flag;
}

gmt_PwrStateType GetPowerState(void)
{
   return PwrState;
}

BOOL IsPBP_AthenaL_SyncValid(void)
{
   BOOL ReturnValue = FALSE;

/*ChengLin 20130425: Enable PBP Deep Sleep Check*/
#if (defined(ST_4K2K_93xx_BOARD) && ENABLE_PBP_ATHENA_L_DEEP_SLEEP_CHECK)   
   /*ChengLin 20130423: Avoid Athena R go to sleep mode when Athena L normal display images in PBP mode*/
   if(GetChipID_4K2K()== CHIPID_4K2K_R)
   {
      if(UserPrefCaptureM == CAPTURE_MODE_HALF_HORIZONTAL && IsSyncOn()== FALSE)
      {
         gm_TimerStart(PBP_DEEP_SLEEP_TIMEOUT_10ms_TMR, 50); //Wait 500ms for check Athena Left 
         while(SocIPCSendCmdPacketParser(SOCIPC_DEVICEL_DST_ADDRESS, SICMD_RW_IMAGE_SYNC_STATE_Status, NULL)!= TRUE)
         {
            if(gm_TimerCheck(PBP_DEEP_SLEEP_TIMEOUT_10ms_TMR) == TIMER_TMO)
            {
               gm_TimerStop(PBP_DEEP_SLEEP_TIMEOUT_10ms_TMR);          
               pwr_ext_msg("\n<TimeOut> Athena Left check image sync state fail.\n",0);
               break;
            }
            gm_Delay10ms(1);    
         }

         if(SOCIPC_GetAthena_L_ImageSyncState()==IMAGE_ON_STATE)
         {         
            pwr_ext_msg("PWR_HDR_SubFun: Ignore PwrDeepSleep due to Athena L Image Sync State keep at IMAGE_ON_STATE.",0);
            ReturnValue = TRUE;
         }            
      }
      else
      {
         ReturnValue = FALSE;
      }
   }
#endif   
   return ReturnValue;
}

//******************************************************************************
// USAGE          : Power Handler functions
//                  For the option DEEP_SLEEP_MAIN:
//                  The condition to enter the Deep Sleep FSM is if the main channel is set to the VGA port.
//                  The condition to enter the actual Deep Sleep state is if there is no HS and VS.
//                  For the option DEEP_SLEEP_MAIN_AND_PIP:
//                  The condition to enter the Deep Sleep FSM is if the main channel is set to the VGA port AND
//                  either there is no pip on OR if there is a pip on, then if the pip channel is set to the VGA port.
//                  The condition to enter the actual Deep Sleep state is if there is no HS and VS on both the main and pip.
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        : B_PowerModeSetupReq        (R/W)
//******************************************************************************
void PowerHandler(void)
{

#ifndef DEEP_SLEEP_DISABLED

   if ((PwrState == Pwr_Off))
      return;


   /*ChengLin 20130425: Enable PBP Deep Sleep Check*/
#if (defined(ST_4K2K_93xx_BOARD) && ENABLE_PBP_ATHENA_L_DEEP_SLEEP_CHECK) 
      if((GetChipID_4K2K()== CHIPID_4K2K_L) && (UserPrefCaptureM == CAPTURE_MODE_HALF_HORIZONTAL))
      {
         if(gm_TimerCheck(PBP_SLEEP_CHECK_TIMEOUT_1ms_TMR) == TIMER_TMO)
         {
            gm_TimerStop(PBP_SLEEP_CHECK_TIMEOUT_1ms_TMR);
            Image_output_Request(); //Trigger Athena Right INT1
            pwr_ext_msg("\n<TimeOut> Athena L PBP_SLEEP_CHECK_TIMEOUT_1ms_TMR.\n",0);         
         }
      }
      else if((GetChipID_4K2K()== CHIPID_4K2K_R) && (UserPrefCaptureM == CAPTURE_MODE_HALF_HORIZONTAL))
      {

         if(gm_TimerCheck(PBP_SLEEP_CHECK_TIMEOUT_1ms_TMR) == TIMER_OK)
         {
            if((IsSyncOn() == FALSE) && (IsPBP_AthenaL_SyncValid()))
            {
               OSDRequestCancelMsg(gmvb_MainChannel, MTO_NoSignal);
               KeypadLightControl(KEYPAD_LED_GREEN, LED_NORMAL);               
               AthenaRIgnoreSleep_PBP = TRUE;
               pwr_ext_msg("\nPowerHandler_Counting - Athena R NoSignal and Athena L sync exist!!\n",0);
            }
            else
            {
               AthenaRIgnoreSleep_PBP = FALSE;
            }
         
         }
         else if((gm_TimerCheck(PBP_SLEEP_CHECK_TIMEOUT_1ms_TMR) == TIMER_TMO) ||(PBP_AthenaR_ReCheck_AthenaLStatus))
         {
            gm_TimerStop(PBP_SLEEP_CHECK_TIMEOUT_1ms_TMR);
            if((IsSyncOn() == FALSE) && (IsPBP_AthenaL_SyncValid()))
            {
               OSDRequestCancelMsg(gmvb_MainChannel, MTO_NoSignal);
               KeypadLightControl(KEYPAD_LED_GREEN, LED_NORMAL);               
               AthenaRIgnoreSleep_PBP = TRUE;
               pwr_ext_msg("\nPowerHandler- Athena R NoSignal and Athena L sync exist!!\n",0);
            }
            else
            {
               AthenaRIgnoreSleep_PBP = FALSE;
            }
            
            PBP_AthenaR_ReCheck_AthenaLStatus = FALSE;
            PBP_StartPowerCheck_AthenaR = TRUE;
            pwr_ext_msg("\n<TimeOut> Athena R PBP_SLEEP_CHECK_TIMEOUT_1ms_TMR.\n",0);         
         }

         if(!PBP_StartPowerCheck_AthenaR)
            return;
            }
#endif

/*ChengLin 20130507: Enable 2 Logical Port Deep Sleep Check*/
#if (defined(ST_4K2K_93xx_BOARD) && ENABLE_TWO_LOGICAL_PORTS_SLEEP_CHECK)
   if((GetChipID_4K2K()== CHIPID_4K2K_R) && (UserPrefCaptureM != CAPTURE_MODE_HALF_HORIZONTAL))
   {
      if((gm_TimerCheck(DP_TWO_LOGICAL_PORTS_SLEEP_CHECK_TIMEOUT_1ms_TMR) == TIMER_TMO))
      {
         gm_TimerStop(DP_TWO_LOGICAL_PORTS_SLEEP_CHECK_TIMEOUT_1ms_TMR);
         if((IsSyncOn() == FALSE) && (IsPBP_AthenaL_SyncValid()))
         {
            OSDRequestCancelMsg(gmvb_MainChannel, MTO_NoSignal);
            KeypadLightControl(KEYPAD_LED_GREEN, LED_NORMAL);               
            pwr_ext_msg("\nPowerHandler- MST Athena R NoSignal and Athena L sync exist!!\n",0);
         }
         DP_TWO_LogicalPorts_StartPowerCheck_AthenaR = TRUE;
         pwr_ext_msg("\n<TimeOut> Athena R DP_TWO_LOGICAL_PORTS_SLEEP_CHECK_TIMEOUT_1ms_TMR.\n",0);         
      }
   
      if(!DP_TWO_LogicalPorts_StartPowerCheck_AthenaR)
         return;
   }
#endif

   PIP_STATUS = UserPrefPipMode;

#ifdef DEEP_SLEEP_MAIN_AND_PIP
   if (PwrState == Pwr_On)
   {
      if (!IsGotoSleep())
         return;
   }
#endif

   switch (PwrState)
   {
      case Pwr_On:
         if (IsSyncOn())
         {
            PwrState = Pwr_On;		
  	//	BK_ON(); //leo add
  			pwr_msg("Pwr_On ",0);

            return;
         }

      /*ChengLin 20130507: Enable 2 Logical Port Deep Sleep Check*/
      #ifdef ST_4K2K_93xx_BOARD 
      #if (defined(DP_HUB_2_LOG_PORTS) && ENABLE_TWO_LOGICAL_PORTS_SLEEP_CHECK)
      if((GetChipID_4K2K()== CHIPID_4K2K_R) && (UserPrefCaptureM != CAPTURE_MODE_HALF_HORIZONTAL) && (IsSyncOn() == FALSE) \
         && (gm_GetDpDigitalID(gB_CurrentInputConnector[gmvb_MainChannel])==DID_DPRX0)) //Add by ChengLin 20130604
      {
      #if 1
         BYTE b_StableAssumed=FALSE;
         if(gm_TimerCheck(TWO_LOGICAL_PORTS_DEEP_SLEEP_TIMEOUT_10ms_TMR) != TIMER_OK) // Not running
         {
            SocIPCSendCmdPacketParser(SOCIPC_DEVICEL_DST_ADDRESS, SICMD_RO_AthenaL_SyncStatus, NULL);
            gm_TimerStart(TWO_LOGICAL_PORTS_DEEP_SLEEP_TIMEOUT_10ms_TMR, 100);
         }
         else
         {
            b_StableAssumed=TRUE;
         }
        
         if((gAthena_LEFT_FE_Slave_Parameters.AthenaLSyncStatus==TRUE)||(b_StableAssumed==TRUE))
         {  
            OSDRequestCancelMsg(gmvb_MainChannel, MTO_NoSignal);
            KeypadLightControl(KEYPAD_LED_GREEN, LED_NORMAL);
            pwr_ext_msg("\nPWR_HDR: Ignore PwrDeepSleep due to Athena L Image DP Sync Valid.\n",0);
            return;
         }

      #else
         
            gm_TimerStart(TWO_LOGICAL_PORTS_DEEP_SLEEP_TIMEOUT_10ms_TMR, 50); //Wait 500ms for check Athena Left 
            while(SocIPCSendCmdPacketParser(SOCIPC_DEVICEL_DST_ADDRESS, SICMD_RO_AthenaL_SyncStatus, NULL)!= TRUE)
            {
               if(gm_TimerCheck(TWO_LOGICAL_PORTS_DEEP_SLEEP_TIMEOUT_10ms_TMR) == TIMER_TMO)
               {
                  gm_TimerStop(TWO_LOGICAL_PORTS_DEEP_SLEEP_TIMEOUT_10ms_TMR);          
                  pwr_ext_msg("\n<TimeOut> Athena Left check stable state fail.\n",0);
                  break;
               }
               gm_Delay10ms(1);    
            }

            if(gAthena_LEFT_FE_Slave_Parameters.AthenaLSyncStatus==TRUE)
            {  
               OSDRequestCancelMsg(gmvb_MainChannel, MTO_NoSignal);
               KeypadLightControl(KEYPAD_LED_GREEN, LED_NORMAL);
               pwr_ext_msg("\nPWR_HDR: Ignore PwrDeepSleep due to Athena L Image DP Sync Valid.\n",0);
               return;
            } 
            
         }
      #endif
      }
      #endif
      #endif

      /*ChengLin 20130425: Enable PBP Deep Sleep Check*/
      #if (defined(ST_4K2K_93xx_BOARD) && ENABLE_PBP_ATHENA_L_DEEP_SLEEP_CHECK)          
         if((GetChipID_4K2K()== CHIPID_4K2K_R) && (UserPrefCaptureM == CAPTURE_MODE_HALF_HORIZONTAL))
         {
            if(((IsSyncOn() == FALSE) && (AthenaRIgnoreSleep_PBP)))
            {
               OSDRequestCancelMsg(gmvb_MainChannel, MTO_NoSignal);
               KeypadLightControl(KEYPAD_LED_GREEN, LED_NORMAL);
               return;
            }
         }
      #endif
         gm_TimerStart(DEEP_SLEEP_100ms_TMR, DEEPSLEEP_TIME);
         PwrState = Pwr_Sleep;       
         pwr_msg("PWR_HDR: PwrOn to PwrSleep",0);
         break;

      case Pwr_Sleep:
      #ifdef ST_4K2K_93xx_BOARD
      /*ChengLin 20130718: Before "DEEP_SLEEP_100ms_TMR" timeout, system still need to check sync status, once sync is ON then stop "DEEP_SLEEP_100ms_TMR" counting.*/
         if (gm_TimerCheck(DEEP_SLEEP_100ms_TMR) == TIMER_OK)
         {
            if (IsSyncOn())
            {
               pwr_msg("PWR_HDR: Stop DEEP_SLEEP_100ms_TMR - SyncOn.",0);
               gm_TimerStop(DEEP_SLEEP_100ms_TMR);            
               PwrState = Pwr_On;
   //		BK_ON(); //leo add
   		pwr_msg("Pwr_On 1 ",0);
               if (SystemFlags.sys.IgnoreSleep == TRUE)
                  OSDRequestMsg(gmvb_MainChannel, MTO_ExitPowerDeepSleep, REQUEST_NOW);
               return;
            }

            #if ENABLE_PBP_ATHENA_L_DEEP_SLEEP_CHECK   
            if((GetChipID_4K2K()== CHIPID_4K2K_R) && (UserPrefCaptureM == CAPTURE_MODE_HALF_HORIZONTAL))
            {
               if((IsSyncOn() == FALSE) && (AthenaRIgnoreSleep_PBP))
               {
                  pwr_msg("PWR_HDR: Stop DEEP_SLEEP_100ms_TMR - SyncOn and Ignore PBP Sleep.",0);
                  gm_TimerStop(DEEP_SLEEP_100ms_TMR);                
                  PwrState = Pwr_On;         
   	//		BK_ON(); //leo add
   			pwr_msg("Pwr_On 2 ",0);
                  if (SystemFlags.sys.IgnoreSleep == TRUE)
                     OSDRequestMsg(gmvb_MainChannel, MTO_ExitPowerDeepSleep, REQUEST_NOW);
                  return;
               }
            }
            #endif
         }
      #endif
      
         if (gm_TimerCheck(DEEP_SLEEP_100ms_TMR) == TIMER_TMO) // Check timer for Pwr_DeepSleep
         {
      #if (defined(ST_4K2K_93xx_BOARD) && ENABLE_PBP_ATHENA_L_DEEP_SLEEP_CHECK)          
            gm_TimerStop(DEEP_SLEEP_100ms_TMR);
      #endif
            #ifdef VIDEO_CAPTURE
            if (Photo_Index >= 1)
            {
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
               CDATA_Flag = 4;
            }
            else
            #endif
            {
               if (IsSyncOn())
               {
                  PwrState = Pwr_On;
		//   BK_ON(); //leo add
		   pwr_ext_msg("Pwr_On 3 ",0);
                  if (SystemFlags.sys.IgnoreSleep == TRUE)
                     OSDRequestMsg(gmvb_MainChannel, MTO_ExitPowerDeepSleep, REQUEST_NOW);
                  return;
               }

               /*ChengLin 20130425: Enable PBP Deep Sleep Check*/
            #if(defined(ST_4K2K_93xx_BOARD) && ENABLE_PBP_ATHENA_L_DEEP_SLEEP_CHECK)   
               if((GetChipID_4K2K()== CHIPID_4K2K_R) && (UserPrefCaptureM == CAPTURE_MODE_HALF_HORIZONTAL))
               {
                  if((IsSyncOn() == FALSE) && (AthenaRIgnoreSleep_PBP))
                  {
                     PwrState = Pwr_On;      
			  // BK_ON(); //leo add
			   pwr_ext_msg("Pwr_On 4",0);
                     if (SystemFlags.sys.IgnoreSleep == TRUE)
                        OSDRequestMsg(gmvb_MainChannel, MTO_ExitPowerDeepSleep, REQUEST_NOW);
                     return;
                  }
               }
            #endif
            
               if (SystemFlags.sys.IgnoreSleep == FALSE)
               {
            /*ChengLin 20130621: Patch for Nvidia Tiled Display Driver can't wake up issue once PC set to sleep mode*/               
            #if 0//defined(ST_4K2K_93xx_BOARD) && defined(DP_HUB_2_LOG_PORTS)  
                  if((GetChipID_4K2K()== CHIPID_4K2K_L) && (gm_GetDpDigitalID(gB_CurrentInputConnector[gmvb_MainChannel])==DID_DPRX0) && (UserPrefCaptureM != CAPTURE_MODE_HALF_HORIZONTAL))
                  {
                     {
                        pwr_msg("PWR_HDR: Athena L Clear Two Logical Port Flags.",0);
                        ResetDpRx0MstInfoUpdatFlag();
                     }
                  }
            #endif               
                  OSDRequestMsg(gmvb_MainChannel, MTO_EnterPowerDeepSleep, REQUEST_NOW);
               }
            }
         }

         break;

      case Pwr_DeepSleep:
         if (IsSyncOn())
         {
            PowerDeepSleepOutRequest();
         }
         break;

      case Pwr_Transition:
if((UserPrefCurrentInputMain == ALL_BOARDS_CVBS1)&& (UserPrefCurrentInputMain == ALL_BOARDS_SVIDEO1))
{
         PwrTransitionCount = DEEP_SLEEP_CONDITION_TIME;
         gm_TimerStart(PWR_TRANSITION_100ms_TMR, PwrTransitionCount);
		//BK_OFF();
}
         if (gm_TimerCheck(PWR_TRANSITION_100ms_TMR) == TIMER_STOP)
         {
            PwrTransitionCount = PWR_TRANSITION_COUNT;
            gm_TimerStart(PWR_TRANSITION_100ms_TMR, PwrTransitionCount);
         }
         else if (gm_TimerCheck(PWR_TRANSITION_100ms_TMR) == TIMER_TMO)
         {  
            #if 0//def ST_4K2K_93xx_BOARD 
            #if ENABLE_TWO_LOGICAL_PORTS_SLEEP_CHECK            
               /*ChengLin 20130509: If OSD into Deep Sleep Mode, for MST 1 stream on Athena L must to ignore Deep Sleep.*/
               if(GetChipID_4K2K()== CHIPID_4K2K_R)
               {
                  if((UserPrefCaptureM != CAPTURE_MODE_HALF_HORIZONTAL) &&(gm_GetDpDigitalID(gB_CurrentInputConnector[gmvb_MainChannel])==DID_DPRX0) && (gm_ReadRegWord(DP12RX_RL_CONFIG) & DPRX_TYPE))
                  {
                     SocIPCSendCmdPacketParser(SOCIPC_DEVICEL_DST_ADDRESS, SICMD_RO_AthenaL_SyncStatus, NULL);
                     if((IsSyncOn() == FALSE) && (gAthena_LEFT_FE_Slave_Parameters.AthenaLSyncStatus==TRUE))                  
                     {
                        PwrState = Pwr_On;
 			  BK_ON(); //leo add
                        KeypadLightControl(KEYPAD_LED_GREEN, LED_NORMAL);
                        OSDRequestMsg(gmvb_MainChannel, MTO_ExitPowerDeepSleep, REQUEST_NOW);
                        pwr_ext_msg("PWR_HDR: DP MST and Athena L valid- Re-Power ON_2!!",0);
                        return;
                     }
                  }
               }
            #endif
            #endif

            PowerDeepSleepInRequest();
         }
         else
         {
            WORD Count;
            Count = gm_TimerRead(PWR_TRANSITION_100ms_TMR);
            if (PwrTransitionCount != Count)
            {
               PwrTransitionCount = Count;
               if (IsSyncOn() || IsPwrTransitionKeyPressed)
               {
                  /*ChengLin 20130508: If OSD into Deep Sleep Mode, for MST 1 stream on Athena L must to ignore Deep Sleep.*/
                  #ifdef ST_4K2K_93xx_BOARD
                  #if ENABLE_TWO_LOGICAL_PORTS_SLEEP_CHECK
                  if(GetChipID_4K2K()== CHIPID_4K2K_R)
                  {
                     if((UserPrefCaptureM != CAPTURE_MODE_HALF_HORIZONTAL) &&(gm_GetDpDigitalID(gB_CurrentInputConnector[gmvb_MainChannel])==DID_DPRX0) && (gm_ReadRegWord(DP12RX_RL_CONFIG) & DPRX_TYPE))
                     {
                        SocIPCSendCmdPacketParser(SOCIPC_DEVICEL_DST_ADDRESS, SICMD_RO_AthenaL_SyncStatus, NULL);
                        if((IsSyncOn() == FALSE) && (gAthena_LEFT_FE_Slave_Parameters.AthenaLSyncStatus==TRUE))                  
                        {
                           IsPwrTransitionKeyPressed = FALSE;
                           pwr_ext_msg("PWR_HDR: DP MST and Athena L valid- Re-Power ON!!",0);
                        }
                     }
                  }
                  #endif
                  #endif
                  
						#ifndef ST_4K2K_93xx_BOARD
      					gm_DisplayOnOff(On);
						
                if(PanelID == CMO_V390DK1_LS1)
                        {
                                        gm_Delay100ms(6);
                                        ST_4K2K_I2C();
                        }

		#endif

		
                  if (SystemFlags.sys.MissionSleep)
                  {
                     PowerDeepSleepOutRequest();
                  }
                  else
                  {
                     PwrState = Pwr_On;
  		//	BK_ON(); //leo add
  			pwr_ext_msg("Pwr_On 5 ",0);
                     KeypadLightControl(KEYPAD_LED_GREEN, LED_NORMAL);
                     OSDRequestMsg(gmvb_MainChannel, MTO_ExitPowerDeepSleep, REQUEST_NOW);
                  }
               }             
            }
         }
         break;

      case Pwr_Off: // Run in LPM, Mission turn off.
         break;
   }
#endif
}

WORD LpmWakeupPortInfo(gmt_UserDefConnNames B_InputConnector)
{
   WORD result;

 #if defined(ST_4K2K_93xx_BOARD)
 /*ChengLin 20130429: Correct Port Info for 4k2k board*/ 
   switch (B_InputConnector)
   {
      case ALL_BOARDS_DP1:
      case ALL_BOARDS_DP3: //For Mini DP wake-up from LPM Mode 
      case ALL_BOARDS_HDMI://For Athena FE      
         result = (B_InputConnector&0xFF) | (0<<8); // Combo-Phy0
         break;
      case ALL_BOARDS_DP2:     
         result = (B_InputConnector&0xFF) | (2<<8); // Combo-Phy2
         break;
      default: // ALL_BOARDS_VGA1 
         result = (B_InputConnector&0xFF) | (0xFFU<<8); // Without Combo-Phy
         break;
   }
 #elif defined(RD1_93xx_BOARD)
   switch (B_InputConnector)
   {
      case ALL_BOARDS_DP1:
         result = (B_InputConnector&0xFF) | (0<<8); // Combo-Phy0
         break;
      case ALL_BOARDS_DVI1:      
         result = (B_InputConnector&0xFF) | (1<<8); // Combo-Phy1
         break;
      case ALL_BOARDS_HDMI:        
         result = (B_InputConnector&0xFF) | (2<<8); // Combo-Phy2
         break;
      default: // ALL_BOARDS_VGA1 
         result = (B_InputConnector&0xFF) | (0xFFU<<8); // Without Combo-Phy
         break;
   }
#elif defined(EV3_93xx_BOARD)
   switch (B_InputConnector)
   {
      case ALL_BOARDS_DP1:
         result = (B_InputConnector&0xFF) | (0<<8); // Combo-Phy0
         break;
      case ALL_BOARDS_DVI1:
         result = (B_InputConnector&0xFF) | (1<<8); // Combo-Phy1
         break;
      default: // ALL_BOARDS_VGA1
         result = (B_InputConnector&0xFF) | (0xFFU<<8); // Without Combo-Phy
         break;
   }
#elif defined(EV1_93xx_BOARD)
   switch (B_InputConnector)
   {
      case ALL_BOARDS_DP1:
         result = (B_InputConnector&0xFF) | (0<<8); // Combo-Phy0
         break;
      case ALL_BOARDS_HDMI:
         result = (B_InputConnector&0xFF) | (1<<8); // Combo-Phy1
         break;
      case ALL_BOARDS_DP2:
         result = (B_InputConnector&0xFF) | (2<<8); // Combo-Phy2
         break;
      default: // ALL_BOARDS_VGA1
         result = (B_InputConnector&0xFF) | (0xFFU<<8); // Without Combo-Phy
         break;
   }
#elif defined(EV2_93xx_BOARD)
   switch (B_InputConnector)
   {
      case ALL_BOARDS_HDMI:
         result = (B_InputConnector&0xFF) | (0<<8); // Combo-Phy0
         break;
      case ALL_BOARDS_HDMI2:
         result = (B_InputConnector&0xFF) | (1<<8); // Combo-Phy1
         break;
      case ALL_BOARDS_HDMI3:
         result = (B_InputConnector&0xFF) | (2<<8); // Combo-Phy2
         break;
      default: // ALL_BOARDS_VGA1
         result = (B_InputConnector&0xFF) | (0xFFU<<8); // Without Combo-Phy
         break;
   }
#else
#pragma message "Unknow board type!"
#endif
   return result;
}

//******************************************************************************
//
// FUNCTION       : BYTE LPM_ReceiveStatus(void)
//
// USAGE          : LPM status check function
//
// INPUT          : None
//
// OUTPUT         : LPM power status
//                        0: PWR_SW_OFF
//                        1: PWR_SW_ON (LPM_WAKEUP_KEYPAD/LPM_WAKEUP_AC_ON)
//                        2: PWR_SW_SIGDET_ON (LPM_WAKEUP_CPHY0/LPM_WAKEUP_CPHY1/LPM_WAKEUP_CPHY2/LPM_WAKEUP_ASDET)
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
#if (FEATURE_LOW_POWER_MONITOR == ENABLE)
BYTE LPM_ReceiveStatus(void)
{
   WORD command;
   BYTE status[3];
   BYTE WakeupPort;

   command = 0;   // 0: check status

   if (gm_MessageSend(gmd_MESSAGE_CHECK_STATUS, &command, sizeof(command)) == FALSE)
   {
      pwr_ext_msg("PWR_HDR: LPM check status fail",0);
   }
   else
   {
      gm_TimerStart(LPM_CHECK_STATUS_100ms_TMR, 1);
      while (1)
      {
         if (gm_MessageReceived(gmd_MESSAGE_CHECK_STATUS, status, sizeof(status)))
         {
            //gm_Printf("LPM status = 0x%x, 0x%x, 0x%x",status[0],status[1],status[2]);
            break;
         }
         if (gm_TimerCheck(LPM_CHECK_STATUS_100ms_TMR) == TIMER_TMO)
         {
            pwr_ext_msg("PWR_HDR: LPM check status timeout",0);
            break;
         }
      }
   }

   switch (status[0])
   {
      case PWR_SW_OFF:
         UserPrefPowerState = POWER_STATE_OFF;
         pwr_msg("PWR_HDR: LPM DC Off",0);
         break;
      case PWR_SW_ON:
         UserPrefPowerState = POWER_STATE_ON;
         switch (status[1])
         {
            case LPM_WAKEUP_KEYPAD:
               LPM_CalibrateRingOSC();
               LPM_SendCalibratedTrim();
               pwr_msg("PWR_HDR: LPM DC On",0);

					#ifdef ST_4K2K_93xx_BOARD 
					if(GetChipID_4K2K()== CHIPID_4K2K_R)
					{
					   LPM_Set_DCFlag(FALSE);
                  AllowKeypadAction = TRUE; 
					}
					#endif
               break;
            case LPM_WAKEUP_AC_ON:
               pwr_msg("PWR_HDR: LPM AC On",0);
               #if (FEATURE_HDMI==ENABLE) && (ENABLE_MIN_PORT_OFF_LIMITATION==1)
               HdmiDvi_SendAC_ON_Notice();
               #endif

               #ifdef ST_4K2K_93xx_BOARD
                  #if (ENABLE_SOC_IPC_OSD_SYNC)
                        if(GetChipID_4K2K()==CHIPID_4K2K_R)
                        {
                           AllowKeypadAction = FALSE; 
                           if (SocIPCMasterSlaveOSDSyncSettings())
                              pwr_ext_msg("\n<Soc IPC Message> OSD Sync Settings Successful!!\n",0);
                           else
                              pwr_ext_msg("\n<Soc IPC Message> OSD Sync Settings Failure!!\n",0);
                        }
                  #endif                  
               #endif

               break;
            default:
               break;
         }
         break;
      case PWR_SW_SIGDET_ON:
         UserPrefPowerState = POWER_STATE_ON;
         pwr_msg("PWR_HDR: LPM Signal On",0);
			#ifdef ST_4K2K_93xx_BOARD 
				if(GetChipID_4K2K()== CHIPID_4K2K_R)
				{
               AllowKeypadAction = TRUE; //Add by ChengLin 20130315                  
				}
			#endif        
			
         #if (FEATURE_DISPLAY_PORT==ENABLE) && (HPD_ON_PHYON_NOTIFICATION!=0) && (HPD_ON_PHYON_NOTIFICATION_ONLY_WAKEUP!=0)
               DpRx_SendLpmWakeUp_Notice();   
         #endif
         
         #if defined(RD1_93xx_BOARD) || defined(ST_4K2K_93xx_BOARD)
         switch (status[1])
         {
            case LPM_WAKEUP_CPHY0: // Combo-Phy0
               WakeupPort = ALL_BOARDS_DP1;
               break;
            case LPM_WAKEUP_CPHY1: // Combo-Phy1
               WakeupPort = ALL_BOARDS_DVI1;
               break;
            case LPM_WAKEUP_CPHY2: // Combo-Phy2
               WakeupPort = ALL_BOARDS_HDMI;
               break;
            default: // VGA,Comp
               WakeupPort = ALL_BOARDS_VGA1;
               break;
         }

         #elif defined(EV3_93xx_BOARD)
         switch (status[1])
         {
            case LPM_WAKEUP_CPHY0: // Combo-Phy0
               WakeupPort = ALL_BOARDS_DP1;
               break;
            case LPM_WAKEUP_CPHY1: // Combo-Phy1
               WakeupPort = ALL_BOARDS_DVI1;
               break;
            default: // VGA,Comp
               WakeupPort = ALL_BOARDS_VGA1;
               break;
         }

         #elif defined(EV1_93xx_BOARD)
         switch (status[1])
         {
            case LPM_WAKEUP_CPHY0: // Combo-Phy0
               WakeupPort = ALL_BOARDS_DP1;
               break;
            case LPM_WAKEUP_CPHY1: // Combo-Phy1
               UserPrefCurrentInputMain = ALL_BOARDS_HDMI;
               break;
            case LPM_WAKEUP_CPHY2: // Combo-Phy2
               WakeupPort = ALL_BOARDS_DP2;
               break;
            default: // VGA,Comp
               WakeupPort = ALL_BOARDS_VGA1;
               break;
         }

         #elif defined(EV2_93xx_BOARD)
         switch (status[1])
         {
            case LPM_WAKEUP_CPHY0: // Combo-Phy0
               WakeupPort = ALL_BOARDS_HDMI;
               break;
            case LPM_WAKEUP_CPHY1: // Combo-Phy1
               WakeupPort = ALL_BOARDS_HDMI2;
               break;
            case LPM_WAKEUP_CPHY2: // Combo-Phy2
               WakeupPort = ALL_BOARDS_HDMI3;
               break;
            default: // VGA,Comp
               WakeupPort = ALL_BOARDS_VGA1;
               break;
         }

         #else
            #pragma message "Unknow board type!"
         #endif

         if (UserPrefPipMode != NO_PIP)
         {
            if (WakeupPort != UserPrefCurrentInputPip)
               UserPrefCurrentInputMain = WakeupPort;
         }

      #ifdef AUTO_SCAN_INPUT
         else
         {
            if (WakeupPort == UserPrefCurrentInputPip)
            {
               UserPrefCurrentInputPip = UserPrefCurrentInputMain;
            }
            UserPrefCurrentInputMain = WakeupPort;
         }
      #endif
         break;
   }

   return(status[0]);
}
#endif

#ifdef INACTIVE_DEVICE_POWER_SAVING
//******************************************************************************
//
// FUNCTION       : void PowerUpDevice(BYTE B_Port)
//
// USAGE          : Power up device
//
// INPUT          : Input port
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
void PowerUpDevice(BYTE B_Port)
{
   switch (B_Port)
   {
#if (FEATURE_AIP == ENABLE)
      case ALL_BOARDS_VGA1:
      case ALL_BOARDS_COMP1:
         //reduce ASE slice level  from default 150mv to avoid vrong sync detection with some DVD sources.
         gm_AseSetSyncSliceLevel(gmd_ADC_0, ASE_SS_ST, ASE_SLICE_LEVEL_125mV);
         break;
#endif
      default:
         break;
   }
}

//******************************************************************************
//
// FUNCTION       : void PowerDownDevice(BYTE B_Port)
//
// USAGE          : Power down device
//
// INPUT          : Input port
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
void PowerDownDevice(BYTE B_Port)
{
   switch (B_Port)
   {
#if (FEATURE_AIP == ENABLE)
      case ALL_BOARDS_VGA1:
      case ALL_BOARDS_COMP1:
         gm_WriteRegByte(AFE_DAC_CTRL1, 0);
         gm_WriteRegWord(ADC_CONTROL0, (PD_ADC_A | PD_ADC_B | PD_ADC_C));
         gm_WriteRegWord(ASE_CONTROL, 0);
         gm_WriteRegWord(ASE_CONTROL_HS, 0);
         gm_ClearAndSetBitsWord(CLOCK_CONFIG2, (SCLK_SEL | HD_G_CLK_SEL), (SCLK_CLK_SEL_GND | HD_CLK_SEL_GND<<HD_G_CLK_SEL_SHIFT));
         break;
#endif
      case ALL_BOARDS_DIP1:
         gm_DipPowerSave();
         break;

      default:
         break;
   }
}
#endif


//******************************************************************************

