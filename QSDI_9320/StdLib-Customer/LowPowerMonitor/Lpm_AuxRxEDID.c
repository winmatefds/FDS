//============================================================================= 
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
//   MODULE : Lpm_AuxRxEDID.c 
// 
//   USAGE  : Handle AuxRx EDID R/W process
// 
//   OWN    : 
// 
//============================================================================= 

//******************************************************************************
//
//  I N C L U D E    F I L E S
//
//******************************************************************************

#include ".\LowPowerMonitor\Lpm_All.h"
#include <string.h>
#include <dos.h>


#ifdef AUX_RX_SUPPORT

//******************************************************************************
//
//  L O C A L    D E F I N I T I O N S
//
//******************************************************************************


//******************************************************************************
//
//  G L O B A L   V A R I A B L E S
//
//******************************************************************************
DprxVirtualEdid_t DprxVirtualEdid;
BYTE gB_AuxTransactionSize = AUX_TRANSACTION_SIZE;
BYTE gB_AuxRx_EdidWrOffsetSize = 1;
extern TAuxRxI2C_InternalData* P_AuxRxI2CInternalData;

#ifdef AUX_RX_C2_SUPPORT
DprxVirtualEdid_t DprxVirtualEdid2;
BYTE gB_AuxTransactionSize2 = AUX_TRANSACTION_SIZE;
BYTE gB_AuxRx_EdidWrOffsetSize2 = 1;
extern TAuxRxI2C_InternalData* P_AuxRxI2CInternalData2;
#endif


//******************************************************************************
//
//  C O D E
//
//******************************************************************************
//******************************************************************************
// FUNCTION:   void Lpm_AuxRxEdidHandlerInit(void)
// USAGE:	    Initialize Aux EDID handler
// INPUT:	   None
// OUTPUT:
// GLOBALS:
// USED_REGS:
//******************************************************************************
void Lpm_AuxRxEdidHandlerInit(AUX_CH B_AuxCh)
{
   if(B_AuxCh == AUX_CH_C0)
   {
   	Lpm_DprxVirtualEdidInit(WR_FREE, AUX_CH_C0);
   	DprxVirtualEdid.Ba_EDIDReadBuffer = (BYTE *)EDID_BUFFER_START;
   }
   #ifdef AUX_RX_C2_SUPPORT
   else if(B_AuxCh == AUX_CH_C2)
   {
   	Lpm_DprxVirtualEdidInit(WR_FREE, AUX_CH_C2);
   	DprxVirtualEdid2.Ba_EDIDReadBuffer = (BYTE *)EDID_BUFFER2_START;
   }
   #endif
}

//******************************************************************************
// FUNCTION:   Lpm_AuxRxEdidHandler(DWORD W_Addr, BYTE B_Req, BYTE B_Len)
// USAGE:      Handle Aux EDID read/write transactions
// INPUT:      B_Addr - I2C device address
//             	B_Len - how many byte to be read/write
//             	B_Req - Request command
//             	B_AuxCh - indicate which Aux channel
// OUTPUT:
// GLOBALS:
// USED_REGS:
//******************************************************************************
void Lpm_AuxRxEdidHandler(DWORD DW_Addr, BYTE B_Req, BYTE B_Len, AUX_CH B_AuxCh)
{
   BYTE B_IsStop;
   BYTE B_Addr = (BYTE)DW_Addr;

   B_Addr <<= 1;
   B_IsStop = (B_Req & AUX_REQUEST_MOT) ? FALSE : TRUE;

   //gm_Print("B_Req=0x%x", B_Req);

   if((B_Req & AUX_REQUEST_COMMAND_MASK) == AUX_REQUEST_I2C_READ)
   {
      //gm_Print("Lpm_AuxRxEdidReadHandler",0); 
      Lpm_AuxRxEdidReadHandler(B_Addr, B_Len, B_IsStop, B_AuxCh);
   }
   else if((B_Req & AUX_REQUEST_COMMAND_MASK) == AUX_REQUEST_I2C_WRITE)
   {
      //gm_Print("Lpm_AuxRxEdidWriteHandler",0);
      Lpm_AuxRxEdidWriteHandler(B_Addr, B_Len, B_IsStop, B_AuxCh);
   }
   else
   {
      //gm_Print("TP_AuxEdid_01",0); 
   	//Status update: it should not happen in a normal way of events
   	gm_WriteRegByte(AUXREG(LPM_C0_AUX2OCM_RPLY_LEN, B_AuxCh), 0);
   	gm_WriteRegByte(AUXREG(LPM_C0_AUX2OCM_RPLY_COMM, B_AuxCh), AUX_REPLY_CMD_AUX_ACK);
   }

    //return FALSE;
}

