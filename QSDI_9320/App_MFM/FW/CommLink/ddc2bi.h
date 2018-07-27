#ifndef __DDC2BI_HANDLER_H__
#define __DDC2BI_HANDLER_H__

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
// MODULE:      ddc2bi.h
//
// USAGE:       Header file for the ddc2bi parser handler.
//
//******************************************************************************

//******************************************************************************
//  G L O B A L    D E F I N I T I O N S
//******************************************************************************


//******************************************************************************
//  G L O B A L    V A R I A B L E S
//******************************************************************************
extern BYTE ROM strCapabilitiesRequest[];

//******************************************************************************
//  G L O B A L    F U N C T I O N    P R O T O T Y P E S
//******************************************************************************
void Ddc2biCmdHandler(void);
WORD far GetCapabilitiesStringSize(void);

void far I2CSlaveInitAll(void);
void far I2CSlaveChannelHandler(SLAVE_CH B_SlaveCh);

//#define	DDC2Bi_CONTROL_STATUS_FLAG		0x80

//DDC2Bi commands
#define DDC2B_CMD_DEBUG						0xC2
#define DDC2B_CMD_GetVCPFeature				0x01
#define DDC2B_CMD_GetVCPFeature_Reply		0x02
#define DDC2B_CMD_SetVCPFeature				0x03
#define DDC2B_CMD_SaveCurrentSettings		0x0C
#define DDC2B_CMD_CapabilitiesRequest		0xF3
#define DDC2B_CMD_CapabilitiesRequest_Reply	0xE3
#define DDC2B_CMD_GetTimingReport			0x07
#define DDC2B_CMD_GetTimingReport_Reply		0x4E

//NMV Custom commands
#define DDC2B_CMD_NMV_GetVCPPage			0xC4
#define DDC2B_CMD_NMV_GetVCPPage_Reply		0xC5
#define DDC2B_CMD_NMV_GetPagedVCP			0xC6
#define DDC2B_CMD_NMV_GetPagedVCP_Reply		0xC7
#define DDC2B_CMD_NMV_SetPagedVCP			0xC8
#define DDC2B_CMD_NMV_SetPagedVCP_Reply		0xC7
#define DDC2B_CMD_NMV_EEPROM				0xC0
#define DDC2B_CMD_NMV_EEPROM_Reply			0xC1

//NMV EEPROM commands
#define DDC2B_CMD_NMV_EEPROMRead			0x09
#define DDC2B_CMD_NMV_AssetRead				0x0B
#define DDC2B_CMD_NMV_AssetWrite			0x0E

