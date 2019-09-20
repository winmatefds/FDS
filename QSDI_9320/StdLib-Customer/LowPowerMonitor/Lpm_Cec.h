#ifndef __LPM_CEC_H__
#define __LPM_CEC_H__

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
// MODULE:      Lpm_Cec.h
//
// USAGE:       
//
//******************************************************************************

#ifdef CEC_SUPPORT
#include ".\LowPowerMonitor\CecLib.h"
void Lpm_CecHandler(void);
void Lpm_CecInit(void);
#endif
#endif //__LPM_CEC_H__
