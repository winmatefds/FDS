/*
	$Workfile:   Lpm_PowerMode.h  $
	$Revision: 1.34 $
	$Date	 :     $
*/

#ifndef __LPM_POWERMODE_H__
#define __LPM_POWERMODE_H__

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
// MODULE:      Lpm_PowerMode.h
//
// USAGE:       
//
//******************************************************************************

#define WAIT_TCLK_STABLE_DELAY         5  //ms
#define WAIT_MAIN_PWR_STABLE_DELAY     50 //ms, 64 TCLK min~50ms max. This can be measured on a per-design basis
#define LPM_DC_OFF_TIMEOUT             5000  //ms


typedef enum
{
	LPM_NormalPowerMode,
	LPM_LowPowerMode,
	LPM_PowerOffMode,	
}gmt_LPMPWRState;

typedef enum		//Mission Power state
{
	Pwr_On,				// normal functioning (syncs on both VS and HS)
	Pwr_Sleep,			// OCM clock ->27/8 MHz 
	Pwr_MissionSleep,	// For Mission specical purpose 
	Pwr_Off,				// soft power off (no need to sync monitoring, only power key can wake up system)
	Pwr_Transition
} gmt_PwrStateType;

typedef enum
{
	LPM_EVNT2PWR_NONE 					=	0,
	LPM_EVNT2PWR_PWRKEY 					= BIT0,	
	LPM_EVNT2PWR_AUXWAKE_DETECT		= BIT1, 
	LPM_EVNT2PWR_TMDS_DETECT			= BIT2, 
	LPM_EVNT2PWR_ANASYNC_DETECT		= BIT3, 
	LPM_EVNT2PWR_SG_DETECT_MASK		= (BIT1|BIT2|BIT3), 

	LPM_EVNT2PWR_HPD_DETECT				= BIT4,				//HPD means Cable detection on 1~3 
	LPM_EVNT2PWR_PD_DETECT				= BIT5, 				//PD means Power detection on 1~4	
	LPM_EVNT2PWR_SLEEP_NOTICE     	= BIT6, 	
	LPM_EVNT2PWR_POWER_ON_NOTICE     = BIT7, 	
}gmt_Event2PwrHdr;

void Lpm_PowerModeInit(void);
void Lpm_SetPowerKeyEvent(void);
void Lpm_SetAnyKeyWakeEvent(void);
BOOL Lpm_PowerModeStateChange(void);
void Lpm_PowerModeHandler(void);
void Lpm_PowerModeSetEvent2PwrHdr(gmt_Event2PwrHdr SetEvent);
BYTE Lpm_PowerModeGetState(void);
void Lpm_PowerModeCsmMuxSel(BYTE PwrMode);
void Lpm_PowerDownLpmDevices(BYTE PwrMode);
void Lpm_PowerUpLpmDevices(void);
void Lpm_WarmReset(BYTE IsEnable);
void Lpm_HpdControl(BYTE PwrMode);
void Lpm_ForceDpHpdLow(void);
void Lpm_PowerDownMission(void);
void Lpm_TmdsSwitcherControl(BYTE PwrMode);
BOOL IsMissionPowerON(void);
BOOL IsDCOffMode(void);

#endif //__LPM_POWERMODE_H__
