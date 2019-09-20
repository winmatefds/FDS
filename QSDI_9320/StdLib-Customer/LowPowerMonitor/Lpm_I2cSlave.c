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
// MODULE:      Lpm_I2cSlave.c
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
#include <mem.h>

#ifdef I2C_SLAVE_SUPPORT

#define I2C_WRITE_FIFO_RESET	0
#define USE_DMA_FOR_RFIFO	0	//No I2C_SLAVE_DMA_RFIFO_EN in Athena LPM

//****************************************************************************** 
//
//  G L O B A L    V A R I A B L E S 
//
//****************************************************************************** 
extern BYTE gB_DDC2BiDevId; // = 0x6E;
extern BYTE gB_GprobeCmdSet; // = 0xC2;
extern WORD gW_ManufacturerID; // = 0;
extern BOOL gB_DdcciWaitForParsing[I2C_SLAVE_NUM_CH];

MODE_STATUS_t B_ModeStatus = INTERNAL_ROM_MODE;

I2C_SLAVE_TYPE far* Sp_I2CSlavePtrGlobal[I2C_SLAVE_NUM_CH] = {NULL_PTR,NULL_PTR,NULL_PTR,NULL_PTR};
BYTE far(* I2CSlaveExtHandlerFuncPtr)(SLAVE_CH B_SlaveCh) = NULL_PTR;
I2C_COMM_HANDLER_TYPE Fp_CorrespondingFuncPtr = NULL_PTR;

BYTE B_DDCToAuxStatus = DDC_TO_AUX_FREE;
BYTE B_SlvStatus[I2C_SLAVE_NUM_CH] = {0,0,0,0};
BYTE B_SlvStatus2[I2C_SLAVE_NUM_CH] = {0,0,0,0};
I2C_SLAVE_TYPE I2CSlaveTable[I2C_SLAVE_NUM_CH]={{0},{0},{0},{0}};
BYTE gB_Ddc2Bi_I2C_Status[I2C_SLAVE_NUM_CH];
volatile BYTE gB_I2cSlaveSel = I2C_SLAVE4_CH;//default select I2C_SLAVE4 (VGA), used for select specific I2C_SLAVE register
volatile BYTE B_DevIdReady = FALSE;

#if USE_DMA_FOR_RFIFO
BYTE B_DMAEn = 0;
#endif

//BYTE ROM BA_RX_I2C_SW_DEV_ID[2] = {0x86, 0x84}; 
#if (LPM_EDID_ACCESS_SEL == 2)
extern DprxVirtualEdid_t DprxVirtualEdid;
BYTE LpmEdidWaitProc = 0;
#endif

BYTE I2cBufOverflow = FALSE;

//****************************************************************************** 
//  L O C A L    D E F I N I T I O N S 
//****************************************************************************** 
#define I2C_SLAVE_CLEAR_REG_STATUS() {gm_ClearRegBitsByte(I2C_SLAVE_CTRL1, I2C_SLAVE_EN); gm_SetRegBitsByte(I2C_SLAVE_CTRL1, I2C_SLAVE_EN);}
#define I2C_SLAVE_ADDR_COUNT  		8
#define AUX_I2C_ADDR_MASK   		0x007f  

#define I2C_Ddcci_Length(CH)			((Sp_I2CSlavePtrGlobal[CH]->Buffer[1])&0x7F)
#define I2C_as_Gprobe_Cmd(CH)			(Sp_I2CSlavePtrGlobal[CH]->Buffer[2])
#define I2C_ManufacturerID(CH)			(*(WORD*)(&(Sp_I2CSlavePtrGlobal[CH]->Buffer[3])))


//******************************************************************************
//
//  C O D E
//
//******************************************************************************
//******************************************************************************
// FUNCTION:   Lpm_I2CSlaveInitAll
// USAGE:      Initialization of All I2C slave channels
// INPUT:      None
// OUTPUT:     None
// GLOBALS:
// USED_REGS:  None
//******************************************************************************
void far Lpm_I2CSlaveInitAll (void)
{
	SLAVE_CH B_SlaveCh;
	WORD W_I2cResetDelay = 0x50;	//delay ~50us
	
	//gB_DDC2BiDevId =  BA_RX_I2C_SW_DEV_ID[B_I2C_DEV_INDEX];
	gB_DDC2BiDevId = DDC2BI_ADDR_ID;
	gm_SetRegBitsWord(LPM_MICRO_CONTROL,I2C_SLAVE_RESET);
	while(W_I2cResetDelay--){}	//Wait for I2C slave reset
	gm_ClearRegBitsWord(LPM_MICRO_CONTROL,I2C_SLAVE_RESET);

	#ifndef UART_DEBUG_SUPPORT
	for(B_SlaveCh=I2C_SLAVE1_CH; B_SlaveCh<I2C_SLAVE_NUM_CH ; B_SlaveCh++)
	#else
	for(B_SlaveCh=I2C_SLAVE1_CH; B_SlaveCh<=I2C_SLAVE3_CH ; B_SlaveCh++)
	#endif
	{
		Lpm_I2CSlaveSetChannel(B_SlaveCh);
		Lpm_I2CSlaveInit((BYTE *)&I2CSlaveTable[B_SlaveCh], I2C_SLAVE_RD_LEN, B_SlaveCh);
		Lpm_I2CSlaveAddCommHandler((I2C_SLAVE_HANDLER_TYPE)Lpm_I2CSlaveGProbeHandler, gB_DDC2BiDevId, B_SlaveCh);
		//Init DDC2Bi device ID for all I2C slave channels
		Lpm_I2CSlaveAddI2CAddress(gB_DDC2BiDevId, B_SlaveCh);	
	}

	#if (LPM_EDID_ACCESS_SEL == 2)   //Use I2C SLAVE1 as virtual EDID (FW)
	{
		Lpm_I2CSlaveAddCommHandler((I2C_SLAVE_HANDLER_TYPE)Lpm_I2CSlaveEdidHandler, NVRAM_DEVICE, I2C_SLAVE1_CH);
		Lpm_I2CSlaveAddI2CAddress(NVRAM_DEVICE, I2C_SLAVE1_CH);	
	} 
	#endif

	//Set GPIO15/16 for I2C Slave1, GPIO13/14 for I2C Slave2, GPIO11/12 for I2C Slave3

   /*ChengLin 20130419: For 4k2k board, Athena R don't disable GPIO13 and GPIO14 for avoid DP Mux fail to switch ports*/
   if((BoardType==BOARD_4K2K) && (ChipID4k2K == CHIPID_4K2K_R))
   {
	   gm_ClearRegBitsWord(LPM_GPIO_ENABLE1, (LPM_GPIO11_EN|LPM_GPIO12_EN));
   }
   else
   {
	   gm_ClearRegBitsWord(LPM_GPIO_ENABLE1, (LPM_GPIO11_EN|LPM_GPIO12_EN|LPM_GPIO13_EN|LPM_GPIO14_EN));
   }
	gm_ClearRegBitsWord(LPM_GPIO_ENABLE2, (LPM_GPIO15_EN|LPM_GPIO16_EN));	

	#ifndef UART_DEBUG_SUPPORT
	//Set GPIO 1/2 for I2C Slave4   
	gm_ClearRegBitsWord(LPM_GPIO_ENABLE1, (LPM_GPIO1_EN|LPM_GPIO2_EN));
	gm_ClearRegBitsWord(LPM_MICRO_CONTROL, EN_LPM_UART);
	#endif

	//Enable I2C slave IRQ1~4
	Lpm_InterruptEnable(gmd_DDC1);
	Lpm_InterruptEnable(gmd_DDC2);
	Lpm_InterruptEnable(gmd_DDC3);
	#ifndef UART_DEBUG_SUPPORT
	Lpm_InterruptEnable(gmd_DDC4);		
	#endif
}

//******************************************************************************
// FUNCTION:   Lpm_I2CSlaveInit
// USAGE:      Initialization of I2C slave hardware, buffer, device ID and handler
// INPUT:      BYTE* Bp_StartAddr - pointer to the buffer
// OUTPUT:     WORD - how many bytes need for this module
// GLOBALS:
// USED_REGS:  None
//******************************************************************************
void far Lpm_I2CSlaveInit (BYTE* Bp_StartAddr, BYTE B_AuxRdLen, SLAVE_CH B_SlaveCh)
{

   if (Bp_StartAddr != NULL_PTR)
   {   
      I2C_SLAVE_TYPE far* Sp_I2CSlavePtr;
	  
      Lpm_I2CSlaveSetChannel(B_SlaveCh);
      Sp_I2CSlavePtrGlobal[B_SlaveCh] = (I2C_SLAVE_TYPE*)Bp_StartAddr;
      Sp_I2CSlavePtr = Sp_I2CSlavePtrGlobal[B_SlaveCh];

      memset((void *)Sp_I2CSlavePtr, 0, sizeof(I2C_SLAVE_TYPE));

      Sp_I2CSlavePtr->State = I2C_SLAVE_STATE_FREE;
      Sp_I2CSlavePtr->HandlerState = I2C_SLAVE_HANDLER_FREE;
      Sp_I2CSlavePtr->B_RdLen = B_AuxRdLen;

      gm_WriteRegByte(I2C_SLAVE_CTRL1, 0);
      gm_WriteRegByte(I2C_SLAVE_CTRL1, I2C_SLAVE_EN);
      gm_WriteRegByte(I2C_SLAVE_CTRL1, I2C_SLAVE_EN | I2C_SLAVE_RFIFO_nE_IRQ_EN);

      gm_WriteRegByte(I2C_SLAVE_CTRL2, I2C_SLAVE_STOP_EN | I2C_SLAVE_CLOCK_STRETCH_EN | I2C_SLAVE_REF_CLK_DIV /*| I2C_SLAVE_FIFO_MODE*/);
      gm_WriteRegByte(I2C_SLAVE_CTRL3, I2C_SLAVE_STRETCH_OUT_IRQ_EN | I2C_SLAVE_DEV_ID_IRQ_EN | I2C_SLAVE_RD_REQ_METHOD);
   }

}

