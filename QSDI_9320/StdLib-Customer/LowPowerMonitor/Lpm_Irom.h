#ifndef __LPM_IROM_H__
#define __LPM_IROM_H__

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
// MODULE:      Lpm_Irom.h
//
// USAGE:       
//
//******************************************************************************


// NOTE: This redefinition of ROM is necessary in order to get rid of "far" data
//   which takes up more space when being referenced
#ifdef ROM
#undef ROM
#endif
#define ROM const

//GPIO0		SPST_SWITCH
//GPIO1		UART UDO_0
//GPIO2		UART UDO_1
//GPIO3		SYS_POWERON
//GPIO4		CEC
#define SPST_SWITCH			BIT0
#define SYS_POWERON			BIT3

#define GPIO_INPUTS			SPST_SWITCH
#define GPIO_OUTPUTS		SYS_POWERON
#define GPIO_OPENDRAINS		(SPST_SWITCH|SYS_POWERON)


typedef unsigned int gmt_RET_STAT;  //GSEL Requirement
#define RET_STAT gmt_RET_STAT    //legacy
enum
{                       // Error status
   OK,                     // Function Call OK
   ERR_SEND,               // Error in sending data
   ERR_RCV,             // Error in receiving data
   ERR_TMO,             // Timeout error
   ERR_PARAM,              // Error in input parameters
   ERR_READ,               // Error in reading
   ERR_WRITE,              // Error in writing
   ERR_COMMAND,            // API command failed
   ERR_POINTER,            // Pointer not intialized
   ERR_OVERFLOW,           // Input parameter vlaues over limit
   ERR_MEMORY,             // Out of SDRAM memory
   ERR_NO_DATA,            // IR FIFO queue is empty
   ERR_BUSY,               // Device busy
   ERR_OSD_INVALID_TILESET,   // Invalid tileset
   ERR_OSD_INVALID_TILE,      // Invalid tile number
   ERR_OSD_DISABLED_TILE,     // tile is disabled, can't draw the control
   ERR_OSD_INVALID_SIZE,      // Invalid control size
   ERR_OSD_INVALID_BPP,    // Invalid color depth
   ERR_OSD_INVALID_HILITE,    // HiLite window exceeds max
   ERR_OSD_INVALID_FONT_ID,   // Invalid font id
   ERR_OSD_TOOBIG_STRING,     // Temporary buffer size is too small for rendering
   ERR_LIMIT,              // Reach limit of position
   ERR_ADC_CALIBRATE,         // Error in ADC calibration
   ERR_BIT_NOT_SET,        // Status Bit not set
   ERR_FSB_WR_SDRAM_FIFO_ALMOST_EMPTY,
   ERR_FSB_DATA_EXPANDER_RDY,
   ERR_FSB_WR_XFER_IN_PROGRESS,
   ERR_FSB_WR_SDRAM_FIFO_FULL,
   ERR_FSB_RD_SDRAM_FIFO_FULL,
   ERR_FSB_RD_SDRAM_FIFO_EMPTY,
   ERR_FSB_RD_XFER_IN_PROGRESS,
   ERR_RFU = ERR_FSB_RD_XFER_IN_PROGRESS + 20,  //reserved for future use
   LAST_IROM_DEFINED_RET_STAT = ERR_RFU
};

enum
{
   OSD_INVALID_TILESET = ERR_OSD_INVALID_TILESET,  // Invalid tileset
   OSD_INVALID_TILE = ERR_OSD_INVALID_TILE,  // Invalid tile number
   OSD_DISABLE_TILE = ERR_OSD_DISABLED_TILE, // tile is disabled, can't draw the control
   OSD_INVALID_SIZE = ERR_OSD_INVALID_SIZE,  // Invalid control size
   OSD_INVALID_BPP = ERR_OSD_INVALID_BPP, // Invalid color depth
   OSD_INVALID_FONT_ID = ERR_OSD_INVALID_FONT_ID,  // Invalid font id
   OSD_TOOBIG_STRING = ERR_OSD_TOOBIG_STRING,      // Temporary buffer size is too small for rendering
   ABORTED = LAST_IROM_DEFINED_RET_STAT+1,  // execution was aborted
   FAILED,                       // something failed
   TMR_ACTIVE,                   // Timer is active
   TMR_TIMED_OUT,                // Timer is timed out
   TMR_STOPPED,                  // Timer is stopped
   SYNC_HONVON,                  // Hsync on, Vsync on
   SYNC_HOFFVON,                 // Hsync off, Vsync on
   SYNC_HONVOFF,                 // etc
   SYNC_HOFFVOFF,                // etc
   AA_LOST_FEATURES,             // auto adjust lost features
   ERR_CHANNEL,                  // Channel is Invalid
   ERR_INCORRECT_PIN,            // Incorrect Pin - AIP
   ERR_DECODER_STAT,             // Decoder Status Error
   FEATURE_NO_AVAIL,           // Select feature not available
   ERR_DECODER_NOISY_INPUT,      // Input to Decoder is Noisy
   ERR_DEC_MODE_CHANGE           // Decoder has a Mode Change
};

