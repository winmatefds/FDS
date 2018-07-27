///*****************************************
//  Copyright (C) 2009-2015
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <IT626X_sys.h>
//   @author Jau-chih.Tseng@ite.com.tw
//   @date   2015/03/11
//   @fileversion: IT6263_SAMPLE_V209
//******************************************/

#ifndef _CAT6611_SYS_H_
#define _CAT6611_SYS_H_
////////////////////////////////////////////////////////////////////////////////
// Internal Data Type
////////////////////////////////////////////////////////////////////////////////
#include "typedef.h"
//typedef enum tagHDMI_Aspec {
typedef enum {
    HDMI_4x3= 0, 
    HDMI_16x9=1
} HDMI_Aspec;

typedef enum tagHDMI_OutputColorMode {
//typedef enum  {
    HDMI_RGB444= 0 ,
    HDMI_YUV444=1,
    HDMI_YUV422=2
} HDMI_OutputColorMode ;

//typedef enum tagHDMI_Colorimetry {
typedef enum  {
    HDMI_ITU601= 0,
    HDMI_ITU709=1
} HDMI_Colorimetry ;

///////////////////////////////////////////////////////////////////////
// Output Mode Type
///////////////////////////////////////////////////////////////////////

#define RES_ASPEC_4x3 0
#define RES_ASPEC_16x9 1
#define F_MODE_REPT_NO 0
#define F_MODE_REPT_TWICE 1
#define F_MODE_REPT_QUATRO 3
#define F_MODE_CSC_ITU601 0
#define F_MODE_CSC_ITU709 1

#define Frame_Packing 0
#define Top_and_Botton 6
#define Side_by_Side 8

/*
typedef struct RxCapability{
    BYTE HDMI_VSDB[32];
    BYTE VideoMode ;
    BYTE VDOModeCount ;
    BYTE idxNativeVDOMode ;
    BYTE VDOMode[32] ;//32
    BYTE AUDDesCount ;
    AUDDESCRIPTOR AUDDes[8] ;
    ULONG IEEEOUI ;
    DCSUPPORT dc ;
    BYTE MaxTMDSClock ;
    LATENCY_SUPPORT lsupport ;
    BYTE V_Latency ;
    BYTE A_Latency ;
    BYTE V_I_Latency ;
    BYTE A_I_Latency ;
    SPK_ALLOC   SpeakerAllocBlk ;
    BYTE ValidCEA:1 ;
    BYTE ValidHDMI:1 ;
} RxCapability ;
*/

void InitIT626X_Instance(void);
void HDMITX_ChangeDisplayOption(HDMI_Video_Type VideoMode, HDMI_OutputColorMode OutputColorMode);
void HDMITX_SetOutput(void);
void HDMITX_DevLoopProc(void);
BOOL VideoModeDetect(void);
void ConfigfHdmiVendorSpecificInfoFrame(BYTE _3D_Stru);
#endif // _CAT6611_SYS_H_
