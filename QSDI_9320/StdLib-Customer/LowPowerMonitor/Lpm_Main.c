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
// MODULE:      Lpm_Main.c
//
// USAGE:                     
//
//******************************************************************************
#include <embedded.h>

//******************************************************************************
//
//  I N C L U D E    F I L E S
//
//******************************************************************************
#include ".\LowPowerMonitor\Lpm_All.h"

/*
#ifdef IR_SUPPORT

#include "..\Remote\Faroudja\DCDI-01.h" //default remote, can be changed during LPM code upload.

extern ROM gmt_IrProtocolExt IrProtocol; //should be placed here to provide allocation 

    //in the very begin of code segment
#if (sizeof(gmt_IrProtocolExt) & 1)
    ROM BYTE Dummy = 0;
#endif

#endif 
*/
ROM TConfiguration Configuration = {1};

#define KEY_POWER 12

#define NUM_OF_RX_QUEUES         4// 2
#define QUEUE_LENGTH             10//15 //15bytes in every queue

static gmt_QUEUE Queue[NUM_OF_RX_QUEUES];
static BYTE B_Buffer[NUM_OF_RX_QUEUES * (QUEUE_LENGTH+3)]; // add 3 bytes to avoid queue buffer overflow for gm_MessageInIsr() write data with DWORD size

BOOL powerUpRequest = FALSE;

BYTE RcOscTrim = 3;  //3bits, value from 0 - 7
WORD MeasRcOscVal = 0;
DWORD LpmRcOscClk = SYSTEM_TCLK_FREQ;
DWORD LpmOcmClk = SYSTEM_TCLK_FREQ;
BYTE PBusReqLock = FALSE;

#ifdef IR_SUPPORT
static gmt_Ir IrBuffer;
BYTE IrPowerKey;
#endif

#if LPM_FORCE_POWER_DOWN
extern BYTE IsForceDcOff;
#endif

#ifdef TOUCHPAD_SUPPORT
extern BYTE TouchKeyIntFlag;
#endif

gmt_HandlerPrototype ROM aPf_HandlersRoutines[LpmHdrID_END] =
{
   Lpm_SystemDebug,

   #ifdef I2C_SLAVE_SUPPORT
   Lpm_I2CSlaveHandler,
   #endif	

   #ifdef DDCCI_SUPPORT
   Lpm_DdcciCmdHandler,
   #endif	

   #ifdef CEC_SUPPORT
   Lpm_CecHandler,
   #endif

   #ifdef TCLK_TIMER_SUPPORT			
   Lpm_TclkTimerHandler,
   #endif	

   #ifdef IR_SUPPORT
   Lpm_IrHandler,        
   #endif

   #ifdef KEYPAD_SUPPORT			
   Lpm_KeypadHandler,
   #endif

   #ifdef TOUCHPAD_SUPPORT
   Lpm_TouchPadHandler,
   #endif

   #ifdef SIGNALING_DETECT
   Lpm_SignalDetectHandler,
   #endif

   #ifdef CABLE_DETECT_SUPPORT
   Lpm_CableDetectHandler,
   #endif
};


BOOL aB_HandlersFlags[LpmHdrID_END] =
{
   TRUE,// 	Lpm_SystemDebug,			  //Always On for debug stage

   #ifdef I2C_SLAVE_SUPPORT
   FALSE,
   #endif

   #ifdef DDCCI_SUPPORT
   FALSE,
   #endif

   #ifdef CEC_SUPPORT
   TRUE,//			Lpm_CecHandler,
   #endif

   #ifdef TCLK_TIMER_SUPPORT			
   TRUE,//		Lpm_TclkTimerHandler,
   #endif	

   #ifdef IR_SUPPORT
   TRUE,//		Lpm_IrHandler,        		//Always On
   #endif

   #ifdef KEYPAD_SUPPORT			
   TRUE,//		Lpm_KeypadHandler,		//Always On
   #endif

   #ifdef TOUCHPAD_SUPPORT
   TRUE,//		Lpm_TouchPadHandler
   #endif

   #ifdef SIGNALING_DETECT
   FALSE, //		Lpm_SignalDetectHandler
   #endif

   #ifdef CABLE_DETECT_SUPPORT
   TRUE //		Lpm_CableDetectHandler		//Always On
   #endif
};


void Lpm_MainHandlersTurn(gmt_LpmHdrId B_HdrId, BOOL B_OnOff)
{
   aB_HandlersFlags[B_HdrId] = B_OnOff;
}


//******************************************************************************
//
// FUNCTION:	RET_STAT Lpm_InterruptEnable (ISR_NAME W_IsrName, 
//											WORD W_IntMaskRegVal)
//
// USAGE:       According to the bit value of W_IntMaskRegVal to enable the
//				service in the specified hardware interrupt (W_IsrName).
//
// INPUT:		W_IsrName 		  	- Enumerate value of ISR name
//				W_IntMaskRegVal 	- Select ISR service to enable
//
// OUTPUT:		OK 			- Command accepted.
//				ERR_PARAM 	- Invalid input parameters.
//
// GLOBALS:		None
//
// USED_REGS:	LPM_MICRO_CONTROL	(R/W)
//				LPM_IRQ_STATUS		(W)
//				TCLK_TIMER_CTRL		(R/W)
//				TCLK_TIMER_STATUS	(W)
//
//******************************************************************************
gmt_RET_STAT Lpm_InterruptEnable(gmt_ISR_SOURCE W_IsrSource)
{
   // 1) Cleared interrupt status register by written with a 1
   // 2) Set Interrupt Masks Host Register Bit to enable interrupt

   gmt_RET_STAT status = OK;

   asm cli;
   switch (W_IsrSource)
   {
   	#ifdef I2C_SLAVE_SUPPORT
   	case gmd_DDC1:			
   			gm_SetRegBitsWord(LPM_IRQ_CONTROL, LPM_DDC1_IRQ_MASK); 
   			gm_SetRegBitsWord(LPM_MIS_CSM_IMASK, LPM_MIS_DDC1_IRQ_MSK);
   		gm_WriteRegWord(LPM_IRQ_STATUS, LPM_DDC1_IRQ);         
   			gm_WriteRegWord(LPM_MIS_CSM_ISTATUS, 0xFFFF);   //clear all LPM_MIS IRQ status
   		break;	
   	case gmd_DDC2:			
   		gm_SetRegBitsWord(LPM_IRQ_CONTROL, LPM_DDC2_IRQ_MASK); 
   			gm_SetRegBitsWord(LPM_MIS_CSM_IMASK, LPM_MIS_DDC2_IRQ_MSK);
   		gm_WriteRegWord(LPM_IRQ_STATUS, LPM_DDC2_IRQ);         
   			gm_WriteRegWord(LPM_MIS_CSM_ISTATUS, 0xFFFF);   //clear all LPM_MIS IRQ status         
   		break;
   	case gmd_DDC3:			
   		gm_SetRegBitsWord(LPM_IRQ_CONTROL, LPM_DDC3_IRQ_MASK);          
   			gm_SetRegBitsWord(LPM_MIS_CSM_IMASK, LPM_MIS_DDC3_IRQ_MSK);
   		gm_WriteRegWord(LPM_IRQ_STATUS, LPM_DDC3_IRQ);         
   			gm_WriteRegWord(LPM_MIS_CSM_ISTATUS, 0xFFFF);   //clear all LPM_MIS IRQ status         
   		break;	
   	case gmd_DDC4:			
   		gm_SetRegBitsWord(LPM_IRQ_CONTROL, LPM_DDC4_IRQ_MASK);       
   			gm_SetRegBitsWord(LPM_MIS_CSM_IMASK, LPM_MIS_DDC4_IRQ_MSK);
   		gm_WriteRegWord(LPM_IRQ_STATUS, LPM_DDC4_IRQ);            
   			gm_WriteRegWord(LPM_MIS_CSM_ISTATUS, 0xFFFF);   //clear all LPM_MIS IRQ status         
   		break;	
   	#endif
   	case gmd_AUTOWAKE:			
   		gm_SetRegBitsWord(LPM_IRQ_CONTROL, LPM_AUTOWAKE_IRQ_MASK); 
   		gm_WriteRegWord(LPM_IRQ_STATUS, LPM_AUTOWAKE_IRQ);         
   		break;
   	#ifdef SIGNALING_DETECT   
   	case gmd_ASYNCDET:			
   		gm_SetRegBitsWord(LPM_IRQ_CONTROL, LPM_ASYNCDET_IRQ_MASK); 
   		gm_WriteRegWord(LPM_IRQ_STATUS, LPM_ASYNCDET_IRQ);         
   		break;
   	case gmd_TMDSCLK:			
   		gm_SetRegBitsWord(LPM_IRQ_CONTROL, LPM_TMDSCK_IRQ_MASK); 
   		gm_WriteRegWord(LPM_IRQ_STATUS, LPM_TMDSCK_IRQ);         
   		break;		
   	#endif
   	#ifdef I2C_MASTER_SUPPORT
   	case gmd_I2CM:			
   		gm_SetRegBitsWord(LPM_IRQ_CONTROL, BIT7);//LPM_I2CM_IRQ_MASK
   		gm_WriteRegWord(LPM_IRQ_STATUS, BIT7);//LPM_I2CM_IRQ         
   		break;		
   	#endif
   	#ifdef BROWN_OUT_SUPPORT
   	case gmd_SWDT:			
   		gm_SetRegBitsWord(LPM_IRQ_CONTROL, LPM_SWDT_IRQ_MASK); 
   		gm_WriteRegWord(LPM_IRQ_STATUS, LPM_SWDT_IRQ);         
   		break;			
   	#endif
   	#ifdef IR_SUPPORT
   	case gmd_IR:
   		gm_SetRegBitsWord(LPM_IRQ_CONTROL, LPM_IR_DECODER_MASK);	
   		gm_WriteRegWord(LPM_IRQ_STATUS, LPM_IR_DECODER_STATUS);         
   		break;
   	#endif
   	#ifdef CEC_SUPPORT
   	case gmd_CEC:			
   		gm_SetRegBitsWord(LPM_IRQ_CONTROL, LPM_CEC_DECODER_MASK); 
   		gm_WriteRegWord(LPM_IRQ_STATUS, LPM_CEC_DECODER_STATUS);         
   		break;
   	#endif
   	case gmd_MISSION_MAIL_OUT:
   		gm_SetRegBitsWord(LPM_IRQ_CONTROL, LPM_MISSION_MAIL_OUT_MASK); 
   		gm_WriteRegWord(LPM_IRQ_STATUS, MISSION_MAIL_OUT_READY);         
   		break;
   	case gmd_CSM_HPD_EDPD:
   		gm_SetRegBitsWord(LPM_IRQ_CONTROL, LPM_CSM_HPD_EDPD_IRQ_MASK); 			
   		gm_WriteRegWord(LPM_IRQ_STATUS, LPM_CSM_HPD_EDPD_IRQ_STATUS);         
   		break;	
   	case gmd_TX_HPD:
   		gm_SetRegBitsWord(LPM_IRQ_CONTROL, LPM_TX_HPD_IRQ_MASK); 			
   		gm_WriteRegWord(LPM_IRQ_STATUS, LPM_TX_HPD_IRQ_STATUS);         
   		break;		
   	case gmd_EXTINT:
   		gm_SetRegBitsWord(LPM_IRQ_CONTROL, LPM_EXTINT_MASK); 			
   		gm_WriteRegWord(LPM_IRQ_STATUS, LPM_EXTINT_STATUS);         
   		break;				
   	#ifdef TCLK_TIMER_SUPPORT
   	case gmd_TCLK_TIMER:			
   		gm_SetRegBitsWord(LPM_TCLK_TIMER_CTRL, 1 << TCLK_TIMER_OCM_INT_SEL_SHIFT);
   		gm_SetRegBitsWord(LPM_TCLK_TIMER_CTRL, TCLK_TIMER_THRESH_INT_EN);\
   		gm_WriteRegWord(LPM_TCLK_TIMER_STATUS, TCLK_TIMER_THRESH_EVENT);            
   		break;
   	#endif		
   	default:
   		status = ERR_PARAM;
   		break;
   }
   asm sti;
   return status;
}

