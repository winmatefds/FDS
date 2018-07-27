/*
	$Workfile:   ImageSchemes.c  $
	$Revision: 1.10 $
	$Date: 2012/04/19 03:18:38 $
*/
#define __IMAGE_SCHEMES_C__

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
// MODULE:      adj_display.c
//
//******************************************************************************

//******************************************************************************
//  I N C L U D E    F I L E S
//******************************************************************************

#include "System\all.h"

//*****************************************************************************
// G L O B A L S
//*****************************************************************************

PortSchemeDependentUserPrefType ROM SchemeFactoryDefaultInROM[NUM_IMAGE_SCHEMES] =
{
   {	// 0 Normal
      0,	// Acm3BlueLuma
      0,	// Acm3CyanLuma
      0,	// Acm3GreenLuma
      0,	// Acm3MagentaLuma
      0,	// Acm3RedLuma
      0,	// Acm3YellowLuma
      0,	// Acm3BlackLuma
      1,	// Acm3WhiteLuma
      
      0,	// Acm3BlueHue
      0,	// Acm3CyanHue
      0,	// Acm3GreenHue
      0,	// Acm3MagentaHue
      0,	// Acm3RedHue
      0,	// Acm3YellowHue
      0, // Acm3BlackHue
      1, // Acm3WhiteHue
      
      50,	// Acm3BlueSat
      50,	// Acm3CyanSat
      50,	// Acm3GreenSat
      50,	// Acm3MagentaSat
      50,	// Acm3RedSat
      50,	// Acm3YellowSat
      0,	// Acm3BlackSat
      0,	// Acm3WhiteSat
      
      0,	// AcmBlueHue
      0,	// AcmCyanHue
      0,	// AcmGreenHue
      0,	// AcmMagentaHue
      0,	// AcmRedHue
      0,	// AcmYellowHue
      0,	// AcmBlueSat
      0,	// AcmCyanSat
      0,	// AcmGreenSat
      0,	// AcmMagentaSat
      0,	// AcmRedSat
      0,	// AcmYellowSat
      0,	// Brightness
      256,	// Contrast
      256,	// Saturation
      0,		// Hue
      256,	// UserRedGain
      256,	// UserGreenGain
      256,	// UserBlueGain
      0,	// UserRedOff
      0,	// UserGreenOff
      0,	// UserBlueOff
      USER_TEMP,	// ColorTemp
      GAMMA_TABLE_OFF, // Gamma Table 
      USER, //MFMImageScheme
      0,
      0,
      0,
   },
   {	// 1  Vivid
      0,	// Acm3BlueLuma
      0,	// Acm3CyanLuma
      0,	// Acm3GreenLuma
      0,	// Acm3MagentaLuma
      0,	// Acm3RedLuma
      0,	// Acm3YellowLuma
      0,	// Acm3BlackLuma
      1,	// Acm3WhiteLuma      
      
      0,	// Acm3BlueHue
      0,	// Acm3CyanHue
      0,	// Acm3GreenHue
      0,	// Acm3MagentaHue
      0,	// Acm3RedHue
      0,	// Acm3YellowHue
      0, // Acm3BlackHue
      0, // Acm3WhiteHue      
      
      50,	// Acm3BlueSat
      50,	// Acm3CyanSat
      50,	// Acm3GreenSat
      50,	// Acm3MagentaSat
      50,	// Acm3RedSat
      50,	// Acm3YellowSat
      0,	// Acm3BlackSat
      0,	// Acm3WhiteSat
      
      0,	// AcmBlueHue
      0,	// AcmCyanHue
      0,	// AcmGreenHue
      0,	// AcmMagentaHue
      0,	// AcmRedHue
      0,	// AcmYellowHue
      0,	// AcmBlueSat
      0,	// AcmCyanSat
      0,	// AcmGreenSat
      0,	// AcmMagentaSat
      0,	// AcmRedSat
      0,	// AcmYellowSat
      0,
      256,	// Contrast
      317,
      0,	// Hue
      256,	// UserRedGain
      256,	// UserGreenGain
      256,	// UserBlueGain
      0,	// UserRedOff
      0,	// UserGreenOff
      0,	// UserBlueOff
      CTEMP_9300K,	// ColorTemp
      GAMMA_TABLE_22,  //Gamma Table
      VIVID, //MFMImageScheme
      0,
      0,
      0,
   },
   {	// 2 Cinema
      0,	// Acm3BlueLuma
      0,	// Acm3CyanLuma
      0,	// Acm3GreenLuma
      0,	// Acm3MagentaLuma
      0,	// Acm3RedLuma
      0,	// Acm3YellowLuma
      0,	// Acm3BlackLuma
      1,	// Acm3WhiteLuma      
      
      0,	// Acm3BlueHue
      0,	// Acm3CyanHue
      0,	// Acm3GreenHue
      0,	// Acm3MagentaHue
      0,	// Acm3RedHue
      0,	// Acm3YellowHue
      0, // Acm3BlackHue
      0, // Acm3WhiteHue
      
      50,  // Acm3BlueSat
      50,  // Acm3CyanSat
      50,  // Acm3GreenSat
      50,  // Acm3MagentaSat
      50,  // Acm3RedSat
      50,  // Acm3YellowSat
      0,	// Acm3BlackSat
      0,	// Acm3WhiteSat
      
      0,	// AcmBlueHue
      0,	// AcmCyanHue
      0,	// AcmGreenHue
      0,	// AcmMagentaHue
      0,	// AcmRedHue
      0,	// AcmYellowHue
      0,	// AcmBlueSat
      0,	// AcmCyanSat
      0,	// AcmGreenSat
      0,	// AcmMagentaSat
      0,	// AcmRedSat
      0,	// AcmYellowSat
      0,	// Brightness
      256,	// Contrast
      317,	// Saturation
      0,	// Hue
      256,	// UserRedGain
      256,	// UserGreenGain
      256,	// UserBlueGain
      0,	// UserRedOff
      0,	// UserGreenOff
      0,	// UserBlueOff
      CTEMP_6500K,	// ColorTemp
      GAMMA_TABLE_22, // Gamma table
      CINEMA, //MFMImageScheme
      0,
      0,
      0,
   },
   { // 3 Game
      0,	// Acm3BlueLuma
      0,	// Acm3CyanLuma
      0,	// Acm3GreenLuma
      0,	// Acm3MagentaLuma
      0,	// Acm3RedLuma
      0,	// Acm3YellowLuma
      0,	// Acm3BlackLuma
      1,	// Acm3WhiteLuma      
      
      0,	// Acm3BlueHue
      0,	// Acm3CyanHue
      0,	// Acm3GreenHue
      0,	// Acm3MagentaHue
      0,	// Acm3RedHue
      0,	// Acm3YellowHue
      0, // Acm3BlackHue
      0, // Acm3WhiteHue
      
      50,	// Acm3BlueSat
      50,	// Acm3CyanSat
      50,	// Acm3GreenSat
      50,	// Acm3MagentaSat
      50,	// Acm3RedSat
      50,	// Acm3YellowSat
      0,	// Acm3BlackSat
      0,	// Acm3WhiteSat
      
      0,	// AcmBlueHue
      0,	// AcmCyanHue
      0,	// AcmGreenHue
      0,	// AcmMagentaHue
      0,	// AcmRedHue
      0,	// AcmYellowHue
      0,	// AcmBlueSat
      0,	// AcmCyanSat
      0,	// AcmGreenSat
      0,	// AcmMagentaSat
      0,	// AcmRedSat
      0,	// AcmYellowSat
      0,	// Brightness
      256,	// Contrast
      256,	// Saturation
      0,	// Hue
      256,	// UserRedGain
      256,	// UserGreenGain
      256,	// UserBlueGain
      0,	// UserRedOff
      0,	// UserGreenOff
      0,	// UserBlueOff
      CTEMP_6500K,	// ColorTemp
      GAMMA_TABLE_22, // Gamma Table
      GAME, //MFMImageScheme
      0,
      0,
      0,
   },
   {	// 4  Sport
      0,	// Acm3BlueLuma
      0,	// Acm3CyanLuma
      0,	// Acm3GreenLuma
      0,	// Acm3MagentaLuma
      0,	// Acm3RedLuma
      0,	// Acm3YellowLuma
      0,	// Acm3BlackLuma
      1,	// Acm3WhiteLuma      
      
      0,	// Acm3BlueHue
      0,	// Acm3CyanHue
      0,	// Acm3GreenHue
      0,	// Acm3MagentaHue
      0,	// Acm3RedHue
      0,	// Acm3YellowHue
      0, // Acm3BlackHue
      0, // Acm3WhiteHue
      
      50,	// Acm3BlueSat
      50,	// Acm3CyanSat
      50,	// Acm3GreenSat
      50,	// Acm3MagentaSat
      50,	// Acm3RedSat
      50,	// Acm3YellowSat
      0,	// Acm3BlackSat
      0,	// Acm3WhiteSat
      
      0,	// AcmBlueHue
      0,	// AcmCyanHue
      0,	// AcmGreenHue
      0,	// AcmMagentaHue
      0,	// AcmRedHue
      0,	// AcmYellowHue
      0,	// AcmBlueSat
      0,	// AcmCyanSat
      0,	// AcmGreenSat
      0,	// AcmMagentaSat
      0,	// AcmRedSat
      0,	// AcmYellowSat
      0,	// Brightness
      256,	// Contrast
      256,	// Saturation
      0,	// Hue
      256,	// UserRedGain
      256,	// UserGreenGain
      256,	// UserBlueGain
      0,	// UserRedOff
      0,	// UserGreenOff
      0,	// UserBlueOff
      CTEMP_9300K,	// ColorTemp
      GAMMA_TABLE_22, // Gamma Table
      SPORT, //MFMImageScheme
      0,
      0,
      0,
   },
   //{	// 5 Custom
   //	0,	// Acm3BlueLuma
   //	0,	// Acm3CyanLuma
   //	0,	// Acm3GreenLuma
   //	0,	// Acm3MagentaLuma
   //	0,	// Acm3RedLuma
   //	0,	// Acm3YellowLuma
   // 0,	// Acm3BlackLuma
   // 0,	// Acm3WhiteLuma   
   
   //	0,	// Acm3BlueHue
   //	0,	// Acm3CyanHue
   //	0,	// Acm3GreenHue
   //	0,	// Acm3MagentaHue
   //	0,	// Acm3RedHue
   //	0,	// Acm3YellowHue
   // 0, // Acm3BlackHue
   // 0, // Acm3WhiteHue
   
   //	0,	// Acm3BlueSat
   //	0,	// Acm3CyanSat
   //	0,	// Acm3GreenSat
   //	0,	// Acm3MagentaSat
   //	0,	// Acm3RedSat
   //	0,	// Acm3YellowSat
   // 0,	// Acm3BlackSat
   // 0,	// Acm3WhiteSat
   
   //	0,	// AcmBlueHue
   //	0,	// AcmCyanHue
   //	0,	// AcmGreenHue
   //	0,	// AcmMagentaHue
   //	0,	// AcmRedHue
   //	0,	// AcmYellowHue
   //	0,	// AcmBlueSat
   //	0,	// AcmCyanSat
   //	0,	// AcmGreenSat
   //	0,	// AcmMagentaSat
   //	0,	// AcmRedSat
   //	0,	// AcmYellowSat
   //	0,	// Brightness
   //	256,	// Contrast
   //	256,	// Saturation
   //	0,		// Hue
   //	256,	// UserRed
   //	256,	// UserGreen
   //	256,	// UserBlue
   //	CTEMP_6500K,	// ColorTemp
   //	CUSTOM
   //0,
   //0,
   //0,
   //}
};


