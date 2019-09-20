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
// MODULE:      gm_LPM_IPC.c
//
// USAGE:       Implement the Low Power Monitor Inter-Processor Communication
//
//******************************************************************************

//******************************************************************************
//  I N C L U D E    F I L E S
//******************************************************************************
#include <mem.h>
#include ".\LibInc\gm_Global.h"
#include ".\LibInc\gm_HostReg.h"
#include ".\LibInc\gm_Mcu186.h"
#include ".\LibInc\gm_Register.h"
#include ".\LibInc\gm_LPM_IPC.h"

#ifdef LPM_SIDE
extern void gm_Print (const char far *, WORD);
#else
extern void EXT_FUNC gm_Print (const char far *, WORD);
#endif

BYTE REG_MSG_CONTENT[REG_MSG_BUF];
BYTE REG_MSG_PTR;
BYTE REG_MSG_TOP;
WORD RX_MONITOR;

WORD BACK_TX;
WORD BACK_RX;


//******************************************************************************
//  L O C A L    D E F I N I T I O N S
//******************************************************************************
typedef enum
{
	gmd_SEND_RESUME = 0,
	gmd_SEND_START = BIT0,
	gmd_SEND_STOP = BIT1,
} gmt_SEND_MODE;

//remove warning "Bit fields must be signed or unsigned int"
#pragma warn -bbf

typedef struct
{
	BYTE B_SourceId:4;
	BYTE B_Length:2;
	BYTE B_Mode:2;
} gmt_MESSAGE_HEADER;

typedef struct
{
	gmt_MESSAGE_HEADER MessageHeader;
	BYTE Byte0;
	BYTE Byte1;
	BYTE Byte2;
} gmt_MESSAGE;

#define ERR_QUEUE_ID 0xFF

//******************************************************************************
//  S T A T I C
//******************************************************************************
static gmt_QUEUE far * Queue;
static WORD (far* gFp_GetTime)(void);
static WORD W_TimeoutStatic;
static BYTE B_NumOfQueuesStatic;
static WORD W_QueueBufferSizeStatic;
volatile BYTE dateCounter;//indicates "date" of next message-queue

//lock down the QueueId OldestQueueId that gm_MessageReceived() is checking so that 
// ISR cannot choose it for overwriting. 
static BYTE B_QueueIdLock;		

//******************************************************************************
//  C O D E
//******************************************************************************
//******************************************************************************
//
// FUNCTION:	void gm_MessageInIsr(void)
//
// USAGE:		Called when there is incoming data through mail-box registers
//				This function depends on the caller function to perform all the interrupt
//				handling, including the clearing the status bit.
//
// INPUT:		None
//
// OUTPUT:		None
//
// GLOBALS:		None
//
// USED_REGS:	Mission side:
//				MISSION_MICRO_CONTROL_IN	(R)
//				MISSION_MICRO_MAIL_IN		(R)
//
//				LPM side:
//				LPM_MICRO_CONTROL_IN		(R)
//				LPM_MICRO_MAIL_IN			(R)
//
// PRE_CONDITION: 
//
// POST_CONDITION: 
//
//******************************************************************************
void far gm_MessageInIsr(void)
{
	gmt_MESSAGE Message;
    BYTE B_EmptyIndex;
    gmt_QUEUE far* queuePtr = 0;
	
	*((WORD *)&Message.Byte1) = gm_ReadRegWord(CONTROL_IN);
	*((WORD *)&Message.MessageHeader) = gm_ReadRegWord(MAIL_IN);

    if ((Message.MessageHeader.B_Mode & gmd_SEND_START) == 0) //continue long message loading 
    {
        BYTE queue = B_NumOfQueuesStatic;    	
    	while (queue--)
    	{
    		if ((Queue[queue].QueueInfo.B_State == gmd_IPC_QUEUE_LOADING) 
                && (Queue[queue].QueueInfo.B_SourceId == Message.MessageHeader.B_SourceId)) 
            {
                queuePtr = &Queue[queue];
                break;
            }
    	}	
        if (!queuePtr)
            return;	
        B_EmptyIndex = queuePtr->B_EmptyIndex;
    }
    else //new message arrived - to find queue for it
    {
    	BYTE queue = B_NumOfQueuesStatic;    	
    	BYTE queueOldest = ERR_QUEUE_ID;
    	BYTE expiration = 0;//represents uninitialized    	
    	while (queue--)
    	    if (queue != B_QueueIdLock)
    	{
    		gmt_QUEUE_INFO QueueInfo = Queue[queue].QueueInfo;

    		if (QueueInfo.B_State == gmd_IPC_QUEUE_FREE) 
            {
                queuePtr = &Queue[queue];
                break;                
            }                
    		{   //find oldest queue if no free queue will be found
                BYTE e = dateCounter - QueueInfo.date;
                if ((queueOldest == ERR_QUEUE_ID) || (e > expiration))	
                {
                    expiration = e;
                    queueOldest = queue;
                }                
    		}
    	}        
    	if (!queuePtr) //if went through all queues and cannot find an available queue
    	{
    	    if (queueOldest == ERR_QUEUE_ID)
                return;
            else
                queuePtr = &Queue[queueOldest]; //overwrite the queue with the oldest message   
        }
		queuePtr->QueueInfo.B_SourceId = Message.MessageHeader.B_SourceId;
		queuePtr->QueueInfo.date = dateCounter++;	
        B_EmptyIndex = 0;
    }    
		
    {   //check queue length to prevent buffer overrun and force complete
        BYTE length = W_QueueBufferSizeStatic - B_EmptyIndex; //first calculate maximum available 
    	if (Message.MessageHeader.B_Length > length)
    	    queuePtr->QueueInfo.B_State = gmd_IPC_QUEUE_COMPLETE;        
    	else
    	{
    		length = Message.MessageHeader.B_Length;
    		queuePtr->QueueInfo.B_State = (Message.MessageHeader.B_Mode & gmd_SEND_STOP) ? 
                gmd_IPC_QUEUE_COMPLETE : gmd_IPC_QUEUE_LOADING;
    	}
    	
        *(DWORD*)(&queuePtr->Bp_Buffer[B_EmptyIndex]) = *(DWORD*)(&Message.Byte0);
        queuePtr->B_EmptyIndex = B_EmptyIndex + length;
    }
}

