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
// MODULE:      Lpm_Uart.c
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
#include <mem.h>

//******************************************************************************
//
//  L O C A L    D E F I N E S
//
//******************************************************************************

#ifdef UART_DEBUG_SUPPORT
#define pcbReg(addr)			READ_PCB_REG(addr)
#define UART_TRX_TIMEOUT		1000		//in milliseconds; default is one second delay

#define gmTimerStartWd(timeout) {DW_uart0TimeOutInMS = gm_GetSystemTime()+timeout;}
#define gmTimeOutWd 			(DW_uart0TimeOutInMS<gm_GetSystemTime())

#define WaitConditionTimeout(condition, timeout) \
		if(condition) \
		{ \
			gmTimerStartWd(timeout); \
			while(condition) \
			{ \
				if(gmTimeOutWd) \
				{ \
					return ERR_SEND; \
				} \
			} \
		}

static BOOL uart0ReceiveMode;
static BYTE *uart0PBuf = NULL_PTR;
static BYTE *uart0PTrxData;
static WORD uart0TrxCount;
WORD uart0RcvCount;
static WORD W_Uart0BuffSize;
static DWORD DW_uart0TimeOutInMS;

BYTE Ba_CommBufUart0[UART_BUFFER_SIZE];
BYTE Ba_TrxBufUart0[UART_BUFFER_SIZE];


//******************************************************************************
//
//  C O D E
//
//******************************************************************************

//******************************************************************************
//
// FUNCTION:    void huge interrupt Lpm_UartIsr(void)
//
// USAGE:		Mcu UART0 interrupt service routine and save UART data into
//				receive buffer.
//
// INPUT:		None
//
// OUTPUT:		None
//
// GLOBALS:		uartReceiveMode, uartRcvCount, uartTrxCount, uartPBuf
//
// USED_REGS:	None
//
//******************************************************************************
void interrupt Lpm_UartIsr(void)
{
	asm	{sti} 										// Allow other interrupt
	
	if (pcbReg(SP0STS) & RDR)						// Check if new char received
	{
		if (uart0ReceiveMode && uart0RcvCount < W_Uart0BuffSize)	
		{											// Get data from UART
			uart0PBuf[uart0RcvCount++] = pcbReg(SP0RD);
			Lpm_TimerStart(UART_1ms_TMR, 10);							// Start receive timer
		}
		else
		{
			_AX = pcbReg(SP0RD);					// Read data to clear interrupt
		}
	}
	if (uart0TrxCount && (pcbReg(SP0STS) & THRE))	// Data available to transmit ?
	{											
		pcbReg(SP0TD) = *uart0PTrxData;				// Send data
		uart0PTrxData++;								// Point to next byte
		if (--uart0TrxCount == 0)					// No more data ?
			CLEAR_PCB_REG_BITS(SP0CT, TXIE);		// Disable transmit interrupt
	}

	asm { cli }										// to avoid interrupt between
													// next command and iret
	pcbReg(EOI) = UART0_VTYPE;						// Clear interrupt
}

//******************************************************************************
//
// FUNCTION:    void huge Lpm_UartInit(DWORD D_CpuClock, DWORD D_BaudRate, 
//									BYTE * pbuf, WORD W_BuffSize)
//
// USAGE:		Initialization of UART_0 Control Registers, setup baudrate, buffer and 
//				interrupt service routine.
//
// INPUT:		D_CpuClock = MCU clock
//				D_BaudRate = UART baud rate
//				pbuf = pointer to communication buffer
//
// OUTPUT:		None
//
// GLOBALS:		uartReceiveMode, uartRcvCount, uartTrxCount
//
// USED_REGS:	None
//
//******************************************************************************
void Lpm_UartInit(DWORD D_CpuClock, DWORD D_BaudRate, BYTE * pbuf, 
						WORD W_BuffSize)
{
	uart0ReceiveMode = TRUE;
	uart0TrxCount = 0;
	uart0RcvCount = 0;
	

	// Initialize the Uart0 comm buffer
	uart0PBuf = pbuf;
	W_Uart0BuffSize = W_BuffSize;

	// Setup Serial Port0 - SP0BAUD = D_CpuClock / (D_BaudRate * 16)
	D_BaudRate = D_CpuClock / (D_BaudRate * 8);	// Calculate baudrate value
	
	if (((BYTE)D_BaudRate) & 1)					//round value
	{
		D_BaudRate = (D_BaudRate / 2) + 1;
	}
	else
	{
		D_BaudRate /= 2;
	}

	asm cli;
	//SET_VECTOR(UART0_VECTOR, isrUart);			// Set interrupt vector
	pcbReg(SP0BAUD) = (WORD)D_BaudRate;			// Set baudrate
	pcbReg(SP0CT) = RxNoDMATxNoDMA|RXIE|TMODE|RMODE|DATAMODE1;
	CLEAR_PCB_REG_BITS(IMASK, SP0);

	//GPIO pins for UART or I2C SLV4
	gm_ClearRegBitsWord(LPM_GPIO_ENABLE1, (LPM_GPIO1_EN|LPM_GPIO2_EN));
	//Set LPM_GPIO1,2 for LPM UDO,UDI
	gm_WriteRegWord(I2C_SLAVE4_CTRL1, 0);
	gm_SetRegBitsWord(LPM_MICRO_CONTROL, EN_LPM_UART);
	asm sti;
}

