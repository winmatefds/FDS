#ifndef __LPM_MAIN_H__
#define __LPM_MAIN_H__

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
//============================================================
//
// MODULE:      Lpm_Main.h
//
// USAGE:       
//
//******************************************************************************

#define LPM_STACK_MARK     0x5555
#define LPM_STACK_STOP     0xFF400UL
#define LPM_STACK_START    0xFFC00UL
#define LPM_STACK_SIZE     (LPM_STACK_START - LPM_STACK_STOP)


extern BYTE RcOscTrim;
extern DWORD LpmRcOscClk;
extern DWORD LpmOcmClk;
extern BYTE PBusReqLock;

typedef struct
{
    WORD PermanentPeripheralControl:1;
} TConfiguration;
extern ROM TConfiguration Configuration;

typedef enum
{
	gmd_DDC1,
	gmd_DDC2,
	gmd_DDC3,
	gmd_DDC4,
	gmd_AUTOWAKE,
	gmd_ASYNCDET,
	gmd_TMDSCLK,
	gmd_I2CM,
	gmd_SWDT,
	gmd_IR,
	gmd_CEC,
	gmd_MISSION_MAIL_OUT,
	gmd_CSM_HPD_EDPD,
	gmd_TX_HPD,
	gmd_EXTINT,
	gmd_TCLK_TIMER
} gmt_ISR_SOURCE;


typedef enum
{
		LpmHdrID_SysDebug,
		
	#ifdef I2C_SLAVE_SUPPORT
		LpmHdrID_I2cSlave,
	#endif
	#ifdef DDCCI_SUPPORT
		LpmHdrID_DDC,
	#endif	  
	#ifdef CEC_SUPPORT
		LpmHdrID_Cec,
	#endif
	#ifdef TCLK_TIMER_SUPPORT			
		LpmHdrID_TclkTimer,
	#endif	
   	#ifdef IR_SUPPORT
		LpmHdrID_Ir,        
	#endif
	#ifdef KEYPAD_SUPPORT			
		LpmHdrID_Keypad,
	#endif
	#ifdef TOUCHPAD_SUPPORT
		LpmHdrID_TouchPad,
	#endif	
	#ifdef SIGNALING_DETECT
		LpmHdrID_SgDeTect,
	#endif
	#ifdef CABLE_DETECT_SUPPORT
		LpmHdrID_CabDeTect,
	#endif
		LpmHdrID_END,
}gmt_LpmHdrId;

typedef enum
{
	CPhyNum_0,
	CPhyNum_1,
	CPhyNum_2,
	CPhyNum_End,
	CPhyNum_Num = CPhyNum_End
}gmt_CPhyNum;


typedef enum
{
	CPhyType_None,
	CPhyType_DVI,
	CPhyType_HDMI,
	CPhyType_DP
}gmt_CPhyType;

typedef enum
{
	Port_None,
	Port_DVI,
	Port_HDMI,
	Port_DP_A,
	Port_DP_B,
	Port_VGA,
	Port_End
}gmt_PortChannel;

typedef enum 
{ 
	ALL_BOARDS_CVBS1, 		// 0
	ALL_BOARDS_SVIDEO1,		// 1
	ALL_BOARDS_COMP1, 		// 2
	ALL_BOARDS_VGA1, 		// 3
	ALL_BOARDS_DVI1, 		// 4
	ALL_BOARDS_DVI2, 		// 5
	ALL_BOARDS_DIP1, 		// 6
	ALL_BOARDS_HDMI, 		// 7
	ALL_BOARDS_HDMI2, 		// 8
	ALL_BOARDS_HDMI3, 		// 9
	ALL_BOARDS_DP1, 		// 10
	ALL_BOARDS_DP2, 		// 11
   ALL_BOARDS_DP3, 		// 12 //Special for 4k2k board
	ALL_BOARDS_SLAVE1, 		// 13
	NUM_MASTER_LIST, 		// 14
} ALL_BOARDS_INPUT_PORT;


typedef enum
{
	BUS_MISSION,
	BUS_LPM
}gmt_BusOwn;


//For IPC command gmd_MESSAGE_CHECK_STATUS
typedef enum
{
   PWR_SW_OFF,
   PWR_SW_ON,
   PWR_SW_SIGDET_ON,
   PWR_SW_NUM
}gmt_Power_Mode_Switch;

typedef enum
{
   LPM_WAKEUP_CPHY0,
   LPM_WAKEUP_CPHY1,
   LPM_WAKEUP_CPHY2,
   LPM_WAKEUP_ASDET,
   LPM_WAKEUP_KEYPAD,
   LPM_WAKEUP_AC_ON,
   LPM_WAKEUP_DET_NUM
}gmt_LPM_Wakeup_Detect;

typedef enum
{
   BOARD_EV1,
   BOARD_EV2,
   BOARD_RD1,
   BOARD_RD1_REV_B,
   BOARD_RD2,
   BOARD_EV3,
   BOARD_RD3,
   BOARD_4K2K,
   BOARD_NUM,
}gmt_BoardType;

typedef enum
{
   CHIPID_4K2K_NONE,
   CHIPID_4K2K_L,
   CHIPID_4K2K_FE,
   CHIPID_4K2K_R   
}gmt_4k2k_ChipID;

//#define NumOfComboPhy	3

typedef void (* gmt_HandlerPrototype)(void);

gmt_RET_STAT Lpm_InterruptEnable(gmt_ISR_SOURCE W_IsrSource);
gmt_RET_STAT Lpm_InterruptDisable(gmt_ISR_SOURCE W_IsrSource);
void interrupt Lpm_IrqZeroManager(void);
void interrupt Lpm_IrqOneManager(void);
void interrupt Lpm_IrqTwoManager(void);
void interrupt Lpm_IrqThreeManager(void);
void interrupt Lpm_IrqFourManager(void);
void interrupt Lpm_IrqFiveManager(void);
void interrupt Lpm_isrDivideErrorException(void);
void interrupt Lpm_isrNonMaskableInterrupt(void);
void interrupt Lpm_isrInt0Overflow(void);
void interrupt Lpm_isrArrayBoundsException(void);
void interrupt Lpm_isrUnusedOpcodeException(void);
void interrupt Lpm_isrEscOpcodeException(void);

BOOL IsMissionPoweredUp(void);
void MissionPowerUp(void);
void Lpm_SystemInit(void);
void Lpm_ResetLpmDevices(gmt_BusOwn PBusOwner);
void Lpm_SharedPBusSwitch(gmt_BusOwn PBusOwner);
void Lpm_SharedPBusHandler(void);
void Lpm_SharedHostBusHandler(void);
void Lpm_CalibrationHandler(void);
void Lpm_GpioControlHandler(void);
void Lpm_DebugCmdHandler(void);
void Lpm_MissionTaskHandler(void);
void far Lpm_Main(void);
void Lpm_ClearMailbox(void);
void Lpm_RedLedCtrl(BYTE Switch);
void Lpm_MfpInit(void);
void Lpm_MainHandlersTurn(gmt_LpmHdrId B_HdrId, BOOL B_OnOff);
void Lpm_TestHandler(void);
void Lpm_StackMonitorInit(void);


#endif // __LPM_MAIN_H__
