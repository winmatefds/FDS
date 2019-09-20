//============================================================================= 
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
//   MODULE : Lpm_AuxRxI2C.c 
// 
//   USAGE  : 
// 
//   OWN    : 
// 
//=============================================================================

//****************************************************************************** 
//
//  I N C L U D E    F I L E S 
//
//****************************************************************************** 
#include ".\LowPowerMonitor\Lpm_All.h"


#ifdef AUX_RX_SUPPORT

//****************************************************************************** 
//
//  L O C A L    D E F I N I T I O N S 
//
//****************************************************************************** 

//****************************************************************************** 
//
//  G L O B A L   V A R I A B L E S 
//
//****************************************************************************** 
TAuxRxI2C_InternalData* P_AuxRxI2CInternalData = NULL_PTR;
SW_TransactionStruct_t* pSW_Transaction_ModuleData = NULL_PTR;
gmt_DPRX_AUX_I2C_INTERNAL_DATA* P_AuxRxI2CModuleData = NULL_PTR;
#ifdef AUX_RX_C2_SUPPORT
TAuxRxI2C_InternalData* P_AuxRxI2CInternalData2 = NULL_PTR;
SW_TransactionStruct_t* pSW_Transaction_ModuleData2 = NULL_PTR;
gmt_DPRX_AUX_I2C_INTERNAL_DATA* P_AuxRxI2CModuleData2 = NULL_PTR;
#endif

//****************************************************************************** 
//
//  C O D E 
//
//****************************************************************************** 
//***************************************************************************** 
// FUNCTION     : WORD Lpm_AuxRxI2CSwTransInit(BYTE* PB_StartRAM) 
// USAGE        : Initialize I2C SW transaction module data 
// INPUT        : PB_StartRAM -- address in RAM from where internal data is be allocated;
//                NULL_PTR for request for size of SW_TransactionStruct_t 
// OUTPUT       : number of bytes for SW_TransactionStruct_t 
// GLOBALS      : SW_Transaction_ModuleData 
// USED_REGS    : 
//***************************************************************************** 
WORD Lpm_AuxRxI2CSwTransInit(BYTE* PB_StartRAM, AUX_CH B_AuxCh)
{
    if(PB_StartRAM != NULL_PTR)
    {
		if(B_AuxCh == AUX_CH_C0)
		{    
	        pSW_Transaction_ModuleData = (SW_TransactionStruct_t *)PB_StartRAM;
	        pSW_Transaction_ModuleData->B_Dir = 0;
	        pSW_Transaction_ModuleData->B_SWDevIdCount = 0;
		}
		#ifdef AUX_RX_C2_SUPPORT
		else if(B_AuxCh == AUX_CH_C2)
		{
	        pSW_Transaction_ModuleData2 = (SW_TransactionStruct_t *)PB_StartRAM;
	        pSW_Transaction_ModuleData2->B_Dir = 0;
	        pSW_Transaction_ModuleData2->B_SWDevIdCount = 0;
		}
		#endif
    }

    return sizeof(SW_TransactionStruct_t);
}

//***************************************************************************** 
// FUNCTION     : BYTE Lpm_AuxRxI2CIsSwTransaction(void) 
// USAGE        : Check if I2C-AUX SW trasaction 
// INPUT        : DW_Addr - AUX address 
//                B_AuxCh - indicate which Aux channel
// OUTPUT       : TRUE/FALSE
// GLOBALS      : 
// USED_REGS    : 
//***************************************************************************** 
BOOL Lpm_AuxRxI2CIsSwTransaction(DWORD DW_Addr, AUX_CH B_AuxCh)
{
    BOOL  Bl_Result = FALSE;
    WORD  W_Count;
    if (AUX_I2C_IS_SW_TR(DW_Addr)) return TRUE;


	if(B_AuxCh == AUX_CH_C0)
	{  	
	    if (pSW_Transaction_ModuleData == NULL_PTR) return FALSE;

		for(W_Count = 0; W_Count < pSW_Transaction_ModuleData->B_SWDevIdCount; W_Count++)
	    {
	        if (AUXTX_I2C_HW_ADDR(DW_Addr) == pSW_Transaction_ModuleData->B_SWDevId[W_Count])
	        {
	            Bl_Result = TRUE;
	            break;
	        }
	    }
	}
	#ifdef AUX_RX_C2_SUPPORT
	else if(B_AuxCh == AUX_CH_C2)
	{
	    if (pSW_Transaction_ModuleData2 == NULL_PTR) return FALSE;

		for(W_Count = 0; W_Count < pSW_Transaction_ModuleData2->B_SWDevIdCount; W_Count++)
	    {
	        if (AUXTX_I2C_HW_ADDR(DW_Addr) == pSW_Transaction_ModuleData2->B_SWDevId[W_Count])
	        {
	            Bl_Result = TRUE;
	            break;
	        }
	    }	
	}
	#endif
	
    return Bl_Result;
}

