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
// MODULE:      Lpm_AuxRx.c
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

#ifdef AUX_RX_SUPPORT
//****************************************************************************** 
//
//  G L O B A L   V A R I A B L E S 
//
//****************************************************************************** 
SW_TransactionStruct_t SW_Transaction_ModuleData;
gmt_DPRX_AUX_I2C_INTERNAL_DATA AuxRxI2CModuleData;
TAuxRxI2C_InternalData AuxRxI2CInternalData;
#ifdef AUX_RX_C2_SUPPORT
SW_TransactionStruct_t SW_Transaction_ModuleData2;
gmt_DPRX_AUX_I2C_INTERNAL_DATA AuxRxI2CModuleData2;
TAuxRxI2C_InternalData AuxRxI2CInternalData2;
#endif


//******************************************************************************
//
//  C O D E
//
//******************************************************************************
//***************************************************************************** 
// FUNCTION     : void Lpm_AuxRxRequestISR(void) 
// USAGE        : to handle received Aux ISR
// INPUT        :  B_AuxCh - indicate which Aux channel
// OUTPUT       : 
// GLOBALS      : P_AuxRxModuleData 
// USED_REGS    : LPM_Cx_AUX2OCM_REQ_COMM, LPM_Cx_AUX2OCM_REQ_ADDR, LPM_Cx_AUX2OCM_REQ_LEN
//***************************************************************************** 
void Lpm_AuxRxRequestISR(AUX_CH B_AuxCh)
{
   BYTE  B_Request;
   DWORD DW_Addr;
   BYTE  B_Len;

   B_Request = gm_ReadRegByte(AUXREG(LPM_C0_AUX2OCM_REQ,B_AuxCh));
   DW_Addr = gm_ReadRegDWord_LE(AUXREG(LPM_C0_AUX2OCM_REQ_ADDR,B_AuxCh)) & 0x000fffffUL;
   B_Len = gm_ReadRegByte(AUXREG(LPM_C0_AUX2OCM_REQ_LEN,B_AuxCh)) & AUX_REQUEST_LENGTH_MASK; //0 is 1 byte, etc 

   if(B_Request & AUX_REQUEST_NATIVE_BIT)	//Native 
   {
      //gm_Print("Native Aux Request",0);   
      #ifdef DP_PSEUDO_LINK
      Lpm_PseudoAuxNativeRequest(B_Request, DW_Addr, B_Len, B_AuxCh);
      #endif
   }
   else
   {	//I2c 
      //gm_Print("I2C over Aux Request",0);   
      Lpm_AuxRxLLI2CRequest(B_Request, DW_Addr, B_Len, B_AuxCh);
   }

   gm_SetRegBitsWord(AUXREG(LPM_C0_AUX_STROBE,B_AuxCh), DPRX_AUX_FM_RPLY_RDY);
}

//***************************************************************************** 
// FUNCTION     : BOOL Lpm_AuxRxGetChunkData(BYTE far* PB_Dest, BYTE B_Len) 
// USAGE        : Get received Aux chunk data 
// INPUT        : PB_Dest -- RAM buffer destination to store received data 
//                   B_Len -- data length to be read 
//			   B_AuxCh - indicate which Aux channel
// OUTPUT       : is successful 
// GLOBALS      : 
// USED_REGS    :  
//***************************************************************************** 
BOOL Lpm_AuxRxGetChunkData(BYTE* PB_Dest, BYTE B_Len, AUX_CH B_AuxCh)
{
    WORD W_CurReg;

    if(B_Len == 0 || B_Len > AUX_MAX_BYTES_IN_CHUNK)
        return FALSE;

    W_CurReg = AUX_REQUEST_START_DPCD;

#if 1
	if(B_AuxCh == AUX_CH_C0)
	{
	    while(B_Len--)
	    {
	        //*PB_Dest++ = gm_DpDpcd_ReadRegByte(W_CurReg++);
	        *PB_Dest = Lpm_AuxRxBusReadByte(W_CurReg, B_AuxCh);
	        PB_Dest++;
	        W_CurReg++;
	    }
	}   
#else
	if(B_AuxCh == AUX_CH_C0)
	{
	    while(B_Len--)
	    {
	        *PB_Dest++ = gm_DpDpcd_ReadRegByte(W_CurReg++);
	    }
	}
	#ifdef AUX_RX_C2_SUPPORT
	else if(B_AuxCh == AUX_CH_C2)
	{
	    while(B_Len--)
	    {
	        *PB_Dest++ = gm_Dp2Dpcd_ReadRegByte(W_CurReg++);
	    }	
	}
	#endif
#endif

    return TRUE;
}

