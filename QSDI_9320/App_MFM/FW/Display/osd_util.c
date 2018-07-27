/*
	$Workfile:   osd_util.c  $
	$Revision: 1.6 $
	$Date: 2012/06/28 03:44:50 $
*/
#define __OSD_UTIL_C__
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
// MODULE:     	osd_adj.c
//
// USAGE:		This module contains standard osd adjusters that would not normally be changed.
//				They deal more with the osd library and how the osd is drawn.
//				Functions like ShowOsd, HideOsd, AdjustOsdLanguage, etc.
//
//******************************************************************************

//******************************************************************************
//  I N C L U D E    F I L E S
//******************************************************************************

#include "system\all.h"

//******************************************************************************
//  E X T E R N S
//******************************************************************************

//******************************************************************************
//  D E F I N E S
//******************************************************************************

#define _DEBUG

#ifdef _DEBUG
#define  msg(a)            		gm_Print(a,0)
#define  msg_i(a,b)        	 	gm_Print(a,b)
#else
#define  msg(a)
#define  msg_i(a,b)
#endif

//******************************************************************************
//  G L O B A L    V A R I A B L E S
//******************************************************************************

//******************************************************************************
// S T A T I C   F U N C T I O N S
//******************************************************************************

//******************************************************************************
//  C O D E
//******************************************************************************
#ifdef Action_ShowOsd_WB_Used
//******************************************************************************
//
// FUNCTION       : void ShowOsd(void)
//
// USAGE          : Shows OSD on the display
//
// INPUT          : None
//
// OUTPUT         : none
//
// GLOBALS        : None
//
// USED_REGS      : None
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void ShowOsd(void)
{
   gm_OsdShow();
}
#endif //#ifdef Action_Osd_Show_WB_Used

#ifdef Action_HideOsd_WB_Used
//******************************************************************************
//
// FUNCTION       : void HideOsd(void)
//
// USAGE          : Removes OSD from the display
//
// INPUT          : None
//
// OUTPUT         : none
//
// GLOBALS        : None
//
// USED_REGS      : None
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void HideOsd(void)
{
   gm_OsdHide();
}
#endif //#ifdef Action_Osd_Hide_WB_Used

#ifdef Action_SetOsdTimeout_WB_Used
//******************************************************************************
//
// FUNCTION       : void SetOsdTimeout(void)
//
// USAGE          : Sets OSD hide timeout to a specificed value
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        : gB_OsdTimerStart		(W)
//                  B_OSD_TimeoutTid		(R)
//                  W_OsdTimerResolution	(R)
//                  ModeIndependentUserPreferences.OsdTimeout	(R)
//
// USED_REGS      : None
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void SetOsdTimeout(void)
{
   if (UserPrefOsdTimeout)
   {
      gB_OsdTimerStart[B_OSD_TimeoutTid] = OSD_TIMER_START;
   }
   else
   {
      gB_OsdTimerStart[B_OSD_TimeoutTid] = OSD_TIMER_STOP;
   }

   //
   // OSD menu timeout in second base unit
   //
   gW_OsdTimers[B_OSD_TimeoutTid] =
      (WORD)(((DWORD)UserPrefOsdTimeout * 100) / W_OsdTimerResolution);
}
#endif

#ifdef ADJUSTER_OSDTIMEOUT_DECLARED
//******************************************************************************
//
// FUNCTION       : void AdjustOsdTimeout(void)
//
// USAGE          : Adjuster wrapper for OSD hide timeout user preference
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        : None
//
// USED_REGS      : None
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void AdjustOsdTimeout(void)
{
}
#endif

#ifdef Action_ResetTimer_WB_Used
//******************************************************************************
//
// FUNCTION       : void ResetTimer(void)
//
// USAGE          : Sets OSD timers to a specificed timeout value
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        : gB_OsdTimerStart		(W)
//                  gW_OsdTimers		(W)
//
// USED_REGS      : None
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void ResetTimer(void)
{
   DWORD D_temp = gm_OsdGetActionTripleByteParameter();
   WORD W_Value = (WORD)D_temp;
   BYTE B_OsdTimer = (BYTE)((D_temp >> 16) & TIMER_ID_MASK);

   if (W_Value != 0)
   {
      gB_OsdTimerStart[B_OsdTimer] = OSD_TIMER_START;
   }
   else
   {
      gB_OsdTimerStart[B_OsdTimer] = OSD_TIMER_STOP;
   }

   gW_OsdTimers[B_OsdTimer] = W_Value;
}
#endif

#ifdef ADJUSTER_OSDLANGUAGE_DECLARED
//******************************************************************************
//
// FUNCTION       : void AdjustOsdLanguage(void)
//
// USAGE          : Adjuster wrapper for OSD language user preference.
//                  It also sets osd library language settings.
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        : ModeIndependentUserPreferences.OsdLanguage	(R)
//
// USED_REGS      : None
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void AdjustOsdLanguage(void)
{
   gm_SetOsdLanguage(UserPrefOsdLanguage);
}
#endif

