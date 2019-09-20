/*
   $Workfile:   CecLib.h  $
   $Revision: 1.4 $
   $Date: 2011/12/14 03:44:48 $
*/

//#include "Inc\CecMngrLib.h"

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
//******************************************************************************
//
//  MODULE: CecLib.h
//
//
//  USAGE : Contains declarations for Cec Library
//
//******************************************************************************
#ifndef __CEC_LIB_H__
#define __CEC_LIB_H__

//******************************************************************************
//  G L O B A L  T Y P E S  D E F I N I T I O N S
//******************************************************************************
#ifdef __PARADIGM__
	#pragma pack(push)
	#pragma pack(1)
#endif


//******************************************************************************
//  U T I L I T Y   ( A P P   L E V E L )   T Y P E S
//******************************************************************************
typedef enum
{
    gmd_SAC_NONE_COMMAND,
    gmd_SAC_VOL_UP_PRESS,
    gmd_SAC_VOL_UP_RELEASE,
    gmd_SAC_VOL_DN_PRESS,
    gmd_SAC_VOL_DN_RELEASE,
    gmd_SAC_REQUEST_VOLUME,
    gmd_SAC_TOGGLE_MUTE
}gmt_SYSTEM_AUDIO_COMMANDS;

typedef enum
{
    CEC_AR_UNRECOGN_OPCODE              		= 0,
    CEC_AR_INCORRECT_MODE               		= 1,
    CEC_AR_CANNOT_PROVIDE_SRC           		= 2,
    CEC_AR_INVALID_OPERAND              		= 3,
    CEC_AR_REFUSED                      		= 4
} gmt_CEC_ABORT_REASONS;


//CEC versions
typedef enum
{
    CEC_VERSION_1_1 							= 0x00,
    CEC_VERSION_1_2                             = 0x01,
    CEC_VERSION_1_2a                            = 0x02,
    CEC_VERSION_1_3                             = 0x03,
    CEC_VERSION_1_3a                            = 0x04
}gmt_CEC_VERSIONS;

typedef enum
{
    gmd_SCOk                                    = 0x00,
    gmd_SCErrorLen                              = 0x01,
    gmd_SCErrorDir                              = 0x02,
    gmd_SCErrorOpCode                           = 0x04,
    gmd_SCErrorMask                             = 0x07,
    gmd_SCErrorAbort                            = 0x80
}gmt_CEC_MSG_VALIDITY_CHECK_RESULT;

//Device logical addresses
typedef enum
{
    gmd_CDT_TV                                  = 0x00,
    gmd_CDT_RECORDER_1                          = 0x01,
    gmd_CDT_RECORDER_2                          = 0x02,
    gmd_CDT_TUNER_1                             = 0x03,
    gmd_CDT_PLAYER_1                            = 0x04,
    gmd_CDT_AUDIO_SYSTEM                        = 0x05,
    gmd_CDT_TUNER_2                             = 0x06,
    gmd_CDT_TUNER_3                             = 0x07,
    gmd_CDT_PLAYER_2                            = 0x08,
    gmd_CDT_RECORDER_3                          = 0x09,
    gmd_CDT_TUNER_4                             = 0x0A,
    gmd_CDT_PLAYER_3                            = 0x0B,
    gmd_CDT_RESERVED_12                         = 0x0C,
    gmd_CDT_RESERVED_13                         = 0x0D,
    gmd_CDT_FREE_USE                            = 0x0E,
    gmd_CDT_UNREGISTERED                        = 0x0F
}gmt_CEC_DEVICE_TYPE;