//******************************************************************************
// FUNCTION:   Lpm_I2CSlaveGetAddrBuffer
// USAGE:      get pointer to the buffer
// INPUT:      None
// OUTPUT:     None
// GLOBALS:    None
// USED_REGS:  None
//******************************************************************************
I2C_SLAVE_TYPE far* Lpm_I2CSlaveGetAddrBuffer(SLAVE_CH B_SlaveCh)
{
   return Sp_I2CSlavePtrGlobal[B_SlaveCh];
}

//******************************************************************************
// FUNCTION:   Lpm_I2CSlaveSetChannel
// USAGE:      select i2c slave channel for its own registers and settings
// INPUT:      None
// OUTPUT:     None
// GLOBALS:    None
// USED_REGS:  None
//******************************************************************************
void far Lpm_I2CSlaveSetChannel(SLAVE_CH B_SlaveCh)
{
   gB_I2cSlaveSel = B_SlaveCh;
}

//******************************************************************************
// FUNCTION:   Lpm_I2CSlaveGetChannel
// USAGE:      select i2c slave channel for its own registers and settings
// INPUT:      None
// OUTPUT:     None
// GLOBALS:    None
// USED_REGS:  None
//******************************************************************************
SLAVE_CH far Lpm_I2CSlaveGetChannel(void)
{
   return gB_I2cSlaveSel;
}

//******************************************************************************
// FUNCTION:   Lpm_I2CSlaveAddI2CAddress
// USAGE:      add new device with parameter address
// INPUT:      BYTE B_I2CAddr - I2C address
// OUTPUT:     None
// GLOBALS:
// USED_REGS:  None
//******************************************************************************
void far Lpm_I2CSlaveAddI2CAddress(BYTE B_I2CAddr, SLAVE_CH B_SlaveCh)
{
	BYTE B_Addr2   = B_I2CAddr >> 1;   
	BYTE B_TBLSel, B_TBLBit32, B_TBLNum, B_TBLBit8;
	WORD W_TBLNumAddr;

	Lpm_I2CSlaveSetChannel(B_SlaveCh);
	B_TBLSel  = B_Addr2 / 32;	//32bits TBL segement
	B_TBLBit32 = B_Addr2 % 32;		//which bit of TBL bit[31:0]
	B_TBLNum   = B_TBLBit32 / 8;	//map to which ADDR_TBL?
	B_TBLBit8  = B_TBLBit32 % 8;	//which bit of TBLx_bit[7:0]
	W_TBLNumAddr = I2C_SLV_ADDR_TBL0 + (B_TBLNum << 1);	//Diff of each TBL addr is 2

	gm_ClearAndSetBitsByte(I2C_SLAVE_CTRL3, I2C_SLAVE1_ADDR_TBL_SEL, B_TBLSel << I2C_SLAVE1_ADDR_TBL_SEL_SHIFT);
	gm_SetRegBitsByte(W_TBLNumAddr, (1 << B_TBLBit8));
}

//******************************************************************************
// FUNCTION:   Lpm_I2CSlaveDisableI2CAddress
// USAGE:      Remove new device with parameter address
// INPUT:      BYTE B_I2CAddr - I2C address
// OUTPUT:     None
// GLOBALS:
// USED_REGS:  None
//******************************************************************************
void far Lpm_I2CSlaveRemoveI2CAddress(BYTE B_I2CAddr, SLAVE_CH B_SlaveCh)
{
	BYTE B_Addr2   = B_I2CAddr >> 1;   
	BYTE B_TBLSel, B_TBLBit32, B_TBLNum, B_TBLBit8;
	WORD W_TBLNumAddr;

	Lpm_I2CSlaveSetChannel(B_SlaveCh);
	B_TBLSel  = B_Addr2 / 32;	//32bits TBL segement
	B_TBLBit32 = B_Addr2 % 32;		//which bit of TBL bit[31:0]
	B_TBLNum   = B_TBLBit32 / 8;	//map to which ADDR_TBL?
	B_TBLBit8  = B_TBLBit32 % 8;	//which bit of TBLx_bit[7:0]
	W_TBLNumAddr = I2C_SLV_ADDR_TBL0 + (B_TBLNum << 1);	//Diff of each TBL addr is 2

	gm_ClearAndSetBitsByte(I2C_SLAVE_CTRL3, I2C_SLAVE1_ADDR_TBL_SEL, B_TBLSel << I2C_SLAVE1_ADDR_TBL_SEL_SHIFT);
	gm_ClearRegBitsByte(W_TBLNumAddr, (1 << B_TBLBit8));
}

//******************************************************************************
// FUNCTION:   Lpm_I2CSlaveWriteToFifo
// USAGE:      write data to I2C slave FIFO
// INPUT:      None
// OUTPUT:     None
// GLOBALS:
// USED_REGS:  None
//******************************************************************************
void far Lpm_I2CSlaveWriteToFifo(SLAVE_CH B_SlaveCh)
{
   I2C_SLAVE_TYPE far* Sp_I2CSlavePtr = Sp_I2CSlavePtrGlobal[B_SlaveCh];

   Lpm_I2CSlaveSetChannel(B_SlaveCh);
   
   //gm_Print("Lpm_I2CSlaveWriteToFifo",0);
   
   while((gm_ReadRegByte(I2C_SLAVE_STATUS) & I2C_SLAVE_WRITE_FIFO_NF ) != 0)
   {
      if(Sp_I2CSlavePtr->TxPtr == Sp_I2CSlavePtr->W_TrxCount)
      {
         Sp_I2CSlavePtr->State = I2C_SLAVE_STATE_WR_STRETCH;
         break;
      }
	   #if I2C_WRITE_FIFO_RESET
		if(!IsResetWriteFifo())
	   #endif
		{ // put device id
			// FIFO was reseted after START and STOP
			if(Sp_I2CSlavePtr->W_TrxCount==0)
			{
			//gm_Print("I2C_SLAVE_DATA_DevID=0x%x",(Sp_I2CSlavePtr->B_DevID & I2C_ADDRESS_BITS));
				gm_WriteRegByte(I2C_SLAVE_DATA,(Sp_I2CSlavePtr->B_DevID & I2C_ADDRESS_BITS));
			}    
		}
      //gm_Print("I2C_SLAVE_DATA=0x%x",Sp_I2CSlavePtr->Buffer[Sp_I2CSlavePtr->W_TrxCount]);
      gm_WriteRegByte(I2C_SLAVE_DATA, Sp_I2CSlavePtr->Buffer[Sp_I2CSlavePtr->W_TrxCount]);
      Sp_I2CSlavePtr->W_TrxCount++;
      
      if (Sp_I2CSlavePtr->W_TrxCount >= I2C_SLAVE_BUFFER_SIZE)
      {
         Sp_I2CSlavePtr->W_TrxCount = 0;
      }
   }
   gm_WriteRegByte(I2C_SLAVE_STATUS2, I2C_SLAVE_STRETCH_OUT);
}

//******************************************************************************
// FUNCTION:   Lpm_I2CSlaveSetReply
// USAGE:      Set I2C slave reply data
// INPUT:      None
// OUTPUT:     None
// GLOBALS:
// USED_REGS:  None
//******************************************************************************
void far Lpm_I2CSlaveSetReply(BYTE B_Count, SLAVE_CH B_SlaveCh)
{
   #if I2C_WRITE_FIFO_RESET
   BYTE B_DevID;
   #endif
   I2C_SLAVE_TYPE far* Sp_I2CSlavePtr = Sp_I2CSlavePtrGlobal[B_SlaveCh];

   DISABLE_INTERRUPTS;
   Lpm_I2CSlaveSetChannel(B_SlaveCh);
   #if I2C_WRITE_FIFO_RESET
   B_DevID = (Sp_I2CSlavePtr->B_DevID & I2C_ADDRESS_BITS);
   #endif
   Sp_I2CSlavePtr->TxPtr = B_Count + 1;
   Sp_I2CSlavePtr->W_TrxCount = 0;

   #if I2C_WRITE_FIFO_RESET
   if(IsResetWriteFifo())
   {
   gm_WriteRegByte(I2C_SLAVE_DATA, B_DevID);
   }
   #endif
   //Lpm_I2CSlaveWriteToFifo(B_SlaveCh);
   
   Sp_I2CSlavePtr->B_ReplyReady = 1;

   if(
   #if I2C_WRITE_FIFO_RESET
   	IsResetWriteFifo() ||
   #endif
   	(B_DevIdReady==TRUE)
   )
   {
   gm_SetRegBitsByte(I2C_SLAVE_CTRL3, I2C_SLAVE_STRETCH_OUT_IRQ_EN);
   gm_SetRegBitsByte(I2C_SLAVE_CTRL1, I2C_SLAVE_WFIFO_E_IRQ_EN);
   }
   ENABLE_INTERRUPTS;
}

