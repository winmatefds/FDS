/*
	$Workfile:   Osd_AppDependent.c  $
	$Revision: 1.6 $
	$Date: 2013/03/06 07:13:10 $
*/
#define __OSD_APPDEPENDENT_C__
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
// MODULE:     	Osd_AppDependent.c
//
// USAGE:	This module contains application-dependent standard osd adjusters
//
//******************************************************************************

//******************************************************************************
//  I N C L U D E    F I L E S
//******************************************************************************

#include "system\all.h"
#include "StdLibInc\gm_Register.h"

//******************************************************************************
//  D E F I N E S
//******************************************************************************

//
//	OsdBlendSplashScreen defines
//
#define NUM_BLEND_ENTRIES		8
#define OSD_BLEND_SPEED		50				//Time between each blend level in ms (ie. lower = faster)

static 	BYTE B_OsdBlendSplashScreenState = 0;	//State machine variable for OsdBlendSplashScreen
static 	SBYTE SB_OsdBlendPassCount = 0;			//Counter variable for OsdBlendSplashScreen
static	SBYTE SB_OsdBlendDirection = 0;			//Variable for OsdBlendSplashScreen direction
static	WORD W_PrevSystemTime = 0;				//Previous system time in ms for OsdBlendSplashScreen

//******************************************************************************
//  S T A T I C    F U N C T I O N S
//******************************************************************************

//******************************************************************************
//
// FUNCTION       : static BYTE Normalize(SBYTE i)
//
// USAGE          :
//
// INPUT          : i
//
// OUTPUT         :
//
// GLOBALS        :
//
// USED_REGS      :
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
static BYTE Normalize(SBYTE i)
{
   if (i < 0)
      return 0;
   else if (i > 0xf)
      return 0xf;
   else
      return i;

}

//******************************************************************************
//  C O D E
//******************************************************************************