//******************************************************************************
// FUNCTION:   Lpm_AuxRxEdidReadHandler(B_Addr, B_Len, B_IsStop)
// USAGE:      Handle Aux EDID read transactions
// INPUT:      B_Addr - I2C device address
//             	B_Len - how many byte to be read
//             	B_IsStop - Is Stop or MOT
//             	B_AuxCh - indicate which Aux channel
// OUTPUT:
// GLOBALS:
// USED_REGS:
//******************************************************************************
void Lpm_AuxRxEdidReadHandler(BYTE B_Addr, BYTE B_Len, BYTE B_IsStop, AUX_CH B_AuxCh)
{

	if(B_AuxCh == AUX_CH_C0)
	{
		DprxVirtualEdid.B_WrState = WR_FREE;

	 	if(B_Len == 0)
	    {
			gm_WriteRegByte(LPM_C0_AUX2OCM_RPLY_LEN, 0);
	    	gm_WriteRegByte(LPM_C0_AUX2OCM_RPLY_COMM, AUX_REPLY_CMD_AUX_ACK);

	    	if(B_IsStop)
	       	{
		      	DprxVirtualEdid.W_ReadPointer = 0;
			}
	    }
	    else
		{
			if(B_Len > gB_AuxTransactionSize)
				B_Len = gB_AuxTransactionSize;

			Lpm_AuxRxEdidRead(B_Addr, P_AuxRxI2CInternalData->AB_DataBuff, B_Len, B_AuxCh);
			Lpm_AuxRxSetReplyData(P_AuxRxI2CInternalData->AB_DataBuff, B_Len, B_AuxCh);
			gm_WriteRegByte(LPM_C0_AUX2OCM_RPLY_COMM, AUX_REPLY_CMD_AUX_ACK);
	    }
	}
	#ifdef AUX_RX_C2_SUPPORT
	else if(B_AuxCh == AUX_CH_C2)
	{
		DprxVirtualEdid2.B_WrState = WR_FREE;

	 	if(B_Len == 0)
	    {
			gm_WriteRegByte(LPM_C2_AUX2OCM_RPLY_LEN, 0);
	    	gm_WriteRegByte(LPM_C2_AUX2OCM_RPLY_COMM, AUX_REPLY_CMD_AUX_ACK);

	    	if(B_IsStop)
	       	{
		      	DprxVirtualEdid2.W_ReadPointer = 0;
			}
	    }
	    else
		{
			if(B_Len > gB_AuxTransactionSize2)
				B_Len = gB_AuxTransactionSize2;

			Lpm_AuxRxEdidRead(B_Addr, P_AuxRxI2CInternalData2->AB_DataBuff, B_Len, B_AuxCh);
			Lpm_AuxRxSetReplyData(P_AuxRxI2CInternalData2->AB_DataBuff, B_Len, B_AuxCh);
			gm_WriteRegByte(LPM_C2_AUX2OCM_RPLY_COMM, AUX_REPLY_CMD_AUX_ACK);
	    }
	}		
	#endif
}