#if ((defined ADJUSTER_OSDVPOSITION_DECLARED) || (defined ADJUSTER_OSDHPOSITION_DECLARED))
//******************************************************************************
//
// FUNCTION       : void SetOsdTilePosition(WORD W_TileId, WORD W_TileHpos, WORD W_TileVpos)
//
// USAGE          : Sets OSD tile position taking into account OSD rotation
//
// INPUT          : W_TileId - tile id of a tile
//                  W_TileHpos - new X position
//                  W_TileVpos - new Y position
//
// OUTPUT         : None
//
// GLOBALS        : gmv_OsdDialogs		(R)
//                  gmvb_OsdActiveDialog	(R)
//                  ModeIndependentUserPreferences.OsdRotation		(R)
//
// USED_REGS      : None
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void SetOsdTilePosition(WORD W_TileId, WORD W_TileHpos, WORD W_TileVpos)
{
   gmt_OSD_DIALOG_CTRL ROM *Sp_DialogCtrl = (gmt_OSD_DIALOG_CTRL ROM *)(gmv_OsdDialogs[gmvb_OsdActiveDialog]);
   WORD W_XPos, W_YPos, W_Interval, W_ADJV_OsdPosition;
   SDWORD SDW_AdjMin, SDW_AdjMax, SDW_Pos;

   // Set the interval width
   if ((UserPrefOsdRotation == OSD_NON_ROTATED)
         ||	(UserPrefOsdRotation == OSD_ROTATED_180))
   {
      W_ADJV_OsdPosition = ADJV_OsdHPosition;
      W_Interval = PanelWidth - Sp_DialogCtrl->MaxXpos;
   }
   else
   {
      W_ADJV_OsdPosition = ADJV_OsdVPosition;
      W_Interval = PanelWidth - Sp_DialogCtrl->MaxYpos;
   }

   if (W_Interval > PanelWidth) //checks for overflow **caused if screen size is too small
   {
      W_Interval = 0;
   }

   SDW_AdjMin = gm_GetAdjusterMin(W_ADJV_OsdPosition);
   SDW_AdjMax = gm_GetAdjusterMax(W_ADJV_OsdPosition);
   SDW_Pos = gm_GetAdjuster(W_ADJV_OsdPosition);

   // Rescale the adjustor to the full screen size.
   W_XPos = (WORD)(((SDW_Pos - SDW_AdjMin ) * W_Interval) / ( SDW_AdjMax - SDW_AdjMin ));

   if ((UserPrefOsdRotation == OSD_NON_ROTATED)
         ||	(UserPrefOsdRotation == OSD_ROTATED_180))
   {
      W_ADJV_OsdPosition = ADJV_OsdVPosition;
      W_Interval = PanelHeight - Sp_DialogCtrl->MaxYpos;
   }
   else
   {
      W_ADJV_OsdPosition = ADJV_OsdHPosition;
      W_Interval = PanelHeight - Sp_DialogCtrl->MaxXpos;

   }

   if (W_Interval > PanelHeight) //checks for overflow **caused if screen size is too small
   {
      W_Interval = 0;
   }

   SDW_AdjMin = gm_GetAdjusterMin(W_ADJV_OsdPosition);
   SDW_AdjMax = gm_GetAdjusterMax(W_ADJV_OsdPosition);
   SDW_Pos = gm_GetAdjuster(W_ADJV_OsdPosition);

   // Rescale the adjustor to the full screen size.
   W_YPos = (WORD)((( SDW_Pos - SDW_AdjMin ) * W_Interval) / ( SDW_AdjMax - SDW_AdjMin ));

   if ((UserPrefOsdRotation == OSD_NON_ROTATED)
         ||	(UserPrefOsdRotation == OSD_ROTATED_180))
   {
      gm_OsdSetTilePosition(W_TileId, W_TileHpos + W_XPos, W_TileVpos + W_YPos);
   }
   else
   {
      gm_OsdSetTilePosition(W_TileId, W_TileHpos + W_XPos,  W_TileVpos - W_YPos);
   }
}
#endif //#if ((defined ADJUSTER_OSDVPOSITION_DECLARED) || (defined ADJUSTER_OSDHPOSITION_DECLARED))

#ifdef ADJUSTER_OSDHPOSITION_DECLARED
//******************************************************************************
//
// FUNCTION       : void AdjustOsdHPosition(void)
//
// USAGE          : Adjuster wrapper for OSD horizontal position
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        : gmv_OsdTilePosition	(R)
//
// USED_REGS      : None
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void AdjustOsdHPosition(void)
{
   WORD i;
   WORD W_flag = 1;

   while (1)
   {
      for (i = 0 ; i < MAX_NUMBER_OF_TILES ; i++)
      {
         if (gmv_OsdTilePosition[i].W_XPos != UNKNOWN_TILE_DIALOG_POSITION)
         {
            SetOsdTilePosition(i, gmv_OsdTilePosition[i].W_XPos, gmv_OsdTilePosition[i].W_YPos);
         }
      }

      if (W_flag)
      {
         ReverseTileSet();
         W_flag = 0;
      }
      else
      {
         break;
      }
   }
}
#endif

