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
// MODULE:      Lpm_Ddcci.c
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


//****************************************************************************** 
//
//  G L O B A L   V A R I A B L E S 
//
//****************************************************************************** 
BYTE ExecuteMemTest = 0;
#ifdef AUX_RX_C2_SUPPORT
BYTE ExecuteMemTest2 = 0;
#endif

BYTE gB_DDC2BiDevId = DDC2BI_ADDR_ID;
BOOL gB_DdcciWaitForParsing[I2C_SLAVE_NUM_CH] = {FALSE,FALSE,FALSE,FALSE};


#ifdef DDCCI_SUPPORT

static VCPFeaturePktType VCPFeature;

extern I2C_SLAVE_TYPE* Sp_I2CSlavePtr;
extern BOOL powerUpRequest;
extern volatile BYTE gB_I2cSlaveSel;
extern BYTE gB_Ddc2Bi_I2C_Status[I2C_SLAVE_NUM_CH];

static void Lpm_DdcciParser(BYTE far*B_BuffPtr, HANDLER_USE_t B_whichHandler);
static BOOL Lpm_DdcciVcpParseCommand(BYTE SetVcp);
#ifdef AUX_RX_SUPPORT
static void Lpm_DdcciAuxrxSetReply(BYTE far*B_BuffPtr, AUX_CH B_AuxCh);
#endif
static WORD EndianConv(WORD W_Input);


//******************************************************************************
//
//  C O D E
//
//******************************************************************************
//***************************************************************************** 
// FUNCTION     : void far Lpm_DdcciCmdHandler(void) 
// USAGE        : Handle the received DDC2Bi messages from I2C slave channels and Aux channels
// INPUT        : none 
// OUTPUT       : 
// GLOBALS      : 
// USED_REGS    : 
//***************************************************************************** 
void Lpm_DdcciCmdHandler(void)
{
	HANDLER_USE_t Handler;
	SLAVE_CH SlaveCh;

#ifndef UART_DEBUG_SUPPORT
	for(SlaveCh=I2C_SLAVE1_CH, Handler=I2C_SLAVE1_USE; SlaveCh<I2C_SLAVE_NUM_CH; SlaveCh++, Handler++)
#else
	for(SlaveCh=I2C_SLAVE1_CH, Handler=I2C_SLAVE1_USE; SlaveCh<=I2C_SLAVE3_CH; SlaveCh++, Handler++)
#endif
	{
		if(gB_DdcciWaitForParsing[SlaveCh] == TRUE)
		{
			Lpm_I2CSlaveSetChannel(SlaveCh);
			//gm_Print("CH%d: Lpm_DdcciCmdHandler",SlaveCh+1);
			Lpm_DdcciParser(Lpm_I2CSlaveGetAddrMsgBuffer(SlaveCh), Handler); //ignore 0x51
			gB_Ddc2Bi_I2C_Status[SlaveCh] = MSG_PARSED;
			gB_DdcciWaitForParsing[SlaveCh] = FALSE;
	        // clear read fifo
	        gm_SetRegBitsByte(I2C_SLAVE_CTRL2, I2C_SLAVE_READ_FIFO_RESET);
	        gm_ClearRegBitsByte(I2C_SLAVE_CTRL2, I2C_SLAVE_READ_FIFO_RESET);			
		}
	}	

	#ifdef AUX_RX_SUPPORT
	//Check if GProbe commands or DDCCI data from Aux channels
    if( ExecuteMemTest == EXEC_MEM_DEBUG )
    {
        BYTE *Bp_Buff = Lpm_AuxRxI2CSwGetBuff(AUX_CH_C0);
        ExecuteMemTest = 0;
        Lpm_DdcciParser(Bp_Buff, AUX_C0_USE);
    }
	#ifdef AUX_RX_C2_SUPPORT   
    if( ExecuteMemTest2 == EXEC_MEM_DEBUG )
    {
        BYTE *Bp_Buff = Lpm_AuxRxI2CSwGetBuff(AUX_CH_C2);
        ExecuteMemTest2 = 0;
        Lpm_DdcciParser(Bp_Buff, AUX_C2_USE);
    }
    #endif // #ifdef AUX_RX_C2_SUPPORT
	#endif
}