//******************************************************************************
//
// FUNCTION:	RET_STAT Lpm_InterruptDisable (ISR_NAME W_IsrName, 
//											WORD W_IntMaskRegVal)
//
// USAGE:       According to the bit value of W_IntMaskRegVal to disable the
//				service in the specified hardware interrupt (W_IsrName).
//
// INPUT:		W_IsrName 		  	- Enumerate value of ISR name
//				W_IntMaskRegVal 	- Select ISR service to disable
//
// OUTPUT:		RET_STAT: OK 		- Command accepted.
//				ERR_PARAM 			- Invalid input parameters.
//
// GLOBALS:		None
//
// USED_REGS:	LPM_MICRO_CONTROL	(R/W)
//				LPM_IRQ_STATUS		(W)
//				TCLK_TIMER_CTRL		(R/W)
//				TCLK_TIMER_STATUS	(W)
//
//******************************************************************************
gmt_RET_STAT Lpm_InterruptDisable (gmt_ISR_SOURCE W_IsrSource)
{
   // 1) Clear Interrupt Masks Host Register Bit to disable interrupt
   // 2) Cleared interrupt status register by written with a 1
   gmt_RET_STAT status = OK;

   asm cli;
   switch (W_IsrSource)
   {
   	#ifdef I2C_SLAVE_SUPPORT
   	case gmd_DDC1:			
   		gm_ClearRegBitsWord(LPM_IRQ_CONTROL, LPM_DDC1_IRQ_MASK); 
   		gm_WriteRegWord(LPM_IRQ_STATUS, LPM_DDC1_IRQ);
   		break;	
   	case gmd_DDC2:			
   		gm_ClearRegBitsWord(LPM_IRQ_CONTROL, LPM_DDC2_IRQ_MASK); 
   		gm_WriteRegWord(LPM_IRQ_STATUS, LPM_DDC2_IRQ);
   		break;
   	case gmd_DDC3:			
   		gm_ClearRegBitsWord(LPM_IRQ_CONTROL, LPM_DDC3_IRQ_MASK); 
   		gm_WriteRegWord(LPM_IRQ_STATUS, LPM_DDC3_IRQ);
   		break;	
   	case gmd_DDC4:			
   		gm_ClearRegBitsWord(LPM_IRQ_CONTROL, LPM_DDC4_IRQ_MASK); 
   		gm_WriteRegWord(LPM_IRQ_STATUS, LPM_DDC4_IRQ);
   		break;	
   	#endif
   	case gmd_AUTOWAKE:			
   		gm_ClearRegBitsWord(LPM_IRQ_CONTROL, LPM_AUTOWAKE_IRQ_MASK);
   		gm_WriteRegWord(LPM_IRQ_STATUS, LPM_AUTOWAKE_IRQ); 
   		break;
   	#ifdef SIGNALING_DETECT
   	case gmd_ASYNCDET:			
   		gm_ClearRegBitsWord(LPM_IRQ_CONTROL, LPM_ASYNCDET_IRQ_MASK); 
   		gm_WriteRegWord(LPM_IRQ_STATUS, LPM_ASYNCDET_IRQ);
   		break;
   	case gmd_TMDSCLK:			
   		gm_ClearRegBitsWord(LPM_IRQ_CONTROL, LPM_TMDSCK_IRQ_MASK); 
   		gm_WriteRegWord(LPM_IRQ_STATUS, LPM_TMDSCK_IRQ);
   		break;		
   	#endif
   	#ifdef I2C_MASTER_SUPPORT
   	case gmd_I2CM:			
   		gm_ClearRegBitsWord(LPM_IRQ_CONTROL, BIT7);//LPM_I2CM_IRQ_MASK
   		gm_WriteRegWord(LPM_IRQ_STATUS, BIT7);//LPM_I2CM_IRQ
   		break;		
   	#endif
   	#ifdef BROWN_OUT_SUPPORT
   	case gmd_SWDT:			
   		gm_ClearRegBitsWord(LPM_IRQ_CONTROL, LPM_SWDT_IRQ_MASK); 
   		gm_WriteRegWord(LPM_IRQ_STATUS, LPM_SWDT_IRQ);
   		break;			
   	#endif
   	#ifdef IR_SUPPORT
   	case gmd_IR:
   		gm_ClearRegBitsWord(LPM_IRQ_CONTROL, LPM_IR_DECODER_MASK);	
   		gm_WriteRegWord(LPM_IRQ_STATUS, LPM_IR_DECODER_STATUS);
   		break;
   	#endif
   	#ifdef CEC_SUPPORT
   	case gmd_CEC:			
   		gm_ClearRegBitsWord(LPM_IRQ_CONTROL, LPM_CEC_DECODER_MASK);
   		gm_WriteRegWord(LPM_IRQ_STATUS, LPM_CEC_DECODER_STATUS); 
   		break;
   	#endif
   	case gmd_MISSION_MAIL_OUT:
   		gm_ClearRegBitsWord(LPM_IRQ_CONTROL, LPM_MISSION_MAIL_OUT_MASK); 
   		gm_WriteRegWord(LPM_IRQ_STATUS, MISSION_MAIL_OUT_READY);			
   		break;
   	case gmd_CSM_HPD_EDPD:
   		gm_ClearRegBitsWord(LPM_IRQ_CONTROL, LPM_CSM_HPD_EDPD_IRQ_MASK); 
   		gm_WriteRegWord(LPM_IRQ_STATUS, LPM_CSM_HPD_EDPD_IRQ_STATUS);			
   		break;	
   	case gmd_TX_HPD:
   		gm_ClearRegBitsWord(LPM_IRQ_CONTROL, LPM_TX_HPD_IRQ_MASK); 
   		gm_WriteRegWord(LPM_IRQ_STATUS, LPM_TX_HPD_IRQ_STATUS);			
   		break;		
   	case gmd_EXTINT:
   		gm_ClearRegBitsWord(LPM_IRQ_CONTROL, LPM_EXTINT_MASK); 
   		gm_WriteRegWord(LPM_IRQ_STATUS, LPM_EXTINT_STATUS);			
   		break;			
   	#ifdef TCLK_TIMER_SUPPORT
   	case gmd_TCLK_TIMER:			
   		gm_ClearRegBitsWord(LPM_TCLK_TIMER_CTRL, TCLK_TIMER_THRESH_INT_EN);
   		gm_WriteRegWord(LPM_TCLK_TIMER_STATUS, TCLK_TIMER_THRESH_EVENT);
   		break;			
   	#endif
   	default:
   		status = ERR_PARAM;
   		break;
   }
   asm sti;
   return status;
}

//******************************************************************************
//
// FUNCTION:	static void far interrupt Lpm_IrqZeroManager (void)
// USAGE:		IRQ0 low level service routine.
// INPUT:		None
// OUTPUT:		None
// GLOBALS:		None
// USED_REGS:	LPM_IRQ_STATUS		(W)
//				TCLK_TIMER_STATUS	(W)
//
//******************************************************************************
void interrupt Lpm_IrqZeroManager(void)//derived from IrqZeroManager()
{
   asm sti; // Allow other interrupt

   if (gm_ReadRegWord(LPM_IRQ_STATUS) & LPM_IR_DECODER_STATUS)//IRDecoder
   {
   	#ifdef IR_SUPPORT					
   	Lpm_IrIsr();
   	#endif
   	
   	gm_WriteRegWord(LPM_IRQ_STATUS, LPM_IR_DECODER_STATUS);
   }

   if (gm_ReadRegWord(LPM_IRQ_STATUS) & LPM_CEC_DECODER_STATUS)//CEC
   {
   	#ifdef CEC_SUPPORT
   	gm_CecIsr();
   	#endif
   	gm_WriteRegWord(LPM_IRQ_STATUS, LPM_CEC_DECODER_STATUS);
   }

   asm cli; //to avoid interrupt between next command and iret
   WRITE_PCB_REG(EOI, INT0_VTYPE);	 // Interrupt exit
}

//******************************************************************************
//
// FUNCTION:	static void far interrupt Lpm_IrqOneManager (void)
// USAGE:		IRQ1 low level service routine.
// INPUT:		None
// OUTPUT:		None
// GLOBALS:		None
// USED_REGS:	LPM_IRQ_STATUS		(W)
//				TCLK_TIMER_STATUS	(W)
//
//******************************************************************************
void interrupt Lpm_IrqOneManager(void)//derived from IrqOneManager()
{
   asm sti; // Allow other interrupt

   if (gm_ReadRegWord(LPM_IRQ_STATUS) & MISSION_MAIL_OUT_READY)//This bit will clear when LPM_MICRO_MAIL_IN has been read.
   {
   	gm_MessageInIsr();
   	//gm_WriteRegWord(LPM_IRQ_STATUS, MISSION_MAIL_OUT_READY);//auto clear after reading
   }

   #ifdef TCLK_TIMER_SUPPORT		
   if (gm_ReadRegWord(LPM_TCLK_TIMER_STATUS) & TCLK_TIMER_THRESH_EVENT)
   {
   	Lpm_TclkTimerIsr();
   	gm_WriteRegWord(LPM_TCLK_TIMER_STATUS, TCLK_TIMER_THRESH_EVENT);
   }
   gm_WriteRegWord(LPM_TCLK_TIMER_STATUS, TCLK_TIMER_EVENT_ERR); //just clear this bit without doing anything else
   #endif

   asm cli; //to avoid interrupt between next command and iret
   WRITE_PCB_REG(EOI, INT1_VTYPE);	 // Interrupt exit
}


//******************************************************************************
//
// FUNCTION:	static void far interrupt Lpm_IrqTwoManager (void)
// USAGE:		IRQ2 low level service routine.
// INPUT:		None
// OUTPUT:		None
// GLOBALS:		None
// USED_REGS:	LPM_IRQ_STATUS_0		(W)
//
//******************************************************************************
void interrupt Lpm_IrqTwoManager(void)//derived from IrqTwoManager()
{
   asm sti; // Allow other interrupt

   #ifdef DDCCI_SUPPORT
   if (gm_ReadRegWord(LPM_IRQ_STATUS) & LPM_DDC1_IRQ)//DDC1
   {			
   	Lpm_I2CSlaveIsr(I2C_SLAVE1_CH);
   	gm_WriteRegWord(LPM_IRQ_STATUS, LPM_DDC1_IRQ);
   	gm_WriteRegWord(LPM_MIS_CSM_ISTATUS, LPM_MIS_DDC1_IRQ_STATUS);
   }

   if (gm_ReadRegWord(LPM_IRQ_STATUS) & LPM_DDC2_IRQ)//DDC2
   {			
   	//gm_Print("LPM_MIS_CSM_ISTATUS=0x%x",gm_ReadRegWord(LPM_MIS_CSM_ISTATUS));
   	//gm_Print("LPM_IRQ_STATUS=0x%x",gm_ReadRegWord(LPM_IRQ_STATUS));
   	Lpm_I2CSlaveIsr(I2C_SLAVE2_CH);
   	gm_WriteRegWord(LPM_IRQ_STATUS, LPM_DDC2_IRQ);
   	gm_WriteRegWord(LPM_MIS_CSM_ISTATUS, LPM_MIS_DDC2_IRQ_STATUS);
   }

   if (gm_ReadRegWord(LPM_IRQ_STATUS) & LPM_DDC3_IRQ)//DDC3
   {			
   	Lpm_I2CSlaveIsr(I2C_SLAVE3_CH);
   	gm_WriteRegWord(LPM_IRQ_STATUS, LPM_DDC3_IRQ);
   	gm_WriteRegWord(LPM_MIS_CSM_ISTATUS, LPM_MIS_DDC3_IRQ_STATUS);
   }

   if (gm_ReadRegWord(LPM_IRQ_STATUS) & LPM_DDC4_IRQ)//DDC4
   {			
   	Lpm_I2CSlaveIsr(I2C_SLAVE4_CH);
   	gm_WriteRegWord(LPM_IRQ_STATUS, LPM_DDC4_IRQ);
   	gm_WriteRegWord(LPM_MIS_CSM_ISTATUS, LPM_MIS_DDC4_IRQ_STATUS);
   }
   #endif
   asm cli; //to avoid interrupt between next command and iret
   WRITE_PCB_REG(EOI, INT2_VTYPE);	 // Interrupt exit
}

