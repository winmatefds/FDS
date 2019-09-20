#ifndef __LPM_IR_H__
#define __LPM_IR_H__

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
// MODULE:      Lpm_Ir.h
//
// USAGE:       
//
//******************************************************************************

#ifdef IR_SUPPORT

#define IR_IRQ_DISABLE		0
#define IR_IRQ_ENABLE		1
#define IR_CHANNEL1			0
#define IR_CFG_RAW			0xC
#define IR_INVALID			0xffff
#define IR_POLARITY_MASK	0x80
#define IR_POLARITY_HIGH	0x80
#define IR_POLARITY_LOW		0x00
#define IR_BIT_WIDTH_MASK	0x7F
#define IR_MAX_PACKET_TYPES	 3

enum IrProtocols
{
	IR_PROTOCOL_RECS80 	= 0,
	IR_PROTOCOL_RC5,
};

enum IrProtocolFlags
{
	IR_FLAG_NONE 		= 0,	// None
	IR_FLAG_REPEAT		= BIT1,	// Repeat Key Packet
	IR_FLAG_MULT_START= BIT2,   // Packet contains multiple start pulses
	IR_FLAG_POWER		= BIT3,	// Power Key Packet
	IR_FLAG_REVERSE		= BIT4,	// Reverse Bit order MSB...LSB
};

// Datatype definitions
enum IrStates
{
   IR_STATE_START0 = 0,
   IR_STATE_START1,
   IR_STATE_START2,
   IR_STATE_START3,
   IR_STATE_TROUGH,
   IR_STATE_PAYLOAD,
   IR_STATE_TERM
};

// Packet Definition
typedef struct {
	BYTE			B_Flags;
	BYTE			B_Start0Min;
	BYTE			B_Start0Max;
	BYTE			B_Start1Min;
	BYTE			B_Start1Max;
	BYTE			B_TroughMin;		// RECS80			RC5
	BYTE			B_TroughMax;		// ------			---------
	BYTE			B_Data0Min;			// Logic0Min		SingleMin
	BYTE			B_Data0Max;			// Logic0Max		SingleMax
	BYTE			B_Data1Min;			// Logic1Min		DoubleMin
	BYTE			B_Data1Max;			// Logic1Max		DoubleMax
	BYTE			B_PayloadCount;
	DWORD 			D_AddrMask;
	BYTE			B_AddrShift;
	DWORD 			D_DataMask;
	BYTE			B_DataShift;
	WORD			W_CmdID;
} gmt_IrPacket;

// Protocol Definition
#pragma pack(push)
#pragma pack(1)
typedef struct {
	BYTE			B_Protocol;
	WORD			W_Config;
	WORD			W_Timing;
	BYTE			B_Period;
	BYTE			B_NumPackets;
	gmt_IrPacket	B_Packets[IR_MAX_PACKET_TYPES];	
} gmt_IrProtocol;
#pragma pack(pop)

typedef gmt_IrProtocol gmt_IrProtocolExt;

typedef struct {
	BYTE			B_State;
	BYTE			B_StateCnt;
	BYTE			B_SpecialKey;
	WORD			W_Address;
	DWORD			D_Payload;
} gmt_IrPacketState;

#define MINIMAL_ARRAY_SIZE 1
typedef struct {
	gmt_IrProtocol const far* P_IrProtocol;
	void (far* P_IrCallBack)(void);
	DWORD expTime;
	gmt_RET_STAT valid;
	WORD	cmd;
	WORD	key;
	BYTE index;
	gmt_IrPacketState packetStates[IR_MAX_PACKET_TYPES];
} gmt_Ir;

void Lpm_IrHandler(void);
void Lpm_IrIsr(void);
void Lpm_IrInit(void *p_Ir);
#endif
#endif // __LP_IR_H__

