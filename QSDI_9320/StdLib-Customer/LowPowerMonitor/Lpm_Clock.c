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
// MODULE:      Lpm_Clock.c
//
// USAGE:       
//
//******************************************************************************

//******************************************************************************
//
//  I N C L U D E    F I L E S
//
//******************************************************************************
#include ".\LowPowerMonitor\Lpm_All.h"

#ifdef CLOCK_ADJ_HANDLE

//#define LPM_DEBUG

#ifdef LPM_DEBUG
#define msg(string)         gm_Print(string,0)
#define msg_i(string,value) gm_Print(string,value)
#else
#define msg(string)
#define msg_i(string,value)
#endif

//******************************************************************************
//
//  C O D E
//
//******************************************************************************
#if (LPM_OCMCLK_DEFAULT != 0)
void Lpm_ClockSwitch2TCLK(void)
{
   LpmOcmClk = SYSTEM_TCLK_FREQ;
   gm_WriteRegWord(LPM_CLOCK_CTRL, LPM_TCLK_SEL);
   #ifdef UART_DEBUG_SUPPORT
   Lpm_UartInit(LpmOcmClk, UART_BAUD_RATE, Ba_CommBufUart0, sizeof(Ba_CommBufUart0));
   #endif
   Lpm_TimerInit(SYSTEM_TCLK_FREQ);
   Lpm_I2CMasterSetClock();   //Keep I2C master clock in 100KHz
}
#endif

void Lpm_ClockSwitch2RCOSC(gmt_LPM_OCMCLK_SEL Lpm_OcmClkSel)
{
   DWORD UartBaudRate;
   //Change LPM OCM clock source to RC-OSC
   gm_WriteRegWord(LPM_RCOSC,(RcOscTrim<<LPM_RC_OSC_TRIM_SHIFT)|LPM_RC_OSC_EN);

   switch(Lpm_OcmClkSel)
   {
      case TCLK_DIV_0:
      default:
      {
         LpmOcmClk = LpmRcOscClk;
         UartBaudRate = UART_BAUD_RATE;
         msg("--LPM OCM Clock = RC-OSC Clock"); 
         break;
      }
      case TCLK_DIV_1:
      {
         LpmOcmClk = LpmRcOscClk/2;
         UartBaudRate = UART_BAUD_RATE;      
         msg("--LPM OCM Clock = RC-OSC Clock/2"); 
         break;
      }
      case TCLK_DIV_2:
      {
         LpmOcmClk = LpmRcOscClk/4;
         UartBaudRate = UART_BAUD_RATE;      
         msg("--LPM OCM Clock = RC-OSC Clock/4"); 
         break;
      }
      case TCLK_DIV_3:
      {
         LpmOcmClk = LpmRcOscClk/8;
         UartBaudRate = UART_BAUD_RATE;      
         msg("--LPM OCM Clock = RC-OSC Clock/8"); 
         break;
      }
      case TCLK_DIV_4:
      {
         LpmOcmClk = LpmRcOscClk/16;
         UartBaudRate = 4800L;      
         msg("--LPM OCM Clock = RC-OSC Clock/16"); 
         break;
      }
      case TCLK_DIV_5:
      {
         LpmOcmClk = LpmRcOscClk/32;
         UartBaudRate = 2400L;      
         msg("--LPM OCM Clock = RC-OSC Clock/32"); 
         break;
      }
      case TCLK_DIV_6:
      {
         LpmOcmClk = LpmRcOscClk/64;
         UartBaudRate = 1200L;      
         msg("--LPM OCM Clock = RC-OSC Clock/64"); 
         break;
      }
      case TCLK_DIV_7:
      {
         LpmOcmClk = LpmRcOscClk/128;
         UartBaudRate = 1200L;     
         msg("--LPM OCM Clock = RC-OSC Clock/128"); 
         break;
      }
   }

   gm_WriteRegWord(LPM_CLOCK_CTRL,(2<<LPM_OCMCLK_SEL_SHIFT)|(Lpm_OcmClkSel<<LPM_OCMCLK_DIV_SHIFT));//RC-OSC drives LPM_OCMCLK
   #ifdef UART_DEBUG_SUPPORT
   Lpm_UartInit(LpmOcmClk, UartBaudRate, Ba_CommBufUart0, sizeof(Ba_CommBufUart0)); 
   #else
   UNUSED_VAR(UartBaudRate);
   #endif
   Lpm_TimerInit(LpmOcmClk);
   Lpm_I2CMasterSetClock();   //Keep I2C master clock in 100KHz
}

#else
void Lpm_ClockSwitch2TCLK(void)
{
}
void Lpm_ClockSwitch2RCOSC(gmt_LPM_OCMCLK_SEL Lpm_OcmClkSel)
{
   UNUSED_VAR(Lpm_OcmClkSel);
}
#endif
//******************************************************************************