//******************************************************************************
//
// FUNCTION:	static void far interrupt Lpm_IrqThreeManager (void)
// USAGE:		IRQ3 low level service routine.
// INPUT:		None
// OUTPUT:		None
// GLOBALS:		None
// USED_REGS:	LPM_IRQ_STATUS_0		(W)
//
//******************************************************************************
void interrupt Lpm_IrqThreeManager(void)
{
   asm sti; // Allow other interrupt

   if ((gm_ReadRegWord(LPM_IRQ_STATUS)&LPM_AUTOWAKE_IRQ) & (gm_ReadRegWord(LPM_IRQ_CONTROL)&LPM_AUTOWAKE_IRQ_MASK))
   {
      gm_WriteRegWord(LPM_IRQ_STATUS, LPM_AUTOWAKE_IRQ);
      if(Lpm_PowerModeGetState() == LPM_PowerOffMode)
      {
         #ifdef AUX_RX_SUPPORT
         DWORD DW_AuxC0Status;
         #ifdef AUX_RX_C2_SUPPORT
         DWORD DW_AuxC2Status;
         #endif

         DW_AuxC0Status = gm_ReadRegDWord(LPM_C0_AUX_IRQ_STATUS);// & gm_ReadRegDWord(LPM_C0_AUX_IRQ_CTRL);
         if(DW_AuxC0Status & (DPRX_AUX2OCM_REQ_RECEPTN_STS|DPRX_AUX2I2C_REQ_RECEPTN_STS|DPRX_AUX_DEF_NSTOP_STS))
         {
            gm_WriteRegWord(LPM_CLOCK_CTRL,(2<<LPM_OCMCLK_SEL_SHIFT)|(TCLK_DIV_0<<LPM_OCMCLK_DIV_SHIFT));
            #if 1
            gm_WriteRegDWord(LPM_C0_AUX_IRQ_STATUS, DW_AuxC0Status|DPRX_AUX_DEF_NSTOP_STS|DPRX_AUX_AUTO_WAKE_STS);//Clear AUX status 
            #else
            gm_SetRegBitsDWord(LPM_C0_AUX_IRQ_STATUS, DPRX_AUX_DEF_NSTOP_STS);
            if(DW_AuxC0Status & DPRX_AUX2I2C_REQ_RECEPTN_STS)
               gm_SetRegBitsDWord(LPM_C0_AUX_IRQ_STATUS, DPRX_AUX2I2C_REQ_RECEPTN_STS|DPRX_AUX_AUTO_WAKE_STS); 
            if(DW_AuxC0Status & DPRX_AUX2OCM_REQ_RECEPTN_STS)
               gm_SetRegBitsDWord(LPM_C0_AUX_IRQ_STATUS, DPRX_AUX2OCM_REQ_RECEPTN_STS|DPRX_AUX_AUTO_WAKE_STS); 
            #endif         
            Lpm_AuxRxRequestISR(AUX_CH_C0);
            gm_SetRegBitsWord(LPM_C0_AUX_STROBE, DPRX_AUX_AUTO_WAKEUP_EN);
            gm_WriteRegWord(LPM_CLOCK_CTRL,(2<<LPM_OCMCLK_SEL_SHIFT)|(TCLK_DIV_2<<LPM_OCMCLK_DIV_SHIFT));
            //gm_Print("AUX C0 Trans",0);
         }
         #ifdef AUX_RX_C2_SUPPORT
         DW_AuxC2Status = gm_ReadRegDWord(LPM_C2_AUX_IRQ_STATUS);// & gm_ReadRegDWord(LPM_C2_AUX_IRQ_CTRL);
         if(DW_AuxC2Status & (DPRX_AUX2OCM_REQ_RECEPTN_STS|DPRX_AUX2I2C_REQ_RECEPTN_STS|DPRX_AUX_DEF_NSTOP_STS))
         {
            gm_WriteRegWord(LPM_CLOCK_CTRL,(2<<LPM_OCMCLK_SEL_SHIFT)|(TCLK_DIV_0<<LPM_OCMCLK_DIV_SHIFT));
            #if 1
            gm_WriteRegDWord(LPM_C2_AUX_IRQ_STATUS, DW_AuxC2Status|DPRX_AUX_DEF_NSTOP_STS|DPRX_AUX_AUTO_WAKE_STS);//Clear AUX status 
            #else
            gm_SetRegBitsDWord(LPM_C2_AUX_IRQ_STATUS, DPRX_AUX_DEF_NSTOP_STS);            
            if(DW_AuxC2Status & DPRX_AUX2I2C_REQ_RECEPTN_STS)
               gm_SetRegBitsDWord(LPM_C2_AUX_IRQ_STATUS, DPRX_AUX2I2C_REQ_RECEPTN_STS|DPRX_AUX_AUTO_WAKE_STS); 
            if(DW_AuxC2Status & DPRX_AUX2OCM_REQ_RECEPTN_STS)
               gm_SetRegBitsDWord(LPM_C2_AUX_IRQ_STATUS, DPRX_AUX2OCM_REQ_RECEPTN_STS|DPRX_AUX_AUTO_WAKE_STS);          
            #endif         
            Lpm_AuxRxRequestISR(AUX_CH_C2);
            gm_SetRegBitsWord(LPM_C2_AUX_STROBE, DPRX_AUX_AUTO_WAKEUP_EN);
            gm_WriteRegWord(LPM_CLOCK_CTRL,(2<<LPM_OCMCLK_SEL_SHIFT)|(TCLK_DIV_2<<LPM_OCMCLK_DIV_SHIFT));
         }	
         #endif    
         #endif
      }
      else  //Signaling wakeup for LPM_LowPowerMode
      {
         #ifdef TMDS_CLKDET_BY_AUTOWAKE_IRQ      
         if (gm_ReadRegWord(LPM_AUTOWAKE_STATUS) & TMDS_CKDET_AUTOWAKE_STATUS)
         {
            Lpm_AutoWakeISR(Port_HDMI);//including DVI
            gm_WriteRegWord(LPM_AUTOWAKE_STATUS, TMDS_CKDET_AUTOWAKE_STATUS);
            //gm_Print("TMDS_CKDET_AUTOWAKE_STATUS",0);         
         } 
         #endif   //#ifdef TMDS_CLKDET_BY_AUTOWAKE_IRQ

         #ifdef AUX_RX_SUPPORT
         //Occurs as reaching the number (AUX_WAKE_UP_COUNT) of DPCD address (AUX_AUTO_WAKE_ADDR) transitions
         if(gm_ReadRegDWord(LPM_C0_AUX_IRQ_STATUS) & DPRX_AUX_AUTO_WAKE_STS)
         {          
            if(Lpm_IsDPRxCablePlugged(DPRX_C0)    //DP cable detected
               #ifdef CHECK_AUX_SIG_WITH_DP_PWR
               && Lpm_IsDPRxSrcPwrValid(DPRX_C0)  //source power detected
               #endif
               )
            {
               Lpm_ForceDpHpdLow(); //Avoid DPTx continue LT or Aux trans.         
               Lpm_AutoWakeISR(Port_DP_A);
               //gm_Print("DPRX_AUX_AUTO_WAKE_STS",0);
               //gm_Print("DP source power & cable plug detected!",0);         
            } 
            else
            {
               gm_SetRegBitsWord(LPM_C0_AUX_STROBE, DPRX_AUX_AUTO_WAKEUP_EN);
            }
            gm_WriteRegDWord(LPM_C0_AUX_IRQ_STATUS, DPRX_AUX_AUTO_WAKE_STS);         
         }
         
         #ifdef AUX_RX_C2_SUPPORT
         if(gm_ReadRegDWord(LPM_C2_AUX_IRQ_STATUS) & DPRX_AUX_AUTO_WAKE_STS)
         {  
            if(Lpm_IsDPRxCablePlugged(DPRX_C2)    //DP cable detected
               #ifdef CHECK_AUX_SIG_WITH_DP_PWR
               && Lpm_IsDPRxSrcPwrValid(DPRX_C2)  //source power detected
               #endif
               )
            {
               Lpm_ForceDpHpdLow(); //Avoid DPTx continue LT or Aux trans.
               Lpm_AutoWakeISR(Port_DP_B);
               //gm_Print("DPRX_AUX_AUTO_WAKE_STS",0);
               //gm_Print("DP source power & cable plug detected!",0);         
            } 
            else
            {
               gm_SetRegBitsWord(LPM_C2_AUX_STROBE, DPRX_AUX_AUTO_WAKEUP_EN);
            }
            gm_WriteRegDWord(LPM_C2_AUX_IRQ_STATUS, DPRX_AUX_AUTO_WAKE_STS);         
         }    
         #endif   //#ifdef AUX_RX_C2_SUPPORT
         #endif
      }
      //gm_WriteRegWord(LPM_IRQ_STATUS, LPM_AUTOWAKE_IRQ);
      //gm_Print("AUTOWAKE IRQ",0);
   }  

   #ifdef TMDS_CLKDET_BY_AUTOWAKE_IRQ
   if ((gm_ReadRegWord(LPM_IRQ_STATUS)&LPM_TMDSCK_IRQ) & (gm_ReadRegWord(LPM_IRQ_CONTROL)&LPM_TMDSCK_IRQ_MASK))
   {       
      Lpm_AutoWakeISR(Port_HDMI);//including DVI
      Lpm_InterruptDisable(gmd_TMDSCLK);
      //gm_WriteRegWord(LPM_IRQ_STATUS, LPM_TMDSCK_IRQ);            
      //gm_Print("LPM_TMDSCK_IRQ",0);      
   }
   #endif

   asm cli; //to avoid interrupt between next command and iret
   WRITE_PCB_REG(EOI, INT3_VTYPE);	 // Interrupt exit
}

//******************************************************************************
//
// FUNCTION:	static void far interrupt Lpm_IrqFourManager (void)
// USAGE:		IRQ4 low level service routine.
// INPUT:		None
// OUTPUT:		None
// GLOBALS:		None
// USED_REGS:	LPM_IRQ_STATUS_0		(W)
//
//******************************************************************************
void interrupt Lpm_IrqFourManager(void)//derived from IrqFourManager()
{
   asm sti; // Allow other interrupt
   #ifdef I2C_MASTER_SUPPORT
   if (gm_ReadRegWord(LPM_IRQ_STATUS) & BIT7)//LPM_I2CM_IRQ
   {			
   	gm_WriteRegWord(LPM_IRQ_STATUS, BIT7);//LPM_I2CM_IRQ
   }
   #endif

   #ifdef BROWN_OUT_SUPPORT
   if (gm_ReadRegWord(LPM_IRQ_STATUS) & LPM_SWDT_IRQ)//SWDT
   {			
   	gm_WriteRegWord(LPM_IRQ_STATUS, LPM_SWDT_IRQ);
   }
   #endif	
   asm cli; //to avoid interrupt between next command and iret
   WRITE_PCB_REG(EOI, INT4_VTYPE);	 // Interrupt exit
}

//******************************************************************************
//
// FUNCTION:	static void far interrupt Lpm_IrqFiveManager (void)
// USAGE:		IRQ5 low level service routine.
// INPUT:		None
// OUTPUT:		None
// GLOBALS:		None
// USED_REGS:	LPM_IRQ_STATUS_0		(W)
//
//******************************************************************************
void interrupt Lpm_IrqFiveManager(void)//derived from IrqFourManager()
{
   asm sti; // Allow other interrupt
   if (gm_ReadRegWord(LPM_IRQ_STATUS) & LPM_CSM_HPD_EDPD_IRQ_STATUS)//CSM_HPD_EDPD
   {			
   	gm_WriteRegWord(LPM_IRQ_STATUS, LPM_CSM_HPD_EDPD_IRQ_STATUS);
   }

   if (gm_ReadRegWord(LPM_IRQ_STATUS) & LPM_TX_HPD_IRQ_STATUS)//TX_HPD
   {			
   	gm_WriteRegWord(LPM_IRQ_STATUS, LPM_TX_HPD_IRQ_STATUS);
   }	

   #ifdef TOUCHPAD_SUPPORT
   if (gm_ReadRegWord(LPM_IRQ_STATUS) & LPM_EXTINT_STATUS)//EXTINT
   {			
   	TouchKeyIntFlag = 1;
   	gm_WriteRegWord(LPM_IRQ_STATUS, LPM_EXTINT_STATUS);
   }		
   #endif	

   asm cli; //to avoid interrupt between next command and iret
   WRITE_PCB_REG(EOI, INT5_VTYPE);	 // Interrupt exit
}


//***************************************************************
// USAGE:       ISR Core exceptions handlers
//
// INPUT:		None
// OUTPUT:		None
// GLOBALS:		None
// USED_REGS:	None
//
//***************************************************************
void interrupt Lpm_isrDivideErrorException(void)
{
//	gm_LogPut(DIVIDE_ERR_EXCEPTION);//commented for simplicity sake
	WRITE_PCB_REG(EOI, DIV_ERR_VTYPE);  // Clear interrupt
}

void interrupt Lpm_isrNonMaskableInterrupt(void)
{
//	gm_LogPut(NON_MASKABLE_INTERRUPT);//commented for simplicity sake

	// Reset the WDT
	WRITE_PCB_REG(WDTCON, 0XAAAA);
	WRITE_PCB_REG(WDTCON, 0X5555);
	WRITE_PCB_REG(EOI, NMI_VTYPE);
}

void interrupt Lpm_isrInt0Overflow(void)
{
//	gm_LogPut(INT0_OVERFLOW);//commented for simplicity sake
	WRITE_PCB_REG(EOI, INTO_VTYPE); //Clear interrupt
}

void interrupt Lpm_isrArrayBoundsException(void)
{
//	gm_LogPut(ARRAY_BOUNDS_EXCEPTION);//commented for simplicity sake
	WRITE_PCB_REG(EOI, BOUNDS_VTYPE); //Clear interrupt
}

void interrupt Lpm_isrUnusedOpcodeException(void)
{
//	gm_LogPut(UNUSED_OPCODE_EXCEPTION);//commented for simplicity sake
	WRITE_PCB_REG(EOI, OPCODE_ERR_VTYPE); //Clear interrupt
}

void interrupt Lpm_isrEscOpcodeException(void)
{
//	gm_LogPut(ESC_OPCODE_EXCEPTION);//commented for simplicity sake
	WRITE_PCB_REG(EOI, ESC_VTYPE); //Clear interrupt
}

//******************************************************************************
// USAGE:      Return if mission is powered up or not
// INPUT:      None
// OUTPUT:      TRUE - mission is powered up
//            FALSE - mission is powered down
// GLOBALS:      None
// USED_REGS:   None
//******************************************************************************
BOOL IsMissionPoweredUp(void)
{
   return (gm_ReadRegWord(LPM_MICRO_STATUS) & MISSION_POR) ? TRUE : FALSE;
}

