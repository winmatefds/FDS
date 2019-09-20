/*
	$Workfile:   gm_ACM3D.h  $
	$Revision: 1.10 $
	$Date: 2012/04/11 02:56:19 $
*/
#ifndef __ACM3D_H__
#define __ACM3D_H__
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
// MODULE:      gm_ACM3D.h
//
// USAGE:
//
//******************************************************************************

//*************************************************************************
//	A C M   H A R D W A R E   T Y P E
//*************************************************************************

//*************************************************************************
//	A C M   R E G I O N   D E F I N I T I O N
//*************************************************************************
#if 0
//
// Definition of Basic Color Hue values (HUE)
//
#define HUE_MAG			52	// degree
#define HUE_RED			109	// degree
#define HUE_YEL			171	// degree
#define HUE_GRE			232	// degree
#define HUE_CYA			289	// degree
#define HUE_BLU			351	// degree

#define HUE_APERTURE	14 // degree, half of hue sector
#define HUE_FADE		14 // degree, s.b. not less than hue sector

#define HUE_APERT_MAG	HUE_APERTURE
#define HUE_APERT_RED	HUE_APERTURE
#define HUE_APERT_YEL	HUE_APERTURE
#define HUE_APERT_GRE	HUE_APERTURE
#define HUE_APERT_CYA	HUE_APERTURE
#define HUE_APERT_BLU	HUE_APERTURE

#define HUE_FADE_MAG	HUE_FADE
#define HUE_FADE_RED	HUE_FADE
#define HUE_FADE_YEL	HUE_FADE
#define HUE_FADE_GRE	HUE_FADE
#define HUE_FADE_CYA	HUE_FADE
#define HUE_FADE_BLU	HUE_FADE

//
// Definition of lower and upper region Saturation bounds (SAT)
//
#define SAT_LO_R		35 		// For 8 bit YUV data
#define SAT_UP_R		180 	// For 8 bit YUV data
#define SAT_FADE		35 		// For 8 bit YUV data

#define R1_MAG			SAT_LO_R
#define R1_RED			SAT_LO_R
#define R1_YEL			SAT_LO_R
#define R1_GRE			SAT_LO_R
#define R1_CYA			SAT_LO_R
#define R1_BLU			SAT_LO_R

#define R2_MAG			SAT_UP_R
#define R2_RED			SAT_UP_R
#define R2_YEL			SAT_UP_R
#define R2_GRE			SAT_UP_R
#define R2_CYA			SAT_UP_R
#define R2_BLU			SAT_UP_R

#define SAT_FADE_MAG	SAT_FADE
#define SAT_FADE_RED	SAT_FADE
#define SAT_FADE_YEL	SAT_FADE
#define SAT_FADE_GRE	SAT_FADE
#define SAT_FADE_CYA	SAT_FADE
#define SAT_FADE_BLU	SAT_FADE


//
// Definition of lower and upper region Luminance bounds (LUMI)
//
#define Y1				65 		// For 8 bit YUV data
#define Y2				165 	// For 8 bit YUV data
#define LUMI_FADE		35 		// For 8 bit YUV data

#define Y1_MAG			Y1
#define Y1_RED			Y1
#define Y1_YEL			Y1
#define Y1_GRE			Y1
#define Y1_CYA			Y1
#define Y1_BLU			Y1

#define Y2_MAG			Y2
#define Y2_RED			Y2
#define Y2_YEL			Y2
#define Y2_GRE			Y2
#define Y2_CYA			Y2
#define Y2_BLU			Y2

#define LUMI_FADE_MAG	LUMI_FADE
#define LUMI_FADE_RED	LUMI_FADE
#define LUMI_FADE_YEL	LUMI_FADE
#define LUMI_FADE_GRE	LUMI_FADE
#define LUMI_FADE_CYA	LUMI_FADE
#define LUMI_FADE_BLU	LUMI_FADE
#endif

#define ACM_NUM_OF_HW_REGIONS 8


//******************************************************************************
//  T Y P E    D E F I N I T I O N S
//******************************************************************************

// There are 8 HW ACM 3D regions at all in FLI8538 (Cortez Plus)
enum {
	REGION_MAGENTA = 0, // must start from 0 as used as an array index
	REGION_RED,
	REGION_YELLOW,
	REGION_GREEN,
	REGION_CYAN,
	REGION_BLUE,
	REGION_COLOR6,
	REGION_COLOR7,
	ACM3D_REGION_NUM
};

