/*
	$Workfile:   ChannelPortResource.h  $
	$Revision: 1.2 $
	$Date: 2011/12/14 09:20:57 $
*/
#ifndef __CHANNELPORTRESOURCE_H__
#define __CHANNELPORTRESOURCE_H__
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
// MODULE:      ChannelPortResource.h
//
// USAGE:       Header file: port resource ofr channel services 
//
//******************************************************************************


//******************************************************************************
//  F U N C T I O N    P R O T O T Y P E
//******************************************************************************

// criteria of the resource sharing
BOOL gmp_IsPortResourceCollision		(gmt_PHY_CH Channel);

// checking functions
BOOL gmp_IsPortResourceAvailable		(gmt_PHY_CH Channel);

// primitives of handling the resource particion
BOOL gmp_AcquirePortResource			(gmt_PHY_CH Channel);
BOOL gmp_FreePortResource				(gmt_PHY_CH Channel);

// more complex handling
BOOL gmp_CheckAndAcquirePortResource	(gmt_PHY_CH Channel);
#endif // #ifndef __CHANNELPORTRESOURCE_H__
//*********************************  END  **************************************