//******************************************************************************
// USAGE:      Mission power on
// INPUT:      None
// OUTPUT:     None
// GLOBALS:    None
// USED_REGS:  None
//******************************************************************************
void MissionPowerUp(void)
{
   #if 0
   if (gm_ReadRegWord(LPM_MICRO_CONTROL) & LP_RUN)
   {
   gm_SetRegBitsWord(LPM_GPOUTPUT0, SYS_POWERON);
   gm_WriteRegWord(LPM_GPIO_OPENDRAIN_EN, GPIO_OPENDRAINS);
   gm_WriteRegWord(LPM_GPIO_DIRCTRL, GPIO_OUTPUTS);
   gm_WriteRegWord(LPM_GPIO_ENABLE, (GPIO_INPUTS|GPIO_OUTPUTS));
   }    
   #endif

   //This is the time measuring from the toggling of LPM GPIO which turns on the power supply until the
   //  3.3V rail reaches 2.7V. The measured value on GNSS RD platform is 2ms.
   {
      WORD w = gm_W_GetSystemTime();    
      while (IsMissionPoweredUp() == FALSE)
      {
         if (gm_W_GetSystemTime() - w > 100) 
             break;
      }
   }
   #if 0
   if (Configuration.PermanentPeripheralControl)             
      gm_ClearAndSetBitsWord(LPM_MICRO_CONTROL, RCLK_DISABLE|MISSION_MICRO_PWR_DOWN, MISSION_MAIL_OUT_MASK|BIT0|LP_RUN);
   else
      gm_ClearAndSetBitsWord(LPM_MICRO_CONTROL, RCLK_DISABLE|MISSION_MICRO_PWR_DOWN|LP_RUN, MISSION_MAIL_OUT_MASK|BIT0);

   #if 1 // move after LPM_MICRO_CONTROL is set or GPIO is not initial
   if (gm_ReadRegWord(LPM_MICRO_CONTROL) & LP_RUN)
   {
      gm_SetRegBitsWord(LPM_GPOUTPUT0, SYS_POWERON);
      gm_WriteRegWord(LPM_GPIO_OPENDRAIN_EN, GPIO_OPENDRAINS);
      gm_WriteRegWord(LPM_GPIO_DIRCTRL, GPIO_OUTPUTS);
      gm_WriteRegWord(LPM_GPIO_ENABLE, (GPIO_INPUTS|GPIO_OUTPUTS));
   }
   #endif
   #endif

   Lpm_ClearMailbox();

}

void Lpm_ClearMailbox(void)
{
   WRITE_PCB_REG(EOI, INT1_VTYPE);    // Clear mailbox interrupt; NOTE: may miss one TCLK interrupt due to this
   {
      volatile WORD W_Temp = gm_ReadRegWord(LPM_MICRO_MAIL_IN);//to clear interrupt bit   
   }  
}

void Lpm_RedLedCtrl(BYTE Switch)
{
   if(Switch)	
   	{
     // gm_ClearRegBitsWord(LPM_GPOUTPUT_1,LPM_GPIO20_OUT);      
     gm_SetRegBitsWord(LPM_GPOUTPUT_1,LPM_GPIO20_OUT);
   	}
   else
   	{
   	gm_ClearRegBitsWord(LPM_GPOUTPUT_1,LPM_GPIO20_OUT);    
      //gm_SetRegBitsWord(LPM_GPOUTPUT_1,LPM_GPIO20_OUT);
   	}
   
   gm_SetRegBitsWord(LPM_GPIO_DIRCTRL2,LPM_GPIO20_IO);
   gm_SetRegBitsWord(LPM_GPIO_ENABLE2,LPM_GPIO20_EN);	   
}

void Lpm_MfpInit(void)
{
#if 1
   while( gm_ReadRegWord(LPM_MIS_DATA_IN_0) != gmd_MESSAGE_LPM_GPIO_PINOUT ){}
   
   //=============================================================
   //EV1 Board
   // GPIO0(Bit0): ST sensor INT 1    
   // GPIO1: VGA DDC SDA            
   // GPIO2: VGA DDC SCL  
   // GPIO3: CP2 DP power det
   // GPIO4: CEC  
   // GPIO5: CP2 DP Cable det  
   // GPIO6: CP0 DP Power det
   // GPIO7: CP0 cable det  
   // GPIO8: CP2 DP HPD 
   // GPIO9: CP1 HDMI HPD
   // GPIO10: CP0 DP HPD   
   // GPIO11-12: Not use
   // GPIO13: CP1 HDMI DDC SDA      
   // GPIO14: CP1 HDMI DDC SCL   
   // GPIO15(bit0): LPM master I2C SCL 
   // GPIO16: LPM master I2C SDA 
   // GPIO17: DP TX HPD in   
   // GPIO18: Mission power ctr  
   // GPIO19: Not use    
   // GPIO20: LED Amber

   // LPM_GPADC control
   // ADC_AIN0: Keypad_in1   ADC_AIN2: Keypad2   ADC_AIN3: VGA / DVI cable det
   // ADC_AIN4: Not use
   // 00: Special function 01: GPIO_IN 10:GPIO_OUT 11: Not Use pin

   //======GPIO0 or ST touch sensor device INT_IN======== 
   BoardType = gm_ReadRegByte(LPM_MIS_DATA_IN_12) & 0xF;
	if(BoardType != BOARD_4K2K)
	{
	   if (gm_ReadRegWord(LPM_MIS_DATA_IN_1) & 0x03) 
	   {
	      if ((gm_ReadRegWord(LPM_MIS_DATA_IN_1) & 0x03) == 0x01 )
	      {
	         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO0_EN);
	         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO0_IO);
	      }
	      else if ((gm_ReadRegWord(LPM_MIS_DATA_IN_1) & 0x03) == 0x02 )
	      {
	         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO0_EN);     
	         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO0_IO);
	      }
	   }
	   else
	   {  
	      //ST touch sensor device INT_IN
	   }
	}

   //======GPIO1  or SLV4_SDA========
   if (gm_ReadRegWord(LPM_MIS_DATA_IN_1) & 0x0C) 
   {
      if ((gm_ReadRegWord(LPM_MIS_DATA_IN_1) & 0x0C) == 0x04 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO1_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO1_IO);
      }
      else if ((gm_ReadRegWord(LPM_MIS_DATA_IN_1) & 0x0C) == 0x08 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO1_EN);     
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO1_IO);
      }
   }
   else
   {  
      //SLV4_SDA setting  or UART
   }

   //======GPIO2  or SLV4_SCL========
   if (gm_ReadRegWord(LPM_MIS_DATA_IN_1) & 0x30) 
   {
      if ((gm_ReadRegWord(LPM_MIS_DATA_IN_1) & 0x30) == 0x10 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO2_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO2_IO);
      }
      else if ((gm_ReadRegWord(LPM_MIS_DATA_IN_1) & 0x30) == 0x20 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO2_EN);     
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO2_IO);
      }
   }
   else
   {  
      //SLV4_SCL setting 
   }

   //======GPIO3  or EDPD4 ========
   if (gm_ReadRegWord(LPM_MIS_DATA_IN_1) & 0xC0) 
   {
      if ((gm_ReadRegWord(LPM_MIS_DATA_IN_1) & 0xC0) == 0x40 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO3_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO3_IO);
      }
      else if ((gm_ReadRegWord(LPM_MIS_DATA_IN_1) & 0xC0) == 0x80 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO3_EN);     
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO3_IO);
      }
   }
   else
   {  
      //EDPD4 setting 
   }

   //======GPIO4  or HDMI CEC ========
   if (gm_ReadRegWord(LPM_MIS_DATA_IN_2) & 0x03) 
   {
      if ((gm_ReadRegWord(LPM_MIS_DATA_IN_2) & 0x03) == 0x01 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO4_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO4_IO);
      }
      else if ((gm_ReadRegWord(LPM_MIS_DATA_IN_2) & 0x03) == 0x02 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO4_EN);     
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO4_IO);
      }
   }
   else
   {  
      gm_SetRegBitsWord(CEC_CTRL, CEC_EN);
   }  

   //======GPIO5  or EDPD3 ========
   if (gm_ReadRegWord(LPM_MIS_DATA_IN_2) & 0x0C) 
   {
      if ((gm_ReadRegWord(LPM_MIS_DATA_IN_2) & 0x0C) == 0x04 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO5_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO5_IO);
      }
      else if ((gm_ReadRegWord(LPM_MIS_DATA_IN_2) & 0x0C) == 0x08 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO5_EN);     
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO5_IO);
      }
   }
   else
   {  
      //EDPD3 setting 
   }     

   //======GPIO6  or  EDPD2 ========
   if (gm_ReadRegWord(LPM_MIS_DATA_IN_2) & 0x30) 
   {
      if ((gm_ReadRegWord(LPM_MIS_DATA_IN_2) & 0x30) == 0x10 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO6_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO6_IO);
      }
      else if ((gm_ReadRegWord(LPM_MIS_DATA_IN_2) & 0x30) == 0x20 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO6_EN);     
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO6_IO);
      }
   }
   else
   {  
      //EDPD2 setting 
   }     

   //======GPIO7  or  EDPD1 ========
   if (gm_ReadRegWord(LPM_MIS_DATA_IN_2) & 0xC0) 
   {
      if ((gm_ReadRegWord(LPM_MIS_DATA_IN_2) & 0xC0) == 0x40 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO7_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO7_IO);
      }
      else if ((gm_ReadRegWord(LPM_MIS_DATA_IN_2) & 0xC0) == 0x80 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO7_EN);     
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO7_IO);
      }
   }
   else
   {  
      //EDPD1 setting 
   }     

   //======GPIO8  or HPD3 ========
   if (gm_ReadRegWord(LPM_MIS_DATA_IN_3) & 0x03) 
   {
      if ((gm_ReadRegWord(LPM_MIS_DATA_IN_3) & 0x03) == 0x01 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO8_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO8_IO);
      }
      else if ((gm_ReadRegWord(LPM_MIS_DATA_IN_3) & 0x03) == 0x02 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO8_EN);     
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO8_IO);
      }
   }
   else
   {  
      //HPD3 setting 
   }  

   //======GPIO9  or HPD2 ========
   if (gm_ReadRegWord(LPM_MIS_DATA_IN_3) & 0x0C) 
   {
      if ((gm_ReadRegWord(LPM_MIS_DATA_IN_3) & 0x0C) == 0x04 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO9_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO9_IO);
      }
      else if ((gm_ReadRegWord(LPM_MIS_DATA_IN_3) & 0x0C) == 0x08 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO9_EN);     
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO9_IO);
      }
   }
   else
   {  
      //HPD2 setting 
   }     

   //======GPIO10  or  HPD1 ========
   if (gm_ReadRegWord(LPM_MIS_DATA_IN_3) & 0x30) 
   {
      if ((gm_ReadRegWord(LPM_MIS_DATA_IN_3) & 0x30) == 0x10 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO10_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO10_IO);
      }
      else if ((gm_ReadRegWord(LPM_MIS_DATA_IN_3) & 0x30) == 0x20 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO10_EN);    
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO10_IO);
      }
   }
   else
   {  
      //HPD1 setting 
   }     

   //======GPIO11  or  SLV3_SCL ========
   if (gm_ReadRegWord(LPM_MIS_DATA_IN_3) & 0xC0) 
   {
      if ((gm_ReadRegWord(LPM_MIS_DATA_IN_3) & 0xC0) == 0x40 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO11_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO11_IO);
      }
      else if ((gm_ReadRegWord(LPM_MIS_DATA_IN_3) & 0xC0) == 0x80 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO11_EN);    
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO11_IO);
      }
   }
   else
   {  
      //SLV3_SCL setting 
   }  

   //======GPIO12  or  SLV3_SDA ========
   if (gm_ReadRegWord(LPM_MIS_DATA_IN_4) & 0x03) 
   {
      if ((gm_ReadRegWord(LPM_MIS_DATA_IN_4) & 0x03) == 0x01 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO12_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO12_IO);
      }
      else if ((gm_ReadRegWord(LPM_MIS_DATA_IN_4) & 0x03) == 0x02 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO12_EN);    
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO12_IO);
      }
   }
   else
   {  
      //SLV3_SDA setting 
   }  

   //======GPIO13 or  SLV2_SCL ========
   if (gm_ReadRegWord(LPM_MIS_DATA_IN_4) & 0x0C) 
   {
      if ((gm_ReadRegWord(LPM_MIS_DATA_IN_4) & 0x0C) == 0x04 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO13_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO13_IO);
      }
      else if ((gm_ReadRegWord(LPM_MIS_DATA_IN_4) & 0x0C) == 0x08 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO13_EN);    
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO13_IO);
      }
   }
   else
   {  
      //SLV2_SCL setting 
   }     

   //======GPIO14  or  SLV2_SDA ========
   if (gm_ReadRegWord(LPM_MIS_DATA_IN_4) & 0x30) 
   {
      if ((gm_ReadRegWord(LPM_MIS_DATA_IN_4) & 0x30) == 0x10 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO14_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO14_IO);
      }
      else if ((gm_ReadRegWord(LPM_MIS_DATA_IN_4) & 0x30) == 0x20 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO14_EN);    
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO14_IO);
      }
   }
   else
   {  
      //SLV2_SDA setting 
   }     

   //======GPIO15  or  SLV1_SCL ========
   if (gm_ReadRegWord(LPM_MIS_DATA_IN_4) & 0xC0) 
   {
      if ((gm_ReadRegWord(LPM_MIS_DATA_IN_4) & 0xC0) == 0x40 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE2, LPM_GPIO15_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL2, LPM_GPIO15_IO);
      }
      else if ((gm_ReadRegWord(LPM_MIS_DATA_IN_4) & 0xC0) == 0x80 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE2, LPM_GPIO15_EN);    
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO15_IO);
      }
   }
   else
   {  
      //SLV1_SCL setting 
   }  

   //======GPIO16  or  SLV1_SDA========
   if (gm_ReadRegWord(LPM_MIS_DATA_IN_5) & 0x03) 
   {
      if ((gm_ReadRegWord(LPM_MIS_DATA_IN_5) & 0x03) == 0x01 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE2, LPM_GPIO16_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL2, LPM_GPIO16_IO);
      }
      else if ((gm_ReadRegWord(LPM_MIS_DATA_IN_5) & 0x03) == 0x02 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE2, LPM_GPIO16_EN);    
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL2, LPM_GPIO16_IO);
      }
   }
   else
   {  
      //SLV1_SDA setting 
   }  

   //======GPIO17  or  EDPD4 ========
   if (gm_ReadRegWord(LPM_MIS_DATA_IN_5) & 0x0C) 
   {
      if ((gm_ReadRegWord(LPM_MIS_DATA_IN_5) & 0x0C) == 0x04 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE2, LPM_GPIO17_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL2, LPM_GPIO17_IO);
      }
      else if ((gm_ReadRegWord(LPM_MIS_DATA_IN_5) & 0x0C) == 0x08 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE2, LPM_GPIO17_EN);    
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL2, LPM_GPIO17_IO);
      }
   }
   else
   {  
      //EDPD4 setting 
   }     

   //======GPIO18  ========
   if (gm_ReadRegWord(LPM_MIS_DATA_IN_5) & 0x30) 
   {
      if ((gm_ReadRegWord(LPM_MIS_DATA_IN_5) & 0x30) == 0x10 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE2, LPM_GPIO18_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL2, LPM_GPIO18_IO);
      }
      else if ((gm_ReadRegWord(LPM_MIS_DATA_IN_5) & 0x30) == 0x20 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE2, LPM_GPIO18_EN);    
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL2, LPM_GPIO18_IO);
      }
   }

	if(BoardType != BOARD_4K2K)
	{
   //======GPIO19 ========
   if (gm_ReadRegWord(LPM_MIS_DATA_IN_5) & 0xC0) 
   {
      if ((gm_ReadRegWord(LPM_MIS_DATA_IN_5) & 0xC0) == 0x40 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE2, LPM_GPIO19_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL2, LPM_GPIO19_IO);
      }
      else if ((gm_ReadRegWord(LPM_MIS_DATA_IN_5) & 0xC0) == 0x80 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE2, LPM_GPIO19_EN);    
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL2, LPM_GPIO19_IO);
      }
   }  
	}
   //======GPIO20 LED ========
   if (gm_ReadRegWord(LPM_MIS_DATA_IN_6) & 0x03) 
   {
      if ((gm_ReadRegWord(LPM_MIS_DATA_IN_6) & 0x03) == 0x01 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE2, LPM_GPIO20_EN);
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL2, LPM_GPIO20_IO);
	 gm_SetRegBitsWord(LPM_GPOUTPUT_1,LPM_GPIO20_OUT);    
      }
      else if ((gm_ReadRegWord(LPM_MIS_DATA_IN_6) & 0x03) == 0x02 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE2, LPM_GPIO20_EN);    
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL2, LPM_GPIO20_IO);
	   gm_SetRegBitsWord(LPM_GPOUTPUT_1,LPM_GPIO20_OUT);    
      }
   }

   //====ADC Power Key channel  & value  ====   6~8