//***************************************************************************** 
// FUNCTION     : static void Lpm_DdcciParser(BYTE *B_BuffPtr, HANDLER_USE_t B_whichHandler) 
// USAGE        : Parses the received DDCCI message, checks for syntax and executes the 
//                command. 
// INPUT        : none 
// OUTPUT       : 
// GLOBALS      : 
// USED_REGS    : 
//***************************************************************************** 
#define GET_VCP_RETURN_BYTES	10
static void Lpm_DdcciParser(BYTE far*B_BuffPtr, HANDLER_USE_t B_whichHandler)
{
   BYTE readExpected = 1;
   BYTE far*DdcciCmdPtr = B_BuffPtr;

   if((B_whichHandler==I2C_SLAVE1_USE)||(B_whichHandler==I2C_SLAVE2_USE)||
      (B_whichHandler==I2C_SLAVE3_USE)||(B_whichHandler==I2C_SLAVE4_USE))
   {
      DdcciCmdPtr++;
   }

   {
   /*   BYTE i;
      for(i=0; i<7; i++)
      gm_Print("DdcciBuf=0x%x",B_BuffPtr[i]);
   */}
	switch(*(((BYTE*)DdcciCmdPtr)+1))
	{
		case DDC2B_CMD_GetVCPFeature:		
		//gm_Print("DDC2B_CMD_GetVCPFeature",0);
			//VCPPage = 0; //select Page0
			VCPFeature.value_BigEndian = 0;
			VCPFeature.max_BigEndian = 0xFFFF;
			VCPFeature.vcp_opcode = (((DDC2Bi_GetVCPFeatureCmd_t*)DdcciCmdPtr)->vcp_opcode);
			(((DDC2Bi_GetVCPFeature_ReplyCmd_t*)B_BuffPtr)->length) = 0x08 | DDC2Bi_CONTROL_STATUS_FLAG;
			(((DDC2Bi_GetVCPFeature_ReplyCmd_t*)B_BuffPtr)->command) = DDC2B_CMD_GetVCPFeature_Reply;
			(((DDC2Bi_GetVCPFeature_ReplyCmd_t*)B_BuffPtr)->result) = Lpm_DdcciVcpParseCommand(GET_VCP);
			(((DDC2Bi_GetVCPFeature_ReplyCmd_t*)B_BuffPtr)->a_VCPFeature) = VCPFeature;
			//gm_Print("Get VCP opcode: %x, res: %x", VCPFeature.vcp_opcode, 
			//	(((DDC2Bi_GetVCPFeature_ReplyCmd_t*)gExtDebugCmdPtr)->result));
			break;
		
		case DDC2B_CMD_SetVCPFeature:
		//gm_Print("DDC2B_CMD_SetVCPFeature",0);
			//VCPPage = 0; //select Page0
			VCPFeature.vcp_opcode	= (((DDC2Bi_SetVCPFeatureCmd_t*)DdcciCmdPtr)->vcp_opcode);
			VCPFeature.value_BigEndian	  = (((DDC2Bi_SetVCPFeatureCmd_t*)DdcciCmdPtr)->val_BigEndian);
			Lpm_DdcciVcpParseCommand(SET_VCP);
			*(BYTE far*)B_BuffPtr = 0x80;
			readExpected = 0;
			//gm_Print("Set VCP opcode: %x, res: %x", VCPFeature.vcp_opcode, VCPFeature.value_BigEndian);
			break;

		case DDC2B_CMD_SaveCurrentSettings:
         //gm_Print("Save Current Settings",0);
         //SaveModeIndependentSettings();
         *(BYTE far*)B_BuffPtr = 0x80; //no messages back         
         readExpected = 0;         
         break;
	     /*
		case DDC2B_CMD_CapabilitiesRequest:
		case DDC2B_CMD_GetTimingReport:
		case DDC2B_CMD_TableWrite:
		case DDC2B_CMD_TableRead:
			break;
	     */
		default:
			*(BYTE far*)B_BuffPtr = 0x80; //no messages back
			readExpected = 0;			
			//gm_Print("Unsupported DDC2ci Command [%d]", *((BYTE*)gExtDebugCmdPtr+2));
			break;
	}

	if(readExpected)
	{
		#ifdef AUX_RX_SUPPORT
		if((B_whichHandler == AUX_C0_USE)||(B_whichHandler == AUX_C2_USE))
		{
			AUX_CH B_AuxCh = AUX_CH_C0;
			if(B_whichHandler == AUX_C2_USE)
				B_AuxCh = AUX_CH_C2;
			
			Lpm_DdcciAuxrxSetReply(B_BuffPtr, B_AuxCh);
		}
		else
		#endif
     
		if((B_whichHandler==I2C_SLAVE1_USE)||(B_whichHandler==I2C_SLAVE2_USE)||
			(B_whichHandler==I2C_SLAVE3_USE)||(B_whichHandler==I2C_SLAVE4_USE))
		{
			BYTE B_SlaveCh = B_whichHandler - I2C_SLAVE1_USE;
         BYTE B_Length = B_BuffPtr[0] & 0x7F;
         BYTE B_Checksum, i;
         
         B_Checksum = ((DDC2B_SRC_ADDRESS-1) ^ (Sp_I2CSlavePtrGlobal[B_SlaveCh]->B_DevID & I2C_ADDRESS_BITS));
         for(i = 0; i <= B_Length; i++)            // Process the whole packet 
             B_Checksum ^= B_BuffPtr[i];  // Update checksum 
      
         B_BuffPtr[B_Length+1] = B_Checksum;				

         if(readExpected)
         {
			Lpm_I2CSlaveSetReply(B_Length+3, B_SlaveCh);//map to SLAVE_CH enum
		}
         else
         {
            //Reset receive buffer counter to avoid buffer overflow
            Sp_I2CSlavePtrGlobal[B_SlaveCh]->W_RcvCount = 0;
            Sp_I2CSlavePtrGlobal[B_SlaveCh]->W_AbsRcvCount = 0;
         }
		}
	}
}

