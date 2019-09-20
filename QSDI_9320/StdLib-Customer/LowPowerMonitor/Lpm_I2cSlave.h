#ifndef __LPM_I2CSALVE_H__
#define __LPM_I2CSALVE_H__

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
// MODULE:      Lpm_I2cSlave.h
//
// USAGE:       
//
//******************************************************************************

//******************************************************************************
//  G L O B A L    R E G I S T E R S
//******************************************************************************
#define I2C_SLAVE_REG_OFFSET	(I2C_SLAVE2_CTRL1 - I2C_SLAVE1_CTRL1)
#define I2C_SLAVE_CTRL1			(I2C_SLAVE1_CTRL1 		+ gB_I2cSlaveSel * I2C_SLAVE_REG_OFFSET)
#define I2C_SLAVE_CTRL2			(I2C_SLAVE1_CTRL2 		+ gB_I2cSlaveSel * I2C_SLAVE_REG_OFFSET)
#define I2C_SLAVE_STATUS		(I2C_SLAVE1_STATUS 		+ gB_I2cSlaveSel * I2C_SLAVE_REG_OFFSET)
#define I2C_SLAVE_CLOCK_STATUS	(I2C_SLAVE1_CLOCK_STATUS+ gB_I2cSlaveSel * I2C_SLAVE_REG_OFFSET)
#define I2C_SLAVE_DATA			(I2C_SLAVE1_DATA 		+ gB_I2cSlaveSel * I2C_SLAVE_REG_OFFSET)
#define I2C_SLAVE_RX_DEV_ID		(I2C_SLAVE1_RX_DEV_ID 	+ gB_I2cSlaveSel * I2C_SLAVE_REG_OFFSET)
#define I2C_SLAVE_CTRL3			(I2C_SLAVE1_CTRL3 		+ gB_I2cSlaveSel * I2C_SLAVE_REG_OFFSET)
#define I2C_SLAVE_STATUS2		(I2C_SLAVE1_STATUS2 	+ gB_I2cSlaveSel * I2C_SLAVE_REG_OFFSET)
#define I2C_SLV_ADDR_TBL0		(I2C_SLAVE1_ADDR_TBL0 	+ gB_I2cSlaveSel * I2C_SLAVE_REG_OFFSET)
#define I2C_SLV_ADDR_TBL1		(I2C_SLAVE1_ADDR_TBL1 	+ gB_I2cSlaveSel * I2C_SLAVE_REG_OFFSET)
#define I2C_SLV_ADDR_TBL2		(I2C_SLAVE1_ADDR_TBL2 	+ gB_I2cSlaveSel * I2C_SLAVE_REG_OFFSET)
#define I2C_SLV_ADDR_TBL3		(I2C_SLAVE1_ADDR_TBL3 	+ gB_I2cSlaveSel * I2C_SLAVE_REG_OFFSET)

