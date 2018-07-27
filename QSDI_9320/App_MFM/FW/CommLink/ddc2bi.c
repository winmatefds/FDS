#define __DDC2BI_COMM_C__

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
// MODULE:      Ddc2bi.c
//
// USAGE:       This module contains ddc2bi parser commands
//
//
//******************************************************************************
#include <string.h>
#include "System\all.h"
#include "StdLibInc\gm_Register.h"

//#define DEBUG_DDC2BI_VCP
#ifdef DEBUG_DDC2BI_VCP
#define msg_ddc gm_Printf
#define msg_ddc1 gm_Printf
#else
#define msg_ddc(a,b)
#define msg_ddc1(a,b,c)
#endif

#define DDC2BI_MAX_MSG_LENGTH    0x7F
//#define DDC2B_SRC_ADDRESS  0x51
//#define DDC2BI_DEVICE_ID 0x6E

//To make MCCS 2.0 Compliant
//Definitions related to VCP response
#define DispControllerType  0x0D // 13 for STMicro // 0x02    // 2 for Genesis
#define ApplnEnableKey      0xFFFF  //define a key value here
#define OSD_Enable          0x02    // 2 for enable OSD
#define OSD_Disable         0x01    // 2 for disable OSD
#define VCP_Version        0x0200   // version 2, Revision 0
#define DispTechnologyType  0x03    // 3 for TFT
#define NewControlVal       0x02    // 2 for a new control value has been saved
#define NoNewControlVal     0x01    // 1 for a NO new control value

extern PortSchemeDependentUserPrefType ROM SchemeFactoryDefaultInROM[NUM_IMAGE_SCHEMES]; // PSD

static BYTE VCPPage;
static VCPFeaturePktType VCPFeature;
static BYTE DdcciCmdParsed = FALSE;

#ifdef DDCCi_PROTOCOL_SUPPORT
//The I2C bus specification requires that a host be able to send and receive
//messages of any size up to the maximum of 131 bytes (127 + 4). But for DDC/CI,
//the maximum permitted length is 32 bytes of data to minimize the risk of bus
//contention and to keep the required buffer size in the display to a reasonable
//size/cost. Messages longer than 32 bytes must be transmitted as a number of 32
//byte fragments, the last fragment may be less than 32 bytes if message is not
//a multiple of 32 bytes.
#define DDCCI_MSG_SIZE 32

void Ddcci(void*, HANDLER_USE_t);


BYTE DDC2Bi_VCP_ParseCommand(BYTE SetVcp);
static WORD EndianConv(WORD W_Input);
#endif

static const far BYTE NakPkt[] = {3, 0x0B};

#ifdef USE_COLOR_FIDELITY
BYTE ROM strCapabilitiesRequest[] =
   "(type(LCD)(vcp(02 04 05 06 08 0E 10 12 14(05 08 0A 0B) 16 18 1A 1E 20 30 3E 6C 6E 70 AC AE B6 C6 C8 C9 CA(01 02) DF F0)mccs_ver(2.0)asset_eep(32)mswhql(1))";
#else
BYTE ROM strCapabilitiesRequest[] =
   "(vcp(02 04 05 06 08 0E 10 12 14(05 06 08 0B) 16 18 1A 1E 20 30 3E 60(01 02 03 04 ) 62 6C 6E 70 AC AE B6 C6 C8 C9 CA(01 02) DF)type(LCD)mccs_ver(2.0)asset_eep(32)mswhql(1))";
#endif

//******************************************************************************
// FUNCTION:   I2CSlaveInitAll
// USAGE:      Initialization of All I2C slave channels
// INPUT:      None
// OUTPUT:     None
// GLOBALS:
// USED_REGS:  None
//******************************************************************************
void far I2CSlaveInitAll(void)
{
   SLAVE_CH B_SlaveCh;

   //gm_ProgramLpmClkandPbus();
   gm_ClearAndSetBitsWord(CLOCK_CONFIG1, LPM_CLK_SEL, 1<<LPM_CLK_SEL_SHIFT); //LPM_CLK_SEL=OCM_CLK
   CsmI2cSlvRoute = LPM_I2CSLV_PINS_TO_SLAVE;
   gm_I2CSlaveInitAll(); 
   //Enable all i2c slave1 for LPM i2c master
   gm_WriteRegWord(I2C_SLAVE1_CTRL1, I2C_SLAVE1_EN);
   #ifdef LPM_UART_SUPPORT
   gm_ClearRegBitsByte(I2C_SLAVE4_CTRL1, I2C_SLAVE4_EN);
   #endif 

	for(B_SlaveCh=I2C_SLAVE2_CH; B_SlaveCh<=I2C_SLAVE3_CH ; B_SlaveCh++)
	{
   		gm_I2CSlaveInit((BYTE *)&I2CSlaveTable[B_SlaveCh], I2C_SLAVE_RD_LEN, B_SlaveCh);
   		gm_I2CSlaveAddCommHandler((I2C_SLAVE_HANDLER_TYPE)gm_I2CSlaveGProbeHandler, DDC2BI_ADDR_ID, B_SlaveCh);
   		gm_I2CSlaveAddI2CAddress(DDC2BI_ADDR_ID, B_SlaveCh); 	 
	} 

/*ChengLin 20130117: Remove DDC/CI Interrupt handle on 4K2K FE Athena*/
#ifdef ST_4K2K_93xx_BOARD 
   if(GetChipID_4K2K()==CHIPID_4K2K_FE)
   	gm_I2CSlaveRemoveI2CAddress(DDC2BI_ADDR_ID, I2C_SLAVE3_CH);
#endif

#if ((FEATURE_SOCIPC == ENABLE) && defined(ST_4K2K_93xx_BOARD))|| defined(FEATURE_HOSTIF) 
	for(B_SlaveCh=I2C_SLAVE1_CH; B_SlaveCh<I2C_SLAVE_NUM_CH ; B_SlaveCh++)
	{
	   if(GetChipID_4K2K()==CHIPID_4K2K_L)
	   {
		//Athena LEFT Registration
		gm_I2CSlaveAddCommHandler((I2C_SLAVE_HANDLER_TYPE)SoCIPC_SlavDevHandler, SOCIPC_DEVICEL_DST_ADDRESS, B_SlaveCh);
		gm_I2CSlaveAddI2CAddress(SOCIPC_DEVICEL_DST_ADDRESS, B_SlaveCh);
		}
		else if(GetChipID_4K2K()==CHIPID_4K2K_FE)
		{
		//Athena FE Registration
		gm_I2CSlaveAddCommHandler((I2C_SLAVE_HANDLER_TYPE)SoCIPC_SlavDevHandler, SOCIPC_DEVICEFE_DST_ADDRESS, B_SlaveCh);
		gm_I2CSlaveAddI2CAddress(SOCIPC_DEVICEFE_DST_ADDRESS, B_SlaveCh);	
		}
		#ifdef FEATURE_HOSTIF
		gm_I2CSlaveAddCommHandler((I2C_SLAVE_HANDLER_TYPE)SoCIPC_SlavDevHandler, HOSTIF_DEVICE_DST_ADDRESS, B_SlaveCh);
		gm_I2CSlaveAddI2CAddress(HOSTIF_DEVICE_DST_ADDRESS, B_SlaveCh);
		#endif      
	}
#endif
   
}

//******************************************************************************
// FUNCTION:   I2CSlaveChannelHandler
// USAGE:      Handler for specific I2C Slave channel 
// INPUT:      None
// OUTPUT:     None
// GLOBALS:
// USED_REGS:  None
//******************************************************************************
void far I2CSlaveChannelHandler(SLAVE_CH B_SlaveCh)
{
   PARSER_STATUS_t B_Status = NO_MSG_PARSED;
   I2C_SLAVE_TYPE far* Sp_I2CSlavePtr = Sp_I2CSlavePtrGlobal[B_SlaveCh];   

   gm_I2CSlaveSetChannel(B_SlaveCh);
   
   if(Sp_I2CSlavePtr == NULL_PTR)
   {
      gB_Ddc2Bi_I2C_Status[B_SlaveCh] = B_Status;
	  return;
   }

   B_Status = gm_I2CSlaveExecCommHandler(B_SlaveCh);
   if(B_Status == UNKNOWN_MSG_CMD)
   {
      BYTE B_whichHandler = B_SlaveCh + I2C_SLAVE1_USE;
      
      if((I2C_as_Gprobe_Cmd(B_SlaveCh) == gB_GprobeCmdSet) &&
         (I2C_ManufacturerID(B_SlaveCh) == gW_ManufacturerID))
   	{
         GprobeCmdParser(&Sp_I2CSlavePtr->Buffer[Start_of_GprobeCmd_in_DDC2BiMsg], B_whichHandler);
    }
	else
	{
#ifdef DDCCi_PROTOCOL_SUPPORT   
   #ifdef ST_4K2K_93xx_BOARD
      if(GetChipID_4K2K()!=CHIPID_4K2K_FE)
   #endif
         Ddcci((BYTE*)&Sp_I2CSlavePtr->Buffer, B_whichHandler);
      //gm_Print("DDCCI commands!",0);
#endif      
   }
   }

   if((B_Status == NO_MSG_PARSED) 
    /*|| (B_Status == UNKNOWN_MSG_CMD) && (B_ModeStatus == INTERNAL_ROM_MODE)*/)
   {
      gm_I2CSlaveDefaultHandler(B_SlaveCh);
   }

   gB_Ddc2Bi_I2C_Status[B_SlaveCh] = B_Status;

}

