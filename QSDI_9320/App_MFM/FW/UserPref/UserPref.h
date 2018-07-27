/*
	$Workfile:   UserPref.h  $
	$Revision: 1.8 $
	$Date: 2012/09/04 10:08:03 $
*/
#ifndef __USERPREF_H__
#define __USERPREF_H__
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
// MODULE:      userpref.H
//
// USAGE:       This module contains functions and definitions for the user
//				preferences saved in NVRAM.
//				Also includes the NVRAM map.
//
//******************************************************************************

//******************************************************************************
//  I N C L U D E    F I L E S
//******************************************************************************

#define WRITE_MONITOR_SERIAL_NUMBER

#define B_MONITOR_SERIAL_NUMBER_LENGHT    32 //lenght in bytes of the Monitor Serial Number information in NVRAM.

#define NVRAM_SIZE 4096
#define NVRAM_DP_EDID_SIZE   256
#define NVRAM_DP_EDID_ADDR (NVRAM_SIZE - 256)
#define NVRAM_RCOSC_TRIM_SIZE 1
#define NVRAM_RCOSC_TRIM_ADDR (NVRAM_DP_EDID_ADDR - 1)

//******************************************************************************
//  E X T E R N
//******************************************************************************

extern BYTE B_AutoAdjustCount[NUM_CHANNELS];

//******************************************************************************
//  G L O B A L    D E F I N I T I O N S
//******************************************************************************
#pragma pack(push)
#pragma pack(1)
typedef struct
{
   DWORD DW_MonitorPowerOnTime;
   char Ba_MonitorSerialNumberWriteBuffer[B_MONITOR_SERIAL_NUMBER_LENGHT];
} gmt_MonitorInfo;
#pragma pack(pop)

//
// NVRAM blocks Size definition
//
#define ModeDependentLookUpSize 		sizeof(ModeDependentLookUpType)
#define ChecksumSize					sizeof(BYTE)
#define MPipChannelInfoSize 				0	// For multipip favorites but not sure 
// if it will be implemented changed
// from 32 to 0

//
// Boundary modes definitions
//
#define NonSavedModeLookUpIndex     0xFF
#define EndOfLookUpIndex            0xFF
#define NonStandardModeIndex        0xFF

//
// The maximum delta when comparing with mode saved in NVRAM.
// Normally, these deltas are smaller then ones for standard modes.
//
#define SavedHFreqDelta					5	// 0.5KHz
#define SavedVFreqDelta					5	// 0.5Hz
#define SavedVTotalDelta				5	// 5 lines
#define SavedHperiodDelta				5	// 5 lines

//
// Number of ADC setting (graphic, SD/HD, Composite-NTSC, Composite-PAL, SIF)
//
enum
{
   ADC_SETTING_GRAPHIC,
   ADC_SETTING_HD,
   ADC_SETTING_HD2,
   NumberOfAdcSetting
};


//******************************************************************************
// N V R A M address map
//******************************************************************************

enum NVRAM_MAP
{
#ifdef HDCP_KEY_IN_USERPREF_NVRAM
   //
   // HDCP table
   // Note : HDCP_SIZE define as WORD size
   //
   HdcpBksv        = 0,
   HdcpTableStart  = 5,
   HdcpTableEnd    = HdcpTableStart + (287),
#else
   HdcpTableEnd    = 0,
#endif
   //
   // ADC calibration
   //
   AdcSettingsStart           = HdcpTableEnd,
   AdcSettingsChecksumStart 	= 	AdcSettingsStart + (BandgapSettingSize + ChecksumSize) +
   ((AdcSettingsSize + ChecksumSize)*(NumberOfAdcSetting)),
   AdcSettingEnd              = AdcSettingsChecksumStart + ChecksumSize,

   //
   // Mode dependent setting
   //
   ModeDependentUserIndexStart   = AdcSettingEnd,
   ModeDependentPresetEntryStart = ModeDependentUserIndexStart + 1,
   ModeDependentUserLookUpStart  = ModeDependentPresetEntryStart + ((ModeDependentUserPrefSize +
   //sizeof(B_AutoAdjustCount[gmvb_MainChannel])+
   sizeof(BYTE) + ChecksumSize) * NUM_GRAPHICS_PRESETMODE),
   ModeDependentUserEntryStart   = ModeDependentUserLookUpStart + ((ModeDependentLookUpSize +
   ChecksumSize) * MAX_USER_MODES_SAVED),

#ifdef CVT_MODES_SUPPORT
   ModeDependentCvtIndexStart		= ModeDependentUserEntryStart +
   (ModeDependentUserPrefSize + /*sizeof(B_AutoAdjustCount[gmvb_MainChannel])*/sizeof(BYTE) + ChecksumSize) * MAX_USER_MODES_SAVED,
   ModeDependentCvtLookUpStart   = ModeDependentCvtIndexStart + 1,
   ModeDependentCvtEntryStart	   = ModeDependentCvtLookUpStart + (ModeDependentLookUpSize + ChecksumSize) * MAX_CVT_MODES_SAVED,
   ModeDependentEnd              = ModeDependentCvtEntryStart +
   (ModeDependentUserPrefSize + /*sizeof(B_AutoAdjustCount[gmvb_MainChannel])*/sizeof(BYTE) + ChecksumSize) * MAX_CVT_MODES_SAVED,
#else
   ModeDependentEnd              = ModeDependentUserEntryStart +
   (ModeDependentUserPrefSize + /*sizeof(B_AutoAdjustCount[gmvb_MainChannel])*/sizeof(BYTE) + ChecksumSize) * MAX_USER_MODES_SAVED,
#endif