//***************************************************************************** 
// FUNCTION     : void Lpm_AuxRxI2CAddSwDevId(BYTE B_I2CDevId, AUX_CH B_AuxCh)
// USAGE        : Add device ID for SW I2C-AUX handling
// INPUT        : B_I2CDevId - I2C device ID
//                B_AuxCh - indicate which Aux channel
// OUTPUT       :  
// GLOBALS      : 
// USED_REGS    : 
//***************************************************************************** 
void Lpm_AuxRxI2CAddSwDevId(BYTE B_I2CDevId, AUX_CH B_AuxCh)
{
	if(B_AuxCh == AUX_CH_C0)
	{ 
	    if (pSW_Transaction_ModuleData->B_SWDevIdCount < MAX_I2C_SW_ADDRESSES)
	    {
	        pSW_Transaction_ModuleData->B_SWDevId[pSW_Transaction_ModuleData->B_SWDevIdCount] = B_I2CDevId >> 1;
	        pSW_Transaction_ModuleData->B_SWDevIdCount++;
	    }
	}
	#ifdef AUX_RX_C2_SUPPORT
	else if(B_AuxCh == AUX_CH_C2)
	{
	    if (pSW_Transaction_ModuleData2->B_SWDevIdCount < MAX_I2C_SW_ADDRESSES)
	    {
	        pSW_Transaction_ModuleData2->B_SWDevId[pSW_Transaction_ModuleData2->B_SWDevIdCount] = B_I2CDevId >> 1;
	        pSW_Transaction_ModuleData2->B_SWDevIdCount++;
	    }	
	}
	#endif
}

//***************************************************************************** 
// FUNCTION     : void Lpm_AuxRxI2CClearSwDevIdTable(AUX_CH B_AuxCh)
// USAGE        : Clear all device ID of SW I2C-AUX handling
// INPUT        :  B_AuxCh - indicate which Aux channel
// OUTPUT       :  
// GLOBALS      : 
// USED_REGS    : 
//***************************************************************************** 
void Lpm_AuxRxI2CClearSwDevIdTable(AUX_CH B_AuxCh)
{
	if(B_AuxCh == AUX_CH_C0)
	    pSW_Transaction_ModuleData->B_SWDevIdCount = 0;
	#ifdef AUX_RX_C2_SUPPORT
	else if(B_AuxCh == AUX_CH_C2)	
		pSW_Transaction_ModuleData2->B_SWDevIdCount = 0;
	#endif
}

