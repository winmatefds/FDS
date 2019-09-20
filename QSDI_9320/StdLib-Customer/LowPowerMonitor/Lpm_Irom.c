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
// MODULE:      Lpm_Irom.c
//
// USAGE:       
//
//******************************************************************************
#include ".\LowPowerMonitor\Lpm_All.h"

static volatile gmt_STime g_SystemTime;
static gmt_STimeEx Lpm_timer[LPM_MAX_TIMER];

//******************************************************************************
//
//  C O D E
//
//******************************************************************************
#if 0//def CUT_2_1_PATCH
//******************************************************************************
// USAGE:       Register access for word read
// INPUT:
// OUTPUT:      Return register value for a word length
// GLOBALS:	   None 
// USED_REGS: 	None
//******************************************************************************
WORD gm_ReadRegWord(WORD W_RegAddr)
{
   return (((WORD)(*(volatile BYTE *)(W_RegAddr+1))<<8) | (*(volatile BYTE *)(W_RegAddr)));
}
void gm_ClearAndSetBitsWord(WORD W_RegAddr, WORD W_Clear, WORD W_Set) 
{
   (*(volatile WORD *)(W_RegAddr) = ((gm_ReadRegWord(W_RegAddr) & ~(WORD)(W_Clear))) | (WORD)(W_Set));
}
void gm_ClearRegBitsWord(WORD W_RegAddr, WORD W_Clear) 
{
   (*(volatile WORD *)(W_RegAddr) = (gm_ReadRegWord(W_RegAddr) & ~(WORD)(W_Clear)));
}
void gm_SetRegBitsWord(WORD W_RegAddr, WORD W_Set) 
{
   (*(volatile WORD *)(W_RegAddr) = (gm_ReadRegWord(W_RegAddr) | (WORD)(W_Set)));
}
DWORD gm_ReadRegDWord_LE(WORD W_RegAddr)
{
   return (((DWORD)(gm_ReadRegWord(W_RegAddr+2)) << 16) | gm_ReadRegWord(W_RegAddr));
}
void gm_SetRegBitsDWord(WORD W_RegAddr, DWORD DW_Set)
{
   (*(volatile DWORD *)(W_RegAddr) = (gm_ReadRegDWord_LE(W_RegAddr) | (DWORD)(DW_Set)));
}
#endif

//******************************************************************************
// USAGE:       return current system time
// INPUT:
// OUTPUT:      current time in ms since boot up
// GLOBALS:	    gDw_SystemTime
// USED_REGS: 	T2CON
//******************************************************************************
DWORD far gm_GetSystemTime(void)
{
	DWORD Dw_Temp = g_SystemTime.d;
	if (*((WORD*)(&g_SystemTime) + 1) != (WORD)(Dw_Temp>>16))
		Dw_Temp = g_SystemTime.d;	
	return Dw_Temp;
}

//******************************************************************************
// USAGE:       return lo word from current system time 
// INPUT:
// OUTPUT:      current time in ms since boot up
// GLOBALS:	    gDw_SystemTime
// USED_REGS: 	T2CON
//******************************************************************************
WORD far gm_W_GetSystemTime(void)
{
	return (*(WORD* const)&g_SystemTime);
}

//******************************************************************************
// USAGE:      Performs for given number of ms.
// INPUT:      None
// OUTPUT:     TRUE - mission is powered up
//             FALSE - mission is powered down
// GLOBALS:      None
// USED_REGS:   None
//******************************************************************************
void gm_Delay1ms(WORD ms)
{   
    WORD W_SavedTime = gm_W_GetSystemTime();
    while ((gm_W_GetSystemTime() - W_SavedTime) <= ms);    
}


//******************************************************************************
//
// FUNCTION:    interrupt SystemTimerInt(void)
// USAGE:       System timer interrupt service routine.
//				Keep track of general system timer values.
//				The timer2 timeout value is "1ms".
//
// INPUT:       None
// OUTPUT:      None
// GLOBALS:		gm_1msFunc
// USED_REGS:	None
//
//******************************************************************************
void interrupt isrSystemTimer(void)
{
	asm sti; // Allow other interrupt	

	// increment 40 bit systemtimer
	if ((++g_SystemTime.d) == 0)
		++g_SystemTime.b[4];

	CLEAR_PCB_REG_BITS(T2CON, MC);

	asm cli; // to avoid interrupt between next command and iret     
	WRITE_PCB_REG(EOI, TIMER2_VTYPE);
}