//******************************************************************************
// FUNCTION:   Lpm_I2CSlaveReadFromFifo
// USAGE:      read data from I2C slave FIFO
// INPUT:      None
// OUTPUT:     None
// GLOBALS:
// USED_REGS:  None
//******************************************************************************
void far Lpm_I2CSlaveReadFromFifo_new(SLAVE_CH B_SlaveCh)
{
   BYTE B_Buff = 0;
   I2C_SLAVE_TYPE far* Sp_I2CSlavePtr = Sp_I2CSlavePtrGlobal[B_SlaveCh]; 
   
   Lpm_I2CSlaveSetChannel(B_SlaveCh);
   
   while( (gm_ReadRegByte(I2C_SLAVE_STATUS) & I2C_SLAVE_READ_FIFO_NE ) != 0) 
   {
	   if( (gm_ReadRegByte(I2C_SLAVE_RX_DEV_ID) & I2C_SLAVE_RX_RD_WR) != 0 )
	   {
		 Sp_I2CSlavePtr->B_DevID = gm_ReadRegByte (I2C_SLAVE_DATA);
		 Fp_CorrespondingFuncPtr = Lpm_I2CSlaveHLFindCommHandler(Sp_I2CSlavePtr->B_DevID, B_SlaveCh);
 		  if(Fp_CorrespondingFuncPtr == NULL_PTR)
          {
		 	if ((Sp_I2CSlavePtr->State != I2C_SLAVE_STATE_WR) && (Sp_I2CSlavePtr->State != I2C_SLAVE_STATE_STOP_RD)
               && (B_SlvStatus2[B_SlaveCh] & I2C_SLAVE_START) && (Sp_I2CSlavePtr->State != I2C_SLAVE_STATE_FREE))
		 	{
               Sp_I2CSlavePtr->State = I2C_SLAVE_STATE_STOP_RD;
               Sp_I2CSlavePtr->B_Stop = TRUE;
               Sp_I2CSlavePtr->TxPtr = Sp_I2CSlavePtr->W_TrxCount = 0;
		 	}
          }
          else
          {
               Fp_CorrespondingFuncPtr(B_SlaveCh, Sp_I2CSlavePtr->B_DevID, NULL_PTR);
          }
	      gm_SetRegBitsByte(I2C_SLAVE_CTRL3, I2C_SLAVE_STRETCH_OUT_IRQ_EN);
          gm_WriteRegByte(I2C_SLAVE_STATUS2, I2C_SLAVE_DEV_ID_RECEIVED | I2C_SLAVE_START);
         #if I2C_WRITE_FIFO_RESET
         if(!IsResetWriteFifo())
         #endif
         {
            B_DevIdReady = TRUE; // dev id is received
         }
		  continue;
	   }
	  if(Fp_CorrespondingFuncPtr == NULL_PTR)
      {
         if((Sp_I2CSlavePtr->RxPtr == Sp_I2CSlavePtr->W_RcvCount) && (Sp_I2CSlavePtr->W_AbsRcvCount != 0))
         {
            Sp_I2CSlavePtr->State = I2C_SLAVE_STATE_RD_STRETCH;
            break;
         }
		 
         Sp_I2CSlavePtr->Buffer[Sp_I2CSlavePtr->W_RcvCount] = gm_ReadRegByte(I2C_SLAVE_DATA);
         //gm_Print("Buf=0x%x",Sp_I2CSlavePtr->Buffer[Sp_I2CSlavePtr->W_RcvCount]);
         Sp_I2CSlavePtr->W_AbsRcvCount++;
         Sp_I2CSlavePtr->W_RcvCount++;
         if (Sp_I2CSlavePtr->W_RcvCount >= I2C_SLAVE_BUFFER_SIZE)
         {
            Sp_I2CSlavePtr->W_RcvCount = 0;
         }
		 
      }
      else
      {
		 B_Buff	= gm_ReadRegByte (I2C_SLAVE_DATA);
         if(Fp_CorrespondingFuncPtr(B_SlaveCh, Sp_I2CSlavePtr->B_DevID, &B_Buff) == FALSE)
         {
            Sp_I2CSlavePtr->State = I2C_SLAVE_STATE_RD_STRETCH;
            break;
         }
      }
   }
   gm_WriteRegByte(I2C_SLAVE_STATUS2, I2C_SLAVE_STRETCH_IN);
}


//******************************************************************************
// FUNCTION:   Lpm_I2CSlaveReadFromFifo
// USAGE:      read data from I2C slave FIFO
// INPUT:      None
// OUTPUT:     None
// GLOBALS:
// USED_REGS:  None
//******************************************************************************
void far Lpm_I2CSlaveReadFromFifo(SLAVE_CH B_SlaveCh)
{
   BYTE B_Buff;
   I2C_SLAVE_TYPE far* Sp_I2CSlavePtr = Sp_I2CSlavePtrGlobal[B_SlaveCh]; 
   
   Lpm_I2CSlaveSetChannel(B_SlaveCh);
   
   while( (gm_ReadRegByte(I2C_SLAVE_STATUS) & I2C_SLAVE_READ_FIFO_NE ) != 0) 
   {
      if(Fp_CorrespondingFuncPtr == NULL_PTR)
      {
         if((Sp_I2CSlavePtr->RxPtr == Sp_I2CSlavePtr->W_RcvCount) && (Sp_I2CSlavePtr->W_AbsRcvCount != 0))
         {
            Sp_I2CSlavePtr->State = I2C_SLAVE_STATE_RD_STRETCH;
            break;
         }
         Sp_I2CSlavePtr->Buffer[Sp_I2CSlavePtr->W_RcvCount] = gm_ReadRegByte(I2C_SLAVE_DATA);
         Sp_I2CSlavePtr->W_AbsRcvCount++;
         Sp_I2CSlavePtr->W_RcvCount++;

         if (Sp_I2CSlavePtr->W_RcvCount >= I2C_SLAVE_BUFFER_SIZE)
         {
            I2cBufOverflow = TRUE;
            Sp_I2CSlavePtr->W_RcvCount = 0;
         }
         if (Sp_I2CSlavePtr->W_AbsRcvCount >= I2C_SLAVE_BUFFER_SIZE)
         {
            I2cBufOverflow = TRUE;
            Sp_I2CSlavePtr->W_AbsRcvCount = 0;
         }         
      }
      else
      {
         B_Buff = gm_ReadRegByte (I2C_SLAVE_DATA);
         if(Fp_CorrespondingFuncPtr(B_SlaveCh, Sp_I2CSlavePtr->B_DevID, &B_Buff) == FALSE)
         {
            Sp_I2CSlavePtr->State = I2C_SLAVE_STATE_RD_STRETCH;
            break;
         }
      }
   }
   gm_WriteRegByte(I2C_SLAVE_STATUS2, I2C_SLAVE_STRETCH_IN);
}

//******************************************************************************
//
// FUNCTION     :   static BYTE isCompleted_DDC2Bi_Msg(void)
// USAGE        :   Checks for new command in the serial buffer.
//                The function checks for a new compelte command
//                  in the serial input buffer and if the command
//                  is incomplete checks for time out. If time out
//                  is detected it resynchronize the serial link and
//                  exits.
//
// INPUT        :   None
// OUTPUT       :   Returns gmd_TRUE if a command was found in the serial
//                  receive buffer or gmd_FALSE if not.
// GLOBALS      :   gBa_CommBuf
//                  gW_RcvCount
//                  gDebugCmdPtr
// USED_REGS    :   None
//
//******************************************************************************
static BOOL IsCompleted_I2C_Msg(SLAVE_CH B_SlaveCh)
{
   BYTE B_Length;
   WORD W_Isr_RcvI2CCount;
   I2C_SLAVE_TYPE far* Sp_I2CSlavePtr = Sp_I2CSlavePtrGlobal[B_SlaveCh];   
	
   Lpm_I2CSlaveSetChannel(B_SlaveCh);
	
   if (Sp_I2CSlavePtr != NULL_PTR)
   {
      if (Sp_I2CSlavePtr->State != I2C_SLAVE_STATE_STOP_RD)
      {
         return FALSE;
      }

      // only after is received a read stop condition
      W_Isr_RcvI2CCount = Sp_I2CSlavePtr->W_RcvCount;
      
      // Check total packet length received
      B_Length =  (~DDC2Bi_CONTROL_STATUS_FLAG) & Sp_I2CSlavePtr->Buffer[1];  // packet length

      if((W_Isr_RcvI2CCount > 0) && (W_Isr_RcvI2CCount >= (B_Length + 3))) /*||
         (gm_ReadRegByte(I2C_SLAVE_CTRL1) & I2C_SLAVE_DMA_RFIFO_EN))  */
      {  // Message is received

         if(Lpm_I2CSlaveCheckCS(Sp_I2CSlavePtr->B_DevID, B_Length, &Sp_I2CSlavePtr->Buffer[0]))
         {
            //msg("CheckSum is OK !!!",0);
            Sp_I2CSlavePtr->State = I2C_SLAVE_STATE_WR;
            return TRUE;                                                 // message to be parsed
         }
         else
         {
            BYTE B_whichHandler = B_SlaveCh + I2C_SLAVE1_USE;
            //gm_I2CSendAckOrNack(SND_NAK);
            Lpm_SendAckOrNack(SND_NAK, B_whichHandler, (BYTE far*)&Sp_I2CSlavePtr->Buffer[Start_of_GprobeCmd_in_DDC2BiMsg]);
            //msg("CheckSum is not OK !!!",0);
            // clear read fifo
            gm_SetRegBitsByte(I2C_SLAVE_CTRL2, I2C_SLAVE_READ_FIFO_RESET);
            gm_ClearRegBitsByte(I2C_SLAVE_CTRL2, I2C_SLAVE_READ_FIFO_RESET);

            Sp_I2CSlavePtr->State = I2C_SLAVE_STATE_WR;
            return FALSE;                                                // Nothing to parse
         }
      }
      /*else
      {
         BYTE B_whichHandler = B_SlaveCh + I2C_SLAVE1_USE;
         //gm_I2CSendAckOrNack(SND_NAK);
         Lpm_SendAckOrNack(SND_NAK, B_whichHandler, (BYTE*)&Sp_I2CSlavePtr->Buffer[Start_of_GprobeCmd_in_DDC2BiMsg]);
         Sp_I2CSlavePtr->State = I2C_SLAVE_STATE_WR;
      } */
   }

   return FALSE;                                                         // Nothing to parse
}

