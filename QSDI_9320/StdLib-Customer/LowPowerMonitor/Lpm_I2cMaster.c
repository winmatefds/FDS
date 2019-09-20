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
//=============================================================================
//
// MODULE:      Lpm_I2cMaster.c
//
// USAGE:		This module contains functions for I2C master driver.  The
//				hardware support I2C master feature which can read/write to
//				external I2C device.   
//
//******************************************************************************

//******************************************************************************
//
//  I N C L U D E    F I L E S
//
//******************************************************************************
#include ".\LowPowerMonitor\Lpm_All.h"


#ifdef I2C_MASTER_SUPPORT

//****************************************************************************** 
//
//  G L O B A L    D E F I N I T I O N S 
//
//****************************************************************************** 
//#define	_2WIRE_DMA_WR   0
//#define	_2WIRE_DMA_RD   0 // DMA Read is not optimized due to Cortez-BD HW
#define	_2WIRE_VAR_CLK  1
#define _2WIRE_RING     1

#if defined(_2WIRE_DMA_WR) || defined(_2WIRE_DMA_RD)
// Cortez "1001"
#define	OCM_DRQ0_SEL_I2C_MST (WORD)(BIT3 | BIT0)
#define	OCM_DRQ1_SEL_I2C_MST (WORD)(BIT7 | BIT4)
#define	OCM_DRQ2_SEL_I2C_MST (WORD)(BIT11 | BIT8)
#define	OCM_DRQ3_SEL_I2C_MST (WORD)(BIT15 | BIT12)

// Cortez Premium "1100"
//#define	OCM_DRQ0_SEL_I2C_MST (WORD)(BIT3 | BIT2)
//#define	OCM_DRQ1_SEL_I2C_MST (WORD)(BIT7 | BIT6)
//#define	OCM_DRQ2_SEL_I2C_MST (WORD)(BIT11 | BIT10)
//#define	OCM_DRQ3_SEL_I2C_MST (WORD)(BIT15 | BIT14)
#endif

//#if (FEATURE_2WIRE_NUM == 2)
//#define IDX_OFFSET	(I2C2_MST_CTR - I2C_MST_CTR)
//#else
#define IDX_OFFSET	0
//#endif	//(FEATURE_2WIRE_NUM == 2)

//******************************************************************************
//
//  F U N C T I O N    P R O T O T Y P E S
//
//******************************************************************************
static WORD	I2CMasterReg (WORD W_RegAddr);


//****************************************************************************** 
//
//  G L O B A L    V A R I A B L E S 
//
//****************************************************************************** 
// holds the I2C Master index - set to 0 for all cases. In order to change it, a
// call to Lpm_I2CMasterSelectIdx is necessary (applicable for FLI8668 core only,
// as the only core having the second I2C Master block)

//gmt_2WIRE_MASTER_IDX	B_2WireIdx = MASTER_2WIRE_1;

static WORD W_TimeoutCount=1000;


//******************************************************************************
//
//  C O D E
//
//******************************************************************************
//******************************************************************************
//
// FUNCTION       : void Lpm_I2CMasterSetTimeout(WORD W_Timeout)
//
// USAGE          : Set the slave device no response timeout
//
// INPUT          : W_Timeout - The value is CPU loop count while waiting
//								for slave device response.
//								default value = 1000 for MCU-T186 @120MHz
//
// OUTPUT         : None
//
// GLOBALS        : W_TimeoutCount
//
//******************************************************************************
void Lpm_I2CMasterSetTimeout(WORD W_Timeout)
{
	W_TimeoutCount = W_Timeout;
}

//******************************************************************************
//
// FUNCTION       : gmt_RET_PASSFAIL Lpm_I2CMasterIsCmdStatusZero(WORD W_Status)
//
// USAGE          : Check if specified status is zero with timeout to exit
//
// INPUT          : W_Status - I2C host register status bit
//
// OUTPUT         : gmd_PASS (0) - STATUS_CONFIRMED
//                  gmd_FAIL (1) - TIMEOUT
//
// GLOBALS        : none
//
// USED_REGS      : I2C_MST_RXR_SR
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
gmt_RET_PASSFAIL Lpm_I2CMasterIsCmdStatusZero(WORD W_Status)
{
	WORD W_Count=W_TimeoutCount;		// Setup timeout count

	while (W_Count--)  					// Timeout exit
	{
		// W_Status could be one of the below and W_Value is either 1 or 0
		// TIP=1  : Transferring data,     0: Transfer completed
		// Busy=1 : START signal detected, 0: STOP signal detected
		if ((gm_ReadRegWord(I2CMasterReg(LPM_I2C_MST_STATUS)) & W_Status) == 0)
			return gmd_PASS;
	}
	return gmd_FAIL;					// Timeout ...
}

