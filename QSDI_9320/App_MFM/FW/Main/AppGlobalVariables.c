/*
	$Workfile:   AppGlobalVariables.c  $
	$Revision: 1.12 $
	$Date: 2013/06/24 07:05:08 $
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
// MODULE:      AppGlobalVariables.c
//
// USAGE:       This module contains all the Global variables used across all
//				the modules in the Application.
//
//******************************************************************************

//******************************************************************************
//  I N C L U D E    F I L E S
//******************************************************************************

#include "system\all.h"
//
// Application Init Status
//
BOOL 	B_SplashComplete = FALSE;	// Splash screen status

//
// Mode Detect
//
BYTE 	gB_CurrentInputConnector[NUM_CHANNELS] = {NOT_USED, NOT_USED};

//
// Mode Setup
//
BYTE 	B_SetupInProgress;			// Mode Setup In Progress
BOOL	B_FreeRun[NUM_CHANNELS];	// System in Freerun
BYTE	B_DisplayLockMethod;	  	// display Lock Method
// (Closed Loop or DFL)
BYTE 	B_AutoAdjustStatus[NUM_CHANNELS] = {AA_IDLE, AA_IDLE};	// Auto-Adjust status
BYTE 	B_AutoAdjustStarted[NUM_CHANNELS] = {FALSE,FALSE};

//
// Overscan
//
BYTE 	W_OverscanEnabled[NUM_CHANNELS]; 			// 0 = Underscan,    
// 1 = Overscan
//
// Aspect Ratios
//
gmt_ASPECT_RATIO 		B_OutputAspect;					// Panel Aspect Ratio (Determined in Init())
BYTE				 	B_AspectMethod[NUM_CHANNELS];	// Aspect Ratio Applied to Channel

//
// Tuner Application (NTSC) - Parental Control
//
#if 0
gmt_CONTENT_ADVISORY 	TVMainProgramRating;		   	// Program rating for main input (used by OSD)
gmt_CONTENT_ADVISORY 	TVPipProgramRating;			   	// Program rating for pip input (used by OSD)
#endif
BOOL					B_MainProgramBlocked;			// Flag to indicate if program blocked (in main)
BOOL					B_PipProgramBlocked;			// Flag to indicate if program blocked (in pip)

//
// Dynamic Scaling
//
gmt_PHY_CH				B_FocusBorder;				   	// Channel with Focus Border

//
// The following parameter indicates which VBI service is support on the system.
// The parameter will be used to configure the VBI driver during initialization
// time.
//

gmt_VBI_SERVICE VBI_ServiceType;
gmt_VBI_SERVICE VBI_ServicePreferred;

// Signal Status (Present, Nosync, NoCable, None)
BYTE	B_SignalStatus = STATUS_NONE;

// Height condition for SOG signal
BYTE 	B_Overlap_Height = OVERLAP_GR350;

#ifdef USE_SMT
BYTE gmvb_SerialStatus;
#endif

WORD HighBrightness = 0x3FF, LowBrightness = 0x100, AVGBrightness = 0;
BYTE DCR_State = 0, AVGCount =0, TargetBright = 0xFF;

TUartData uartData;

BYTE Input_RGB_Range = 0;
BYTE Input_YUV_Range = 0;

BYTE Input_Color_Domain = 0;
BYTE Output_Color_type = 0;

#if IGNORE_SLEEP
SystemFlags_t SystemFlags = {0x0008};
#else
SystemFlags_t SystemFlags = {0x0000};
#endif

#ifdef NO_SIGNAL_OSD_FLOATING
DWORD NoSignalFloatOsdTimer = 0;
#endif

/*********************************  END  ************************************/
