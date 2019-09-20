#ifndef __LPM_DEBUG_H__
#define __LPM_DEBUG_H__

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
// MODULE:      Lpm_Debug.h
//
// USAGE:       
//
//******************************************************************************

extern const BYTE AckPkt[];
extern const BYTE NakPkt[];

extern BYTE DebugOnly;


#define VAR_REGISTER_RW_SUPPORT	1  //Enabled to support GProbe 7

#define CMD_NAK                0x0B
#define CMD_ACK                0x0C

#define SIO_RxBufLen      256

#define Start_of_GprobeCmd_in_DDC2BiMsg   5   // 3 (device id + length + cmd) + 2 (manufacturer id) 


#define UNUSED_VAR(x)      ((x) = (x))


typedef enum
{
	CMD_gmRead				= 0x06, // read  host reg
	CMD_gmWrite				= 0x07, // write host reg
	//CMD_GetVersion		= 0x08, // get firmware version
	CMD_TurnOffSwitcher		= 0x09, // set firmware in DEBUG  mode
	CMD_TurnOnSwitcher		= 0x0A, // set firmware in NORMAL mode
	CMD_PRINTIT				= 0x0D, // print command
	//CMD_I2C_Read			= 0x0E, // read I2C
	//CMD_I2C_Write			= 0x0F, // write I2C
	CMD_SetParam			= 0x11, // custom command
	CMD_AppsTest			= 0x12, // custom command
	CMD_BurstRAMWr			= 0x13, // burst write RAM buffer
	CMD_BurstNextRAMWr		= 0x14, // burst write RAM buffer from last address
	CMD_WrNvramBlock		= 0x15, // write NVRAM from RAM buffer
	CMD_WrNvram			= 0x16, // write NVRAM from data packet
	CMD_RdNvram			= 0x17, // read back NVRAM
	//CMD_BurstRegWr		= 0x18, // burst reg write
	//CMD_FlashErase		= 0x19, // erase FLASH sector
	//CMD_FlashWrite		= 0x1A, // write FLASH from RAM buffer
	//CMD_FlashRead			= 0x1B, // read  FLASH
	//CMD_FlashCRC			= 0x1C, // get FLASH CRC
	CMD_RamExecute			= 0x1D, // execute code from RAM
	CMD_Reset				= 0x20, // force execution in internal rom
	CMD_GetInfo				= 0x40, // get info record - get versiom type supported 
	CMD_VarLenRegRead       = 0x46,//variable length register (memory) read
	CMD_VarLenRegWrite    	= 0x47,//variable length register (memory) write	
	CMD_gmRead2				= 0x50, // read  host reg extended (DWORD address)
	CMD_gmWrite2			= 0x51, // write host reg extended (DWORD address)
	CMD_RAMRead2			= 0x52, // burst read RAM buffer (DWORD address)
	CMD_RAMWrite2			= 0x53, // burst write RAM buffer (DWORD address)
	CMD_RamExecute2			= 0x54, // execute code from RAM (DWORD address)
} gmt_GProbeCommandCode;


typedef enum SendAckNackDataStruct
{
    SND_NAK = 0,
    SND_ACK,
    SND_DATA,
    SND_NOTHING
}SendAckNackData_t;

// command packet to read gm register 
typedef struct ReadBrCmdStruct
{
    BYTE length; // length of packet 
    BYTE cmd;
    BYTE addr_hi;
    BYTE addr_lo;
    BYTE chk;
}ReadBrCmd_t;

typedef struct // command packet to extended read gm register
{
    BYTE length; // length of packet
    BYTE cmd;
    BYTE addr_3;
    BYTE addr_2;
    BYTE addr_1;
    BYTE addr_0;
}ReadBr2Cmd_t;

// response packet for read gm command 
typedef struct RdBrCmdPktStruct
{
    BYTE   length;                        // length of this response packet 
    BYTE   cmd;                        // response packet type 
    BYTE   addr_hi;                     // address of register read 
    BYTE   addr_lo;
    BYTE   value_hi;                     // value read from register 
    BYTE   value_lo;
    BYTE   chk;                        // checksum of packet 
}RdBrCmdPkt_t;

typedef struct // response packet for read gm command
{
    BYTE    length;                             // length of this response packet
    BYTE    cmd;                                // response packet type
    BYTE    addr_3;                            // address of register read
    BYTE    addr_2;
    BYTE    addr_1;                            // address of register read
    BYTE    addr_0;
    BYTE    value_3;                           // value read from register
    BYTE    value_2;
    BYTE    value_1;                           // value read from register
    BYTE    value_0;
}RdBr2CmdPkt_t;