//VCP commands Page0
#define DDC2B_CMD_VCP_NewControlVal			0x02		//To make MCCS 2.0 Compliant
#define DDC2B_CMD_VCP_ALLReset				0x04
#define DDC2B_CMD_VCP_RestoreContrastBrightness		0x05
#define DDC2B_CMD_VCP_GeometryReset			0x06
#define DDC2B_CMD_VCP_ColorReset				0x08
#define DDC2B_CMD_VCP_Clock					0x0E
#define DDC2B_CMD_VCP_Brightness				0x10
#define DDC2B_CMD_VCP_Contrast					0x12
#define DDC2B_CMD_VCP_SelectColorPreset		0x14
#define DDC2B_CMD_VCP_RedGain					0x16
#define DDC2B_CMD_VCP_R_VT					0x17
#define DDC2B_CMD_VCP_GreenGain				0x18
#define DDC2B_CMD_VCP_G_VT					0x19
#define DDC2B_CMD_VCP_BlueGain					0x1A
#define DDC2B_CMD_VCP_B_VT					0x1B
#define DDC2B_CMD_VCP_AutoSetUp				0x1E
#define DDC2B_CMD_VCP_HPosition				0x20
//#define DDC2B_CMD_VCP_OSD						0xCA
#define DDC2B_CMD_VCP_VPosition				0x30
#define DDC2B_CMD_VCP_ClockPhase				0x3E
#define DDC2B_CMD_VCP_SourceSelect				0x60
#define DDC2B_CMD_VCP_AudioVolume				0x62
#define DDC2B_CMD_VCP_Language				0x68
#define DDC2B_CMD_VCP_TVSaturation				0x8A
#define DDC2B_CMD_VCP_TVSharpness				0x8C
#define DDC2B_CMD_VCP_RED						0x9B
#define DDC2B_CMD_VCP_YELLOW					0x9C
#define DDC2B_CMD_VCP_GREEN					0x9D
#define DDC2B_CMD_VCP_CYAN					0x9E
#define DDC2B_CMD_VCP_BLUE					0x9F
#define DDC2B_CMD_VCP_MAGENDA				0xA0
#define DDC2B_CMD_VCP_SyncType				0xA8
#define DDC2B_CMD_VCP_HFreq					0xAC    //To make MCCS 2.0 Compliant
#define DDC2B_CMD_VCP_VFreq					0xAE    //To make MCCS 2.0 Compliant
#define DDC2B_CMD_VCP_RGBodering				0xB4
#define DDC2B_CMD_VCP_MonitorType				0xB6
#define DDC2B_CMD_VCP_ColorTemp				0xE0
#define DDC2B_CMD_VCP_ControlLock				0xE3
#define DDC2B_CMD_VCP_AutoColor				0xE4
#define DDC2B_CMD_VCP_Settings					0xB0
#define DDC2B_CMD_VCP_DisplayUsageTime		0xC0    //To make MCCS 2.0 Compliant
#define DDC2B_CMD_VCP_ApplnEnKey				0xC6    //To make MCCS 2.0 Compliant
#define DDC2B_CMD_VCP_DispControllerType		0xC8    //To make MCCS 2.0 Compliant
#define DDC2B_CMD_VCP_DispFirmwareLevel		0xC9    //To make MCCS 2.0 Compliant
#define DDC2B_CMD_VCP_OSD						0xCA    //To make MCCS 2.0 Compliant
#define DDC2B_CMD_VCP_OsdLanguage			0xCC
#define DDC2B_CMD_VCP_DisplayPowerMode		0xD6
#define DDC2B_CMD_VCP_MCCSVersion				0xDF    //To make MCCS 2.0 Compliant
#define DDC2B_CMD_VCP_Power					0xE1
#define DDC2B_CMD_VCP_BlackLevel				0x1D
#define DDC2B_CMD_VCP_Red_Video_Black_Level     0x6C
#define DDC2B_CMD_VCP_Green_Video_Black_Level   0x6E
#define DDC2B_CMD_VCP_Blue_Video_Black_Level    0x70

#define DDC2B_Max_VCP_Pages					2
//VCP commands Page2
#define DDC2B_CMD_VCP_ContrastAuto				0x37
#define DDC2B_CMD_VCP_OSDHPosition			0x38
#define DDC2B_CMD_VCP_OSDVPosition				0x39

//Size of Message Packet to send upon Capabilities String Request
#define VCP_GETCAPABILITIES_REPLYMSG_SIZE 0x23

//Supported DDC2B_CMD_VCP_SourceSelect Input sources
#define VCP_SourceSelect_AnalogVideo_1		0x01 		// 1 for Analog video(R/G/B) 1
#define VCP_SourceSelect_AnalogVideo_2		0x02 		// 2 for Analog video(R/G/B) 2
#define VCP_SourceSelect_DigitalVideo_1		0x03 		// 3 for Digital video(TMDS) 1
#define VCP_SourceSelect_DigitalVideo_2		0x04 		// 4 for Digital video(TMDS) 2
#define VCP_SourceSelect_CompositeVideo_1 	0x05 		// 5 for Composite video 1
#define VCP_SourceSelect_CompositeVideo_2 	0x06 		// 6 for Composite video 2
#define VCP_SourceSelect_SVideo_1 			0x07 		// 7 for S video 1
#define VCP_SourceSelect_SVideo_2 			0x08 		// 8 for S video 2
#define VCP_SourceSelect_Tuner_1 			0x09 		// 9 for Tuner 1
#define VCP_SourceSelect_Tuner_2 			0x0A 		// 10 for Tuner 2
#define VCP_SourceSelect_Tuner_3 			0x0B 		// 11 for Tuner 3
#define VCP_SourceSelect_ComponentVideo_1 	0x0C 		// 12 for Component video(YPrPb/YCrCb) 1
#define VCP_SourceSelect_ComponentVideo_2 	0x0D 		// 13 for Component video(YPrPb/YCrCb) 2
#define VCP_SourceSelect_ComponentVideo_3 	0x0E 		// 14 for Component video(YPrPb/YCrCb) 3

