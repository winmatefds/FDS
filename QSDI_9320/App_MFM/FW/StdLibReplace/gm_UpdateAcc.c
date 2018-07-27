/*
   $Workfile:   gm_UpdateAcc.c  $
   $Revision: 1.8 $
   $Date: 2012/10/12 06:15:40 $
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
//=============================================================
//
// MODULE:  gm_UpdateAcc.c
//
// USAGE:   This file contains the routine for updating the Active Contrast Control (ACC) block
//         based on the selected mode chosen by the application
//
//******************************************************************************

//******************************************************************************
//  I N C L U D E    F I L E S
//******************************************************************************

#include "System\all.h"
#include "stdLibInc\gm_Register.h"
#include "StdLibReplace\Acc.h"
#if (FEATURE_ACC3 == ENABLE)
#include "StdLibInc\gm_ACC3.h"
#endif


/****************************************************************************/
//   S T A T I C   V A R I A B L E S                                 */
/****************************************************************************/

//******************************************************************************
//  L O C A L    D E F I N I T I O N S
//******************************************************************************
//#define _DEBUG

#ifdef _DEBUG
#define  msg(a)             gm_Print(a,0)
#define  msg_i(a,b)         gm_Print(a,b)
#else
#define  msg(a)
#define  msg_i(a,b)
#endif

//******************************************************************************
//  G L O B A L    V A R I A B L E S
//******************************************************************************
#if (FEATURE_ACC3 == ENABLE)

extern VQ_ACC_Parameters_t ROM * ROM Sta_AccSchemes[];

gmt_RET_STAT gm_UpdateAcc (gmt_IMAGE_SCHEME_MODE ColorScheme)
{

   WORD W_Status;
   static gmt_IMAGE_SCHEME_MODE CurrentScheme;

   if (CurrentScheme == ColorScheme)
      return OK;
   CurrentScheme = ColorScheme;


   msg_i("Update Acc: Coor Scheme = %d",CurrentScheme);
   if (CurrentScheme)
   {
      VQ_ACC_Parameters_t far * AccTable;
      AccTable = (VQ_ACC_Parameters_t far *)Sta_AccSchemes[CurrentScheme-1];

      gm_ACC3SetParams(AccOptionsTableSelect,(CurrentScheme < IS_MAX) ? &AccTable->ACCHWParams : NULL_PTR,sizeof(VIP_ACCHWParameters_t));
      gm_ACC3SetParams(AccParamTableSelect,(CurrentScheme < IS_MAX) ? &AccTable->ACCSWParams : NULL_PTR,sizeof(VIP_ACCSWParameters_t));
      LLD_ACCSetMemoryParams((CurrentScheme < IS_MAX) ? &AccTable->ACCStatic : NULL_PTR);

      gm_ACC3UpdateModeDependants();

      CLRBIT (gmvw_AccAcmMode, ACC_OFF);
      SETBIT (gmvw_AccAcmMode, ACC_ON);
      gm_VideoProcessingEnable (VIDEO_ACC);
   }
   else
   {
      CLRBIT (gmvw_AccAcmMode, ACC_ON);
      SETBIT (gmvw_AccAcmMode, ACC_OFF);
      gm_VideoProcessingDisable (VIDEO_ACC);
      W_Status = OK;
   }
   return W_Status;
}
#endif
