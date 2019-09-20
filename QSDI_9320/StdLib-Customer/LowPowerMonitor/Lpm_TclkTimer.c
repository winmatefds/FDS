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
// MODULE:      Lpm_TclkTimer.c
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

#ifdef TCLK_TIMER_SUPPORT

//******************************************************************************
//
//  L O C A L    D E F I N E S
//
//******************************************************************************
//#define POWER_CYCLE_RECURRING	

static volatile DWORD DW_TimeInSec;
DWORD DW_WakeTime = 20;//in units of seconds
DWORD DW_SleepTime = 10;//in units of seconds
BOOL Bt_IsWakeAlarmActive = FALSE;
BOOL Bt_IsSleepAlarmActive = FALSE;

//******************************************************************************
//
//  C O D E
//
//******************************************************************************

//******************************************************************************
//
// FUNCTION:	void Lpm_TclkTimerInit(DWORD DW_CpuClock)
//
// USAGE:		Initialize TCLK Timer and interrupts but keep it disabled
//
// INPUT:		DW_CpuClock - CPU clock Frequency
//
// OUTPUT:		None
//
// GLOBALS:		None
//
// USED_REGS:	TCLK_TIMER_CTRL 	(W)
//				TCLK_TIMER_INCR		(W)
//				TCLK_TIMER_THRESH_hi (W)
//				TCLK_TIMER_THRESH_lo (W)
//
// PRE_CONDITION: 
//
// POST_CONDITION:
//
//******************************************************************************
void Lpm_TclkTimerInit(DWORD DW_CpuClock)
{


	DWORD DW_Threshold = DW_CpuClock / 1;//overflow every second
   
	Lpm_InterruptDisable(gmd_TCLK_TIMER);
	// Note: MUST program threshold before turning on!
	gm_WriteRegWord(LPM_TCLK_TIMER_INCR, 0x0001);
	gm_WriteRegWord(LPM_TCLK_TIMER_THRESH_hi, (WORD)(DW_Threshold>>16));
	gm_WriteRegWord(LPM_TCLK_TIMER_THRESH_lo, (WORD)DW_Threshold);
	gm_WriteRegWord(LPM_TCLK_TIMER_CTRL, TCLK_TIMER_EN);
	Lpm_InterruptEnable(gmd_TCLK_TIMER);

}

//******************************************************************************
//
// FUNCTION:	void Lpm_TclkTimerEnable(void)
//
// USAGE:		Enable TCLK Timer
//
// INPUT:		None
//
// OUTPUT:		None
//
// GLOBALS:		None
//
// USED_REGS:	TCLK_TIMER_CTRL 	(R/W)
//
// PRE_CONDITION: 
//
// POST_CONDITION:
//
//******************************************************************************
void Lpm_TclkTimerEnable(void)
{
	gm_SetRegBitsWord(LPM_TCLK_TIMER_CTRL, TCLK_TIMER_EN);
}

//******************************************************************************
//
// FUNCTION:	void Lpm_TclkTimerDisable(void)
//
// USAGE:		Disable TCLK Timer
//
// INPUT:		None
//
// OUTPUT:		None
//
// GLOBALS:		None
//
// USED_REGS:	TCLK_TIMER_CTRL 	(R/W)
//
// PRE_CONDITION: 
//
// POST_CONDITION:
//
//******************************************************************************
void Lpm_TclkTimerDisable(void)
{
	gm_ClearRegBitsWord(LPM_TCLK_TIMER_CTRL, TCLK_TIMER_EN);
}

//******************************************************************************
//
// FUNCTION:	void Lpm_TclkTimerReset(void)
//
// USAGE:		Reset both hardware TCLK Timer and software counting variable
//
// INPUT:		None
//
// OUTPUT:		None
//
// GLOBALS:		None
//
// USED_REGS:	TCLK_TIMER_CTRL 	(R/W)
//
// PRE_CONDITION: 
//
// POST_CONDITION:
//
//******************************************************************************
void Lpm_TclkTimerReset(void)
{

	gm_SetRegBitsWord(LPM_TCLK_TIMER_CTRL, TCLK_TIMER_RESET);
	asm {nop}
	DW_TimeInSec = 0;
	gm_ClearRegBitsWord(LPM_TCLK_TIMER_CTRL, TCLK_TIMER_RESET);	
}