//******************************************************************************
//
// FUNCTION       : gmt_RET_PASSFAIL Lpm_I2CMasterAckReceived(void)
//
// USAGE          : If I2C status register return one (No acknowledge received) in
//                  RxACK bit after TIP bit indicate data transfer completed,
//                  return gmd_FAIL.
//
// INPUT          : None
//
// OUTPUT         : gmt_RET_PASSFAIL
//
// GLOBALS        : gW_TimerCount
//
// USED_REGS      : I2C_MST_RXR_SR
//
// PRE_CONDITION  : 
//
// POST_CONDITION : 
//
//******************************************************************************
gmt_RET_PASSFAIL Lpm_I2CMasterAckReceived(void)
{
	if (Lpm_I2CMasterIsCmdStatusZero(LPM_I2C_M_TIP) == gmd_PASS) // Wait for data transfer complete.
		if (Lpm_I2CMasterIsCmdStatusZero(LPM_I2C_M_RxACK) == gmd_PASS) // RxACK=1 : No acknowledge received
			return gmd_PASS;					// RxACK=0 : Acknowledge received
	return gmd_FAIL;
}

//******************************************************************************
//
// FUNCTION       : gmt_RET_PASSFAIL  Lpm_I2CMasterStart(BYTE B_DevAddr, gmt_2WIRE_CONTROL Ctrl)
//
// USAGE          : Command I2C master to generate a start condition and
//                  waiting for acknowledgment.
//
// INPUT          : B_DevAddr - I2C slave address
//                  Ctrl - Control mode, either Read(gmd_2WIRE_RD = 1) or write(gmd_2WIRE_WR = 0)
//
// OUTPUT         : gmt_RET_PASSFAIL - gmd_FAIL or gmd_PASS 
//
// GLOBALS        : None
//
// USED_REGS      : I2C_MST_TXR_CR,
//
// PRE_CONDITION  : 
//
// POST_CONDITION : 
//
//******************************************************************************
gmt_RET_PASSFAIL  Lpm_I2CMasterStart(BYTE B_DevAddr, gmt_2WIRE_CONTROL Ctrl)
{
	// set Ctrl = Read or Write command to slave
	// I2C_WR = write to slave (no Ack send).
	// I2C_STA = START condition.
	gm_WriteRegWord (I2CMasterReg (LPM_I2C_MST_TXR_CTRL), ((B_DevAddr | Ctrl) << 8) | LPM_I2C_M_WR | LPM_I2C_M_STA);
	// Waiting for Acknowledgment
	return Lpm_I2CMasterAckReceived();
}

//******************************************************************************
//
// FUNCTION       : gmt_RET_PASSFAIL  Lpm_I2CMasterCheck(gmt_2WIRE_RING B_Port, BYTE B_DevAddr)
//
// USAGE          : Perform START and STOP on the specific device to determine
//                  whether the deivce acknowledged.
//
// INPUT          : B_Port
//                  B_DevAddr = I2C slave device address
//
// OUTPUT         : 
//
// GLOBALS        : None
//
// USED_REGS      : I2C_MST_TXR_CR used to command the external I2C
//                  device as specified in B_DevAddr and B_RegAddr.
//                  I2C_MST_RXR_SR contains previous I2C command status and the data
//                  received from the external I2C device.
//
// PRE_CONDITION  : 
//
// POST_CONDITION : 
//
//******************************************************************************
gmt_RET_PASSFAIL  Lpm_I2CMasterCheck(gmt_2WIRE_RING B_Port, BYTE B_DevAddr)
{
	gmt_RET_PASSFAIL result;

	B_Port--;					// Dummy to be implement
	result = Lpm_I2CMasterStart(B_DevAddr, gmd_2WIRE_WR);
	Lpm_I2CMasterStop();
	return result;
}

