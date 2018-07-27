/*
	$Workfile:   ModeHandler.h  $
	$Revision: 1.6 $
	$Date: 2013/04/02 02:13:49 $
*/
#ifndef __MODEHANDLER_H__
#define __MODEHANDLER_H__

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
// MODULE:      ModeHandler.h
//
// USAGE:       This module contains Mode Handler defintiion
//
//******************************************************************************

//******************************************************************************
//  D E F I N E S
//******************************************************************************

#define NO_SIGNAL_DELAY					4500	// Time (ms) for No Signal to show up


//******************************************************************************
//  G L O B A L    D E F I N I T I O N S
//******************************************************************************
//
// OSD system Messages to WB
// NOTE : MUST be align with MainSystemOsdEvents and PipSystemOsdEvents
//		  in osd_input.c
//
typedef enum
{
   MTO_Empty,							// No message in buffer
   //
   MTO_SystemStartUp,					// System Power up first time
   MTO_StartPowerDown,					// System about to enter power down
   MTO_PowerDownNow,					// System power now
   MTO_PowerUp,						// System Power up from power down
   MTO_DisplayOn,						// Turn on display
   MTO_DisplayOff,						// Turn off display only (e.g. Audio is still on)
   //
   MTO_SplashScreen,					// Start Splash Screen
   MTO_StartPowerSaving,				// System about to enter power saving
   MTO_PowerSavingNow,					// System in power saving mode now
   MTO_ExitPowerSaving,				// System return to normal operation
   //
   MTO_GoingToSleep,					// Sleep alarm is going off
   MTO_WakeUp,						// Wake alarm is going off
   //
   MTO_SyncPresent,					// Sync present but not stable timing
   MTO_ValidMode,						// Valid detected from mode table
   MTO_InvalidMode,					// Invalid input mode
   MTO_OutOfRange,						// Out of HW support mode
   MTO_PseudoMode,						// Estimated mode
   MTO_Overlapped,						// Input timing overlapped
   MTO_NoSync,							// Cable connected but not sync signal
   MTO_NoCable,						// Cable disconnected
   MTO_ModeUnstable,					// Input mode unstable or mode change
   MTO_NoSignal,						// Video mode selected but no proper signal detected
   //
   MTO_AutoAdjustStart,				// Auto adjust start
   MTO_AutoAdjustCompleted,			// Auto adjust completed
   MTO_AutoAdjustFailed,				// Auto adjust failed
   //
   MTO_TV_AutoScanStart,				// TV tuner auto channel scan start
   MTO_TV_ChannelChanged,				// TV tuner channel number changed
   MTO_TV_AutoScanCompleted,			// TV tuner auto channel scan end
#ifdef LCDTV_PAL
   MTO_TV_FrequencyChanged,			// TV auto scan for PAL
#endif
   MTO_TV_ProgramBlocked,				// Program Blocked - for VCHIP (based on XDS data)
   MTO_TV_ProgramUnBlocked,			// Program UnBlocked - for VCHIP (based on XDS data)
   MTO_TV_ChannelBlocked,				// Channel always blocked - independent of XDS data
   MTO_TV_ChannelUnBlocked,			// Unblock Blocked Channel - independent of XDS data

   MTO_AudioMute,						// Audio Mute
   MTO_AudioOn,						// Audio on back from Mute
   MTO_AudioSourceChanged,				// Audio source changed (MAIN, SAP)
   MTO_AudioTypeChanged,				// Audio type changed (Mono, Stereo)

   MTO_MinuteUpdate,					//Minute Update from Real time clock
   //
   MTO_CloseCaptionOn,				//Turn on CC
   MTO_CloseCaptionOff,				//Turn off CC
   MTO_UpdateMultiPipChannel,
   MTO_MultiPipOff,						//Turn off MultiPip
   MTO_UnFreeze,						//Turn off Freeze on main channel
   MTO_DisplayBlockedChannelNum,	 	// parental blocked channel number display
   MTO_EraseBlockedChannelNum,	 	// parental blocked channel number display
   MTO_EnterPowerDeepSleep,			// enter Deep Sleep state message
   MTO_ExitPowerDeepSleep	,			// exit Deep Sleep state message
   MTO_TtxStatusDisplayOn,
   MTO_TV_ManualScanCompleted,
   MTO_TV_ManualScanStart ,
   MTO_TeletextOff,
   MTO_PipIsNotAllowed,
   MTO_AspectRatioUpdate,				// update Aspect Ratio
   MTO_ADCCalibrationCompleted,		// update ADC calibration parameters
   MTO_RemoveOSD,                  // Remove OSD from screen
   MTO_ChangeInputToHDMI,			// Change Input to HDMI
   MTO_ShowCECString,              // Show CEC String
   MTO_CEC_NewDeviceDetected,
   MTO_CEC_DevDataBaseChange,
   MTO_CEC_Command_OK,
   MTO_CEC_Command_Failed,
   MTO_CEC_Command_Pwr_Off,
   MTO_CEC_Command_Pwr_On,
   MTO_HideCECString,
   MTO_ChangeInputToHDMI2,			// Change Input to HDMI
   MTO_ChangeInputToHDMI3,			// Change Input to HDMI
   MTO_CEC_Text4UserReady,
   MTO_DrawVolumeSlider,
   MTO_UpdateMuteIcon,
   MTO_SystemAudioStatusChanged,
   MTO_IspStatusChanged,			// SMART_ISP
   MTO_IspAutoDetected,			// SMART_ISP_AUTO_DETECTION
	MTO_OSDPort_4kx2K,			// 4Kx2K OSD Port display 20120418
	MTO_Test_Pattern1,
	MTO_Test_Pattern2,
	MTO_Test_Pattern3,
   MTO_MAX                             			// Max number MTO message

} MessageToOsdType ;


//******************************************************************************
//  F U N C T I O N    P R O T O T Y P E S
//******************************************************************************

void ModeHandlerInit	(void);
void ModeHandler		(void);

//******************************************************************************
//  G L O B A L S
//******************************************************************************

extern	BYTE B_ModeHandlerInit;

#endif

//*********************************  END  **************************************

