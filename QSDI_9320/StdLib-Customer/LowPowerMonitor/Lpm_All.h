/*
	$Workfile:   Lpm_All.h  $
	$Revision: 1.11 $
	$Date: 2011/12/15 01:43:01 $
*/
#ifndef __LPM_ALL_H__
#define __LPM_ALL_H__
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
// MODULE:      Lpm_all.h
//
// USAGE:       Header file for all include files requires and
//				hardware specific definition.
//
//******************************************************************************

//******************************************************************************
//	C O M P I L E R    O P T I O N S										
//******************************************************************************

//******************************************************************************
//	G L O B A L    D E F I N I T I O N S									
//******************************************************************************

//******************************************************************************
//  G L O B A L    I N C L U D E	F I L E
//******************************************************************************
#include ".\LibInc\gm_ChipFeature.h"
#include ".\LibInc\gm_Register.h"
#include ".\LibInc\gm_Global.h"
#include ".\LibInc\gm_HostReg.h"
#include ".\LibInc\gm_LPM_IPC.h"
#include ".\LibInc\gm_Mcu186.h"
#include ".\LowPowerMonitor\Lpm_System.h"
#include ".\LowPowerMonitor\Lpm_Irom.h"
#include ".\LowPowerMonitor\Lpm_Debug.h"
#include ".\LowPowerMonitor\Lpm_Ddcci.h"
#include ".\LowPowerMonitor\Lpm_I2cSlave.h"
#include ".\LowPowerMonitor\Lpm_I2cSlaveHL.h"
#include ".\LowPowerMonitor\Lpm_I2cMaster.h"
#include ".\LowPowerMonitor\Lpm_Nvram.h"
#include ".\LowPowerMonitor\Lpm_Csm.h"
#include ".\LowPowerMonitor\Lpm_AuxRx.h"
#include ".\LowPowerMonitor\Lpm_AuxRxI2C.h"
#include ".\LowPowerMonitor\Lpm_AuxRxEDID.h"
#include ".\LowPowerMonitor\Lpm_Uart.h"
#include ".\LowPowerMonitor\Lpm_Main.h"
#include ".\LowPowerMonitor\Lpm_PowerMode.h"
#include ".\LowPowerMonitor\Lpm_TclkTimer.h"
#include ".\LowPowerMonitor\Lpm_BrownOut.h"
#include ".\LowPowerMonitor\Lpm_CableDetect.h"
#include ".\LowPowerMonitor\Lpm_Cec.h"
#include ".\LowPowerMonitor\Lpm_Clock.h"
#include ".\LowPowerMonitor\Lpm_Ir.h"
#include ".\LowPowerMonitor\Lpm_Keypad.h"
#include ".\LowPowerMonitor\Lpm_SignalingDetect.h"
#include ".\LowPowerMonitor\Lpm_PseudoLT.h"
#include ".\LowPowerMonitor\Lpm_HostIF.h"
#include ".\LowPowerMonitor\Lpm_BrownOut.h"
#include ".\LowPowerMonitor\Lpm_Clock.h"
#include ".\LowPowerMonitor\Lpm_TouchPad.h"
#include ".\LowPowerMonitor\Lpm_GlobalVariables.h"


#endif

//*********************************  END  **************************************