#ifdef KEYPAD_SUPPORT
{
   BYTE numChannels = gm_ReadRegByte(LPM_MIS_DATA_IN_7);
   BYTE offChannel = gm_ReadRegByte(LPM_MIS_DATA_IN_8);
   BYTE offValue = gm_ReadRegByte(LPM_MIS_DATA_IN_9);
   #if LPM_FORCE_POWER_DOWN
   IsForceDcOff = gm_ReadRegByte(LPM_MIS_DATA_IN_8+1);
   #endif
   // timeout , threshold , total channel ,power key channel  ,  power key value
   Lpm_KeypadLbwAdcInit(4, 6, numChannels, offChannel, offValue);   

   if (numChannels == 4)
   {
      gm_WriteRegWord(LPM_GPADC_GPIO_CONTROL, GPADC_ADC3IN_DIS|GPADC_ADC2IN_DIS|GPADC_ADC1IN_DIS|GPADC_ADC0IN_DIS);
   }
   else // Use 3 ADC (0, 1, 2)
   {
      // default gpadc3 as gpio output high
      gm_WriteRegWord(LPM_GPADC_GPIO_CONTROL, GPADC_GPIO3_IO|GPADC_ADC2IN_DIS|GPADC_ADC1IN_DIS|GPADC_ADC0IN_DIS);
      gm_SetRegBitsWord(LPM_GPADC_GPOUTPUT, GPADC_GPIO3_OUT);
   }
}
#endif

   //==== Reserved ====    9,10
   //gm_ReadRegWord(LPM_MIS_DATA_IN_10);
   //gm_ReadRegWord(LPM_MIS_DATA_IN_11);

   //==== Board define==== 11 
   BoardType = gm_ReadRegByte(LPM_MIS_DATA_IN_12) & 0xF;

   switch(BoardType)
   {
      case BOARD_EV1:
         // Combo Phy define
         CPhyType[CPhyNum_0] = CPhyType_DP;
         CPhyType[CPhyNum_1] = CPhyType_HDMI;
         CPhyType[CPhyNum_2] = CPhyType_DP;    
         // DPRx_A pins define
         DpRxHpdPin[DPRX_C0] = LPM_GPIO10_EN;
         DpRxSrcPwrPin[DPRX_C0] = PD2_IN_STATUS;   //LPM_GPIO6_EDPD2
         DpRxCblDetPin[DPRX_C0] = PD1_IN_STATUS;   //LPM_GPIO7_EDPD1
         // DPRx_B pins define
         DpRxHpdPin[DPRX_C2] = LPM_GPIO8_EN;
         DpRxSrcPwrPin[DPRX_C2] = PD4_IN_STATUS;   //LPM_GPIO3_EDPD4
         DpRxCblDetPin[DPRX_C2] = PD3_IN_STATUS;   //LPM_GPIO5_EDPD3
         // HDMI/DVI pins define
         HdmiDviHpdPin[CPhyNum_1] = LPM_GPIO9_EN;

         gm_Print("LPM set for EV1 Board!",0);
         break;

      case BOARD_EV2:
         // Combo Phy define
         CPhyType[CPhyNum_0] = CPhyType_HDMI;
         CPhyType[CPhyNum_1] = CPhyType_HDMI;
         CPhyType[CPhyNum_2] = CPhyType_HDMI;   
         // HDMI/DVI pins define
         HdmiDviHpdPin[CPhyNum_0] = LPM_GPIO10_EN;         
         HdmiDviHpdPin[CPhyNum_1] = LPM_GPIO9_EN;
         HdmiDviHpdPin[CPhyNum_2] = LPM_GPIO8_EN;

         gm_Print("LPM set for EV2 Board!",0);
         break;

      case BOARD_EV3:
         // Combo Phy define
         CPhyType[CPhyNum_0] = CPhyType_DP;
         CPhyType[CPhyNum_1] = CPhyType_DVI;   //Dual-DVI 
         CPhyType[CPhyNum_2] = CPhyType_None; 
         // DPRx_A pins define
         DpRxHpdPin[DPRX_C0] = LPM_GPIO10_EN;
         DpRxSrcPwrPin[DPRX_C0] = PD2_IN_STATUS;   //LPM_GPIO6_EDPD2
         DpRxCblDetPin[DPRX_C0] = PD1_IN_STATUS;   //LPM_GPIO7_EDPD1
         // HDMI/DVI pins define
         HdmiDviHpdPin[CPhyNum_1] = LPM_GPIO9_EN;  //Dual-DVI

         gm_Print("LPM set for EV3 Board!",0);
         break;

      case BOARD_4K2K:
			ChipID4k2K = gm_ReadRegByte(LPM_MIS_DATA_IN_12)>>4;
         // Combo Phy define
         if(ChipID4k2K == CHIPID_4K2K_FE)
         {
	         CPhyType[CPhyNum_0] = CPhyType_HDMI;
	         CPhyType[CPhyNum_1] = CPhyType_None; 
	         CPhyType[CPhyNum_2] = CPhyType_None;   
         }
			else 
			{
				CPhyType[CPhyNum_0] = CPhyType_DP;
	         CPhyType[CPhyNum_1] = CPhyType_None;
	         CPhyType[CPhyNum_2] = CPhyType_DP; 
			}
         // DPRx_A pins define
         DpRxHpdPin[DPRX_C0] = LPM_GPIO10_EN;
         DpRxSrcPwrPin[DPRX_C0] = PD2_IN_STATUS;   //LPM_GPIO6_EDPD2
         DpRxCblDetPin[DPRX_C0] = PD1_IN_STATUS;   //LPM_GPIO7_EDPD1

         DpRxHpdPin[DPRX_C2] = LPM_GPIO8_EN;
         DpRxSrcPwrPin[DPRX_C2] = PD4_IN_STATUS;   //LPM_GPIO3_EDPD4
         DpRxCblDetPin[DPRX_C2] = PD3_IN_STATUS;   //LPM_GPIO5_EDPD3
         
         // HDMI/DVI pins define
         //HdmiDviHpdPin[CPhyNum_1] = LPM_GPIO9_EN;  //Dual-DVI
         //ChipID4k2K = gm_ReadRegByte(LPM_MIS_DATA_IN_12)>>4;
         gm_Print("LPM set for 4K2K Board-(%d)!",ChipID4k2K);
         break;
  
      case BOARD_RD1:
      case BOARD_RD2:
      case BOARD_RD3:
      case BOARD_RD1_REV_B:
      default:
         // Combo Phy define
         CPhyType[CPhyNum_0] = CPhyType_DP;
         CPhyType[CPhyNum_1] = CPhyType_DVI;
         CPhyType[CPhyNum_2] = CPhyType_HDMI;  
         // DPRx_A pins define
         DpRxHpdPin[DPRX_C0] = LPM_GPIO10_EN;
         DpRxSrcPwrPin[DPRX_C0] = PD2_IN_STATUS;   //LPM_GPIO6_EDPD2
         DpRxCblDetPin[DPRX_C0] = PD1_IN_STATUS;   //LPM_GPIO7_EDPD1   
         // HDMI/DVI pins define
         HdmiDviHpdPin[CPhyNum_1] = LPM_GPIO9_EN;         
         HdmiDviHpdPin[CPhyNum_2] = LPM_GPIO8_EN; 

         if(BoardType == BOARD_RD1)
            gm_Print("LPM set for RD1 Board!",0);
         else if(BoardType == BOARD_RD2)
            gm_Print("LPM set for RD2 Board!",0);
         else if(BoardType == BOARD_RD3)
            gm_Print("LPM set for RD3 Board!",0);
         break;
   }
   #ifdef IR_SUPPORT
   IrPowerKey = gm_ReadRegByte(LPM_MIS_DATA_IN_13);
   #endif

   ProductId = gm_ReadRegWord(LPM_MIS_DATA_IN_14);
   ProductRev = gm_ReadRegWord(LPM_MIS_DATA_IN_15);
   //gm_Print("ProductId = 0x%x", ProductId);
   //gm_Print("ProductRev = 0x%x", ProductRev);
   