//I2C_SLAVE_CTRL1
#define I2C_SLAVE_EN				I2C_SLAVE1_EN
#define I2C_SLAVE_RFIFO_nE_IRQ_EN	I2C_SLAVE1_RFIFO_nE_IRQ_EN
#define I2C_SLAVE_WFIFO_E_IRQ_EN	I2C_SLAVE1_WFIFO_E_IRQ_EN
#define I2C_SLAVE_DATA_ERROR_IRQ_EN	I2C_SLAVE1_DATA_ERROR_IRQ_EN
#define I2C_SLAVE_RFIFO_AF_IRQ_EN	I2C_SLAVE1_RFIFO_AF_IRQ_EN
#define I2C_SLAVE_WFIFO_AE_IRQ_EN	I2C_SLAVE1_WFIFO_AE_IRQ_EN
//I2C_SLAVE_CTRL2
#define I2C_SLAVE_DMA_WRITE_FIFO_EN	I2C_SLAVE1_DMA_WRITE_FIFO_EN
#define I2C_SLAVE_STOP_EN			I2C_SLAVE1_STOP_EN
#define I2C_SLAVE_READ_FIFO_RESET	I2C_SLAVE1_READ_FIFO_RESET
#define I2C_SLAVE_WRITE_FIFO_RESET 	I2C_SLAVE1_WRITE_FIFO_RESET
#define I2C_SLAVE_CLOCK_STRETCH_EN 	I2C_SLAVE1_CLOCK_STRETCH_EN
#define I2C_SLAVE_REF_CLK_DIV		I2C_SLAVE1_REF_CLK_DIV
#define I2C_SLAVE_REF_CLK_DIV_SHIFT	I2C_SLAVE1_REF_CLK_DIV_SHIFT
#define I2C_SLAVE_FIFO_MODE			I2C_SLAVE1_FIFO_MODE
//I2C_SLAVE_STATUS
#define I2C_SLAVE_READ_FIFO_NE 		I2C_SLAVE1_READ_FIFO_NE
#define I2C_SLAVE_WRITE_FIFO_NF		I2C_SLAVE1_WRITE_FIFO_NF
#define I2C_SLAVE_DATA_ERROR		I2C_SLAVE1_DATA_ERROR
#define I2C_SLAVE_READ_FIFO_AF 		I2C_SLAVE1_READ_FIFO_AF
#define I2C_SLAVE_READ_FIFO_F		I2C_SLAVE1_READ_FIFO_F
#define I2C_SLAVE_WRITE_FIFO_AE		I2C_SLAVE1_WRITE_FIFO_AE
#define I2C_SLAVE_WRITE_FIFO_E 		I2C_SLAVE1_WRITE_FIFO_E
#define I2C_SLAVE_STOP 				I2C_SLAVE1_STOP
//I2C_SLAVE_CLOCK_STATUS
#define I2C_SLAVE_STRETCH_OUT_STS	I2C_SLAVE1_STRETCH_OUT_STS
#define I2C_SLAVE_STRETCH_IN_STS	I2C_SLAVE1_STRETCH_IN_STS
//I2C_SLAVE_RX_DEV_ID
#define I2C_SLAVE_RX_RD_WR 			I2C_SLAVE1_RX_RD_WR
//I2C_SLAVE_CTRL3
#define I2C_SLAVE_START_IRQ_EN 		I2C_SLAVE1_START_IRQ_EN
#define I2C_SLAVE_STRETCH_IN_IRQ_EN	I2C_SLAVE1_STRETCH_IN_IRQ_EN
#define I2C_SLAVE_STRETCH_OUT_IRQ_EN I2C_SLAVE1_STRETCH_OUT_IRQ_EN
#define I2C_SLAVE_DEV_ID_IRQ_EN		I2C_SLAVE1_DEV_ID_IRQ_EN
#define I2C_SLAVE_MAKE_STRETCH 		I2C_SLAVE1_MAKE_STRETCH
#define I2C_SLAVE_RD_REQ_METHOD		I2C_SLAVE1_RD_REQ_METHOD
//I2C_SLAVE_STATUS2
#define I2C_SLAVE_START				I2C_SLAVE1_START
#define I2C_SLAVE_STRETCH_OUT		I2C_SLAVE1_STRETCH_OUT
#define I2C_SLAVE_STRETCH_IN		I2C_SLAVE1_STRETCH_IN
#define I2C_SLAVE_DEV_ID_RECEIVED	I2C_SLAVE1_DEV_ID_RECEIVED


//******************************************************************************
//  G L O B A L    D E F I N I T I O N S
//******************************************************************************

#define  I2C_ADDRESS_BITS        0xFE
#define  ALL_I2C_ADDRESSES       0xFE
#define  I2C_DHA_BUFFER_SIZE     5

#define  MAX_FUNC_NUM            10
#define  I2C_SLAVE_BUFFER_SIZE   132
#define  I2C_SLAVE_RD_LEN        1

#define I2C_SLAVE1_ADDR_TBL_SEL_SHIFT	6


//DO NOT change the sequence!!
typedef enum
{
   I2C_SLAVE1_CH,
   I2C_SLAVE2_CH,
   I2C_SLAVE3_CH,
   I2C_SLAVE4_CH,   
   I2C_SLAVE_NUM_CH
}SLAVE_CH;

typedef BYTE far (* I2C_SLAVE_HANDLER_TYPE)(SLAVE_CH B_SlaveCh);


typedef enum
{
   DDC_TO_AUX_FREE = 0,
   DDC_TO_AUX_I2C_SLAVE_1,
   DDC_TO_AUX_I2C_SLAVE_2,
   DDC_TO_AUX_I2C_SLAVE_3,
   DDC_TO_AUX_I2C_SLAVE_4,
   DDC_TO_AUX_END
}DDC_TO_AUX_STATUS;

typedef enum
{
   I2C_SLAVE_HANDLER_FREE = 0,
   I2C_SLAVE_I2C_RD_WAIT,
   I2C_SLAVE_I2C_WR_WAIT,
   I2C_SLAVE_AUX_LAST,
   I2C_SLAVE_TO_AUX_WR_ER,
   I2C_SLAVE_TO_AUX_RD_ER,
   I2C_SLAVE_AUX_WR_END
}I2C_SLAVE_HANDLER_ENUM;

typedef enum
{
   I2C_COMM_DISABLE_ADDR = 0,
   I2C_COMM_INTERNAL_DDC2Bi,
   I2C_COMM_RX_I2C_SW,
   I2C_COMM_RX_DPCD,
   I2C_COMM_RX_I2C_MASTER,
   I2C_COMM_HOST_REGISTERS,
   I2C_COMM_RX_I2C_MASTER_DDC,
   I2C_COMM_RX_DDC_HANDLER,
   I2C_COMM_RX_CLEAR_SW_ADDR_TABLE,
   I2C_COMM_DIRECT_HW_ACCESS,
   I2C_COMM_LAST
}I2C_SLAVE_COMM_ENUM;