//Cec Messages Opcodes
typedef enum
{
    CEC_MSG_FEATURE_ABORT	                    = 0x00,  // <Feature Abort> message
    CEC_MSG_IMAGE_VIEW_ON	                    = 0x04,  // <Image View On> message
    CEC_MSG_TUNER_STEP_INCREMENT                = 0x05,  // <Tuner Step Increment> message
    CEC_MSG_TUNER_STEP_DECREMENT                = 0x06,  // <Tuner Step Decrement> message
    CEC_MSG_TUNER_DEVICE_STATUS                 = 0x07,  // <Tuner Device Status>
    CEC_MST_GIVE_TUNER_DEVICE_STATUS            = 0X08,
    CEC_MSG_RECORD_ON                           = 0x09,  // <Record On> message
    CEC_MSG_RECORD_STATUS                       = 0x0A,
    CEC_MSG_RECORD_OFF                          = 0x0B,
    CEC_MSG_TEXT_VIEW_ON                        = 0x0D,  // <Text View On> message
    CEC_MSG_RECORD_TV_SCREEN                    = 0x0F,
    CEC_MSG_GIVE_DECK_STATUS                    = 0x1A,  // <Give Deck Status> message
    CEC_MSG_DECK_STATUS                         = 0x1B,
    CEC_MSG_SET_MENU_LANGUAGE                   = 0x32,  // <Set Menu Language>
    CEC_MSG_CLEAR_ANALOGUE_TIMER                = 0x33,  // <Clear analogue timer>
    CEC_MSG_SET_ANALOGUE_TIMER                  = 0x34,  // <Set analogue timer>
    CEC_MSG_TIMER_STATUS                        = 0x35,  // <Timer status>
    CEC_MSG_STANDBY                             = 0x36,  // <Standby> message
    CEC_MSG_PLAY                                = 0x41,
    CEC_MSG_DECK_CONTROL                        = 0x42,
    CEC_MSG_TIMER_CLEARED_STATUS                = 0x43,  // <Timer cleared status>
    CEC_MSG_USER_CONTROL_PRESSED                = 0x44,
    CEC_MSG_USER_CONTROL_RELEASED               = 0x45,
    CEC_MSG_GIVE_OSD_NAME                       = 0x46,  // <Give OSD Name> message
    CEC_MSG_SET_OSD_NAME                        = 0x47,  // <Set OSD Name> message
    CEC_MSG_SET_OSD_STRING                      = 0x64,  // <Set OSD String> message
    CEC_MSG_SET_TIMER_PROGRAM_TITLE             = 0x67,  // <Set timer program title>
    CEC_MSG_SYSTEM_AUDIO_MODE_REQUEST           = 0x70,
    CEC_MSG_GIVE_AUDIO_STATUS                   = 0x71,  // <Give audio status>
    CEC_MSG_SET_SYSTEM_AUDIO_MODE               = 0x72,
    CEC_MSG_REPORT_AUDIO_STATUS                 = 0x7A,  // <Report Audio Status>
    CEC_MSG_GIVE_SYSTEM_AUDIO_MODE_STATUS       = 0x7D,  // <Give System Audio Mode Status>
    CEC_MSG_SYSTEM_AUDIO_MODE_STATUS            = 0x7E,  // <System Audio Mode Status>
    CEC_MSG_ROUTING_CHANGE                      = 0x80,  // <Routing Change> message
    CEC_MSG_ROUTING_INFORMATION                 = 0x81,  // <Routing information>
    CEC_MSG_ACTIVE_SOURCE                       = 0x82,  // <Active Source> message
    CEC_MSG_GIVE_PHYSICAL_ADDRESS               = 0x83,  // <Give Physical Address> message
    CEC_MSG_REPORT_PHYSICAL_ADDRESS             = 0x84,  // <Report physical address> message; this message is received when new device connect to network
    CEC_MSG_REQUEST_ACTIVE_SOURCE               = 0x85,  // <Request active source>
    CEC_MSG_SET_STREAM_PATH                     = 0x86,  // <Set Stream Path>
    CEC_MSG_DEVICE_VENDOR_ID                    = 0x87,  // <Device Vendor ID>
    CEC_MSG_VENDOR_COMMAND                      = 0x89,  // <Vendor Command>
    CEC_MSG_VENDOR_REMOTE_BUTTON_DOWN           = 0x8A,
    CEC_MSG_VENDOR_REMOTE_BUTTON_UP             = 0x8B,
    CEC_MSG_GIVE_DEVICE_VENDOR_ID               = 0x8C,
    CEC_MSG_MENU_REQUEST                        = 0x8D,
    CEC_MSG_MENU_STATUS                         = 0x8E,
    CEC_MSG_GIVE_DEVICE_POWER_STATUS            = 0x8F,  // <Give Device Power Status>
    CEC_MSG_REPORT_POWER_STATUS                 = 0x90,  // <Report Power Status>
    CEC_MSG_GET_MENU_LANGUAGE                   = 0x91,  // <Get Menu Language>
    CEC_MSG_SELECT_ANALOGUE_SERVICE             = 0x92,
    CEC_MSG_SELECT_DIGITAL_SERVICE              = 0x93,
    
    CEC_MSG_SET_DIGITAL_TIMER                   = 0x97,  // <Set Digital Timer>
    CEC_MSG_CLEAR_DIGITAL_TIMER                 = 0x99,  // <Clear digital timer>
    CEC_MSG_SET_AUDIO_RATE                      = 0x9A,
    CEC_MSG_INACTIVE_SOURCE                     = 0x9D,  // <Inactive Source> 
    CEC_MSG_CEC_VERSION                         = 0x9E,  // <CEC version>
    CEC_MSG_GET_CEC_VERSION                     = 0x9F,  // <Get CEC version>
    CEC_MSG_VENDOR_COMMAND_WITH_ID              = 0xA0,
    CEC_MSG_CLEAR_EXTERNAL_TIMER                = 0xA1,  // <Clear external timer>
    CEC_MSG_SET_EXTERNAL_TIMER                  = 0xA2,  // <Set external timer>
    CEC_MSG_ABORT                               = 0xFF   // <Abort> message
} gmt_CEC_MESSAGES_ENUM;

