/*
            $Workfile:   AppGlobalVariables.h  $
            $Revision: 1.18 $
            $Date    :   $
*/

#ifndef __APPGLOBALVARIABLES_H__
#define __APPGLOBALVARIABLES_H__
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
// MODULE:      AppGlobalVariables.h
//
// USAGE:       This module contains all the Global variables used across all
//				the modules in the Application. This is the header file
//				containing the externs declarations
//
//******************************************************************************

//******************************************************************************
//  G L O B A L    D E F I N E S
//******************************************************************************

//
// Mode Setup Blender Control
//
#define		BLENDER_NO_ACTION		0x00
#define		BLENDER_WAIT_MAIN		0x01
#define		BLENDER_READY_MAIN		0x02
#define		BLENDER_ON_MAIN			0x04
#define		BLENDER_MASK_MAIN		(BLENDER_WAIT_MAIN|BLENDER_READY_MAIN|BLENDER_ON_MAIN)
#define		BLENDER_WAIT_PIP		0x10
#define		BLENDER_READY_PIP		0x20
#define		BLENDER_ON_PIP			0x40
#define		BLENDER_MASK_PIP		(BLENDER_WAIT_PIP|BLENDER_READY_PIP|BLENDER_ON_PIP)
#define		BLENDER_BOTH_REQUIRED	0x80
#define		BLENDER_READY_BOTH		(BLENDER_READY_MAIN|BLENDER_READY_PIP)

enum
{
   STATUS_NONE,
   STATUS_SYNC_PRESENT,
   STATUS_NO_SYNC,
   STATUS_NO_CABLE,
   STATUS_INVALID_MODE
};

//******************************************************************************
//  G L O B A L    D E F I N I T I O N S
//******************************************************************************

enum				// Auto-adjust status
{
   AA_IDLE,		// Not doing anything
   AA_START,		// AA in progress
   AA_ABORT,		// AA abort by used
};

#define gmd_TRUE			 TRUE
#define gmd_FALSE			 FALSE

enum
{
   OFF = 0,
   ON  = 1
};

typedef union SystemFlagsUnion
{
   WORD all;
   struct
   {
      WORD DramCodeExecution:1;
      WORD SerialFlashInit:1;
      WORD PrintDebug:1;
      WORD IgnoreSleep:1;
      WORD MissionSleep:1; // 1: Keep Mission OCM power on for sleep mode
   }sys;
}SystemFlags_t;

//******************************************************************************
//  E X T E R N S
//******************************************************************************

//
// Application Init Status
//
extern BOOL		B_SplashComplete;			// Splash screen status

//
// Mode Detect
//
extern BYTE 	gB_CurrentInputConnector[NUM_CHANNELS];

//
// Mode Setup
//
extern BYTE		B_SetupInProgress;			// Mode Setup In Progress
extern BOOL		B_FreeRun[NUM_CHANNELS];	// System in Freerun
extern BYTE		B_DisplayLockMethod;		// display Lock Method (Closed Loop or DFL)
extern BYTE 	B_AutoAdjustStatus[NUM_CHANNELS];			// Auto-Adjust status
extern BYTE 	B_AutoAdjustStarted[NUM_CHANNELS];

//
// Overscan
//
extern BYTE    	W_OverscanEnabled[NUM_CHANNELS];		// 0 = Underscan, 1 = Overscan

//
// Aspect Ratios
//
extern gmt_ASPECT_RATIO 	B_OutputAspect;							// Panel Aspect Ratio (Determined in Init())
extern BYTE				 	B_AspectMethod[NUM_CHANNELS];			// Aspect Ratio Applied to Channel

//
// Dynamic Scaling
//
extern gmt_PHY_CH			B_FocusBorder;							// Channel with Focus Border

//
// Tuner Application (NTSC) - Parental Control
//
#if 0
extern gmt_CONTENT_ADVISORY 	TVMainProgramRating;				// Program rating for main input (used by OSD)
extern gmt_CONTENT_ADVISORY 	TVPipProgramRating;					// Program rating for pip input (used by OSD)
#endif
extern BOOL						B_MainProgramBlocked;				// Flag to indicate if program blocked (in main)
extern BOOL						B_PipProgramBlocked;				// Flag to indicate if program blocked (in pip)

//
// VBI
//
extern gmt_VBI_SERVICE 		VBI_ServiceType;            			// Teletex, CC, or none

//
// Buffer for caching Blocked Channels list reads
//
extern WORD W_BlockedChannelListBuffer[(125 + 8)/16];

extern BYTE	B_SignalStatus;

extern 	BYTE 	B_Overlap_Height;

#ifdef USE_SMT
extern BYTE gmvb_SerialStatus;
#endif

extern WORD HighBrightness, LowBrightness, AVGBrightness;
extern BYTE DCR_State, AVGCount, TargetBright;
extern TUartData uartData;

extern BYTE Input_RGB_Range;
extern BYTE Input_YUV_Range;

extern BYTE Input_Color_Domain;
extern BYTE Output_Color_type;

extern SystemFlags_t SystemFlags;

extern DWORD NoSignalFloatOsdTimer;

#endif //__APPGLOBALVARIABLES_H__

