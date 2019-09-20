#ifndef __LPM_AUXRX_H__
#define __LPM_AUXRX_H__

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
// MODULE:      Lpm_AuxRx.h
//
// USAGE:       
//
//******************************************************************************

#define AUX_RX_C2_SUPPORT


//****************************************************************************** 
//  G L O B A L    D E F I N I T I O N S 
//******************************************************************************
//#define DPRX_AUX_DPCD_PHYSICAL_BASE    0x11800ul
//#define DPRX_AUX_DPCD_RANGE_MIN        0x0000ul
//#define DPRX_AUX_DPCD_RANGE_MAX        0xfffful //as while Spec is not finished //0x0420ul 

//#define DPRX_AUX_IS_CORRECT_DPCD_ADDR(AuxAddr)  ((AuxAddr) >= DPRX_AUX_DPCD_RANGE_MIN && (AuxAddr) <= DPRX_AUX_DPCD_RANGE_MAX)
//#define DPRX_AUX_DPCD_ADDRESS2PHYSICAL(AuxAddr) (DPRX_AUX_DPCD_PHYSICAL_BASE + (AuxAddr))

//#define AUXRX_MAX_TIMEOUT_VALUE     0xA8 

#define AUX_MAX_BYTES_IN_CHUNK   16

//Reply 
#define AUX_REPLY_CMD_MASK        0x0F

#define AUX_REPLY_CMD_AUX_ACK     0x00
#define AUX_REPLY_CMD_AUX_NACK    0x01
#define AUX_REPLY_CMD_AUX_DEFER   0x02
#define AUX_REPLY_CMD_I2C_NACK    0x04
#define AUX_REPLY_CMD_I2C_DEFER   0x08

#define AUX_REPLY_LENGTH_MASK     0x1F

//Request 
#define AUX_REQUEST_I2C_WRITE     0
#define AUX_REQUEST_I2C_READ      1
#define AUX_REQUEST_I2C_STATUS    2
#define AUX_REQUEST_MOT           4

#define AUX_REQUEST_NATIVE_WRITE  0
#define AUX_REQUEST_NATIVE_READ   1
#define AUX_REQUEST_NATIVE_BIT    8

#define AUX_REQUEST_DIRECTION_MASK  1
#define AUX_REQUEST_LENGTH_MASK     0x0F
#define AUX_REQUEST_COMMAND_MASK    3

#define LPM_C0_AUX_OPT_CTRL			(LPM_C0_AUX_CONTROL + 1)
#define LPM_C0_AUX_CLK				(LPM_C0_AUX_CONTROL + 2)
#define LPM_C0_AUX_TBUS_SEL			(LPM_C0_AUX_CONTROL + 3)
#define LPM_C2_AUX_OPT_CTRL			(LPM_C2_AUX_CONTROL + 1)
#define LPM_C2_AUX_CLK				(LPM_C2_AUX_CONTROL + 2)
#define LPM_C2_AUX_TBUS_SEL			(LPM_C2_AUX_CONTROL + 3)


#define LPM_C0_AUX2OCM_REQ_LEN		(LPM_C0_AUX2OCM_REQ + 1)
#define LPM_C0_AUX2OCM_RPLY_LEN		(LPM_C0_AUX2OCM_RPLY_COMM + 1)
#define LPM_C2_AUX2OCM_REQ_LEN		(LPM_C2_AUX2OCM_REQ + 1)
#define LPM_C2_AUX2OCM_RPLY_LEN		(LPM_C2_AUX2OCM_RPLY_COMM + 1)


//RX AUX Req/Rep 
#define DPRX_DPCD_BASE_ADDR   	 0x1F000UL
#define DPRX2_DPCD_BASE_ADDR   	 0x1B000UL
#define DPRX_DPCD_BASE_OFFSET	 (DPRX_DPCD_BASE_ADDR - DPRX2_DPCD_BASE_ADDR)
#define AUX_REQUEST_START_DPCD   0x7E0
#define AUX_REPLY_START_DPCD     0x7F0

#define LPM_AUXCH_REG_OFFSET		(LPM_C2_AUX_CONTROL - LPM_C0_AUX_CONTROL)
#define AUXREG(AuxReg,AuxCh)	(AuxReg + (AuxCh * LPM_AUXCH_REG_OFFSET))