//Remote Control Pass Through feature codes
typedef enum{
    gmd_UCC_SELECT                              = 0x0000,
    gmd_UCC_UP                                  = 0x0001,
    gmd_UCC_DOWN                                = 0x0002,
    gmd_UCC_LEFT                                = 0x0003,
    gmd_UCC_RIGHT                               = 0x0004,
    gmd_UCC_RIGHT_UP                            = 0x0005,
    gmd_UCC_RIGHT_DOWN                          = 0x0006,
    gmd_UCC_LEFT_UP                             = 0x0007,
    gmd_UCC_LEFT_DOWN                           = 0x0008,

    gmd_UCC_VOLUME_UP                           = 0x0041,
    gmd_UCC_VOLUME_DN                           = 0x0042,
    gmd_UCC_MUTE                                = 0x0043,
    
    gmd_UCC_PLAY                                = 0x0044,
    gmd_UCC_STOP                                = 0x0045,
    gmd_UCC_PAUSE                               = 0x0046,
    gmd_UCC_RECORD                              = 0x0047,
    gmd_UCC_REWIND                              = 0x0048,
    gmd_UCC_FAST_FORWARD                        = 0x0049,
    gmd_UCC_EJECT                               = 0x004A,
    gmd_UCC_STOP_RECORDING                      = 0x004D,
    gmd_OSD_CEC_IDLE                            = 0x00FF
}gmt_USER_CONTROL_COMMANDS;
//******************************************************************************
//  M A N A G E R   T Y P E S
//******************************************************************************
typedef enum
{
	// Message hasen't any status. Init state.
	gmd_CEC_MNGR_TX_STS_NONE,
	//Message is wating to be transmitted. Transition state
	gmd_CEC_MNGR_TX_STS_WAITING_FOR_TRANSMISSION,
	//Message is being transmitted. Transition state
	gmd_CEC_MNGR_TX_STS_TRANSMITTING,
	//Message has been transmitted and acknowledged. Transition state
	gmd_CEC_MNGR_TX_STS_ACKNOWLEDGED,
	//Message has been transmitted but not acknowledged. Terminal state
	gmd_CEC_MNGR_TX_STS_NOT_ACKNOWLEDGED,
	//Message transmission terminated with error. Terminal state
	gmd_CEC_MNGR_TX_STS_ERROR,
	//Message is considered and acted on by follower. 1 sec after it has been acknowledged.
	//Terminal state
	gmd_CEC_MNGR_TX_STS_ACTED_ON,
	//Message has been aborted. <Feature Abort> returned for this message.
	//Terminal state
	gmd_CEC_MNGR_TX_STS_ABORTED,
	//Query message has been replied. Parameters of reply are avalible at Bp_RespResult.
	//Terminal state
	gmd_CEC_MNGR_TX_STS_REPLIED,
	//Query message has not been replied. No response within 1 sec
	//Terminal state
	gmd_CEC_MNGR_TX_STS_NOT_REPLIED,
} gmt_CEC_MNGR_MESSAGE_STATUS;