//******************************************************************************
// FUNCTION       : BYTE DDC2Bi_VCP_ParseCommand(BYTE SetVcp)
// USAGE          : VCP Commands Parser.
// INPUT          : SetVcp
// OUTPUT         : Returns 0 if feature is supported and 0 otherwise.
// GLOBALS        : None
// USED_REGS      : None
//******************************************************************************
static BOOL Lpm_DdcciVcpParseCommand(BYTE SetVcp)
{
    BOOL IsFeatureSupported = TRUE;
    //default values for Max and Current Values
    WORD W_VCPValue = 0;
    WORD W_VCPMax = 0x100;
    BYTE B_VCPType = 0;

    //converts values to Little Endian
    if (SetVcp)
        W_VCPValue = EndianConv(VCPFeature.value_BigEndian);   
   
    switch (VCPFeature.vcp_opcode)
    {
        case DDC2B_CMD_VCP_DisplayPowerMode: 
        {
            if (SetVcp == SET_VCP && W_VCPValue)
            {
            	#ifdef POWER_MODE_HANDLE
            	gmt_Event2PwrHdr PwrEvent = LPM_EVNT2PWR_NONE;
				
                //gm_Print("DisplayPowerMode_SET_VCP",0);
                
                if(W_VCPValue == DPM_ON)
                {
                  //gm_Print("DPM_ON",0);
                	if(Lpm_PowerModeGetState() != LPM_NormalPowerMode)
	                    PwrEvent = LPM_EVNT2PWR_POWER_ON_NOTICE;			
                }
                else if(W_VCPValue == DPM_OFF)
                {
                  //gm_Print("DPM_OFF",0);
                	if(Lpm_PowerModeGetState() == LPM_NormalPowerMode)
                    PwrEvent = LPM_EVNT2PWR_SLEEP_NOTICE;			
                }
                else if(W_VCPValue == DPM_HARDOFF)
                {
                  //gm_Print("DPM_HARDOFF",0);
                	if(Lpm_PowerModeGetState() != LPM_PowerOffMode)
                    PwrEvent = LPM_EVNT2PWR_PWRKEY;
                }
				
                Lpm_PowerModeSetEvent2PwrHdr(PwrEvent);
                #endif
            }
            else if (SetVcp == GET_VCP)
            {  
                #ifdef POWER_MODE_HANDLE 
            	BYTE PwrState = Lpm_PowerModeGetState();
				
                //gm_Print("DisplayPowerMode_GET_VCP",0);
            
                if(PwrState == LPM_NormalPowerMode)
                    W_VCPValue = DPMS_ON;
                else if(PwrState == LPM_LowPowerMode)
                    W_VCPValue = DPMS_SUSPEND;
                else	//LPM_PowerOffMode
                    W_VCPValue = DPM_OFF;
                #else
                W_VCPValue = DPM_ON;
                #endif

                B_VCPType = VCP_SETPARAMETER;
                //gm_Print("Current Power Mode: %d", W_VCPValue);
            }
            break;
        }
        default:
        {
           //error condition - unsupported vcp opcode
           IsFeatureSupported = FALSE;
           break;
        }
  	}

   if (!SetVcp) //Get VCP
   {
      VCPFeature.max_BigEndian = EndianConv(W_VCPMax);
      VCPFeature.value_BigEndian = EndianConv(W_VCPValue);
      VCPFeature.vcp_type = B_VCPType;
   }
   
   if (IsFeatureSupported)
   {
      return VCP_SUPPORTED; 
   }
   else //unsupported VCP code
   {
      VCPFeature.max_BigEndian = 0;
      VCPFeature.value_BigEndian = 0;
      return VCP_UNSUPPORTED;
   }
}