//******************************************************************************
// USAGE:
//
// INPUT:
// OUTPUT:
// GLOBALS:
//******************************************************************************
BYTE far Lpm_I2CSlaveExecCommHandler(SLAVE_CH B_SlaveCh)
{
   BYTE B_Count;
   BYTE B_DevID;
   I2C_SLAVE_HANDLER_TYPE CorrespondingFuncPtr = NULL_PTR;
   I2C_SLAVE_TYPE far* Sp_I2CSlavePtr = Sp_I2CSlavePtrGlobal[B_SlaveCh];   

   Lpm_I2CSlaveSetChannel(B_SlaveCh);

   if(Sp_I2CSlavePtr == NULL_PTR) 
      return NO_MSG_PARSED;

   B_DevID = Sp_I2CSlavePtr->B_DevID & I2C_ADDRESS_BITS;

   //msg("B_DevID = 0x%X",B_DevID);

   for(B_Count = 1; B_Count <= Sp_I2CSlavePtr->Bp_CorrDevIDArray[0]; B_Count++)
   {
      if((B_DevID == Sp_I2CSlavePtr->Bp_CorrDevIDArray[B_Count])
         || (ALL_I2C_ADDRESSES == Sp_I2CSlavePtr->Bp_CorrDevIDArray[B_Count]))
      {
         //msg("Execute CommHandler B_DevID = %X",B_DevID);
         CorrespondingFuncPtr = Sp_I2CSlavePtr->Fp_CommHandlerArray[(B_Count-1)];
         return CorrespondingFuncPtr(B_SlaveCh);
      }
   }
   return NO_MSG_PARSED;
}

//******************************************************************************
// USAGE:  Default I2C Handler.
//
// INPUT:
// OUTPUT:
// GLOBALS:
//******************************************************************************
BYTE far Lpm_I2CSlaveCreateNullMsg(BYTE B_Stop, BYTE B_DevID, SLAVE_CH B_SlaveCh)
{
   BYTE B_Data = B_DevID & I2C_ADDRESS_BITS;
   I2C_SLAVE_TYPE far* Sp_I2CSlavePtr = Sp_I2CSlavePtrGlobal[B_SlaveCh];   

   Lpm_I2CSlaveSetChannel(B_SlaveCh);

   if(B_Stop)
   {
      Sp_I2CSlavePtr->B_NullMsgPtr = 0;
   }
   else
   {
      switch(Sp_I2CSlavePtr->B_NullMsgPtr++)
      {
         case 1: B_Data = NULL_MSG_SECOND_BYTE;
            break;
         case 2: B_Data ^= INIT_NULL_MSG_CS;
            break;
         case 0:
         default: 
            break;
      };

      if(Sp_I2CSlavePtr->B_NullMsgPtr == NULL_MSG_LENGTH)
         Sp_I2CSlavePtr->B_NullMsgPtr = 0;
   }
   return B_Data;
}

//******************************************************************************
// USAGE:  Default I2C Handler.
//
// INPUT:
// OUTPUT:
// GLOBALS:
//******************************************************************************
BYTE far Lpm_I2CSlaveDefaultHandler(SLAVE_CH B_SlaveCh)
{
   I2C_SLAVE_TYPE far* Sp_I2CSlavePtr = Sp_I2CSlavePtrGlobal[B_SlaveCh];

   Lpm_I2CSlaveSetChannel(B_SlaveCh);
   
   if(gm_ReadRegByte(I2C_SLAVE_CLOCK_STATUS) & I2C_SLAVE_STRETCH_OUT_STS)
   {
      if(Sp_I2CSlavePtr->B_ActiveFlag > 1)
      {
         BYTE B_Data;

         B_Data = Lpm_I2CSlaveCreateNullMsg(0, Sp_I2CSlavePtr->B_DevID, B_SlaveCh);
         gm_WriteRegByte(I2C_SLAVE_DATA, B_Data);
         Sp_I2CSlavePtr->State = I2C_SLAVE_STATE_WR_STRETCH;
         Sp_I2CSlavePtr->B_ActiveFlag = 1;
      }
      else
      {
         Sp_I2CSlavePtr->B_ActiveFlag++;
      }
   }
   else
   {
      if((Sp_I2CSlavePtr->B_ActiveFlag > 0) && (Sp_I2CSlavePtr->State == I2C_SLAVE_STATE_FREE))
      {
         Sp_I2CSlavePtr->B_Stop = FALSE;
         Sp_I2CSlavePtr->B_ActiveFlag = 0;
         Lpm_I2CSlaveCreateNullMsg(1, 0, B_SlaveCh);
      }
   }

   return NO_MSG_PARSED;
}

//******************************************************************************
// FUNCTION:   Lpm_I2CSlaveHandler
// USAGE:      Handler for I2C Slave should call from main loop
// INPUT:      None
// OUTPUT:     None
// GLOBALS:
// USED_REGS:  None
//******************************************************************************
void Lpm_I2CSlaveHandler(void)
{
	Lpm_I2CSlaveChannelHandler(I2C_SLAVE1_CH);
	Lpm_I2CSlaveChannelHandler(I2C_SLAVE2_CH);
	Lpm_I2CSlaveChannelHandler(I2C_SLAVE3_CH);
	Lpm_I2CSlaveChannelHandler(I2C_SLAVE4_CH);
}

//******************************************************************************
// FUNCTION:   Lpm_I2CSlaveChannelHandler
// USAGE:      Handler for specific I2C Slave channel 
// INPUT:      None
// OUTPUT:     None
// GLOBALS:
// USED_REGS:  None
//******************************************************************************
void far Lpm_I2CSlaveChannelHandler(SLAVE_CH B_SlaveCh)
{

   PARSER_STATUS_t B_Status = NO_MSG_PARSED;
   I2C_SLAVE_TYPE far* Sp_I2CSlavePtr = Sp_I2CSlavePtrGlobal[B_SlaveCh];   

   Lpm_I2CSlaveSetChannel(B_SlaveCh);
   
   if(Sp_I2CSlavePtr == NULL_PTR)
   {
      gB_Ddc2Bi_I2C_Status[B_SlaveCh] = B_Status;
      return;
   }

   if(I2CSlaveExtHandlerFuncPtr == NULL_PTR)
   {
      B_Status = Lpm_I2CSlaveExecCommHandler(B_SlaveCh);
      if((B_Status == NO_MSG_PARSED) ||
         ((B_Status == UNKNOWN_MSG_CMD) && (gB_DdcciWaitForParsing[B_SlaveCh] == FALSE))
         && (B_ModeStatus == INTERNAL_ROM_MODE))
      {
         Lpm_I2CSlaveDefaultHandler(B_SlaveCh);
      }
   }
   else
      B_Status = I2CSlaveExtHandlerFuncPtr(B_SlaveCh);

   gB_Ddc2Bi_I2C_Status[B_SlaveCh] = B_Status;

}


//******************************************************************************
// FUNCTION:   Lpm_I2CSlaveSetExtHandler
// USAGE:      Sets pointer to external handler
// INPUT:      FuncPtr -- pointer to external function
// OUTPUT:     None
// GLOBALS:
// USED_REGS:  None
//******************************************************************************
void far Lpm_I2CSlaveSetExtHandler(BYTE far(* FuncPtr)(SLAVE_CH B_SlaveCh))
{
   I2CSlaveExtHandlerFuncPtr = FuncPtr;
}