//******************************************************************************
//
// FUNCTION       : void Lpm_I2CMasterStop(void)
//
// USAGE          : Command 2Wire master to generate a stop condition and waiting for
//                  2Wire status register return zero in BUSY bit.
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        : None
//
// USED_REGS      : I2C_MST_TXR_CR
//
// PRE_CONDITION  : 
//
// POST_CONDITION : 
//
//******************************************************************************
void Lpm_I2CMasterStop(void)
{
   gm_WriteRegWord (I2CMasterReg (LPM_I2C_MST_TXR_CTRL), LPM_I2C_M_STO);
   Lpm_I2CMasterIsCmdStatusZero(BUSY);
}

//******************************************************************************
//
// FUNCTION       : gmt_RET_PASSFAIL  Lpm_I2CMasterGetByte(WORD W_Cmd, BYTE * Bp_Data)
//
// USAGE          : Command I2C master read a byte from the slave
//
// INPUT          : Cmd - A I2C master command
//                  Bp_Data - a container for the receiving byte
//
// OUTPUT         : gmt_RET_PASSFAIL - Error status
//
// GLOBALS        : None
//
// USED_REGS      : I2C_MST_TXR_CR and I2C_MST_RXR_SR
//
// PRE_CONDITION  : 
//
// POST_CONDITION : 
//
//******************************************************************************
gmt_RET_PASSFAIL  Lpm_I2CMasterGetByte(WORD W_Cmd, BYTE far* Bp_Data)
{
	gm_WriteRegWord (I2CMasterReg (LPM_I2C_MST_TXR_CTRL), W_Cmd);	 // Read command												
	if (Lpm_I2CMasterIsCmdStatusZero(LPM_I2C_M_TIP) == gmd_FAIL)	// Data transfer completed ?
		return gmd_FAIL;				
	
	// Shift RCV_BYTE to low byte and the then copy to Bp_Data
	*Bp_Data = gm_ReadRegWord (I2CMasterReg (LPM_I2C_MST_STATUS)) >> 8;
	return gmd_PASS;
}


//******************************************************************************
//
// FUNCTION       : gmt_RET_PASSFAIL Lpm_I2CMasterPutByte(BYTE far * Bp_Data)
//
// USAGE          : Command I2C master write a byte to the slave
//
// INPUT          : Bp_Data - a container of the sending byte
//
// OUTPUT         : RET_STAT - Error status
//
// GLOBALS        : None
//
// USED_REGS      : I2C_MST_TXR_CR
//
// PRE_CONDITION  : 
//
// POST_CONDITION : 
//
//******************************************************************************
gmt_RET_PASSFAIL Lpm_I2CMasterPutByte(BYTE far * Bp_Data)
{
   WORD  W_I2cCmd = *Bp_Data;
   W_I2cCmd <<= 8;  //Shift Bp_Data to High byte   
   
   gm_WriteRegWord (I2CMasterReg (LPM_I2C_MST_TXR_CTRL), W_I2cCmd | LPM_I2C_M_WR); //set WR (write to slave) and send out

   return (Lpm_I2CMasterAckReceived());
}

//******************************************************************************
//
// FUNCTION       : gmt_RET_STAT Lpm_I2CMasterWriteAddrBuffer (BYTE B_DevAddr,
//                  										BYTE far *Bp_RegAddr, BYTE B_AddrLen,
//                  										BYTE far *Bp_Data, WORD W_Len)
//
// USAGE          : A sequence of command to I2C master for writing data from 
//                  the specified I2C slave with ONE byte sub-address.
//
// INPUT          : B_DevAddr	-	I2C slave device address
//                  Bp_RegAddr - Pointer of sub-address of slave device
//                  B_AddrLen - Length of sub-address buffer
//                  Bp_Data - Pointer of data buffer
//                  W_Len	-	Length of data buffer
//
// OUTPUT         : OK			-	Command accepted
//                  ERR_PARAM	-	Invalid input parameter W_Len
//                  ERR_POINTER	-	Invalid input parameter Bp_Data
//
// GLOBALS        : None
//
// USED_REGS      : None
//
// PRE_CONDITION  : 
//
// POST_CONDITION : 
//
//******************************************************************************
gmt_RET_STAT Lpm_I2CMasterWriteAddrBuffer (BYTE B_DevAddr,
										BYTE far *Bp_RegAddr, BYTE B_AddrLen,
										BYTE far *Bp_Data, WORD W_Len)
{
	gmt_RET_PASSFAIL result;

	// Validate input parameters
	if (Bp_Data == NULL_PTR)
		return ERR_POINTER;
	else if (W_Len == 0)
		return ERR_PARAM;

	result = Lpm_I2CMasterStart(B_DevAddr, gmd_2WIRE_WR);

	// SEND Slave write sub-address
	while ((result == gmd_PASS) && (B_AddrLen--))							
		result = Lpm_I2CMasterPutByte(Bp_RegAddr++);	
	
	// Write data from buffer into device
	while ((result == gmd_PASS) && (W_Len--))
		result = Lpm_I2CMasterPutByte(Bp_Data++);	
	
	Lpm_I2CMasterStop();	
	if (result != gmd_PASS) 
		return FAILED;
	return OK;								
}

