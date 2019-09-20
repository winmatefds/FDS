#ifndef __LPM_I2CMASTER_H__
#define __LPM_I2CMASTER_H__

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
// MODULE:      Lpm_I2cMaster.h
//
// USAGE:       
//
//******************************************************************************

//****************************************************************************** 
//  G L O B A L    D E F I N I T I O N S 
//****************************************************************************** 
//
// Enumerators for 2Wire function set
//
typedef enum
{
   RING_ALL = 0,  // all rings are enabled, for BACKWARD COMPATIBILITY
   RING_NOCHANGES,
   RING0,
   RING1,
   RING2,
} gmt_2WIRE_RING;

typedef enum
{
   MASTER_2WIRE_1,   			// HW Master 2Wire 1
#if (FEATURE_2WIRE_NUM == 2)
   MASTER_2WIRE_2,       		// HW Master 2Wire 2
#endif
} gmt_2WIRE_MASTER_IDX;

typedef enum
{
   NO_STOP_BIT = 0,
   STOP_BIT,
} gmt_2WIRE_STOP;

typedef enum
{
	MASTER_2WIRE_DMA,
	MASTER_2WIRE_I2C,
} gmt_2WIRE_MODE;

typedef enum
{
   gmd_2WIRE_WR = 0, // Write to slave
   gmd_2WIRE_RD = 1, // Read from slave
} gmt_2WIRE_CONTROL;

typedef enum
{
   gmd_NACK_CMD = BIT0, // When a receiver, send ACK(0) or NAK(1)
   gmd_WR = BIT1, // Write to slave
   gmd_RD = BIT2, // Read from slave
   gmd_STO = BIT3, // Generate stop condition
   gmd_STA = BIT4, // Generate (repeated) start condition
   gmd_CTRL_MODE = BIT5, //
} gmt_2WIRE_COMMAND;

//
// I2C SCL FREQ (100KHz)
//
#define I2C_100K_FREQ		(100000L)
#define I2C_100K_SCALE		(((LpmOcmClk / I2C_100K_FREQ) / 5) - 2)

//****************************************************************************** 
//  G L O B A L    V A R I A B L E S 
//****************************************************************************** 


//****************************************************************************** 
//  G L O B A L    F U N C T I O N    P R O T O T Y P E S 
//****************************************************************************** 
gmt_RET_PASSFAIL  Lpm_I2CMasterStart(BYTE B_DevAddr, gmt_2WIRE_CONTROL Ctrl);
gmt_RET_PASSFAIL  Lpm_I2CMasterCheck(gmt_2WIRE_RING B_Port, BYTE B_DevAddr);
void              Lpm_I2CMasterStop(void);
gmt_RET_PASSFAIL  Lpm_I2CMasterGetByte(WORD W_Cmd, BYTE far* Bp_Data);
gmt_RET_PASSFAIL  Lpm_I2CMasterPutByte(BYTE far * Bp_Data);
gmt_RET_STAT      Lpm_I2CMasterWriteAddrBuffer(BYTE B_DevAddr, BYTE far *Bp_RegAddr, BYTE B_AddrLen, BYTE far *Bp_Data, WORD W_Len);
gmt_RET_STAT      Lpm_I2CMasterReadAddrBuffer (BYTE B_DevAddr, BYTE far *Bp_RegAddr, BYTE B_AddrLen, BYTE far *Bp_Data, WORD W_Len);
gmt_RET_PASSFAIL  Lpm_I2CMasterWriteBlock(gmt_2WIRE_RING B_Port, BYTE B_DevAddress, BYTE far *Bp_Buffer, BYTE B_Length, gmt_2WIRE_STOP B_Stop);
gmt_RET_PASSFAIL  Lpm_I2CMasterReadBlock (gmt_2WIRE_RING B_Port, BYTE B_DevAddress, BYTE far *Bp_Buffer, BYTE B_Length, gmt_2WIRE_STOP B_Stop);
gmt_RET_PASSFAIL  Lpm_I2CMasterConfig(gmt_2WIRE_MASTER_IDX B_2WireMaster, WORD W_2WireFreq, gmt_2WIRE_RING B_Port);
// NOTE: Selector function for the I2C engine (gmt_2WIRE_MASTER_IDX) meaningful only to Cortez Advanced which has 2 engines.
// For all other chips, the call is not necessary. By default, the selection is set to MASTER_2WIRE_1.
void              Lpm_I2CMasterSelectIdx (gmt_2WIRE_MASTER_IDX B_Idx);
void              Lpm_I2CMasterSetTimeout(WORD W_Timeout);
void              Lpm_I2CMasterInit(void);
void              Lpm_I2CMasterCsmSel(BYTE I2cMstSel);
void              Lpm_I2CMasterSetClock(void);

#endif //__LPM_I2CMASTER_H__