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
//=============================================================================
//
// MODULE:      Lpm_Csm.c
//
// USAGE:		This module contains common functions for CSM.  
//
//******************************************************************************

//******************************************************************************
//  I N C L U D E    F I L E S
//******************************************************************************
#include ".\LowPowerMonitor\Lpm_All.h"

//****************************************************************************** 
//  G L O B A L    D E F I N I T I O N S 
//****************************************************************************** 

//******************************************************************************
//  F U N C T I O N    P R O T O T Y P E S
//******************************************************************************

//****************************************************************************** 
//  G L O B A L    V A R I A B L E S 
//****************************************************************************** 


//******************************************************************************
//  C O D E
//******************************************************************************
//******************************************************************************
// FUNCTION       : void Lpm_CsmI2CMuxSelect (I2cMuxSelect MuxSel)
// USAGE          : Control CSM I2C MUX path
// INPUT          :  None
// OUTPUT         : None
// GLOBALS        : 
// USED_REGS      : None
// PRE_CONDITION  :              
// POST_CONDITION :                
//******************************************************************************
void Lpm_CsmI2CMuxSelect (I2cMuxSelect MuxSel)
{
   //Refer to Athena register spec v1.7 p205
   switch(MuxSel)
   {
      //LPM I2C MASTER USE
      case MUX_DDC1_LpmI2cMaster: // 2
         gm_SetRegBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_I2C_M_SEL);
      break;
      //GPROBE, ISP or TOOL USE
      case MUX_DDC1_MisI2cSlv1: // 3
         gm_ClearRegBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_I2C_M_SEL);
         gm_ClearRegBitsWord(MFP_SPARE1, HDCP_MAP_OVERRIDE);
      break;
      case MUX_DDC2_MisI2cSlv2: // 4
      case MUX_DDC4_MisI2cSlv4: // 6
         gm_ClearAndSetBitsWord(MFP_SPARE1, HDCP_MAP_OVERRIDE, 1);
      break;
      case MUX_DDC3_MisI2cSlv3: // 5
         gm_ClearAndSetBitsWord(MFP_SPARE1, HDCP_MAP_OVERRIDE, 2);
      break;
      //DPRX0 AUX USE
      case MUX_DDC1_MisDpC0Aux2I2c_I2cSlv1: // 7
         gm_ClearRegBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_I2C_M_SEL);
         gm_SetRegBitsWord(MFP_SPARE1, HDCP_MAP_OVERRIDE);
         gm_ClearRegBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_AUX_SEL);
         gm_ClearAndSetBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_DP0_I2C_MUX_SEL, 2<<LPM_DP0_I2C_MUX_SEL_SHIFT);
      break;
      case MUX_DDC1_MisDpC0Aux2I2c_ExtDev: // 8
         gm_ClearRegBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_I2C_M_SEL);
         gm_SetRegBitsWord(MFP_SPARE1, HDCP_MAP_OVERRIDE);			
         gm_ClearRegBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_AUX_SEL);
         gm_SetRegBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_DP0_I2C_MUX_SEL);
      break;      
      case MUX_DDC1_LpmDpC0Aux2I2c_I2cSlv1: // 9
         gm_ClearRegBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_I2C_M_SEL);
         gm_SetRegBitsWord(MFP_SPARE1, HDCP_MAP_OVERRIDE);
         gm_SetRegBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_AUX_SEL);
         gm_ClearAndSetBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_DP0_I2C_MUX_SEL, 2<<LPM_DP0_I2C_MUX_SEL_SHIFT);
      break;
      case MUX_DDC1_LpmDpC0Aux2I2c_ExtDev: // 10
         gm_ClearRegBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_I2C_M_SEL);
         gm_SetRegBitsWord(MFP_SPARE1, HDCP_MAP_OVERRIDE);			
         gm_SetRegBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_AUX_SEL);
         gm_SetRegBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_DP0_I2C_MUX_SEL);
      break;		
      //DPRX1 AUX USE
      case MUX_DDC3_MisDpC2Aux2I2c_I2cSlv3: // 11
         gm_ClearRegBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_AUX_SEL);
         gm_SetRegBitsWord(MFP_SPARE1, HDCP_MAP_OVERRIDE);			
         gm_ClearAndSetBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_DP1_I2C_MUX_SEL, 2<<LPM_DP1_I2C_MUX_SEL_SHIFT);
      break;
      case MUX_DDC3_MisDpC2Aux2I2c_ExtDev: // 12
         gm_ClearRegBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_AUX_SEL);
         gm_SetRegBitsWord(MFP_SPARE1, HDCP_MAP_OVERRIDE);			
         gm_SetRegBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_DP1_I2C_MUX_SEL);
      break;      
      case MUX_DDC3_LpmDpC2Aux2I2c_I2cSlv3: // 13
         gm_SetRegBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_AUX_SEL);
         gm_SetRegBitsWord(MFP_SPARE1, HDCP_MAP_OVERRIDE);			
         gm_ClearAndSetBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_DP1_I2C_MUX_SEL, 2<<LPM_DP1_I2C_MUX_SEL_SHIFT);
      break;
      case MUX_DDC3_LpmDpC2Aux2I2c_ExtDev: // 14
         gm_SetRegBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_AUX_SEL);
         gm_SetRegBitsWord(MFP_SPARE1, HDCP_MAP_OVERRIDE);			
         gm_SetRegBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_DP1_I2C_MUX_SEL);
      break;	
      //HDMI USE
      case MUX_DDC1_LpmI2cSlv1_MisHdmiCp: // 15
         gm_ClearRegBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_I2C_M_SEL);
         gm_SetRegBitsWord(MFP_SPARE1, HDCP_MAP_OVERRIDE);			
         gm_ClearRegBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_AUX_SEL);
         gm_ClearRegBitsWord(LPM_CSM_I2C_MUX_CTRL, 2<<LPM_DP0_I2C_MUX_SEL_SHIFT);
         gm_ClearAndSetBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_HDMI_I2C_MUX_SEL, 1<<LPM_HDMI_I2C_MUX_SEL_SHIFT);
      break;		
      case MUX_DDC2_LpmI2cSlv2_MisHdmiCp: // 16
         gm_SetRegBitsWord(MFP_SPARE1, HDCP_MAP_OVERRIDE);			
         gm_ClearAndSetBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_HDMI_I2C_MUX_SEL, 2<<LPM_HDMI_I2C_MUX_SEL_SHIFT);
      break;			
      case MUX_DDC3_Lpmi2cSlv3_MisHdmiCp: // 17
         gm_SetRegBitsWord(MFP_SPARE1, HDCP_MAP_OVERRIDE);			
         gm_ClearRegBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_AUX_SEL);
         gm_ClearRegBitsWord(LPM_CSM_I2C_MUX_CTRL, 2<<LPM_DP1_I2C_MUX_SEL_SHIFT);
         gm_SetRegBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_HDMI_I2C_MUX_SEL);
      break;	
      //DVI USE
      case MUX_DDC2_LpmI2cSlv2_MisDviCp: // 18
         gm_SetRegBitsWord(MFP_SPARE1, HDCP_MAP_OVERRIDE);			
         gm_ClearAndSetBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_HDMI_I2C_MUX_SEL, 3<<LPM_HDMI_I2C_MUX_SEL_SHIFT);
         gm_SetRegBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_DVI_I2C_MUX_SEL);
      break;
      //VGA USE
      case MUX_DDC4_LpmI2cSlv4: // 19
         gm_SetRegBitsWord(MFP_SPARE1, HDCP_MAP_OVERRIDE);			
      break;
      default:
      break;
   }
}

//******************************************************************************