//******************************************************************************
//
// FUNCTION       : gmt_RET_STAT Lpm_I2CMasterReadAddrBuffer (BYTE B_DevAddr,
//                  	BYTE far *Bp_RegAddr, BYTE B_AddrLen, BYTE *Bp_Data, WORD W_Len)
//
// USAGE          : A sequence of command to I2C master for reading data from 
//                  the specified I2C slave with ONE byte sub-address.
//
// INPUT          : B_DevAddr	-	I2C slave device address
//                  Bp_RegAddr - Pointer of sub-address of slave device
//                  B_AddrLen - Length of sub-address buffer
//                  Bp_Data - Pointer of data buffer
//                  W_Len -	Length of data buffer
//
// OUTPUT         : OK			-	Command accepted
//                  ERR_PARAM	-	Invalid input parameter W_Len
//                  ERR_POINTER	-	Invalid input parameter Bp_Data
//
// GLOBALS        : None
//
// USED_REGS      : None
//
// PRE_CONDITION  : 
//
// POST_CONDITION : 
//
//******************************************************************************
gmt_RET_STAT Lpm_I2CMasterReadAddrBuffer (BYTE B_DevAddr,
	BYTE far *Bp_RegAddr, BYTE B_AddrLen, BYTE far*Bp_Data, WORD W_Len)
{
	gmt_RET_PASSFAIL result;

	// Validate input parameters
	if (Bp_Data == NULL_PTR)
		return ERR_POINTER;
	if (W_Len == 0)
		return ERR_PARAM;

	result = Lpm_I2CMasterStart(B_DevAddr, gmd_2WIRE_WR);

	//SEND Slave read sub-address
	while ((result == gmd_PASS) && (B_AddrLen--))
		result = Lpm_I2CMasterPutByte(Bp_RegAddr++);	
		
	// repeated START for read operation
	if (result == gmd_PASS)
		result = Lpm_I2CMasterStart(B_DevAddr, gmd_2WIRE_RD);
	
	// Read data from slave device into buffer
	while ((result == gmd_PASS) && (W_Len--))	
		result = Lpm_I2CMasterGetByte (((W_Len == 0) ? (LPM_I2C_M_RD|LPM_I2C_M_ACK) : (LPM_I2C_M_RD)), Bp_Data++);
	
	Lpm_I2CMasterStop();									
	if (result != gmd_PASS) 
		return FAILED;
	return OK;								
}