//******************************************************************************
// FUNCTION:   Lpm_AuxRxEdidRead(BYTE B_DevAddress, BYTE *Bp_Buffer, WORD W_Length)
// USAGE:      Reading from the EDID buffer using ISR
// INPUT:      BYTE B_DevAddress - I2C address
//             	BYTE* Bp_Buffer - pointer to read buffer
//             	WORD W_Length - how many bytes should be read
//             	B_AuxCh - indicate which Aux channel
// OUTPUT:
// GLOBALS:
// USED_REGS:  None
//******************************************************************************
void Lpm_AuxRxEdidRead(BYTE B_DevAddress, BYTE *Bp_Buffer, WORD W_Length, AUX_CH B_AuxCh)
{
   	if(B_DevAddress == EDID_EEPROM_DEV_ADDR)
   	{
   		if(B_AuxCh == AUX_CH_C0)
   		{
	   		_fmemcpy((BYTE *)Bp_Buffer, (BYTE *)&DprxVirtualEdid.Ba_EDIDReadBuffer[DprxVirtualEdid.W_ReadPointer], W_Length);
	   		DprxVirtualEdid.W_ReadPointer += (BYTE)W_Length;
   		}
		#ifdef AUX_RX_C2_SUPPORT
		else if(B_AuxCh == AUX_CH_C2)
   		{
	   		_fmemcpy((BYTE *)Bp_Buffer, (BYTE *)&DprxVirtualEdid2.Ba_EDIDReadBuffer[DprxVirtualEdid2.W_ReadPointer], W_Length);
	   		DprxVirtualEdid2.W_ReadPointer += (BYTE)W_Length;
   		}		
		#endif
	}
}

//******************************************************************************
// FUNCTION:   Lpm_AuxRxEdidWriteHandler(B_Addr, B_Len, B_IsStop)
// USAGE:      Handle Aux EDID write transactions
// INPUT:      B_Addr - I2C device address
//             	B_Len - how many byte to be written
//             	B_IsStop - Is Stop or MOT
//             	B_AuxCh - indicate which Aux channel
// OUTPUT:
// GLOBALS:
// USED_REGS:
//******************************************************************************
void Lpm_AuxRxEdidWriteHandler(BYTE B_Addr, BYTE B_Len, BYTE B_IsStop, AUX_CH B_AuxCh)
{
   if(B_AuxCh == AUX_CH_C0)
   {
      if(B_Len != 0)
      {
         //BYTE i;

         //gm_Print("B_Addr=0x%x",B_Addr);
         //gm_Print("B_Len=0x%x",B_Len);
         Lpm_AuxRxGetChunkData(P_AuxRxI2CInternalData->AB_DataBuff, B_Len, B_AuxCh);

         //for(i=0; i<B_Len; i++)
         //   gm_Print("AB_DataBuff=0x%x", P_AuxRxI2CInternalData->AB_DataBuff[i]);


         Lpm_AuxRxEdidWrite(B_Addr, P_AuxRxI2CInternalData->AB_DataBuff, B_Len, B_AuxCh);
         gm_WriteRegByte(LPM_C0_AUX2OCM_RPLY_LEN, 0);
         gm_WriteRegByte(LPM_C0_AUX2OCM_RPLY_COMM, AUX_REPLY_CMD_AUX_ACK);

         if(B_IsStop)
         {
            //gm_Print("B_IsStop",0);
            DprxVirtualEdid.B_WrState = WR_STOP;
            Lpm_AuxRxEdidWrite(B_Addr, NULL_PTR, 0, B_AuxCh);
         }
      }
      else
      {
         if(B_IsStop)
         {
            DprxVirtualEdid.B_WrState = WR_STOP;
         }
         else
         {
            DprxVirtualEdid.B_WrState = WR_START;
         }
         Lpm_AuxRxEdidWrite(B_Addr, NULL_PTR, 0, B_AuxCh);
      }
   }
   #ifdef AUX_RX_C2_SUPPORT
   else if(B_AuxCh == AUX_CH_C2)
   {
      if(B_Len != 0)
      {
         Lpm_AuxRxGetChunkData(P_AuxRxI2CInternalData2->AB_DataBuff, B_Len, B_AuxCh);

         Lpm_AuxRxEdidWrite(B_Addr, P_AuxRxI2CInternalData2->AB_DataBuff, B_Len, B_AuxCh);
         gm_WriteRegByte(LPM_C2_AUX2OCM_RPLY_LEN, 0);
         gm_WriteRegByte(LPM_C2_AUX2OCM_RPLY_COMM, AUX_REPLY_CMD_AUX_ACK);

         if(B_IsStop)
         {
            DprxVirtualEdid2.B_WrState = WR_STOP;
            Lpm_AuxRxEdidWrite(B_Addr, NULL_PTR, 0, B_AuxCh);
         }
      }
      else
      {
         if(B_IsStop)
         {
            DprxVirtualEdid2.B_WrState = WR_STOP;
         }
         else
         {
            DprxVirtualEdid2.B_WrState = WR_START;
         }
         Lpm_AuxRxEdidWrite(B_Addr, NULL_PTR, 0, B_AuxCh);
      }
   }		
   #endif
}

