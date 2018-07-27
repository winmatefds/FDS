/*
$Workfile:   DisplayPortConfig.h  $
$Revision: 1.11 $
$Date: 2012/08/20 03:43:17 $
*/
//*****************************************************************************************
//*** COPYRIGHT (C) STMicroelectronics 2010 All rights reserved.                        ***
//*** This document contains proprietary and confidential information of the            ***
//*** STMicroelectronics Group. This document is not to be copied in whole or part.     ***
//*** STMicroelectronics assumes no responsibility for the consequences of use of       ***
//*** such information nor for any infringement of patents or other rights of third     ***
//*** parties which may result from its use. No license is granted by implication       ***
//*** or otherwise under any patent or patent rights of STMicroelectronics.             ***
//*** STMicroelectronics products are not authorized for use as critical components     ***
//*** in life support devices or systems without express written approval of            ***
//*** STMicroelectronics.                                                               ***
//*****************************************************************************************
//***                                                                                   ***
//*** MODULE:DisplayPortConfig.h                                                        ***
//***                                                                                   ***
//*** USAGE:The module contains Definitions used for DisplayPort Configuration          ***
//***                                                                                   ***
//*****************************************************************************************

#ifndef __DISPLAYPORTCONFIG_H__
#define __DISPLAYPORTCONFIG_H__
//*****************************************************************************************
//  MAJOR DISPLAY PORT DEFINITIONS ---> Need to defined based on board configration by User
//*****************************************************************************************
#define EXTERNAL_MUX0_AVAILABLE        0           //  1: An exteranl mux in front of CPHY0 
#define NUM_OF_CNTR_ON_CPHY0           1           //  Number of physical connectors on CPHY0

#ifdef EV3_93xx_BOARD
#define EXTERNAL_MUX2_AVAILABLE        0           //  1: An exteranl mux in front of CPHY2 
#define NUM_OF_CNTR_ON_CPHY2           0           //  Number of physical connectors on CPHY2
#else
#define EXTERNAL_MUX2_AVAILABLE        0           //  1: An exteranl mux in front of CPHY2 
#define NUM_OF_CNTR_ON_CPHY2           1           //  Number of physical connectors on CPHY2
#endif

#define SELECT_DPTX_TYPE      USE_DPTX_DYNAMIC_SWITCH  //  Options: NO_DPTX_OUTPUT 
                                                       //           USE_MST_DPTX_OUTPUT
                                                       //           USE_IDP_DPTX_OUTPUT
                                                       //           USE_DPTX_DYNAMIC_SWITCH 
                                                       //           USE_DPTX_MANUAL_SWITCH
//*****************************************************************************************
//  MAJOR DISPLAY PORT DEFINITIONS END
//*****************************************************************************************
#endif // __DISPLAYPORTCONFIG_H__
//*****************************************  END  *****************************************
