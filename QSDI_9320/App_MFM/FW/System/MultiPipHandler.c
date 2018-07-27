/*
	$Workfile:   MultiPipHandler.c  $
	$Revision: 1.3 $
	$Date: 2011/12/15 03:12:03 $
*/
#define __MULTIPIPHANDLER_C__

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
// MODULE:      MultiPipHandler.c
//
// USAGE:       This module contains main() function and main Loop related
//              functions.
//
//******************************************************************************

//******************************************************************************
//  I N C L U D E    F I L E S
//******************************************************************************

#include "System\all.h"
#include "mem.h"

//******************************************************************************
//  L O C A L    D E F I N I T I O N S
//******************************************************************************

#define RESTORE_PIP_INPUT_AFTER_EXIT

#define MPIP_MAX_TIMEOUT_COUNT			2

//#define _DEBUG_MSG
#ifdef _DEBUG_MSG
#define msg(x,y)		gm_Print(x,y)
#else
#define msg(x,y)
#endif

typedef struct
{
   BYTE ScanOrder;
   BYTE InputSource;
   BYTE ChannelNumber;
} MultiPipMapping;

typedef struct
{
   MultiPipStates CurrentState;
   BYTE CurrentWindow;
   BYTE AnchorWindow;
   BYTE AnchorChannelNumber;
   MultiPipScanMode ScanMode;
   MultiPipStyleInfo ROM *Stp_CurrentStyle;
   BYTE MaxWindows;
   PipModes PreviousPipMode;
   BYTE PreviousPipInput;
   gmt_SOURCE_TYPE PreviousPipChannelSource;
   MultiPipMapping Mapping[MULTIPIP_MAX_WINDOWS];
} MultiPipData;

//******************************************************************************
//  G L O B A L     V A R I A B L E
//******************************************************************************

//******************************************************************************
//  F U N C T I O N    P R O T O T Y P E S
//******************************************************************************

//******************************************************************************
//  S T A T I C     V A R I A B L E
//******************************************************************************
static MultiPipData MPipPrivData;
static BOOL Bt_IsFreshStart;
static BOOL Bt_MemoryWriteFrozen;

//******************************************************************************
//  C O D E
//******************************************************************************


//******************************************************************************
//
// FUNCTION       : BOOL IsMultiPipOn(void)
//
// USAGE          : Returns TRUE if MPip is on, and FALSE if MPip is off
//
// INPUT          : None
//
// OUTPUT         : TRUE or 0
//
// GLOBALS        : UserPrefPipMode (R)
//
// USED_REGS      : None
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
BOOL IsMultiPipOn(void)
{
   return 0;
}

//******************************************************************************
//
// FUNCTION       : void MultiPipStartIdle(void)
//
// USAGE          : Sets multipip state machine to MULTIPIP_IDLE
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        : MULTIPIP_10ms_TMR timer
//
// USED_REGS      : None
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void MultiPipStartIdle(void)
{

}

//******************************************************************************
//
// FUNCTION       : void MultiPipHandler(void)
//
// USAGE          : Multipip state machine as called from mainloop
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        : MPipPrivData.CurrentState (R)
//                  MPipPrivData.Stp_CurrentStyle->ScanTimeout (R)
//                  MPipPrivData.CurrentWindow (R)
//                  MPipPrivData.Stp_CurrentStyle->AnchorPresent (R)
//
// USED_REGS      : None
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void MultiPipHandler(void)
{
}

//******************************************************************************
//
// FUNCTION       : void MultiPipSetDisplay(gmt_PHY_CH B_Channel)
//
// USAGE          : Sets the display configurations for output HActive, VActive, HStart, VStart for
//                  Pip or Main
//
// INPUT          : B_Channel - gmvb_MainChannel or gmvb_PipChannel
//
// OUTPUT         : None
//
// GLOBALS        : MPipPrivData.Stp_CurrentStyle->W_MainWidth (R)
//                  MPipPrivData.Stp_CurrentStyle->W_MainHeight (R)
//                  MPipPrivData.Stp_CurrentStyle->W_MultiPipHStart (R)
//                  MPipPrivData.Stp_CurrentStyle->W_MainVStart (R)
//                  gmvb_PipChannel (R)
//                  gmvb_MainChannel (R)
//                  gmvw_OutputHStart (W)
//                  gmvw_OutputStart (W)
//                  gmvw_OutputHActive (W)
//                  gmvw_OutputVActive (W)
//
// USED_REGS      : None
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void MultiPipSetDisplay(gmt_PHY_CH B_Channel)
{
   B_Channel++;
}

