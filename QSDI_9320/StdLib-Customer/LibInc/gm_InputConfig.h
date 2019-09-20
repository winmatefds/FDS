/*
    $Workfile:   gm_InputConfig.h  $
      $Revision: 1.4 $
      $Date: 2011/12/14 03:44:48 $
*/
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
// MODULE :gm_InputConfig.h
//
// USAGE  :This module contains Tables and defines used for Configuration of 
//         Front-End (AFE+DFE).
//
// NOTE   :This file is generated from the spreadsheet Cortez_InputConfig.xls on,
//         9/24/2004 at 12:55:46 PM
//*******************************************************************************
 
//******************************************************************************
//  IMPORTANT : P L E A S E   D O   N O T   E D I T  T H I S  F I L E
//******************************************************************************
 
//******************************************************************************
//  G L O B A L    D E F I N I T I O N S
//******************************************************************************
 
#ifndef __GM_INPUTCONFIG_H__
#define __GM_INPUTCONFIG_H__
//#include "System\all.h"
 
 //Master list of all input ports spanning all projects
typedef enum 
{ 
	ALL_BOARDS_CVBS1, 		// 0
	ALL_BOARDS_SVIDEO1,		// 1
	ALL_BOARDS_COMP1, 		// 2
	ALL_BOARDS_VGA1, 		// 3
	ALL_BOARDS_DVI1, 		// 4
	ALL_BOARDS_DVI2, 		// 5
	ALL_BOARDS_DIP1, 		// 6
	ALL_BOARDS_HDMI, 		// 7
	ALL_BOARDS_HDMI2, 		// 8
	ALL_BOARDS_HDMI3, 		// 9
	ALL_BOARDS_DP1, 		// 10
	ALL_BOARDS_DP2, 		// 11
	NOT_USED, 				// 12
	ALL_BOARDS_SLAVE1, 		// 13
	NUM_MASTER_LIST, 		// 14
} ALL_BOARDS_INPUT_PORT;
 
#define NUM_OF_CONNECTORS 14
#define NUM_OF_REGISTERS 13
#define NUM_OF_ANALOG_INP 1
#define NUM_OF_DIGITAL_INP 0

//User-Defined Connector Names
typedef enum USER_DEF_CONN_NAMES
{
	CVBS1 = ALL_BOARDS_CVBS1,
	SVIDEO1 = ALL_BOARDS_SVIDEO1,
	COMP1 = ALL_BOARDS_COMP1,
	VGA1 = ALL_BOARDS_VGA1,
	DP1 = ALL_BOARDS_DP1,
	DVI1 = ALL_BOARDS_DVI1,
	DIP1 = ALL_BOARDS_DIP1,
	HDMI1 = ALL_BOARDS_HDMI,
}gmt_UserDefConnNames; //End of Use Defined Connector Names

extern gmt_InputConfigTable ROM gmvs_InputConfigTbl[];

extern WORD ROM gmvw_AFERegAddress[];
extern BYTE ROM gmvb_AFERegistersVals[];

#endif // __GM_INPUTCONFIG_H__
//*********************************  END  **************************************