//***************************************************************************** 
// FUNCTION     : BYTE Lpm_AuxRxI2CSwHandler(WORD W_Address, BYTE B_Dir, BYTE *Bp_Buffer, WORD W_Length, BYTE B_Stop) 
// USAGE        : Is used by AuxRx.c to handle received chunk of I2C transactions 
// INPUT        : W_Address - received I2C address (possible whole word is used to let identify SW-created transaction) 
//                B_Dir - 0: Read / 1: Write 
//                Bp_Buffer - pointer to data buffer 
//                W_Length - data length in bytes: 0 is 0 bytes, 1 is 1 byte, etc 
//                B_Stop - 0: MOT / 1: Stop
//                B_AuxCh - indicate which Aux channel
// OUTPUT       : FALSE if transaction is failed or TRUE - if OK 
// GLOBALS      : 
// USED_REGS    : 
//***************************************************************************** 
BYTE Lpm_AuxRxI2CSwHandler(WORD W_Address, BOOL B_Dir, BYTE *Bp_Buffer, WORD W_Length, BYTE B_Stop, AUX_CH B_AuxCh)
{
    BYTE B_Count;
    BYTE B_Low_Address = ((W_Address & 0x7F) << 1);
    //B_Low_Address = B_Low_Address << 1; 

    //gm_Print("0x%X",B_Low_Address); 

	if(B_AuxCh == AUX_CH_C0)
	{
    if(pSW_Transaction_ModuleData->B_Dir != B_Dir)
    {
        //open new transaction 
        pSW_Transaction_ModuleData->W_CurCount = 0;
        pSW_Transaction_ModuleData->B_Dir = B_Dir;
        pSW_Transaction_ModuleData->W_SendDevID = 0;
        pSW_Transaction_ModuleData->B_DevID = (B_Low_Address & 0xFE);
    }

    if(B_Dir)
    {  // we have received data 

        for(B_Count = 0; B_Count < W_Length; B_Count++)
        pSW_Transaction_ModuleData->AB_Data[pSW_Transaction_ModuleData->W_CurCount++] = Bp_Buffer[B_Count];

        if(B_Stop)
        {
            BYTE  B_Len;

            pSW_Transaction_ModuleData->B_ReplyReady = FALSE;
            pSW_Transaction_ModuleData->B_NullMsgPtr = 0;

            // Check total packet length received 
            B_Len =  (~DDC2Bi_CONTROL_STATUS_FLAG) & pSW_Transaction_ModuleData->AB_Data[1]; // packet length 

            if(pSW_Transaction_ModuleData->W_CurCount >= (B_Len + 3))
            {                                                                                // Message is received 
                if(Lpm_I2CSlaveCheckCS(pSW_Transaction_ModuleData->B_DevID, B_Len, &pSW_Transaction_ModuleData->AB_Data[0]))
                {
                    //Bp_CurMsg = (BYTE*)pSW_Transaction_ModuleData->AB_Data;                    // Checksum is correct 
                    //Lpm_GprobeCmdParser(AUX_C0_USE); 
                    ExecuteMemTest = EXEC_MEM_DEBUG;
                }
                else
                {
                    Lpm_SendAckOrNack(SND_NAK, AUX_C0_USE, (BYTE far*)&pSW_Transaction_ModuleData->AB_Data[Start_of_GprobeCmd_in_DDC2BiMsg]);
					//Lpm_WriteCmdPkt(&NakPkt[0], AUX_C0_USE);
                }
            }
        }
    }
    else
    {
        if(W_Length == 0)
            return TRUE;

        if((pSW_Transaction_ModuleData->B_ReplyReady) && (pSW_Transaction_ModuleData->B_NullMsgPtr == 0))
        {
            if ((pSW_Transaction_ModuleData->W_CurCount == 0) && (pSW_Transaction_ModuleData->W_SendDevID == 0))
            {
                pSW_Transaction_ModuleData->W_SendDevID = 1;
                Bp_Buffer[0] = pSW_Transaction_ModuleData->B_DevID;
                B_Count = 1;
            }
            else
            {
                B_Count = 0;
            }

            for(; B_Count < W_Length; B_Count++)
            Bp_Buffer[B_Count] = pSW_Transaction_ModuleData->AB_Data[(pSW_Transaction_ModuleData->W_CurCount++)];

            if(B_Stop)
            {
                pSW_Transaction_ModuleData->B_ReplyReady = FALSE;
            }
        }
        else
        {
            for(B_Count = 0; B_Count < W_Length; B_Count++)
            {
                switch(pSW_Transaction_ModuleData->B_NullMsgPtr++)
                {
                    case 1: Bp_Buffer[B_Count] = NULL_MSG_SECOND_BYTE;
                        break;
                    case 2: Bp_Buffer[B_Count] = INIT_NULL_MSG_CS ^ pSW_Transaction_ModuleData->B_DevID;
                        break;
                    case 0:
                    default: Bp_Buffer[B_Count] = pSW_Transaction_ModuleData->B_DevID;
                        break;
                }

                if(pSW_Transaction_ModuleData->B_NullMsgPtr == NULL_MSG_LENGTH)
                    pSW_Transaction_ModuleData->B_NullMsgPtr = 0;
            }

            if(B_Stop)
            {
                pSW_Transaction_ModuleData->B_NullMsgPtr = 0;
            }
        }
    }
	}
	#ifdef AUX_RX_C2_SUPPORT
	else if(B_AuxCh == AUX_CH_C2)
	{
    if(pSW_Transaction_ModuleData2->B_Dir != B_Dir)
    {
        //open new transaction 
        pSW_Transaction_ModuleData2->W_CurCount = 0;
        pSW_Transaction_ModuleData2->B_Dir = B_Dir;
        pSW_Transaction_ModuleData2->W_SendDevID = 0;
        pSW_Transaction_ModuleData2->B_DevID = (B_Low_Address & 0xFE);
    }

    if(B_Dir)
    {  // we have received data 

        for(B_Count = 0; B_Count < W_Length; B_Count++)
        pSW_Transaction_ModuleData2->AB_Data[pSW_Transaction_ModuleData2->W_CurCount++] = Bp_Buffer[B_Count];

        if(B_Stop)
        {
            BYTE  B_Len;

            pSW_Transaction_ModuleData2->B_ReplyReady = FALSE;
            pSW_Transaction_ModuleData2->B_NullMsgPtr = 0;

            // Check total packet length received 
            B_Len =  (~DDC2Bi_CONTROL_STATUS_FLAG) & pSW_Transaction_ModuleData2->AB_Data[1]; // packet length 

            if(pSW_Transaction_ModuleData2->W_CurCount >= (B_Len + 3))
            {                                                                                // Message is received 
                if(Lpm_I2CSlaveCheckCS(pSW_Transaction_ModuleData2->B_DevID, B_Len, &pSW_Transaction_ModuleData2->AB_Data[0]))
                {
                    //Bp_CurMsg = (BYTE*)pSW_Transaction_ModuleData2->AB_Data;                    // Checksum is correct 
                    //Lpm_GprobeCmdParser(AUX_C2_USE); 
                    ExecuteMemTest2 = EXEC_MEM_DEBUG;
                }
                else
                {
                    Lpm_SendAckOrNack(SND_NAK, AUX_C2_USE, (BYTE far*)&pSW_Transaction_ModuleData2->AB_Data[Start_of_GprobeCmd_in_DDC2BiMsg]);
                    //Lpm_WriteCmdPkt(&NakPkt[0], AUX_C2_USE);
                }
            }
        }
    }
    else
    {
        if(W_Length == 0)
            return TRUE;

        if((pSW_Transaction_ModuleData2->B_ReplyReady) && (pSW_Transaction_ModuleData2->B_NullMsgPtr == 0))
        {
            if ((pSW_Transaction_ModuleData2->W_CurCount == 0) && (pSW_Transaction_ModuleData2->W_SendDevID == 0))
            {
                pSW_Transaction_ModuleData2->W_SendDevID = 1;
                Bp_Buffer[0] = pSW_Transaction_ModuleData2->B_DevID;
                B_Count = 1;
            }
            else
            {
                B_Count = 0;
            }

            for(; B_Count < W_Length; B_Count++)
            Bp_Buffer[B_Count] = pSW_Transaction_ModuleData2->AB_Data[(pSW_Transaction_ModuleData2->W_CurCount++)];

            if(B_Stop)
            {
                pSW_Transaction_ModuleData2->B_ReplyReady = FALSE;
            }
        }
        else
        {
            for(B_Count = 0; B_Count < W_Length; B_Count++)
            {
                switch(pSW_Transaction_ModuleData2->B_NullMsgPtr++)
                {
                    case 1: Bp_Buffer[B_Count] = NULL_MSG_SECOND_BYTE;
                        break;
                    case 2: Bp_Buffer[B_Count] = INIT_NULL_MSG_CS ^ pSW_Transaction_ModuleData2->B_DevID;
                        break;
                    case 0:
                    default: Bp_Buffer[B_Count] = pSW_Transaction_ModuleData2->B_DevID;
                        break;
                }

                if(pSW_Transaction_ModuleData2->B_NullMsgPtr == NULL_MSG_LENGTH)
                    pSW_Transaction_ModuleData2->B_NullMsgPtr = 0;
            }

            if(B_Stop)
            {
                pSW_Transaction_ModuleData2->B_NullMsgPtr = 0;
            }
        }
    }
	}	
	#endif
    return TRUE;
}

