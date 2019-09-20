#ifndef __LPM_GLOBALVARIABLE_H__
#define __LPM_GLOBALVARIABLE_H__

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
// MODULE:      Lpm_GlobalVariables.h
//
// USAGE:       
//
//******************************************************************************

extern WORD LbadcCableLastState[2];
extern WORD LbadcCableNowState[2];
extern WORD LpmGpioLastState[2];
extern WORD LpmGpioNowState[2];

extern BOOL CableUpdateRequest;
extern gmt_BoardType BoardType;
extern gmt_4k2k_ChipID ChipID4k2K;
extern gmt_CPhyType CPhyType[CPhyNum_Num];
extern gmt_CPhyType HdmiDviHpdPin[CPhyNum_Num];
extern gmt_CPhyType HdmiDviCblDetPin[CPhyNum_Num][1];
extern WORD DpRxSrcPwrPin[DPRX_NUM];
extern WORD DpRxCblDetPin[DPRX_NUM];
extern WORD DpRxHpdPin[DPRX_NUM];

extern BYTE SigDetPortNum;
extern BYTE CurInputPort[8];
extern BYTE IsSyncDetected;
extern BYTE WakeupFrom;
extern BYTE ExternalSwitch;
extern WORD CsmI2cMuxCtrlBackup;
extern WORD CsmHpdOutCtrlBackup;
extern WORD CsmHpdPdInCtrlBackup;

extern WORD ProductId;
extern WORD ProductRev;


#endif  //__LPM_GLOBALVARIABLES_H__