// receive/transmit buffer syntax defines
#define	LENGTH								0
#define	COMMAND							1
#define	PARAMETER							2

#define ADJUSTER_RBLACKLEVEL_DECLARED
#define ADJUSTER_GBLACKLEVEL_DECLARED
#define ADJUSTER_BBLACKLEVEL_DECLARED

#ifdef USE_COLOR_FIDELITY
#if 1//USE_DYNAMIC_GAMUTMAPPING_SUPPORT
#define DDC2B_CMD_VCP_SimplifiedColorProfile    0xF0    //steven le, 8/14/2009
#define DDC2B_CMD_Table_Write						 0xE7
#define DDC2B_CMD_Table_Read							 0xE2
#define DDC2B_CMD_Table_Reply						 0xE4

typedef struct DDC2Bi_WGS_VCPCmdStruct
{
   BYTE length;
   BYTE command;      //Table command
   BYTE vcp_opcode;   //0xF0
   //WORD value;
   BYTE value_H;
   BYTE value_L;
   BYTE mode;
   BYTE par;
   BYTE chk;
}  DDC2Bi_WGS_VCPCmd_t;

typedef struct DDC2Bi_WGS_ReplyCmdStruct
{
   BYTE length;
   BYTE command;      //Table command
   BYTE vcp_opcode;   //0xF0
   //WORD value;
   BYTE value_H;
   BYTE value_L;
   //WORD status;
   BYTE status_H;
   BYTE status_L;
   BYTE chk;
}  DDC2Bi_WGS_ReplyCmd_t;

#endif
#endif // USE_COLOR_FIDELITY

typedef enum
{
   DDCCI_CTEMP_SRGB = 1,
   DDCCI_CTEMP_DISPLAY_NATIVE,	//02
   DDCCI_CTEMP_4000K,			//03
   DDCCI_CTEMP_5000K,			//04
   DDCCI_CTEMP_6500K,			//05
   DDCCI_CTEMP_7500K,			//06
   DDCCI_CTEMP_8200K,			//07
   DDCCI_CTEMP_9300K,			//08
   DDCCI_CTEMP_10000K,			//09
   DDCCI_CTEMP_11500K,			//0A
   DDCCI_CTEMP_USER1,			//0B
   DDCCI_CTEMP_USER2,			//0C
   DDCCI_CTEMP_USER3			//0D
} DDC_COLOR_TEMP;
typedef struct DDC2Bi_GetVCPFeatureCmdStruct
{
   BYTE length;
   BYTE command;
   BYTE vcp_opcode;
   BYTE chk;
}DDC2Bi_GetVCPFeatureCmd_t;

typedef struct VCPFeaturePktBodyStruct
{
   BYTE vcp_opcode;
   BYTE vcp_type;
   WORD max_BigEndian;
   WORD value_BigEndian;
}VCPFeaturePktType;