//***************************************************************************** 
// FUNCTION     : BYTE* Lpm_AuxRxI2CSwGetBuff(void) 
// USAGE        : Get pointer to buffer which contains whole I2C-Aux message 
// INPUT        :  B_AuxCh - indicate which Aux channel
// OUTPUT       : far pointer to buffer 
// GLOBALS      : 
// USED_REGS    : 
//***************************************************************************** 
BYTE* Lpm_AuxRxI2CSwGetBuff(AUX_CH B_AuxCh)
{
	#ifdef AUX_RX_C2_SUPPORT
	if(B_AuxCh == AUX_CH_C2)
		return (BYTE*)pSW_Transaction_ModuleData2->AB_Data;	
	else //if(B_AuxCh == AUX_CH_C0)
		return (BYTE*)pSW_Transaction_ModuleData->AB_Data;
	#else
		UNUSED_VAR(B_AuxCh);
		return (BYTE*)pSW_Transaction_ModuleData->AB_Data;
	#endif
}

//***************************************************************************** 
// FUNCTION     : BYTE* Lpm_AuxRxI2CSwGetAddrModuleData(void) 
// USAGE        : Get pointer to I2C SW module data structure
// INPUT        :  B_AuxCh - indicate which Aux channel
// OUTPUT       : far pointer to module data structure 
// GLOBALS      : 
// USED_REGS    : 
//***************************************************************************** 
SW_TransactionStruct_t* Lpm_AuxRxI2CSwGetAddrModuleData(AUX_CH B_AuxCh)
{
	#ifdef AUX_RX_C2_SUPPORT
	if(B_AuxCh == AUX_CH_C2)
		return pSW_Transaction_ModuleData2;
	else //if(B_AuxCh == AUX_CH_C0)
    	return pSW_Transaction_ModuleData;
	#else
		UNUSED_VAR(B_AuxCh);
		return pSW_Transaction_ModuleData;
	#endif
}