// command packet to write gm register 
typedef struct WriteBrCmdStruct
{
    BYTE   length; // length of packet 
    BYTE   cmd;
    BYTE   addr_hi;
    BYTE   addr_lo;
    BYTE   value_hi;
    BYTE   value_lo;
    BYTE   chk;
}WriteBrCmd_t;

typedef struct // command packet to write gm register
{
   BYTE    length; // length of packet
   BYTE    cmd;
   BYTE    addr_3;       
    BYTE    addr_2;
    BYTE    addr_1;       
    BYTE    addr_0;
    BYTE    value_3;      
    BYTE    value_2;
    BYTE    value_1;      
    BYTE    value_0;
} WriteBr2Cmd_t;

// command packet for burst write to RAM 
typedef struct BurstWriteRAM
{
    BYTE    length; // length of packet 
    BYTE   cmd;
    BYTE   addr_hi;
    BYTE   addr_lo;
    BYTE   value[SIO_RxBufLen-6];
}BurstWrRAMCmd_t;

// command packet for burst write to RAM
typedef struct 
{
    BYTE    length; // length of packet
    BYTE    cmd;
    BYTE    addr_3;
    BYTE    addr_2;
    BYTE    addr_1;
    BYTE    addr_0;
    BYTE    value[SIO_RxBufLen-7];
}BurstWrRAM2Cmd_t;

// command packet for burst write to RAM from last address 
typedef struct BurstWriteNextRAM
{
    BYTE    length; // length of packet 
    BYTE   cmd;
    BYTE   value[SIO_RxBufLen-4];
}BurstWrNextRAMCmd_t;

typedef BYTE debugCmd_t;

typedef enum HANDLER_USE
{
	SERIAL_USE      = 0,
	I2C_SLAVE1_USE  = 1,	
	I2C_SLAVE2_USE  = 2,
	I2C_SLAVE3_USE 	= 3,
	I2C_SLAVE4_USE 	= 4,	
	AUX_C0_USE      = 5,
	AUX_C2_USE      = 6,	
}HANDLER_USE_t;

typedef enum MODE_STATUS
{
   EXTERNAL_ROM_MODE = 0,
   INTERNAL_ROM_MODE,
   ISP_RAM_MODE,
}MODE_STATUS_t;


// commands delegated  to the external rom code handle 
typedef enum PARSER_STATUS
{
    NO_MSG_PARSED,
    MSG_PARSED,
    UNKNOWN_MSG_CMD
}PARSER_STATUS_t;

#if VAR_REGISTER_RW_SUPPORT
//--- Variable length registers' access 
// read I2C command structure 
typedef struct
{
    BYTE   MessageLength;
    BYTE   Command;
    BYTE  AddressLength;
    BYTE    Regs[SIO_RxBufLen - 5];
}TVarLenReg_Read;

typedef struct
{
    BYTE   MessageLength;
    BYTE   Command;
    BYTE  AddressLength;
    BYTE    Regs[SIO_RxBufLen - 5];
}TVarLenReg_Write;
#endif

typedef union 
{
    BYTE far* pt;
    DWORD dw;
    struct {
        WORD offs;
        WORD segm;                        
        } w;
} TFarAddr;

typedef struct
{
    BYTE    length;                             // Packet length
    BYTE    cmd;                                // Command
    BYTE    addr_3;
    BYTE    addr_2;
    BYTE    addr_1;
    BYTE    addr_0;
    BYTE    count_3;
    BYTE    count_2;
    BYTE    count_1;
    BYTE    count_0;
} RamReadCmd_t;

typedef struct 
{
    BYTE    length;                             // Packet length
    BYTE    cmd;                                // Command
    BYTE    value[SIO_RxBufLen-3];
} RamReadResp_t;

//Gets the byte offset to a structure member.
//The argument struct_type is a struct type. struct_member is any element of the struct 
//that can be accessed through the member selection operators or pointers.
#define offs_strct(s_name, m_name)  (unsigned)&(((s_name far *)0)->m_name)


void Lpm_SystemDebug(void);
PARSER_STATUS_t Lpm_GprobeCmdParser(HANDLER_USE_t B_whichHandler, debugCmd_t far* Bp_MessageBuff);
BYTE Lpm_AppstestCmdParser(HANDLER_USE_t B_whichHandler, debugCmd_t far* Bp_MessageBuff);
BYTE IsSerialMsgCompleted(void);
void Lpm_SendAckOrNack(BYTE B_Ack, BYTE B_whichHandler, debugCmd_t far* Bp_MessageBuff);
void Lpm_WriteCmdPkt(BYTE far *Bp_Buf,  HANDLER_USE_t B_whichHandler);
void Lpm_UartWriteCmdPkt(BYTE far *Bp_Buf);
void gm_Print (const char far *Bp_Str, WORD W_Value);

#endif //__LPM_DEBUG_H__

