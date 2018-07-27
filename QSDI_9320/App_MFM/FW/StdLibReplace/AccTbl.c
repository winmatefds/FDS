/*
	$Workfile:   AccTbl.c  $
	$Revision: 1.6 $
	$Date: 2012/10/16 10:00:30 $
*/
#define __ACC_TBL_C__
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
// MODULE:	AccTbl.c
//
// USAGE:	This file contains table for Active Contrast Control (ACC)
//
//******************************************************************************

//******************************************************************************
//  I N C L U D E    F I L E S
//******************************************************************************

#include "StdLibInc\gm_ChipFeature.h"
#include "stdLibInc\gm_Global.h"
#include "StdLibReplace\Acc.h"

#if FEATURE_ACC3
//******************************************************************************
//  E X T E R N S
//******************************************************************************

extern VQ_ACC_Parameters_t ROM AccVivid;
extern VQ_ACC_Parameters_t ROM AccCinema;
extern VQ_ACC_Parameters_t ROM AccNature;
extern VQ_ACC_Parameters_t ROM AccSport;
extern VQ_ACC_Parameters_t ROM AccLinear;

//***************************************************************************
//  C O D E
//***************************************************************************

VQ_ACC_Parameters_t ROM * ROM Sta_AccSchemes[] =
{
   &AccVivid,
   &AccCinema,
   &AccNature,
   &AccSport,
   &AccLinear,
};

#endif