//***************************************************************************** 
// FUNCTION     : BOOL Lpm_AuxRxI2CSwCallback(DWORD W_Addr, BYTE B_Command, BYTE B_Length) 
// USAGE        : Is used to connect this module to AuxRx 
// INPUT        : DWORD W_Addr, BYTE B_Command, BYTE B_Length -- request's parameters
//                B_AuxCh - indicate which Aux channel
// OUTPUT       : 
// GLOBALS      : 
// USED_REGS    : 
//***************************************************************************** 
BOOL Lpm_AuxRxI2CSwCallback(DWORD W_Addr, BYTE B_Command, BYTE B_Length, AUX_CH B_AuxCh)
{
    BOOL B_IsWrite;
    BYTE B_Chunk[AUX_MAX_BYTES_IN_CHUNK];

    B_IsWrite = !((B_Command & AUX_REQUEST_COMMAND_MASK) == AUX_REQUEST_I2C_READ);

    if(B_IsWrite)
    {
        Lpm_AuxRxGetChunkData(B_Chunk, B_Length, B_AuxCh);

        Lpm_AuxRxI2CSwHandler((WORD) W_Addr,
        B_IsWrite,
        B_Chunk,
        B_Length,
        (B_Command & AUX_REQUEST_MOT) ? FALSE : TRUE,
        B_AuxCh);

        Lpm_AuxRxSetReplyData(NULL_PTR, 0, B_AuxCh);
    }
    else
    {//Read 
        Lpm_AuxRxI2CSwHandler((WORD) W_Addr,
        B_IsWrite,
        B_Chunk,
        B_Length,
        (B_Command & AUX_REQUEST_MOT) ? FALSE : TRUE,
        B_AuxCh);

        Lpm_AuxRxSetReplyData(B_Chunk, B_Length, B_AuxCh);

    }

    Lpm_AuxRxSetReplyCmd(AUX_REPLY_CMD_AUX_ACK, B_AuxCh);

    return FALSE;
}