#else
   BYTE LPMGPIO_Config[13];
   while( (gm_MessageReceived(gmd_MESSAGE_LPM_GPIO_PINOUT, &LPMGPIO_Config[0], 13)==0) ); 

   #if 0	
   gm_Print("----LPMGPIO_Config[0] =0x%x",LPMGPIO_Config[0]);
   gm_Print("----LPMGPIO_Config[1] =0x%x",LPMGPIO_Config[1]);
   gm_Print("----LPMGPIO_Config[2] =0x%x",LPMGPIO_Config[2]);
   gm_Print("----LPMGPIO_Config[3] =0x%x",LPMGPIO_Config[3]);
   gm_Print("----LPMGPIO_Config[4] =0x%x",LPMGPIO_Config[4]);
   gm_Print("----LPMGPIO_Config[5] =0x%x",LPMGPIO_Config[5]);
   gm_Print("----LPMGPIO_Config[6] =0x%x",LPMGPIO_Config[6]);	
   gm_Print("----LPMGPIO_Config[7] =0x%x",LPMGPIO_Config[7]);
   gm_Print("----LPMGPIO_Config[8] =0x%x",LPMGPIO_Config[8]);
   gm_Print("----LPMGPIO_Config[9] =0x%x",LPMGPIO_Config[9]);	
   #endif
   //=============================================================
   //EV1 Board
   // GPIO0(Bit0): ST sensor INT 1	  
   // GPIO1: VGA DDC SDA            
   // GPIO2: VGA DDC SCL  
   // GPIO3: CP2 DP power det
   // GPIO4: CEC	
   // GPIO5: CP2 DP Cable det  
   // GPIO6: CP0 DP Power det
   // GPIO7: CP0 cable det	 
   // GPIO8: CP2 DP HPD	
   // GPIO9: CP1 HDMI HPD
   // GPIO10: CP0 DP HPD	
   // GPIO11-12: Not use
   // GPIO13: CP1 HDMI DDC SDA	   
   // GPIO14: CP1 HDMI DDC SCL	
   // GPIO15(bit0): LPM master I2C SCL	
   // GPIO16: LPM master I2C SDA	
   // GPIO17: DP TX HPD in	  
   // GPIO18: Mission power ctr	
   // GPIO19: Not use	 
   // GPIO20: LED Amber

   // LPM_GPADC control
   // ADC_AIN0: Keypad_in1	  ADC_AIN2: Keypad2   ADC_AIN3: VGA / DVI cable det
   // ADC_AIN4: Not use
   // 00: Special function	01: GPIO_IN	10:GPIO_OUT	11: Not Use pin

   //======GPIO0 or ST touch sensor device INT_IN======== 
   if (LPMGPIO_Config[0] & 0x03) 
   {
      if ((LPMGPIO_Config[0] & 0x3) ==0x01 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1,LPM_GPIO0_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL1,LPM_GPIO0_IO);
      }
      if ((LPMGPIO_Config[0] & 0x3) ==0x02 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1,LPM_GPIO0_EN);		
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1,LPM_GPIO0_IO);
      }
   }
   else
   {	
      //ST touch sensor device INT_IN
   }
	
   //======GPIO1  or SLV4_SDA========
   if (LPMGPIO_Config[0] & 0x0c) 
   {
      if ((LPMGPIO_Config[0] & 0x0c) ==0x04 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1,LPM_GPIO1_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL1,LPM_GPIO1_IO);
      }
      if ((LPMGPIO_Config[0] & 0x0c) ==0x08 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1,LPM_GPIO1_EN);		
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1,LPM_GPIO1_IO);
      }
   }
   else
   {	
      //SLV4_SDA setting  or UART
   }

   //======GPIO2  or SLV4_SCL========
   if (LPMGPIO_Config[0] & 0x30) 
   {
      if ((LPMGPIO_Config[0] & 0x30) ==0x10 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1,LPM_GPIO2_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL1,LPM_GPIO2_IO);
      }
      if ((LPMGPIO_Config[0] & 0x30) ==0x20 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1,LPM_GPIO2_EN);		
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1,LPM_GPIO2_IO);
      }
   }
   else
   {	
      //SLV4_SCL setting 
   }

   //======GPIO3  or EDPD4 ========
   if (LPMGPIO_Config[0] & 0xc0) 
   {
      if ((LPMGPIO_Config[0] & 0xc0) ==0x40 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1,LPM_GPIO3_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL1,LPM_GPIO3_IO);
      }
      if ((LPMGPIO_Config[0] & 0xc0) ==0x80 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1,LPM_GPIO3_EN);		
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1,LPM_GPIO3_IO);
      }
   }
   else
   {	
      //EDPD4 setting 
   }

   //======GPIO4  or HDMI CEC ========
   if (LPMGPIO_Config[1] & 0x03) 
   {
      if ((LPMGPIO_Config[1] & 0x3) ==0x01 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1,LPM_GPIO4_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL1,LPM_GPIO4_IO);
      }
      if ((LPMGPIO_Config[1] & 0x3) ==0x02 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1,LPM_GPIO4_EN);		
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1,LPM_GPIO4_IO);
      }
   }
   else
   {	
      gm_SetRegBitsWord(CEC_CTRL, CEC_EN);
   }	

   //======GPIO5  or EDPD3 ========
   if (LPMGPIO_Config[1] & 0x0c) 
   {
      if ((LPMGPIO_Config[1] & 0xc) ==0x04 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1,LPM_GPIO5_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL1,LPM_GPIO5_IO);
      }
      if ((LPMGPIO_Config[1] & 0xc) ==0x08 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1,LPM_GPIO5_EN);		
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1,LPM_GPIO5_IO);
      }
   }
   else
   {	
      //EDPD3 setting 
   }		

   //======GPIO6  or  EDPD2 ========
   if (LPMGPIO_Config[1] & 0x30) 
   {
      if ((LPMGPIO_Config[1] & 0x30) ==0x10 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1,LPM_GPIO6_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL1,LPM_GPIO6_IO);
      }
      if ((LPMGPIO_Config[1] & 0x30) ==0x20 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1,LPM_GPIO6_EN);		
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1,LPM_GPIO6_IO);
      }
   }
   else
   {	
      //EDPD2 setting 
   }		

   //======GPIO7  or  EDPD1 ========
   if (LPMGPIO_Config[1] & 0xc0) 
   {
      if ((LPMGPIO_Config[1] & 0xc0) ==0x40 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1,LPM_GPIO7_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL1,LPM_GPIO7_IO);
      }
      if ((LPMGPIO_Config[1] & 0xc0) ==0x80 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1,LPM_GPIO7_EN);		
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1,LPM_GPIO7_IO);
      }
   }
   else
   {	
      //EDPD1 setting 
   }		

   //======GPIO8  or HPD3 ========
   if (LPMGPIO_Config[2] & 0x03) 
   {
      if ((LPMGPIO_Config[2] & 0x03) ==0x01 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1,LPM_GPIO8_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL1,LPM_GPIO8_IO);
      }
      if ((LPMGPIO_Config[2] & 0x03) ==0x02 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1,LPM_GPIO8_EN);		
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1,LPM_GPIO8_IO);
      }
   }
   else
   {	
      //HPD3 setting 
   }	

   //======GPIO9  or HPD2 ========
   if (LPMGPIO_Config[2] & 0x0c) 
   {
      if ((LPMGPIO_Config[2] & 0x0c) ==0x04 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1,LPM_GPIO9_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL1,LPM_GPIO9_IO);
      }
      if ((LPMGPIO_Config[2] & 0x0c) ==0x08 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1,LPM_GPIO9_EN);		
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1,LPM_GPIO9_IO);
      }
   }
   else
   {	
      //HPD2 setting 
   }		

   //======GPIO10  or  HPD1 ========
   if (LPMGPIO_Config[2] & 0x30) 
   {
      if ((LPMGPIO_Config[2] & 0x30) ==0x10 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1,LPM_GPIO10_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL1,LPM_GPIO10_IO);
      }
      if ((LPMGPIO_Config[2] & 0x30) ==0x20 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1,LPM_GPIO10_EN);		
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1,LPM_GPIO10_IO);
      }
   }
   else
   {	
      //HPD1 setting 
   }		

   //======GPIO11  or  SLV3_SCL ========
   if (LPMGPIO_Config[2] & 0xc0) 
   {
      if ((LPMGPIO_Config[2] & 0xc0) ==0x40 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1,LPM_GPIO11_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL1,LPM_GPIO11_IO);
      }
      if ((LPMGPIO_Config[2] & 0xc0) ==0x80 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1,LPM_GPIO11_EN);		
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1,LPM_GPIO11_IO);
      }
   }
   else
   {	
      //SLV3_SCL setting 
   }	

   //======GPIO12  or  SLV3_SDA ========
   if (LPMGPIO_Config[3] & 0x03) 
   {
      if ((LPMGPIO_Config[3] & 0x03) ==0x01 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1,LPM_GPIO12_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL1,LPM_GPIO12_IO);
      }
      if ((LPMGPIO_Config[3] & 0x03) ==0x02 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1,LPM_GPIO12_EN);		
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1,LPM_GPIO12_IO);
      }
   }
   else
   {	
      //SLV3_SDA setting 
   }	

   //======GPIO13 or  SLV2_SCL ========
   if (LPMGPIO_Config[3] & 0x0c) 
   {
      if ((LPMGPIO_Config[3] & 0x0c) ==0x04 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1,LPM_GPIO13_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL1,LPM_GPIO13_IO);
      }
      if ((LPMGPIO_Config[3] & 0x0c) ==0x08 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1,LPM_GPIO13_EN);		
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1,LPM_GPIO13_IO);
      }
   }
   else
   {	
      //SLV2_SCL setting 
   }		

   //======GPIO14  or  SLV2_SDA ========
   if (LPMGPIO_Config[3] & 0x30) 
   {
      if ((LPMGPIO_Config[3] & 0x30) ==0x10 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1,LPM_GPIO14_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL1,LPM_GPIO14_IO);
      }
      if ((LPMGPIO_Config[3] & 0x30) ==0x20 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1,LPM_GPIO14_EN);		
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1,LPM_GPIO14_IO);
      }
   }
   else
   {	
      //SLV2_SDA setting 
   }		

   //======GPIO15  or  SLV1_SCL ========
   if (LPMGPIO_Config[3] & 0xc0) 
   {
      if ((LPMGPIO_Config[3] & 0xc0) ==0x40 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE2,LPM_GPIO15_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL2,LPM_GPIO15_IO);
      }
      if ((LPMGPIO_Config[3] & 0xc0) ==0x80 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE2,LPM_GPIO15_EN);		
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1,LPM_GPIO15_IO);
      }
   }
   else
   {	
      //SLV1_SCL setting 
   }	

   //======GPIO16  or  SLV1_SDA========
   if (LPMGPIO_Config[4] & 0x03) 
   {
      if ((LPMGPIO_Config[4] & 0x03) ==0x01 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE2,LPM_GPIO16_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL2,LPM_GPIO16_IO);
      }
      if ((LPMGPIO_Config[4] & 0x03) ==0x02 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE2,LPM_GPIO16_EN);		
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL2,LPM_GPIO16_IO);
      }
   }
   else
   {	
      //SLV1_SDA setting 
   }	

   //======GPIO17  or  EDPD4 ========
   if (LPMGPIO_Config[4] & 0x0c) 
   {
      if ((LPMGPIO_Config[4] & 0x0c) ==0x04 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE2,LPM_GPIO17_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL2,LPM_GPIO17_IO);
      }
      if ((LPMGPIO_Config[4] & 0x0c) ==0x08 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE2,LPM_GPIO17_EN);		
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL2,LPM_GPIO17_IO);
      }
   }
   else
   {	
      //EDPD4 setting 
   }		

   //======GPIO18  ========
   if (LPMGPIO_Config[4] & 0x30) 
   {
      if ((LPMGPIO_Config[4] & 0x30) ==0x10 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE2,LPM_GPIO18_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL2,LPM_GPIO18_IO);
      }
      if ((LPMGPIO_Config[4] & 0x30) ==0x20 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE2,LPM_GPIO18_EN);		
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL2,LPM_GPIO18_IO);
      }
   }

   //======GPIO19 ========
   if (LPMGPIO_Config[4] & 0xc0) 
   {
      if ((LPMGPIO_Config[4] & 0xc0) ==0x40 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE2,LPM_GPIO19_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL2,LPM_GPIO19_IO);
      }
      if ((LPMGPIO_Config[4] & 0xc0) ==0x80 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE2,LPM_GPIO19_EN);		
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL2,LPM_GPIO19_IO);
      }
   }	

   //======GPIO20 LED ========
   if (LPMGPIO_Config[5] & 0x03) 
   {
      if ((LPMGPIO_Config[5] & 0x03) ==0x01 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE2,LPM_GPIO20_EN);
         gm_ClearRegBitsWord(LPM_GPIO_DIRCTRL2,LPM_GPIO20_IO);
      }
      if ((LPMGPIO_Config[5] & 0x03) ==0x02 )
      {
         gm_SetRegBitsWord(LPM_GPIO_ENABLE2,LPM_GPIO20_EN);		
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL2,LPM_GPIO20_IO);
      }
   }

   //====ADC Power Key channel  & value  ====	6~8
   #ifdef KEYPAD_SUPPORT
   // timeout , threshold , total channel ,power key channel  ,  power key value
   Lpm_KeypadLbwAdcInit(4, 6, LPMGPIO_Config[6], LPMGPIO_Config[7], LPMGPIO_Config[8]); 	

   if (LPMGPIO_Config[6]==4)
   {
      gm_WriteRegWord(LPM_GPADC_GPIO_CONTROL,0x0F);
   }
   else // Use 3 ADC	
   {
      gm_WriteRegWord(LPM_GPADC_GPIO_CONTROL,0x87); // default gpadc3 as gpio output
      gm_SetRegBitsWord(LPM_GPADC_GPOUTPUT,GPADC_GPIO3_OUT);	// default gpadc3 output as high
   }
   #endif

   //==== Reserved ====		9,10
   //LPMGPIO_Config[9];
   //LPMGPIO_Config[10];

   //==== Board define====	11 
   BoardType = LPMGPIO_Config[11];

   switch(BoardType)
   {
      case BOARD_EV1:
         // Combo Phy define
         CPhyType[CPhyNum_0] = CPhyType_DP;
         CPhyType[CPhyNum_1] = CPhyType_HDMI;
         CPhyType[CPhyNum_2] = CPhyType_DP;    
         // DPRx_A pins define
         DpRxHpdPin[DPRX_C0] = LPM_GPIO10_EN;
         DpRxSrcPwrPin[DPRX_C0] = PD2_IN_STATUS;   //LPM_GPIO6_EDPD2
         DpRxCblDetPin[DPRX_C0] = PD1_IN_STATUS;   //LPM_GPIO7_EDPD1
         // DPRx_B pins define
         DpRxHpdPin[DPRX_C2] = LPM_GPIO8_EN;
         DpRxSrcPwrPin[DPRX_C2] = PD4_IN_STATUS;   //LPM_GPIO3_EDPD4
         DpRxCblDetPin[DPRX_C2] = PD3_IN_STATUS;   //LPM_GPIO5_EDPD3
         // HDMI/DVI pins define
         HdmiDviHpdPin[CPhyNum_1] = LPM_GPIO9_EN;
   
         gm_Print("LPM set for EV1 Board!",0);
         break;

      case BOARD_EV2:
         // Combo Phy define
         CPhyType[CPhyNum_0] = CPhyType_HDMI;
         CPhyType[CPhyNum_1] = CPhyType_HDMI;
         CPhyType[CPhyNum_2] = CPhyType_HDMI;   
         // HDMI/DVI pins define
         HdmiDviHpdPin[CPhyNum_0] = LPM_GPIO10_EN;         
         HdmiDviHpdPin[CPhyNum_1] = LPM_GPIO9_EN;
         HdmiDviHpdPin[CPhyNum_2] = LPM_GPIO8_EN;

         gm_Print("LPM set for EV2 Board!",0);
         break;

      case BOARD_EV3:
         // Combo Phy define
         CPhyType[CPhyNum_0] = CPhyType_DP;
         CPhyType[CPhyNum_1] = CPhyType_DVI;   //Dual-DVI 
         // DPRx_A pins define
         DpRxHpdPin[DPRX_C0] = LPM_GPIO10_EN;
         DpRxSrcPwrPin[DPRX_C0] = PD2_IN_STATUS;   //LPM_GPIO6_EDPD2
         DpRxCblDetPin[DPRX_C0] = PD1_IN_STATUS;   //LPM_GPIO7_EDPD1
         // HDMI/DVI pins define
         HdmiDviHpdPin[CPhyNum_1] = LPM_GPIO9_EN;  //Dual-DVI
   
         gm_Print("LPM set for EV3 Board!",0);
         break;


      case BOARD_RD1:
      case BOARD_RD2:
      default:
         // Combo Phy define
         CPhyType[CPhyNum_0] = CPhyType_DP;
         CPhyType[CPhyNum_1] = CPhyType_DVI;
         CPhyType[CPhyNum_2] = CPhyType_HDMI;  
         // DPRx_A pins define
         DpRxHpdPin[DPRX_C0] = LPM_GPIO10_EN;
         DpRxSrcPwrPin[DPRX_C0] = PD2_IN_STATUS;   //LPM_GPIO6_EDPD2
         DpRxCblDetPin[DPRX_C0] = PD1_IN_STATUS;   //LPM_GPIO7_EDPD1   
         // HDMI/DVI pins define
         HdmiDviHpdPin[CPhyNum_1] = LPM_GPIO9_EN;         
         HdmiDviHpdPin[CPhyNum_2] = LPM_GPIO8_EN; 

         if(BoardType == BOARD_RD2)
            gm_Print("LPM set for RD2 Board!",0);
         else
            gm_Print("LPM set for RD1 Board!",0);
         break;
   }
   #ifdef IR_SUPPORT
   IrPowerKey = LPMGPIO_Config[12];
   #endif
