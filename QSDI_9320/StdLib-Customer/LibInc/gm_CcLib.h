/*
	$Workfile:   gm_CcLib.h  $
	$Revision: 1.2 $
	$Date: 2011/12/14 09:20:57 $
*/

#ifndef __GM_CCLIB_H__
	#define __GM_CCLIB_H__
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
// MODULE:      gm_CcLib.h
//
// USAGE:		Header file for the CC library
//
//******************************************************************************

//******************************************************************************
//  G L O B A L    D E F I N I T I O N S
//******************************************************************************
typedef enum
{
	CCD_Main_Process_Event_None,
	CCD_Main_Process_Event_Done,
	CCD_Main_Process_Event_Exit
} gmt_CCD_MAIN_PROCESS_EVENT;

//
// CC configuration
// The following definition defines the configuration type for CC service
//
typedef enum
{
   CC_SERVICE_OFF,            // CC display off
   CC_SERVICE_CC1,            // CC1
   CC_SERVICE_CC2,            // CC2
   CC_SERVICE_CC3,            // CC3
   CC_SERVICE_CC4,            // CC4
   CC_SERVICE_T1,             // Text1
   CC_SERVICE_T2,             // Text2
   CC_SERVICE_T3,             // Text3
   CC_SERVICE_T4,             // Text4
   CC_SERVICE_ON,             // CC display on
   CC_SERVICE_BLOCK,          // don't show CC data (used for channel block)
   CC_SERVICE_UNBLOCK,        // release blocking of CC data
   CC_SERVICE_SOURCE_CHG,     // TV channel or input source changed
   CC_SERVICE_BG_ENABLE,      // Enable different bg colors support
   CC_SERVICE_BG_DISABLE,     // Disable different bg colors support
   CC_SERVICE_FOCUS_1,        // Set CC focus to slicer on first decoder
                              // Note: this parameter is ONLY valid for chip
                              //       with two slicers and linked with CC lib
                              //       supports multiple instances only.
   CC_SERVICE_FOCUS_2,        // Set CC focus to slicer on second decoder
                              // Note: this parameter is ONLY valid for chip
                              //       with two slicers and linked with CC lib
                              //       supports multiple instances only.
	CC_SERVICE_SOURCE2_CHG,		// Signal to the CC Library that the TV channel
										// or input of the non-focus channel source
										// channged so that the VChip memory would be 
										// reset.  CC and XDS on focus channel not affected
                              // Note: this parameter is ONLY valid for chip
                              //       with two slicers and linked with CC lib
                              //       supports multiple instances only.									
   CC_SERVICE_MAX          
} gmt_CC_SERVICE;

// CC Freeze Mode
// The following definitions define the appreance of the CC when screen freeze
// is ON
//
typedef enum
{
   CC_FREEZE_MODE_OFF,        // Un-freeze CC
   CC_FREEZE_MODE_1,          // CC is stop during screen freeze
   CC_FREEZE_MODE_2,          // CC is continue during screen freeze
   CC_FREEZE_MODE_MAX
} gmt_CC_FREEZE_MODE;

//
// XDS - Content Advisory
// The following definitions define the return number for the
// program rating function.  The numbers are defined in such a way that
// the upper byte contains the Rating System (a3-a0) and the lower bytes
// contains the Age_Rating or MPA_Rating for the corresponding Rating System.
//
typedef  enum
{
#define SYSTEM_MPA      0x00
#define SYSTEM_MPA2     0x02
   RATE_MPA_NA = (SYSTEM_MPA<<8) + 0,        // 0x000
   RATE_MPA_G,                               // 0x001
   RATE_MPA_PG,                              // 0x002
   RATE_MPA_PG_13,                           // 0x003
   RATE_MPA_R,                               // 0x004
   RATE_MPA_NC_17,                           // 0x005
   RATE_MPA_X,                               // 0x006
   RATE_MPA_NOT_RATED,                       // 0x007
   
#define SYSTEM_USTV     0x01
   RATE_US_TV_NONE = (SYSTEM_USTV<<8) + 0,   // 0x100
   RATE_US_TV_Y,                             // 0x101
   RATE_US_TV_Y7,                            // 0x102
   RATE_US_TV_G,                             // 0x103
   RATE_US_TV_PG,                            // 0x104
   RATE_US_TV_14,                            // 0x105
   RATE_US_TV_MA,                            // 0x106
   RATE_US_TV_NONE_7,                        // 0x107

#define SYSTEM_CA_ENG   0x03                 
   RATE_CA_ENG_E = (SYSTEM_CA_ENG<<8) + 0,   // 0x300
   RATE_CA_ENG_C,                            // 0x301
   RATE_CA_ENG_C8_UP,                        // 0x302
   RATE_CA_ENG_G,                            // 0x303
   RATE_CA_ENG_PG,                           // 0x304
   RATE_CA_ENG_14_UP,                        // 0x305
   RATE_CA_ENG_18_UP,                        // 0x306
   RATE_CA_ENG_INVALID_PACKET,               // 0x307

#define SYSTEM_CA_FR    0x07
   RATE_CA_FR_E = (SYSTEM_CA_FR<<8) + 0,     // 0x700
   RATE_CA_FR_G,                             // 0x701
   RATE_CA_FR_8_UP,                          // 0x702
   RATE_CA_FR_13_UP,                         // 0x703
   RATE_CA_FR_16_UP,                         // 0x704   
   RATE_CA_FR_18_UP,                         // 0x705
   RATE_CA_FR_INVALID_PACKET,                // 0x706
   RATE_CA_FR_INVALID_PACKET1,               // 0x707
} gmt_CONTENT_ADVISORY;

