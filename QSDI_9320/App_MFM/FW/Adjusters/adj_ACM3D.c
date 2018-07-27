#define __ADJ_ACM3D_C__
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
// MODULE:      adj_ACM3D.c
//
// USAGE:      This module contains adjusters for ACM3D functions.
//
//******************************************************************************

//******************************************************************************
//  I N C L U D E    F I L E S
//******************************************************************************

#include "System\All.h"
#include "StdLibInc\gm_Register.h"
#include "StdLibInc\gm_ACM3D.h"

#if (FEATURE_ACM_3D == ENABLE)


//******************************************************************************
//  D E F I N E S
//******************************************************************************

//#define _DEBUG
#ifdef _DEBUG
#define msg(a)            gm_Print(a,0)
#define msg_i(a,b)         gm_Print(a,b)
#else
#define msg(a)
#define msg_i(a,b)
#endif

//******************************************************************************
//  G L O B A L    V A R I A B L E S
//******************************************************************************

static void gmp_ACM3DUpdate(void);

//******************************************************************************
//  C O D E
//******************************************************************************

//******************************************************************************
// FUNCTION      :   void ACM3D_Init(void)
//
// USAGE      :   Initialize the ACM3D adjuster.
//
// INPUT      :   None
//
// OUTPUT      :   None
//
// GLOBALS      :   ACM_Params_Default            (R)
//
// USED_REGS   :   None
//
// PRE_CONDITION   :
//
// POST_CONDITION   :
//******************************************************************************
void ACM3D_Init(void)
{
   #define ACM3_THFADE_R_CUTOFF_OFFSET 8
   //gm_SetRegBitsWord(ACM3_CTRL, ACM3_OVERLAP_EN); 
   gm_SetRegBitsWord(ACM3_CTRL, ACM3_OVERLAP_EN | ACM3_THFADE_LORGAIN_
                                |(8<<ACM3_THFADE_LOR_VAL_SHIFT)
                                |(112<<ACM3_THFADE_R_CUTOFF_OFFSET));   

   // Set Parameter
   LLD_ACMSetParameters((VIP_ACMParameters_t far *)&ACM_Params_Default.ACMParams);
   // setup default regions and correction params, but do not enable
	gm_ACM3DSetRegionDefinitionTablePtr((VIP_ACMRegionDefinitions_t far *)&ACM_Params_Default.ACMRegionDefs);
	gm_ACM3DSetCorrectionParamsTablePtr((VIP_ACMRegionCorrectionParameters_t far *)&ACM_Params_Default.ACMRegionCorrectionParams);

   gm_ACM3DInit();

   if (IsAcmUsed())
   {
      AdjustACM3DColorScheme();
   }
}

//******************************************************************************
//
// FUNCTION   :   static void ACM3DSetColorScheme(BYTE B_ColorScheme)
//
// USAGE   :   Selects proper ACM3D tables based on Scheme
//
// INPUT   :   None
//
// OUTPUT   :   None
//
// GLOBALS   :   None
//
// USED_REGS:   None
//
// PRE-CONDITION:   None
//
//******************************************************************************
void ACM3DSetColorScheme(BYTE B_ColorScheme)
{
   if((B_ColorScheme >= NUM_ACM3D_COLOR_SCHEMES) && (B_ColorScheme != VWD_ACM3D_SCHEME))
      return;

   switch ( B_ColorScheme )
   {
      case OFF_ACM3D_SCHEME:
      default:
         // to set default regions for the case when separate registers are adjusted
         LLD_ACMSetParameters((VIP_ACMParameters_t far *)&ACM_Params_Default.ACMParams);
      	gm_ACM3DSetRegionDefinitionTablePtr((VIP_ACMRegionDefinitions_t far *)&ACM_Params_Default.ACMRegionDefs);
      	gm_ACM3DSetCorrectionParamsTablePtr((VIP_ACMRegionCorrectionParameters_t far *)&ACM_Params_Default.ACMRegionCorrectionParams);         
         break;
         
      case CINEMA_ACM3D_SCHEME:
         LLD_ACMSetParameters((VIP_ACMParameters_t far *)&ACM_Params_Cinema.ACMParams);
      	gm_ACM3DSetRegionDefinitionTablePtr((VIP_ACMRegionDefinitions_t far *)&ACM_Params_Cinema.ACMRegionDefs);
      	gm_ACM3DSetCorrectionParamsTablePtr((VIP_ACMRegionCorrectionParameters_t far *)&ACM_Params_Cinema.ACMRegionCorrectionParams);         
         break;
         
      case GAME_ACM3D_SCHEME:
         LLD_ACMSetParameters((VIP_ACMParameters_t far *)&ACM_Params_Game.ACMParams);
      	gm_ACM3DSetRegionDefinitionTablePtr((VIP_ACMRegionDefinitions_t far *)&ACM_Params_Game.ACMRegionDefs);
      	gm_ACM3DSetCorrectionParamsTablePtr((VIP_ACMRegionCorrectionParameters_t far *)&ACM_Params_Game.ACMRegionCorrectionParams);         
         break;
         
      case VIVID_ACM3D_SCHEME:
         LLD_ACMSetParameters((VIP_ACMParameters_t far *)&ACM_Params_Vivid.ACMParams);
      	gm_ACM3DSetRegionDefinitionTablePtr((VIP_ACMRegionDefinitions_t far *)&ACM_Params_Vivid.ACMRegionDefs);
      	gm_ACM3DSetCorrectionParamsTablePtr((VIP_ACMRegionCorrectionParameters_t far *)&ACM_Params_Vivid.ACMRegionCorrectionParams);         
         break;
         
      case SPORTS_ACM3D_SCHEME:
         LLD_ACMSetParameters((VIP_ACMParameters_t far *)&ACM_Params_Sport.ACMParams);
      	gm_ACM3DSetRegionDefinitionTablePtr((VIP_ACMRegionDefinitions_t far *)&ACM_Params_Sport.ACMRegionDefs);
      	gm_ACM3DSetCorrectionParamsTablePtr((VIP_ACMRegionCorrectionParameters_t far *)&ACM_Params_Sport.ACMRegionCorrectionParams);         
         break;
         
      case VWD_ACM3D_SCHEME:
         LLD_ACMSetParameters((VIP_ACMParameters_t far *)&ACM_Params_VWD.ACMParams);
      	gm_ACM3DSetRegionDefinitionTablePtr((VIP_ACMRegionDefinitions_t far *)&ACM_Params_VWD.ACMRegionDefs);
      	gm_ACM3DSetCorrectionParamsTablePtr((VIP_ACMRegionCorrectionParameters_t far *)&ACM_Params_VWD.ACMRegionCorrectionParams);
         break;
   }
}

static void gmp_ACM3DUpdate(void)
{
   gm_ACM3DUpdateCtrl(ACM3_EN, ACC_ACM_FULLSCREEN);
}
#endif   // (FEATURE_ACM_3D == ENABLE)
//*********************************  END  **************************************