//***************************************************************************** 
// FUNCTION     : void Lpm_AuxRxSetReplyData(BYTE* PB_Mem, BYTE B_Len) 
// USAGE        : Set AUX2OCM reply data
// INPUT        : PB_Mem - RAM buffer for data replying
//                   B_Len - data length to be write
//                   B_AuxCh - indicate which Aux channel
// OUTPUT       : 
// GLOBALS      : 
// USED_REGS    : LPM_Cx_AUX2OCM_RPLY_LEN
//***************************************************************************** 
void Lpm_AuxRxSetReplyData(BYTE* PB_Mem, BYTE B_Len, AUX_CH B_AuxCh)
{
    WORD W_CurReg;
    W_CurReg = AUX_REPLY_START_DPCD;

    gm_WriteRegByte(AUXREG(LPM_C0_AUX2OCM_RPLY_LEN,B_AuxCh), B_Len);
	
#if 1
   gm_WriteRegByte(LPM_C0_AUX2OCM_RPLY_LEN, B_Len);
   
   while(B_Len--)
   {
      Lpm_AuxRxBusWriteByte(W_CurReg, *PB_Mem, B_AuxCh);
      PB_Mem++;
      W_CurReg++;
   }     
#else
	if(B_AuxCh == AUX_CH_C0)
	{
    	while(B_Len--)
	    {
	        gm_DpDpcd_WriteRegByte(W_CurReg++, *PB_Mem++);
	    }		
	}
	#ifdef AUX_RX_C2_SUPPORT
	else if(B_AuxCh == AUX_CH_C2)
	{
	    while(B_Len--)
	    {
	        gm_Dp2Dpcd_WriteRegByte(W_CurReg++, *PB_Mem++);
	    }	
	}
	#endif
#endif
}

//***************************************************************************** 
// FUNCTION     : void Lpm_AuxRxSetReplyCmd 
// USAGE        : Set AUX2OCM reply command
// INPUT        : B_RepCmd - reply command
//		  B_AuxCh - indicate which Aux channel
// OUTPUT       : 
// GLOBALS      : 
// USED_REGS    : LPM_Cx_AUX2OCM_RPLY_COMM
//***************************************************************************** 
void Lpm_AuxRxSetReplyCmd(BYTE B_RepCmd, AUX_CH B_AuxCh)
{
    gm_WriteRegByte(AUXREG(LPM_C0_AUX2OCM_RPLY_COMM,B_AuxCh), B_RepCmd);
}

//******************************************************************************
// FUNCTION: void Lpm_AuxRxBusReadByte()
// USAGE: Read a byte by OCM-to-AUX bus
// INPUT: DWORD DW_Addr -- 20-bits address
// OUTPUT: None
// GLOBALS: None
// USED_REGS: LPM_Cx_OCM2AUX_BUS_ADDR, LPM_Cx_OCM2AUX_RD_DATA, LPM_Cx_AUX_STROBE
//******************************************************************************
BYTE Lpm_AuxRxBusReadByte(DWORD DW_Addr, AUX_CH B_AuxCh)
{
    //gm_DpRx_WriteRegTriBytes(DPRX_OCM2AUX_BUS_ADDR, DW_Addr);
    gm_WriteRegDWord(AUXREG(LPM_C0_OCM2AUX_BUS_ADDR,B_AuxCh), DW_Addr);
    gm_SetRegBitsWord(AUXREG(LPM_C0_AUX_STROBE,B_AuxCh), DPRX_AUX_BUS_RD_STB);
    
    while(gm_ReadRegWord(AUXREG(LPM_C0_AUX_STROBE,B_AuxCh)) & DPRX_AUX_BUS_RD_STB) ;
            
    return gm_ReadRegByte(AUXREG(LPM_C0_OCM2AUX_BUS_RD_DATA,B_AuxCh));
}