//******************************************************************************
// FUNCTION:   Lpm_AuxRxEdidWrite
// USAGE:      Writing to the EEPROM using ISR
// INPUT:      BYTE W_DevAddress - I2C address
//             	BYTE* Bp_Buffer - pointer to read buffer
//            	WORD W_Length - how many bytes should be read
//             	B_AuxCh - indicate which Aux channel
// OUTPUT:
// GLOBALS:
// USED_REGS:  None
//******************************************************************************
void Lpm_AuxRxEdidWrite(BYTE B_DevAddress, BYTE *Bp_Buffer, WORD W_Length, AUX_CH B_AuxCh)
{

	if(B_AuxCh == AUX_CH_C0)
	{
  	if((W_Length != 0) && (DprxVirtualEdid.B_WrState == WR_FREE))
  	{   // for run two steps one time
		Lpm_DprxVirtualEdidInit(WR_ADDR1, B_AuxCh);
  	}

  	switch(DprxVirtualEdid.B_WrState)
  	{
  		case WR_START:
      		Lpm_DprxVirtualEdidInit(WR_ADDR1, B_AuxCh);
		break;

      	case WR_ADDR1:
      		if(gB_AuxRx_EdidWrOffsetSize == 1)
         	{
         		DprxVirtualEdid.W_Offset = Bp_Buffer[0];

            	if(W_Length > 1)
            	{
            		//#if DPRX_EDID_FLASH_WR_ENABLE
             	   	//_fmemcpy(&DprxVirtualEdid.Ba_EDIDWriteBuffer[DprxVirtualEdid.W_WritePointer], &Bp_Buffer[1], W_Length-1);
             	   	//#endif
   					DprxVirtualEdid.W_WritePointer += (BYTE)(W_Length - 1);
            	}
            	DprxVirtualEdid.B_WrState = WR_DATA;
         	}
         	else
         	{
          		if(W_Length > 1)
            	{
             		DprxVirtualEdid.W_Offset = (Bp_Buffer[0] << 8) || Bp_Buffer[1];
             	   	//#if DPRX_EDID_FLASH_WR_ENABLE
               		//_fmemcpy(&DprxVirtualEdid.Ba_EDIDWriteBuffer[DprxVirtualEdid.W_WritePointer], &Bp_Buffer[2], W_Length-2);
             	   	//#endif
               		DprxVirtualEdid.W_WritePointer += (BYTE)(W_Length - 2);
               		DprxVirtualEdid.B_WrState = WR_DATA;
            	}
            	else
            	{
             		DprxVirtualEdid.W_Offset = (Bp_Buffer[0] << 8);
               		DprxVirtualEdid.B_WrState = WR_ADDR2;
            	}
         	}
         	DprxVirtualEdid.W_ReadPointer = Bp_Buffer[0];
		break;

		case WR_ADDR2:
      		DprxVirtualEdid.W_Offset |= Bp_Buffer[1];
      		//#if DPRX_EDID_FLASH_WR_ENABLE
         	//_fmemcpy(&DprxVirtualEdid.Ba_EDIDWriteBuffer[DprxVirtualEdid.W_WritePointer], &Bp_Buffer[1], W_Length-1);
      		//#endif
         	DprxVirtualEdid.W_WritePointer += W_Length - 1;
         	DprxVirtualEdid.B_WrState = WR_DATA;
		break;

      	case WR_DATA:
      		//#if DPRX_EDID_FLASH_WR_ENABLE
      		//_fmemcpy(&DprxVirtualEdid.Ba_EDIDWriteBuffer[DprxVirtualEdid.W_WritePointer], Bp_Buffer, W_Length);
      		//#endif
         	DprxVirtualEdid.W_WritePointer += W_Length;
         	break;

     	case WR_STOP:
     		//#if DPRX_EDID_FLASH_WR_ENABLE
     		//if(gB_AuxRx_EdidWrProtect == AUX_EDID_WRITE_UNLOCK)
         	{
         	//	_fmemcpy(&DprxVirtualEdid.Ba_EDIDReadBuffer[DprxVirtualEdid.W_Offset], DprxVirtualEdid.Ba_EDIDWriteBuffer, DprxVirtualEdid.W_WritePointer);
         	//	Lpm_DprxVirtualEdidWrite();
        	}
     		//#endif
         	Lpm_DprxVirtualEdidInit(WR_FREE, B_AuxCh);
         break;

     	default:
     		DprxVirtualEdid.B_WrState = WR_FREE;
			UNUSED_VAR(B_DevAddress);//only for pressing warming message
        break;
  	}	
	}
	#ifdef AUX_RX_C2_SUPPORT
	else if(B_AuxCh == AUX_CH_C2)
	{
  	if((W_Length != 0) && (DprxVirtualEdid2.B_WrState == WR_FREE))
  	{   // for run two steps one time
		Lpm_DprxVirtualEdidInit(WR_ADDR1, B_AuxCh);
  	}

  	switch(DprxVirtualEdid2.B_WrState)
  	{
  		case WR_START:
      		Lpm_DprxVirtualEdidInit(WR_ADDR1, B_AuxCh);
		break;

      	case WR_ADDR1:
      		if(gB_AuxRx_EdidWrOffsetSize2 == 1)
         	{
         		DprxVirtualEdid2.W_Offset = Bp_Buffer[0];

            	if(W_Length > 1)
            	{
            		//#if DPRX_EDID_FLASH_WR_ENABLE
             	   	//_fmemcpy(&DprxVirtualEdid.Ba_EDIDWriteBuffer[DprxVirtualEdid.W_WritePointer], &Bp_Buffer[1], W_Length-1);
             	   	//#endif
   					DprxVirtualEdid2.W_WritePointer += (BYTE)(W_Length - 1);
            	}
            	DprxVirtualEdid2.B_WrState = WR_DATA;
         	}
         	else
         	{
          		if(W_Length > 1)
            	{
             		DprxVirtualEdid2.W_Offset = (Bp_Buffer[0] << 8) || Bp_Buffer[1];
             	   	//#if DPRX_EDID_FLASH_WR_ENABLE
               		//_fmemcpy(&DprxVirtualEdid.Ba_EDIDWriteBuffer[DprxVirtualEdid.W_WritePointer], &Bp_Buffer[2], W_Length-2);
             	   	//#endif
               		DprxVirtualEdid2.W_WritePointer += (BYTE)(W_Length - 2);
               		DprxVirtualEdid2.B_WrState = WR_DATA;
            	}
            	else
            	{
             		DprxVirtualEdid2.W_Offset = (Bp_Buffer[0] << 8);
               		DprxVirtualEdid2.B_WrState = WR_ADDR2;
            	}
         	}
         	DprxVirtualEdid2.W_ReadPointer = Bp_Buffer[0];
		break;

		case WR_ADDR2:
      		DprxVirtualEdid2.W_Offset |= Bp_Buffer[1];
      		//#if DPRX_EDID_FLASH_WR_ENABLE
         	//_fmemcpy(&DprxVirtualEdid.Ba_EDIDWriteBuffer[DprxVirtualEdid.W_WritePointer], &Bp_Buffer[1], W_Length-1);
      		//#endif
         	DprxVirtualEdid2.W_WritePointer += W_Length - 1;
         	DprxVirtualEdid2.B_WrState = WR_DATA;
		break;

      	case WR_DATA:
      		//#if DPRX_EDID_FLASH_WR_ENABLE
      		//_fmemcpy(&DprxVirtualEdid.Ba_EDIDWriteBuffer[DprxVirtualEdid.W_WritePointer], Bp_Buffer, W_Length);
      		//#endif
         	DprxVirtualEdid2.W_WritePointer += W_Length;
         	break;

     	case WR_STOP:
     		//#if DPRX_EDID_FLASH_WR_ENABLE
     		//if(gB_AuxRx_EdidWrProtect == AUX_EDID_WRITE_UNLOCK)
         	{
         	//	_fmemcpy(&DprxVirtualEdid.Ba_EDIDReadBuffer[DprxVirtualEdid.W_Offset], DprxVirtualEdid.Ba_EDIDWriteBuffer, DprxVirtualEdid.W_WritePointer);
         	//	Lpm_DprxVirtualEdidWrite();
        	}
     		//#endif
         	Lpm_DprxVirtualEdidInit(WR_FREE, B_AuxCh);
         break;

     	default:
     		DprxVirtualEdid2.B_WrState = WR_FREE;
			UNUSED_VAR(B_DevAddress);//only for pressing warming message
        break;
  	}	
	}	
	#endif
}

