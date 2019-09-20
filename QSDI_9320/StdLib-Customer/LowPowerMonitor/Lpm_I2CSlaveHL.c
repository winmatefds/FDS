
/*
   $Workfile:   Lpm_I2CSlaveHL.c  $
   $Revision: 1.9 $
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
//  MODULE: Lpm_I2CSlaveHL.c
//
//******************************************************************************

//******************************************************************************
//  I N C L U D E    F I L E S
//******************************************************************************
#include ".\LowPowerMonitor\Lpm_All.h"
#include <mem.h>

#ifdef I2C_SLAVE_SUPPORT
//******************************************************************************
//  L O C A L    D E F I N I T I O N S
//******************************************************************************

//******************************************************************************
//  G L O B A L   V A R I A B L E S
//******************************************************************************
I2C_SLAVE_HL_TYPE    I2CSlaveHLModuleData[I2C_SLAVE_NUM_CH]={{0},{0},{0},{0}};


//******************************************************************************
//  C O D E
//******************************************************************************
//******************************************************************************
// FUNCTION:   Lpm_I2CSlaveInit
// USAGE:      Init the I2C slave
// INPUT:      BYTE* Bp_StartAddr - pointer to the buffer
// OUTPUT:     WORD - how many bytes need for this module
// GLOBALS:
// USED_REGS:  None
//******************************************************************************
void Lpm_I2CSlaveHLInit (BYTE B_MaxFunc, SLAVE_CH B_SlaveCh)
{
   I2CSlaveHLModuleData[B_SlaveCh].B_MaxFuncNum = B_MaxFunc;
   I2CSlaveHLModuleData[B_SlaveCh].B_CurFuncNum = 0; 
}

//******************************************************************************
// USAGE:
//
// INPUT:
// OUTPUT:
// GLOBALS:
//******************************************************************************
BOOL Lpm_I2CSlaveHLAddCommHandler(I2C_COMM_HANDLER_TYPE Fp_NewCommHandler, BYTE B_CorrDevID, SLAVE_CH B_SlaveCh)
{
   if((I2CSlaveHLModuleData[B_SlaveCh].B_CurFuncNum >= I2CSlaveHLModuleData[B_SlaveCh].B_MaxFuncNum)
      || (B_CorrDevID & 0x01)
      || (I2CSlaveHLModuleData[B_SlaveCh].Bp_CorrDevIDArray == NULL_PTR)
      || (I2CSlaveHLModuleData[B_SlaveCh].Fp_CommHandlerArray == NULL_PTR))
   {
      //msg("Lpm_I2CSlaveAddCommHandler() ERROR: CommHandlerArray is full",0);
      return FALSE;
   }

   I2CSlaveHLModuleData[B_SlaveCh].Fp_CommHandlerArray[I2CSlaveHLModuleData[B_SlaveCh].B_CurFuncNum] = Fp_NewCommHandler;
   I2CSlaveHLModuleData[B_SlaveCh].Bp_CorrDevIDArray[I2CSlaveHLModuleData[B_SlaveCh].B_CurFuncNum++] = B_CorrDevID;

   //msg("Lpm_I2CSlaveAddCommHandler() was done for DevID = 0x%X",B_CorrDevID);

   return TRUE;
}

//******************************************************************************
// USAGE:
//
// INPUT:
// OUTPUT:
// GLOBALS:
//******************************************************************************
I2C_COMM_HANDLER_TYPE Lpm_I2CSlaveHLFindCommHandler(BYTE B_DevID, SLAVE_CH B_SlaveCh)
{
   BYTE B_Count;
   I2C_COMM_HANDLER_TYPE Fp_CorrespondingFuncPtr = NULL_PTR;

   if(I2CSlaveHLModuleData[B_SlaveCh].B_CurFuncNum == 0)
      return Fp_CorrespondingFuncPtr;
      
   B_DevID = B_DevID & I2C_ADDRESS_BITS;
   //msg("B_DevID = 0x%X",B_DevID);

   for(B_Count = 0; B_Count <= I2CSlaveHLModuleData[B_SlaveCh].B_CurFuncNum; B_Count++)
   {
      if(B_DevID == I2CSlaveHLModuleData[B_SlaveCh].Bp_CorrDevIDArray[B_Count])
      {
         //msg("CommHandler for B_DevID = %X was found",B_DevID);
         Fp_CorrespondingFuncPtr = I2CSlaveHLModuleData[B_SlaveCh].Fp_CommHandlerArray[B_Count];
      }
   }
   return Fp_CorrespondingFuncPtr;
}


//******************************************************************************
// FUNCTION:   Lpm_I2CSlaveHLGetAddrBuffer
// USAGE:      get pointer to the buffer
// INPUT:      None
// OUTPUT:     None
// GLOBALS:    None
// USED_REGS:  None
//******************************************************************************
I2C_SLAVE_HL_TYPE* Lpm_I2CSlaveHLGetAddrModuleData(SLAVE_CH B_SlaveCh)
{
   return &I2CSlaveHLModuleData[B_SlaveCh];
}
#endif

