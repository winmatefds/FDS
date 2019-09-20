/*
	$Workfile:   gm_TableLoader.h  $
	$Revision: 1.2 $
	$Date: 2011/12/14 10:35:32 $
*/
#ifndef __GM_TABLELOADER_H__
#define __GM_TABLELOADER_H__
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
// MODULE:      gm_TableLoader.h
//
// USAGE:       Header file for Table Access Bridge Drivers
//
//******************************************************************************

//******************************************************************************
//  G L O B A L    D E F I N I T I O N S
//******************************************************************************

//#define ACC_ENTRY_START 0xa930

#define NUM_OF_ACM_TABLES		2

#define WAIT_TABLE_ACCESS_BRIDGE_XROM(W_Flag)						\
	while(1)														\
	{																\
		if((gm_ReadRegWord(TABLE_ACCESS_BRIDGE_STATUS) & (W_Flag)))	\
		{															\
			if(gm_TimerCheck(SYSTEM_WATCHDOG_TMR) == TIMER_TMO)		\
			{														\
				break;									\
			}														\
		}															\
		else														\
		{															\
			break;													\
		}															\
	}

//******************************************************************************
//  E X T E R N
//******************************************************************************

#if 0
extern ROM  BYTE 	Ba_AccVivid[];		// ACC Vivid Table
extern ROM  BYTE 	Ba_AccSports[];		// ACC Vivid Table
extern ROM  BYTE 	Ba_AccMovie[];		// ACC Vivid Table
#endif

//
// Acc tables
//
extern ROM BYTE CinemaACCTransFuncData_01[];

//
// Acm tables
//
extern ROM BYTE ACMFineFleshToneTables[3][3][80];

//******************************************************************************
//  F U N C T I O N    P R O T O T Y P E
//******************************************************************************
gmt_RET_STAT gmp_LoadAcmTable(BYTE far * B_CMIntpTablePtr, WORD const W_SizeOfCMIntpTable);

#endif

//*********************************  END  **************************************