#pragma pack(push)
#pragma pack(1)
typedef struct DDC2Bi_GetVCPFeature_ReplyCmdStruct
{
   BYTE				length;
   BYTE				command;
   BYTE				result;
   VCPFeaturePktType	a_VCPFeature;
   BYTE chk;
}DDC2Bi_GetVCPFeature_ReplyCmd_t;
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
typedef struct DDC2Bi_SetVCPFeatureCmdStruct
{
   BYTE length;
   BYTE command;
   BYTE vcp_opcode;
   WORD val_BigEndian;
}DDC2Bi_SetVCPFeatureCmd_t;
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
typedef struct DDC2Bi_GetPagedVCPCmdStruct
{
   BYTE length;
   BYTE command;
   BYTE vcp_page;
   BYTE vcp_opcode;
}DDC2Bi_GetPagedVCPCmd_t;
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
typedef struct DDC2Bi_GetPagedVCP_ReplyCmdStruct
{
   BYTE				length;
   BYTE				command;
   BYTE				result;
   BYTE				vcp_page;
   VCPFeaturePktType	b_VCPFeature;
   BYTE chk;
}DDC2Bi_GetPagedVCP_ReplyCmd_t;
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
typedef struct DDC2Bi_SetPagedVCPCmdStruct
{
   BYTE length;
   BYTE command;
   BYTE vcp_page;
   BYTE vcp_opcode;
   WORD value_BigEndian;
   BYTE chk;
}DDC2Bi_SetPagedVCPCmd_t;
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
typedef struct DDC2Bi_GetVCPPage_ReplyCmdStruct
{
   BYTE length;
   BYTE command;
   BYTE vcp_page;
   BYTE vcp_maxpage;
}DDC2Bi_GetVCPPage_ReplyCmd_t;
#pragma pack(pop)

//NMV EEPROM commands definitions
#pragma pack(push)
#pragma pack(1)
typedef struct DDC2Bi_EEPROMCmdStruct
{
   BYTE length;
   BYTE command;
   BYTE eeprom_cmd;
}DDC2Bi_EEPROMCmd_t;
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
typedef struct DDC2Bi_EEPROMReadCmdStruct
{
   BYTE length;
   BYTE command;
   BYTE eeprom_cmd;
   BYTE leng;
   WORD addr;
   BYTE chk;
}DDC2Bi_EEPROMReadCmd_t;
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
typedef struct DDC2Bi_AssetReadCmdStruct
{
   BYTE length;
   BYTE command;
   BYTE eeprom_cmd;
   BYTE offset;
   BYTE leng;
   BYTE chk;
}DDC2Bi_AssetReadCmd_t;
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
typedef struct DDC2Bi_AssetWriteCmdStruct
{
   BYTE length;
   BYTE command;
   BYTE eeprom_cmd;
   BYTE offset;
}DDC2Bi_AssetWriteCmd_t;
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
typedef struct
{
   BYTE length;
   BYTE command;
   WORD offset_BigEndian;
   BYTE chk;
}DdcciCapabilitiesRequest;
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
typedef struct DDC2Bi_GetTiming_ReplyCmd
{
   BYTE length;
   BYTE command;
   BYTE status;
   WORD h_freq_BigEndian;
   WORD v_freq_BigEndian;
} DDC2Bi_GetTiming_ReplyCmd_t;
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
typedef struct
{
   BYTE length;
   BYTE command;
   WORD offset_BigEndian;
   BYTE data;
} DdcciCapabilitiesReply;
#pragma pack(pop)

typedef enum SendAckNackDataStruct
{
    SND_NAK = 0,
    SND_ACK,
    SND_DATA,
    SND_NOTHING
}SendAckNackData_t;

#define SetBit(VAR,BIT)         ( VAR |= BIT )
#define ClrBit(VAR,BIT)         ( VAR &= ~BIT )
#define CheckBit(VAR,BIT)       ( VAR | BIT )
#define RetBit(VAR,BIT)         (( VAR & BIT ) != 0 )

#define	GET_VCP	0
#define	SET_VCP	1

#define Start_of_GprobeCmd_in_DDC2BiMsg   5   // 3 (device id + length + cmd) + 2 (manufacturer id)
#define I2C_as_Gprobe_Cmd(CH)			(Sp_I2CSlavePtrGlobal[CH]->Buffer[2])
#define I2C_ManufacturerID(CH)			(*(WORD*)(&(Sp_I2CSlavePtrGlobal[CH]->Buffer[3])))

#endif //#ifndef __DDC2BI_HANDLER_H___
//*********************************  END  **************************************