//******************************************************************************
// USAGE          : DDC2bi port commands parser. Gets the message and passes
//                  it on to the appropriate parser; Gprobe or DDC, based on
//                  the type of message received. Passes control to the Customer
//                  Protocol handler if that is being used.
//******************************************************************************
void Ddc2biCmdHandler(void)
{
#if 1
   I2CSlaveChannelHandler(I2C_SLAVE1_CH);
   I2CSlaveChannelHandler(I2C_SLAVE2_CH);
   I2CSlaveChannelHandler(I2C_SLAVE3_CH);
   I2CSlaveChannelHandler(I2C_SLAVE4_CH);
#else

   BYTE buf[DDC2BI_MAX_MSG_LENGTH+1];
   PARSER_STATUS_t status;

   gDebugCmdPtr = (BYTE far*)buf;
   switch (gm_DDC2BiRead(buf))
   {
      case READ_PKT_OK:
         if ((buf[1] == 0xC2) && (*(WORD*)(&buf[2]) == 0))
         {
#if (FEATURE_GPROBE_UART == REV2)
            if (gm_GprobeCmdParser(DDC2BI_USE, (BYTE far*)(&buf[4])) != UNKNOWN_MSG_CMD)
               return;
#endif
            GprobeCmdParser(&buf[4], DDC2BI_USE);
         }
         else // DDC2Ci command parser
         {
#ifdef DDCCi_PROTOCOL_SUPPORT
            Ddcci(buf, DDC2BI_USE);
#endif
         }
         break;

      case READ_PKT_WRONG_CS:
         gm_DDC2BiWriteCmdPkt((BYTE far*)NakPkt);
         return;
   }
#endif
}

//******************************************************************************
#ifdef DDCCi_PROTOCOL_SUPPORT

//******************************************************************************

//******************************************************************************
// USAGE          : Swaps the upper and the lower byte of the input Word
// INPUT          : W_Input
// OUTPUT         : The converted word
//******************************************************************************
static WORD EndianConv(WORD W_Input)
{
   return (W_Input >> 8) | (W_Input << 8);
}

#ifdef USE_COLOR_FIDELITY
#if 1
void DDC2Bi_TableRw_ParseCommand(void* gExtDebugCmdPtr, BYTE Rw_VCP)
{
   BYTE length;
   BYTE command;
   BYTE B_vcp_opcode;
   WORD value;
   BYTE B_mode;
   BYTE B_par;
   BYTE chk;

#if 0
   B_vcp_opcode = *(((BYTE*)gExtDebugCmdPtr)+2);
   value = *(((WORD*)gExtDebugCmdPtr)+3);
   B_mode = *(((BYTE*)gExtDebugCmdPtr)+5);
   B_par = *(((BYTE*)gExtDebugCmdPtr)+6);
#else
   B_vcp_opcode = (((DDC2Bi_WGS_VCPCmd_t*)gExtDebugCmdPtr)->vcp_opcode);
   value = ((((DDC2Bi_WGS_VCPCmd_t*)gExtDebugCmdPtr)->value_H) << 8) | (((DDC2Bi_WGS_VCPCmd_t*)gExtDebugCmdPtr)->value_L);
   B_mode = (((DDC2Bi_WGS_VCPCmd_t*)gExtDebugCmdPtr)->mode);
   B_par = (((DDC2Bi_WGS_VCPCmd_t*)gExtDebugCmdPtr)->par);
#endif


   // 20100209 #1, Added Color Fidelity
   Rw_VCP = Rw_VCP;

   switch (B_vcp_opcode)
   {
      case DDC2B_CMD_VCP_SimplifiedColorProfile:
         {
            gm_Print(" SimplifiedColorProfile ... 0x%x",B_mode);
            gm_Print("  Value = 0x%x",value);
            gm_Print("  Par = 0x%x",B_par);

            switch (B_mode)
            {
               case WGSM_QUERY_CAPABILITY:
                  GetWGSCapabilities((WGSCapStruct*)(&(((DDC2Bi_WGS_ReplyCmd_t*)gExtDebugCmdPtr)->status_H)));
                  ((DDC2Bi_WGS_ReplyCmd_t*)gExtDebugCmdPtr)->length    = 0x8D;    //
                  break;

               case WGSM_SWITCH_COLORMODE:
                  if (SetColorSpaceMode(B_par)==0x00)
                  {
                     //(((DDC2Bi_WGS_ReplyCmd_t*)gExtDebugCmdPtr)->status) = 0x0000;
                     (((DDC2Bi_WGS_ReplyCmd_t*)gExtDebugCmdPtr)->status_H) = 0x00;
                     (((DDC2Bi_WGS_ReplyCmd_t*)gExtDebugCmdPtr)->status_L) = 0x00;
                  }
                  else
                  {
                     //(((DDC2Bi_WGS_ReplyCmd_t*)gExtDebugCmdPtr)->status) = 0xFFFF;
                     (((DDC2Bi_WGS_ReplyCmd_t*)gExtDebugCmdPtr)->status_H) = 0xFF;
                     (((DDC2Bi_WGS_ReplyCmd_t*)gExtDebugCmdPtr)->status_L) = 0xFF;
                  }

                  ((DDC2Bi_WGS_ReplyCmd_t*)gExtDebugCmdPtr)->value_H = 0x00;
                  ((DDC2Bi_WGS_ReplyCmd_t*)gExtDebugCmdPtr)->value_L = 0x00;
                  ((DDC2Bi_WGS_ReplyCmd_t*)gExtDebugCmdPtr)->length    = 0x86;    //
                  break;

               case WGSM_UPDATE_COLORMODE:
                  {
                     //BYTE* ptr = &(B_par);
                     BYTE* ptr = &(((DDC2Bi_WGS_VCPCmd_t*)gExtDebugCmdPtr)->par);

                     gm_Print("  WGSM_UPDATE_COLORMODE ... %d",(*ptr));
                     //gm_Print("  1 ... 0x%x",*(ptr+1));
                     //gm_Print("  2 ... 0x%x",*(ptr+2));
                     //gm_Print("  3 ... 0x%x",*(ptr+3));
                     //gm_Print("  4 ... 0x%x",*(ptr+4));
                     //gm_Print("  5 ... 0x%x",*(ptr+5));
                     //gm_Print("  6 ... 0x%x",*(ptr+6));
                     //gm_Print("  7 ... 0x%x",*(ptr+7));
                     //gm_Print("  8 ... 0x%x",*(ptr+8));
                     if (*ptr==WGSM_CS_MODE_PROFILE && *(ptr+1)==WGSM_UPDATE_SRC_PROFILE) //update simplified color profile
                     {
                        BYTE i, size;

                        //to swap bytes
                        ptr = ptr + 2; //
                        for (i=0; i<sizeof(GamutDataStruct)+1; i=i+2)
                        {
                           size = *ptr;
                           *ptr = *(ptr+1);
                           *(ptr+1) = size;

                           ptr = ptr+2;
                        }

                        ptr = &(((DDC2Bi_WGS_VCPCmd_t*)gExtDebugCmdPtr)->par);
                        size = *(ptr+2);

                        if (size ==0x1C || size==0x2C) //right size, 28 or 44 bytes
                        {
                           if (UpdateSimplifiedColorProfile((GamutDataStruct*)(ptr+4), size-2, *(ptr+1)) == 0x00)
                           {
                              //DoGamutMapping(gamutDataAddr, 0);
                              {
                                 //(((DDC2Bi_WGS_ReplyCmd_t*)gExtDebugCmdPtr)->status) = 0x0000;
                                 (((DDC2Bi_WGS_ReplyCmd_t*)gExtDebugCmdPtr)->status_H) = 0x00;
                                 (((DDC2Bi_WGS_ReplyCmd_t*)gExtDebugCmdPtr)->status_L) = 0x00;
                              }
                           }
                           else
                           {
                              //(((DDC2Bi_WGS_ReplyCmd_t*)gExtDebugCmdPtr)->status) = 0xFFFF;
                              (((DDC2Bi_WGS_ReplyCmd_t*)gExtDebugCmdPtr)->status_H) = 0xFF;
                              (((DDC2Bi_WGS_ReplyCmd_t*)gExtDebugCmdPtr)->status_L) = 0xFF;
                           }
                        }

                     }
                     else
                     {
                        {
                           //(((DDC2Bi_WGS_ReplyCmd_t*)gExtDebugCmdPtr)->status) = 0xFFFF;
                           (((DDC2Bi_WGS_ReplyCmd_t*)gExtDebugCmdPtr)->status_H) = 0xFF;
                           (((DDC2Bi_WGS_ReplyCmd_t*)gExtDebugCmdPtr)->status_L) = 0xFF;
                        }
                     }

                     ((DDC2Bi_WGS_ReplyCmd_t*)gExtDebugCmdPtr)->value_H = 0x00;
                     ((DDC2Bi_WGS_ReplyCmd_t*)gExtDebugCmdPtr)->value_L = 0x00;
                     ((DDC2Bi_WGS_ReplyCmd_t*)gExtDebugCmdPtr)->length    = 0x86;    //
                     break;
                  }

               case WGSM_QUERY_COLORMODE:
               case WGSM_QUERY_NATIVEPROFILE:
               case WGSM_UPDATE_NATIVEPROFILE:
                  break;

               case WGSM_DISPLAY_COLOR:
                  {
                     //add code here for display pattern color
                     if (((DDC2Bi_WGS_ReplyCmd_t*)gExtDebugCmdPtr)->length==0x8C)
                     {
                     }

                     //enable / disable color processing
                     //if(txBuffer[LENGTH]==0x86 || txBuffer[LENGTH]==0x8C)
                     {
                        if (B_par==WGSM_DISPLAY_COLOR_DISABLE)
                           DisableColorProcessing();
                        else
                           EnableColorProcessing();
                     }

                     {
                        //(((DDC2Bi_WGS_ReplyCmd_t*)gExtDebugCmdPtr)->status) = 0x0000;
                        (((DDC2Bi_WGS_ReplyCmd_t*)gExtDebugCmdPtr)->status_H) = 0x00;
                        (((DDC2Bi_WGS_ReplyCmd_t*)gExtDebugCmdPtr)->status_L) = 0x00;
                     }
                     ((DDC2Bi_WGS_ReplyCmd_t*)gExtDebugCmdPtr)->value_H = 0x00;
                     ((DDC2Bi_WGS_ReplyCmd_t*)gExtDebugCmdPtr)->value_L = 0x00;
                     ((DDC2Bi_WGS_ReplyCmd_t*)gExtDebugCmdPtr)->length    = 0x86;    //
                     break;
                  }

               case WGSM_CALIBRATION_MODE:
                  break;

               default:
                  ;
            }
         }
         break;

      default:
         ;
   }

}
#endif
#endif // USE_COLOR_FIDELITY