//******************************************************************************
// FUNCTION:   Lpm_IsDDCCITransaction
// USAGE:  Used to check if DDCCI transaction
//
// INPUT:  pointer to received buffer
// OUTPUT: TRUE means DDCCI transaction, otherwise return FALSE
// GLOBALS:
//******************************************************************************
BYTE Lpm_IsDdcciTransaction(BYTE far*B_BuffPtr)
{
	BYTE I2C_as_Vcp_Opcode = B_BuffPtr[2];
	
	switch(I2C_as_Vcp_Opcode)
	{
		case DDC2B_CMD_GetVCPFeature:
		case DDC2B_CMD_SetVCPFeature:	
		case DDC2B_CMD_SaveCurrentSettings:	
		case DDC2B_CMD_CapabilitiesRequest:
		case DDC2B_CMD_GetTimingReport:
		case DDC2B_CMD_TableRead:
		case DDC2B_CMD_TableWrite:
		//gm_Print("Lpm DDC Transaction",0);
			return TRUE;
	}
	
	return FALSE;
}

#ifdef AUX_RX_SUPPORT
//******************************************************************************
// FUNCTION       : static void Lpm_DdcciAuxrxSetReply(BYTE *B_BuffPtr)
// USAGE          : Reply DDCCI via AuxRx channels
// INPUT          : B_BuffPtr
// OUTPUT         : None
// GLOBALS        : None
// USED_REGS      : None
//******************************************************************************
static void Lpm_DdcciAuxrxSetReply(BYTE far*B_BuffPtr, AUX_CH B_AuxCh)
{
	BYTE B_Checksum, B_Length, Index;
	SW_TransactionStruct_t *SW_TrnsData = Lpm_AuxRxI2CSwGetAddrModuleData(B_AuxCh);
	
	B_Checksum = ((DDC2B_SRC_ADDRESS-1) ^ SW_TrnsData->B_DevID);
	B_Length = B_BuffPtr[0] ^ DDC2Bi_CONTROL_STATUS_FLAG;
	
	for (Index=0; Index < B_Length + 1; Index++)			// Process the whole packet 
	{
		B_Checksum ^= B_BuffPtr[Index]; 			  // Update checksum 
		//gm_Print("R 0x%X",B_BuffPtr[Index]); 
	}
	
	B_BuffPtr[B_Length + 1] = B_Checksum;
	B_BuffPtr[B_Length + 2] = DDC2B_DEST_ADDRESS;
	
	SW_TrnsData->B_ReplyReady = TRUE;	

}
#endif

//******************************************************************************
// FUNCTION       : static WORD EndianConv(WORD W_Input) 
// USAGE          : Swaps the upper and the lower byte of the input Word
// INPUT          : W_Input
// OUTPUT         : The converted word
//******************************************************************************
static WORD EndianConv(WORD W_Input) 
{
   return (W_Input >> 8) | (W_Input << 8);
}

#endif	
//******************************************************************************

