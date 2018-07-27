/*
   $Workfile:   ColorSchemeTbl.c  $
   $Revision: 1.14 $
   $Date: 2012/02/14 07:54:03 $
*/
#define __COLORSCHEMETBL_C__
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
// MODULE:        ColorSchemeTbl.c
//
// USAGE:   This file contains arrays for summarizing the available Adaptive
//            Color Management (ACM) tables and associated data.
//
//******************************************************************************

//******************************************************************************
//  I N C L U D E    F I L E S
//******************************************************************************

#include "StdLibInc\gm_Global.h"
#include "StdLibInc\gm_ChipFeature.h"
#if (FEATURE_ACM_3D == ENABLE)
#include "StdLibInc\gm_ACM3D.h"
#endif

//******************************************************************************
//  D E F I N E S
//******************************************************************************

//******************************************************************************
//  E X T E R N S
//******************************************************************************

//***************************************************************************
//  C O D E
//***************************************************************************

#if (FEATURE_ACM_3D == ENABLE)

VQ_ACM_Parameters_t ROM ACM_Params_Default =
{
   {
      sizeof(VQ_ACM_Parameters_t),
      0, // STLAYER_VIDEO_ACM, // Useless in Athena
      0
   },
   {
      1,	// ThFadeLorGainEn
      8,	// ThFadeLorVal
      112,	// ThFadeLorCutOff
   },
   8,    // Number of active regions
   {
      // W_HueCenter, W_HueAperture, B_R1, B_R2, B_Y1, B_Y2, B_HueFade, B_SatFade, B_LumiFade
      {200,   5, 400, 2240, 480, 3440,  20, 400, 400},
      {255,  10, 288, 2240, 320, 3600,  20, 288, 288},
      {315,  10, 560, 2240, 800, 3520,  20, 400, 400},
      {  9,   1, 560, 2288, 800, 3520,  25, 400, 400},
      {123,   4, 400, 2000, 1760, 2960,  30, 320, 400},
      {123,   4, 400, 2400, 320, 1600,  25, 320, 400},
      {164,   1, 400, 2240, 400, 3200,  16, 320, 240},
      {  0,   0,   0,   0,   0,   0,   5,  80,  80},
   },
   {
      // hue_offset, hue_gain, sat_offset, sat_gain, lum_offset, lum_gain, u_vect, v_vect, alpha
      {  8,  -2,   0,  67, -106,  64,   0,   0,  26},
      {-10,   2, -32,  67, -68,  64,   0,   0,  20},
      { -9,  -3,   0,  65, -68,  60,   0,   0,  14},
      {-15,   3, -32,  67, -80,  64,   0,   0,   0},
      { -2,  -2,   0,  67,   0,  64,   0,   0,  11},
      { -5,  -2,   0,  67,   0,  65,   0,   0,  11},
      {  2,   2,   0,  65, -20,  64,   0,   0,  16},
      {  0,   0,   0,  64,   0,  64,   0,   0,   0},
   }
} ;

VQ_ACM_Parameters_t ROM ACM_Params_Vivid =
{
	{
		sizeof(VQ_ACM_Parameters_t), // Size
		0, // STLAYER_VIDEO_ACM, // Signature
		768 // Version
	},
   {    
   	0,	// ThFadeLorGainEn
   	5,	// ThFadeLorVal
   	128,	// ThFadeLorCutOff
   },
	8,	// Number of active regions
	{
		// W_HueCenter, B_HueAperture, B_R1, B_R2, B_Y1, B_Y2, B_HueFade, B_SatFade, B_LumiFade
		{180,   4, 192, 496, 480, 3440,  10,  80, 400},
		{315,  10, 320, 2240, 736, 3520,  20, 256, 256},
		{110,  12, 1152, 2016, 640, 3520,  16, 256, 400},
		{172,   4, 832, 2368, 400, 3840,   8, 288, 240},
		{232,  31, 320, 1792, 384, 3840,  16, 256, 256},
		{120,  20, 384, 912, 1280, 3200,  22, 320, 640},
		{  0,   0,   0,   0,   0,   0,   5,  80,  80},
		{  0,   0,   0,   0,   0,   0,   5,  80,  80},
	},
	{
		// hue_offset, hue_gain, sat_offset, sat_gain, lum_offset, lum_gain, u_vect, v_vect, alpha
		{  4,   4,  32,  67,   0,  64,   0,   0,  16},
		{  0,  -4,  64,  68, -32,  60,   0,   0,  16},
		{ -2,   0,   0,  66,   0,  64,   0,   0,  16},
		{ -4,  -2,  64,  67,   0,  64,   0,   0,  16},
		{  0,  -2,  96,  67,   0,  64,   0,   0,  16},
		{ -1,  -2,   0,  64,  16,  64,   0,   0,   8},
		{  0,   0,   0,  64,   0,  64,   0,   0,   0},
		{  0,   0,   0,  64,   0,  64,   0,   0,   0},
	}
} ;

