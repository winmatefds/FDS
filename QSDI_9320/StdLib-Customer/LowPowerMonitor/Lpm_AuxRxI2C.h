
/*
$Workfile:  Lpm_AuxRxI2C.h  $
$Revision: 1.6 $
$Date: 2011/12/15 01:43:01 $
*/

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
//   MODULE : Lpm_AuxRxI2C.h 
// 
//   USAGE  : Header file for AuxRxI2C module
// 
//   OWN    : 
// 
//============================================================================= 
#ifndef __LPM_AUXRXI2C_H__ 
#define __LPM_AUXRXI2C_H__


//****************************************************************************** 
//  G L O B A L    D E F I N I T I O N S 
//****************************************************************************** 

/*typedef enum SWTransStatusEnum
{
    SW_Trans_None,
    SW_Trans_Opened
}SWTransStatusType;
*/

typedef BOOL (far* T_AuxRxI2C_ExtCallback) (BYTE B_Addr, BYTE* PB_Buff, BYTE* PB_Length, BYTE B_Stop, BYTE* PB_Status);
typedef BOOL (far* gmt_DPRX_AUX_EXT_HANDLER) (DWORD W_Addr, BYTE B_Command, BYTE B_Length);

//internal data of module 
typedef struct
{
    T_AuxRxI2C_ExtCallback CallbackStartTimer;
    T_AuxRxI2C_ExtCallback CallbackCheckTimer;
    T_AuxRxI2C_ExtCallback CallbackIfToPostponeRq;

    BYTE AB_DataBuff[AUX_MAX_BYTES_IN_CHUNK];

}TAuxRxI2C_InternalData;


//Module internal data 
typedef struct
{
    gmt_DPRX_AUX_EXT_HANDLER ExtSWI2CHandler;
    gmt_DPRX_AUX_EXT_HANDLER ExtI2CHandler;

}gmt_DPRX_AUX_I2C_INTERNAL_DATA;


typedef struct SW_TransactionStruct   // SW Transactions Struct 
{
    BYTE  AB_Data[MAX_DDC2BI_MSG_SIZE];
    WORD  W_CurCount;
    WORD  W_SendDevID;
    BYTE  B_Dir;
    BYTE  B_DevID;
    BYTE  B_SWDevIdCount;
    BYTE  B_SWDevId[MAX_I2C_SW_ADDRESSES];
    BYTE  B_ReplyReady;
    BYTE  B_NullMsgPtr;
}SW_TransactionStruct_t;


//****************************************************************************** 
//  G L O B A L   V A R I A B L E S 
//****************************************************************************** 

//****************************************************************************** 
//  G L O B A L   F U N T I O N S 
//****************************************************************************** 
WORD Lpm_AuxRxI2CSwTransInit(BYTE* PB_StartRAM, AUX_CH B_AuxCh);
BOOL Lpm_AuxRxI2CIsSwTransaction(DWORD DW_Addr, AUX_CH B_AuxCh);
void Lpm_AuxRxI2CAddSwDevId(BYTE B_I2CDevId, AUX_CH B_AuxCh);
void Lpm_AuxRxI2CClearSwDevIdTable(AUX_CH B_AuxCh);
BYTE Lpm_AuxRxI2CSwHandler(WORD W_Address, BOOL B_Dir, BYTE *Bp_Buffer, WORD W_Length, BYTE B_Stop, AUX_CH B_AuxCh);
BYTE* Lpm_AuxRxI2CSwGetBuff(AUX_CH B_AuxCh);
SW_TransactionStruct_t* Lpm_AuxRxI2CSwGetAddrModuleData(AUX_CH B_AuxCh);
BOOL Lpm_AuxRxI2CSwCallback(DWORD W_Addr, BYTE B_Command, BYTE B_Length, AUX_CH B_AuxCh);
WORD Lpm_AuxRxI2CInit(BYTE* PB_StartRAM,
    T_AuxRxI2C_ExtCallback PStartTimer,
    T_AuxRxI2C_ExtCallback PCheckTimer,
    T_AuxRxI2C_ExtCallback PCheckToPostpone,
    AUX_CH B_AuxCh);
TAuxRxI2C_InternalData* Lpm_AuxRxI2CGetModuleData(AUX_CH B_AuxCh);
WORD Lpm_AuxRxLLI2CInit(BYTE* PB_StartRAM,
    gmt_DPRX_AUX_EXT_HANDLER ExtSWI2CFunc,
    gmt_DPRX_AUX_EXT_HANDLER ExtI2CFunc, 
    AUX_CH B_AuxCh);
void Lpm_AuxRxLLI2CRequest(BYTE B_Request, DWORD DW_Addr, BYTE B_Len, AUX_CH B_AuxCh);


#endif // __LPM_AUXRXI2C_H__ 
//********************************* END **************************************** 