//******************************************************************************
//
// FUNCTION       : gmt_RET_PASSFAIL Lpm_I2CMasterWriteBlock(gmt_2WIRE_RING B_Port, BYTE B_DevAddr,
//                  									BYTE far *Bp_Buffer, BYTE B_Length, 
//                  									gmt_2WIRE_STOP B_Stop)
//
// USAGE          : Writes one or more bytes from the buffer to the I2C device
//
// INPUT          : B_Port
//                  B_DevAddr - Address of the device that need to be written
//                  Bp_Buffer - Buffer that contains the Sub address(if any)
//                     and the Data that need to be written.
//                  B_Length -   Number of Bytes in the Buffer.
//                  B_Stop -  Stop Condition. TRUE indicates to send a Stop Condition
//                     at the end of the packet.
//
// OUTPUT         : gmt_RET_PASSFAIL : Status of the Write function.
//
// GLOBALS        : None
//
// USED_REGS      : None
//
// PRE_CONDITION  : 
//
// POST_CONDITION : 
//
//******************************************************************************
gmt_RET_PASSFAIL Lpm_I2CMasterWriteBlock(gmt_2WIRE_RING B_Port, BYTE B_DevAddr,
									BYTE far *Bp_Buffer, BYTE B_Length, 
									gmt_2WIRE_STOP B_Stop)
{

	B_Port--;					// Dummy to be implement

	// Validate input parameters
	if ((Bp_Buffer == 0) || (B_Length == 0)) return gmd_FAIL;

#ifdef _2WIRE_DMA_WR	

	//
	// use DMA for writing to Hudson for messages with STOP condition only
	//
	// TODO: the actual DevAddr for any 2Wire device must be replaced by their 'DEVICE_ADDRESS'
	//
	if ((B_DevAddr == 0x6C) && (B_Stop == STOP_BIT)) // B_DevAddr == 0x6C
	{
		//
		// DMA access to I2C Master
		//
		
		{
			WORD W_DMACtrl;

      B_Stop = B_Stop;
			gm_ClearRegBitsWord(DMA_REQUEST_CTRL, OCM_DRQ1_SEL);
			gm_SetRegBitsWord( DMA_REQUEST_CTRL, OCM_DRQ1_SEL_I2C_MST ); // 1001 - I2C Master


			// Start bit
			Lpm_I2CMasterStart(B_DevAddr, gmd_2WIRE_WR); // first byte is not a DMA transferred byte
													// to be expected an upgrade of I2C Master DMA HW

			gm_SetRegBitsWord (I2CMasterReg (I2C_MST_CTR), I2C_DMA_EN);
			gm_ClearRegBitsWord (I2CMasterReg (I2C_MST_TXR_CTRL), I2C_STA );

			//program source pointer
			WRITE_PCB_REG(D1SRCH, (WORD)((DWORD)(BYTE*)Bp_Buffer >> 16)); // 
			WRITE_PCB_REG(D1SRCL, (WORD)(BYTE*)Bp_Buffer);

			//program destination pointer
			WRITE_PCB_REG(D1DSTH, (WORD) 0);
			WRITE_PCB_REG(D1DSTL, I2C_MST_TXR_CTRL+1); 		// to setup a I2C_TX_DATA BYTE address

			//program how many bytes to transfer
			gm_WriteRegWord (I2CMasterReg (I2C_MST_DMA_CNTR), B_Length ); 	// the nuber of bytes to be transferred
			WRITE_PCB_REG(D1TC, (WORD)B_Length ); 			// the nuber of bytes to be transferred
			
			W_DMACtrl = (WORD)((WORD)DMA_DEST_POINTER_IS_MEMORY |
					DMA_DEST_POINTER_NOT_DEC | DMA_DEST_POINTER_NOT_INC | 
					DMA_SRC_POINTER_IS_MEMORY |
					DMA_SRC_POINTER_NOT_DEC | DMA_SRC_POINTER_INC |
					DMA_TC_TERMINATE | DMA_INT_DISABLE_ON_TERM |
					DMA_DEST_SYNC | DMA_LOW_PRIORITY | DMA_DISABLE_TIMER_REQ |
					DMA_ENABLE_STOP_BIT_UPDATE | DMA_START_OPERATIONS | DMA_BYTE_TRANSFERS); // 0x9686

			// Start DMA
			WRITE_PCB_REG(D1CON, W_DMACtrl); // Set Start/Stop bit to initiate DMA
		}

		while ( READ_PCB_REG( D1TC ) );
		while( gm_ReadRegWord (I2CMasterReg (I2C_MST_STATUS)) & BUSY);

		gm_SetRegBitsWord( DMA_REQUEST_CTRL, OCM_DRQ1_SEL );

		return gmd_PASS;
	}

	else

#endif
	{
		gmt_RET_PASSFAIL result;

		//
		// Regular FW (byte by byte) access to I2C Master
		//
		
		result = Lpm_I2CMasterStart(B_DevAddr, gmd_2WIRE_WR);

		// SEND Slave write sub-address(if persist) and than data
		while ((result == gmd_PASS) && (B_Length--))    
		  result = Lpm_I2CMasterPutByte(Bp_Buffer++);   

		if(B_Stop == STOP_BIT) Lpm_I2CMasterStop();

		return result;
	}
}

