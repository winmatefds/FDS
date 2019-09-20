/*
	$Workfile:   Lpm_SignalingDetect.h  $
	$Revision: 1.25 $
	$Date	 :     $
*/
#ifndef __LPM_SIGNALINGDETECT_H__
#define __LPM_SIGNALINGDETECT_H__

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
// MODULE:      Lpm_SignalingDetect.h
//
// USAGE:       
//
//******************************************************************************
#define AUTOWAKE_CH_CPHY0           1
#define AUTOWAKE_CH_CPHY2           2

#define CLK_MS_REF_TCLK             0
#define CLK_MS_REF_TCLK_DIV8        1
#define CLK_MS_REF_TCLK_DIV16       2
#define CLK_MS_REF_TCLK_DIV32       3

#define CSYNC_CHECK_TIME            100
#define SSYNC_CHECK_TIME            200   // Change to 200ms for RC-OSC difference
#define SOG_CHECK_TIME              350   // Change to 350ms to support Vsync 24Hz
#define SOY_CHECK_TIME              150

#define LPM_CLK_MS_REF_DIV_SHIFT    7   
#define AUTOWAKE_PD1_TRG_SHIFT      4
#define LPM2C2_HDMI_CLK_DET_THRES_SHIFT   9

#define TMDS_DET_THRES              0  // 0=78mV; 1=50mV; 2=19mV; 3=0mV. May affect detect time

#define MAX_SIGDET_PORT_NUM         4

typedef enum
{
   LPM_ASDET_STOP,
   LPM_ASDET_CS_SETUP,
   LPM_ASDET_CS_CHECK,			
   LPM_ASDET_SS_SETUP,
   LPM_ASDET_SS_CHECK,	
   LPM_ASDET_SOG_SETUP,
   LPM_ASDET_SOG_CHECK,
   LPM_ASDET_SOY_SETUP,
   LPM_ASDET_SOY_CHECK,	
}gmt_LPM_ASDET_State;


void Lpm_SignalDetectInit(void);
void Lpm_SignalDetectStop(void);
void Lpm_SignalDetectHandler(void);
void Lpm_TmdsClkDetectInit(gmt_CPhyNum CPhyNum);
gmt_Event2PwrHdr Lpm_DigitalTmdsClkDetect(void);
gmt_Event2PwrHdr Lpm_AnalogSyncDetect(void);
void Lpm_AutoWakePortSetup(gmt_PortChannel B_Port);
void far Lpm_AutoWakeISR(gmt_PortChannel B_Port);
BYTE Lpm_IsWakeupPort(gmt_PortChannel B_Port);
BYTE Lpm_IsAllPortWakeup(void);
BYTE Lpm_IsWakeupTmdsCPhy(gmt_CPhyNum CPhyNum);
void Lpm_ExternalSwitchCheck(void);
BYTE Lpm_GetWakeupExtraInfo(gmt_LPM_Wakeup_Detect CPhyNum);


#endif //__LPM_SIGNALINGDETECT_H__
