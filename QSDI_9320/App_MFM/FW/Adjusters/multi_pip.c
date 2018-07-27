/*
    $Workfile:   multi_pip.c  $
    $Revision: 1.3 $
    $Date: 2011/12/15 03:12:00 $
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
// MODULE:      multi_pip.c
//
// USAGE:       This module contains application level code for multi pip
//
//******************************************************************************

//******************************************************************************
//  I N C L U D E    F I L E S
//******************************************************************************

#include "System\all.h"

//******************************************************************************
//  C O D E
//******************************************************************************

//******************************************************************************
//
// FUNCTION       : BYTE CanMultiPipBeOn(void)
//
// USAGE          : returns whether MultiPip can be turned on
//
// INPUT          :
//
// OUTPUT         : True or False
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
BYTE CanMultiPipBeOn(void)
{
#ifndef NO_PIP_SUPPORT
   return FALSE;
#else
   return FALSE;
#endif	// ifndef NO_PIP_SUPPORT
}

//******************************************************************************
//
// FUNCTION       : void TurnOnMultiPipStyle(void)
//
// USAGE          : Turns on multi-pip with the style set in the userpref
//
// INPUT          :
//
// OUTPUT         : None
//
// GLOBALS        : UserPrefMPipConfig	(R)
//
// USED_REGS      : None
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void TurnOnMultiPipStyle(void)
{
}

//******************************************************************************
//
// FUNCTION       : void MPipConfigIncDec(void)
//
// USAGE          : Increment and decrements MPIP config userpref with wrapping taken care of
//
// INPUT          :
//
// OUTPUT         : None
//
// GLOBALS        : UserPrefMPipConfig	(R)
//
// USED_REGS      : None
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void MPipConfigIncDec(void)
{
}

//******************************************************************************
//
// FUNCTION       : BOOL IsMultiPipControlLocked(void)
//
// USAGE          : Returns true if MPIP controls are locked
//
// INPUT          :
//
// OUTPUT         : b_MultiPIPLockControl or False
//
// GLOBALS        : b_MultiPIPLockControl	(R)
//
// USED_REGS      : None
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
BOOL IsMultiPipControlLocked(void)
{
   return FALSE;
}

//******************************************************************************
//
// FUNCTION       : void LockMultiPipControl(void)
//
// USAGE          : Locks MPIP controls
//
// INPUT          :
//
// OUTPUT         : None
//
// GLOBALS        : b_MultiPIPLockControl	(R)
//
// USED_REGS      : None
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void LockMultiPipControl(void)
{
}

//******************************************************************************
//
// FUNCTION       : void UnlockMultiPipControl(void)
//
// USAGE          : Unlocks MPIP controls
//
// INPUT          :
//
// OUTPUT         : None
//
// GLOBALS        : b_MultiPIPLockControl	(R)
//
// USED_REGS      : None
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void UnlockMultiPipControl(void)
{
}