//
// XDS configuration
// The following definition defines the Channel Information Class service
//
typedef enum
{
   CI_NETWORK_NAME,            // Channel Information Class - Network Name
   CI_CALL_LETTERS,            // Channel Information Class - Call Letters
   CI_CHANNEL_INFO_MAX          
} gmt_CHANNEL_INFO;

typedef enum
{
	CC_INTERLACED_DISPLAY_MODE		= 0,
	CC_PROGRESSIVE_DISPLAY_MODE		= 1
} gmt_CcDisplayMode;

/*
** the following macro is used to enable/disable the slicer for 
** VBI Line 21 for CC capture.
** 
*/
typedef enum
{
   CC_FIELD_NONE,             // both fields are disabled
   CC_FIELD_ODD,              // odd field is enabled
   CC_FIELD_EVEN,             // even field is enabled
   CC_FIELD_BOTH              // both fields are enabled
} gmt_CcFieldParity;   
      
//******************************************************************************
//  M A C R O   D E F I N I T I O N
//****************************************************************************** 
//
// XDS configuration
// The following definition defines the Channel Information Class service
//
#define CALL_LETTERS_LEN      4     // fixed length of call lettes string
#define CHANNEL_NBR_LEN       2     // fixed length of over-the-are channel number

//******************************************************************************
//  E X T E R N A L    V A R I A B L E S                                    
//******************************************************************************
extern BYTE gmv_CcDisplayMode;

//******************************************************************************
//  G L O B A L    F U N C T I O N    P R O T O T Y P E S                   
//******************************************************************************

//******************************************************************************
//  E X T E R N A L    F U N C T I O N    P R O T O T Y P E S                   
//******************************************************************************
BOOL CCD_Main_Process_Open(void);
BOOL CCD_Main_Process_Close(void);
gmt_CCD_MAIN_PROCESS_EVENT CCD_Main_Process(void);
BOOL CCD_Main_GetStatus(void);
void CC_PipService( unsigned char B_Field,
					unsigned char B_Data1,
                    unsigned char B_Data2 );


gmt_RET_STAT gm_CC_Config(gmt_CC_SERVICE service);

gmt_RET_STAT gm_ContentAdvisoryGet(gmt_CONTENT_ADVISORY *Wp_CA, BYTE *Bp_SubCode);
gmt_RET_STAT gm_ChannelInformationGet(gmt_CHANNEL_INFO type, BYTE *Bp_CI, BYTE MaxLen);
gmt_RET_STAT gm_ProgramIDNumberGet(BYTE *Bp_Info, BYTE B_MaxLen);
gmt_RET_STAT gm_LocalTimeZoneDSTGet(BYTE *Bp_Info, BYTE B_MaxLen);
gmt_RET_STAT gm_TimeOfDayGet(BYTE *Bp_Info, BYTE B_MaxLen);

void gm_CcSetDrawProp(BYTE ROM * ROM *Bp_CcFontTablePtr,
                          WORD W_CcTileId,
                          WORD W_CcCharacterWidth,
                          WORD W_CcCharacterHeight,
                          WORD W_CcCharacterSize,
                          WORD W_CcXposTileOffset,
                          WORD W_CcYposTileOffset);

gmt_RET_STAT gm_CcFreeze(gmt_CC_SERVICE service, gmt_CC_FREEZE_MODE mode);

BYTE gm_CcDataLostPeriodGet(void);
void gm_CcDataLostPeriodSet(BYTE duration);
BYTE gm_CcParityErrorPeriodGet(void);
BYTE gm_CcParityErrorThresholdGet(void);
void gm_CcParityErrorRateSet(BYTE threshold, BYTE duration);
BYTE gm_CcParityErrorCountGet(void);

/*
** Function to return CC data activite status from the CC decoder library
*/
gmt_RET_STAT gm_CcDataStatusGet(gmt_CC_SERVICE gmt_ChannelType);

/*
** Function to return Content Advisory packet in RAW format without parity bit.
** maximum 2 bytes
*/
gmt_RET_STAT gm_ContentAdvisoryRawGet(BYTE *Bp_CA, BYTE B_MaxLen);

/*
** Function to return Content Advisory packet in decoded format 
** Note: This function is for the non-focus channel in an application with
**       Fli8668 with dual VChip support
*/
gmt_RET_STAT gm_ContentAdvisoryGet2(gmt_CONTENT_ADVISORY *Wp_CA, BYTE *Bp_SubCode);

/*
** Function to return Content Advisory packet in RAW format without parity bit
** maximum 2 bytes
** Note: This function is for the non-focus channel in an application with
**       Fli8668 with dual VChip support
*/
gmt_RET_STAT gm_ContentAdvisoryRawGet(BYTE *Bp_CA, BYTE B_MaxLen);

#endif //#ifndef __GM_TTXLIB_H__
//*********************************  END  ************************************** 
