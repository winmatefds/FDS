#ifndef __LPM_DDCCI_H__
#define __LPM_DDCCI_H__

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
// MODULE:      Lpm_Ddcci.h
//
// USAGE:       
//
//******************************************************************************
extern BYTE ExecuteMemTest;
extern BYTE ExecuteMemTest2;

#define DDC2BI_ADDR_ID        			0x6E
#define DDC2B_DEST_ADDRESS              0x6E
#define DDC2B_SRC_ADDRESS               0x51
#define DDC2Bi_CONTROL_STATUS_FLAG      BIT7
#define DDC2BI_NULL_MSG_LENGTH          0x80
#define DDC2BI_MAX_MSG_LENGTH           0x7F 
#define DDC2BI_BUF_SIZE 				(DDC2BI_MAX_MSG_LENGTH+4) 

#define I2C_ADDRESS_BITS        		0xFE
#define INIT_NULL_MSG_CS        		(0x80 ^ 0x50)
#define NULL_MSG_SECOND_BYTE    		0x80


#ifdef DDCCI_SUPPORT

#define DDC2BI_BUFFER_SIZE    			132
#define NULL_MSG_SIZE         			3
#define SEND_NULL_MSG_COUNT   			2

//DDCCI commands
#define DDC2B_CMD_DEBUG						0xC2
#define DDC2B_CMD_GetVCPFeature				0x01
#define DDC2B_CMD_GetVCPFeature_Reply		0x02
#define DDC2B_CMD_SetVCPFeature				0x03
#define DDC2B_CMD_SaveCurrentSettings		0x0C
#define DDC2B_CMD_CapabilitiesRequest		0xF3
#define DDC2B_CMD_CapabilitiesRequest_Reply	0xE3
#define DDC2B_CMD_GetTimingReport			0x07
#define DDC2B_CMD_GetTimingReport_Reply		0x4E
#define DDC2B_CMD_TableRead					0xE2
#define DDC2B_CMD_TableRead_Reply			0xE4
#define DDC2B_CMD_TableWrite				0xE7

//DDCCI VCP codes
#define DDC2B_CMD_VCP_DisplayPowerMode		0xD6
#define DDC2B_CMD_VCP_MCCSVersion			0xDF    //To make MCCS 2.0 Compliant

//VCP type code
#define VCP_SETPARAMETER		0
#define VCP_MOMENTARY			1

#define VCP_SUPPORTED			0
#define VCP_UNSUPPORTED			1

#define	GET_VCP	0
#define	SET_VCP	1

#define DPM_ON					0x01
#define DPM_OFF					0x04	//Sleep, Low power mode
#define DPM_HARDOFF				0x05	//power off the display; equivalent to turning off power using power key
//DPMS are removed from MCCS v2.2 spec.
#define DPMS_ON					0x01
#define DPMS_STANDBY			0x02
#define DPMS_SUSPEND			0x03
#define DPMS_OFF				0x04	//Sleep, Low power mode
#define DPMS_HARDOFF			0x05


// used for DDC2Bi channel selection
typedef enum
{
   DDC2BI_CHANNEL0 = 0,
   DDC2BI_CHANNEL1 = 1,
   DDC2BI_CHANNEL2 = 2,
   DDC2BI_CHANNEL3 = 3,
} DDC2BI_CHANNEL;

/*typedef enum
{
    READ_PKT_OK,
    READ_PKT_NO_MSG,
    READ_PKT_WRONG_CS,
} TReadPkt;
*/
typedef enum 
{
   DDC2BI_STATE_RCV_STOP_OCCURED,
   DDC2BI_STATE_READY,
   DDC2BI_STATE_CONSECUTIVE_TRANSMISSION
}DDC2BI_STATES;

typedef struct
{
    BYTE devId;
    DDC2BI_CHANNEL channel;
    BYTE trCount;
    BYTE rcvCount;
    BYTE trLength;
    volatile DDC2BI_STATES state;
    BYTE trxTimeout; //transmission timeout in ms;
    BYTE buf[DDC2BI_BUF_SIZE]; 
    BYTE turnOff;//temporal to have ability to turn off new hardware:  BIT0=1 stop-condition hardware reset WFIFO is off, BIT1=1 = divider of timeout is off (DDC2B_ADDR_CS_TO:15) 
} TDdc2bi;

typedef struct DDC2Bi_GetVCPFeatureCmdStruct
{
	BYTE length;
	BYTE command;
	BYTE vcp_opcode;
	BYTE chk;
}DDC2Bi_GetVCPFeatureCmd_t;

typedef struct VCPFeaturePktBodyStruct
{
	BYTE vcp_opcode;
	BYTE vcp_type;
	WORD max_BigEndian;
	WORD value_BigEndian;
}VCPFeaturePktType;

#pragma pack(push)
#pragma pack(1)
typedef struct DDC2Bi_GetVCPFeature_ReplyCmdStruct
{
	BYTE				length;
	BYTE				command;
	BYTE				result;
	VCPFeaturePktType	a_VCPFeature;
	BYTE chk;
}DDC2Bi_GetVCPFeature_ReplyCmd_t;
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
typedef struct DDC2Bi_SetVCPFeatureCmdStruct
{
	BYTE length;
	BYTE command;
	BYTE vcp_opcode;
	WORD val_BigEndian;
}DDC2Bi_SetVCPFeatureCmd_t;
#pragma pack(pop)

typedef struct
{
BYTE     TxLen;
BYTE     RxLen;
BYTE     Dest;
BYTE     OutState;
BYTE     State;
BYTE     ReplyReady;
WORD     DevAddr;
WORD     W_TrxDDC2BiCount;
WORD     W_RcvDDC2BiCount;
BYTE     Buffer[DDC2BI_BUFFER_SIZE];
//WORD     W_AuxBuffAddr;                       // For Debug only 
//BYTE     AuxBuff[(DDC2BI_BUFFER_SIZE * 2)];   // For Debug only 
}DDC2BI_TYPE;

void Lpm_DdcciCmdHandler(void);
BYTE Lpm_IsDdcciTransaction(BYTE far*B_BuffPtr);

#endif

#endif //__LPM_DDCCI_H__