//******************************************************************************
//
// FUNCTION:	void gm_MessageInit(gmt_QUEUE far * QueuePtr, BYTE B_NumOfQueues,
//									BYTE far * B_Buffer, WORD W_TotalBufferSize,
//									WORD W_Timeout, WORD (far* GetTimePtr)(void))
//
// USAGE:		Initialize software settings for mail-box communication.
//
// INPUT:		QueuePtr - pointer to buffer for storing queue header info
//							on the receiving side
//				B_NumOfQueues - number of receiving queues
//				Bp_Buffer - pointer to buffer for storing messages on the
//							receiving side
//				W_TotalBufferSize - total size of buffer for storing messages
//									on the receiving side
//				W_Timeout - max wait time (in ms) for the sending side before giving
//							up sending operation
//				GetTimePtr - pointer to function that returns current system time
//
// OUTPUT:		None
//
// GLOBALS:		None
//
// USED_REGS:	None
//
// PRE_CONDITION:
//
// POST_CONDITION:
//
//******************************************************************************
void far gm_MessageInit(gmt_QUEUE far * QueuePtr, BYTE B_NumOfQueues,
  void far* Bp_Buffer, WORD W_TotalBufferSize, WORD W_Timeout, WORD (far* GetTimePtr)(void))
{
	BYTE B_Index;
    B_QueueIdLock = ERR_QUEUE_ID;
    dateCounter = 0;
	W_QueueBufferSizeStatic = W_TotalBufferSize/B_NumOfQueues;
	B_NumOfQueuesStatic = B_NumOfQueues;
	Queue = QueuePtr;
	for (B_Index = 0; B_Index < B_NumOfQueuesStatic; B_Index++)
	{
		Queue[B_Index].QueueInfo.B_State = gmd_IPC_QUEUE_FREE;
		Queue[B_Index].Bp_Buffer = Bp_Buffer;
		Bp_Buffer = (BYTE far*)Bp_Buffer + W_QueueBufferSizeStatic;
	}

	W_TimeoutStatic = W_Timeout;
	gFp_GetTime = GetTimePtr;
}

