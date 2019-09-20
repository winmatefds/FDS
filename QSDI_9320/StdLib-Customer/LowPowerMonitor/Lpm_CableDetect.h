#ifndef __LPM_CABLEDETECT_H__
#define __LPM_CABLEDETECT_H__

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
// MODULE:      Lpm_CableDetect.h
//
// USAGE:       
//
//******************************************************************************

#define 	CableHandlerPeriod		50			//How long does cable handler run in main loop 
#define 	LpmGpio0CheckMask		0x00E7		//Mask which GPIO bit for checking , GPIO3,5,6,7
#define 	LpmGpio1CheckMask		0x0000		//No use 


//------------------ GPADC values for cable detection ------------------
//---- Following 8bit ADC values are converted from 10bit raw ADC values ----
#define LBADC_DELTA           7 // Tolerance
#define LBADC_EQUAL(a,b) ((a) < (b)+LBADC_DELTA) && ((a) > (b)-LBADC_DELTA)

//EV1 board
#define EV1_LBADC_HDMI_CABLE        208 // ADC value for only DVI cable plugged
#define EV1_LBADC_VGA_CABLE         157 // ADC value for only VGA cable plugged
#define EV1_LBADC_HDMI_VGA_CABLE    123 // ADC value for both DVI and VGA cable plugged

//EV1 board
#define EV2_LBADC_HDMI_B_CABLE      208 // ADC value for only DVI cable plugged
#define EV2_LBADC_VGA_CABLE         157 // ADC value for only VGA cable plugged
#define EV2_LBADC_HDMI_B_VGA_CABLE  123 // ADC value for both DVI and VGA cable plugged

//EV3 board
#define EV3_LBADC_VGA_CABLE         123 // ADC value for only VGA cable plugged

//RD1 & RD2 board
#define RD1_LBADC_DVI_CABLE         208 // ADC value for only DVI cable plugged
#define RD1_LBADC_VGA_CABLE         157 // ADC value for only VGA cable plugged
#define RD1_LBADC_DVI_VGA_CABLE     123 // ADC value for both DVI and VGA cable plugged

//RD3 board
#define RD3_LBADC_DVI_CBL              238 // ADC value for DVI cable plugged only
#define RD3_LBADC_VGA_CBL              206 // ADC value for VGA cable plugged only
#define RD3_LBADC_DVI_CBL5V            188 // ADC value for DVI cable plugged+DVI_5V valid only
#define RD3_LBADC_VGA_CBL5V            127 // ADC value for DVI cable plugged & VGA cable plugged+VGA_5V valid
#define RD3_LBADC_DVI_CBL_VGA_CBL      171 // ADC value for DVI cable plugged & VGA cable plugged
#define RD3_LBADC_DVI_CBL_VGA_CBL5V    113 // ADC value for DVI cable plugged & VGA cable plugged+VGA_5V valid
#define RD3_LBADC_DVI_CBL5V_VGA_CBL    143 // ADC value for DVI cable plugged+DVI_5V valid & VGA cable plugged
#define RD3_LBADC_DVI_CBL5V_VGA_CBL5V  100 // ADC value for DVI cable plugged+DVI_5V valid & VGA cable plugged+VGA_5V valid
#define RD3_LBADC_HDMI_CBL             211 // ADC value for HDMI cable plugged only
#define RD3_LBADC_HDMI_CBL5V           159 // ADC value for HDMI cable plugged+HDMI_5V valid


#define Lpm_IsHdmiDviSrcPwrValid    Lpm_IsCPhySrcPwrValid
#define Lpm_IsHdmiDviCablePlugged   Lpm_IsCPhyCablePlugged

void Lpm_CableDetectHandlerInit(void);
void Lpm_CableDetectHandler(void);
BYTE Lpm_IsDPRxSrcPwrValid(DPRX_SEL DPRxSel);
BYTE Lpm_IsDPRxCablePlugged(DPRX_SEL DPRxSel);
BYTE Lpm_IsCPhySrcPwrValid(gmt_CPhyNum CPhyNum);
BYTE Lpm_IsCPhyCablePlugged(gmt_CPhyNum CPhyNum);
BYTE Lpm_IsVGASrcPwrValid(void);
BYTE Lpm_IsVGACablePlugged(void);


#endif //__LPM_CABLEDETECT_H__