//For TCLK=27MHz
#define AUXCLK_SEL_TCLK_DIV12			(0)
#define AUXCLK_SEL_TCLK_DIV13			(BIT7)
#define AUXCLK_SEL_TCLK_DIV25			(BIT8)
#define AUXCLK_SEL_TCLK_DIV26			(BIT8|BIT7)


#define MAX_DDC2BI_MSG_SIZE           132
#define MAX_I2C_SW_ADDRESSES          16

#define EXEC_MEM_DEBUG                106
#define NULL_MSG_LENGTH         	  3

#define AUXTX_I2C_SW_MARKER   0x0080
#define AUXTX_I2C_ADDR_MASK   0x007f
#define AUXTX_I2C_HW_ADDR(A)   (AUXTX_I2C_ADDR_MASK & (WORD)(A))
#define AUXTX_I2C_SW_ADDR(A)   ((AUXTX_I2C_ADDR_MASK & (WORD)(A)) | AUXTX_I2C_SW_MARKER)
#define AUX_I2C_IS_SW_TR(A)   ((A) & AUXTX_I2C_SW_MARKER)

#define AUTOWAKE_AUX2OCM_TRG_SHIFT     13

#if 0
//__ADDR24__
#define ADDR_TO_DWORD(addr) ((DWORD)\
        ((void _seg *)(((WORD)((addr) >> 16)) << 8) + (void __near *)((WORD)(addr))))
#else        
//__ADDR20__
#define ADDR_TO_DWORD(addr) ((DWORD)\
        ((void _seg *)(((WORD)((addr) >> 16)) << 12) + (void __near *)((WORD)(addr))))
#endif

#define gm_DpDpcd_WriteRegByte(W_RegAddr, B_RegValue) \
    gm_WriteRegByteFar(ADDR_TO_DWORD(DPRX_DPCD_BASE_ADDR), W_RegAddr, B_RegValue)

#define gm_DpDpcd_ReadRegByte(W_RegAddr) \
    gm_ReadRegByteFar(ADDR_TO_DWORD(DPRX_DPCD_BASE_ADDR), W_RegAddr)

#define gm_Dp2Dpcd_WriteRegByte(W_RegAddr, B_RegValue) \
    gm_WriteRegByteFar(ADDR_TO_DWORD(DPRX2_DPCD_BASE_ADDR), W_RegAddr, B_RegValue)

#define gm_Dp2Dpcd_ReadRegByte(W_RegAddr) \
    gm_ReadRegByteFar(ADDR_TO_DWORD(DPRX2_DPCD_BASE_ADDR), W_RegAddr)


typedef enum
{
   AUX_CH_C0 = 0,
   AUX_CH_C2,  
   AUX_CH_END,
}AUX_CH;

typedef enum
{
   DPRX_C0 = 0,
   DPRX_C2,  
   DPRX_END,
   DPRX_NUM = DPRX_END,
}DPRX_SEL;


//****************************************************************************** 
//  G L O B A L   V A R I A B L E S 
//****************************************************************************** 


//****************************************************************************** 
//  G L O B A L   F U N T I O N S 
//****************************************************************************** 
void Lpm_AuxRxRequestISR(AUX_CH B_AuxCh);
BOOL Lpm_AuxRxGetChunkData(BYTE* PB_Dest, BYTE B_Len, AUX_CH B_AuxCh);
void Lpm_AuxRxSetReplyData(BYTE* PB_Mem, BYTE B_Len, AUX_CH B_AuxCh);
void Lpm_AuxRxSetReplyCmd(BYTE B_RepCmd, AUX_CH B_AuxCh);
BYTE Lpm_AuxRxBusReadByte(DWORD DW_Addr, AUX_CH B_AuxCh);
void Lpm_AuxRxBusWriteByte(DWORD DW_Addr, BYTE B_Data, AUX_CH B_AuxCh);
void Lpm_AuxRxPhyInit(void);
gmt_RET_STAT Lpm_AuxRxInit(AUX_CH B_AuxCh);


#endif //__LPM_AUXRX_H__