//******************************************************************************
// FUNCTION:   Lpm_I2CSlaveIsrNew
// USAGE:	I2C slave interrupt service routine, used when I2C slave FIFO mode is enabled
// INPUT:	None
// OUTPUT:	None
// GLOBALS:
//******************************************************************************
void far Lpm_I2CSlaveIsrNew(SLAVE_CH B_SlaveCh)
{
   BYTE  B_Buff;
   I2C_SLAVE_TYPE far* Sp_I2CSlavePtr = Sp_I2CSlavePtrGlobal[B_SlaveCh];   

   Lpm_I2CSlaveSetChannel(B_SlaveCh);
   
   #if USE_DMA_FOR_RFIFO
   B_DMAEn = (gm_ReadRegByte(I2C_SLAVE_CTRL1) & I2C_SLAVE_DMA_RFIFO_EN);
   #endif

   B_SlvStatus[B_SlaveCh] = gm_ReadRegByte(I2C_SLAVE_STATUS);
   B_SlvStatus2[B_SlaveCh] = gm_ReadRegByte(I2C_SLAVE_STATUS2);

   //gm_Print("SLV%d ISR",(B_SlaveCh+1)); 


   if ((Sp_I2CSlavePtr == NULL_PTR) || (B_SlvStatus[B_SlaveCh] & I2C_SLAVE_DATA_ERROR))
   {
      gm_SetRegBitsByte(I2C_SLAVE_CTRL2, I2C_SLAVE_READ_FIFO_RESET);
      gm_ClearRegBitsByte(I2C_SLAVE_CTRL2, I2C_SLAVE_READ_FIFO_RESET);
      gm_WriteRegByte(I2C_SLAVE_STATUS, B_SlvStatus[B_SlaveCh]);
      gm_WriteRegByte(I2C_SLAVE_STATUS2, B_SlvStatus2[B_SlaveCh]);
      gm_ClearRegBitsByte(I2C_SLAVE_CTRL1,I2C_SLAVE_EN);
      gm_SetRegBitsByte(I2C_SLAVE_CTRL1,I2C_SLAVE_EN);
      
      if(Fp_CorrespondingFuncPtr != NULL_PTR)
      {
         Fp_CorrespondingFuncPtr(B_SlaveCh, Sp_I2CSlavePtr->B_DevID, NULL_PTR);
      }
      return;
   }

#if  I2C_WRITE_FIFO_RESET
   if(IsResetWriteFifo())
	{
	//Start: Patch for I2C Write FIFO reset   
      if ((B_SlvStatus2[B_SlaveCh] & I2C_SLAVE_START) &&
				(0 != (gm_ReadRegByte(I2C_SLAVE_CTRL3)& I2C_SLAVE_START_IRQ_EN))
		  )
	   {
			gm_SetRegBitsByte(I2C_SLAVE_CTRL2, I2C_SLAVE_WRITE_FIFO_RESET);  
			gm_ClearRegBitsByte(I2C_SLAVE_CTRL1, I2C_SLAVE_WFIFO_E_IRQ_EN);
			gm_ClearRegBitsByte(I2C_SLAVE_CTRL2, I2C_SLAVE_WRITE_FIFO_RESET); 
			gm_ClearRegBitsByte(I2C_SLAVE_CTRL3,I2C_SLAVE_START_IRQ_EN);
	   }
	//End: Patch for I2C Write FIFO reset     
	}
#endif

   if(B_SlvStatus2[B_SlaveCh] & I2C_SLAVE_DEV_ID_RECEIVED)
   {
      if ((Sp_I2CSlavePtr->State != I2C_SLAVE_STATE_WR) && (Sp_I2CSlavePtr->State != I2C_SLAVE_STATE_STOP_RD)
         && (B_SlvStatus2[B_SlaveCh] & I2C_SLAVE_START) && (Sp_I2CSlavePtr->State != I2C_SLAVE_STATE_FREE))
      {
         #if USE_DMA_FOR_RFIFO
         if(B_DMAEn)
         {
            WORD W_Count = DDC2BI_BUFFER_SIZE - READ_PCB_REG(D3TC);
            if(W_Count > 0)
            {
               Sp_I2CSlavePtr->W_RcvCount = W_Count;
               Sp_I2CSlavePtr->W_AbsRcvCount = W_Count;

               // Reset DMA CH3
               WRITE_PCB_REG (D3DSTH, 0);
               WRITE_PCB_REG (D3DSTL, Sp_I2CSlavePtr->Buffer);
               WRITE_PCB_REG (D3TC, DDC2BI_BUFFER_SIZE);
            }
         }
         else
		 #endif
         {
            Lpm_I2CSlaveReadFromFifo_new(B_SlaveCh);
         }
         
         if(Fp_CorrespondingFuncPtr == NULL_PTR)
         {
            Sp_I2CSlavePtr->State = I2C_SLAVE_STATE_STOP_RD;
            Sp_I2CSlavePtr->B_Stop = TRUE;
            Sp_I2CSlavePtr->TxPtr = Sp_I2CSlavePtr->W_TrxCount = 0;
         }
         else
         {
           // Fp_CorrespondingFuncPtr(B_SlaveCh, Sp_I2CSlavePtr->B_DevID, NULL_PTR);    
    
         }
         gm_SetRegBitsByte(I2C_SLAVE_CTRL3, I2C_SLAVE_STRETCH_OUT_IRQ_EN);
      }
      gm_WriteRegByte(I2C_SLAVE_STATUS2, I2C_SLAVE_DEV_ID_RECEIVED | I2C_SLAVE_START);
      return;
   }
   if (B_SlvStatus[B_SlaveCh] & I2C_SLAVE_STOP)
   {

      gm_WriteRegByte(I2C_SLAVE_STATUS, I2C_SLAVE_STOP);
      //gm_WriteRegByte(I2C_SLAVE_STATUS2, B_SlvStatus2);
      gm_SetRegBitsByte(I2C_SLAVE_CTRL3, I2C_SLAVE_STRETCH_OUT_IRQ_EN);
	  #if USE_DMA_FOR_RFIFO
      if(B_DMAEn)
      {
         WORD W_Count = DDC2BI_BUFFER_SIZE - READ_PCB_REG(D3TC);
         if(W_Count > 0)
         {
            Sp_I2CSlavePtr->State = I2C_SLAVE_STATE_RD;
            Sp_I2CSlavePtr->W_RcvCount = W_Count;
            Sp_I2CSlavePtr->W_AbsRcvCount = W_Count;

            // Reset DMA CH3
            WRITE_PCB_REG (D3DSTH, 0);
            WRITE_PCB_REG (D3DSTL, Sp_I2CSlavePtr->Buffer);
            WRITE_PCB_REG (D3TC, DDC2BI_BUFFER_SIZE);
         }
      }
	  #endif

      if((Sp_I2CSlavePtr->State != I2C_SLAVE_STATE_FREE) || (B_SlvStatus[B_SlaveCh] & I2C_SLAVE_READ_FIFO_NE))    
      {
         if ((Sp_I2CSlavePtr->State != I2C_SLAVE_STATE_WR) && (Sp_I2CSlavePtr->State != I2C_SLAVE_STATE_WR_STRETCH))
         {
         	#if USE_DMA_FOR_RFIFO
            if(!B_DMAEn)
			#endif
            {
               Lpm_I2CSlaveReadFromFifo_new(B_SlaveCh);
            }
            if(Fp_CorrespondingFuncPtr == NULL_PTR)
            {
               Sp_I2CSlavePtr->State = I2C_SLAVE_STATE_STOP_RD;
               Sp_I2CSlavePtr->B_Stop = TRUE;
               Sp_I2CSlavePtr->TxPtr = Sp_I2CSlavePtr->W_TrxCount = 0;
            }
            else
            {
               Fp_CorrespondingFuncPtr(B_SlaveCh, Sp_I2CSlavePtr->B_DevID, NULL_PTR);
            }

            gm_WriteRegByte(I2C_SLAVE_STATUS, I2C_SLAVE_STOP);
         }
         else
         {
            Sp_I2CSlavePtr->TxPtr = Sp_I2CSlavePtr->W_TrxCount = 0;
            Sp_I2CSlavePtr->RxPtr = Sp_I2CSlavePtr->W_RcvCount = 0;
            Sp_I2CSlavePtr->W_AbsRcvCount = 0;

	#if  I2C_WRITE_FIFO_RESET
            if(IsResetWriteFifo())
	{
		// clear wr fifo
		gm_SetRegBitsByte(I2C_SLAVE_CTRL2, I2C_SLAVE_WRITE_FIFO_RESET);  //Nirmala   2/25/10 //I2C Write FIFO reset patch
	}
	#endif
		gm_ClearRegBitsByte(I2C_SLAVE_CTRL1, I2C_SLAVE_WFIFO_E_IRQ_EN);

	#if  I2C_WRITE_FIFO_RESET
            if (IsResetWriteFifo())
		gm_ClearRegBitsByte(I2C_SLAVE_CTRL2, I2C_SLAVE_WRITE_FIFO_RESET); //Nirmala   2/25/10 //I2C Write FIFO reset patch
	#endif
	
            Sp_I2CSlavePtr->State = I2C_SLAVE_STATE_FREE;
            Sp_I2CSlavePtr->B_ReplyReady = 0;
            if(Fp_CorrespondingFuncPtr != NULL_PTR)
            {
               Fp_CorrespondingFuncPtr(B_SlaveCh, Sp_I2CSlavePtr->B_DevID, NULL_PTR);
            }

            gm_WriteRegByte(I2C_SLAVE_STATUS, B_SlvStatus[B_SlaveCh]);
         }
      }
      return;
   }

   if (B_SlvStatus[B_SlaveCh] & I2C_SLAVE_READ_FIFO_NE )
   {
      Sp_I2CSlavePtr->State = I2C_SLAVE_STATE_RD;
      Lpm_I2CSlaveReadFromFifo_new(B_SlaveCh);
   }
   else  //Read FIFO is empty
   {
      if((B_SlvStatus[B_SlaveCh] & I2C_SLAVE_WRITE_FIFO_E) || (B_SlvStatus2[B_SlaveCh] & I2C_SLAVE_STRETCH_OUT)) 
      {
         if(Fp_CorrespondingFuncPtr != NULL_PTR)
         {
         
            if(Fp_CorrespondingFuncPtr(B_SlaveCh, Sp_I2CSlavePtr->B_DevID, &B_Buff) == FALSE)
            {
               Sp_I2CSlavePtr->State = I2C_SLAVE_STATE_WR_STRETCH;
               gm_ClearRegBitsByte(I2C_SLAVE_CTRL3, I2C_SLAVE_STRETCH_OUT_IRQ_EN);
            }			
            gm_ClearRegBitsByte(I2C_SLAVE_CTRL1, I2C_SLAVE_WFIFO_E_IRQ_EN);

            gm_WriteRegByte(I2C_SLAVE_DATA, B_Buff);

#if I2C_WRITE_FIFO_RESET
		      if(IsResetWriteFifo())
		{
			//Start: Patch for I2C Write FIFO reset 
			gm_SetRegBitsByte(I2C_SLAVE_CTRL3,I2C_SLAVE_START_IRQ_EN);
			gm_SetRegBitsByte(I2C_SLAVE_STATUS2 ,I2C_SLAVE_START);
			//End: Patch for I2C Write FIFO reset 
		}
#endif
            gm_WriteRegByte(I2C_SLAVE_STATUS2, I2C_SLAVE_STRETCH_OUT);            
            Sp_I2CSlavePtr->State = I2C_SLAVE_STATE_WR;
			
            return;
         }

         if(Sp_I2CSlavePtr->B_ReplyReady == 0)
         {
            #if 0
            if(Sp_I2CSlavePtr->State != I2C_SLAVE_STATE_WR)
               Sp_I2CSlavePtr->State = I2C_SLAVE_STATE_STOP_RD;

            Sp_I2CSlavePtr->B_Stop = TRUE;

            gm_WriteRegByte(I2C_SLAVE_STATUS2, B_SlvStatus2[B_SlaveCh]);
            gm_ClearRegBitsByte(I2C_SLAVE_CTRL3, I2C_SLAVE_STRETCH_OUT_IRQ_EN);
            gm_ClearRegBitsByte(I2C_SLAVE_CTRL1, I2C_SLAVE_WFIFO_E_IRQ_EN);
            return;
            #else  
            //Nothing to send - Fill null message to WFIFO
            ((BYTE far*)&Sp_I2CSlavePtr->Buffer)[0] = DDC2BI_NULL_MSG_LENGTH;
            ((BYTE far*)&Sp_I2CSlavePtr->Buffer)[1] = DDC2BI_ADDR_ID^INIT_NULL_MSG_CS;     
            ((BYTE far*)&Sp_I2CSlavePtr->Buffer)[2] = DDC2BI_ADDR_ID; //to keep WFIFO non empty in case host requests more bytes
            Lpm_I2CSlaveSetReply(3, B_SlaveCh);
            #endif         
         }
         if(
         #if I2C_WRITE_FIFO_RESET
         	!IsResetWriteFifo() &&
         #endif
         	B_DevIdReady == FALSE)
         {
         	return; // borrow this bit to indicate if dev id is received
         }
         Sp_I2CSlavePtr->State = I2C_SLAVE_STATE_WR;
         Lpm_I2CSlaveWriteToFifo(B_SlaveCh);
		 //Nirmala:2/23/2009 
		//When there is no more data to write to write FIFO and more number of data is read DpTx hangs.
		//Issue with below IRQs been generated continuously even after CRO statuses are cleared.
		 if(Sp_I2CSlavePtr->TxPtr == Sp_I2CSlavePtr->W_TrxCount) 
		 	{
            #if I2C_WRITE_FIFO_RESET
            if(!IsResetWriteFifo())
            #endif
            {
            	B_DevIdReady = FALSE;
            	// borrow this bit to indicate if dev id is received
            }
            gm_ClearRegBitsByte(I2C_SLAVE_CTRL1, I2C_SLAVE_WFIFO_E_IRQ_EN);
            gm_ClearRegBitsByte(I2C_SLAVE_CTRL3, I2C_SLAVE_STRETCH_OUT_IRQ_EN);	
         }
      }
   }
}

