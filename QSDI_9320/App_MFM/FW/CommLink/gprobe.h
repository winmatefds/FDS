/*
	$Workfile:   gprobe.h  $
	$Revision: 1.8 $
	$Date: 2012/01/06 03:20:53 $
*/
#define __GPROBE_H__
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
// MODULE:      gProbe.h
//
// USAGE:       Header file for gProbe debug definition
//
//******************************************************************************

//******************************************************************************
//  G L O B A L    D E F I N I T I O N S
//******************************************************************************
//
// gProbe command codes, which are not supported in IROM
//
#ifdef USER_EXTENDED_GPROBE_CMD
#define CMD_SetParam        0x11    // custom command
#define CMD_AppsTest        0x12    // custom command
#endif

// Extended (executed outside IROM) gProbe command codes
#define CMD_WrNvramBlock      0x15    // write NVRAM from RAM buffer
#define CMD_WrNvram           0x16    // write NVRAM from data packet
#define CMD_RdNvram           0x17    // read back NVRAM
#define CMD_DDC2BiPrintPortSelect 0x43 //select DDC2Bi to printout message.

#define CMD_SMTProtocol		0xFE	// Recieve and send SMT messages.

#define CMD_NAK   0x0B
#define CMD_ACK   0x0C
#define CMD_INVALID  0xFF


#define CMD_RS232_Get				0x30	// force chip soft reset
#define CMD_RS232_Set				0x40	// force chip soft reset
#define CMD_ST_Set				0x33	// force chip soft reset

#define CMD_READ_ID         0xAB     // flash device cmd 
#define CMD_READ_ID2        0x9F     // for ST and MXIC

#define XROM_SIGNATURE_OFFS     0x0000   // location of signature, fixed
#define CONFIG_DATA_OFFSET      0x32     //offset from 0x410000

//******************************************************************************
//         D E B U G   C O M M A N D   S T R U C T U R E S
//******************************************************************************
#if (CUSTOMER_TYPE ==Eurobiz)
// Invoke apps debug function - extended debugger command
typedef struct AppsFuncCmdStruct
{
   BYTE	length;				// length of this command packet
   BYTE	cmd;
   BYTE   ID;  
   BYTE	Index;				// function number
} AppsFuncCmd_t;

#else

typedef struct AppsFuncCmdStruct
{
   BYTE	length;				// length of this command packet
   BYTE	cmd;
   BYTE	Index;				// function number
} AppsFuncCmd_t;

#endif   
// command packet to Read a data from NVRAM
typedef struct ReadNvram
{
   BYTE	len; 				// length of this command packet
   BYTE	cmd;
   BYTE	NvramAddr_hi;
   BYTE	NvramAddr_lo;
   BYTE	length;
   BYTE	chk;
} RdNvramCmd_t;

// command packet to Write RAM buffer in NVRAM
typedef struct WriteNvramBlock
{
   BYTE	len; // length of this command packet
   BYTE	cmd;
   BYTE	RamAddr_hi;
   BYTE	RamAddr_lo;
   BYTE	NvramAddr_hi;
   BYTE	NvramAddr_lo;
   BYTE	Length_hi;
   BYTE	Length_lo;
   BYTE	chk;
} WrNvramBlockCmd_t;

// response packet for read packet command
typedef struct RdCmdPktStruct
{
   BYTE   length;                      // length of this response packet
   BYTE   cmd;                         // response packet type
   BYTE   value[SIO_RxBufLen-3];       // value read from register
   BYTE   chk;                         // checksum of packet
} RdCmdPkt_t;

typedef struct 
{
    WORD ConfigCrc;             // crc of configuration table
    WORD ConfigLength;         // size in bytes of configuration table, code starts immediatly after configuration table
    DWORD FlashSize;             // size in bytes of flash device
    WORD CodeCrc;               // crc of code, crc of code data region (starting immediatly after configuration table
                                    // and ending at end of flash (determine by flashsize
    BYTE fw_major;             // major relese number of application (1 or 1.23.156)
    BYTE fw_minor;             // minor release number (23 of 1.23.156)
    WORD fw_build;             // build number (156 of 1.23.156)
} TXromConfigData;


typedef struct WinmateGet {
    BYTE   length;                 // length of this response packet
    BYTE   cmd;                    // response packet type
    BYTE	ID;				// ID //0512 sam add
    BYTE   index;                    // response packet type
    BYTE   value;  			// Get index
    BYTE   chk;                    // checksum of packet
	
} WinmateGetCmd_t;

