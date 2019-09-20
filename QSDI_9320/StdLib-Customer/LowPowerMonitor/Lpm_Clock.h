#ifndef __LPM_CLOCK_H__
#define __LPM_CLOCK_H__

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
// MODULE:      Lpm_Clock.h
//
// USAGE:       
//
//******************************************************************************
#define LPM_OCMCLK_DIV_VALUE	2	//TCLK/4 =6.75Mhz
#define LPM_AUXCLK_DIV_VALUE	1	//01 =TCLK /13 = 2.08Mhz
#define LPM_PWMCLK_DIV_VALUE	7	//2^7 =128


#define SYSTEM_TCLK_FREQ         27000000L
#define MISSION_OCMCLK_FREQ      150000000L


typedef enum
{
  TCLK_DIV_0,  //27 MHz
  TCLK_DIV_1,  //13.5 MHz
  TCLK_DIV_2,  //6.75 MHz
  TCLK_DIV_3,  //3.375 MHz
  TCLK_DIV_4,  //1.68 MHz
  TCLK_DIV_5,  //843 KHz
  TCLK_DIV_6,  //421 KHz
  TCLK_DIV_7,  //211 KHz
} gmt_LPM_OCMCLK_SEL;


void Lpm_ClockSwitch2TCLK(void);
void Lpm_ClockSwitch2RCOSC(gmt_LPM_OCMCLK_SEL Lpm_OcmClkSel);

#endif //__LPM_CLOCK_H__