#ifdef ADJUSTER_OSDVPOSITION_DECLARED
//******************************************************************************
//
// FUNCTION       : void AdjustOsdVPosition(void)
//
// USAGE          : Adjuster wrapper for OSD vertical position
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        : gmv_OsdTilePosition		(R)
//
// USED_REGS      : None
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void AdjustOsdVPosition(void)
{
   WORD i;
   WORD W_flag = 1;

   while (1)
   {
      for (i = 0 ; i < MAX_NUMBER_OF_TILES ; i++)
      {
         if (gmv_OsdTilePosition[i].W_YPos != UNKNOWN_TILE_DIALOG_POSITION)
         {
            SetOsdTilePosition(i, gmv_OsdTilePosition[i].W_XPos,
                               gmv_OsdTilePosition[i].W_YPos);
         }
      }

      if (W_flag)
      {
         ReverseTileSet();
         W_flag = 0;
      }
      else
      {
         break;
      }
   }

}
#endif

#ifdef ADJUSTER_OSDROTATION_DECLARED
//******************************************************************************
//
// FUNCTION       : void AdjustOsdRotation(void)
//
// USAGE          : Adjuster wrapper for OSD rotation
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        : ModeIndependentUserPreferences.OsdHPosition	(R/W)
//                  ModeIndependentUserPreferences.OsdHPosition		(R/W)
//                  ModeIndependentUserPreferences.OsdRotation		(R)
//
// USED_REGS      : OSD_CONTROL	(W)
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void AdjustOsdRotation(void)
{
   // When we rotated the OSD we need to make sure it's not hanging off the
   // screen and adjust it to the maximum value if it is.
   WORD W_MaxOsdVpos = (WORD)gm_GetAdjusterMax(ADJV_OsdVPosition);
   WORD W_MaxOsdHpos = (WORD)gm_GetAdjusterMax(ADJV_OsdHPosition);

   // Set the values to their maximum values if they are out of range
   if (UserPrefOsdHPosition > W_MaxOsdHpos)
   {
      UserPrefOsdHPosition = W_MaxOsdHpos;
   }

   if (UserPrefOsdVPosition > W_MaxOsdVpos)
   {
      UserPrefOsdVPosition = W_MaxOsdVpos;
   }

   #if 0
   if ((UserPrefOsdRotation == OSD_ROTATED_180) ||
         (UserPrefOsdRotation == OSD_ROTATED_270))
   {
      //turn on OSD h and V flip to perform rotation of 180 degrees
      gm_OsdHFlipControl(TRUE);
      gm_OsdVFlipControl(TRUE);
   }
   else
   {
      //turn off OSD h and V flip
      gm_OsdHFlipControl(FALSE);
      gm_OsdVFlipControl(FALSE);
   }
   #else

   switch (UserPrefOsdRotation)
   {
      case OSD_NON_ROTATED:
if((UserPrefImageRotation==OSD_NON_ROTATED) || (UserPrefImageRotation==OSD_ROTATED_90))
{
         gm_OsdHFlipControl(FALSE);
         gm_OsdVFlipControl(FALSE);
}
else
{
         gm_OsdHFlipControl(FALSE);
         gm_OsdVFlipControl(TRUE);

}
         break;
      case OSD_ROTATED_90:
         gm_OsdHFlipControl(FALSE);
         gm_OsdVFlipControl(FALSE);
         break;
   //  case OSD_ROTATED_180:
   //      gm_OsdHFlipControl(FALSE);
   //      gm_OsdVFlipControl(TRUE);
   //     break;
      case OSD_ROTATED_270:
         gm_OsdHFlipControl(TRUE);
         gm_OsdVFlipControl(TRUE);
         break;
      default:
         break;
   }
	#endif

   gm_ForceUpdate(gmvb_MainChannel, INPUT_OUTPUT_CLOCK);
	gmvb_OsdRotation = UserPrefOsdRotation;
}
#endif

#ifdef Action_PostEvent_WB_Used
//******************************************************************************
//
// FUNCTION       : void PostEvent(void)
//
// USAGE          : Posts an OSD Event
//
// INPUT          : None
//
// OUTPUT         : none
//
// GLOBALS        : None
//
// USED_REGS      : None
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void PostEvent(void)
{
   gm_PostEvent();
}
#endif //#ifdef Action_Osd_Show_WB_Used

//*********************************  END  **************************************