typedef enum
{
   I2C_SLAVE_STATE_FREE = 0,
   I2C_SLAVE_STATE_STOP_RD,
   I2C_SLAVE_STATE_STOP_WR,
   I2C_SLAVE_STATE_WR_STRETCH,
   I2C_SLAVE_STATE_RD_STRETCH,
   I2C_SLAVE_STATE_RD,
   I2C_SLAVE_STATE_WR,
   I2C_SLAVE_STATE_ERR,
   I2C_SLAVE_STATE_END
}I2C_SLAVE_STATE_ENUM;

typedef struct
{
   BYTE     B_Stop;
   WORD     TxPtr;
   WORD     RxPtr;
   BYTE     State;
   BYTE     HandlerState;
   BYTE     B_DevID;
   BYTE     B_ReplyReady;
   WORD     DevAddr;
   WORD     W_TrxCount;
   WORD     W_RcvCount;
   WORD     W_AbsRcvCount;
   BYTE     Buffer[I2C_SLAVE_BUFFER_SIZE];
   BYTE     B_RdLen;
   BYTE     B_NullMsgPtr;
   BYTE     B_ActiveFlag;
   BYTE     Bp_CorrDevIDArray[MAX_FUNC_NUM];
   I2C_SLAVE_HANDLER_TYPE  Fp_CommHandlerArray[MAX_FUNC_NUM];
   BYTE     B_IsDDCPacket;
   BYTE     B_DDCSendStop;
   BYTE     B_DDCLength;
}I2C_SLAVE_TYPE;

extern I2C_SLAVE_TYPE far* Sp_I2CSlavePtrGlobal[I2C_SLAVE_NUM_CH];
//extern I2C_SLAVE_TYPE I2CSlaveTable;
//extern BYTE gB_Ddc2Bi_I2C_Status;


void far Lpm_I2CSlaveInitAll (void);
void far Lpm_I2CSlaveInit (BYTE* Bp_StartAddr, BYTE B_AuxRdLen, SLAVE_CH B_SlaveCh);
I2C_SLAVE_TYPE far* Lpm_I2CSlaveGetAddrBuffer(SLAVE_CH B_SlaveCh);
void far Lpm_I2CSlaveSetChannel(SLAVE_CH B_SlaveCh);
SLAVE_CH far Lpm_I2CSlaveGetChannel(void);
void far Lpm_I2CSlaveAddI2CAddress(BYTE B_I2CAddr, SLAVE_CH B_SlaveCh);
void far Lpm_I2CSlaveRemoveI2CAddress(BYTE B_I2CAddr, SLAVE_CH B_SlaveCh);
void far Lpm_I2CSlaveWriteToFifo(SLAVE_CH B_SlaveCh);
void far Lpm_I2CSlaveReadFromFifo_new(SLAVE_CH B_SlaveCh);
void far Lpm_I2CSlaveReadFromFifo(SLAVE_CH B_SlaveCh);
void Lpm_I2CSlaveHandler(void);
void far Lpm_I2CSlaveChannelHandler(SLAVE_CH B_SlaveCh);
void far Lpm_I2CSlaveIsr(SLAVE_CH B_SlaveCh);
void far Lpm_I2CSlaveIsrNew(SLAVE_CH B_SlaveCh);
void far Lpm_I2CSlaveSetReply(BYTE B_Count, SLAVE_CH B_SlaveCh);
void far Lpm_I2CSlaveSetExtHandler(BYTE far(* FuncPtr)(SLAVE_CH B_SlaveCh));
BYTE far Lpm_I2CSlaveExecCommHandler(SLAVE_CH B_SlaveCh);
BYTE far Lpm_I2CSlaveDefaultHandler(SLAVE_CH B_SlaveCh);
BYTE far Lpm_I2CSlaveCreateNullMsg(BYTE B_Stop, BYTE B_DevID, SLAVE_CH B_SlaveCh);
BOOL far Lpm_I2CSlaveCheckCS(BYTE B_DevID, BYTE B_Len, BYTE far*Bp_Buff);
BYTE far* Lpm_I2CSlaveGetAddrMsgBuffer(SLAVE_CH B_SlaveCh);
BOOL far Lpm_I2CSlaveAddCommHandler(I2C_SLAVE_HANDLER_TYPE Fp_NewCommHandler, BYTE B_CorrDevID, SLAVE_CH B_SlaveCh);
BYTE far Lpm_I2CSlaveGProbeHandler(SLAVE_CH B_SlaveCh);
BYTE far Lpm_I2CSlaveEdidHandler(SLAVE_CH B_SlaveCh);
BOOL IsResetWriteFifo(SLAVE_CH B_SlaveCh);


#endif //__LPM_I2CSALVE_H__
