/*
$Workfile:   Lpm_AuxRxEDID.h  $
$Revision: 1.8 $
$Date: 2012/04/13 03:28:12 $
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
//   MODULE : Lpm_AuxRxEDID.h 
// 
//   USAGE  : Handle AuxRx EDID R/W process
// 
//   OWN    : 
// 
//============================================================================= 
#ifndef __LPM_AUXRXEDID_H__
#define __LPM_AUXRXEDID_H__

/****************************************************************************/
/*  D E F I N I T I O N S                                                   */
/****************************************************************************/
#define	AUX_TRANSACTION_SIZE			16		// One AUX Transation Data Lenght
#define EDID_BUFFER_START				0xFFD00UL
#define EDID_BUFFER2_START				0xFFC00UL
#define EDID_BUFFER_SIZE				256
#define AUX_EDID_WRITE_UNLOCK			0x5A
#define	EDID_EEPROM_DEV_ADDR			0xA0


typedef struct
{
    BYTE *Ba_EDIDReadBuffer; 
    BYTE B_WrState;
    WORD W_Offset;
    WORD W_ReadPointer;
    WORD W_WritePointer;
} DprxVirtualEdid_t;

typedef enum 
{
   WR_FREE,
   WR_START,
   WR_ADDR1,
   WR_ADDR2,
   WR_DATA,
   WR_STOP
} AUX_EDID_WRITE_TYPE;

//******************************************************************************
//  G L O B A L   F U N T I O N S
//******************************************************************************
void Lpm_AuxRxEdidHandlerInit(AUX_CH B_AuxCh);
void Lpm_AuxRxEdidHandler(DWORD DW_Addr, BYTE B_Req, BYTE B_Len, AUX_CH B_AuxCh);
void Lpm_AuxRxEdidReadHandler(BYTE B_Addr, BYTE B_Len, BYTE B_IsStop, AUX_CH B_AuxCh);
void Lpm_AuxRxEdidRead(BYTE B_DevAddress, BYTE *Bp_Buffer, WORD W_Length, AUX_CH B_AuxCh);
void Lpm_AuxRxEdidWriteHandler(BYTE B_Addr, BYTE B_Len, BYTE B_IsStop, AUX_CH B_AuxCh);
void Lpm_AuxRxEdidWrite(BYTE B_DevAddress, BYTE *Bp_Buffer, WORD W_Length, AUX_CH B_AuxCh);
void Lpm_DprxVirtualEdidInit(BYTE B_WrState, AUX_CH B_AuxCh);
//void Lpm_DprxVirtualEdidWrite(void);

//******************************************************************************
//  E X T E R N A L    V A R I A B L E S                                    
//******************************************************************************


#endif	//__LPM_AUXRXEDID_H__