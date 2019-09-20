
/*
   $Workfile:   Lpm_I2CSlaveHL.h  $
   $Revision: 1.5 $
   $Date: 2011/12/15 01:43:01 $
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
//******************************************************************************
//
//  MODULE: Lpm_I2CSlaveHL.h
//
//
//  USAGE:  Header file for I2C slave high level module
//
//******************************************************************************
#ifndef __LPM_I2CSLAVEHL_H__
#define __LPM_I2CSLAVEHL_H__


//******************************************************************************
//  G L O B A L    D E F I N I T I O N S
//******************************************************************************
#define MAX_DEV_IDS 3

typedef BOOL far (* I2C_COMM_HANDLER_TYPE)(BYTE B_Channel, BYTE B_DevID, BYTE *Bp_Buff);

typedef struct
{
   BYTE                   B_MaxFuncNum;
   BYTE                   B_CurFuncNum;
   BYTE                   Bp_CorrDevIDArray[MAX_DEV_IDS];
   I2C_COMM_HANDLER_TYPE  Fp_CommHandlerArray[MAX_DEV_IDS];
}I2C_SLAVE_HL_TYPE;


//******************************************************************************
//  G L O B A L   V A R I A B L E S
//******************************************************************************

//extern I2C_SLAVE_HL_TYPE I2CSlaveHLModuleData;


//******************************************************************************
//  G L O B A L   F U N T I O N S
//******************************************************************************

I2C_COMM_HANDLER_TYPE Lpm_I2CSlaveHLFindCommHandler(BYTE B_DevID, SLAVE_CH B_SlaveCh);
I2C_SLAVE_HL_TYPE* Lpm_I2CSlaveHLGetAddrModuleData(SLAVE_CH B_SlaveCh);


#endif // __LPM_I2CSLAVEHL_H__