VQ_ACM_Parameters_t ROM ACM_Params_Cinema =
{
	{
		sizeof(VQ_ACM_Parameters_t),
		0, // STLAYER_VIDEO_ACM,
		768
	},
   {    
   	0,	// ThFadeLorGainEn
   	5,	// ThFadeLorVal
   	128,	// ThFadeLorCutOff
   },
	8,	// Number of active regions
	{
		// W_HueCenter, B_HueAperture, B_R1, B_R2, B_Y1, B_Y2, B_HueFade, B_SatFade, B_LumiFade
      {200,   5, 400, 2240, 480, 3440,  20, 400, 400},
      {255,  10, 288, 2240, 320, 3600,  20, 288, 288},
      {315,  10, 560, 2240, 800, 3520,  20, 400, 400},
      {  9,   1, 560, 2288, 800, 3520,  25, 400, 400},
      {123,   4, 400, 2000, 1760, 2960,  30, 320, 400},
      {123,   4, 400, 2400, 320, 1600,  25, 320, 400},
      {164,   1, 400, 2240, 400, 3200,  16, 320, 240},
      {  0,   0,   0,   0,   0,   0,   5,  80,  80},
	},
{
		// hue_offset, hue_gain, sat_offset, sat_gain, lum_offset, lum_gain, u_vect, v_vect, alpha
		{ 10,  -3,   0,  70, -128,  64,   0,   0,  16},
		{-10,   3, -32,  70, -80,  64,   0,   0,  16},
		{ -9,  -4,   0,  68, -128,  60,   0,   0,  16},
		{-15,   4, -32,  68, -96,  64,   0,   0,  10},
		{ -2,  -2,   0,  68,   0,  64,   0,   0,  10},
		{ -5,  -2,   0,  68,   0,  66,   0,   0,  10},
		{  2,   2,   0,  68, -80,  64,   0,   0,  16},
      {  0,   0,   0,  64,   0,  64,   0,   0,   0},
	}
};

VQ_ACM_Parameters_t ROM ACM_Params_Game =
{
{
		sizeof(VQ_ACM_Parameters_t),
		0, // STLAYER_VIDEO_ACM,
		768
	},
{
   	0,	// ThFadeLorGainEn
   	5,	// ThFadeLorVal
   	128,	// ThFadeLorCutOff
   },
	8,	// Number of active regions
{
		// W_HueCenter, B_HueAperture, B_R1, B_R2, B_Y1, B_Y2, B_HueFade, B_SatFade, B_LumiFade
		{180,   4, 192, 496, 480, 3440,  10,  80, 400},
		{315,  10, 320, 2240, 736, 3520,  20, 256, 256},
		{110,  12, 1152, 2016, 640, 3520,  16, 256, 400},
		{172,   4, 832, 2368, 400, 3840,   8, 288, 240},
		{232,  31, 320, 1792, 384, 3840,  16, 256, 256},
		{120,  20, 384, 912, 1280, 3200,  22, 320, 640},
		{  0,   0,   0,   0,   0,   0,   5,  80,  80},
		{  0,   0,   0,   0,   0,   0,   5,  80,  80},
	},
{
		// hue_offset, hue_gain, sat_offset, sat_gain, lum_offset, lum_gain, u_vect, v_vect, alpha
		{  4,   4,  32,  67,   0,  64,   0,   0,  16},
		{  0,  -4,  64,  68, -32,  60,   0,   0,  16},
		{ -2,   0,   0,  66,   0,  64,   0,   0,  16},
		{ -4,  -2,  64,  67,   0,  64,   0,   0,  16},
		{  0,  -2,  96,  67,   0,  64,   0,   0,  16},
		{ -1,  -2,   0,  64,  16,  64,   0,   0,   8},
   {  0,   0,   0,  64,   0,  64,   0,   0,   0},
   {  0,   0,   0,  64,   0,  64,   0,   0,   0},
	}
};

