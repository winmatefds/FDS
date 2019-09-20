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
//=============================================================================
//
// MODULE:      Lpm_BrownOut.c
//
// USAGE:       
//
//******************************************************************************

//******************************************************************************
//
//  I N C L U D E    F I L E S
//
//******************************************************************************
#include ".\LowPowerMonitor\Lpm_All.h"

#ifdef BROWN_OUT_SUPPORT

//******************************************************************************
//
//  C O D E
//
//******************************************************************************

void Lpm_BrownOutInit(void)
{
   // Set Watch dog counter to 0x3FFFC00 * 37ns = 2.48 sec
   gm_WriteRegWord(LPM_OCM_SW_WDT_VAL ,0xffff); 

   //Write Enable Trigger  for Unlock Sequence. Write a 1 followed by a zero to trigger each bit of the unlock sequence.
   //The Software must write a 24 bit ASCII serial Sequence "RUN" (all caps) MSB first (MSB of hex value of 'R' ) to start the watchdog counter.
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x02);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0A);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x06);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0E);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x02);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0A);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x06);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0E);

   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x02);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0A);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x02);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0A);         
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x06);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0E);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x02);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0A);

   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x02);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0A);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x06);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0E);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x02);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0A);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x06);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0E);

   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x02);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0A);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x06);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0E);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x02);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0A);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x06);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0E);

   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x02);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0A);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x06);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0E);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x02);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0A);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x02);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0A);

   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x06);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0E);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x06);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0E);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x06);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0E);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x02);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0A);   

   //Enables the Software Watchdog Timer. On Timeout, this circuit will generate a reset 
   //Add Test mode , not reset 
   gm_SetRegBitsByte(LPM_OCM_SW_WDT_CTRL,EN_OCM_SW_WDT);	

   Lpm_BrownOutReTrigger();
   Lpm_InterruptEnable(gmd_SWDT);

}


void Lpm_BrownOutReTrigger(void)
{
   //Writes a 1 to this bit to retrigger and preloading of OCM_SW_WDT_VAL into the Software-WDT.
   gm_SetRegBitsByte(LPM_OCM_SW_WDT_TRIG, OCM_SOFT_WDT_TRIG);
}

//******************************************************************************
#endif

#ifdef LPM_RESET_SUPPORT
void Lpm_ResetLpm(void)
{
   // Reset LPM by Brown Out
   // Set Watch dog counter to 1 = 37.89 us
   gm_WriteRegWord(LPM_OCM_SW_WDT_VAL ,1); 

   //Write Enable Trigger  for Unlock Sequence. Write a 1 followed by a zero to trigger each bit of the unlock sequence.
   //The Software must write a 24 bit ASCII serial Sequence "RUN" (all caps) MSB first (MSB of hex value of 'R' ) to start the watchdog counter.
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x02);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0A);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x06);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0E);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x02);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0A);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x06);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0E);

   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x02);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0A);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x02);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0A);         
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x06);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0E);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x02);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0A);

   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x02);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0A);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x06);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0E);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x02);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0A);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x06);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0E);

   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x02);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0A);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x06);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0E);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x02);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0A);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x06);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0E);

   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x02);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0A);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x06);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0E);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x02);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0A);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x02);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0A);

   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x06);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0E);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x06);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0E);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x06);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0E);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x02);
   gm_WriteRegByte(LPM_OCM_SW_WDT_CTRL,0x0A);   

   //Enables the Software Watchdog Timer. On Timeout, this circuit will generate a reset 
   gm_SetRegBitsByte(LPM_OCM_SW_WDT_CTRL,EN_OCM_SW_WDT);	

   //Writes a 1 to this bit to retrigger and preloading of OCM_SW_WDT_VAL into the Software-WDT.
   gm_SetRegBitsByte(LPM_OCM_SW_WDT_TRIG, OCM_SOFT_WDT_TRIG);

   while(gm_ReadRegByte(LPM_OCM_SW_WDT_CTRL)&EN_OCM_SW_WDT);    

}
#endif