   //
   // Port dependent setting
   //
   PortSchemeDependentIndexStart    = ModeDependentEnd,
   PortSchemeDependentEntryStart    = PortSchemeDependentIndexStart + 1,
   PortSchemeDependentEnd  = PortSchemeDependentEntryStart + ((PortSchemeDependentUserPrefSize + ChecksumSize) *
   NUM_OF_IMAGE_SCHEMES_ENTRIES),    // PSD

   ModeIndependentStart         = PortSchemeDependentEnd,
   ModeIndependentChecksumStart = ModeIndependentStart + ModeIndependentUserPrefSize,
   ModeIndependentEnd           = ModeIndependentChecksumStart + ChecksumSize,

   //
   // AutoScanInput default VIDEO Input setting
   //

   AutoScanInputStart  = ModeIndependentEnd,
#ifdef AUTOSCANINPUT_NUM_RINGS
   AutoScanInputEnd    = AutoScanInputStart + sizeof(BYTE) * 2 * AUTOSCANINPUT_NUM_RINGS, // num of rings * 2 channels
#else
   AutoScanInputEnd    = AutoScanInputStart,
#endif

   //
   // Monitor Power On Time information
   //
   MonitorPowerOnTimeStart  = AutoScanInputEnd,
   MonitorPowerOnTimeEnd    = MonitorPowerOnTimeStart + sizeof(DWORD), // (DWORD)->4 bytes are used to store Monitor Power On Time information in NVRAM.

   MonitorChecksumStart	    = MonitorPowerOnTimeEnd,
   MonitorChecksumEnd       = MonitorChecksumStart + ChecksumSize,

   //
   // Monitor Serial Number information
   //
   MonitorSerialNumberStart  = MonitorChecksumEnd,
   MonitorSerialNumberEnd    = MonitorSerialNumberStart + B_MONITOR_SERIAL_NUMBER_LENGHT, //bytes used to store Monitor Serial Number information into NVRAM.

   RcOscTrimStart            = NVRAM_RCOSC_TRIM_ADDR,
   RcOscTrimEnd              = NVRAM_RCOSC_TRIM_ADDR + NVRAM_RCOSC_TRIM_SIZE,
   
   DpEdidTableStart          = NVRAM_DP_EDID_ADDR,
   DpEdidTableEnd            = NVRAM_DP_EDID_ADDR + NVRAM_DP_EDID_SIZE,
};

#if MonitorSerialNumberEnd > DpEdidTableStart
#error NVRAM overlap with DP EDID!
#endif

#if ( MonitorSerialNumberEnd >= NVRAM_SIZE)
#error NVRAM usage is beyond hardware limited 4k-byte.
#endif

//******************************************************************************
//  F U N C T I O N    P R O T O T Y P E S
//******************************************************************************

void InitNvram(void);
void NvramCheck	(void);
BYTE CalculateChecksum	(void const* Bp_Buffer, BYTE B_Size);

void ReadModeIndependentSettings	(void);
void SaveModeIndependentSettings	(void);

void InitModeDependentDescr			(void);
void InitModeDependentLookUp(void);
BOOL CheckModeDependentDescriptor(void);
void SaveModeDependentSettings(gmt_PHY_CH B_Channel, ModeDependentLookUpType * LookUpTypePtr);
void RetrieveGraphicsSettings(gmt_PHY_CH B_Channel, ModeDependentLookUpType * LookUpTypePtr);

BOOL ReadAdcSettings				(gmt_ADC_ID B_AdcId, BYTE *AdcSettings, BYTE B_Port);
BOOL SaveAdcSettings				(gmt_ADC_ID B_AdcId, BYTE *AdcSettings, BYTE B_Port);
BOOL ReadBandgapSetting		(BYTE *BandgapSetting);
BOOL SaveBandgapSetting		(BYTE *BandgapSetting);
BOOL ReadDecoderTbcSettings			(BYTE *AdcSettings, BYTE B_Port);
BOOL SaveDecoderTbcSettings			(BYTE *AdcSettings, BYTE B_Port);

void InitPortSchemeDependentDescr			(void);
void InitPortSchemeDependent				(void);
BOOL CheckPortSchemeDependentEntry		(BYTE B_Index);
BOOL CheckPortSchemeDependentDescriptor	(void);
BOOL ReadPortSchemeDependentEntry(BYTE B_Index);
void SavePortSchemeDependentSettings		(void);

// Functions used for Saving single UserPref into NVRAM solution
void SaveUserPref(gmt_OSD_Adjuster_Info * AdjusterInfo);
void RetrieveUserPref(gmt_OSD_Adjuster_Info * AdjusterInfo);
void SaveModeDependentUserPrefValue(gmt_PHY_CH B_Channel, gmt_OSD_Adjuster_Info * AdjusterInfo);
void RetrieveModeDependentUserPrefValue(gmt_PHY_CH B_Channel, gmt_OSD_Adjuster_Info * AdjusterInfo);
// Function used for reading the Monitor Power On Time from NVRAM.
BOOL ReadMonitorPowerOnTimefromNVRAM(DWORD *pDW_MonitorPowerOnTime);

// Function used for saving the Monitor Power On Time into NVRAM.
void WriteMonitorPowerOnTimeintoNVRAM(DWORD *pDW_MonitorPowerOnTime);


// Function used for reading the Monitor Serial Number from NVRAM.
void ReadMonitorSerialNumberfromNVRAM(char *pBa_MonitorSerialNumberWriteBuffer);

#ifdef WRITE_MONITOR_SERIAL_NUMBER
// This function is used only for testing since the Monitor Serial Number is burned in NVRAM in the manufacturing process.
void WriteMonitorSerialNumberintoNVRAM(char *pBa_MonitorSerialNumberWriteBuffer);
#endif


#endif

void EditChannelList(void);

//********************************  END  **************************************
