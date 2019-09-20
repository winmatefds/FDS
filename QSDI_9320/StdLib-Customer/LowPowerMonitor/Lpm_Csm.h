#ifndef __LPM_CSM_H__
#define __LPM_CSM_H__

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
// MODULE:      Lpm_Csm.h
//
// USAGE:       
//
//******************************************************************************

//****************************************************************************** 
//  G L O B A L    D E F I N I T I O N S 
//****************************************************************************** 
#define LPM_AUX_SEL_SHIFT                               0	
#define LPM_DVI_I2C_MUX_SEL_SHIFT                       1	
#define LPM_HDMI_I2C_MUX_SEL_SHIFT                      2
#define LPM_DP0_I2C_MUX_SEL_SHIFT                       4
#define LPM_DP1_I2C_MUX_SEL_SHIFT                       6
#define LPM_I2C_M_SEL_SHIFT								8
#define HDCP_MAP_OVERRIDE_SHIFT                         0


//Refer to Athena register spec v1.7 p205
typedef enum
{
	//LPM I2C MASTER USE
	MUX_DDC1_LpmI2cMaster,				//2:I2C master is configured for connection to external I2C slave device
	//GPROBE, ISP or TOOL USE
	MUX_DDC1_MisI2cSlv1,				//3:Allow ISP ON LPM DDC1 port
	MUX_DDC2_MisI2cSlv2,				//4:Allow ISP ON LPM DDC2 port
	MUX_DDC3_MisI2cSlv3,				//5:Allow ISP ON LPM DDC3 port
	MUX_DDC4_MisI2cSlv4,				//6:Allow ISP ON LPM DDC4 port	
	//DPRX0 AUX USE
	MUX_DDC1_MisDpC0Aux2I2c_I2cSlv1,	//7:I2C slave1 is used for Mission DP1 AUX2I2C DDC
	MUX_DDC1_MisDpC0Aux2I2c_ExtDev,		//8:External device on DDC1 por used for Mission: DP1 AUX2I2C DDC
	MUX_DDC1_LpmDpC0Aux2I2c_I2cSlv1,	//9:I2C slave1 is used for LPM DP1 AUX2I2C DDC
	MUX_DDC1_LpmDpC0Aux2I2c_ExtDev,		//10:External device on DDC1 port used for LPM: DP1 AUX2I2C DDC
	//DPRX1 AUX USE
	MUX_DDC3_MisDpC2Aux2I2c_I2cSlv3,	//11:I2C slave3 is used for Mission DP2 AUX2I2C DDC
	MUX_DDC3_MisDpC2Aux2I2c_ExtDev,		//12:External device on DDC3 port used for Mission DP2 AUX2I2C master
	MUX_DDC3_LpmDpC2Aux2I2c_I2cSlv3,	//13:I2C slave3 is used for DP2 AUX2I2C DDC
	MUX_DDC3_LpmDpC2Aux2I2c_ExtDev,		//14:External device on DDC3 port used for LPM: DP2 AUX2I2C DDC
	//HDMI USE
	MUX_DDC1_LpmI2cSlv1_MisHdmiCp,		//15:LPM_I2C_SLV1 & MIS_HDMI_HDCP_SLAVE on LPM_DDC1 PORT. EXT EDID OPTIONAL
	MUX_DDC2_LpmI2cSlv2_MisHdmiCp,		//16:LPM_I2C_SLV2 & MIS_HDMI_HDCP_SLAVE on LPM_DDC2 PORT. EXT EDID OPTIONAL
	MUX_DDC3_Lpmi2cSlv3_MisHdmiCp,		//17:LPM_I2C_SLV3 & MIS_HDMI_HDCP_SLAVE on LPM_DDC3 PORT. EXT EDID OPTIONAL
	//DVI USE
	MUX_DDC2_LpmI2cSlv2_MisDviCp,		//18:DVI is enabled. HDMI is not configured on i2C slave2 or DDC2 port. These are used by DVI	
	//VGA USE
	MUX_DDC4_LpmI2cSlv4,				//29:LPM: I2C slave4 is used with external master on DDC4 port
	MUX_Num,
} I2cMuxSelect;

//Mapping to registers LPM_CSM_I2C_MUX_CTRL and MFP_SPARE1
typedef struct 
{
	BYTE LpmI2cMSel;		//LPM_CSM_I2C_MUX_CTRL  bit[8]
	BYTE HdcpMapOverride;	//MFP_SPARE1  bit[1:0]	
	BYTE LpmAuxSel;			//LPM_CSM_I2C_MUX_CTRL  bit[0]
	BYTE LpmDp1I2cMuxSel;	//LPM_CSM_I2C_MUX_CTRL  bit[5:4]
	BYTE LpmDp2I2cMuxSel;	//LPM_CSM_I2C_MUX_CTRL  bit[7:6]
	BYTE LpmHdmiI2cMuxSel;	//LPM_CSM_I2C_MUX_CTRL  bit[3:2]		
	BYTE LpmDviI2cMuxSel;	//LPM_CSM_I2C_MUX_CTRL  bit[1]
} I2cMuxTableParams;

//****************************************************************************** 
//  G L O B A L    V A R I A B L E S 
//****************************************************************************** 


//****************************************************************************** 
//  G L O B A L    F U N C T I O N    P R O T O T Y P E S 
//****************************************************************************** 
void Lpm_CsmI2CMuxSelect (I2cMuxSelect MuxSel);

#endif //__LPM_CSM_H__