//******************************************************************************
//
// FUNCTION:	BOOL gm_MessageSend(gmt_MESSAGE_SOURCE B_SourceId,
//									void far * B_Buffer, WORD W_Length)
//
// USAGE:		Break up message into small chucks to be sent over through the
//				mail-box mechanism
//
// INPUT:		B_SourceId - sender ID
//				B_Buffer - pointer to buffer which contains data to be sent
//				W_Length - length of message in bytes
//
// OUTPUT:		TRUE if complete message is sent successfully; otherwise FALSE
//
// GLOBALS:		None
//
// USED_REGS:	None
//
// PRE_CONDITION: 
//
// POST_CONDITION: 
//
//******************************************************************************
BOOL far gm_MessageSend(gmt_MESSAGE_SOURCE B_SourceId, void far* B_Buffer, WORD W_Length)
{
	BYTE far* const B_BufferStart = B_Buffer;
	BYTE far* const B_BufferEnd = (BYTE far*)B_Buffer + W_Length;
	gmt_MESSAGE MessageTemp;
	MessageTemp.MessageHeader.B_SourceId = B_SourceId;

	while (B_Buffer < B_BufferEnd)
	{
        {
		    WORD W_SavedTime;
    		if (gFp_GetTime)
	    		W_SavedTime = (*gFp_GetTime)();
		
    		while (gm_ReadRegWord(IRQ_STATUS) & PENDING_WRITE)
    	        if (gFp_GetTime && (((*gFp_GetTime)() - W_SavedTime) >= W_TimeoutStatic))
    				return FALSE;			
        }        
		MessageTemp.MessageHeader.B_Mode = (B_Buffer == B_BufferStart) ? gmd_SEND_START : gmd_SEND_RESUME;
        MessageTemp.MessageHeader.B_Length = 3;
        if (B_Buffer >= B_BufferEnd - 3) //last packet
        {
            MessageTemp.MessageHeader.B_Mode |= gmd_SEND_STOP;
            MessageTemp.MessageHeader.B_Length = (BYTE)(B_BufferEnd-B_Buffer);
        }
        MessageTemp.Byte0 = *((BYTE far*)B_Buffer)++;
        if (B_Buffer < B_BufferEnd)
        {
            MessageTemp.Byte1 = *((BYTE far*)B_Buffer)++;
            if (B_Buffer < B_BufferEnd)
                MessageTemp.Byte2 = *((BYTE far*)B_Buffer)++;
    		gm_WriteRegWord(CONTROL_OUT, *(WORD*)(&MessageTemp.Byte1));//Byte2 and Byte1 combined
	    }
        gm_WriteRegWord(MAIL_OUT, *(WORD*)(&MessageTemp.MessageHeader));//Byte0 and header combined
	}
	return TRUE;
}

//******************************************************************************
//
// FUNCTION:	BOOL gm_MessageReceived(gmt_MESSAGE_SOURCE B_SourceId,
//								        void far * Bp_Buffer, WORD W_BufferSize)
//
// USAGE:		Analyze all storage queues and return an available one
//
// INPUT:		B_SourceId - source ID
//				Bp_Buffer - pointer to buffer to which the complete message is copied
//				W_BufferSize - size of buffer
//
// OUTPUT:		Length of message; max length is W_BufferSize
//
// GLOBALS:		None
//
// USED_REGS:	None
//
// PRE_CONDITION:
//
// POST_CONDITION:
//
//******************************************************************************
WORD far gm_MessageReceived(gmt_MESSAGE_SOURCE B_SourceId, 
    void far * Bp_Buffer, WORD W_BufferSize)
{
    while (1) 
    {
        BYTE expiration = 0;//represents uninitialized 
        BYTE queueOldest = ERR_QUEUE_ID;
        BYTE queue = B_NumOfQueuesStatic;        
        BYTE chkDateCounter = dateCounter; //syncronization with ISR, to indicate that ISR updated queues 
        while (queue--)
    	{
    		gmt_QUEUE_INFO QueueInfo = Queue[queue].QueueInfo;
    		if ((QueueInfo.B_State == gmd_IPC_QUEUE_COMPLETE) && (QueueInfo.B_SourceId == B_SourceId))
    		{   //find oldest message
                BYTE e = chkDateCounter - QueueInfo.date;
                if ((queueOldest == ERR_QUEUE_ID) || (e > expiration))	
                {
                    expiration = e;
                    queueOldest = queue;
        		}
    		}		
    	}
        //	lock down this queue when we check its status; otherwise, ISR
        //  may overwrite it and we will be checking obsolete data.        
        B_QueueIdLock = queueOldest;
        if (chkDateCounter == dateCounter) //if ISR has updated queue during loop, start again
            break; 
    }

    if (B_QueueIdLock != ERR_QUEUE_ID)
	{
        WORD length = 0;	
	    if (Queue[B_QueueIdLock].B_EmptyIndex > 0)
		{
		    length = Queue[B_QueueIdLock].B_EmptyIndex;
    		if (length > W_BufferSize)
                length = W_BufferSize;
    		_fmemcpy(Bp_Buffer, Queue[B_QueueIdLock].Bp_Buffer, length);
        }
		Queue[B_QueueIdLock].QueueInfo.B_State = gmd_IPC_QUEUE_FREE;
        B_QueueIdLock = ERR_QUEUE_ID;
        return length;
	}
   	return 0;
}

#define ATHENA_CEC_21
#ifdef ATHENA_CEC_21
#undef gm_ReadRegWord(x)
#define gm_ReadRegWord(x) ((((WORD)gm_ReadRegByte(x+1))<<8)|((WORD)gm_ReadRegByte(x)))
#endif