//******************************************************************************
//
// FUNCTION:    BOOL Lpm_UartIsTrxBufEmpty(void)
//
// USAGE:		Check whether all data in transmit buffer is completed.
//
// INPUT:		None
//
// OUTPUT:		TRUE if transmission complete flag is set and
//				number of bytes left in the buffer is 0
//				FALSE if number of bytes left in the buffer is not 0
//
// GLOBALS:		uartTrxCount
//
// USED_REGS:	None
//
// NOTES:		The UART is double buffered, checking the TEMT is to 
//				guarantee the last bit has been sent.
//
//******************************************************************************
BOOL Lpm_UartIsTrxBufEmpty(void)
{
	return ((pcbReg(SP0STS) & TEMT) && (uart0TrxCount == 0)) ? TRUE : FALSE;
}

//******************************************************************************
//
// FUNCTION:    gmt_RET_STAT huge Lpm_UartTrxChar(BYTE B_Data)
//
// USAGE:		Transmit a single byte of data through UART
//
// INPUT:		B_Data - data to send
//
// OUTPUT:		None
//
// GLOBALS:		None
//
// USED_REGS:	None
//
//******************************************************************************
gmt_RET_STAT Lpm_UartTrxChar(BYTE B_Data)
{
#if 1
	WaitConditionTimeout(!(Lpm_UartIsTrxBufEmpty()), UART_TRX_TIMEOUT);

	pcbReg(SP0TD) = B_Data;				// Send data

	WaitConditionTimeout(!(pcbReg(SP0STS) & THRE), UART_TRX_TIMEOUT);

	return OK;
#else
	while(Lpm_UartIsTrxBufCompleted()); 		// Make sure the trx buffer is cleared 
	WRITE_PCB_REG(SP0TD, B_Data);			 // Send data 
	while(!(READ_PCB_REG(SP0STS) & THRE))	   // wait for tx ready to be set 
	{
	}
   return OK;
#endif
}

//***************************************************************************** 
// FUNCTION     : void Lpm_UartTrxBuf(BYTE *Bp_Data, WORD W_Len) 
// USAGE        : Transmit a buffer of data through UART 
// INPUT        : Bp_Data - buffer pointer 
//                W_Len - number of byte to transmit 
// OUTPUT       : 
// GLOBALS      : Bp_TrxBuf, W_TrxCount 
// USED_REGS    : 
//                NOTES:      Application need to check before or after transmission 
//                to confirm data have been transmitted. 
//                The function "UartIsTrxCompleted" should be used. 
//                It is using the serial interrupt transmission service 
//***************************************************************************** 
void Lpm_UartTrxBuf(BYTE *Bp_Data, WORD W_Len)
{
    while(Lpm_UartIsTrxBufCompleted());         // Make sure the trx buffer is cleared 
    uart0PTrxData  = Bp_Data;                  // Save buffer pointer 
    uart0TrxCount = W_Len;                   // Adjust counter 
    SET_PCB_REG_BITS(SP0CT, TXIE);            // Enable transmit interrupt 
}

//***************************************************************************** 
// FUNCTION     : WORD Lpm_UartIsTrxBufCompleted(void) 
// USAGE        : Check whether all data in transmit buffer is completed. 
// INPUT        : 
// OUTPUT       : Number of byte left. 0 = transmission completed 
//                otherwise = number of bytes left in the buffer 
// GLOBALS      : W_TrxCount 
// USED_REGS    : 
//***************************************************************************** 
WORD Lpm_UartIsTrxBufCompleted(void)
{
    return (uart0TrxCount);                  // Number of byte left 
}

//******************************************************************************
//
// FUNCTION:    WORD huge Lpm_UartRcvBuf(BYTE * pdata)
//
// USAGE:		Receive a packet of data through UART.
//				It based on timeout to determine packet received.
//
// INPUT:		pdata - buffer pointer
//
// OUTPUT:		Number of byte received
//
// GLOBALS:		uartRcvCount, uart0PBuf
//
// USED_REGS:	None
//
//******************************************************************************
WORD Lpm_UartRcvBuf(BYTE * pdata)
{
	if (uart0RcvCount && Lpm_TimerTimeOut(UART_1ms_TMR))
	{
		WORD W_Count;
        asm cld; memcpy(pdata, uart0PBuf, uart0RcvCount); // Copy receive buffer
		W_Count = uart0RcvCount;							// Save receive count
		uart0RcvCount = 0;									// Clear count

		return (W_Count);								// Receive buffer count
	}
	else
	{
		return 0;										// No Data yet
	}
}

//******************************************************************************
//
// FUNCTION:    void huge Lpm_StopRcvMsg(void)
//
// USAGE:		Stop receiving incomming bytes into the communication buffer
//				Used for serial and ddc2bi communication handlers
//
// INPUT:		None
//
// OUTPUT:		None
//
// GLOBALS:		gB_RcvStatus
//
// USED_REGS:	None
//
//******************************************************************************
void Lpm_StopRcvMsg(void)
{
	uart0ReceiveMode = FALSE;					// stop receiving messages
}

//******************************************************************************
//
// FUNCTION:    void huge Lpm_StartRcvMsg(void)
//
// USAGE:		Start receiving incoming bytes into the communication buffer
//				Used for serial and ddc2bi communication handlers
//
// INPUT:		None
//
// OUTPUT:		None
//
// GLOBALS:		gB_RcvStatus
//
// USED_REGS:	None
//
//******************************************************************************
void Lpm_StartRcvMsg(void)
{
	uart0ReceiveMode = TRUE;						// start receiving messages
}
#endif