VQ_ACM_Parameters_t ROM ACM_Params_Sport =
{
{
		sizeof(VQ_ACM_Parameters_t),
		0, // STLAYER_VIDEO_ACM,
		768
	},
{
   	0,	// ThFadeLorGainEn
   	5,	// ThFadeLorVal
   	128,	// ThFadeLorCutOff
   },
	8,	// Number of active regions
	{
		// W_HueCenter, B_HueAperture, B_R1, B_R2, B_Y1, B_Y2, B_HueFade, B_SatFade, B_LumiFade
		{171,  10, 160, 2880, 2080, 2112,   5,  80,  80},
		{289,  10, 160, 2880,   0, 4080,   5, 160,  80},
		{208,  10, 160, 2880, 320, 2720,   5, 160,  80},
		{ 51,  10, 160, 2880, 1600, 1920,   5, 160,  80},
		{108,  10, 160, 2880,   0, 4080,   5, 160,  80},
		{323,  10, 288, 960, 2240, 3840,   5, 160,  80},
		{  0, 180, 160, 2880, 480, 640,   5, 160,  80},
		{  0,   0,   0,   0,   0,   0,   5,  80,  80},
	},
{
		// hue_offset, hue_gain, sat_offset, sat_gain, lum_offset, lum_gain, u_vect, v_vect, alpha
   {0,   0,   0,   64,   0,   64,   0,   0,   0},
   {0,   0,   0,   64,   0,   64,   0,   0,   0},
		{  0,   0, 320,  64,   0,  64,   0,   0,   0},
   {0,   0,   0,   64,   0,   64,   0,   0,   0},
   {0,   0,   0,   64,   0,   64,   0,   0,   0},
		{  0,   0, 240,  70,   0,  64,  96, -64,   0},
   {0,   0,   0,   64,   0,   64,   0,   0,   0},
   {0,   0,   0,   64,   0,   64,   0,   0,   0},
	}
};

VQ_ACM_Parameters_t ROM ACM_Params_VWD =
{
{
		sizeof(VQ_ACM_Parameters_t),
		0, // STLAYER_VIDEO_ACM,
		768
	},
{
   	0,	// ThFadeLorGainEn
   	5,	// ThFadeLorVal
   	128,	// ThFadeLorCutOff
   },
	8,	// Number of active regions
{
		// W_HueCenter, B_HueAperture, B_R1, B_R2, B_Y1, B_Y2, B_HueFade, B_SatFade, B_LumiFade
      {123,   4, 608,  880, 960, 2300, 272, 640, 640},
      {315,  10,  35,  1280,   50, 1536,  20,  80, 512},   // Z2: Blue region
      {223,  32, 512,  1280, 1280, 2304, 320, 480, 512},   // Z3: Green region
      {  0,   0,   0,    0,   0,    0,   0,   0,   0},
      {  0,   0,   0,    0,   0,    0,   0,   0,   0},
      {  0,   0,   0,    0,   0,    0,   0,   0,   0},
      {  0,   0,   0,    0,   0,    0,   0,   0,   0},
      {  0,   0,   0,    0,   0,    0,   0,   0,   0},
	},
{
		// hue_offset, hue_gain, sat_offset, sat_gain, lum_offset, lum_gain, u_vect, v_vect, alpha
      {  0,   0,   0,  50,   0,  64,   0,   0,   0},
      {  0,   0,  20,  48,   0,  60,  21,  15,  15},
      {  0,   0,   0, 122,   0,  64,   0,   0,   0},
      {  0,   0,   0,  64,   0,  64,   0,   0,   0},
      {  0,   0,   0,  64,   0,  64,   0,   0,   0},
      {  0,   0,   0,  64,   0,  64,   0,   0,   0},
      {  0,   0,   0,  64,   0,  64,   0,   0,   0},
      {  0,   0,   0,  64,   0,  64,   0,   0,   0},
	}
};

#endif
//************************* E N D ********************************************