#ifdef __PARADIGM__
	#pragma pack(push)
	#pragma pack(1)
#endif

typedef struct
{
	BYTE B_DestLogAddr;
	BYTE B_OpCode; //Init value should be 0xFF, <Abort>, indicating element is free for use
	DWORD DW_TxTime; //Time at which message was transmitted, ms
	gmt_CEC_MNGR_MESSAGE_STATUS B_Status; // Being transmitted, Transmitted, Error, Acknoleged, Acted on/Aborted, Replied/Timed out
	WORD W_RespActTime; //Time after expiration of which a message considered as not acted on or not responded, sec, 1 - default value, 0 - time is not monitored
	BYTE B_RespOpCode; //OpCode that is expected as response, if no response is expected then 0xFF
	BYTE* Bp_RespResult; //Pointer to the buffer where the data of response message shall be stored to; the buffer is allocated by caller
    BYTE B_RespLen; // Length of coming frame
} gmt_CEC_MNGR_TX_MESSAGE_HANDLE;

#ifdef __PARADIGM__
	#pragma pack(pop)
#endif


//******************************************************************************
//  D R I V E R   T Y P E S
//******************************************************************************
typedef	struct
{
    WORD W_CecRegBaseAddress;
    BYTE B_NumberOfRetransmissions;         // Number of retransmissions
    BYTE B_SizeOfRxBuff;                    // size of Rx buffer
    BYTE CEC_RAM * Bp_CecRxBufferAddress;   // Pointer to CEC Rx buffer
    DWORD DW_TClkHz;                        // clock 
} gmt_CEC_INIT;

typedef struct
{
   BYTE B_CecStdMajor;
   BYTE B_CecStdMinor;
   
   BYTE B_CecIpVersionMajor;
   BYTE B_CecIpVersionMinor;
   BYTE B_CecIpRevision;

   WORD W_LibReleaseNumber;
   WORD W_LibBuildNumber;

}gmt_CEC_LIB_VERSION;

#ifdef __PARADIGM__
	#pragma pack(pop)
#endif

typedef enum
{
    gmd_CEC_FRAME_RX_DONE       =   BIT0,   // CEC Rx buffer contains at least one frame that is ready for reading
    gmd_CEC_FRAME_RX_FULL       =   BIT1,   // CEC Rx buffer is full and receiving is stopped.
    gmd_CEC_FRAME_TX_BUSY       =   BIT2,   // CEC line was busy at the moment Tx driver wants to start transmission
    gmd_CEC_FRAME_TX_ERROR      =   BIT3,   // The last CEC frame transmission was unsuccessful
    gmd_CEC_FRAME_TX_ARBITER    =   BIT4,   // another device capture the line during transmission of initiator address
    gmd_CEC_FRAME_TX_ACK        =   BIT5,   // The last CEC frame transmission has been acknowledged
    gmd_CEC_FRAME_TX_DONE       =   BIT6    // The last CEC frame transmission has been completed
} gmt_CEC_FRAME_STATUS;

typedef enum
{
    gmd_CEC_BUS_FREE            =   BIT0,   // CEC bus is free and ready for use
    gmd_CEC_BUS_RX_BUSY         =   BIT1,   // CEC frame is being received
    gmd_CEC_BUS_TX_BUSY         =   BIT2,   // CEC frame is being transmitted
    gmd_CEC_BUS_RX_NACK         =   BIT3,   // Respond with NACK for directly addressed message and ACK for broadcast
    gmd_CEC_BUS_ERROR           =   BIT4,   // CEC Line error
    gmd_CEC_BUS_INACTIVE        =   BIT5,   // CEC bus is inactive during Free Time
}gmt_CEC_BUS_STATUS;

//******************************************************************************
//  M A C R O   D E F I N I T I O N S 
//******************************************************************************

//******************************************************************************
//  A P P L I C A T I O N   M A C R O   D E F I N I T I O N S
//******************************************************************************
#define CEC_TV_ADDRESS              0x00
#define CEC_FREEUSE_ADDRESS         0x0E
#define CEC_UNREG_ADDRESS           0x0F

//******************************************************************************
//  M A N A G E R   M A C R O   D E F I N I T I O N S
//******************************************************************************
#define	NULL_MSG_ID 0

