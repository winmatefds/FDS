#ifndef __LPM_BROWNOUT_H__
#define __LPM_BROWNOUT_H__

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
// MODULE:      Lpm_BrownOut.h
//
// USAGE:       
//
//******************************************************************************

void Lpm_BrownOutInit(void);
void Lpm_BrownOutReTrigger(void);
void Lpm_ResetLpm(void);

#endif //__LPM_BROWNOUT_H__
