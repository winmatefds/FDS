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
//=============================================================================
//
// MODULE:      Lpm_GlobalVariables.c
//
// USAGE:       
//
//******************************************************************************

//******************************************************************************
//
//  I N C L U D E    F I L E S
//
//******************************************************************************
#include ".\LowPowerMonitor\Lpm_All.h"


//****************************************************************************** 
//
//  G L O B A L   V A R I A B L E S 
//
//****************************************************************************** 
WORD LbadcCableNowState[2];
BOOL CableUpdateRequest = FALSE;
gmt_BoardType BoardType = BOARD_RD1;  
gmt_4k2k_ChipID ChipID4k2K = CHIPID_4K2K_NONE;
gmt_CPhyType CPhyType[CPhyNum_Num];
gmt_CPhyType HdmiDviHpdPin[CPhyNum_Num] = {0};
gmt_CPhyType HdmiDviCblDetPin[CPhyNum_Num][1] = {{0}};
WORD DpRxSrcPwrPin[DPRX_NUM] = {0};
WORD DpRxCblDetPin[DPRX_NUM] = {0};
WORD DpRxHpdPin[DPRX_NUM] = {0};

#ifdef PD_MONITORED_BY_LPM_GPIO
WORD LpmGpioLastState[2];
WORD LpmGpioNowState[2];
#endif

BYTE SigDetPortNum = 0;
BYTE CurInputPort[8] = {0xFF};//Sig detect port info
BYTE IsSyncDetected = FALSE;
BYTE WakeupFrom = LPM_WAKEUP_AC_ON;
BYTE ExternalSwitch = 0xFF; // External switch for one Phy
WORD CsmI2cMuxCtrlBackup;
WORD CsmHpdOutCtrlBackup;
WORD CsmHpdPdInCtrlBackup;

WORD ProductId = 0x9301;   //0x9301 means cut2
WORD ProductRev = 0x0002;  //rev2 (cut2.1)

BYTE IsDpSrcPwrWakeup[DPRX_NUM] = {FALSE};
BYTE IsHdmiDviSrcPwrWakeup[CPhyNum_Num] = {FALSE};
BYTE IsVgaSrcPwrWakeup = FALSE;


//******************************************************************************