//return the size of capabilities string for segment crossing case
WORD far GetCapabilitiesStringSize(void)
{
   return sizeof(strCapabilitiesRequest);
}

//******************************************************************************
// USAGE          : DDCCi command parser. Parses, Interprets and executes the
//                  DDCCi commands. It also prepares the response to be sent.
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        : gExtDebugCmdPtr: Points to the communication buffer.
//
// USED_REGS      : None
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
#pragma argsused
//#define DDC2BiCmdPtr gExtDebugCmdPtr
void Ddcci(void* gExtDebugCmdPtr, HANDLER_USE_t handler)
{
   BYTE readExpected = 1;
   BYTE EndCapStrFlag = 0; 
   BYTE *DDC2BiCmdPtr = gExtDebugCmdPtr;  
   WORD W_FillDataNum, W_FillZerosNum = 0;   

   /*ChengLin: 20120309: Add "AUX_C0_USE" and "AUX_C2_USE" for shift MCCS Data Buffer*/   
   if((handler == I2C_SLAVE1_USE)||(handler == I2C_SLAVE2_USE)||
      (handler == I2C_SLAVE3_USE)||(handler == I2C_SLAVE4_USE))
   {
      DDC2BiCmdPtr++;
   }
   else if((handler == AUX_C0_USE)||(handler == AUX_C2_USE))
   {
      DDC2BiCmdPtr++;
   }

   switch (*(((BYTE*)DDC2BiCmdPtr)+1))
   {
      case DDC2B_CMD_GetVCPFeature:
         VCPPage = 0; //select Page0
         VCPFeature.value_BigEndian = 0;
         VCPFeature.max_BigEndian = 0xFFFF;
         VCPFeature.vcp_opcode = (((DDC2Bi_GetVCPFeatureCmd_t*)DDC2BiCmdPtr)->vcp_opcode);
         (((DDC2Bi_GetVCPFeature_ReplyCmd_t*)gExtDebugCmdPtr)->length) = 0x08 | DDC2Bi_CONTROL_STATUS_FLAG;
         (((DDC2Bi_GetVCPFeature_ReplyCmd_t*)gExtDebugCmdPtr)->command) = DDC2B_CMD_GetVCPFeature_Reply;
         (((DDC2Bi_GetVCPFeature_ReplyCmd_t*)gExtDebugCmdPtr)->result) = DDC2Bi_VCP_ParseCommand(GET_VCP);
         (((DDC2Bi_GetVCPFeature_ReplyCmd_t*)gExtDebugCmdPtr)->a_VCPFeature) = VCPFeature;
         msg_ddc1("Get VCP opcode: %x, res: %x", VCPFeature.vcp_opcode,
                  (((DDC2Bi_GetVCPFeature_ReplyCmd_t*)gExtDebugCmdPtr)->result));
         break;

      case DDC2B_CMD_SetVCPFeature:
         VCPPage = 0; //select Page0
         VCPFeature.vcp_opcode   = (((DDC2Bi_SetVCPFeatureCmd_t*)DDC2BiCmdPtr)->vcp_opcode);
         VCPFeature.value_BigEndian    = (((DDC2Bi_SetVCPFeatureCmd_t*)DDC2BiCmdPtr)->val_BigEndian);
         DDC2Bi_VCP_ParseCommand(SET_VCP);
         *(BYTE far*)gExtDebugCmdPtr = 0x80;
         readExpected = 0;
         msg_ddc1("Set VCP opcode: %x, res: %x", VCPFeature.vcp_opcode, VCPFeature.value_BigEndian);
         break;

      case DDC2B_CMD_SaveCurrentSettings:
         msg_ddc("Save Current Settings",0);
         SaveModeIndependentSettings();
         *(BYTE far*)gExtDebugCmdPtr = 0x80; //no messages back
         readExpected = 0;
         break;

      case DDC2B_CMD_CapabilitiesRequest:
         {
            //capabilities reply thru I2C slave channel is moved to FastDdcciParser()
            //DO NOT handle here to avoid data collision!!
            if (gm_GetDdcciFastParserFlag())
               return;

            {
               //Note. Next code and data block is a template for Capabilities stings support.
               //Each CpReqStr entry must not exceed 32 bytes by DDCCI standard
               //#if USE_DYNAMIC_GAMUTMAPPING_SUPPORT
               WORD W_offset;
               WORD W_length;
               BYTE B_Cnt;
               W_offset = (((DdcciCapabilitiesRequest*)DDC2BiCmdPtr)->offset_BigEndian);
               (((DdcciCapabilitiesReply*)gExtDebugCmdPtr)->length) = VCP_GETCAPABILITIES_REPLYMSG_SIZE | DDC2Bi_CONTROL_STATUS_FLAG;
               (((DdcciCapabilitiesReply*)gExtDebugCmdPtr)->command) = DDC2B_CMD_CapabilitiesRequest_Reply;
               (((DdcciCapabilitiesReply*)gExtDebugCmdPtr)->offset_BigEndian) = W_offset;
               W_offset = EndianConv(W_offset);
               msg_ddc("Get Capabilities offset %d", W_offset);

               if (W_offset < sizeof(strCapabilitiesRequest))
               {
                  W_length = sizeof(strCapabilitiesRequest) - W_offset;

                  if (W_length >= DDCCI_MSG_SIZE)
                  {
                     W_length = DDCCI_MSG_SIZE + 3; //32 bytes cap str + 2 bytes offset + 1 byte checksum
                     W_FillDataNum = DDCCI_MSG_SIZE;                   
                     W_FillZerosNum = 0;                  
                  }
                  else
                  {
                     W_FillDataNum = W_length;
                     W_length += 2;    //add 2 bytes for offset  
                     W_FillZerosNum = DDCCI_MSG_SIZE - W_length; 
                     EndCapStrFlag = 1;
                  }
                     
                  _fmemcpy(&(((DdcciCapabilitiesReply*)gExtDebugCmdPtr)->data),
                           &strCapabilitiesRequest[W_offset], W_FillDataNum);        
               }
               else  //Fill null messages to buffer for handling extra data read
               {
                  BYTE B_SlaveCh = handler - I2C_SLAVE1_USE;
                  
                  for(B_Cnt=0; B_Cnt<DDCCI_MSG_SIZE; B_Cnt++)
                     *(&(((DdcciCapabilitiesReply*)gExtDebugCmdPtr)->data)+B_Cnt) = gm_I2CSlaveCreateNullMsg(0, DDC2BI_ADDR_ID, B_SlaveCh);
                
                  W_length = 3;
                  EndCapStrFlag = 2;              
               }
               
               ((DdcciCapabilitiesReply*)gExtDebugCmdPtr)->length = (BYTE)W_length | DDC2Bi_CONTROL_STATUS_FLAG;
               break; 
            }
         }
      case DDC2B_CMD_GetTimingReport:
         {
#define OutOfRangeBit		BIT7
#define UnstblCountBit		BIT6
#define HSyncPolarityBit	BIT1
#define VSyncPolarityBit	BIT0
            BYTE B_Status = 0;
            gmt_INPUT_FLAGS B_Flags = stInput[gmvb_MainChannel].ModeFlags;
            msg_ddc("Get Timing Report",0);

            if (B_Flags & gmd_MODE_OUT_RANGE ) //Out of range
               SetBit(B_Status, OutOfRangeBit);
            if (B_Flags & gmd_MODE_UNKNOWN)
               SetBit(B_Status, UnstblCountBit);
            if (!(B_Flags & gmd_MODE_NEG_HSYNC))
               SetBit(B_Status, HSyncPolarityBit);
            if (!(B_Flags & gmd_MODE_NEG_VSYNC))
               SetBit(B_Status, VSyncPolarityBit);
            (((DDC2Bi_GetTiming_ReplyCmd_t*)gExtDebugCmdPtr)->length) = 0x06;
            (((DDC2Bi_GetTiming_ReplyCmd_t*)gExtDebugCmdPtr)->command) = DDC2B_CMD_GetTimingReport_Reply;
            (((DDC2Bi_GetTiming_ReplyCmd_t*)gExtDebugCmdPtr)->status) = B_Status;
            (((DDC2Bi_GetTiming_ReplyCmd_t*)gExtDebugCmdPtr)->h_freq_BigEndian) = EndianConv((WORD)(((DWORD)stInput[gmvb_MainChannel].Hfrq *100) >> 8));
            (((DDC2Bi_GetTiming_ReplyCmd_t*)gExtDebugCmdPtr)->v_freq_BigEndian) = EndianConv(stInput[gmvb_MainChannel].Vfrq *10);
            break;
         }

      case DDC2B_CMD_NMV_EEPROM:
         msg_ddc("EEPROM",0);
         break;

      case DDC2B_CMD_NMV_GetVCPPage:
         msg_ddc("GetVCPPage",0);
         (((DDC2Bi_GetVCPPage_ReplyCmd_t*)gExtDebugCmdPtr)->length) = 0x03 |DDC2Bi_CONTROL_STATUS_FLAG;
         (((DDC2Bi_GetVCPPage_ReplyCmd_t*)gExtDebugCmdPtr)->command) = DDC2B_CMD_NMV_GetVCPPage_Reply;
         (((DDC2Bi_GetVCPPage_ReplyCmd_t*)gExtDebugCmdPtr)->vcp_page) = VCPPage;
         (((DDC2Bi_GetVCPPage_ReplyCmd_t*)gExtDebugCmdPtr)->vcp_maxpage) = DDC2B_Max_VCP_Pages;
         break;

      case DDC2B_CMD_NMV_GetPagedVCP: //NMV custom commands
         msg_ddc("GetPagedVCP",0);
         VCPPage = (((DDC2Bi_GetPagedVCPCmd_t*)DDC2BiCmdPtr)->vcp_page);
         VCPFeature.value_BigEndian = 0;
         VCPFeature.max_BigEndian = 0xFFFF;
         VCPFeature.vcp_opcode = (((DDC2Bi_GetPagedVCPCmd_t*)DDC2BiCmdPtr)->vcp_opcode);
         (((DDC2Bi_GetPagedVCP_ReplyCmd_t*)gExtDebugCmdPtr)->length) = 0x09 | DDC2Bi_CONTROL_STATUS_FLAG;
         (((DDC2Bi_GetPagedVCP_ReplyCmd_t*)gExtDebugCmdPtr)->command) = DDC2B_CMD_NMV_GetPagedVCP_Reply;
         (((DDC2Bi_GetPagedVCP_ReplyCmd_t*)gExtDebugCmdPtr)->result) = DDC2Bi_VCP_ParseCommand(GET_VCP);
         (((DDC2Bi_GetPagedVCP_ReplyCmd_t*)gExtDebugCmdPtr)->vcp_page) = VCPPage;
         (((DDC2Bi_GetPagedVCP_ReplyCmd_t*)gExtDebugCmdPtr)->b_VCPFeature)= VCPFeature;
         break;

      case DDC2B_CMD_NMV_SetPagedVCP:
         msg_ddc("SetPagedVCP",0);
         VCPPage = (((DDC2Bi_SetPagedVCPCmd_t*)DDC2BiCmdPtr)->vcp_page);
         VCPFeature.vcp_opcode = (((DDC2Bi_SetPagedVCPCmd_t*)DDC2BiCmdPtr)->vcp_opcode);
         VCPFeature.value_BigEndian    = (((DDC2Bi_SetPagedVCPCmd_t*)DDC2BiCmdPtr)->value_BigEndian);
         (((DDC2Bi_GetPagedVCP_ReplyCmd_t*)gExtDebugCmdPtr)->length) = 0x09 | DDC2Bi_CONTROL_STATUS_FLAG;
         (((DDC2Bi_GetPagedVCP_ReplyCmd_t*)gExtDebugCmdPtr)->command) = DDC2B_CMD_NMV_SetPagedVCP_Reply;
         (((DDC2Bi_GetPagedVCP_ReplyCmd_t*)gExtDebugCmdPtr)->result) = DDC2Bi_VCP_ParseCommand(SET_VCP);
         (((DDC2Bi_GetPagedVCP_ReplyCmd_t*)gExtDebugCmdPtr)->vcp_page) = VCPPage;
         (((DDC2Bi_GetPagedVCP_ReplyCmd_t*)gExtDebugCmdPtr)->b_VCPFeature) = VCPFeature;
         break;
#ifdef USE_COLOR_FIDELITY
#if USE_DYNAMIC_GAMUTMAPPING_SUPPORT
      case DDC2B_CMD_Table_Write:
         {
            BYTE cmd;
            //BYTE wgs_Mode = *(((BYTE*)DDC2BiCmdPtr)+5);

            cmd = ((((DDC2Bi_WGS_VCPCmd_t*)gExtDebugCmdPtr)->mode) & 0x1) || ((((DDC2Bi_WGS_VCPCmd_t*)gExtDebugCmdPtr)->mode)==WGSM_DISPLAY_COLOR);
            //cmd = ((wgs_Mode & 0x1) || (wgs_Mode == WGSM_DISPLAY_COLOR));

            gm_Print(">> Mode1 = 0x%x",(((DDC2Bi_WGS_VCPCmd_t*)gExtDebugCmdPtr)->mode));
            gm_Print(">> Mode2 = 0x%x",(((DDC2Bi_WGS_VCPCmd_t*)DDC2BiCmdPtr)->mode));

            DDC2Bi_TableRw_ParseCommand(gExtDebugCmdPtr, cmd);

            (((DDC2Bi_WGS_ReplyCmd_t*)DDC2BiCmdPtr)->command)  = DDC2B_CMD_Table_Reply;
            //*(((BYTE*)DDC2BiCmdPtr)+1) = DDC2B_CMD_Table_Reply;

            msg_ddc("TableWrite VCP",0);

            break;
         }
      case DDC2B_CMD_Table_Read:
         {
            msg_ddc("TableRead VCP",0);
            break;
         }
#endif
#endif // USE_COLOR_FIDELITY

      default:
         *(BYTE far*)gExtDebugCmdPtr = 0x80; //no messages back
         readExpected = 0; 
         msg_ddc("Unsupported DDCCI Command [%d]", *((BYTE*)gExtDebugCmdPtr+2));
   }
#if (FEATURE_DISPLAY_PORT == ENABLE)
   if (handler == AUX_C0_USE || handler == AUX_C2_USE)
   	{
	   /*ChengLin: 20120309: For Fix system shutdown when operate DP MCCS Set commands*/   	
      if(readExpected)
      {
      DpRxDdcciAuxSetReply((BYTE*)gExtDebugCmdPtr,handler-AUX_C0_USE);
      }
	  else
	  {
		pSW_Transaction_ModuleDataGlobal[handler-AUX_C0_USE]->W_CurCount = 0;  
	  }
   	}
   else
#endif //(FEATURE_DISPLAY_PORT == ENABLE)

#if 1
   if((handler == I2C_SLAVE1_USE)||(handler == I2C_SLAVE2_USE)||
      (handler == I2C_SLAVE3_USE)||(handler == I2C_SLAVE4_USE))
   {
      BYTE B_SlaveCh = handler - I2C_SLAVE1_USE;
      BYTE B_Length = (*(BYTE*)gExtDebugCmdPtr) & 0x7F;
      BYTE B_Checksum, i;
      
      B_Checksum = ((DDC2B_SRC_ADDRESS-1) ^ (Sp_I2CSlavePtrGlobal[B_SlaveCh]->B_DevID & I2C_ADDRESS_BITS));
      for(i = 0; i <= B_Length; i++)            // Process the whole packet 
          B_Checksum ^= ((BYTE far*)gExtDebugCmdPtr)[i];  // Update checksum 

      ((BYTE far*)gExtDebugCmdPtr)[B_Length+1] = B_Checksum;

      if(EndCapStrFlag == 1)  //fill zeros for WFIFO's spare space
      {
         BYTE B_Cnt;
         for(B_Cnt=0; B_Cnt<W_FillZerosNum+3; B_Cnt++)
            *(&(((DdcciCapabilitiesReply*)gExtDebugCmdPtr)->data)+W_FillDataNum+B_Cnt) = 0;
      }

      if(readExpected)
      {
         if(EndCapStrFlag)
         {
            gm_I2CSlaveSetReply(DDCCI_MSG_SIZE+6, B_SlaveCh);  //Last cap string fragment with zeros as ending
            EndCapStrFlag = 0;
         }
         else
         {
            gm_I2CSlaveSetReply(B_Length+3, B_SlaveCh);
         }
      }
      else
      {
         //Prepare null message for unsupported commands & unexpected read
         gm_I2CSlaveSetReply(B_Length+3, B_SlaveCh);
         Sp_I2CSlavePtrGlobal[B_SlaveCh]->W_RcvCount = 0;
         Sp_I2CSlavePtrGlobal[B_SlaveCh]->W_AbsRcvCount = 0;
      }
   }
#else
   if(handler == DDC2BI_USE)
      gm_DDC2BiWrite((BYTE*)gExtDebugCmdPtr+1, (*(BYTE*)gExtDebugCmdPtr) & 0x7F);
#endif
}