//******************************************************************************
// FUNCTION:   Lpm_DprxVirtualEdidInit(B_WrState)
// USAGE:      Initialize VirtualEDID R/W pointer and status
// INPUT:      B_WrState - Write status
//             	B_AuxCh - indicate which Aux channel
// OUTPUT:
// GLOBALS:
// USED_REGS:
//******************************************************************************
void Lpm_DprxVirtualEdidInit(BYTE B_WrState, AUX_CH B_AuxCh)
{
	if(B_AuxCh == AUX_CH_C0)
	{
		DprxVirtualEdid.W_ReadPointer = 0;
		DprxVirtualEdid.W_WritePointer = 0;
		DprxVirtualEdid.B_WrState = B_WrState;
	}
	#ifdef AUX_RX_C2_SUPPORT
	else if(B_AuxCh == AUX_CH_C2)
	{
		DprxVirtualEdid2.W_ReadPointer = 0;
		DprxVirtualEdid2.W_WritePointer = 0;
		DprxVirtualEdid2.B_WrState = B_WrState;
	}
	#endif
}

#if 0//DPRX_EDID_FLASH_WR_ENABLE
//******************************************************************************
// FUNCTION:   Lpm_DprxVirtualEdidWrite(void)
// USAGE:
// INPUT:
// OUTPUT:
// GLOBALS:
// USED_REGS:
//******************************************************************************
void far Lpm_DprxVirtualEdidWrite(void)
{
	gm_WriteProtect_Flash(FALSE);

	//Detect flash type
	gm_FlashAutoDetect(&B_FlashChipID);

	//erase one sector
    gm_FlashEraseBlock(CMD_SECTOR_ERASE, HUGE(DPRX_AUX_EDID_FLASH_256K_ADDR_32BIT));
	
	//program edid data and write back signature
	if(B_FlashChipSize == SIZE_512K)
	{
		gm_FlashWriteBlock(HUGE(DPRX_AUX_EDID_FLASH_512K_ADDR_32BIT), EDID_BUFFER_SIZE, (BYTE far *)DprxVirtualEdid.Ba_EDIDReadBuffer);	
		gm_FlashWriteBlock(HUGE(SIGNATURE_FLASH_512K_ADDR_32BIT), SIGNAUTRE_LEN, (BYTE far *)gB_FlashSignature);	
	}
	else
	{
		gm_FlashWriteBlock(HUGE(DPRX_AUX_EDID_FLASH_256K_ADDR_32BIT), EDID_BUFFER_SIZE, (BYTE far *)DprxVirtualEdid.Ba_EDIDReadBuffer);	
		gm_FlashWriteBlock(HUGE(SIGNATURE_FLASH_256K_ADDR_32BIT), SIGNAUTRE_LEN, (BYTE far *)gB_FlashSignature);	
	}
	gm_WriteProtect_Flash(TRUE);

}
#endif
#endif	//AUX_RX_SUPPORT