//******************************************************************************
// FUNCTION:   Lpm_I2CSlaveIsrOld
// USAGE:	I2C slave interrupt service routine, used when I2C slave FIFO mode is disabled
// INPUT:	None
// OUTPUT:	None
// GLOBALS:
//******************************************************************************
void far Lpm_I2CSlaveIsrOld(SLAVE_CH B_SlaveCh)
{
   BYTE  B_Buff;
   I2C_SLAVE_TYPE far* Sp_I2CSlavePtr = Sp_I2CSlavePtrGlobal[B_SlaveCh];   

   Lpm_I2CSlaveSetChannel(B_SlaveCh);
   
   #if USE_DMA_FOR_RFIFO
   B_DMAEn = (gm_ReadRegByte(I2C_SLAVE_CTRL1) & I2C_SLAVE_DMA_RFIFO_EN);		
   #endif

   B_SlvStatus[B_SlaveCh] = gm_ReadRegByte(I2C_SLAVE_STATUS);
   B_SlvStatus2[B_SlaveCh] = gm_ReadRegByte(I2C_SLAVE_STATUS2);


   if ((Sp_I2CSlavePtr == NULL_PTR) || (B_SlvStatus[B_SlaveCh] & I2C_SLAVE_DATA_ERROR))
   {
      gm_SetRegBitsByte(I2C_SLAVE_CTRL2, I2C_SLAVE_READ_FIFO_RESET);
      gm_ClearRegBitsByte(I2C_SLAVE_CTRL2, I2C_SLAVE_READ_FIFO_RESET);
      gm_WriteRegByte(I2C_SLAVE_STATUS, B_SlvStatus[B_SlaveCh]);
      gm_WriteRegByte(I2C_SLAVE_STATUS2, B_SlvStatus2[B_SlaveCh]);
      if(Fp_CorrespondingFuncPtr != NULL_PTR)
      {
         Fp_CorrespondingFuncPtr(B_SlaveCh, Sp_I2CSlavePtr->B_DevID, NULL_PTR);
      }
      return;
   }

   if(B_SlvStatus2[B_SlaveCh] & I2C_SLAVE_DEV_ID_RECEIVED)
   {
      if ((Sp_I2CSlavePtr->State != I2C_SLAVE_STATE_WR) && (Sp_I2CSlavePtr->State != I2C_SLAVE_STATE_STOP_RD)
         && (B_SlvStatus2[B_SlaveCh] & I2C_SLAVE_START) && (Sp_I2CSlavePtr->State != I2C_SLAVE_STATE_FREE))
      {
         #if USE_DMA_FOR_RFIFO
         if(B_DMAEn)
         {
            WORD W_Count = DDC2BI_BUFFER_SIZE - READ_PCB_REG(D3TC);
            if(W_Count > 0)
            {
               Sp_I2CSlavePtr->W_RcvCount = W_Count;
               Sp_I2CSlavePtr->W_AbsRcvCount = W_Count;

               // Reset DMA CH3
               WRITE_PCB_REG (D3DSTH, 0);
               WRITE_PCB_REG (D3DSTL, Sp_I2CSlavePtr->Buffer);
               WRITE_PCB_REG (D3TC, DDC2BI_BUFFER_SIZE);
            }
         }
         else
		 #endif
         {
            Lpm_I2CSlaveReadFromFifo(B_SlaveCh);
         }
         
         if(Fp_CorrespondingFuncPtr == NULL_PTR)
         {
            Sp_I2CSlavePtr->State = I2C_SLAVE_STATE_STOP_RD;
            Sp_I2CSlavePtr->B_Stop = TRUE;
            Sp_I2CSlavePtr->TxPtr = Sp_I2CSlavePtr->W_TrxCount = 0;
         }
         else
         {
            Fp_CorrespondingFuncPtr(B_SlaveCh, Sp_I2CSlavePtr->B_DevID, NULL_PTR);
         }
         gm_SetRegBitsByte(I2C_SLAVE_CTRL3, I2C_SLAVE_STRETCH_OUT_IRQ_EN);
      }

      Sp_I2CSlavePtr->B_DevID = gm_ReadRegByte(I2C_SLAVE_RX_DEV_ID);

      Sp_I2CSlavePtr->DevAddr = (Sp_I2CSlavePtr->DevAddr & (~AUX_I2C_ADDR_MASK)) | (Sp_I2CSlavePtr->B_DevID >> 1);

      Fp_CorrespondingFuncPtr = Lpm_I2CSlaveHLFindCommHandler(Sp_I2CSlavePtr->B_DevID, B_SlaveCh);
      
      gm_WriteRegByte(I2C_SLAVE_STATUS2, I2C_SLAVE_DEV_ID_RECEIVED | I2C_SLAVE_START);
      return;
   }

   if (B_SlvStatus[B_SlaveCh] & I2C_SLAVE_STOP)
   {
      gm_WriteRegByte(I2C_SLAVE_STATUS, B_SlvStatus[B_SlaveCh]);
      gm_WriteRegByte(I2C_SLAVE_STATUS2, B_SlvStatus2[B_SlaveCh]);
      gm_SetRegBitsByte(I2C_SLAVE_CTRL3, I2C_SLAVE_STRETCH_OUT_IRQ_EN);

      #if USE_DMA_FOR_RFIFO	
      if(B_DMAEn)
      {
         WORD W_Count = DDC2BI_BUFFER_SIZE - READ_PCB_REG(D3TC);
         if(W_Count > 0)
         {
            Sp_I2CSlavePtr->State = I2C_SLAVE_STATE_RD;
            Sp_I2CSlavePtr->W_RcvCount = W_Count;
            Sp_I2CSlavePtr->W_AbsRcvCount = W_Count;

            // Reset DMA CH3
            WRITE_PCB_REG (D3DSTH, 0);
            WRITE_PCB_REG (D3DSTL, Sp_I2CSlavePtr->Buffer);
            WRITE_PCB_REG (D3TC, DDC2BI_BUFFER_SIZE);
         }
      }
	  #endif

      if((Sp_I2CSlavePtr->State != I2C_SLAVE_STATE_FREE) || (B_SlvStatus[B_SlaveCh] & I2C_SLAVE_READ_FIFO_NE))    
      {
         if ((Sp_I2CSlavePtr->State != I2C_SLAVE_STATE_WR) && (Sp_I2CSlavePtr->State != I2C_SLAVE_STATE_WR_STRETCH))
         {
         	#if USE_DMA_FOR_RFIFO
            if(!B_DMAEn)
			#endif
            {
               Lpm_I2CSlaveReadFromFifo(B_SlaveCh);
            }
            if(Fp_CorrespondingFuncPtr == NULL_PTR)
            {
               Sp_I2CSlavePtr->State = I2C_SLAVE_STATE_STOP_RD;
               Sp_I2CSlavePtr->B_Stop = TRUE;
               Sp_I2CSlavePtr->TxPtr = Sp_I2CSlavePtr->W_TrxCount = 0;
            }
            else
            {
               Fp_CorrespondingFuncPtr(B_SlaveCh, Sp_I2CSlavePtr->B_DevID, NULL_PTR);
               //gm_Print("S2",0);
            }

            if(I2cBufOverflow)
            {
               I2cBufOverflow = FALSE;
               Sp_I2CSlavePtr->TxPtr = Sp_I2CSlavePtr->W_TrxCount = 0;
               Sp_I2CSlavePtr->RxPtr = Sp_I2CSlavePtr->W_RcvCount = 0;            
               Sp_I2CSlavePtr->W_AbsRcvCount = 0;               
            }

            gm_WriteRegByte(I2C_SLAVE_STATUS, B_SlvStatus[B_SlaveCh]);
         }
         else
         {
            Sp_I2CSlavePtr->TxPtr = Sp_I2CSlavePtr->W_TrxCount = 0;
            Sp_I2CSlavePtr->RxPtr = Sp_I2CSlavePtr->W_RcvCount = 0;
            Sp_I2CSlavePtr->W_AbsRcvCount = 0;

            // clear wr fifo
            gm_SetRegBitsByte(I2C_SLAVE_CTRL2, I2C_SLAVE_WRITE_FIFO_RESET);
            gm_ClearRegBitsByte(I2C_SLAVE_CTRL1, I2C_SLAVE_WFIFO_E_IRQ_EN);
            gm_ClearRegBitsByte(I2C_SLAVE_CTRL2, I2C_SLAVE_WRITE_FIFO_RESET);

            Sp_I2CSlavePtr->State = I2C_SLAVE_STATE_FREE;
            Sp_I2CSlavePtr->B_ReplyReady = 0;
            if(Fp_CorrespondingFuncPtr != NULL_PTR)
            {
               Fp_CorrespondingFuncPtr(B_SlaveCh, Sp_I2CSlavePtr->B_DevID, NULL_PTR);
               //gm_Print("S3",0);
            }

            gm_WriteRegByte(I2C_SLAVE_STATUS, B_SlvStatus[B_SlaveCh]);
         }
      }
      return;
   }

   if (B_SlvStatus[B_SlaveCh] & I2C_SLAVE_READ_FIFO_NE )
   {
      Sp_I2CSlavePtr->State = I2C_SLAVE_STATE_RD;
      Lpm_I2CSlaveReadFromFifo(B_SlaveCh);
   }
   else
   {
      if((B_SlvStatus[B_SlaveCh] & I2C_SLAVE_WRITE_FIFO_E) || (B_SlvStatus2[B_SlaveCh] & I2C_SLAVE_STRETCH_OUT)) 
      {
         if(Fp_CorrespondingFuncPtr != NULL_PTR)
         {
            if(Fp_CorrespondingFuncPtr(B_SlaveCh, Sp_I2CSlavePtr->B_DevID, &B_Buff) == FALSE)
            {
               Sp_I2CSlavePtr->State = I2C_SLAVE_STATE_WR_STRETCH;
               gm_ClearRegBitsByte(I2C_SLAVE_CTRL3, I2C_SLAVE_STRETCH_OUT_IRQ_EN);
            }
            gm_ClearRegBitsByte(I2C_SLAVE_CTRL1, I2C_SLAVE_WFIFO_E_IRQ_EN);
            gm_WriteRegByte(I2C_SLAVE_DATA, B_Buff);
            gm_WriteRegByte(I2C_SLAVE_STATUS2, I2C_SLAVE_STRETCH_OUT);
            
            Sp_I2CSlavePtr->State = I2C_SLAVE_STATE_WR;
            return;
         }
         
         if(Sp_I2CSlavePtr->B_ReplyReady == 0)
         {
            #if 0
            if(Sp_I2CSlavePtr->State != I2C_SLAVE_STATE_WR)
               Sp_I2CSlavePtr->State = I2C_SLAVE_STATE_STOP_RD;

            Sp_I2CSlavePtr->B_Stop = TRUE;

            gm_WriteRegByte(I2C_SLAVE_STATUS2, B_SlvStatus2[B_SlaveCh]);
            gm_ClearRegBitsByte(I2C_SLAVE_CTRL3, I2C_SLAVE_STRETCH_OUT_IRQ_EN);
            gm_ClearRegBitsByte(I2C_SLAVE_CTRL1, I2C_SLAVE_WFIFO_E_IRQ_EN);
            return;
            #else  
            //Nothing to send - Fill null message to WFIFO
            ((BYTE far*)&Sp_I2CSlavePtr->Buffer)[0] = DDC2BI_NULL_MSG_LENGTH;
            ((BYTE far*)&Sp_I2CSlavePtr->Buffer)[1] = DDC2BI_ADDR_ID^INIT_NULL_MSG_CS;     
            ((BYTE far*)&Sp_I2CSlavePtr->Buffer)[2] = DDC2BI_ADDR_ID; //to keep WFIFO non empty in case host requests more bytes
            Lpm_I2CSlaveSetReply(3, B_SlaveCh);
            #endif
         }
         Sp_I2CSlavePtr->State = I2C_SLAVE_STATE_WR;
         Lpm_I2CSlaveWriteToFifo(B_SlaveCh);

         //Disable WFIFO empty IRQ and stretch out IRQ when all data is replied
         //This fix is to prevent too many IRQ raised that makes OCM extremely busy!!
         if(Sp_I2CSlavePtr->TxPtr == Sp_I2CSlavePtr->W_TrxCount) 
         {
            B_DevIdReady = FALSE;
            gm_ClearRegBitsByte(I2C_SLAVE_CTRL1, I2C_SLAVE_WFIFO_E_IRQ_EN);
            gm_ClearRegBitsByte(I2C_SLAVE_CTRL3, I2C_SLAVE_STRETCH_OUT_IRQ_EN);  
         }          
      }
   }
}

