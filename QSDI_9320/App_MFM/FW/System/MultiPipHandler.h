/*
	$Workfile:   MultiPipHandler.h  $
	$Revision: 1.3 $
	$Date: 2011/12/15 03:12:03 $
*/

#ifndef __MULTIPIP_HANDLER_H__
#define __MULTIPIP_HANDLER_H__
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
// MODULE:      MultiPipHandler.h
//
// USAGE:       Header file for application Multipip Handler definition
//
//******************************************************************************

//******************************************************************************
//  G L O B A L    D E F I N I T I O N S
//******************************************************************************
typedef enum
{
   NON_MOVING_HIGHLIGHT,
   MOVING_HIGHLIGHT
};

typedef enum
{
   NO_ANCHOR,
   USE_ANCHOR
};

typedef enum MultiPipStates
{
   MULTIPIP_IDLE,
   MULTIPIP_SCANNING,
   MULTIPIP_NAVIGATING,
}MultiPipStates;

typedef enum
{
   MPIP_PAGES,			/*Reload Anchor Channel, and Scan up from that point on*/
   MPIP_SCAN,				/*No Anchor, always scanning up next valid channel*/
   MPIP_RELOAD,			/*Always reload what was last channel/source in pip window*/
   NUM_SCAN_MODES

} MultiPipScanMode;

#define MULTIPIP_MAX_ROWS			4
#define MULTIPIP_MAX_COLS  			4
#define MULTIPIP_MAX_WINDOWS  	16

#ifdef __PARADIGM__
#pragma pack(push)
#pragma pack(1)
#endif
typedef struct MultiPipStruct
{
   BYTE B_NumMultiPipRows; 		//Number of rows the panel is divided for multi pip
   BYTE B_NumMultiPipCols; 			//Number of columns the panel is divided for multi pip
   BYTE B_NumPips; 				//Number of pip windows enabled in the multi pip configuration
   WORD W_MultiPipMask;			 //This variable is bit indexed.
   /*
   If a bit is set, that particular pip window is enabled in the
   multi pip grid containing a maximum of 4 rows and 4 columns.
   Pip window indexing is from top left to bottom right.
   Thus the 0th bit (LSB) represents the top left pip (1st row and 4th column),
   1st bit represents the pip in 1st row and 2nd column and so on. The 15th bit
   thus represents the bottom right pip (4th row and 4th column)
   */
   WORD W_PipWidth; 				//The width of each pip window
   WORD W_PipHeight; 				//The Height of each pip window
   WORD W_MultiPipHStart; 			//The Horizontal start position in pixels for the multi pip configuration
   WORD W_MultiPipVStart; 			//The Vertical start position in pixels for the multi pip configuration
   WORD W_BorderWidth; 			//The width of the border around each pip window
   WORD W_BorderHeight; 			//The height of the border around each window
   WORD W_BorderColor; 			//The color of the border around each window (RGB565 format)
   WORD W_HighLightBorderWidth; 	//The width of the border for the highlight window
   WORD W_HighLightBorderHeight; 	//The height of the border for the highlight window
   WORD W_HighLightBorderColor; 	//The color of the border for the highlight window (RGB565 format)
   gmt_PIP_HIGHLIGHT_ORIGIN HighlightOrigin;
   //The origin of the highlight border. Either inside border window, or on top
   WORD W_MainWidth; 				//The width of the main window
   WORD W_MainHeight; 			//The height of the main window
   WORD W_MainHStart;  			//The Horizontal start position of the main window in pixels
   WORD W_MainVStart;  			//The Vertical start position of the main window in pixels
   BYTE Ba_ScanOrder[MULTIPIP_MAX_WINDOWS];
   BYTE B_ChPosition;				//The Position of MPip Channel Number (TOP_LEFT, BOTTOM_LEFT, TOP_RIGHT, BOTTOM_RIGHT)
   WORD W_XPadding;				//XOffset Pixel padding of channel number from respective edge
   WORD W_YPadding;				//YOffset Pixel padding of channel number from respective edge
   BYTE MovingHighlight;			//If Highlight window follows the active scanning window
   WORD ScanTimeout;				//Timeout using 10ms resolution timer value of each scan window
   BYTE AnchorPresent;				//If Scanning will stop at the anchor after one scan cycle
   WORD NavigateTimeout;			//Timeout using 10ms resolution timer value during navigation
} MultiPipStyleInfo;
#ifdef __PARADIGM__
#pragma pack(pop)
#endif
//******************************************************************************
//  E X T E R N     V A R I A B L E
//******************************************************************************

//******************************************************************************
//  G L O B A L    F U N C T I O N    P R O T O T Y P E S
//******************************************************************************

//application
BOOL IsMultiPipOn(void);
void MultiPipStartIdle(void);
void MultiPipHandler(void);

//mode handler
void MultiPipSetDisplay(gmt_PHY_CH B_Channel);
void MultiPipSetup(void);

//OSD & adjustor functions
void MultiPipIncWindow(void);
void MultiPipDecWindow(void);
void MultiPipPageUp(void);
void MultiPipPageDown(void);
void MultiPipChannelUp(void);
void MultiPipChannelDown(void);
void MultiPipSetScanMode(MultiPipScanMode ScanMode);
void MultiPipTurnOn(MultiPipStyleInfo ROM *MPipStyle);
void MultiPipTurnOff(void);
void MultiPipSetSourceInMain(void);

#endif	//#ifndef __MULTIPIP_HANDLER_H__
//*********************************  END  **************************************