#ifdef NULL_MSG_ID
#define INVALID_MSG_ID	0xFF
#else
#define INVALID_MSG_ID	0
#endif

//******************************************************************************
//  G L O B A L  M A C R O S  D E F I N I T I O N S
//******************************************************************************


//******************************************************************************
//  G L O B A L    F U N C T I O N S '   P R O T O T Y P E S
//******************************************************************************

//******************************************************************************
//  U T I L I T Y ' S ( A P P   L E V E L )   P R O T O T Y P E S
//******************************************************************************
gmt_CEC_MSG_VALIDITY_CHECK_RESULT far gm_CecUtilMessageValidityCheck(BYTE B_Code, BYTE B_Length, BOOL Directed);
BYTE far gm_CecUtilExtractDigitFromPhysAddr(WORD PhysAddr, BYTE Digit);
BOOL far gm_CecUtilIsMessageNotReplied(gmt_CEC_MNGR_MESSAGE_STATUS fs);
void gm_CecUtilRequestAudioRcvrAudioStatus(void);
BOOL gm_CecUtilIsDeviceAbleToRecord(BYTE B_LogAddr);
void gm_CecUtilReportPhysicalAddress(BYTE B_DestAddr, BYTE B_Type);
void gm_CecUtilRequestSystemAudioStatus(void);
void gm_CecUtilSayCecVersion1_3(BYTE B_DestLogAddr);
void gm_CecUtilGetCecVersion(BYTE B_DestLogAddr);
void gm_CecUtilRequestActiveSource(BYTE B_DestLogAddr);
void gm_CecUtilFeatureAbort(BYTE B_DestLogAddr, BYTE B_FeatureOpCode, BYTE B_FeatureAbortReason);
BYTE gm_CecUtilEnumerateCecDevicesWithinSameType(BYTE increment, BYTE B_DeviceType, BYTE B_CecLogicalAddress);
BOOL gm_CecAppSystemAudioHandlerRequest(gmt_SYSTEM_AUDIO_COMMANDS Command);
void gm_CecAppSystemAudioHandler(void);

//******************************************************************************
//  C E C   A P P   P R O T O T Y P E S
//******************************************************************************
void gm_CecAppSetPhysicalAddress(WORD W_Value);
WORD gm_CecAppGetPhysicalAddess(void);
void gm_CecAppLogAddrHandler(void);
BOOL gm_CecAppLogAddrHandlerRequest(void);

//******************************************************************************
//  M A N A G E R ' S   P R O T O T Y P E S
//******************************************************************************
BYTE gm_CecMngrTxMessage(BYTE B_DestLogAddr, BYTE B_DataLen, BYTE CEC_RAM * Bp_Data, BYTE B_LogFlag,
								   WORD W_RespActTime);

BYTE gm_CecMngrTxQueryMessage(BYTE B_DestLogAddr, BYTE B_DataLen, BYTE CEC_RAM * Bp_Data,
		WORD W_RespActTime, BYTE B_RespOpCode, BYTE CEC_RAM * Bp_RespResult, BYTE B_RespLen);

BOOL gm_CecMngrRxMessage(BYTE CEC_RAM * Bp_SrcLogAddr, BYTE CEC_RAM * Bp_DestLogAddr,
                        BYTE CEC_RAM * Bp_DataLen, BYTE CEC_RAM * Bp_Data);

gmt_CEC_MNGR_MESSAGE_STATUS 	gm_CecMngrGetTxMessageStatus(BYTE B_MsgId);
gmt_CEC_MNGR_TX_MESSAGE_HANDLE 	gm_CecMngrGetTxMessageHandle(BYTE B_MsgId);

void gm_CecMngrRxHandler(void);
void gm_CecMngrTxHandler(void);

void gm_CecMngrTxManagerReset(void);

BOOL gm_CecMngrIsTerminalState(gmt_CEC_MNGR_MESSAGE_STATUS B_Status);

//******************************************************************************
//  D R I V E R ' S   P R O T O T Y P E S
//******************************************************************************
void    CEC_EXT_FUNC gm_CecInit(gmt_CEC_INIT far * Sp_CECInitValuesPtr);

