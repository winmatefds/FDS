#ifndef __GM_UPDATEVIDEOPROCESSINGFEATURES_H__
#define __GM_UPDATEVIDEOPROCESSINGFEATURES_H__
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
// MODULE:      gm_UpdateVideoProcessing.h
//
// USAGE:		Contains common definitions and declarations for modules involved in Video processing
//				like gm_UpdateMADI.c; gm_UpdateTNR.c; gm_UpdateCCS.c; gm_UpdateSharpNoise; gm_AdaptiveMadi.c
//
//******************************************************************************

//******************************************************************************
//  G L O B A L    D E F I N I T I O N S
//******************************************************************************

#if (FEATURE_LUMA == REV1)
	#define MAX_LUMA_THRESHOLD		128
	#define LUMA_BLACK_LEVEL		16
	#define LUMA_WHITE_LEVEL 		219
#elif (FEATURE_LUMA == REV2)
	#define MAX_LUMA_THRESHOLD		512
	#define LUMA_BLACK_LEVEL 		64
	#define LUMA_WHITE_LEVEL 		(219*4)
#endif

// enumerator for Sharpness Noise Coring, TNR and MADI thresholds based on input
enum
{
	NTSC_CVBS_PEDESTAL_INDEX,
	NTSC_SVIDEO_PEDESTAL_INDEX,
	NTSC_COMP_PEDESTAL_INDEX,
	NTSC_RF_PEDESTAL_INDEX,
	NTSCPALSECAM_CVBS_NOPED_INDEX,
	NTSCPALSECAM_SVIDEO_NOPED_INDEX,
	NTSCPALSECAM_COMP_NOPED_INDEX,
	NTSCPALSECAM_RF_NOPED_INDEX,
	HDTV_INDEX,
	DVI_INDEX,
	OFF_INDEX,
	MAX_INPUT_INDEX
};

typedef enum 
{
	Motion_Low = 0,
	Motion_Std = 1,
	Motion_High = 2,
	Motion_Vert = 3,
	Motion_VertL = 4,
	Motion_VertS = 5,
	Motion_VertH = 6,
	Motion_Other = 7
}t_MotionLevel;

//******************************************************************************
//  E X T E R N    F U N C T I O N S
//******************************************************************************
//extern BYTE gmp_DeterminePedestalType(gmt_ConnTypes Connector, gmt_DECODER_VIDEO_MODE Mode);
extern void gmp_SetAdaptiveFlags(DWORD W_Select, BOOL BL_Action);


#endif

