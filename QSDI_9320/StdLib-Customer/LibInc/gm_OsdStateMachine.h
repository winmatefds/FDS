/*
	$Workfile:   gm_OsdStateMachine.h  $
	$Revision: 1.2 $
	$Date: 2011/12/14 09:46:51 $
*/
#ifndef __OSD_SM_H__
#define __OSD_SM_H__
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
// MODULE:      osd_sm.h
//
// USAGE:
//
//******************************************************************************

//******************************************************************************
//  T Y P E    D E F I N I T I O N S
//******************************************************************************
//Reference Type
typedef enum
{
	RFT_NoReference,	//not a reference
	RFT_ShortIDRef,		//1 byte ID vary within the range 0x0 - 0x7f
	RFT_ShortPtrRef,	// NOT SUPPORTED
	RFT_LongIDRef,		//2 bytes ID vary within the range 0x0 - 0x7fff
	RFT_LongPtrRef		// NOT SUPPORTED
};

//enter state flags
typedef enum
{
	OES_Enter_Stop		= 0x01,
	OES_Enter_Parent	= 0x02
};

// types of action data for SM4 
typedef enum			//WB4
{
	Data_None = 0x00,
	Data_DirectFromSAT = 0x40,
	Data_Pointer = 0x80,
	Data_Block = 0xC0
} ;	//ActionParameters;

typedef BYTE far (*FPT_ActionFunc)(BYTE);			
typedef void far (*gmt_OSD_ACTION_FUNC)(void);	//WB4

//******************************************************************************
//  L O C A L    D E F I N I T I O N S
//******************************************************************************
#define ACTIONOPREF_SHORTIDREF \
		gmv_ActionOpSet[GetCurrentSATOffsetByte() & (~BIT7)]

#define ACTIONOPSET_LONGIDREF \
		gmv_ActionOpSet[GetCurrentSATOffsetWord() & (~((WORD)(BIT7|BIT6) << 8))]

#define CURRENT_STATE_PTR	gmv_OsdThreadContextArray[gB_CurrentThreadId].Bp_CurrentStatePtr
#define NEXT_STATE_PTR		gmv_OsdThreadContextArray[gB_CurrentThreadId].Bp_NextStatePtr
#define SAVE_STATE_PTR		gmv_OsdThreadContextArray[gB_CurrentThreadId].Bp_SaveStatePtr

#define SAT_CB_00_EVT_SHIFT		0x0003
#define SAT_CB_10_EVT_SHIFT		0x0002
#define SAT_CB_11_MAC_SHIFT		0x0002

#define SAT_CB_00_EVT_MASK		(BIT3 | BIT4 | BIT5)
#define SAT_CB_00_ACT_MASK		(BIT0 | BIT1 | BIT2)

#define SAT_CB_10_EVT_MASK		(BIT2 | BIT3 | BIT4 | BIT5)
#define SAT_CB_10_CND_MASK		(BIT0 | BIT1)

#define SAT_CB_11_MAC_MASK		(BIT2 | BIT3 | BIT4)
#define SAT_CB_11_CND_MASK		(BIT0 | BIT1)
#define SAT_CB_11_ATREF			(BIT5)
#define SAT_CB_11_ATREF_MASK	(BIT4 | BIT3 | BIT2 | BIT1 | BIT0)

#define SAT_CB_MASK				(BIT7 | BIT6)
#define ACT_DATA_TYPE_MASK		(BIT7 | BIT6)

//******************************************************************************
//  G L O B A L    V A R I A B L E S
//******************************************************************************
extern BYTE gB_CurrentThreadId;
extern BYTE ROM * ROM *gBp_GlobalStateSet;
extern WORD gW_OsdPostedEvent;

//******************************************************************************
//  E X T E R N A L    F U N C T I O N    P R O T O T Y P E S
//******************************************************************************
WORD GetCurrentStateIndex(void);
BYTE GetReferenceType(BYTE B_Baram);
void	CallActionWithParams(  gmt_OSD_ACTION_FUNC Fp_ActionFunctionPtr, BYTE ROM * Bp_DataPtr, BYTE B_DataSize );
BYTE GetCurrentSATOffsetByte(void);
WORD GetCurrentSATOffsetWord(void);
DWORD GetCurrentSATOffsetTripleByte(void);

#endif // __OSD_SM__
//*********************************  END  **************************************