//******************************************************************************
//
// FUNCTION       : BYTE DDC2Bi_VCP_ParseCommand(BYTE SetVcp)
//
// USAGE          : VCP Commands Parser.
//
// INPUT          : SetVcp
//
// OUTPUT         : Returns 1 if feature is supported and 0 otherwise.
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
/*ChengLin 20130115: Removed 4k2k "DDC2Bi_VCP_ParseCommand()" for new Athena R behaviors*/
BYTE DDC2Bi_VCP_ParseCommand(BYTE SetVcp)
{
   WORD W_Temp;
   BYTE B_Temp;
   BOOL IsFeatureSupported = TRUE;

   //default values for Max and Current Values
   WORD W_VCPValue = 0;
   //WORD W_VCPMax = 0x100;
   WORD W_VCPMax = 0xFFFF;
   BYTE B_VCPType = 0x01;
   ModeDependentLookUpType LookUpType;

#ifdef ST_4K2K_93xx_BOARD
   BYTE DDC_Cmd_Buff[3]={0};
#endif

   //converts values to Little Endian
   if (SetVcp)
      W_VCPValue = EndianConv(VCPFeature.value_BigEndian);

   if (VCPPage == 0)
   {
      switch (VCPFeature.vcp_opcode)
      {
         case DDC2B_CMD_VCP_NewControlVal: //To make MCCS 2.0 Compliant
            {
               static BYTE B_DDCCINewControlVal;
               if (SetVcp == SET_VCP && W_VCPValue)
               {
                  W_VCPValue = NoNewControlVal;
                  B_DDCCINewControlVal = 0;
               }
               else if (SetVcp == GET_VCP)
               {  //Check for modeIndependent & mode dependent variables saved in NVRAM
                  W_VCPValue = B_DDCCINewControlVal ? NewControlVal: NoNewControlVal; // 2 - a new control value has been saved
                  msg_ddc("NewControlVal: %d", W_VCPValue);
               }
               break;
            }

         case DDC2B_CMD_VCP_MCCSVersion: //To make MCCS 2.0 Compliant
            if (SetVcp == GET_VCP)
            {
               W_VCPValue = VCP_Version; //Version 2, revision 0 of VESA Monitor control command set document
               msg_ddc("MCCSVersion: %d", W_VCPValue);
            }
            break;

         case DDC2B_CMD_VCP_HFreq: //To make MCCS 2.0 Compliant
            W_VCPValue = stInput[gmvb_MainChannel].Hfrq*100;
            W_VCPMax = (WORD)(((DWORD)stInput[gmvb_MainChannel].Hfrq*100) >> 16); // W_VCPMax is the high byte
            msg_ddc("DDC2B_CMD_VCP_HFreq %d", W_VCPValue);
            break;

         case DDC2B_CMD_VCP_VFreq:   //To make MCCS 2.0 Compliant
            W_VCPValue = stInput[gmvb_MainChannel].Vfrq*10;
            msg_ddc("DDC2B_CMD_VCP_VFreq %d", W_VCPValue);
            break;

         case DDC2B_CMD_VCP_MonitorType:
            W_VCPValue = DispTechnologyType; //To make MCCS 2.0 Compliant
            break;

         case DDC2B_CMD_VCP_DisplayUsageTime:
            {
               if (!SetVcp)
               {
                  extern DWORD DW_MonitorPowerOnTime;

                  W_VCPValue = (WORD)(DW_MonitorPowerOnTime/60); // A 2byte value, DW_MonitorPowerOnTime is in minute
               }
               break;
            }

         case DDC2B_CMD_VCP_ApplnEnKey: //To make MCCS 2.0 Compliant
            W_VCPValue = ApplnEnableKey; //A 2byte value
            break;

         case DDC2B_CMD_VCP_DispControllerType:
            //byte 1 = 2 for Genesis Controller, byte 2 = controller type (product_ID)
            W_VCPValue = DispControllerType | ((WORD)gm_ReadRegByte(PRODUCT_ID) << 8); // 
            W_VCPMax = gm_ReadRegWord(PRODUCT_ID) >> 8;
            //Byte 3 - provide a numeric indication of controller type (Rev ID) - ???
            break;

         case DDC2B_CMD_VCP_DispFirmwareLevel:
            W_VCPValue = 0x96; //A 2byte value - f/w level installed in the display
            break;

         case DDC2B_CMD_VCP_ALLReset:
            if (SetVcp && W_VCPValue)
            {
               BYTE B_Channel;
               
               msg_ddc( "Factory Reset", 0);
               //FactoryReset();
               UserPrefImageScheme = FactoryDefaultInROM.ImageScheme;
               UserPrefBrightness = SchemeFactoryDefaultInROM[UserPrefImageScheme].Brightness;
               UserPrefContrast = SchemeFactoryDefaultInROM[UserPrefImageScheme].Contrast;
               UserPrefUserRedGain = SchemeFactoryDefaultInROM[UserPrefImageScheme].UserRedGain;
               UserPrefUserGreenGain = SchemeFactoryDefaultInROM[UserPrefImageScheme].UserGreenGain;
               UserPrefUserBlueGain = SchemeFactoryDefaultInROM[UserPrefImageScheme].UserBlueGain;
               UserPrefUserRedOff = SchemeFactoryDefaultInROM[UserPrefImageScheme].UserRedOff;
               UserPrefUserGreenOff = SchemeFactoryDefaultInROM[UserPrefImageScheme].UserGreenOff;
               UserPrefUserBlueOff = SchemeFactoryDefaultInROM[UserPrefImageScheme].UserBlueOff;
               AdjustBrightness();
               AdjustContrast();
               AdjustRedGain();
               AdjustBlueGain();
               AdjustGreenGain();
               AdjustRedOff();
               AdjustBlueOff();
               AdjustGreenOff();

            #if FEATURE_AIP == ENABLE
               if (gm_IsInputTripleADC(gmvb_MainChannel))
                  B_Channel = gmvb_MainChannel;
               else if (gm_IsInputTripleADC(gmvb_PipChannel))
                  B_Channel = gmvb_PipChannel;
               else
                  break;
               
               UserPrefHTotal = gmvw_InputHTotalOrg[B_Channel];
               UserPrefHStart = gmvw_InputHStartOrg[B_Channel];
               UserPrefHSyncDelay = gmvsw_InputHSyncOffset[B_Channel];
               UserPrefVStart = gmvw_InputVStartOrg[B_Channel];
               gmvsw_InputHSyncOffset[B_Channel] = 0;
               gmvsw_InputVSyncOffset[B_Channel] = 0;
               UserPrefVSyncDelay = gmvsw_InputVSyncOffset[B_Channel];
               UserPrefHSyncPhase = gmvb_InputSamplingPhase[B_Channel];
               
               gm_InputSetHPosition(B_Channel);
               gm_InputSetVPosition(B_Channel);
               SetupHTotal(UserPrefHTotal);
               gm_AutoGeometry(B_Channel, gmd_DEFAULT_AUTO);
            #endif
            
            #ifdef USE_COLOR_FIDELITY
             #if USE_DYNAMIC_GAMUTMAPPING_SUPPORT
               DisableColorProcessing();
             #endif
            #endif // USE_COLOR_FIDELITY

               /*ChengLin 20130117: Move to here for Sync DDC/CI commands for Athena Right/Left*/
               #ifdef ST_4K2K_93xx_BOARD
               if(GetChipID_4K2K()==CHIPID_4K2K_R)
               {
         			//Send SocIPC DDC/CI Command to Athena L
         			DDC_Cmd_Buff[0] = DDC2B_CMD_VCP_ALLReset;
         			DDC_Cmd_Buff[1] = 0;
         			DDC_Cmd_Buff[2] = 0;
         								
         			if(SocIPCSendCmdPacketParser(SOCIPC_DEVICEL_DST_ADDRESS, SICMD_RW_DDCI_CONTRL, DDC_Cmd_Buff) == FALSE)
         			{
         				msg_ddc("\n<Soc IPC Error> Device Left DDC Send Reset Command Failure.\n",0);
            		}
               }				
               #endif

			
            }
            else
            {
               W_VCPMax = 1;
               B_VCPType = 1;		   
            }
            break;

#ifdef ADJUSTER_BRIGHTNESS_DECLARED
         case DDC2B_CMD_VCP_RestoreContrastBrightness:
            {
               if (SetVcp && W_VCPValue)
               {
                  #ifdef ST_4K2K_93xx_BOARD
                  if(GetChipID_4K2K()==CHIPID_4K2K_R)
                  {
         				//Send SocIPC DDC/CI Command to Athena L
         				DDC_Cmd_Buff[0] = DDC2B_CMD_VCP_RestoreContrastBrightness;
         				DDC_Cmd_Buff[1] = 0;
         				DDC_Cmd_Buff[2] = 0;

         				if(SocIPCSendCmdPacketParser(SOCIPC_DEVICEL_DST_ADDRESS, SICMD_RW_DDCI_CONTRL, DDC_Cmd_Buff) == FALSE)
         				{
         					msg_ddc("\n<Soc IPC Error> Device Left DDC Send Restore Contrast Brightness Command Failure.\n",0);
       					}
   					}
                  #endif				
                  UserPrefBrightness = SchemeFactoryDefaultInROM[UserPrefImageScheme].Brightness;
                  UserPrefContrast = SchemeFactoryDefaultInROM[UserPrefImageScheme].Contrast;
                  AdjustBrightness();
                  AdjustContrast();	  
               }
               else
               {	   
                  W_VCPMax = 1;
               }
               break;
            }
#endif

#ifdef ADJUSTER_USERREDGAIN_DECLARED
         case DDC2B_CMD_VCP_ColorReset:
            {
               if (SetVcp && W_VCPValue)
               {
                  UserPrefUserRedGain = SchemeFactoryDefaultInROM[UserPrefImageScheme].UserRedGain;
                  UserPrefUserGreenGain = SchemeFactoryDefaultInROM[UserPrefImageScheme].UserGreenGain;
                  UserPrefUserBlueGain = SchemeFactoryDefaultInROM[UserPrefImageScheme].UserBlueGain;
                  UserPrefUserRedOff = SchemeFactoryDefaultInROM[UserPrefImageScheme].UserRedOff;
                  UserPrefUserGreenOff = SchemeFactoryDefaultInROM[UserPrefImageScheme].UserGreenOff;
                  UserPrefUserBlueOff = SchemeFactoryDefaultInROM[UserPrefImageScheme].UserBlueOff;
                  AdjustRedGain();
                  AdjustBlueGain();
                  AdjustGreenGain();
                  AdjustRedOff();
                  AdjustBlueOff();
                  AdjustGreenOff();
				  
                  /*ChengLin 20130117: Move to here for Sync DDC/CI commands for Athena Right/Left*/
                  #ifdef ST_4K2K_93xx_BOARD
                  if(GetChipID_4K2K()==CHIPID_4K2K_R)
                  {
         				//Send SocIPC DDC/CI Command to Athena L
         				DDC_Cmd_Buff[0] = DDC2B_CMD_VCP_ColorReset;
         				DDC_Cmd_Buff[1] = 0;
         				DDC_Cmd_Buff[2] = 0;
         								  
         				if(SocIPCSendCmdPacketParser(SOCIPC_DEVICEL_DST_ADDRESS, SICMD_RW_DDCI_CONTRL, DDC_Cmd_Buff) == FALSE)
         				{
         					msg_ddc("\n<Soc IPC Error> Device left DDC Send Color Reset Command Failure.\n",0);
         				}
   					}
					   #endif
               }
               else
               {
                  W_VCPMax = 1;
               }
               break;
            }
#endif

#if FEATURE_AIP == ENABLE
         case DDC2B_CMD_VCP_GeometryReset:
            {            
               if (SetVcp && W_VCPValue)
               {          
                  BYTE B_Channel;
                  if (gm_IsInputTripleADC(gmvb_MainChannel))
                     B_Channel = gmvb_MainChannel;
                  else if (gm_IsInputTripleADC(gmvb_PipChannel))
                     B_Channel = gmvb_PipChannel;
                  else
                     break;
	                    
                  UserPrefHTotal = gmvw_InputHTotalOrg[B_Channel];
                  UserPrefHSyncPhase = gmvb_InputSamplingPhase[B_Channel];
                  UserPrefHStart = gmvw_InputHStartOrg[B_Channel];
                  UserPrefVStart = gmvw_InputVStartOrg[B_Channel];
                  gmvsw_InputHSyncOffset[B_Channel] = 0;
                  gmvsw_InputVSyncOffset[B_Channel] = 0;
                  UserPrefHSyncDelay = gmvsw_InputHSyncOffset[B_Channel];
                  UserPrefVSyncDelay = gmvsw_InputVSyncOffset[B_Channel];
                  
                  gm_InputSetHPosition(B_Channel);
                  gm_InputSetVPosition(B_Channel);
                  SetupHTotal(UserPrefHTotal);
                  gm_AutoGeometry(B_Channel, gmd_DEFAULT_AUTO);			  
               }
               else
               {
                  W_VCPMax = 1;
               }
               break;
            }
#endif

#ifdef ADJUSTER_BRIGHTNESS_DECLARED
         case DDC2B_CMD_VCP_Brightness:
            {
               if (SetVcp)
               {
                  UserPrefBrightness = (W_VCPValue*2) + (WORD)gm_GetAdjusterMin(ADJV_Brightness);
                  msg_ddc("calling AdjustBrightness [%d]", UserPrefBrightness);
                  AdjustBrightness();

               /*ChengLin 20130117: Move to here for Sync DDC/CI commands for Athena Right/Left*/
               #ifdef ST_4K2K_93xx_BOARD
                  if(GetChipID_4K2K()==CHIPID_4K2K_R)
                  {
         				//Send SocIPC DDC/CI Command to Athena L
         				DDC_Cmd_Buff[0] = DDC2B_CMD_VCP_Brightness;
         				DDC_Cmd_Buff[1] = (BYTE)(W_VCPValue>>8);
         				DDC_Cmd_Buff[2] = (BYTE)(W_VCPValue & 0x00FF);
         									 
         				if(SocIPCSendCmdPacketParser(SOCIPC_DEVICEL_DST_ADDRESS, SICMD_RW_DDCI_CONTRL, DDC_Cmd_Buff) == FALSE)
         				{
         					msg_ddc("\n<Soc IPC Error> Device left DDC Send Brightness Command Failure.\n",0);
        					}
   					}
   				#endif	

				  
               }
               else
               {
                  W_VCPValue = (UserPrefBrightness - (WORD)gm_GetAdjusterMin(ADJV_Brightness))/2;
                  W_VCPMax = ((WORD)gm_GetAdjusterMax(ADJV_Brightness) - (WORD)gm_GetAdjusterMin(ADJV_Brightness))/2;
                  B_VCPType = 1;		   
               }
               break;
            }
#endif

#ifdef ADJUSTER_RBLACKLEVEL_DECLARED
         case DDC2B_CMD_VCP_Red_Video_Black_Level:
            {
            }
#endif
#ifdef ADJUSTER_GBLACKLEVEL_DECLARED
         case DDC2B_CMD_VCP_Green_Video_Black_Level:
            {
            }
#endif
#ifdef ADJUSTER_BBLACKLEVEL_DECLARED
         case DDC2B_CMD_VCP_Blue_Video_Black_Level:
            {
            }
#endif
#ifdef ADJUSTER_CONTRAST_DECLARED
         case DDC2B_CMD_VCP_Contrast:
            {
               if (SetVcp)
               {            
                  //no need to conv to LittleEndian because
                  //we need to do word to byte conversion anyways	  
                  UserPrefContrast = (W_VCPValue*2) + (WORD)gm_GetAdjusterMin(ADJV_Contrast);
                  msg_ddc("calling adjustcontrast [%d]", UserPrefContrast);
                  AdjustContrast();

                  /*ChengLin 20130117: Move to here for Sync DDC/CI commands for Athena Right/Left*/				  
                  #ifdef ST_4K2K_93xx_BOARD
                  if(GetChipID_4K2K()==CHIPID_4K2K_R)
                  {
         				//Send SocIPC DDC/CI Command to Athena L
         				DDC_Cmd_Buff[0] = DDC2B_CMD_VCP_Contrast;
         				DDC_Cmd_Buff[1] = (BYTE)(W_VCPValue>>8);
         				DDC_Cmd_Buff[2] = (BYTE)(W_VCPValue & 0x00FF);
         														
         				if(SocIPCSendCmdPacketParser(SOCIPC_DEVICEL_DST_ADDRESS, SICMD_RW_DDCI_CONTRL, DDC_Cmd_Buff) == FALSE)
         				{
         					msg_ddc("\n<Soc IPC Error> Device left DDC Send Contrast Command Failure.\n",0);
         				}
				      }
				      #endif
               }
               else
               {	               
                  W_VCPValue = (UserPrefContrast - (WORD)gm_GetAdjusterMin(ADJV_Contrast))/2;
                  W_VCPMax = ((WORD)gm_GetAdjusterMax(ADJV_Contrast) - (WORD)gm_GetAdjusterMin(ADJV_Contrast))/2;
                  B_VCPType = 0;			  
               }
               break;
            }
#endif
#ifdef UserPrefColorTemp
         case DDC2B_CMD_VCP_SelectColorPreset:
            {
               if (SetVcp)
               {
                  switch (W_VCPValue)
                  {
                     case DDCCI_CTEMP_6500K    :
                        UserPrefColorTemp= CTEMP_6500K;
                        break;
                     case DDCCI_CTEMP_7500K    :
                        UserPrefColorTemp= CTEMP_7500K;
                        break;
                     case DDCCI_CTEMP_9300K        :
                        UserPrefColorTemp = CTEMP_9300K;
                        break;
                     //case DDCCI_CTEMP_11500K    :
                     //   UserPrefColorTemp = CTEMP_11500K;
                     //   break;
                     case DDCCI_CTEMP_USER1     :
                        UserPrefColorTemp = USER_TEMP;
                        break;
                  }

                  msg_ddc("calling adjustcolor_temp [%d]", UserPrefColorTemp);
                  AdjustColorTemp();

                  /*ChengLin 20130117: Move to here for Sync DDC/CI commands for Athena Right/Left*/  
                  #ifdef ST_4K2K_93xx_BOARD
                  if(GetChipID_4K2K()==CHIPID_4K2K_R)
                  {
         				//Send SocIPC DDC/CI Command to Athena L 		 
         				DDC_Cmd_Buff[0] = DDC2B_CMD_VCP_SelectColorPreset;
         				DDC_Cmd_Buff[1] = (BYTE)(W_VCPValue>>8);
         				DDC_Cmd_Buff[2] = (BYTE)(W_VCPValue & 0x00FF);
         																	 
         				if(SocIPCSendCmdPacketParser(SOCIPC_DEVICEL_DST_ADDRESS, SICMD_RW_DDCI_CONTRL, DDC_Cmd_Buff) == FALSE)
         				{
         					msg_ddc("\n<Soc IPC Error> Device left DDC Send Color Preset Command Failure.\n",0);
        					}
     					}
     					#endif
               }
               else
               { 
                  switch (UserPrefColorTemp)
                  {
                     case CTEMP_6500K    :
                        W_VCPValue = DDCCI_CTEMP_6500K;
                        break;

                     case CTEMP_7500K    :
                        W_VCPValue = DDCCI_CTEMP_7500K;
                        break;

                     case CTEMP_9300K    :
                        W_VCPValue = DDCCI_CTEMP_9300K;
                        break;

                     //case CTEMP_11500K    :
                     //   W_VCPValue = DDCCI_CTEMP_11500K;
                     //   break;
                        /*case USER_TEMP     :
                            W_VCPValue = DDCCI_CTEMP_USER1;
                            UserPrefColorTemp
                            break;*/
                        //The below default task was added to pass Windows DTM
                        //Without it, DTM tries to set UserPrefColorTemp to 0
                        //As well the above case was commented out to pass DTM
                        //This is a known Microsoft bug
                     default    :
                        W_VCPValue = DDCCI_CTEMP_6500K;
                        break;

                  }
                  B_VCPType    = 0x01;    //Set Parameter
#if 1
                  W_VCPMax = 0xFFFF;    //this is the number of possible choices for this non-continuous VCP value
#else
                  W_VCPMax = 3;    //this is the number of possible choices for this non-continuous VCP value
#endif
               }

    
               break;
            }
#endif
#ifdef Action_HideOsd_WB_Used
         case DDC2B_CMD_VCP_OSD:
            {
               if (SetVcp)
               {
                  if (W_VCPValue == 1) // hide osd if it's on
                  {
                     gm_OsdHide();
                  }
                  else if (W_VCPValue == 2) //show osd
                  {
                     gm_OsdShow();
                  }

                  /*ChengLin 20130117: Move to here for Sync DDC/CI commands for Athena Right/Left*/ 
                  #ifdef ST_4K2K_93xx_BOARD
                  if(GetChipID_4K2K()==CHIPID_4K2K_R)
                  {
            			//Send SocIPC DDC/CI Command to Athena L		 
            			DDC_Cmd_Buff[0] = DDC2B_CMD_VCP_OSD;
            			DDC_Cmd_Buff[1] = (BYTE)(W_VCPValue>>8);
            			DDC_Cmd_Buff[2] = (BYTE)(W_VCPValue & 0x00FF);
            																	 
            			if(SocIPCSendCmdPacketParser(SOCIPC_DEVICEL_DST_ADDRESS, SICMD_RW_DDCI_CONTRL, DDC_Cmd_Buff) == FALSE)
            			{
            				msg_ddc("\n<Soc IPC Error> Device left DDC Send OSD Command Failure.\n",0);
          				}
       				}
       				#endif
               }
               else
               {                
                  B_VCPType = 0;
                  W_VCPMax = 2;			  
               }
               break;
            }
#endif

#if FEATURE_AIP == ENABLE 

#ifdef ADJUSTER_HSTART_DECLARED
         case DDC2B_CMD_VCP_HPosition:
            {            
               if (SetVcp)
               {               
                  msg_ddc("Set H Position [%d]", W_VCPValue);
                  UserPrefHSyncDelay = MaxHStart() - W_VCPValue;
                  AdjustHStart();			  
               }
               else
               {
                  W_VCPValue = MaxHStart() - UserPrefHSyncDelay;
                  W_VCPMax = MaxHStart() -MinHStart();
               }
               break;
            }
#endif

#ifdef ADJUSTER_VSTART_DECLARED
         case DDC2B_CMD_VCP_VPosition:
            {
               if (SetVcp)
               {             
                  msg_ddc("Set V Position [%d]", W_VCPValue);
                  UserPrefVSyncDelay = W_VCPValue + MinVStart();
                  AdjustVStart();			  
               }
               else
               {
                  W_VCPValue = UserPrefVSyncDelay - MinVStart();
                  W_VCPMax = MaxVStart() - MinVStart();
               }
               break;
            }
#endif

#ifdef Action_AdjustHTotal_WB_Used
         case DDC2B_CMD_VCP_Clock:
            {
               if (SetVcp)
               {               
                  UserPrefHTotal = W_VCPValue + MinHTotal();
                  AdjustHTotal();			  
               }
               else
               {
                  W_VCPValue = UserPrefHTotal - MinHTotal();
                  W_VCPMax = MaxHTotal() - MinHTotal();
               }
               break;
            }
#endif

#ifdef Action_AdjustHSyncPhase_WB_Used
         case DDC2B_CMD_VCP_ClockPhase:
            {
               if (SetVcp)
               {              
                  UserPrefHSyncPhase = W_VCPValue;
                  AdjustHSyncPhase();			  
               }
               else
               {
                  W_VCPValue = UserPrefHSyncPhase;
                  W_VCPMax = 0x3F;
               }
               break;
            }
#endif
#endif // FEATURE_AIP == ENABLE 

#ifdef ADJUSTER_USERREDGAIN_DECLARED
         case DDC2B_CMD_VCP_RedGain:

		{	
               if (SetVcp)
               {
                  UserPrefAcm3RedLuma = W_VCPValue + (WORD)gm_GetAdjusterMin(ADJV_Acm3RedLuma);
                  AdjustAcmRedLuma();

            
                  #ifdef ST_4K2K_93xx_BOARD
                  if(GetChipID_4K2K()==CHIPID_4K2K_R)
                  {
         				//Send SocIPC DDC/CI Command to Athena L							  
         				DDC_Cmd_Buff[0] = DDC2B_CMD_VCP_RedGain;
         				DDC_Cmd_Buff[1] = (BYTE)(W_VCPValue>>8);
         				DDC_Cmd_Buff[2] = (BYTE)(W_VCPValue & 0x00FF);
         																					  
         				if(SocIPCSendCmdPacketParser(SOCIPC_DEVICEL_DST_ADDRESS, SICMD_RW_DDCI_CONTRL, DDC_Cmd_Buff) == FALSE)
         				{
         					msg_ddc("\n<Soc IPC Error> Device left DDC Send Red Gain Command Failure.\n",0);
        					}
    					}
                  #endif	   	  
               }
               else
               {               
                  W_VCPValue = UserPrefAcm3RedLuma - (WORD)gm_GetAdjusterMin(ADJV_Acm3RedLuma);
                  W_VCPMax = (WORD)gm_GetAdjusterMax(ADJV_Acm3RedLuma) - (WORD)gm_GetAdjusterMin(ADJV_Acm3RedLuma);
                  B_VCPType = 0;			  
               }
               break;
            }
	
#endif

#ifdef ADJUSTER_USERGREENGAIN_DECLARED
         case DDC2B_CMD_VCP_GreenGain:	 	
            {
               if (SetVcp)
               {
                  UserPrefAcm3RedLuma = W_VCPValue + (WORD)gm_GetAdjusterMin(ADJV_Acm3GreenLuma);
                  AdjustAcmGreenLuma();


                  #ifdef ST_4K2K_93xx_BOARD
                  if(GetChipID_4K2K()==CHIPID_4K2K_R)
                  {
         				//Send SocIPC DDC/CI Command to Athena L 									   
         				DDC_Cmd_Buff[0] = DDC2B_CMD_VCP_GreenGain;
         				DDC_Cmd_Buff[1] = (BYTE)(W_VCPValue>>8);
         				DDC_Cmd_Buff[2] = (BYTE)(W_VCPValue & 0x00FF);
         							   
         				if(SocIPCSendCmdPacketParser(SOCIPC_DEVICEL_DST_ADDRESS, SICMD_RW_DDCI_CONTRL, DDC_Cmd_Buff) == FALSE)
         				{
         					msg_ddc("\n<Soc IPC Error> Device left DDC Send Green Gain Command Failure.\n",0);
        					}
     					}
                  #endif				
               }
               else
               {               
                  W_VCPValue = UserPrefAcm3GreenLuma - (WORD)gm_GetAdjusterMin(ADJV_Acm3GreenLuma);
                  W_VCPMax = (WORD)gm_GetAdjusterMax(ADJV_Acm3GreenLuma) - (WORD)gm_GetAdjusterMin(ADJV_Acm3GreenLuma);
                  B_VCPType = 0;			  
               }
               break;
            }
          
#endif

#ifdef ADJUSTER_USERBLUEGAIN_DECLARED
         case DDC2B_CMD_VCP_BlueGain: 	
            {
               if (SetVcp)
               {
                  UserPrefAcm3BlueLuma = W_VCPValue + (WORD)gm_GetAdjusterMin(ADJV_Acm3BlueLuma);
                  AdjustAcmBlueLuma();

                  #ifdef ST_4K2K_93xx_BOARD
                  if(GetChipID_4K2K()==CHIPID_4K2K_R)
                  {
         				//Send SocIPC DDC/CI Command to Athena L 														   
         				DDC_Cmd_Buff[0] = DDC2B_CMD_VCP_BlueGain;
         				DDC_Cmd_Buff[1] = (BYTE)(W_VCPValue>>8);
         				DDC_Cmd_Buff[2] = (BYTE)(W_VCPValue & 0x00FF);
         																											   
         				if(SocIPCSendCmdPacketParser(SOCIPC_DEVICEL_DST_ADDRESS, SICMD_RW_DDCI_CONTRL, DDC_Cmd_Buff) == FALSE)
         				{
         					msg_ddc("\n<Soc IPC Error> Device left DDC Send Blue Gain Command Failure.\n",0);
        					}
     					}
                  #endif				
               }
               else
               {	               
                  W_VCPValue = UserPrefAcm3BlueLuma - (WORD)gm_GetAdjusterMin(ADJV_Acm3BlueLuma);
                  W_VCPMax = (WORD)gm_GetAdjusterMax(ADJV_Acm3BlueLuma) - (WORD)gm_GetAdjusterMin(ADJV_Acm3BlueLuma);
                  B_VCPType = 0;			  
               }
               break;
            }

#endif

#if 0//def ADJUSTER_SHARPNESS_DECLARED
         case DDC2B_CMD_VCP_TVSharpness:
            {
               if (SetVcp)
               {
                  UserPrefSharpness= W_VCPValue;
                  AdjustSharpness();

                  /*ChengLin 20130117: Move to here for Sync DDC/CI commands for Athena Right/Left*/ 
                  #ifdef ST_4K2K_93xx_BOARD
                  if(GetChipID_4K2K()==CHIPID_4K2K_R)
                  {
         				//Send SocIPC DDC/CI Command to Athena L																  
         				DDC_Cmd_Buff[0] = DDC2B_CMD_VCP_TVSharpness;
         				DDC_Cmd_Buff[1] = (BYTE)(W_VCPValue>>8);
         				DDC_Cmd_Buff[2] = (BYTE)(W_VCPValue & 0x00FF);
         																																	  
         				if(SocIPCSendCmdPacketParser(SOCIPC_DEVICEL_DST_ADDRESS, SICMD_RW_DDCI_CONTRL, DDC_Cmd_Buff) == FALSE)
         				{
         					msg_ddc("\n<Soc IPC Error> Device left DDC Send Sharpness Command Failure.\n",0);
        					}
     					}
                  #endif  
               }
               else
               {	   
                  W_VCPValue = UserPrefSharpness;
               }
               break;
            }
#endif

#if 0//def ADJUSTER_SATURATION_DECLARED
         case DDC2B_CMD_VCP_TVSaturation:
            {
               if (SetVcp)
               {  
                  UserPrefSaturation = W_VCPValue;
                  AdjustSaturation();

                  /*ChengLin 20130117: Move to here for Sync DDC/CI commands for Athena Right/Left*/ 
                  #ifdef ST_4K2K_93xx_BOARD
                  if(GetChipID_4K2K()==CHIPID_4K2K_R)
                  {
         				//Send SocIPC DDC/CI Command to Athena L 																				   
         				DDC_Cmd_Buff[0] = DDC2B_CMD_VCP_TVSaturation;
         				DDC_Cmd_Buff[1] = (BYTE)(W_VCPValue>>8);
         				DDC_Cmd_Buff[2] = (BYTE)(W_VCPValue & 0x00FF);
         																																				   
         				if(SocIPCSendCmdPacketParser(SOCIPC_DEVICEL_DST_ADDRESS, SICMD_RW_DDCI_CONTRL, DDC_Cmd_Buff) == FALSE)
         				{
         					msg_ddc("\n<Soc IPC Error> Device left DDC Send Saturation Command Failure.\n",0); 
        					}
     					}
                  #endif				
               }
               else
               {              
                  W_VCPValue = UserPrefSaturation;
               }
               break;
            }
#endif

#ifdef Action_AutoAdjust_WB_Used
#if FEATURE_AIP == ENABLE

         case DDC2B_CMD_VCP_AutoSetUp:
            {
               if (SetVcp && W_VCPValue)
               {              
                  ModeSetupAutoAdjust(gmvb_MainChannel);
               }
               else
               {
                  W_VCPMax = 1;
                  B_VCPType = 1;
               }
               break;
            }
#endif
#endif


#if 0//def ADJUSTER_OSDLANGUAGE_DECLARED
         case DDC2B_CMD_VCP_OsdLanguage:
            {
               if (SetVcp)
               {
                  switch (W_VCPValue)
                  {
                     case 1://chinese
                        UserPrefOsdLanguage = 0;
                        AdjustOsdLanguage();
                        break;
                     case 2: //english
                        UserPrefOsdLanguage = 1;
                        AdjustOsdLanguage();
                  }

                  /*ChengLin 20130117: Move to here for Sync DDC/CI commands for Athena Right/Left*/ 

                  #ifdef ST_4K2K_93xx_BOARD
                  if(GetChipID_4K2K()==CHIPID_4K2K_R)
                  {
                     //Send SocIPC DDC/CI Command to Athena L																									  
                     DDC_Cmd_Buff[0] = DDC2B_CMD_VCP_OsdLanguage;
                     DDC_Cmd_Buff[1] = (BYTE)(W_VCPValue>>8);
                     DDC_Cmd_Buff[2] = (BYTE)(W_VCPValue & 0x00FF);

                     if(SocIPCSendCmdPacketParser(SOCIPC_DEVICEL_DST_ADDRESS, SICMD_RW_DDCI_CONTRL, DDC_Cmd_Buff) == FALSE)
                     {
                        msg_ddc("\n<Soc IPC Error> Device left DDC Send OSD Language Command Failure.\n",0);
                     }
                  }
                  #endif

               }
               else
               {                              
                  if (UserPrefOsdLanguage == 1)
                     W_VCPValue = 1;
                  else if (UserPrefOsdLanguage == 0)
                     W_VCPValue = 2;
                  else W_VCPValue = 0;

                  W_VCPMax = 11;			  
               }
               break;
            }
#endif

            // TODO: JasonL - revisit these functions and implement

#ifdef ADJUSTER_CURRENTINPUTMAIN_DECLARED
         case DDC2B_CMD_VCP_SourceSelect:
            {
               B_Temp = UserPrefCurrentInputMain;
               if (SetVcp)
               {
                  switch (W_VCPValue)
                  {
                     case 1:
                        B_Temp = ALL_BOARDS_VGA1;
                        break;
                     case 2:
                        B_Temp = ALL_BOARDS_DVI1;
                        break;
                     case 3:
                        B_Temp = ALL_BOARDS_HDMI;
                        break;
                     case 4:
                        B_Temp = ALL_BOARDS_DP1;
                        break;

                     default:
                        IsFeatureSupported = FALSE;
                        break;
                  }

                  if (B_Temp != UserPrefCurrentInputMain)
                  {
                     UserPrefCurrentInputMain = B_Temp;
                     AdjustCurrentInputMain();
                  }
               }
               else
               {
                  //W_VCPMax = VCP_SourceSelect_ComponentVideo_1;
                  W_VCPMax = 0xFFFF;
                  switch (UserPrefCurrentInputMain)
                  {
                     case ALL_BOARDS_VGA1:
                        W_VCPValue = 1;
                        break;
                     case ALL_BOARDS_DVI1:
                        W_VCPValue = 2;
                        break;
                     case ALL_BOARDS_HDMI:
                        W_VCPValue = 3;
                        break;
                     case ALL_BOARDS_DP1:
                        W_VCPValue = 4;
                        break;


                     default:
                        IsFeatureSupported = FALSE;
                        break;
                  }
               }
               break;
            }
#endif


#if 0//def ADJUSTER_OSDLANGUAGE_DECLARED
         case DDC2B_CMD_VCP_Language:
            {
               if (SetVcp)
               {
                  if (W_VCPValue)
                  {
                     UserPrefOsdLanguage = W_VCPValue - 1;
                     AdjustOsdLanguage();
                  }

                  /*ChengLin 20130117: Move to here for Sync DDC/CI commands for Athena Right/Left*/ 
                  #ifdef ST_4K2K_93xx_BOARD
                  if(GetChipID_4K2K()==CHIPID_4K2K_R)
                  {
         				//Send SocIPC DDC/CI Command to Athena L																														  
         				DDC_Cmd_Buff[0] = DDC2B_CMD_VCP_Language;
         				DDC_Cmd_Buff[1] = (BYTE)(W_VCPValue>>8);
         				DDC_Cmd_Buff[2] = (BYTE)(W_VCPValue & 0x00FF);
         																																													  
         				if(SocIPCSendCmdPacketParser(SOCIPC_DEVICEL_DST_ADDRESS, SICMD_RW_DDCI_CONTRL, DDC_Cmd_Buff) == FALSE)
         				{
         					msg_ddc("\n<Soc IPC Error> Device left DDC Send Language Command Failure.\n",0);
       					}
     					}
                  #endif
               }
               else
               {
                  W_VCPValue = UserPrefOsdLanguage + 1;
                  W_VCPMax = 2;			  
               }
               break;
            }
#endif

         case DDC2B_CMD_VCP_AudioVolume:
            {


               if (SetVcp)
               {              
                  UserPrefAudioVolume = W_VCPValue;
                  AdjustHSyncPhase();			  
               }
               else
               {
                  W_VCPValue = UserPrefAudioVolume;
                  W_VCPMax = 0x1F;
		     B_VCPType = 1;	
               }
               break;
           
            }





         case DDC2B_CMD_VCP_Settings:
            {
               if (SetVcp)
               {
                  SaveModeIndependentSettings();
               #if FEATURE_AIP == ENABLE
                  LookUpType.W_Hfreq = stInput[gmvb_MainChannel].Hfrq;
                  LookUpType.W_Vfreq = stInput[gmvb_MainChannel].Vfrq;
                  LookUpType.W_Vtotal = stInput[gmvb_MainChannel].Vtotal;
                  SaveModeDependentSettings(gmvb_MainChannel, &LookUpType);
               #endif
               }
               else
               {
                  B_VCPType =1;
                  W_VCPMax =1;
               }
               break;
            }

         default:
            {
               //error condition - unsupported vcp opcode

               IsFeatureSupported = FALSE;
               break;
            }
      }
   }
   else if (VCPPage == 2)
   {
      switch (VCPFeature.vcp_opcode)
      {
#if 0//def ADJUSTER_OSDHPOSITION_DECLARED
         case DDC2B_CMD_VCP_OSDHPosition:
            {
               if (SetVcp)
               {
                  UserPrefOsdHPosition = W_VCPValue;
                  AdjustOsdHPosition();

                  /*ChengLin 20130117: Move to here for Sync DDC/CI commands for Athena Right/Left*/ 				  
                  #ifdef ST_4K2K_93xx_BOARD
                  if(GetChipID_4K2K()==CHIPID_4K2K_R)
                  {
         				//Send SocIPC DDC/CI Command to Athena L 																																		   
         				DDC_Cmd_Buff[0] = DDC2B_CMD_VCP_OSDHPosition;
         				DDC_Cmd_Buff[1] = (BYTE)(W_VCPValue>>8);
         				DDC_Cmd_Buff[2] = (BYTE)(W_VCPValue & 0x00FF);
         																																																   
         				if(SocIPCSendCmdPacketParser(SOCIPC_DEVICEL_DST_ADDRESS, SICMD_RW_DDCI_CONTRL, DDC_Cmd_Buff) == FALSE)
         				{
         					msg_ddc("\n<Soc IPC Error> Device left DDC Send OSD HPosition Command Failure.\n",0);
        					}
     					}
                  #endif				
               }
               else
               {	               
                  W_VCPValue= UserPrefOsdHPosition;
               }
               break;
            }
#endif






#if 0//def ADJUSTER_OSDVPOSITION_DECLARED
         case DDC2B_CMD_VCP_OSDVPosition:
            {
               if (SetVcp)
               {
                  UserPrefOsdVPosition = W_VCPValue;
                  AdjustOsdVPosition();

                  //*ChengLin 20130117: Move to here for Sync DDC/CI commands for Athena Right/Left
                  #ifdef ST_4K2K_93xx_BOARD
                  if(GetChipID_4K2K()==CHIPID_4K2K_R)
                  {
         				//Send SocIPC DDC/CI Command to Athena L 																																			   
         				DDC_Cmd_Buff[0] = DDC2B_CMD_VCP_OSDVPosition;
         				DDC_Cmd_Buff[1] = (BYTE)(W_VCPValue>>8);
         				DDC_Cmd_Buff[2] = (BYTE)(W_VCPValue & 0x00FF);
         																																																				   
         				if(SocIPCSendCmdPacketParser(SOCIPC_DEVICEL_DST_ADDRESS, SICMD_RW_DDCI_CONTRL, DDC_Cmd_Buff) == FALSE)
         				{
         					msg_ddc("\n<Soc IPC Error> Device left DDC Send OSD VPosition Command Failure.\n",0);
        					}
    					}
                  #endif				
               }
               else
               {	               
                  W_VCPValue= UserPrefOsdVPosition;
               }
               break;
            }
#endif

         default:
            {
               //error condition - unsupported vcp opcode
               IsFeatureSupported = FALSE;
               break;
            }
      }
   }
   else
   { //error condition - unsupported page

      //set current page to 0
      VCPPage = 0;
      // NOTE:
      //readExpected = 0; //no message back if page is not supported

      IsFeatureSupported = FALSE;
   }

   if (!SetVcp)
   {
      VCPFeature.max_BigEndian = EndianConv(W_VCPMax);
      VCPFeature.value_BigEndian = EndianConv(W_VCPValue);
      VCPFeature.vcp_type = B_VCPType;
   }

   if (IsFeatureSupported)
      return 0;
   else
   {
      //VCPFeature.max_BigEndian = 0;
      VCPFeature.value_BigEndian = 0;

      VCPFeature.max_BigEndian = 0xffff;

      return 1;
   }
}

#endif //DDCCi_PROTOCOL_SUPPORT

#undef __DDC2BI_COMM_C__