//******************************************************************************
// FUNCTION: void Lpm_AuxRxBusWriteByte()
// USAGE: Write a byte by OCM-to-AUX bus
// INPUT: DWORD DW_Addr -- 20-bits address
//        BYTE B_Data -- data byte
// OUTPUT: None
// GLOBALS: None
// USED_REGS: LPM_Cx_OCM2AUX_BUS_ADDR, LPM_Cx_OCM2AUX_WR_DATA, LPM_Cx_AUX_STROBE
//******************************************************************************
void Lpm_AuxRxBusWriteByte(DWORD DW_Addr, BYTE B_Data, AUX_CH B_AuxCh)
{
    while(gm_ReadRegWord(AUXREG(LPM_C0_AUX_STROBE,B_AuxCh)) & DPRX_AUX_BUS_WR_STB) ; // wait for previous byte sending
    
    //gm_DpRx_WriteRegTriBytes(DPRX_OCM2AUX_BUS_ADDR, DW_Addr);
    gm_WriteRegDWord(AUXREG(LPM_C0_OCM2AUX_BUS_ADDR,B_AuxCh), DW_Addr);
    gm_WriteRegByte(AUXREG(LPM_C0_OCM2AUX_BUS_WR_DATA,B_AuxCh), B_Data);
    gm_SetRegBitsWord(AUXREG(LPM_C0_AUX_STROBE,B_AuxCh), DPRX_AUX_BUS_WR_STB);
}

//******************************************************************************
// FUNCTION:   void Lpm_AuxRxPhyInit(void)
// USAGE:      Initialize DpRx Aux PHY
// INPUT:      None
// OUTPUT:     None
// USED_REGS:  None
//******************************************************************************
void Lpm_AuxRxPhyInit(void)
{
	//Should Aux Phy be initialized in LPM??
	/*
    gm_WriteRegByte(DPRX_AUX_GAIN, 0x40);
    gm_WriteRegByte(DPRX_AUX_PREEMPH, 0x00);

    gm_WriteRegByte(DPRX_AUX_PHY_CTRL, 0x60);
    gm_WriteRegByte(DPRX_AUX_PHY_CTRL, 0x70);
    gm_WriteRegByte(DPRX_AUX_PHY_CTRL, 0x60);
    */
}