BOOL    CEC_EXT_FUNC gm_CecTxFrame(BYTE B_DestLogAddr, BYTE B_DataLen, BYTE CEC_RAM * Bp_Data);
BOOL    CEC_EXT_FUNC gm_CecRxFrame(BYTE CEC_RAM * Bp_SrcLogAddr, BYTE CEC_RAM * Bp_DestLogAddr, BYTE CEC_RAM * Bp_DataLen, BYTE CEC_RAM * Bp_Data);

void    CEC_EXT_FUNC gm_CecIsr(void);

BYTE    CEC_EXT_FUNC gm_CecGetFrameStatus(void);
void    CEC_EXT_FUNC gm_CecClearFrameStatusBits(BYTE W_CECFrameStatusMask);
void    CEC_EXT_FUNC gm_CecSetAckControl(BOOL B_AckCtrl);
BOOL    CEC_EXT_FUNC gm_CecGetAckControl(void);
void    CEC_EXT_FUNC gm_CecSetLogAddr(BYTE B_LogAddr);
BYTE    CEC_EXT_FUNC gm_CecGetLogAddr(void);
DWORD   CEC_EXT_FUNC gm_CecGetTxTime(void);
BYTE    CEC_EXT_FUNC gm_CecGetBusStatus(void);
void    CEC_EXT_FUNC gm_CecEnable(void);
void    CEC_EXT_FUNC gm_CecDisable(void);
void    CEC_EXT_FUNC gm_CecClearRxBuffer(void);

gmt_CEC_LIB_VERSION CEC_EXT_FUNC gm_CecGetLibVersion(void);

//******************************************************************************
//  E X T E R N A L   F U N C T I O N S
//******************************************************************************
//extern  DWORD    EXT_FUNC gm_ReadRegDWord(WORD W_RegAddr);
extern  DWORD    EXT_FUNC gm_GetSystemTime(void);
extern  DWORD    EXT_FUNC gm_SclCecGetSystemTime100us(void);
extern  void     EXT_FUNC gm_SclCecPrint(char EXT_ROM *Bp_String, WORD W_Value);
extern  WORD     EXT_FUNC (*gm_SclCecGetSystemTime100usRAM)(void);

//******************************************************************************
//  CEC  DRV D E B U G: this part is for debug version of library
//******************************************************************************
#ifdef __PARADIGM__
	#pragma pack(push)
	#pragma pack(1)
#endif

typedef	struct
{
    BYTE B_SizeOfLogBuff;                    // size of Log buffer
    WORD CEC_RAM * Wp_CecLogBufferAddress;   // Pointer to CEC Log buffer
} gmt_CEC_DBG_INIT;

#ifdef __PARADIGM__
	#pragma pack(pop)
#endif

typedef enum
{
   gmd_CEC_DBG_FRAME_BUS        = 0,
   gmd_CEC_DBG_REG              = BIT14,
   #pragma warn -sig    // BIT15 Causes "Conversion may lose significant digits" warning because we are using the sign bit
   gmd_CEC_DBG_DATA             = BIT15,
   gmd_CEC_DBG_TYPES_MASK       = BIT15 | BIT14,
   #pragma warn +sig
   gmd_CEC_DBG_MAX_TYPES
}gmt_CEC_DBG_DATA_TYPES;


typedef enum
{
   gmd_CEC_DBG_WR_IDX           = BIT11,
   gmd_CEC_DBG_RD_IDX           = BIT12,
   gmd_CEC_DBG_RX_DATA          = BIT12 | BIT11,
   gmd_CEC_DBG_DATA_MASK        = BIT13 | BIT12 | BIT11
}gmt_CEC_DBG_DATA_SUBTYPES;

typedef enum
{
   gmd_CEC_DBG_NONE     = 0,
   gmd_CEC_DBG_INIT     = 1,
   gmd_CEC_DBG_DRV      = (1 << 1),
}gmt_CEC_DBG_MODULES;

void CEC_EXT_FUNC gm_CecDebugInit(gmt_CEC_DBG_INIT CEC_RAM * Sp_CecDbgInitValuesPtr );
BYTE CEC_EXT_FUNC gm_CecGetLogCount(void);
void CEC_EXT_FUNC gm_CecPrintLog(void);

#endif // __CEC_LIB_H__
