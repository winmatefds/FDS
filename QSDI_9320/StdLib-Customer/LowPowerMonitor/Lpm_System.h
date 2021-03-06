#ifndef __SYSTEM_H__
#define __SYSTEM_H__

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
// MODULE:      system.h
//
// USAGE:                                                         
//
//******************************************************************************

//******************************************************************************
// L P M    M O D U L E    S W I T C H 
//******************************************************************************
//Undefine to remove unused LPM modules and save code size

//#define CEC_SUPPORT				//2396 Bytes
//#define UART_DEBUG_SUPPORT       //To support UART debug, I2C SLV4 shall be disabled due to pins sharing (LPM_GPIO1,2)
//#define TCLK_TIMER_SUPPORT		//280 Bytes
#define IR_SUPPORT				//1314 Byte
#define KEYPAD_SUPPORT			// 500 Byte
//#define TOUCHPAD_SUPPORT

#define DDCCI_SUPPORT
#define I2C_SLAVE_SUPPORT
#define AUX_RX_SUPPORT
#define I2C_MASTER_SUPPORT
//#define BROWN_OUT_SUPPORT
//#define LPM_RESET_SUPPORT
//#define DP_PSEUDO_LINK
#define SIGNALING_DETECT
#define CABLE_DETECT_SUPPORT

#define POWER_MODE_HANDLE
#define CLOCK_ADJ_HANDLE


//******************************************************************************
// L P M    F U N C T I O N    S W I T C H 
//******************************************************************************

//====== Wake Up Settings ======
#define ANY_KEY_WAKEUP
#define HDMI_DVI_SRC_PWR_WAKEUP   //effective only for RD3 board
#define VGA_SRC_PWR_WAKEUP    //effective only for RD3 board
#define DP_SRC_PWR_WAKEUP     

//#define TMDS_CLKDET_BY_AUTOWAKE_IRQ   //undefined: Polling for signaling detection

//====== Cable Status Check Settings ======
#define CHECK_CPHY_CABLE_STS
//#define CHECK_VGA_CABLE_STS    //some VGA cables are not ground well that ADC value for cable status might be wrong!!
#define CHECK_DP_CABLE_STS

//====== Source Power Check Settings ======
#define CHECK_CPHY_SRC_PWR       //effective only for RD3 board
#define CHECK_VGA_SRC_PWR        //effective only for RD3 board
#define CHECK_DP_SRC_PWR     
//#define CHECK_AUX_SIG_WITH_DP_PWR  //avoid waking up by DP Aux noise

//====== Other Settings ======
#define USE_CALIBRATED_TRIM         1// 0         //Receive calibrated trim from Mission for RC-OSC?
#define LPM_OCMCLK_DEFAULT          0// 0        //0: LPM_RCOSC only,  1:MIS_TCLK+LPM_RCOSC,  2: MIS_OCMCLK+LPM_RCOSC
#define LPM_EDID_ACCESS_SEL         0// 1         //0: AUX2OCM+EDID cache   1: AUX2I2C+EEPROM   2: I2C SLAVE1 as virtual EDID (FW)
#define DDC1_FOR_LPM_I2C_MST_ONLY   0// 1        //0: DDC1 pins switched between I2C slave1 & LPM I2C master  1: DDC1 pins for LPM I2C master only
#define LPM_FORCE_POWER_DOWN        0// 1        //1: LPM will force Mission power down when Mission can't send power off command in time

//====== Debug Settings ======
//Mission takes PBus and get PD information via 0x9D0E (LPM_CSM_HPD_PD_IN_STATUS), So default is disable 
//#define PD_MONITORED_BY_LPM_GPIO			


#endif //__SYSTEM_H__