//***************************************************************************** 
// FUNCTION     : Lpm_AuxRxInit() 
// USAGE        : Initialize AuxRx clock, memory and set the callback functions
// INPUT        : none 
// OUTPUT       : gmd_OK if initialization OK 
// GLOBALS      : 
// USED_REGS    : LPM_Cx_AUX_IRQ_CTRL, LPM_CLOCK_CTRL, LPM_COMBOx_CONTROL, LPM_Cx_AUX_CONTROL, LPM_Cx_AUX_STROBE
//***************************************************************************** 
gmt_RET_STAT Lpm_AuxRxInit(AUX_CH B_AuxCh)
{

   if(B_AuxCh == AUX_CH_C0)
   {
   //ONLY AUX_AX_C0
   Lpm_AuxRxI2CSwTransInit((BYTE *)(&SW_Transaction_ModuleData), AUX_CH_C0);
   Lpm_AuxRxI2CAddSwDevId(DDC2BI_ADDR_ID, AUX_CH_C0);
   //Lpm_AuxRxLLI2CInit((BYTE *)(&AuxRxI2CModuleData), gm_AuxRxI2CSwCallback, gm_AuxRxEdidHandler, AUX_CH_C0);
   Lpm_AuxRxLLI2CInit((BYTE *)(&AuxRxI2CModuleData), NULL_PTR, NULL_PTR, AUX_CH_C0);
   Lpm_AuxRxI2CInit((BYTE *)(&AuxRxI2CInternalData), NULL_PTR, NULL_PTR, NULL_PTR, AUX_CH_C0);

   Lpm_AuxRxEdidHandlerInit(AUX_CH_C0);

   gm_WriteRegDWord(LPM_C0_AUX_IRQ_STATUS, 0x1FFFFUL);  //clear all Aux IRQ status
   gm_WriteRegWord(LPM_C0_AUX_IRQ_CTRL, DPRX_AUX2OCM_REQ_IRQ_EN|DPRX_AUX2I2C_REQ_IRQ_EN);

   //set AUX CLK to 2.077MHz
   //gm_SetRegBitsByte(LPM_C0_AUX_OPT_CTRL, BIT7);
   gm_SetRegBitsWord(LPM_C0_AUX_CONTROL, DPRX_AUX_CLK_EN);
   gm_ClearRegBitsWord(LPM_C0_AUX_CONTROL, DPRX_AUX_POWER_DOWN);  
   if((LpmRcOscClk > 25000000L) && (LpmRcOscClk <= SYSTEM_TCLK_FREQ))
      gm_WriteRegByte(LPM_C0_AUX_CLK, 0x65);//AUX_CLK(~2MHz)=LPM_CLK/(5+1+6+1)
   else if((LpmRcOscClk > SYSTEM_TCLK_FREQ) && (LpmRcOscClk <= 29000000L))
      gm_WriteRegByte(LPM_C0_AUX_CLK, 0x66);//AUX_CLK(~2MHz)=LPM_CLK/(6+1+6+1)
   else if(LpmRcOscClk > 29000000L)
      gm_WriteRegByte(LPM_C0_AUX_CLK, 0x76);//AUX_CLK(~2MHz)=LPM_CLK/(6+1+7+1)
   else //if(LpmRcOscClk <= 25000000L)
      gm_WriteRegByte(LPM_C0_AUX_CLK, 0x55);//AUX_CLK(~2MHz)=LPM_CLK/(5+1+5+1)

   Lpm_AuxRxPhyInit();

   gm_WriteRegByte(LPM_C0_AUX_CONTROL, DPRX_DEFER_TIMEOUT|DPRX_AUX_LONG_PREAMBL);  //set 300us DEFER timeout & 30 zeros preamble
   gm_WriteRegWord(LPM_C0_AUX_STROBE, DPRX_AUX2OCM_TO_EN|DPRX_AUX_AUTO_WAKEUP_EN);

   //gm_SetRegBitsWord(LPM_AUTOWAKE_CTRL, AUTOWAKE_CH_CPHY0<<LPM_AUTOWAKE_AUX_TRG_SHIFT);
   //gm_SetRegBitsWord(LPM_AUTOWAKE_CTRL, AUTOWAKE_CH_CPHY0<<AUTOWAKE_AUX2OCM_TRG_SHIFT);

   #if (LPM_EDID_ACCESS_SEL != 0)
   gm_WriteRegWord(LPM_C0_AUX2I2C_CLK_SCALE, (WORD)(LpmRcOscClk/I2C_100K_FREQ/6));
   // BROADCAST is set. All addresses, except those programmed in the DPRX_AUX_ADDRx 
   // registers will be sent out over AUX_I2C lines. 
   gm_WriteRegWord(LPM_C0_AUX2I2C_CTRL, DPRX_AUX2I2C_EN|DPRX_AUX2I2C_BROADCAST
                                       |DPRX_AUX2I2C_TO_EN|DPRX_AUX2I2C_REPLY_TO); 
   #endif
   Lpm_InterruptEnable(gmd_AUTOWAKE);
   }

   #ifdef AUX_RX_C2_SUPPORT 
   if(B_AuxCh == AUX_CH_C2)
   {   
   Lpm_AuxRxI2CSwTransInit((BYTE *)(&SW_Transaction_ModuleData2), AUX_CH_C2);		
   Lpm_AuxRxI2CAddSwDevId(DDC2BI_ADDR_ID, AUX_CH_C2);	
   //Lpm_AuxRxLLI2CInit((BYTE *)(&AuxRxI2CModuleData), gm_AuxRxI2CSwCallback, gm_AuxRxEdidHandler, AUX_CH_C0);
   Lpm_AuxRxLLI2CInit((BYTE *)(&AuxRxI2CModuleData2), NULL_PTR, NULL_PTR, AUX_CH_C2);	
   Lpm_AuxRxI2CInit((BYTE *)(&AuxRxI2CInternalData2), NULL_PTR, NULL_PTR, NULL_PTR, AUX_CH_C2);
   
   Lpm_AuxRxEdidHandlerInit(AUX_CH_C2);

   gm_WriteRegDWord(LPM_C2_AUX_IRQ_STATUS, 0x1FFFFUL);  //clear all Aux IRQ status
   gm_WriteRegWord(LPM_C2_AUX_IRQ_CTRL, DPRX_AUX2OCM_REQ_IRQ_EN|DPRX_AUX2I2C_REQ_IRQ_EN);

   //set AUX CLK to 2.077MHz
   //gm_SetRegBitsByte(LPM_C2_AUX_OPT_CTRL, BIT7);
   gm_SetRegBitsWord(LPM_C2_AUX_CONTROL, DPRX_AUX_CLK_EN);
   gm_ClearRegBitsWord(LPM_C2_AUX_CONTROL, DPRX_AUX_POWER_DOWN);    
   if((LpmRcOscClk > 25000000L) && (LpmRcOscClk <= SYSTEM_TCLK_FREQ))
      gm_WriteRegByte(LPM_C2_AUX_CLK, 0x65);//AUX_CLK(~2MHz)=LPM_CLK/(5+1+6+1)
   else if((LpmRcOscClk > SYSTEM_TCLK_FREQ) && (LpmRcOscClk <= 29000000L))
      gm_WriteRegByte(LPM_C2_AUX_CLK, 0x66);//AUX_CLK(~2MHz)=LPM_CLK/(6+1+6+1)
   else if(LpmRcOscClk > 29000000L)
      gm_WriteRegByte(LPM_C2_AUX_CLK, 0x76);//AUX_CLK(~2MHz)=LPM_CLK/(6+1+7+1)
   else //if(LpmRcOscClk <= 25000000L)
      gm_WriteRegByte(LPM_C2_AUX_CLK, 0x55);//AUX_CLK(~2MHz)=LPM_CLK/(5+1+5+1)

   Lpm_AuxRxPhyInit();

   gm_WriteRegByte(LPM_C2_AUX_CONTROL, DPRX_DEFER_TIMEOUT|DPRX_AUX_LONG_PREAMBL);  //set 300us DEFER timeout & 30 zeros preamble
   gm_WriteRegWord(LPM_C2_AUX_STROBE, DPRX_AUX2OCM_TO_EN|DPRX_AUX_AUTO_WAKEUP_EN);

   //gm_SetRegBitsWord(LPM_AUTOWAKE_CTRL, AUTOWAKE_CH_CPHY2<<LPM_AUTOWAKE_AUX_TRG_SHIFT);
   //gm_SetRegBitsWord(LPM_AUTOWAKE_CTRL, AUTOWAKE_CH_CPHY2<<AUTOWAKE_AUX2OCM_TRG_SHIFT);

   #if (LPM_EDID_ACCESS_SEL != 0)
   gm_WriteRegWord(LPM_C2_AUX2I2C_CLK_SCALE, (WORD)(LpmRcOscClk/I2C_100K_FREQ/6));
   // BROADCAST is set. All addresses, except those programmed in the DPRX_AUX_ADDRx 
   // registers will be sent out over AUX_I2C lines. 
   gm_WriteRegWord(LPM_C2_AUX2I2C_CTRL, DPRX_AUX2I2C_EN|DPRX_AUX2I2C_BROADCAST
                                       |DPRX_AUX2I2C_TO_EN|DPRX_AUX2I2C_REPLY_TO); 
   #endif   
   
   Lpm_InterruptEnable(gmd_AUTOWAKE);
   }
   #endif // AUX_RX_C2_SUPPORT

   return OK;
}

#endif	//AUX_RX_SUPPORT
//******************************************************************************