//******************************************************************************
//
// FUNCTION       :  gmp_Read2WireBlock(BYTE B_DevAddress, BYTE *Bp_Buffer,
//                WORD W_Len, gmt_2WIRE_STOP B_Stop);
//
// USAGE          :	Reads one or more bytes from the buffer to the 2wire device
//
// INPUT          : B_Port
//                  B_DevAddr - Address of the Device from where the data
//						  needs to be read.
//                  Bp_Buffer	- Buffer that contains the data read
//						  from the device.
//                  B_Length	 - Number of Bytes that need to be read.
//                  B_Stop		- Stop Condition.
//						TRUE indicates to send a Stop Condition
//						at the end of the packet.
//						FALSE indicates do not send a Stop
//						Condition at the end of the packet.
//
// OUTPUT         : gmt_RET_PASSFAIL : Status of the Read function.
//                  gmd_PASS - Read was successful
//                  gmd_FAIL - Read was not successful.
//
// GLOBALS        :  None
//
// USED_REGS      :  None
//
// PRE_CONDITION  :   
//
// POST_CONDITION :   
//
//******************************************************************************
gmt_RET_PASSFAIL Lpm_I2CMasterReadBlock(gmt_2WIRE_RING B_Port, BYTE B_DevAddr, BYTE far*Bp_Buffer,
                  BYTE B_Length, gmt_2WIRE_STOP B_Stop)
{
	gmt_RET_PASSFAIL result;

	B_Port--;					// Dummy to be implement

	// Validate input parameters
	if ((Bp_Buffer == 0) || (B_Length == 0)) return gmd_FAIL;

#ifdef _2WIRE_DMA_RD	

	//
	// use DMA for writing to Hudson 
	//
	
	if ((B_DevAddr == 0x6C)) // B_DevAddr == 0x6C
	{
		//
		// DMA access to I2C Master
		//
		{
			WORD W_DMACtrl;

			gm_ClearRegBitsWord(DMA_REQUEST_CTRL, OCM_DRQ1_SEL);
			gm_SetRegBitsWord( DMA_REQUEST_CTRL, OCM_DRQ1_SEL_I2C_MST ); // 1001 - I2C Master

			// Start bit for reading
			Lpm_I2CMasterStart(B_DevAddr, gmd_2WIRE_RD);

			// set DMA-related registers
			gm_SetRegBitsWord (I2CMasterReg (I2C_MST_CTR), I2C_DMA_EN);
			gm_ClearRegBitsWord (I2CMasterReg (I2C_MST_TXR_CTRL), I2C_STA | I2C_STO | I2C_ACK | I2C_WR );

			//program source pointer
			WRITE_PCB_REG(D1SRCH, (WORD) 0); // 
			WRITE_PCB_REG(D1SRCL, I2C_MST_STATUS+1); // I2C_RX_DATA is a high byte of I2C_MST_STATUS word

			//program destination pointer
			WRITE_PCB_REG(D1DSTH, (WORD)((DWORD)(BYTE*)Bp_Buffer >> 16));
			WRITE_PCB_REG(D1DSTL, (WORD)(BYTE*)Bp_Buffer); 

			//program how many bytes to transfer
			gm_WriteRegWord (I2CMasterReg (I2C_MST_DMA_CNTR), B_Length ); // the nuber of bytes to be transferred
			WRITE_PCB_REG(D1TC, (WORD)B_Length );

			W_DMACtrl = (WORD)((WORD)DMA_DEST_POINTER_IS_MEMORY |
					DMA_DEST_POINTER_NOT_DEC | DMA_DEST_POINTER_INC | 
					DMA_SRC_POINTER_IS_MEMORY |
					DMA_SRC_POINTER_NOT_DEC | DMA_SRC_POINTER_NOT_INC |
					DMA_TC_TERMINATE | DMA_INT_DISABLE_ON_TERM |
					DMA_SRC_SYNC | DMA_LOW_PRIORITY | DMA_DISABLE_TIMER_REQ |
					DMA_ENABLE_STOP_BIT_UPDATE | DMA_START_OPERATIONS | DMA_BYTE_TRANSFERS); // 

			// Start DMA
			WRITE_PCB_REG(D1CON, W_DMACtrl); // Set Start/Stop bit to initiate DMA
		}

		while ( READ_PCB_REG( D1TC ) );
		while( gm_ReadRegWord (I2CMasterReg (I2C_MST_STATUS) ) & BUSY);

		gm_SetRegBitsWord( DMA_REQUEST_CTRL, OCM_DRQ1_SEL );

		if(B_Stop == STOP_BIT) Lpm_I2CMasterStop();

	}

	else
#endif
	{
		//
		// Regular FW (byte by byte) access to I2C Master
		//
		result = Lpm_I2CMasterStart(B_DevAddr, gmd_2WIRE_RD);

		//If the ACK is Rxd,Start Address & Data Transmission
		while ((result == gmd_PASS) && (B_Length--))
			result = Lpm_I2CMasterGetByte(((B_Length == 0) ? (LPM_I2C_M_ACK | LPM_I2C_M_RD) : (LPM_I2C_M_RD)), 
				Bp_Buffer++);
		
		if(B_Stop == STOP_BIT)
			Lpm_I2CMasterStop();

	}

	return result;
}