//******************************************************************************
// FUNCTION:   Lpm_I2CSlaveIsr
// USAGE:	I2C slave interrupt service routine, if FIFO mode used, then calls Lpm_I2CSlaveISR_new
//			otherwise calls Lpm_I2CSlaveISR_old
// INPUT:	None
// OUTPUT:	None
// GLOBALS:
//******************************************************************************
void far Lpm_I2CSlaveIsr(SLAVE_CH B_SlaveCh)
{

	SLAVE_CH B_SlaveChTemp = Lpm_I2CSlaveGetChannel();
	
	Lpm_I2CSlaveSetChannel(B_SlaveCh);

   #if (LPM_EDID_ACCESS_SEL == 2)
   LpmEdidWaitProc = 1;   
	gm_Print("Lpm_I2CSlaveIsr",0);
   #endif

   	if(0 != (gm_ReadRegByte(I2C_SLAVE_CTRL2) & I2C_SLAVE_FIFO_MODE))
   	{   	  	
   	  	Lpm_I2CSlaveIsrNew(B_SlaveCh);		  
   	}
   	else
   	{
   	  	Lpm_I2CSlaveIsrOld(B_SlaveCh);
   	}
   
	Lpm_I2CSlaveSetChannel(B_SlaveChTemp);
   
 }

//******************************************************************************
// FUNCTION:   Lpm_I2CSlaveGetAddrMsgBuffer
// USAGE:	Used to get i2c slave buffer
//
// INPUT:	None
// OUTPUT:	I2C slave buffer
// GLOBALS:
//******************************************************************************
BYTE far* Lpm_I2CSlaveGetAddrMsgBuffer(SLAVE_CH B_SlaveCh)
{
   return Sp_I2CSlavePtrGlobal[B_SlaveCh]->Buffer;
}