//******************************************************************************
//
// FUNCTION       : void MultiPipSetup(void)
//
// USAGE          : Function calls to setup multipip parameters
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        : gmvb_PipChannel	(R)
//                  MPipPrivData.Stp_CurrentStyle (R)
//                  gmvw_OutputHActive (R)
//                  gmvw_OutputVActive (R)
//
// USED_REGS      : None
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void MultiPipSetup(void)
{
}

//******************************************************************************
//
// FUNCTION       : void MultiPipIncWindow(void)
//
// USAGE          : Increments window to next one in scan order based on AnchorWindow
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        : MPipPrivData.AnchorWindow
//
// USED_REGS      : None
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void MultiPipIncWindow(void)
{
}

//******************************************************************************
//
// FUNCTION       : void MultiPipDecWindow(void)
//
// USAGE          : Decrements window to previous one in scan order based on AnchorWindow
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        : MPipPrivData.AnchorWindow
//
// USED_REGS      : None
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void MultiPipDecWindow(void)
{
}

//******************************************************************************
//
// FUNCTION       : void MultiPipPageUp(void)
//
// USAGE          : Wrapper to change channel upwards by delta amount equal to number of mpip windows
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        : MPipPrivData.MaxWindows
//
// USED_REGS      : None
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void MultiPipPageUp(void)
{
}

//******************************************************************************
//
// FUNCTION       : void MultiPipPageDown(void)
//
// USAGE          : Wrapper to change channel downwards by delta amount equal to -ve number of mpip windows
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        : MPipPrivData.MaxWindows
//
// USED_REGS      : None
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void MultiPipPageDown(void)
{
}

//******************************************************************************
//
// FUNCTION       : void MultiPipChannelUp(void)
//
// USAGE          : Wrapper to change channel upwards by delta amount equal to 1
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
void MultiPipChannelUp(void)
{
}
//******************************************************************************
//
// FUNCTION       : void MultiPipChannelDown(void)
//
// USAGE          : Wrapper to change channel downwards by delta amount equal to -1
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
void MultiPipChannelDown(void)
{
}

//******************************************************************************
//
// FUNCTION       : void MultiPipSetScanMode(MultiPipScanMode ScanMode)
//
// USAGE          : Accessor function to set mpip scan mode
//
// INPUT          : ScanMode
//
// OUTPUT         : None
//
// GLOBALS        : MPipPrivData.ScanMode	(W)
//
// USED_REGS      : None
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void MultiPipSetScanMode(MultiPipScanMode ScanMode)
{
   ScanMode++;
}

//******************************************************************************
//
// FUNCTION       : void MultiPipTurnOn(MultiPipStyleInfo ROM *MPipStyle)
//
// USAGE          : Entry point for Application interface to turn on multipip
//
// INPUT          : MPipStyle
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
void MultiPipTurnOn(MultiPipStyleInfo ROM *MPipStyle)
{
   MPipStyle++;
}

//******************************************************************************
//
// FUNCTION       : void MultiPipTurnOff(void)
//
// USAGE          : Application interface to turn off multipip
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        : gmvb_MainChannel (R)
//                  gmvb_PipChannel (R)
//                  MPipPrivData.PreviousPipMode (R)
//
// USED_REGS      : None
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void MultiPipTurnOff(void)
{
}

//******************************************************************************
//
// FUNCTION       : void MultiPipSetSourceInMain(void)
//
// USAGE          : Application interface function to set the mpip source/chanel into the main channel
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        : MPipPrivData.Mapping array (R)
//
// USED_REGS      : None
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void MultiPipSetSourceInMain(void)
{
}



/***********************************  END  ************************************/