/*******************************************************************************
* Extended functions to support Master 2Wire, Clock Frequency, Ring Switching  *
*******************************************************************************/

// before Hudson message
// 	Lpm_I2CMasterConfig( MASTER_2WIRE_1, I2C_MST_CLK_HIGH_FREQ, RING1 );

// before Hudson Power up message
// 	Lpm_I2CMasterConfig( MASTER_2WIRE_1, I2C_MST_CLK_SCALE_20kHz, RING1 );

// before Hudson ImmWrite(), ImmRead()
// 	Lpm_I2CMasterConfig( MASTER_2WIRE_1, I2C_MST_CLK_HIGH_FREQ, RING1 );


// after Hudson message
// 	Lpm_I2CMasterConfig( MASTER_2WIRE_1, I2C_MST_CLK_SCALE_100kHz, RING_ALL );


//******************************************************************************
//
// FUNCTION       : gmt_RET_PASSFAIL Lpm_I2CMasterConfig(gmt_2WIRE_MASTER_IDX B_2WireMaster, WORD W_2WireFreq, gmt_2WIRE_RING B_Port)
//
// USAGE          : Configure the 2wire device
//
// INPUT          : B_2WireMaster - Master 12C HW block number
//                  W_2WireFreq - divider for I2C_MST_CLK_SCALE
//                  B_Port - I2C Ring enable enumerator
//
// OUTPUT         : gmt_RET_PASSFAIL - Status of the Read function
//                  gmd_PASS - Read was successful
//                  gmd_FAIL - Read was not successful
//
// GLOBALS        : None
//
// USED_REGS      : None
//
// PRE_CONDITION  : 
//                  
//
// POST_CONDITION : 
//                  
//
//******************************************************************************
gmt_RET_PASSFAIL Lpm_I2CMasterConfig(gmt_2WIRE_MASTER_IDX B_2WireMaster, WORD W_2WireFreq, gmt_2WIRE_RING B_Port)
{
	// Validate input parameters
	if (W_2WireFreq == 0) return gmd_FAIL;

	B_2WireMaster++; // dummy

	// set up clock frequency
#ifdef _2WIRE_VAR_CLK
	gm_WriteRegWord( I2CMasterReg (LPM_I2C_MST_CLK_SCALE), W_2WireFreq );
#endif

	// ring switching
#ifdef _2WIRE_RING
	switch (B_Port) 
	{
		case RING_ALL:
			gm_WriteRegWord (I2CMasterReg (I2C_MST_CTR), 
				I2C_SDA_SCL0_EN | I2C_SDA_SCL1_EN | I2C_SDA_SCL2_EN | I2C_EN);
			break;
		case RING0:
			gm_WriteRegWord (I2CMasterReg (I2C_MST_CTR), I2C_SDA_SCL0_EN | I2C_EN);
			break;
		case RING1:
			gm_WriteRegWord (I2CMasterReg (I2C_MST_CTR), I2C_SDA_SCL1_EN | I2C_EN);
			break;
		case RING2:
			gm_WriteRegWord (I2CMasterReg (I2C_MST_CTR), I2C_SDA_SCL2_EN | I2C_EN);
			break;
		case RING_NOCHANGES:
		default:
			break;
	}
#endif

	return gmd_PASS;

}

//******************************************************************************
//
// FUNCTION       : static WORD	I2CMasterReg (WORD W_RegAddr)
//
// USAGE          : Returns the register's address taking into account
//					the index previously selected and the pre-defined offset
//
// INPUT          : W_RegAddr - register address (engine 0)
//
// OUTPUT         : WORD - the register's address qualified by 
//					the index previously selected and the offset
//
// GLOBALS        : None
//
// USED_REGS      : None
//
// PRE_CONDITION  : 
//                  
// POST_CONDITION : 
//                  
//
//******************************************************************************
static WORD	I2CMasterReg (WORD W_RegAddr)
{
	//return ((B_2WireIdx * IDX_OFFSET) + W_RegAddr);
	return W_RegAddr;
}