//******************************************************************************
//
// FUNCTION       : void OsdBlendSplashScreen(void)
//
// USAGE          : Called by OSD WB project and main loop when splash screen is dislayed.
//                  Performs blending of image in a gradual matter in and out
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        : None
//
// USED_REGS      : OSD_BLEND_TABLE0		(W)
//                  OSD_BLEND_TABLE1		(W)
//                  PB_CTRL					(R/W)
//                  DISP_LUT_CONTROL		(W)
//                  PB_BKGND				(W)
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void OsdBlendSplashScreen(void)
{
   static WORD gamma_save;
   SBYTE j = 0;
   BYTE OffsetLag = 3;
   BYTE MaxValue = 0xf;
   BYTE Blend[NUM_BLEND_ENTRIES];
   BYTE BitmapBlend;
   WORD W_CurrentSystemTime = 0;

   switch (B_OsdBlendSplashScreenState)
   {
      case 0:

         //
         // Disable gamma tables first because gamma affects the background but not the OSD image. When
         // that happens, there is a border around the image so we diable gamma first and enable it after the
         // splash screen.
         //
#if defined(UserPrefGammaTable)
         gamma_save = gm_ReadRegWord(DISP_LUT_CONTROL);
         gm_WriteRegWord (DISP_LUT_CONTROL, 0);	// Disable Gamma Tables
#endif
         gm_WriteRegWord(OSD_BLEND_TABLE0, 0xffff);
         gm_WriteRegWord(OSD_BLEND_TABLE1, 0xffff);
         gm_SyncUpdateWait(gmvb_MainChannel, OUTPUT_CLOCK);

         //
         // Start Blending in the Splash Screen
         //
         gm_OsdShow();

         gm_BlenderOnOff(CH_A, FALSE);
         gm_BlenderOnOff(CH_B, FALSE);
      //   gm_WriteRegWord(PB_BKGND, 0xFFFF); //0x8C51);			//custom BG color RGB(136,136,136)

	   gm_WriteRegWord(PB_BKGND, 0);				//custom BG color RGB(0,0,0)
         gm_SyncUpdateWait(0, OUTPUT_CLOCK);		// Display clock - no need for channel number

         W_PrevSystemTime = gm_GetSystemTimeW();		//Store previous system time

         B_OsdBlendSplashScreenState = 1;			// Advance to next state

      case 1:
			{
				#ifdef ST_4K2K_93xx_BOARD
				BYTE RLDelay = 0;
				#endif
	         W_CurrentSystemTime = gm_GetSystemTimeW();	//Store current system time

				// SS
				#ifdef ST_4K2K_93xx_BOARD
				if(GetChipID_4K2K()==CHIPID_4K2K_FE)
					RLDelay = 150;
				//else if(GetChipID_4K2K()==CHIPID_4K2K_L)
				//	RLDelay = 150;
				
				if ((W_CurrentSystemTime - W_PrevSystemTime) > RLDelay)
				#else
	         if ((W_CurrentSystemTime - W_PrevSystemTime) > OSD_BLEND_SPEED)	//Only blend if predefined time has elasped
	         #endif
	         {
	            W_PrevSystemTime = W_CurrentSystemTime;		//Store current system time

	            gm_WriteRegWord(OSD_BLEND_TABLE0, 0);
	            gm_WriteRegWord(OSD_BLEND_TABLE1, 0);

	            for (j = 0; j < NUM_BLEND_ENTRIES; j++)
	            {
	               SBYTE BlendValue = SB_OsdBlendPassCount - (j * OffsetLag);
	               if (SB_OsdBlendDirection == 0)
	                  Blend[j] = Normalize(MaxValue - BlendValue);
	               else
	                  Blend[j] = Normalize(BlendValue);
	            }
	            if (SB_OsdBlendDirection == 0)
	            {
	               BitmapBlend = Blend[1];
	            }
	            else
	            {
	               BitmapBlend = Blend[7];
	            }
	            gm_WriteRegWord(OSD_BLEND_TABLE0, Blend[3] << 12 | Blend[2] << 8 | Blend[1] << 4 | BitmapBlend);
	            gm_WriteRegWord(OSD_BLEND_TABLE1, Blend[7] << 12 | Blend[6] << 8 | Blend[5] << 4 | Blend[4]);
	            gm_SyncUpdateWait(0, OUTPUT_CLOCK);

	            SB_OsdBlendPassCount++;						// Increment number of blending passes

					// 20130305 ... FE Splash screen
					#ifdef ST_4K2K_93xx_BOARD
					if(GetChipID_4K2K()==CHIPID_4K2K_FE)
					{
						if (SB_OsdBlendPassCount == (0x01 + (NUM_BLEND_ENTRIES * OffsetLag) - 1) )
		            {
		            	gm_WriteRegWord(DISPLAY_CONTROL,0x07);
						}
					}
					#endif
					
	            if (SB_OsdBlendPassCount == (MaxValue + (NUM_BLEND_ENTRIES * OffsetLag) - 1) )
	            {
	               if (SB_OsdBlendDirection == 1)				// Check to see if two passes have elapsed
	                  B_OsdBlendSplashScreenState = 2;		// Advance to next state

	               SB_OsdBlendDirection++;						// Switch blend direction
	               SB_OsdBlendPassCount = 0;					// Reset number of blend passes
	            }

	         }
      	}
         break;

      case 2:

			// 20130305
			#ifdef ST_4K2K_93xx_BOARD
			if(GetChipID_4K2K()==CHIPID_4K2K_FE)
			{
				gm_WriteRegWord(DISPLAY_CONTROL,0x01);	// 20130305
				SocIPCSyncInterruptViaSlaveFE();			// Send interrupt to R when SS done
			}
			#endif
			
			gm_OsdHide();

         //
         // Check if the user has not turned off gamma or there is a valid gamma table before enabling gamma
         //
#if defined(UserPrefGammaTable)
         gm_WriteRegWord(DISP_LUT_CONTROL, gamma_save);
#endif
         gm_WriteRegWord(OSD_BLEND_TABLE0, 0);		// Restore blender values
         gm_WriteRegWord(OSD_BLEND_TABLE1, 0);
         gm_WriteRegWord(PB_BKGND, 0);				//custom BG color RGB(0,0,0)
         gm_SyncUpdateWait(gmvb_MainChannel, OUTPUT_CLOCK);
         B_OsdBlendSplashScreenState = 3;		// Advance to next state
         break;

      case 3:
         // Send MTO_SplashScreen to kick OSD from splash screen state to default state
         OSDRequestMsg(gmvb_MainChannel, MTO_SplashScreen, REQUEST_NOW);
         B_OsdBlendSplashScreenState = 4;		// Advance to next state
         break;

      case 4:
         // Send System Startup Message while in Default state
         OSDRequestMsg(gmvb_MainChannel, MTO_SystemStartUp, REQUEST_NOW);
         B_SplashComplete = TRUE;				// Flag splash screen complete
         B_OsdBlendSplashScreenState = 5;		// Advance to next state
         break;

      default:
         // Do nothing in this state
         break;

   }

}

//******************************************************************************
//
// FUNCTION       : void OsdBlend(void)
//
// USAGE          : Called by OSD WB project when OSD menu is dislayed.  Programs Blend table regs
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        : None
//
// USED_REGS      : OSD_BLEND_TABLE0, OSD_BLEND_TABLE1
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void OsdBlend(void)
{
/*
   if (!OsdTtxOnFlag)
   {
      gm_WriteRegWord(OSD_BLEND_TABLE0, (0x0003|gm_ReadRegWord(OSD_BLEND_TABLE0)));
      gm_SyncUpdate(0, OUTPUT_CLOCK);		// Display clock - no need for channel number
   }
   else
   {
      gm_WriteRegWord(OSD_BLEND_TABLE0, 0x0);
      gm_SyncUpdate(0, OUTPUT_CLOCK);
   }
   */
   		gm_WriteRegWord(OSD_BLEND_TABLE0,UserPrefOsdBlend*3);
		gm_SyncUpdate(0, OUTPUT_CLOCK);
}

//*********************************  END  **************************************
