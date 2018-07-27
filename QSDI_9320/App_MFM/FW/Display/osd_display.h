/*
	$Workfile:   osd_display.h  $
	$Revision: 1.18 $
	$Date: 2013/04/08 02:01:38 $
*/
#ifndef __OSD_DISPLAY_H__
#define __OSD_DISPLAY_H__
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
// MODULE:      osd_display.h
//
// USAGE:
//
//******************************************************************************

typedef enum ImageSchemeEnum
{
   USER=0,
   VIVID,
   CINEMA,
   GAME,
   SPORT,
	NUM_IMAGE_SCHEMES,
	LINEAR,
} ImageSchemes;

#define VWD_SCHEME     100      // Color scheme for VWD

//
// Color Space definition
//
typedef enum COLOR_SPACE
{
   ColorSpace_sRGB = 0,
   ColorSpace_Adobe98,
   ColorSpace_XvYcc,
	ColorSpace_DICOM,
   MAX_COLOR_SPACE
} COLOR_SPACE;



//
// Color Temperature definition
//
typedef enum COLOR_TEMP
{
   DEFAULT_CTEMP = 0,
	CTEMP_4000K,
   CTEMP_5000K,
   CTEMP_6500K,
   CTEMP_7500K,
	CTEMP_8200K,
   CTEMP_9300K,
	CTEMP_10000K,
	CTEMP_11500K,
   USER_TEMP,
   MAX_COLOR_TEMP
} COLOR_TEMP;

//
// Image scheme adjustment parameters
//
typedef struct
{
   SBYTE	sB_Brightness;
   WORD	W_Contrast;
   WORD	W_Saturation;
} gmt_ColorScheme;

//
// Flesh tones compensation levels
//
typedef enum FleshToneLevel
{
   FT_OFF = 0,
   FT_LOW,
   FT_MODERATE,
   FT_HIGH,
   NUM_FLESH_TONE
} FleshToneLevel;

//
// Simultaneous Actions in diferent thread
//
typedef enum SimultaneousAction
{
   ERASE_BLOCKED_CHANNEL,
   DISPLAY_BLOCKED_CHANNEL
} SimultaneousAction;

//
// CSC 3x3 matrix selection
//
typedef enum
{
   CSC_FULL_EXPANSION,
   CSC_LINEAR_EXPANSION,
   CSC_PARTIAL_EXPANSION,
   CSC_UNITY_MATRIX,
} CSC_MATRIX;


//
// UART Switch Selection
//
typedef enum
{
	UART_TO_ATHENA_R,
	UART_TO_ATHENA_L,
   UART_TO_ATHENA_FE,
   UART_TO_VEGA,
}UART_TO;

//
// PBP SETTING
//
typedef enum
{
	PBP_OFF,
	PBP_DP1_HDMI,
   PBP_DP2_HDMI,
   PBP_HDMI_DP1,
   PBP_HDMI_DP2,
}PBP_SETTING;

// OSD status
//

//******************************************************************************
//  E X T E R N A L    F U N C T I O N    P R O T O T Y P E S
//******************************************************************************
void AdjustPanelBacklight(BYTE B_Percent);
void AdjustBrightness(void);
void AdjustContrast(void);
void AdjustSaturation(void);
void AdjustHue(void);
void AdjustSharpness(void);
void AdjustSharpnessRGB(void);
void Adjust3DNR(void);
void AdjustMPEGNR(void);
void AdjustGNR(void);

void AdjustRedGain(void);
void AdjustGreenGain(void);
void AdjustBlueGain(void);

void AdjustRedOff(void);
void AdjustGreenOff(void);
void AdjustBlueOff(void);

void AdjustScartRedGain(void);
void AdjustScartGreenGain(void);
void AdjustScartBlueGain(void);

void AdjustScartRedOff(void);
void AdjustScartGreenOff(void);
void AdjustScartBlueOff(void);

BOOL IsColorTempAvailable(BYTE ColorTempRequest);
void AdjustColorTemp(void);
void ColorSpace_Setup(void);
void AdjustColorSpace(void);
void AdjustScartRGBViaOverlay(void);

void AdjustImageScheme(void);
void AdjustFleshTone(void);
void FreezeDisplay(void);
void ToggleFreezeVar(void);

void AdjustDynamicContrast(void);
BYTE GetMaxAccTable (void);
void AdjustFleshToneLevel(void);
BOOL IsCCDefined(void);
BOOL IsTTXDefined(void);
BOOL IsNoTunerDefined(void);
void ToggleInfoDisplay(void);
BOOL IsHDMIDefined(void);

void SetOsdBitFlag(void);
void ClrOsdBitFlag(void);
#define OsdTtxOnBit		BIT0
#define OsdTtxOnFlag		(UserPrefOsdBitFlags & OsdTtxOnBit)
BOOL IsAutoScanDefined(void);
void SimultaneousActionInDifferentThread(void);
void SetOsdReadyForPowerDown(void);
void SetOsdRequestForPowerUp(void);
void SelectCscMatrix(CSC_MATRIX Mode);


BOOL IsNoSync(void);
BOOL IsNoCable(void);
BOOL IsInvalidMode(void);
BOOL ToggleMenuStyle(void);
BOOL DisplayInfoCond(BOOL mainDisplay, int numnum);
BOOL IsPipFocusRight(void);
BOOL IsPipNoSync(void);
void ScheduleNoSignalMsg(void);
void CancelNoSignalMsg(void);
BOOL IsAUDIO_LS_HP_INDEPENDENTDefined(void);

BOOL IsGameModeApplicable(void);
void AdjustGameMode(void);
void AdjustXvYcc(void);
void AdjustDCR(void);
void DCR_Handler( void);
void PublicDisplaySetup(void);

#endif