//
// FAIL / PASS enumeration
//
typedef enum
{
   gmd_FAIL = 0,
   gmd_PASS = 1,
} gmt_RET_PASSFAIL;


typedef union 
{
   BYTE b[5];
   WORD w;
   DWORD d;
} gmt_STime;

// 7 6 5 4 3 2 1 0 byte Precise system time in ms and clk (Cortez Premium: 35 clk = 1mks)
// | | | | | | |_|________WORD clk, clk
// |_|_|_|_|_|________BYTE ms.b[5]
//     |_|_|_|________DWORD ms.d, ms
//         |_|________WORD ms.wt, ms
//#pragma pack(push)
//#pragma pack(1)
typedef struct 
{
   WORD clk;
   gmt_STime ms;
} gmt_STimeEx;

enum
{
   START_1ms,                 // ***** 1ms timer start
   DELAY_1ms_TMR=START_1ms,      // (0) Delay timer
   UART_1ms_TMR,             // (1) UART communication timer
   NVRAM_1ms_TMR,            // (2) Nvram timer
   ASYNC_1ms_TMR,            // (3) Async timer   
   PWROFF_1ms_TMR,           // (4) Power Off timer
   END_1ms,                // ***** 1ms timer end
   LPM_MAX_TIMER=END_1ms
};


#define LPM_SEGBIT2SHIFT 4 // 20-bit mode
//#define LPM_SEGBIT2SHIFT 8 // 24-bit mode
#define LPM_LINEAR_TO_FPTR(Linear_Address) ((void far *)\
           ((void _seg *)(((WORD)(Linear_Address >> 16)) << (16 - LPM_SEGBIT2SHIFT)) +\
           (void __near *)((WORD)Linear_Address)))
#define LPM_LINEAR_TO_FPTR_B(Linear_Address) ((BYTE far *)\
              ((void _seg *)(((WORD)(Linear_Address >> 16)) << (16 - LPM_SEGBIT2SHIFT)) +\
              (void __near *)((WORD)Linear_Address)))
#define LPM_LINEAR_TO_FPTR_W(Linear_Address) ((WORD far *)\
           ((void _seg *)(((WORD)(Linear_Address >> 16)) << (16 - LPM_SEGBIT2SHIFT)) +\
           (void __near *)((WORD)Linear_Address)))
#define LPM_LINEAR_TO_FPTR_DW(Linear_Address) ((DWORD far *)\
              ((void _seg *)(((WORD)(Linear_Address >> 16)) << (16 - LPM_SEGBIT2SHIFT)) +\
              (void __near *)((WORD)Linear_Address)))

#define gm_RegBitShiftValue(BitName, Value) ((Value) << (BitName ## _SHIFT))

#if 1 // LPM use word access
#undef gm_ReadRegWord(W_RegAddr)
#undef gm_ClearAndSetBitsWord(W_RegAddr, W_Clear, W_Set)
#undef gm_ClearRegBitsWord(W_RegAddr, W_Clear) 
#undef gm_SetRegBitsWord(W_RegAddr, W_Set)
#undef gm_ReadRegDWord_LE(W_RegAddr)
#undef gm_SetRegBitsDWord(DW_RegAddr, DW_Set)

#define gm_ReadRegWord(W_RegAddr) (*(volatile WORD *)(W_RegAddr))
#define gm_ClearAndSetBitsWord(W_RegAddr, W_Clear, W_Set) (*(volatile WORD *)(W_RegAddr) = \
    ((*(volatile WORD*)(W_RegAddr) & ~(WORD)(W_Clear))) | (WORD)(W_Set))
#define gm_ClearRegBitsWord(W_RegAddr, W_Clear) (*(volatile WORD *)(W_RegAddr) &= ~(WORD)(W_Clear))
#define gm_SetRegBitsWord(W_RegAddr, W_Set) (*(volatile WORD *)(W_RegAddr) |= (WORD)(W_Set))
#define gm_ReadRegDWord_LE(W_RegAddr) \
	   (((DWORD)(*(volatile WORD hostreg*)(W_RegAddr+2)) << 16) | *(volatile WORD hostreg*)(W_RegAddr))
#define gm_SetRegBitsDWord(DW_RegAddr, DW_Set) (*(volatile DWORD *)(DW_RegAddr) |= (DWORD)(DW_Set))
#endif
DWORD far gm_GetSystemTime(void);
WORD far gm_W_GetSystemTime(void);
void gm_Delay1ms(WORD ms);
void Lpm_TimerInit(DWORD D_CpuClock);
void Lpm_TimerStart(BYTE B_Timer, WORD W_TimeOutInMS);
BOOL Lpm_TimerTimeOut(BYTE B_Timer);
void Lpm_Delay1ms(WORD W_TimeOutInMS);
void Lpm_McuInit(void);


#endif //__LPM_IROM_H__