//******************************************************************************
//
// FUNCTION:	DWORD Lpm_TclkTimerGetTime(void)
//
// USAGE:		Return time in seconds
//
// INPUT:		None
//
// OUTPUT:		Time in seconds
//
// GLOBALS:		None
//
// USED_REGS:	None
//
// PRE_CONDITION: 
//
// POST_CONDITION:
//
//******************************************************************************
DWORD Lpm_TclkTimerGetTime(void)
{
	/*The following strange-looking code is to avoid the need to disable and re-enable
	the interrupts (using sti and cli) when retrieving the time.

	The problem with not disabling interrupts is that since this variable is a double word,
	it takes two accesses to update the whole variable. An interrupt can happen in between
	those two accesses. The variable DW_TimeInSec is updated by an interrupt every 
	second. Let's say it has the current value of 0x0001FFFF. During the copying of this 
	value to DW_Temp, an interrupt happens in between those two accesses. And the 
	upper word is always copied first. So the following can happen if we do not disable
	interrupts:

	1. Upper word of DW_TimeInSec is copied to upper word of DW_Temp; so now upper
	word of DW_Temp is 0x0001.

	2. An interrupt happens to increment DW_TimeInSec so now it becomes 0x00020000.

	3. Interrupt service routine exits and the lower word of DW_TimeInSec is copied to 
	the lower word of DW_Temp, so now lower word of DW_Temp is 0x0000. So putting
	the upper and lower word together DW_Temp will becomes 0x00010000 which is 
	different from DW_TimeInSec at that moment.
	*/

	DWORD DW_Temp = DW_TimeInSec;
	if (*((WORD*)(&DW_TimeInSec) + 1) != (WORD)(DW_Temp>>16))
		DW_Temp = DW_TimeInSec;	
	return DW_Temp;
}

//******************************************************************************
//
// FUNCTION:	void Lpm_TclkTimerIsr(void)
//
// USAGE:		Interrupt service routine for Tclk Timer. It increments a software counter.
//
// INPUT:		None
//
// OUTPUT:		None
//
// GLOBALS:		None
//
// USED_REGS:	None
//
// PRE_CONDITION: 
//
// POST_CONDITION:
//
//******************************************************************************
void Lpm_TclkTimerIsr(void)
{
	DW_TimeInSec++;
}

//******************************************************************************
//
// FUNCTION:	BOOL Lpm_TclkTimerHandler(void)
//
// USAGE:		Return if current time reaches preconfigured wake/sleep time. 
//
// INPUT:		None
//
// OUTPUT:		TRUE - time is up
//				FALSE - time is not up yet
//
// GLOBALS:		None
//
// USED_REGS:	None
//
// PRE_CONDITION: 
//
// POST_CONDITION:
//
//******************************************************************************
extern BOOL powerUpRequest;
void Lpm_TclkTimerHandler(void)
{
   // BOOL Bt_IsTimeUp = FALSE;
    DWORD DW_Now = Lpm_TclkTimerGetTime();
    BOOL Bt_IsMissionUp = IsMissionPoweredUp();

    if (DW_Now == DW_WakeTime)
    {
        if (Bt_IsWakeAlarmActive)
        {
            Bt_IsWakeAlarmActive = FALSE;
            #ifdef POWER_CYCLE_RECURRING			
                DW_WakeTime = DW_Now + 20;//wake up in 20s
            #endif
            if (!Bt_IsMissionUp)
				powerUpRequest = TRUE;
        }
    }
    #ifdef POWER_CYCLE_RECURRING
        else
            Bt_IsWakeAlarmActive = TRUE;
    #endif

	if (DW_Now == DW_SleepTime)
    {
        if (Bt_IsSleepAlarmActive)
        {
            Bt_IsSleepAlarmActive = FALSE;
            #ifdef POWER_CYCLE_RECURRING
                DW_SleepTime = DW_Now + 20;//sleep in 20s
            #endif
            if (Bt_IsMissionUp)
            {
                BYTE B_Buffer = TRUE;
                gm_MessageSend(gmd_MESSAGE_POWER, (BYTE far *)&B_Buffer, sizeof(B_Buffer));//send to mission so mission can power down itself
            }
        }
    }
    #ifdef POWER_CYCLE_RECURRING
        else
            Bt_IsSleepAlarmActive = TRUE;
    #endif
	//return Bt_IsTimeUp;
}
#endif