typedef struct WinmateGet1 {
    BYTE   length;                 // length of this response packet
    BYTE   index;                  // response packet type
    BYTE   value;  			// Get index
    BYTE   chk;                     // checksum of packet
	
} WinmateGetCmd_t1;

typedef struct WinmateSet {
    BYTE   length;                 // length of this response packet
    BYTE   cmd;                    // response packet type
    BYTE   ID;				// ID //0512 sam add    
    BYTE   index;  			// Get index
    BYTE   value;                 // checksum of packet
    BYTE   chk;                     // checksum of packet    
	
} WinmateSetCmd_t;

typedef struct WinmateSet1 {
    BYTE   length;                 // length of this response packet
    BYTE   cmd;                    // response packet type
    BYTE   index;  			// Get index
    BYTE   value;                  // checksum of packet
    BYTE   chk;                     // checksum of packet
    } WinmateSetCmd_t1;


typedef struct WinmateSet2{
    BYTE   length;                 // length of this response packet
    BYTE   cmd;                    // response packet type
    BYTE   index;  			// Get index
    BYTE   Input;                  // checksum of packet
    BYTE   filp_t;                  // checksum of packet
    BYTE   LED_Setting;                  // checksum of packet
    BYTE   Scaling;                  // checksum of packet
    BYTE   Pattern;                  // checksum of packet    
    BYTE   chk;                     // checksum of packet
    } WinmateSetCmd_t2;

 
enum 
{
   WinGet_MainSource = 0x04,
   WinGet_PipSource = 0x05,   
   WinGet_Power=0x06,   
   WinGet_Bright =0x10,
   WinGet_Contrast = 0x11,
   WinGet_Hue = 0x12,
   WinGet_Saturation = 0x13,
   WinGet_InterBright = 0x14,   	
   WinGet_PIP = 0x20,
//   WinGet_PIPSize = 0x21,
   WinGet_Scaling = 0x22,
//   WinGet_sRGB= 0x30,
   WinGet_Gamma = 0x31,   
   WinGet_Temp = 0x32,      
   WinGet_Color_R = 0x33,
   WinGet_Color_G = 0x34,
   WinGet_Color_B = 0x35,
      WinGet_Color_Y = 0x36,
   WinGet_Color_M = 0x37,
   WinGet_Color_C = 0x38,
   WinGet_Volume = 0x50,
//   WinGet_Balance = 0x51,
//   WinGet_Treble = 0x52,
//   WinGet_Bass = 0x53,
   WinGet_Mute = 0x54,   
   WinGet_FLIP = 0x60,
//   WinGet_LUX = 0x10,  
};
enum 
{ 	
    
   WinSet_Auto = 0x01,   	
   WinSet_Recall = 0x02,   	   
   WinSet_ColorBalance = 0x03,
   WinSet_MainSource = 0x04,
   WinSet_PipSource = 0x05,
   WinSet_Power = 0x06,
   WinSet_Bright = 0x10, 
   WinSet_Contrast  = 0x11,
 //  WinSet_Hue = 0x12,
 //  WinSet_Saturation  = 0x13, 
   WinSet_InterBright = 0x14,
   WinSet_PIP = 0x20,
 //  WinSet_PIPSize = 0x21,
   WinSet_Scaling = 0x22,

//   WinSet_sRGB = 0x30,  	
   WinSet_Gamma = 0x31,  	   
   WinSet_Temp = 0x32,  	      
   WinSet_Color_R = 0x33,  	
   WinSet_Color_G = 0x34,  	
   WinSet_Color_B = 0x35,  	   
   WinSet_Color_Y = 0x36,  	
   WinSet_Color_M = 0x37,  	
   WinSet_Color_C = 0x38,  
   WinSet_Volume = 0x50,
 //  WinSet_Balance = 0x51,
 //  WinSet_Treble = 0x52,
 //  WinSet_Bass = 0x53,
   WinSet_Mute = 0x54,   

   WinSet_FLIP=0x60,    
   WinSet_Pattern=0x70,
    WinSet_LED=0x80,
     WinSet_OUTPUT=0x90,
     WinSet_Source_c=0xa0,
	
};

//******************************************************************************
//  G L O B A L    F U N C T I O N    P R O T O T Y P E S
//******************************************************************************
PARSER_STATUS_t SerialCmdHandler	(void);
void GprobeCmdParser(BYTE far* pBuf, HANDLER_USE_t B_whichHandler);

#ifdef USER_EXTENDED_GPROBE_CMD
void  SetParam				(void);
DWORD GetParam				(BYTE B_Index);
void UserExtendableCmd(HANDLER_USE_t B_whichHandler, BYTE B_TestNum);
#endif
//*********************************  END  **************************************