#endif   
}

//******************************************************************************
// FUNCTION:   void Lpm_SystemInit(void)
// USAGE:      System initialization
// INPUT:      None
// OUTPUT:     None
// GLOBALS:    None
// USED_REGS:  None
//******************************************************************************
void Lpm_SystemInit(void)
{

	 Lpm_ClearMailbox(); 

	 Lpm_McuInit();

	 #if (LPM_OCMCLK_DEFAULT == 0)
	 gm_WriteRegWord(LPM_RCOSC,(RcOscTrim<<LPM_RC_OSC_TRIM_SHIFT)|LPM_RC_OSC_EN);
	 gm_WriteRegWord(LPM_CLOCK_CTRL, 2<<LPM_OCMCLK_SEL_SHIFT);//RC-OSC drives LPM_OCMCLK
	 #elif (LPM_OCMCLK_DEFAULT == 1)
	 gm_WriteRegWord(LPM_CLOCK_CTRL,LPM_TCLK_SEL);//0x01
	 #else //(LPM_OCMCLK_DEFAULT == 2)
	 gm_WriteRegWord(LPM_CLOCK_CTRL, LPM_OCMCLK_SEL);//0x06
	 #endif
	
	 //Set LPM PBus Master controling parameters
	 gm_WriteRegWord(LPM_PBUS_MSTR_CTRL_0,0x2121);
	 gm_WriteRegWord(LPM_PBUS_MSTR_CTRL_1,0x33);
	
	 Lpm_TimerInit(SYSTEM_TCLK_FREQ);
	 
	 gm_MessageInit((gmt_QUEUE far *)Queue, 
		 NUM_OF_RX_QUEUES, 
		 (BYTE far *)B_Buffer, sizeof(B_Buffer), 
		 500, //message sent timeout in ms
		 (WORD (far*)(void))gm_W_GetSystemTime);
	 Lpm_InterruptEnable(gmd_MISSION_MAIL_OUT);

	 
 	 #ifdef IR_SUPPORT	 
	 	Lpm_IrInit(&IrBuffer);
	 #endif

 	 #ifdef CEC_SUPPORT	 
	 if (Configuration.PermanentPeripheralControl)
	 {
		 Lpm_CecInit();
		 Lpm_InterruptEnable(gmd_CEC);
	 }
	 #endif	
	
 	 #ifdef UART_DEBUG_SUPPORT
	 #if (LPM_OCMCLK_DEFAULT != 2)
	 Lpm_UartInit(SYSTEM_TCLK_FREQ, UART_BAUD_RATE, Ba_CommBufUart0, sizeof(Ba_CommBufUart0));
	 #else
	 Lpm_UartInit(MISSION_OCMCLK_FREQ, UART_BAUD_RATE, Ba_CommBufUart0, sizeof(Ba_CommBufUart0));
	 #endif
 	 #endif
	 
	 #ifdef KEYPAD_SUPPORT		 
	 Lpm_KeypadInit();
 	 #endif
	 
 	 #ifdef I2C_MASTER_SUPPORT
	 Lpm_I2CMasterInit();
 	 #endif

 	 #ifdef I2C_SLAVE_SUPPORT
	 //Lpm_I2CSlaveInitAll();  //Init I2C slave as entering low power mode
	 //Lpm_CsmI2CMuxSelect(MUX_DDC2_LpmI2cSlv2_MisHdmiCp);
	 #endif

	 #ifdef TOUCHPAD_SUPPORT  
	 Lpm_TouchPadInit();
	 #endif

 	 #ifdef TCLK_TIMER_SUPPORT
	 Lpm_TclkTimerInit(SYSTEM_TCLK_FREQ);
	 Lpm_TclkTimerReset();
 	 #endif 

	 #ifdef POWER_MODE_HANDLE
	 Lpm_PowerModeInit();
	 #endif
	 
	 #ifdef BROWN_OUT_SUPPORT
	 Lpm_BrownOutInit();
	 #endif

	 #ifdef CABLE_DETECT_SUPPORT
	 Lpm_CableDetectHandlerInit();	 
	 #endif

    gm_SetRegBitsWord(LPM_MICRO_CONTROL, LPM_HSPAD_HYST | LPM_VSPAD_HYST); // Enable Schmitt Trigger for ADC H/V sync
}

//******************************************************************************
// FUNCTION:   void Lpm_ResetLpmDevices(gmt_BusOwn PBusOwner)
// USAGE:      Reset LPM device when LPM clock source is changed
// INPUT:      PBusOwner
// OUTPUT:     None
// GLOBALS:    None
// USED_REGS:  LPM_MICRO_CONTROL
//******************************************************************************
void Lpm_ResetLpmDevices(gmt_BusOwn PBusOwner)
{

   //Reset the PBus devices
   if(PBusOwner == BUS_LPM)   
   {
      gm_SetRegBitsWord(LPM_C0_AUX_STROBE, DPRX_AUX2OCM_RESET);
      gm_SetRegBitsWord(LPM_C2_AUX_STROBE, DPRX_AUX2OCM_RESET);  
      gm_SetRegBitsWord(LPM_C0_AUX2I2C_RESET, DPRX_AUX2I2C_RESET);
      gm_SetRegBitsWord(LPM_C2_AUX2I2C_RESET, DPRX_AUX2I2C_RESET);  
      gm_SetRegBitsWord(LPM_MICRO_CONTROL, I2C_SLAVE_RESET);
   } 
   //Reset other devices
   gm_SetRegBitsWord(LPM_MICRO_CONTROL, LPM_CEC_IR_SOFT_RESET|I2C_MASTER_RESET);
   gm_SetRegBitsWord(LPM_TCLK_TIMER_CTRL, TCLK_TIMER_RESET);


   Lpm_Delay1ms(1);

   if(PBusOwner == BUS_LPM)
   {
      gm_ClearRegBitsWord(LPM_C0_AUX_STROBE, DPRX_AUX2OCM_RESET);
      gm_ClearRegBitsWord(LPM_C2_AUX_STROBE, DPRX_AUX2OCM_RESET);
      gm_ClearRegBitsWord(LPM_C0_AUX2I2C_RESET, DPRX_AUX2I2C_RESET);
      gm_ClearRegBitsWord(LPM_C2_AUX2I2C_RESET, DPRX_AUX2I2C_RESET);         
      gm_ClearRegBitsWord(LPM_MICRO_CONTROL, I2C_SLAVE_RESET);  
   }
   gm_ClearRegBitsWord(LPM_MICRO_CONTROL, LPM_CEC_IR_SOFT_RESET|I2C_MASTER_RESET);  
   gm_ClearRegBitsWord(LPM_TCLK_TIMER_CTRL, TCLK_TIMER_RESET);    

   #ifdef I2C_SLAVE_SUPPORT
   Lpm_I2CSlaveInitAll();
   #endif
   #ifdef I2C_MASTER_SUPPORT
  // Lpm_I2CMasterInit();
   #endif   
   #ifdef IR_SUPPORT
   Lpm_IrInit(&IrBuffer);
   #endif

   #ifdef AUX_RX_SUPPORT
   Lpm_AuxRxInit(AUX_CH_C0);
   #ifdef AUX_RX_C2_SUPPORT
   Lpm_AuxRxInit(AUX_CH_C2);
   #endif
   #endif
}

//******************************************************************************
// FUNCTION:   void Lpm_SharedPBusSwitch(gmt_BusOwn PBusOwner)
// USAGE:      PBus own switch
// INPUT:      PBusOwner
// OUTPUT:     None
// GLOBALS:    None
// USED_REGS:  LPM_MICRO_CONTROL
//******************************************************************************
void Lpm_SharedPBusSwitch(gmt_BusOwn PBusOwner)
{
	if(PBusOwner == BUS_MISSION)	//PBus access will be controlled by Mission
	{
		gm_ClearRegBitsWord(LPM_MICRO_CONTROL, LPM_PBUS_OWN);
	}
	else if(PBusOwner == BUS_LPM)	//PBus access will be controlled byd LPM
	{
		gm_SetRegBitsWord(LPM_MICRO_CONTROL, LPM_PBUS_OWN);
	}
}

//******************************************************************************
// FUNCTION:   void Lpm_SharedPBusHandler(void)
// USAGE:      PBus ownership handler
// INPUT:      None
// OUTPUT:     None
// GLOBALS:    None
// USED_REGS:  LPM_MICRO_CONTROL
//******************************************************************************
void Lpm_SharedPBusHandler(void)
{
   #define PBUS_OWN_REQ_INV   1  // 1: To avoid PBus switch to LPM as executing reset 0
   if(PBusReqLock == FALSE)
   {
   	if(gm_ReadRegByte(LPM_MICRO_STATUS) & LPM_PBUS_OWN_REQ)
   	{
   	   #if PBUS_OWN_REQ_INV
         Lpm_SharedPBusSwitch(BUS_LPM);
         #else
   		Lpm_SharedPBusSwitch(BUS_MISSION);
         #endif
   	}
   	else
   	{
   	   #if PBUS_OWN_REQ_INV
         Lpm_SharedPBusSwitch(BUS_MISSION);
         #else   	
   	   Lpm_SharedPBusSwitch(BUS_LPM);
         #endif
   	}
   }
}

//******************************************************************************
// FUNCTION:   void Lpm_SharedHostBusHandler(void)
// USAGE:      Host bus ownership handler
// INPUT:      None
// OUTPUT:     None
// GLOBALS:    None
// USED_REGS:  LPM_MICRO_CONTROL
//******************************************************************************
void Lpm_SharedHostBusHandler(void)
{
	gmt_BusOwn HostBusOwner;
	
	//The Host Bus access is controlled by LPM as default
	if (gm_MessageReceived(gmd_MESSAGE_HOSTBUS, &HostBusOwner, sizeof(HostBusOwner)))
	{
		if(HostBusOwner == BUS_MISSION)
		{
			gm_ClearRegBitsWord(LPM_MICRO_CONTROL, LP_RUN);
			//gm_Print("Host Bus owned by Mission",0);
		}
		else
		{
			gm_SetRegBitsWord(LPM_MICRO_CONTROL, LP_RUN);
			//gm_Print("Host Bus owned by LPM",0);
		}
	}
}

//******************************************************************************
// FUNCTION:   void Lpm_CalibrationHandler(void)
// USAGE:      Calibration initialization
// INPUT:      None
// OUTPUT:     None
// GLOBALS:    None
// USED_REGS:  None
//******************************************************************************
void Lpm_CalibrationHandler(void)
{
   #if USE_CALIBRATED_TRIM
   #define CLKMEAS_27MHz   0x3FF
   WORD CalibratedData[2];
   if (gm_MessageReceived(gmd_MESSAGE_CALIBRATION, CalibratedData, sizeof(CalibratedData)))
   {   
      RcOscTrim = CalibratedData[0] & 0x07; // 3-bit trim value
      MeasRcOscVal = CalibratedData[1];
      LpmRcOscClk = SYSTEM_TCLK_FREQ/CLKMEAS_27MHz*MeasRcOscVal;
      #if (LPM_OCMCLK_DEFAULT == 0)
      Lpm_ClockSwitch2RCOSC(TCLK_DIV_0);   
      #else
      gm_WriteRegWord(LPM_RCOSC, (RcOscTrim << LPM_RC_OSC_TRIM_SHIFT)|LPM_RC_OSC_EN);
      #endif
      gm_Print("Calibrated RC-OSC Trim = 0x%x", RcOscTrim);
      gm_Print("Measured RC-OSC Value = 0x%x", MeasRcOscVal);
      gm_Print("Measured RC-OSC Clock = %d000Hz", (WORD)(LpmRcOscClk/1000));
   }
   #endif
}

