/*
	$Workfile:   Adjustor.c  $
	$Revision: 1.8 $
	$Date: 2012/07/09 06:40:44 $
*/
#define __ADJUSTOR_C__

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
// MODULE:      Adjustor.c
//
// USAGE:       This module contains set of utility functions called from
//				OSD adjusters
//
//******************************************************************************


//******************************************************************************
//  I N C L U D E    F I L E S
//******************************************************************************

#include "System\all.h"
#include "StdLibInc\gm_Register.h"
#include "TestHarness\AppsTest.h"
//******************************************************************************
//  D E F I N E S
//******************************************************************************

//#define _DEBUG
#ifdef _DEBUG
#define msg(string)         	gm_Print(string,0)
#define msg_i(string,value) 	gm_Print(string,value)
#else
#define msg(string)
#define msg_i(string,value)
#endif

//*****************************************************************************
//  C O D E
//*****************************************************************************

//******************************************************************************
//
// FUNCTION       : void MadiSetup(gmt_PHY_CH B_Channel)
//
// USAGE          : Call this function to configure MADI,TNR,CCS based on input port
//
// INPUT          : B_Channel        : Current input channel
//
// OUTPUT         : None
//
// GLOBALS        : None
//
// USED_REGS      : Setup set of TNR, MADI, CCS registers in gm_AdaptiveGlobalMotion()
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void MadiSetup(gmt_PHY_CH B_Channel)
{
   if (B_Channel)
   {
   }
}

//******************************************************************************
//
// FUNCTION       : void SetupHTotal(WORD W_Htotal)
//
// USAGE          : Calls gm_SetupHTotal() to set up the input pixel clock frequency.
//
// INPUT          : W_Htotal = input H total value
//
// OUTPUT         : None
//
// GLOBALS        : None
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
#if FEATURE_AIP == ENABLE
void SetupHTotal(WORD W_Htotal)
{
   BYTE B_Channel;
   
   if (gm_IsInputTripleADC(gmvb_MainChannel))
      B_Channel = gmvb_MainChannel;
   else if (gm_IsInputTripleADC(gmvb_PipChannel))
      B_Channel = gmvb_PipChannel;
   else
      return;

   gm_SetupHTotal(W_Htotal, B_Channel);
   stInput[B_Channel].Htotal = W_Htotal;
}
#endif




void AdjustBackLight(void)
{
	BYTE backlight_temp,range_temp;	
	//
	//	Assuming back-light takes domain value 0-255
	//
/*
	   BYTE Data;
 BYTE Addr = 0x7;
 
   gm_Write2WireBlock(1, 0x42, (BYTE*) &Addr, 1, FALSE);
   gm_Read2WireBlock(1, 0x42, (BYTE*) &Data, 1, TRUE);

Addr = 0x10;

      gm_Write2WireBlock(1, 0x42, (BYTE*) &Addr, 1, FALSE);
   gm_Read2WireBlock(1, 0x42, (BYTE*) &Data, 1, TRUE);

*/
	// NOTE: to be implemented the UserPrefBackLight
/*
	#if (PWM_FREQ==DIV_1024)	
		gm_WriteRegWord(PWM3_CONFIG, 0x0043);
	#elif (PWM_FREQ==DIV_8)	
		gm_WriteRegWord( PWM3_CONFIG, 0x000F);
	#elif (PWM_FREQ==DIV_512)	
		gm_WriteRegWord( PWM3_CONFIG, 0x0041);
	#else
		gm_WriteRegWord(PWM3_CONFIG, 0x0003);
	#endif
*/

 	//gm_WriteRegWord(PWM3_CTRL,  (0x6E | ( (WORD)UserPrefBackLight<< 8) ) );// Jack modify for ACS 



	#if 0//(_BACKLIGHT_PWM_INVERSE==0)  //DC
		range_temp=((BYTE)gm_GetAdjusterMax(ADJV_BackLight))+UserPrefMinBrightness;
		backlight_temp=(UserPrefBackLight+UserPrefMinBrightness);
		gm_WriteRegWord(PWM3_CTRL,  (range_temp | ( ((WORD)(backlight_temp)) << 8) ) );
	//			gm_Printf("backlight_temp=%d",backlight_temp);
	//	gm_Printf("range_temp=%d",range_temp);

	//	gm_Printf("range_temp=%d",range_temp);
	#else
		range_temp=((BYTE)gm_GetAdjusterMax(ADJV_BackLight))+UserPrefMinBrightness;
		backlight_temp=range_temp-(UserPrefBackLight+UserPrefMinBrightness);
		gm_WriteRegWord(PWM3_CTRL,  (range_temp | ( ((WORD)(backlight_temp)) << 8) ) );
	//			gm_Printf("backlight_temp=%d",backlight_temp);
	//	gm_Printf("range_temp=%d",range_temp);




	#endif

}


void AdjustBurnInMode(void)
{
            BYTE i;
            //WORD W_RESTORE_PB_BKGND;
	WORD BurnInColor[7] = //Godwin add 950123
	{
	0,0x1f,0x7f,0x7e0,0xf800,0xf81f,0xffff
	};

	 

              gm_ClearRegBitsWord(PB_CTRL, PB_MAIN_ENABLE);
 		gm_ClearRegBitsWord(PB_CTRL, PB_PIP_ENABLE);


	while (UserPrefBurnInMode)

        {
            i++;
	if(i>=8)
		i=0;

		gm_WriteRegWord(PB_BKGND, BurnInColor[i]);
		   
		gm_SetRegBitsWord(HOST_CONTROL, ODP_FORCE_UPDATE);
	
               gm_Delay100ms(30);//0);


        }
              gm_SetRegBitsWord(PB_CTRL, PB_MAIN_ENABLE);
 		gm_SetRegBitsWord(PB_CTRL, PB_PIP_ENABLE);
		gm_SetRegBitsWord(HOST_CONTROL, ODP_FORCE_UPDATE);
}
/***********************************  END  ************************************/