void far gm_RegMsgInit(void)
{
	BYTE msg_size;
	BYTE buf_size;

	REG_MSG_PTR = 0;
	REG_MSG_TOP = 0;
	RX_MONITOR = 0;
	BACK_TX = 0;
	BACK_RX = 0;

//	gm_Print("Reg Message Init", 0);
	for (msg_size = 20; msg_size < REG_MSG_NUM; msg_size++)
	{
		gm_WriteRegWord(REG_MSG_OUT + 2*msg_size, 0);
	}
	for (buf_size = 0; buf_size < REG_MSG_BUF; buf_size++)
			REG_MSG_CONTENT[buf_size] = 0;
}

BOOL far gm_RegMsgSd(gmt_MESSAGE_SOURCE B_SourceId, void far* B_Buffer, WORD W_Length)
{

	BACK_TX++;
	//gm_Print("BACK_TX = %d", BACK_TX);
	if (W_Length > REG_MSG_SIZE)
	{
		//gm_Print("### REG MSG SIZE OVERFLOW ###", 0);
		return FALSE;
	}
	if ((W_Length + REG_MSG_TOP) > REG_MSG_BUF)
	{
	//	gm_Print("### REG MSG BUF OVERFLOW ###", 0);
		return FALSE;
	}

	REG_MSG_CONTENT[REG_MSG_TOP] = B_SourceId;
	REG_MSG_TOP++;
	REG_MSG_CONTENT[REG_MSG_TOP] = (BYTE)W_Length;
	REG_MSG_TOP++;
	_fmemcpy(&(REG_MSG_CONTENT[REG_MSG_TOP]), B_Buffer, W_Length);
	REG_MSG_TOP += W_Length;

	return TRUE;
}


WORD far gm_RegMsgRv(gmt_MESSAGE_SOURCE B_SourceId, void far * Bp_Buffer)
{
	WORD rx_num = gm_ReadRegWord(REG_MSG_OUT_RX);
	WORD rx_cmd = (WORD)B_SourceId;
	WORD rx_len;
	WORD len;
	
	if (rx_cmd != gm_ReadRegWord(REG_MSG_IN_CMD))
	{
		return 0;
	}
	
	if (gm_ReadRegWord(REG_MSG_IN_TX) == rx_num)
	{
		return 0;		//no new command
	}

	if (BACK_RX == 0)
	{
		if (gm_ReadRegWord(REG_MSG_IN_TX) != 1)
			return 0;			//This is for Mission wake up checking
	}
	BACK_RX++;
	//gm_Print("BACK_RX = %d", BACK_RX);

	rx_len = gm_ReadRegWord(REG_MSG_IN_LEN);
	
	for (len = 0; len < rx_len; len++) 
	{
		((BYTE *)Bp_Buffer)[len] = (BYTE)gm_ReadRegWord(REG_MSG_IN_START + 2*len);
		//gm_Print("Receive = 0x%x", gm_ReadRegWord(REG_MSG_IN_START + 2*len));
	}
	gm_WriteRegWord(REG_MSG_OUT_RX, rx_num + 1);
	return rx_len;
	
}

void RegMsgUpdate(void)
{
	WORD cmd_len;
	WORD len;
	WORD tx_num = gm_ReadRegWord(REG_MSG_OUT_TX);
	
	gm_WriteRegWord(REG_MSG_OUT_CMD, REG_MSG_CONTENT[REG_MSG_PTR]);
	REG_MSG_PTR++;
	gm_WriteRegWord(REG_MSG_OUT_LEN, REG_MSG_CONTENT[REG_MSG_PTR]);
	cmd_len = REG_MSG_CONTENT[REG_MSG_PTR];
	REG_MSG_PTR++;
	for (len = 0; len < cmd_len; len++)
	{
		gm_WriteRegWord(REG_MSG_OUT_START + 2*len, REG_MSG_CONTENT[REG_MSG_PTR]);
		//gm_Print("SEND = 0x%x", REG_MSG_CONTENT[REG_MSG_PTR]);
		REG_MSG_PTR++;
	}	
	gm_WriteRegWord(REG_MSG_OUT_TX, tx_num + 1);
}

void far RegMsgHandler(void)
{
	WORD tx_num = gm_ReadRegWord(REG_MSG_OUT_TX);
	WORD rx_num_remote = gm_ReadRegWord(REG_MSG_IN_RX);

	if (rx_num_remote > BACK_TX)
		return;			//This is for Mission wake up checking
	
	if (BACK_TX > tx_num) { 
		if (rx_num_remote == RX_MONITOR)
		{
			if (tx_num == rx_num_remote)
			{
				RegMsgUpdate();
				return;
			}
		}
		if (rx_num_remote > RX_MONITOR)
		{
		//	gm_Print("@@@ remote receive", 0);
			RX_MONITOR++;
			RegMsgUpdate();
		}
	}
	else
	{
		if (rx_num_remote > RX_MONITOR)	
		{
			RX_MONITOR++;
		//	gm_Print("^^^ remote receive", 0);
			REG_MSG_PTR = 0;
			REG_MSG_TOP = 0;
		}
	}
}