//***************************************************************************** 
// FUNCTION     : WORD Lpm_AuxRxI2CInit(WORD W_ChunkLifeTO, BYTE* PB_StartRAM) 
// USAGE        : Allocate and init global data and/or return size of required globals 
// INPUT        : PB_StartRAM -- address in RAM from where internal data is be allocated; NULL_PTR for request for size of internal data 
//                PStartTimer - pointer to external callback starttimer
//                PCheckTimer - pointer to external callback checktimer 
//                B_AuxCh - indicate which Aux channel
// OUTPUT       : required RAM size for globals in bytes 
// GLOBALS      : P_AuxRxI2CInternalData 
// USED_REGS    : 
//***************************************************************************** 
WORD Lpm_AuxRxI2CInit(BYTE* PB_StartRAM,
    T_AuxRxI2C_ExtCallback PStartTimer,
    T_AuxRxI2C_ExtCallback PCheckTimer,
    T_AuxRxI2C_ExtCallback PCheckToPostpone,
    AUX_CH B_AuxCh)
{
    if(PB_StartRAM != NULL_PTR)
    {
		#ifdef AUX_RX_C2_SUPPORT
		if(B_AuxCh == AUX_CH_C2)
		{
	        P_AuxRxI2CInternalData2 = (TAuxRxI2C_InternalData*)PB_StartRAM;
	        P_AuxRxI2CInternalData2->CallbackStartTimer = PStartTimer;
	        P_AuxRxI2CInternalData2->CallbackCheckTimer = PCheckTimer;
	        P_AuxRxI2CInternalData2->CallbackIfToPostponeRq = PCheckToPostpone;
		}
    	else //if(B_AuxCh == AUX_CH_C0)
    	#endif
    	{
	        P_AuxRxI2CInternalData = (TAuxRxI2C_InternalData*)PB_StartRAM;
	        P_AuxRxI2CInternalData->CallbackStartTimer = PStartTimer;
	        P_AuxRxI2CInternalData->CallbackCheckTimer = PCheckTimer;
	        P_AuxRxI2CInternalData->CallbackIfToPostponeRq = PCheckToPostpone;
    	}
    }

    return sizeof(TAuxRxI2C_InternalData);
}

//***************************************************************************** 
// FUNCTION     : TAuxRxI2C_InternalData* Lpm_AuxRxI2CGetModuleData(void) 
// USAGE        : Get pointer to AuxRx I2C module data 
// INPUT        : B_AuxCh - indicate which Aux channel
// OUTPUT       : pointer to structure with global data 
// GLOBALS      : P_AuxRxI2CInternalData 
// USED_REGS    : 
//***************************************************************************** 
TAuxRxI2C_InternalData* Lpm_AuxRxI2CGetModuleData(AUX_CH B_AuxCh)
{
	#ifdef AUX_RX_C2_SUPPORT
	if(B_AuxCh == AUX_CH_C2)
		return P_AuxRxI2CInternalData2;
	else //if(B_AuxCh == AUX_CH_C0)
		return P_AuxRxI2CInternalData;
	
	#else
		UNUSED_VAR(B_AuxCh);
		return P_AuxRxI2CInternalData;
	#endif
		
}

