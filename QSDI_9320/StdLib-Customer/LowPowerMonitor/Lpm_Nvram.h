#ifndef __LPM_NVRAM_H__
#define __LPM_NVRAM_H__

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
// MODULE:  	Lpm_Nvram.h
//
// USAGE :  	Standard low level NVRAM read/write function prototypes
//
//******************************************************************************

//******************************************************************************
//  G L O B A L    D E F I N I T I O N S
//******************************************************************************

#define NVRAM_DEVICE		   0xA0 //0xA2       // NVRAM device address
#define NVRAM_ADDR_LEN		1 // 2        // NVRAM address length
#define NVRAM_PAGE_SIZE		8 // 32			// Page write size

#define NVRAM_OFFSET_PWR_MODE    0
#define NVRAM_LENGTH_PWR_MODE    1


//******************************************************************************
//	E X T E R N A L    V A R I A B L E S
//******************************************************************************

extern BYTE gB_NVRamError;			// NVram error bit indicator


//******************************************************************************
//	G L O B A L    F U N C T I O N    P R O T O T Y P E S
//******************************************************************************

gmt_RET_STAT far Lpm_NvramCheckDevice(WORD W_RegAddr);
BYTE far	Lpm_NvramReadByte(WORD W_RegAddr);
gmt_RET_STAT far Lpm_NvramWriteByte(WORD W_RegAddr, BYTE B_Data);
gmt_RET_STAT far Lpm_NvramReadBuffer(WORD W_RegAddr, BYTE far*Bp_Buf, WORD W_Length);
gmt_RET_STAT far Lpm_NvramWriteBuffer(WORD W_RegAddr, BYTE far*Bp_Buf, WORD W_Length);

#endif	//__LPM_NVRAM_H__
//**********************************  END  *************************************

