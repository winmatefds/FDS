#ifndef __LPM_TCLKTIMER_H__
#define __LPM_TCLKTIMER_H__
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
// MODULE:      Lpm_TclkTimer.h
//
// USAGE:       
//
//******************************************************************************
void Lpm_TclkTimerHandler(void);
void Lpm_TclkTimerInit(DWORD DW_CpuClock);
void Lpm_TclkTimerEnable(void);
void Lpm_TclkTimerDisable(void);
void Lpm_TclkTimerReset(void);
void Lpm_TclkTimerIsr(void);
DWORD Lpm_TclkTimerGetTime(void);
#endif //__LPM_TCLKTIMER_H__