//******************************************************************************
//  T Y P E    D E F I N I T I O N S
//******************************************************************************
//
// Correction parameters definition struct, 9 params for each Region
//
typedef struct
{
    BYTE ThetaFadeLoRGainEnable; 
    BYTE ThetaFadeLoRValue;
    BYTE ThetaFadeLoRCutoff;
} VIP_ACMParameters_t;

typedef struct
{
    WORD W_HueCenter;
    WORD W_HueAperture;
    WORD W_R1;
    WORD W_R2;
    WORD W_Y1;
    WORD W_Y2;
    WORD W_HueFade;
    WORD W_SatFade;
    WORD W_LumiFade;
} VIP_ACMRegionDefinitions_t;

typedef struct
{
    SWORD SW_HueOffs;
    SWORD SW_HueGain;
    SWORD SW_SatOffs;
    WORD W_SatGain;
    SWORD SW_LumiOffs;
    WORD W_LumiGain;
    SWORD SW_U_Vect;
    SWORD SW_V_Vect;
    WORD W_Alpha;
} VIP_ACMRegionCorrectionParameters_t;

typedef struct
{
    LLD_TuningData_t descriptor; // descriptor
    VIP_ACMParameters_t ACMParams;
    BYTE RegionsNum;                //num of active regions
    VIP_ACMRegionDefinitions_t ACMRegionDefs[ACM_NUM_OF_HW_REGIONS];
    VIP_ACMRegionCorrectionParameters_t ACMRegionCorrectionParams[ACM_NUM_OF_HW_REGIONS];
} VQ_ACM_Parameters_t;

//
// Run-time Calculated Correction parameters definition struct, 5 params for each Region
//
typedef struct gmt_ACM3DCalcCorrParamsStruct
{
	WORD	W_1_RLo;		// 1/5...1/63, 0.10u, ACM_Zx_SATR1_SOFT
	WORD	W_1_RUp;		// 1/5...1/63, 0.10u, ACM_Zx_SATR2_SOFT
	WORD	W_1_YLo;		// 1/5...1/63, 0.10u, ACM_Zx_LUMI1_SOFT
	WORD	W_1_YUp;		// 1/5...1/63, 0.10u, ACM_Zx_LUMI2_SOFT
} gmt_ACM3DCalcCorrParams;

//******************************************************************************
//  G L O B A L    F U N C T I O N S
//******************************************************************************

extern VIP_ACMRegionDefinitions_t far * gPa_ACM3DRegionDefs;
extern VIP_ACMRegionCorrectionParameters_t far * gPa_ACM3DCorrectionParamDefs;

extern SBYTE	gPa_ACM3DSixAxisAdjusterArray[6][3];
extern BYTE gB_ACM3D_MaxSliderRange;  
extern SBYTE	ROM gma_SatLumGainOffsetArrayFromSatAdj[6][8];	
extern SBYTE	ROM gma_SatLumGainOffsetArrayFromLumAdj[6][8];
extern SBYTE	ROM gma_HueLumGainOffsetArrayFromHueAdj[6][8];

void gm_ACM3DInit( void );
void gm_ACM3DUpdateCtrl(BYTE B_Enable, BYTE B_WindowCtrl);
void LLD_ACMSetParameters(VIP_ACMParameters_t far * Params_p);
void gm_ACM3DSetRegionDefinitionTablePtr(VIP_ACMRegionDefinitions_t far * pt_stRegionDef);
void gm_ACM3DSetCorrectionParamsTablePtr(VIP_ACMRegionCorrectionParameters_t far * pt_stCorrParams);
/*
void gm_ACM3DUpdateSaturation(BYTE B_Region, WORD W_SaturationAdjuster);
void gm_ACM3DUpdateHue(BYTE B_Region, SWORD SW_HueAdjuster);
void gm_ACM3DUpdateLumaGain(BYTE B_Region, WORD W_LumaAdjuster);
void gm_ACM3DUpdateLumaOffs(BYTE B_Region, BYTE B_LumaAdjuster);
*/
#endif	// __ACM3D_H__