//******************************************************************************
//
// FUNCTION:    void Lpm_TimerInit(DWORD D_CpuClock)
// USAGE:       Initialization of system timer Control Registers and setup
//				Timer interrupt routine.  Use Mcu timer2 for system timer.
//
// INPUT:       D_CpuClock - CPU clock speed
// OUTPUT:      None
// GLOBALS:		g_SystemTime, gm_1msFunc
// USED_REGS: 	None
//
//******************************************************************************
void Lpm_TimerInit(DWORD D_CpuClock)
{
	g_SystemTime.d = 0; 
	g_SystemTime.b[4] = 0;

	WRITE_PCB_REG(T2CMPA, (WORD)(((D_CpuClock/2000) + 1) >> 1)); //Setup Timer2 to 1ms timeout
	WRITE_PCB_REG(T2CNT, 0);						//Clear counter
	WRITE_PCB_REG(T2CON, EN|INH|INT|CONT);			//Enable Timer2 with interrupt in continue mode
	SET_VECTOR(TIMER2_VECTOR, isrSystemTimer);		//Setup Timer2 Interrupt vector
	WRITE_PCB_REG(T0CON, 0);	//Disable timer0 interrupt
	WRITE_PCB_REG(T1CON, 0);	//Disable timer1 interrupt
	
	CLEAR_PCB_REG_BITS(IMASK, TMR);	//Enable Timers Iterrupts (IF flag still not set !)
}

//******************************************************************************
//
// FUNCTION:    void Lpm_TimerStart(BYTE B_Timer, WORD W_TimeOutInMS)
//
// USAGE:		Emulate behaviour of Lpm_TimerStartEx(), which cannot be used because
//				timer on low power CPU is not set up.
//
// INPUT:		W_TimeOutInMS
//
// OUTPUT:		None
//
// GLOBALS:		None
//
// USED_REGS:	None
//
//******************************************************************************
void Lpm_TimerStart(BYTE B_Timer, WORD W_TimeOutInMS)
{
	Lpm_timer[B_Timer].ms.d = gm_GetSystemTime() + W_TimeOutInMS;
}

//******************************************************************************
//
// FUNCTION:    BOOL Lpm_TimerTimeOut(BYTE B_Timer)
//
// USAGE:		Emulate behaviour of gm_TimerCheckEx(), which cannot be used because
//				timer on low power CPU is not set up.
//
// INPUT:      None
//
// OUTPUT:		TRUE - time out
//				FALSE - not time out yet
//
// GLOBALS:      None
//
// USED_REGS:   None
//
//******************************************************************************
BOOL Lpm_TimerTimeOut(BYTE B_Timer)
{   
	return (Lpm_timer[B_Timer].ms.d < gm_GetSystemTime());
}

//******************************************************************************
//
// FUNCTION:    void Lpm_Delay1ms(WORD W_TimeOutInMS)
//
// USAGE:		Delay 
//
// INPUT:		None
//
// OUTPUT:		TRUE - time out
//				FALSE - not time out yet
//
// GLOBALS:		None
//
// USED_REGS:	None
//
//******************************************************************************
void Lpm_Delay1ms(WORD W_TimeOutInMS)
{
   Lpm_TimerStart(DELAY_1ms_TMR, W_TimeOutInMS);
	while(1)
   {  
      if(Lpm_TimerTimeOut(DELAY_1ms_TMR))
         break;
   }
}

//******************************************************************************
//
// USAGE:		Initialize MCU
//
// INPUT:		None
// OUTPUT:		None
// GLOBALS:		None
// USED_REGS:	None
//
//******************************************************************************
void Lpm_McuInit(void)
{		
    //Set up ISR vectors and priorities
	SET_VECTOR(DIV_ERR_VECTOR, Lpm_isrDivideErrorException);
	SET_VECTOR(NMI_VECTOR, Lpm_isrNonMaskableInterrupt);
	SET_VECTOR(INTO_VECTOR, Lpm_isrInt0Overflow);
	SET_VECTOR(BOUNDS_VECTOR, Lpm_isrArrayBoundsException);
	SET_VECTOR(OPCODE_ERR_VECTOR, Lpm_isrUnusedOpcodeException);
	SET_VECTOR(ESC_VECTOR, Lpm_isrEscOpcodeException);
	SET_VECTOR(INT0_VECTOR, Lpm_IrqZeroManager);
	SET_VECTOR(INT1_VECTOR, Lpm_IrqOneManager);
	SET_VECTOR(INT2_VECTOR, Lpm_IrqTwoManager);
	SET_VECTOR(INT3_VECTOR, Lpm_IrqThreeManager);
	SET_VECTOR(INT4_VECTOR, Lpm_IrqFourManager);
	SET_VECTOR(INT5_VECTOR, Lpm_IrqFiveManager);


#ifdef UART_DEBUG_SUPPORT	
    SET_VECTOR(UART0_VECTOR, Lpm_UartIsr);
#endif

	WRITE_PCB_REG(TCUCON, 3);		// Set system timers priority
	WRITE_PCB_REG(I0CON, 4|BIT4);	// Set IRQ0 priority, level-triggered
	WRITE_PCB_REG(I1CON, 5|BIT4);	// Set IRQ1 priority, level-triggered
	
	WRITE_PCB_REG(I2CON, 5|BIT4);	// Set IRQ2 priority, level-triggered
	WRITE_PCB_REG(I3CON, 5|BIT4);	// Set IRQ3 priority, level-triggered
	WRITE_PCB_REG(I4CON, 5|BIT4);	// Set IRQ4 priority, level-triggered
	WRITE_PCB_REG(I5CON, 5|BIT4);	// Set IRQ5 priority, level-triggered

	WRITE_PCB_REG(SP0CON, 3);		// Set Uart0 priority
}


//******************************************************************************

