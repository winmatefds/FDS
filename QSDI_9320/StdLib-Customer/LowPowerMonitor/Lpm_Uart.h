#ifndef __LPM_UART_H__
#define __LPM_UART_H__

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
// MODULE:      Lpm_Uart.h
//
// USAGE:       
//
//******************************************************************************

#define UART_BUFFER_SIZE	  256
#define UART_BAUD_RATE		  19200L


typedef unsigned int gmt_RET_STAT;  //GSEL Requirement


extern WORD uart0RcvCount;
extern BYTE Ba_CommBufUart0[UART_BUFFER_SIZE];
extern BYTE Ba_TrxBufUart0[UART_BUFFER_SIZE];


BOOL gm_UartHandler(void);
void interrupt Lpm_UartIsr(void);
void Lpm_UartInit(DWORD D_CpuClock, DWORD D_BaudRate, BYTE * pbuf, WORD W_BuffSize);
BOOL Lpm_UartIsTrxBufEmpty(void);
gmt_RET_STAT Lpm_UartTrxChar(BYTE B_Data);
void Lpm_UartTrxBuf(BYTE *Bp_Data, WORD W_Len);
WORD Lpm_UartIsTrxBufCompleted(void);
WORD Lpm_UartRcvBuf(BYTE * pdata);
void Lpm_StopRcvMsg(void);
void Lpm_StartRcvMsg(void);


#endif //__LPM_UART_H__