//******************************************************************************
// FUNCTION:   Lpm_I2CSlaveAddCommHandler
// USAGE:	Used to add I2C slave common handler for specific device ID
//
// INPUT:	Fp_NewCommHandler - pointer to new common handler
//			B_CorrDevID - Device ID
// OUTPUT:	return status
// GLOBALS:
//******************************************************************************
BOOL far Lpm_I2CSlaveAddCommHandler(I2C_SLAVE_HANDLER_TYPE Fp_NewCommHandler, BYTE B_CorrDevID, SLAVE_CH B_SlaveCh)
{
   BYTE B_FuncNum;
   I2C_SLAVE_TYPE far* Sp_I2CSlavePtr = Sp_I2CSlavePtrGlobal[B_SlaveCh];   

   Lpm_I2CSlaveSetChannel(B_SlaveCh);

   if((Sp_I2CSlavePtr == NULL_PTR) || (Sp_I2CSlavePtr->Bp_CorrDevIDArray == NULL_PTR))
      return FALSE;

   B_FuncNum = Sp_I2CSlavePtr->Bp_CorrDevIDArray[0];

   if((B_FuncNum >= MAX_FUNC_NUM) || (B_CorrDevID & 0x01))
   {
      //gm_Print("Lpm_I2CSlaveAddCommHandler() ERROR: CommHandlerArray is full",0);
      return FALSE;
   }

   Sp_I2CSlavePtr->Fp_CommHandlerArray[B_FuncNum++] = Fp_NewCommHandler;
   Sp_I2CSlavePtr->Bp_CorrDevIDArray[B_FuncNum] = B_CorrDevID;
   Sp_I2CSlavePtr->Bp_CorrDevIDArray[0] = B_FuncNum;
   //gm_Print("Lpm_I2CSlaveAddCommHandler() was done for DevID = 0x%X",B_CorrDevID);

   return TRUE;
}

//******************************************************************************
// FUNCTION:   Lpm_I2CSlaveGProbeHandler
// USAGE:  I2C slave Gprobe command handler
//
// INPUT:  None
// OUTPUT:	Parser status 
// GLOBALS:
//******************************************************************************
BYTE far Lpm_I2CSlaveGProbeHandler(SLAVE_CH B_SlaveCh)
{
   PARSER_STATUS_t B_Status = NO_MSG_PARSED;
   I2C_SLAVE_TYPE far* Sp_I2CSlavePtr = Sp_I2CSlavePtrGlobal[B_SlaveCh];   

   Lpm_I2CSlaveSetChannel(B_SlaveCh);
   
   if(IsCompleted_I2C_Msg(B_SlaveCh))
   {
      //BYTE i;   
      //gm_Print("IsCompleted_I2C_Msg",0);
      //for(i=0; i<10; i++)
      //   gm_Print("Buffer=0x%x",Sp_I2CSlavePtr->Buffer[i]);      

      #if 1//def POWER_MODE_HANDLE
      /*if(Lpm_PowerModeGetState() == LPM_NormalPowerMode)
      {
         if (gm_MessageSend(gmd_MESSAGE_DDC, (BYTE far*)Sp_I2CSlavePtr->Buffer, I2C_Ddcci_Length(B_SlaveCh)+3)==FALSE)
         {
            gm_Print("$ Transfer DDC2B GProbe command to Mission FAIL!",0); 
         }
      }
      else  //LPM_LowPowerMode ; LPM_PowerOffMode
      */
      {    
      if ((I2C_as_Gprobe_Cmd(B_SlaveCh) == gB_GprobeCmdSet) &&
         (I2C_ManufacturerID(B_SlaveCh) == gW_ManufacturerID))
      {
         BYTE B_whichHandler = B_SlaveCh + I2C_SLAVE1_USE;

            B_Status = Lpm_GprobeCmdParser(B_whichHandler, (BYTE far*)Sp_I2CSlavePtr->Buffer);
         //gm_Print("G",0);

         // clear read fifo
         gm_SetRegBitsByte(I2C_SLAVE_CTRL2, I2C_SLAVE_READ_FIFO_RESET);
         gm_ClearRegBitsByte(I2C_SLAVE_CTRL2, I2C_SLAVE_READ_FIFO_RESET);
      }
         #ifdef DDCCI_SUPPORT
         else if(Lpm_IsDdcciTransaction(Sp_I2CSlavePtr->Buffer))
      {
		gB_DdcciWaitForParsing[B_SlaveCh] = TRUE;
         }
         #endif   
         else
         {		  
         B_Status = UNKNOWN_MSG_CMD;      // extern it will be solved the ddc2bi commands,
                                          // other the Gprobe command set !!!
      }   
   }
      #else    //NO power mode handler, always transfer DDC data to Mission      
      
      gmt_DDC_INFO DdcMailData;
      
      DdcMailData.SlaveCh = B_SlaveCh;
      DdcMailData.Length = Sp_I2CSlavePtr->B_DDCLength = I2C_Ddcci_Length(B_SlaveCh)+3;
      _fmemcpy((BYTE far*)LPM_MIS_DATA_OUT_0, (BYTE far*)Sp_I2CSlavePtr->Buffer, Sp_I2CSlavePtr->B_DDCLength);
      if(gm_MessageSend(gmd_MESSAGE_DDC, &DdcMailData, 1))
      {
         DWORD TimeStart = gm_GetSystemTime();
         BYTE TimeOut_ms = 100;   
         
         while(1)
      {
            if(gm_MessageReceived(gmd_MESSAGE_DDC, &DdcMailData, 1))
            {
               //BYTE i;
               _fmemcpy(Sp_I2CSlavePtr->Buffer, (BYTE far*)LPM_MIS_DATA_IN_0, DdcMailData.Length);
               
               Lpm_I2CSlaveSetReply(DdcMailData.Length, DdcMailData.SlaveCh);
               
               //for(i=0; i<12; i++)
               //   gm_Print("Lpm Rcv DDC data = 0x%x",((BYTE far*)Sp_I2CSlavePtr->Buffer)[i]);              

               // clear read fifo
               gm_SetRegBitsByte(I2C_SLAVE_CTRL2, I2C_SLAVE_READ_FIFO_RESET);
               gm_ClearRegBitsByte(I2C_SLAVE_CTRL2, I2C_SLAVE_READ_FIFO_RESET);             
               break;
            }
            else if(gm_GetSystemTime() > (TimeStart + TimeOut_ms))   //check if timeout
      {
               gm_Print("$ Error: Wait DDC_MSG from Mission timeout!",0);
               break;
            }
         }
      }
      else
      {
         gm_Print("$ Error: Send DDC data to Mission FAIL!",0); 
      }
      #endif   //#ifdef POWER_MODE_HANDLE
   }
   
   return B_Status;
}

#if (LPM_EDID_ACCESS_SEL == 2)   //Use I2C SLAVE1 as virtual EDID (FW)
//******************************************************************************
// FUNCTION:   Lpm_I2CSlaveEdidHandler
// USAGE:  Use I2C Slave to handle EDID request from AUX_I2C_MST
//
// INPUT:  None
// OUTPUT:	Parser status 
// GLOBALS:
//******************************************************************************
BYTE far Lpm_I2CSlaveEdidHandler(SLAVE_CH B_SlaveCh)
{
   PARSER_STATUS_t B_Status = NO_MSG_PARSED;
   I2C_SLAVE_TYPE far* Sp_I2CSlavePtr = Sp_I2CSlavePtrGlobal[B_SlaveCh]; 

   Lpm_I2CSlaveSetChannel(B_SlaveCh);

   if(LpmEdidWaitProc)
   {
      gm_Print("B_DevID= 0x%x",Sp_I2CSlavePtr->B_DevID);      

      if(Sp_I2CSlavePtr->B_DevID == NVRAM_DEVICE)
      {
         gm_Print("Slave1-Buffer= 0x%x",Sp_I2CSlavePtr->Buffer[0]);      
         _fmemcpy(Sp_I2CSlavePtr->Buffer, &DprxVirtualEdid.Ba_EDIDReadBuffer[DprxVirtualEdid.W_ReadPointer], 8);
         DprxVirtualEdid.W_ReadPointer += 8;
         Lpm_I2CSlaveSetReply(8, B_SlaveCh);
      }

      // clear read fifo
      gm_SetRegBitsByte(I2C_SLAVE_CTRL2, I2C_SLAVE_READ_FIFO_RESET);
      gm_ClearRegBitsByte(I2C_SLAVE_CTRL2, I2C_SLAVE_READ_FIFO_RESET);
      LpmEdidWaitProc = 0;
   }

   return B_Status;
}
#endif

#if I2C_WRITE_FIFO_RESET
//******************************************************************************
// FUNCTION:   IsResetWriteFifo
// USAGE:      option to reset write FIFO
// INPUT:      None
// OUTPUT:     None
// GLOBALS:    None
// USED_REGS:  None
//******************************************************************************
BOOL far IsResetWriteFifo(void)
{
	    return FALSE;
}
#endif
#endif

//******************************************************************************
// FUNCTION:   Lpm_I2CSlaveCheckCS
// USAGE:
//
// INPUT:
// OUTPUT:
// GLOBALS:
//******************************************************************************
BOOL far Lpm_I2CSlaveCheckCS(BYTE B_DevID, BYTE B_Len, BYTE far*Bp_Buff)
{
   BYTE B_Count;
   BYTE B_Checksum = (B_DevID & I2C_ADDRESS_BITS);           // Init checksum

   for (B_Count = 0; B_Count < (B_Len + 2); B_Count++)       // Calculate packet checksum
   {
      B_Checksum ^= Bp_Buff[B_Count];
      //gm_Print("0x%X",Bp_Buff[B_Count]);
   }

   if (B_Checksum == Bp_Buff[(B_Len + 2)])                  // Checksum correct?
   {
      return TRUE;                                          // Yes
   }
   return FALSE;                                            // No
}


//******************************************************************************