//******************************************************************************
// FUNCTION       : void	Lpm_I2CMasterSelectIdx (gmt_2WIRE_MASTER_IDX B_Idx)
// USAGE          : Select the I2C Master engine (only for FLI8668 core)
// INPUT          : B_Idx - 0 or 1 selctor for the I2C Master engine
// OUTPUT         : None
// GLOBALS        : B_2WireIdx - holds the selected I2C Master engine index
// USED_REGS      : None
// PRE_CONDITION  :              
// POST_CONDITION :                
//******************************************************************************
void Lpm_I2CMasterSelectIdx (gmt_2WIRE_MASTER_IDX B_Idx)
{
#if 0//(FEATURE_2WIRE_NUM == 2)
	B_2WireIdx = B_Idx;
#else
	B_Idx = B_Idx;
#endif //(FEATURE_2WIRE_NUM == 2)
	
}

//******************************************************************************
// FUNCTION       : void Lpm_I2CMasterInit (void)
// USAGE          : I2c master initialization
// INPUT          :  None
// OUTPUT         : None
// GLOBALS        : 
// USED_REGS      : None
// PRE_CONDITION  :              
// POST_CONDITION :                
//******************************************************************************
void Lpm_I2CMasterInit(void)
{
   gm_WriteRegWord(LPM_I2C_MST_CTR, LPM_I2C_M_EN);
   gm_SetRegBitsWord(LPM_MICRO_CONTROL,I2C_MASTER_RESET);
   gm_ClearRegBitsWord(LPM_MICRO_CONTROL,I2C_MASTER_RESET);
   Lpm_I2CMasterSetClock();
   gm_SetRegBitsByte(I2C_SLAVE1_CTRL1, I2C_SLAVE1_EN);
   #if DDC1_FOR_LPM_I2C_MST_ONLY
   Lpm_SharedPBusSwitch(BUS_LPM);
   gm_SetRegBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_I2C_M_SEL);
   if(Lpm_PowerModeGetState() == LPM_NormalPowerMode)
      Lpm_SharedPBusSwitch(BUS_MISSION); 
   #endif
   //Lpm_InterruptEnable(gmd_I2CM);
}

//******************************************************************************
// FUNCTION       : void Lpm_I2CMasterCsmMux (void)
// USAGE          : I2c master CSM Mux select
// INPUT          :  None
// OUTPUT         : None
// GLOBALS        : 
// USED_REGS      : None
// PRE_CONDITION  :              
// POST_CONDITION :                
//******************************************************************************
void Lpm_I2CMasterCsmSel(BYTE I2cMstSel)
{
   #if DDC1_FOR_LPM_I2C_MST_ONLY
   UNUSED_VAR(I2cMstSel);
   return;

   #else //DDC1 pins switched between I2C slave1 & LPM I2C master
   Lpm_SharedPBusSwitch(BUS_LPM);
   
   if(I2cMstSel)
      gm_SetRegBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_I2C_M_SEL);
   else
      gm_ClearRegBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_I2C_M_SEL);

   if(Lpm_PowerModeGetState() == LPM_NormalPowerMode)
      Lpm_SharedPBusSwitch(BUS_MISSION);
   #endif
}

//******************************************************************************
// FUNCTION       : void Lpm_I2CMasterSetClock (void)
// USAGE          : I2c master clock scale setting
// INPUT          :  None
// OUTPUT         : None
// GLOBALS        : 
// USED_REGS      : None
// PRE_CONDITION  :              
// POST_CONDITION :                
//******************************************************************************
void Lpm_I2CMasterSetClock(void)
{
   WORD I2cPreScale = (WORD)I2C_100K_SCALE;
   if(((LpmOcmClk / I2C_100K_FREQ) / 5) <= 2) //SCL clock limitation due to LPM OCM clock divided
      I2cPreScale = 0;                        //In this case, I2C clock < 100KHz
   gm_WriteRegWord(LPM_I2C_MST_CLK_SCALE, I2cPreScale);
}

#endif	//#ifdef I2C_MASTER_SUPPORT
//******************************************************************************