//***************************************************************************** 
// FUNCTION     : WORD huge Lpm_AuxRxLLI2CInit(BYTE* PB_StartRAM) 
// USAGE        : Init I2C-Aux memory region and call back functions.
// INPUT        : PB_StartRAM -- address in RAM from where internal data is be allocated; NULL_PTR for request for size of internal data 
//                ExtSWI2CFunc -- NULL_PTR if IROM handler for SW I2C data has to be used, or pointer to external handler 
//                ExtI2CFunc -- NULL_PTR if IROM handler for I2C data has to be used, or pointer to external handler 
//                B_AuxCh - indicate which Aux channel
// OUTPUT       : Number of bytes for gmt_DPRX_AUX_I2C_INTERNAL_DATA 
// GLOBALS      : P_AuxRxModuleData 
// USED_REGS    : DPRX_AUX2DPCD_CONTROL, DPRX_AUX2OCM_REQ_COMM, DPRX_AUX2OCM_CTRL, DPRX_AUX_CONTROL 
//***************************************************************************** 
WORD Lpm_AuxRxLLI2CInit(BYTE* PB_StartRAM,
    gmt_DPRX_AUX_EXT_HANDLER ExtSWI2CFunc,
    gmt_DPRX_AUX_EXT_HANDLER ExtI2CFunc,
    AUX_CH B_AuxCh
    )
{
    if(PB_StartRAM != NULL_PTR)
    {
    	if(B_AuxCh == AUX_CH_C0)
    	{
	        P_AuxRxI2CModuleData = (gmt_DPRX_AUX_I2C_INTERNAL_DATA*)PB_StartRAM;
	        P_AuxRxI2CModuleData->ExtSWI2CHandler  = ExtSWI2CFunc;
	        P_AuxRxI2CModuleData->ExtI2CHandler    = ExtI2CFunc;
    	}
		#ifdef AUX_RX_C2_SUPPORT
		else if(B_AuxCh == AUX_CH_C2)
		{
	        P_AuxRxI2CModuleData2 = (gmt_DPRX_AUX_I2C_INTERNAL_DATA*)PB_StartRAM;
	        P_AuxRxI2CModuleData2->ExtSWI2CHandler  = ExtSWI2CFunc;
	        P_AuxRxI2CModuleData2->ExtI2CHandler    = ExtI2CFunc;		
		}
		#endif
    }

    return sizeof(gmt_DPRX_AUX_I2C_INTERNAL_DATA);
}

//***************************************************************************** 
// FUNCTION     : void Lpm_AuxRxLLI2CRequest(BYTE B_Request, DWORD DW_Addr, BYTE  B_Len) 
// USAGE        : Handle received I2C over AUX requests
// INPUT        : B_Request - request command
//                DW_Addr - request address
//                B_Len - request length
//                B_AuxCh - indicate which Aux channel
// OUTPUT       : 
// GLOBALS      : 
// USED_REGS    : 
//***************************************************************************** 
void Lpm_AuxRxLLI2CRequest(BYTE B_Request, DWORD DW_Addr, BYTE B_Len, AUX_CH B_AuxCh)
{
    BOOL B_IsSW;

    B_IsSW = Lpm_AuxRxI2CIsSwTransaction(DW_Addr, B_AuxCh);
    B_Len = (B_Request & DPRX_AUX2OCM_SHRT_INST) ? 0 : (B_Len + 1);
    B_Request &= 0x0F;

    if(B_IsSW)
    {
        //gm_Print("Lpm_AuxRxI2CSwCallback",0);    
        //if(P_AuxRxI2CModuleData->ExtSWI2CHandler)
        //    P_AuxRxI2CModuleData->ExtSWI2CHandler(DW_Addr, B_Request, B_Len);
        Lpm_AuxRxI2CSwCallback(DW_Addr, B_Request, B_Len, B_AuxCh);
    }
    else
    {
        //BOOL B_ExtHanFlag = TRUE;

        //if(P_AuxRxI2CModuleData->ExtI2CHandler)
        //    B_ExtHanFlag = P_AuxRxI2CModuleData->ExtI2CHandler(DW_Addr, B_Request, B_Len);

        //if(B_ExtHanFlag)
        {
            //gm_Print("Lpm_AuxRxEdidHandler",0);        
            Lpm_AuxRxEdidHandler(DW_Addr, B_Request, B_Len, B_AuxCh);
        }
    }
}

#endif//AUX_RX_SUPPORT

//********************************* END **************************************** 
