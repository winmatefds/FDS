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
// MODULE:      Lpm_PseudoLT.c
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

#ifdef DP_PSEUDO_LINK
//Below setting need to check Athena SPEC 
#define DPRX_AUX_DPCD_PHYSICAL_BASE    0x1F000ul
#define DPRX_AUX_DPCD_RANGE_MIN        0x0000ul
#define DPRX_AUX_DPCD_RANGE_MAX        0x0FFFul 
#define DPRX_AUX_IS_CORRECT_DPCD_ADDR(AuxAddr)  ((AuxAddr) >= DPRX_AUX_DPCD_RANGE_MIN && (AuxAddr) <= DPRX_AUX_DPCD_RANGE_MAX)
#define ComboRegBlock	(0x9E50-0x9DC0) 

BYTE AlignStatusUpdate=FALSE; 



#define DpcdReadRegByte(W_RegAddr) \
    gm_ReadRegByteFar(DPRX_AUX_DPCD_PHYSICAL_BASE, W_RegAddr)
#define DpcdWriteRegByte(W_RegAddr, B_RegValue) \
    gm_WriteRegByteFar(DPRX_AUX_DPCD_PHYSICAL_BASE, W_RegAddr, B_RegValue)

//******************************************************************************
//
//  C O D E
//
//******************************************************************************

//***************************************************************************** 
// FUNCTION     : void Lpm_PseudoAuxNativeRequest(BYTE B_Request, DWORD DW_Addr, BYTE  B_Len, AUX_CH B_AuxCh) 
// USAGE        : Handle received Native AUX requests
// INPUT        : B_Request - request command
//                DW_Addr - request address
//                B_Len - request length
//                B_AuxCh - Aux channel
// OUTPUT       : 
// GLOBALS      : 
// USED_REGS    : 
//***************************************************************************** 
void Lpm_PseudoAuxNativeRequest(BYTE B_Request, DWORD DW_Addr, BYTE  B_Len, AUX_CH B_AuxCh)
 {

 	if(DPRX_AUX_IS_CORRECT_DPCD_ADDR(DW_Addr))
 	{
 		BYTE Request=B_Request;
		BYTE Len=B_Len;
		DWORD Addr=DW_Addr;

		BYTE Pattern=DpcdReadRegByte(DPRX_DPCD_TRAINING_PATRN_SET)&0x03;
		BYTE Lane=DpcdReadRegByte(DPRX_DPCD_MAX_LANE_COUNT)&0x0f;		
			
		Len++;	 				//Kam for pass while rule 

		gm_WriteRegByte(LPM_C0_AUX2OCM_RPLY_COMM+B_AuxCh*ComboRegBlock, AUX_REPLY_CMD_AUX_ACK);// | DPRX_AUX2OCM_RPLY_CMD_RDY);
		if ((Request & AUX_REQUEST_DIRECTION_MASK) == AUX_REQUEST_NATIVE_WRITE)
	       {	//Write

	              gmt_DP_REG_ADDR ReceiveBuf;
	              BYTE B_DpcdData;

	              gm_WriteRegByte(LPM_C0_AUX2OCM_RPLY_LEN+B_AuxCh*ComboRegBlock, 0);
			if (!(Len == 0 || Len > 16))		// HW buffer only 16 Byte
	              {
	                    ReceiveBuf = 0x7E0;		//Actual address on DPRX_AUX_DPCD_PHYSICAL_BASE + 0x7e0
	                    while (Len)
	                    {

	                        	B_DpcdData = DpcdReadRegByte(ReceiveBuf);
	                        	DpcdWriteRegByte(Addr,B_DpcdData);
	                        	ReceiveBuf++;
	                        	Addr++;
				   	Len--;
	                    }
	              }
	      	}
		else
	      	{	//Read
	      		gmt_DP_REG_ADDR SendBuf;
	              BYTE B_DpcdData;

			SendBuf= 0x7F0;
			while (Len)
		       {
		             	B_DpcdData = DpcdReadRegByte(Addr);
				#if 1	//This Part is a trick for pseudo link 				
				if (Addr==DPRX_DPCD_L1L0_STATUS)
				{
					if (Pattern==1)
						DpcdWriteRegByte(SendBuf, 0x11);								
					if (Pattern==2)
						DpcdWriteRegByte(SendBuf,0x77);	

					AlignStatusUpdate=TRUE;
				}
				else if (Addr==DPRX_DPCD_L3L2_STATUS && Lane==4)
				{
					if (Pattern==1)
						DpcdWriteRegByte(SendBuf, 0x11);								
					if (Pattern==2)
						DpcdWriteRegByte(SendBuf, 0x77);	
					AlignStatusUpdate=TRUE;
				}
				else if (Addr==DPRX_DPCD_LANE_ALIGN_STATUS && AlignStatusUpdate==TRUE)
				{
					AlignStatusUpdate=FALSE;
					if (Pattern==1)
						DpcdWriteRegByte(SendBuf, 0x00);								
					if (Pattern==2)
					{
						DpcdWriteRegByte(SendBuf, 0x01);	

						//Send notice to Mission because LT is Ok now!
					}
				}
					else
				#endif
						DpcdWriteRegByte(SendBuf, B_DpcdData);

							
		              Addr++;
		              SendBuf++;
				Len--;							
			}
		      	gm_WriteRegByte(LPM_C0_AUX2OCM_RPLY_LEN+B_AuxCh*ComboRegBlock,B_Len);		//0 is 1 Byte etc.. 
		}
        }
        else
        {	//Wrong address 
            	gm_WriteRegByte(LPM_C0_AUX2OCM_RPLY_LEN+B_AuxCh*ComboRegBlock, 0);
            	gm_WriteRegByte(LPM_C0_AUX2OCM_RPLY_COMM+B_AuxCh*ComboRegBlock, AUX_REPLY_CMD_I2C_NACK);
        }
}

#endif
//******************************************************************************