//******************************************************************************
// FUNCTION:   void Lpm_GpioControlHandler(void)
// USAGE:      Set the LPM_GPIO value assigned from Mission (for LED, DDVI Mux control)
// INPUT:      None
// OUTPUT:     None
// GLOBALS:    None
// USED_REGS:  None
//******************************************************************************
void Lpm_GpioControlHandler(void)
{
   WORD  msg[3];  //{reg_action, reg_addr, value}
   
   if ( gm_MessageReceived(gmd_MESSAGE_LPM_GPIO_CTRL, &msg[0], sizeof(msg)) )
   {
      //Program specific LPM_GPIO
      if (msg[0] == Clear_bit) 
      {
         gm_ClearRegBitsWord(msg[1], msg[2]);
      }
      else if(msg[0] == Set_bit) 
      {
         gm_SetRegBitsWord(msg[1], msg[2]);
      }
      else if(msg[0] == Write_Word) 
      {
         gm_WriteRegWord(msg[1], msg[2]);  
      }
      else if(msg[0] == Read_Word) 
      {
         WORD value = gm_ReadRegWord(msg[1]);
         gm_MessageSend(gmd_MESSAGE_LPM_GPIO_CTRL, &value, sizeof(value));
      }
   }
}

//******************************************************************************
// FUNCTION:   void Lpm_DebugCmdHandler(void)
// USAGE:      Handle the command from Mission for debug usage
// INPUT:      None
// OUTPUT:     None
// GLOBALS:    None
// USED_REGS:  None
//******************************************************************************
void Lpm_DebugCmdHandler(void)
{
   WORD  command[3];//{RW, reg_addr, value}
   
   if ( gm_MessageReceived(gmd_MESSAGE_DEBUG, &command[0], sizeof(command)) )
   {
      if (command[0] == 1) //write specific register
      {
         gm_WriteRegWord(command[1], command[2]);
      }
      else if (command[0] == 0)  //read specific register
      {
         WORD value = gm_ReadRegWord(command[1]);
         if ( gm_MessageSend(gmd_MESSAGE_DEBUG, &value, sizeof(value)) )
         {
            //gm_Print("--address 0x%x", command[1]);            
            //gm_Print("--value = 0x%x", value);
         }
      }
#ifdef LPM_RESET_SUPPORT
      else if (command[0] == 2)  //reset LPM
      {
         Lpm_ResetLpm();
      }
#endif      
      
   }
}

//******************************************************************************
// FUNCTION:   void Lpm_CheckStatusCmdHandler(void)
// USAGE:      Handle the Check Status command from Mission
// INPUT:      None
// OUTPUT:     None
// GLOBALS:    None
// USED_REGS:  None
//******************************************************************************
void Lpm_CheckStatusCmdHandler(void)
{
   BYTE  command[1];
   
   if ( gm_MessageReceived(gmd_MESSAGE_CHECK_STATUS, &command[0], sizeof(command)) )
   {
      if (command[0] == 0)  //Mission requests {power mode switch, wakeup detected ch.} during ApplicationInit
      {
         BYTE value[3];
         
         if(IsSyncDetected)
         {
            IsSyncDetected = FALSE;         
            value[0] = PWR_SW_SIGDET_ON;
         }
         else
         {
            if((BoardType==BOARD_4K2K) && (ChipID4k2K != CHIPID_4K2K_R))
               value[0] = PWR_SW_ON;
            else   
               value[0] = Lpm_NvramReadByte(NVRAM_OFFSET_PWR_MODE);
         }
         
         value[1] = WakeupFrom;
         value[2] = Lpm_GetWakeupExtraInfo(WakeupFrom);

         if ( gm_MessageSend(gmd_MESSAGE_CHECK_STATUS, &value[0], sizeof(value)) )
         {
            //gm_Print("--power switch = %d", value[0]);            
            //gm_Print("--wakeup from = %d", value[1]);
         }

         gm_ClearRegBitsWord(LPM_MICRO_CONTROL, LPM_RST_OVR);  //Clear RST_OVR to avoid reset key can't work
      }
      else if (command[0] == 1)  //Mission requests cable status
      {
         CableUpdateRequest = TRUE;  
      }
   }
}

//******************************************************************************
// FUNCTION:   void Lpm_MissionTaskHandler(void)
// USAGE:      Handle Mission's requests from mailbox
// INPUT:      None
// OUTPUT:     None
// GLOBALS:    None
// USED_REGS:  None
//******************************************************************************
void Lpm_MissionTaskHandler(void)
{
	Lpm_CalibrationHandler();
	Lpm_SharedPBusHandler();
	Lpm_SharedHostBusHandler();   
	Lpm_GpioControlHandler();
	Lpm_CheckStatusCmdHandler();
	Lpm_DebugCmdHandler();
	RegMsgHandler();
}

//******************************************************************************
// FUNCTION:   void Lpm_StackMonitorInit(void)
// USAGE:      Fill the stack area with a constant pattern to be able to monitor a stack watermark
// INPUT:      None
// OUTPUT:     None
// GLOBALS:    None
// USED_REGS:  None
//******************************************************************************
void Lpm_StackMonitorInit(void)
{
   #define LPM_STACK_INIT_LEN  (LPM_STACK_SIZE - 0x100) //reserved 256 bytes for code start usage
   
   DWORD DW_Addr = LPM_STACK_STOP;
   WORD W_Cnt = (WORD)LPM_STACK_INIT_LEN;

   while(W_Cnt)
   {
      *LPM_LINEAR_TO_FPTR_W(DW_Addr) = LPM_STACK_MARK;
      DW_Addr += 2;
      W_Cnt -= 2;
   }
}

//******************************************************************************
//
// FUNCTION:   void far Lpm_Main(void)
//
// USAGE:      Initialize low power mode, call various handlers and keep track
//               of the status of the low power state machine.
//
// INPUT:      None
//
// OUTPUT:      None
//
// GLOBALS:      None
//
// USED_REGS:   LPM_MICRO_CONTROL   (R/W)
//            LPM_MICRO_STATUS   (R)
//
// PRE_CONDITION  : 
//
// POST_CONDITION : 
//
//******************************************************************************
void far Lpm_Main(void)
{
   asm cli;

   Lpm_StackMonitorInit();

   //Enable PER_SWITCH_MODE to make sure the shared host bus is idle then do the switch
   //According to designer, enable PER_SWITCH_MODE first then set LP_RUN
   //Set PBus to LPM first to pass the initializations of PBus HW modules
   gm_SetRegBitsWord(LPM_MICRO_CONTROL, PER_SWITCH_MODE);
   gm_SetRegBitsWord(LPM_MICRO_CONTROL, LPM_PBUS_OWN);//switch PBus to LPM
   gm_SetRegBitsWord(LPM_MICRO_CONTROL, LP_RUN);      //switch Host Bus to LPM 

   Lpm_SystemInit();

   //Move PBus switching control to Lpm_SharedPBusHandler(), wait Mission to send PBus request
   //gm_ClearRegBitsWord(LPM_MICRO_CONTROL, LPM_PBUS_OWN);  //Switch the PBus ownership back to Mission
   
   gm_WriteRegWord(LPM_MICRO_CONTROL_OUT, 0xA5A5); //mission is waiting for LPM's response
   //Set LP_SRAM_LOADED to inform Mission LPM code is running and inits are complete
   gm_SetRegBitsWord(LPM_MICRO_CONTROL, BIT0/*LP_SRAM_LOADED*/);	

   Lpm_MfpInit();

   gm_RegMsgInit();
   
   #ifdef UART_DEBUG_SUPPORT
   gm_Print("LPM Main Loop Start!",0);
   #endif
	
   while(TRUE)
   {	
      if (!DebugOnly)   //DebugOff
      {
      	static BOOL HdrCount=0;
      	BOOL HdrProcessed;

         Lpm_MissionTaskHandler();
      	#ifdef POWER_MODE_HANDLE
      	Lpm_PowerModeHandler();
      	#endif   	
         
      	HdrProcessed=FALSE;
      	while(HdrProcessed==FALSE)
      	{
      		if (aB_HandlersFlags[HdrCount]==TRUE)
      		{
      			aPf_HandlersRoutines[HdrCount]();
      			HdrProcessed=TRUE;
      		}
      		HdrCount++;
      		if(HdrCount==LpmHdrID_END)
      		{
      			HdrCount=0;
      			HdrProcessed=TRUE;				
      		}
      	}
      	//Lpm_TestHandler();
      }
      else  //DebugOn
      {
      	Lpm_SystemDebug();		
      	#ifdef I2C_SLAVE_SUPPORT
      	Lpm_I2CSlaveHandler();
      	#endif	      
      }

      #ifdef BROWN_OUT_SUPPORT
      Lpm_BrownOutReTrigger();
      #endif
   }
}

void Lpm_TestHandler(void)
{
#if 0 //Kam debug mailbox 
   	if(gm_ReadRegByte(0xdc66)==3)		// set low power mode , OCM 216KHz , Uart baudrate 9600
   	{
   		gm_SetRegBitsWord(0xDC30,LPM_PBUS_OWN);		
   		gm_WriteRegWord(LPM_RCOSC,0x05);
   		
   		#if (SYSTEM_SLEEP_TCLK_FREQ == 27000000L)
   			gm_WriteRegWord(LPM_CLOCK_CTRL,0x04);
   		#elif (SYSTEM_SLEEP_TCLK_FREQ == 13500000L)
   			gm_WriteRegWord(LPM_CLOCK_CTRL,0x14);
   		#elif (SYSTEM_SLEEP_TCLK_FREQ ==  6750000L)
   			gm_WriteRegWord(LPM_CLOCK_CTRL,0x24);
   		#elif (SYSTEM_SLEEP_TCLK_FREQ ==  3380000L)
   			gm_WriteRegWord(LPM_CLOCK_CTRL,0x34);	
   		#elif (SYSTEM_SLEEP_TCLK_FREQ ==  1690000L)
   			gm_WriteRegWord(LPM_CLOCK_CTRL,0x44);
   		#elif (SYSTEM_SLEEP_TCLK_FREQ ==   843000L)
   			gm_WriteRegWord(LPM_CLOCK_CTRL,0x54);
   		#elif (SYSTEM_SLEEP_TCLK_FREQ ==   421000L)
   			gm_WriteRegWord(LPM_CLOCK_CTRL,0x64);
   		#elif (SYSTEM_SLEEP_TCLK_FREQ ==   210000L)
   			gm_WriteRegWord(LPM_CLOCK_CTRL,0x74);
         #endif
   		
   		Lpm_TimerInit(SYSTEM_SLEEP_TCLK_FREQ);
         #ifdef UART_DEBUG_SUPPORT
    		Lpm_UartInit(SYSTEM_SLEEP_TCLK_FREQ, 19200L, Ba_CommBufUart0, sizeof(Ba_CommBufUart0));
         #endif

   		gm_SetRegBitsWord(LPM_AFE_CTRL,LPM_AFE_EN_ST_ASE|LPM_AFE_EN_ST_ASE_HS);
   		gm_SetRegBitsWord(LPM_PD_CONTROL,LPM2AFE_PD);
   		gm_WriteRegByte(0xdc66,0x0);
   	}

   	if(gm_ReadRegByte(0xdc66)==4)		// set low power mode , OCM 216KHz , Uart baudrate 9600
   	{
   		gm_SetRegBitsWord(0xDC30,LPM_PBUS_OWN);	
   		Lpm_SignalDetectInit();
   		gm_WriteRegByte(0xdc66,0x0);
   	}

   	if(gm_ReadRegByte(0xdc66)==5)		// set low power mode , OCM 216KHz , Uart baudrate 9600
   	{
         #ifdef SIGNALING_DETECT    	
   		Lpm_SignalDetectStop();
         #endif
   		gm_WriteRegByte(0xdc66,0x0);
   	}		
#endif 

#if 0
if(gm_ReadRegByte(0xdc66)==5)
{
	// Set Watch dog counter to 0x3FFFC00 * 37ns = 2.48 sec
	gm_WriteRegWord(LPM_OCM_SW_WDT_VAL ,0xffff); 
	
	//When set to 0 , this will enable the RESET Generation.
	gm_ClearRegBitsByte(LPM_OCM_SW_WDT_CTRL,OCM_SOFT_WDT_TEST);

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
   
	//Mask IRQ from LPM Software WDT. 
	gm_SetRegBitsWord(LPM_IRQ_CONTROL ,LPM_SWDT_IRQ_MASK);
   
	//Enables the Software Watchdog Timer. On Timeout, this circuit will generate a reset 
	//Add Test mode , not reset 
	gm_SetRegBitsByte(LPM_OCM_SW_WDT_CTRL,EN_OCM_SW_WDT);	
}
#endif

}

//******************************************************************************

