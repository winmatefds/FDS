/*
	$Workfile:   ExternalDeviceConfig.c  $
	$Revision: 1.13 $
	$Date: 2011/12/13 10:32:40 $
*/

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
// MODULE:      ExtDeviceConfig.c
//
// USAGE:       This module contains functions that is used for
//				Configuration of External Devices based in Connector
//
//******************************************************************************

//******************************************************************************
//  I N C L U D E    F I L E S
//******************************************************************************

#include "System\all.h"

//******************************************************************************
//  D E F I N E S
//******************************************************************************

//******************************************************************************
//  S T A T I C   V A R I A B L E S
//******************************************************************************

#ifdef ANX9021_DEV_ID
static BOOL Bt_HDMI_Connected = FALSE;
#endif
//******************************************************************************
//  C O D E
//******************************************************************************

//******************************************************************************
//
// FUNCTION       : void ConfigureExtDevicesVideo(gmt_UserDefConnNames B_InputConnector)
//
// USAGE          : Call this function to configure External Devices based on
//                  input Connector
//
// INPUT          : B_InputConnector - Input Connector
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
void ConfigureExtDevicesVideo(gmt_UserDefConnNames B_InputConnector)
{
   //
   // Digital Input Processing
   //
   switch (B_InputConnector)
   {
      default:
         break;
   }//End of Switch
}

static void SSM2602_ByteWrite(BYTE B_Address, BYTE B_Data)
{
   BYTE Ba_Buffer[2];
   BYTE Addr ;
   Ba_Buffer[0] = B_Address;
   Ba_Buffer[1] = B_Data;



	if (gm_Write2WireBlock(1, 0x34, (BYTE*)Ba_Buffer, 2, TRUE) == gmd_PASS)
    {
         //gm_Printf("Write command   SSM2602 by I2C 0x42", 0);
    }
    else
    {
      gm_Printf("Write command  SSM2602 Failed=%x",B_Address);
    }
/*
	gm_Delay1ms(2);



Addr=B_Address;
   gm_Printf("read from 0x%x succeeded ",Addr);


	   if (gm_Write2WireBlock(1, 0x34, (BYTE*)&Addr, 1, FALSE) == gmd_FAIL)
    		{
      			gm_Printf("Write command  SSM2602 Failed=%x",Addr);

		}


          if (gm_Read2WireBlock(1, 0x34, (BYTE*) &Addr, 1, TRUE) == gmd_FAIL)
          {
      			gm_Printf("Write command  SSM2602 Failed=%x",Addr);
          }





             gm_Printf("Data=0x%x",Addr);


*/

	gm_Delay1ms(2);
   
}





static void ADV7180_ByteWrite(BYTE B_Address, BYTE B_Data)
{
   BYTE Ba_Buffer[2];

   Ba_Buffer[0] = B_Address;
   Ba_Buffer[1] = B_Data;

 //  gm_Write2WireBlock(1, 0x40, Ba_Buffer, 2, TRUE);


	if (gm_Write2WireBlock(1, 0x42, (BYTE*)Ba_Buffer, 2, TRUE) == gmd_PASS)
    {
     //    gm_Printf("Write command   ADV7180l by I2C 0x42", 0);
	   UserPrefVIDEO_D=1;
    }
    else
    {
      gm_Printf("Write command  ADV7180 Failed=%x",B_Address);
	   UserPrefVIDEO_D=0;
    }
   
}


void ADV7180_CVBS(void)
{
//gm_Delay10ms(20);
 //ADV7180_ByteWrite(0x0F, 0x80); 

//gm_Delay10ms(50);
/*
   ADV7180_ByteWrite(0x0, 0x04); 
   ADV7180_ByteWrite(0x04,0x57);
   ADV7180_ByteWrite(0x17, 0x41);
   ADV7180_ByteWrite(0x31, 0x02); 
   ADV7180_ByteWrite(0x3d, 0xa2); 
   ADV7180_ByteWrite(0x3e,0x6a);
   ADV7180_ByteWrite(0x3f, 0xa0);
   ADV7180_ByteWrite(0x0e, 0x80); 
   ADV7180_ByteWrite(0x55, 0x81); 
   ADV7180_ByteWrite(0x0e, 0x00); 
*/

   ADV7180_ByteWrite(0x0, 0x04); 
   ADV7180_ByteWrite(0x04,0x57);
   ADV7180_ByteWrite(0x17, 0x41);
   ADV7180_ByteWrite(0x31, 0x02); 
   ADV7180_ByteWrite(0x3d, 0xa2); 
   ADV7180_ByteWrite(0x3e,0x6a);
   ADV7180_ByteWrite(0x3f, 0xa0);
   ADV7180_ByteWrite(0x0e, 0x20);

   ADV7180_ByteWrite(0x40, 0xD6); 
   ADV7180_ByteWrite(0x4C, 0x06); 
   ADV7180_ByteWrite(0x0e, 0x00); 
   ADV7180_ByteWrite(0x0e, 0x80); 
   ADV7180_ByteWrite(0x55, 0x81); 
   ADV7180_ByteWrite(0x0e, 0x00); 
//ADV7180_ByteWrite(0x0E, 0x20); // Access interrupt/VDP register space 
//ADV7180_ByteWrite(0x40, 0x36); // Drive high when active   Manual interrupt mode enabled   Pseudo sync only 
//ADV7180_ByteWrite(0x43, 0x02); // Clears SD_UNLOCK_Q 
//ADV7180_ByteWrite(0x44, 0x63);// Unmasks SD_LOCK_Q   SD_UNLOCK_Q bit SD_FR_CHNG_Q  MV_PS_CS_Q bit 


/*
   ADV7180_ByteWrite(0x0, 0x04); 
   ADV7180_ByteWrite(0x04,0x57);
   ADV7180_ByteWrite(0x17, 0x41);
   ADV7180_ByteWrite(0x31, 0x02); 
   ADV7180_ByteWrite(0x3d, 0xa2); 
   ADV7180_ByteWrite(0x3e,0x6a);
   ADV7180_ByteWrite(0x3f, 0xa0);
   ADV7180_ByteWrite(0x0e, 0x80); 
   ADV7180_ByteWrite(0x55, 0x81); 
   ADV7180_ByteWrite(0x0e, 0x00); 

*/


}



void ADV7180_SVIDEO(void)
{

#if (DISPLAY_FUNCTION==VGA_DVI_2CVBS)
//CVBS
   ADV7180_ByteWrite(0x0, 0x00); 
   ADV7180_ByteWrite(0x04,0x57);
   ADV7180_ByteWrite(0x17, 0x41);
   ADV7180_ByteWrite(0x31, 0x02); 
   ADV7180_ByteWrite(0x3d, 0xa2); 
   ADV7180_ByteWrite(0x3e,0x6a);
   ADV7180_ByteWrite(0x3f, 0xa0);
   ADV7180_ByteWrite(0x0e, 0x80); 
   ADV7180_ByteWrite(0x55, 0x81); 
   ADV7180_ByteWrite(0x0e, 0x00); 
#else
//SVIDEO
   ADV7180_ByteWrite(0x0, 0x06); 
   ADV7180_ByteWrite(0x04,0x57);
   ADV7180_ByteWrite(0x31, 0x02); 
   ADV7180_ByteWrite(0x3d, 0xa2); 
   ADV7180_ByteWrite(0x3e,0x6a);
   ADV7180_ByteWrite(0x3f, 0xa0);
   ADV7180_ByteWrite(0x0e, 0x80); 
   ADV7180_ByteWrite(0x55, 0x81); 
   ADV7180_ByteWrite(0x0e, 0x00); 
   ADV7180_ByteWrite(0x39, 0x80); 
#endif



}



void SSM2602_init(void)
{




/* i2s
SSM2602_ByteWrite(0x00,0x17);//0x00	0x017; Set ADC left channel 0dB
SSM2602_ByteWrite(0x02,0x17);//0x01	0x017; Set ADC right channel 0dB
SSM2602_ByteWrite(0x04,0x79);//0x02	0x079; Set DAC left channel 0dB
SSM2602_ByteWrite(0x06,0x79);//0x03	0x079; Set DAC right channel 0dB
//SSM2602_ByteWrite(0x04,0x02);//0x04	0x002; ADC select Line_IN, MIC mute
SSM2602_ByteWrite(0x08,0x02);//0x04	0x002; ADC select Line_IN, MIC mute
SSM2602_ByteWrite(0x0a,0x08);//0x05	0x008; DAC mute
//SSM2602_ByteWrite(0x06,0x0a);//0x06	0x00A; DAC, MIC power down
SSM2602_ByteWrite(0x0c,0x0a);//0x06	0x00A; DAC, MIC power down
SSM2602_ByteWrite(0x0e,0x4a);//0x07	0x04A; Master mode, 24bit, I2S format
SSM2602_ByteWrite(0x10,0x20);//0x08	0x020; DAC\ADC sample rate 44.1KHz, Oversample rate = 256fs
SSM2602_ByteWrite(0x12,0x01);//0x09	0x000; Disable digital core
//SSM2602_ByteWrite(0x1e,0x00);//0x0F	0x000; Reset
SSM2602_ByteWrite(0x20,0x7b);//0x10	0x07B; ALC disable,
SSM2602_ByteWrite(0x22,0x32);//0x11	0x032; ALC disable set Decay time and attack time default
SSM2602_ByteWrite(0x24,0x00);//0x12	0x000; Noise gate disable
*/

// Line output 
/*
SSM2602_ByteWrite(0x00,0x17);//0x00	0x017; Set ADC left channel 0dB
SSM2602_ByteWrite(0x02,0x17);//0x01	0x017; Set ADC right channel 0dB
SSM2602_ByteWrite(0x04,0x79);//0x02	0x079; Set DAC left channel 0dB
SSM2602_ByteWrite(0x06,0x79);//0x03	0x079; Set DAC right channel 0dB
//SSM2602_ByteWrite(0x04,0x02);//0x04	0x002; ADC select Line_IN, MIC mute
SSM2602_ByteWrite(0x08,0x0a);//0x04	0x002; ADC select Line_IN, MIC mute
SSM2602_ByteWrite(0x0a,0x08);//0x05	0x008; DAC mute
//SSM2602_ByteWrite(0x06,0x0a);//0x06	0x00A; DAC, MIC power down
SSM2602_ByteWrite(0x0c,0x06);//0x06	0x00A; DAC, MIC power down
SSM2602_ByteWrite(0x0e,0x0a);//0x07	0x04A; Master mode, 24bit, I2S format
SSM2602_ByteWrite(0x10,0x20);//0x08	0x020; DAC\ADC sample rate 44.1KHz, Oversample rate = 256fs
SSM2602_ByteWrite(0x12,0x01);//0x09	0x000; Disable digital core
//SSM2602_ByteWrite(0x1e,0x00);//0x0F	0x000; Reset
SSM2602_ByteWrite(0x20,0x7b);//0x10	0x07B; ALC disable,
SSM2602_ByteWrite(0x22,0x32);//0x11	0x032; ALC disable set Decay time and attack time default
SSM2602_ByteWrite(0x24,0x00);//0x12	0x000; Noise gate disable
*/
//Line in--> I2S output &  Line output 
SSM2602_ByteWrite(0x00,0x17);//0x00	0x017; Set ADC left channel 0dB
SSM2602_ByteWrite(0x02,0x17);//0x01	0x017; Set ADC right channel 0dB
SSM2602_ByteWrite(0x04,0x79);//0x02	0x079; Set DAC left channel 0dB
SSM2602_ByteWrite(0x06,0x79);//0x03	0x079; Set DAC right channel 0dB
SSM2602_ByteWrite(0x08,0x0a);//0x04	0x002; ADC select Line_IN, MIC mute
SSM2602_ByteWrite(0x0a,0x08);//0x05	0x008; DAC mute
SSM2602_ByteWrite(0x0c,0x02);//0x06	0x00A; DAC, MIC power down
SSM2602_ByteWrite(0x0e,0x4a);//0x07	0x04A; Master mode, 24bit, I2S format
SSM2602_ByteWrite(0x10,0x20);//0x08	0x020; DAC\ADC sample rate 44.1KHz, Oversample rate = 256fs
SSM2602_ByteWrite(0x12,0x01);//0x09	0x000; Disable digital core
SSM2602_ByteWrite(0x20,0x7b);//0x10	0x07B; ALC disable,
SSM2602_ByteWrite(0x22,0x32);//0x11	0x032; ALC disable set Decay time and attack time default
SSM2602_ByteWrite(0x24,0x00);//0x12	0x000; Noise gate disable

//Power up sequence
SSM2602_ByteWrite(0x0c,0x10);//Set output power down

SSM2602_ByteWrite(0x12,0x00);//Disable digital core

SSM2602_ByteWrite(0x12,0x01);//0x09	0x001; //Active digital core
SSM2602_ByteWrite(0x0c,0x00);//0x06	0x000; //Enable output power



//Line_IN BYPASS to Headphone and Line_Out

//0x00	0x017; Set ADC left channel 0dB
//0x01	0x017; Set ADC right channel 0dB
//0x02	0x079; Set DAC left channel 0dB
//0x03	0x079; Set DAC right channel 0dB
//0x04	0x00A; Enable Line_In bypass, mute MIC
//0x05	0x008; DAC mute
//0x06	0x006; ADC power down, MIC power down
//0x07	0x00A; Slave mode, 24bit, I2S format
//0x08	0x020; DAC\ADC sample rate 44.1KHz, Oversample rate = 256fs
//0x09	0x000; Disable digital core
//0x0F	0x000; Reset
//0x10	0x07B; ALC disable
//0x11	0x032; ALC disable set Decay time and attack time default
//0x12	0x000; Noise gate disable



/*

0x00	0x017; Set ADC left channel 0dB
0x01	0x017; Set ADC right channel 0dB
0x02	0x079; Set DAC left channel 0dB
0x03	0x079; Set DAC right channel 0dB
0x04	0x002; ADC select Line_IN, MIC mute
0x05	0x008; DAC mute
0x06	0x00A; DAC, MIC power down
0x07	0x04A; Master mode, 24bit, I2S format
0x08	0x020; DAC\ADC sample rate 44.1KHz, Oversample rate = 256fs
0x09	0x000; Disable digital core
0x0F		0x000; Reset
0x10	0x07B; ALC disable,
0x11	0x032; ALC disable set Decay time and attack time default
0x12	0x000; Noise gate disable

*/


UserPrefAudioVolume=25;


}



void ADV7180_Detect(void)
{
   BYTE Addr = 0x10;


	   if (gm_Write2WireBlock(1, 0x42, (BYTE*)&Addr, 1, FALSE) == gmd_FAIL)
    		{
 			gm_Printf("Write to 0x10  at offset 0x%x succeeded", Addr);

		}


          if (gm_Read2WireBlock(1, 0x42, (BYTE*) &Addr, 1, TRUE) == gmd_FAIL)
          {
             gm_Printf("read from 0x10 succeeded, result=0x%x",Addr);
          }

	   if(Addr & 0x01)
	   	{

			   ADV7180_ByteWrite(0x03,0x0c);
	   }
	   else
	   	{


			ADV7180_ByteWrite(0x03,0x40);

	   }


}











//******************************************************************************
//
// FUNCTION       : void ExternalDeviceInit (void)
//
// USAGE          : Initializes the external device on board
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        : None
//
// USED_REGS      : None
//
// PRE_CONDITION  : This function should be called after NVRAM userprefs are available
//
// POST_CONDITION :
//
//******************************************************************************
void ExternalDeviceInit (void)
{
   //
   // Initialize External Decoder
   //
	  SSM2602_init();
	//if((UserPrefCurrentInputMain == ALL_BOARDS_CVBS1) ||(UserPrefCurrentInputPip == ALL_BOARDS_CVBS1))
	   ADV7180_CVBS();
	//else
	//   ADV7180_SVIDEO();

   
	
#ifdef EXTERNAL_DECODER
/*
#ifdef VPC3230_DEV_ID

   vdd_ConfigVideoMode(VDD_1,VDD_MODE_INIT);

#if (DEFAULT_DIP_DAUGHTER_BOARD_OUT == VPC3230_656)
   vdd_SetVideoOutputFormat(VDD_1,VDD_OUTTYPE_656);
   gm_Print("656", 0);
#endif
#if (DEFAULT_DIP_DAUGHTER_BOARD_OUT == VPC3230_601)
   gm_Print("601", 0);
   vdd_SetVideoOutputFormat(VDD_1,VDD_OUTTYPE_601);
#endif


#if (DEFAULT_DIP_DAUGHTER_BOARD_IN == VPC3230_COMPONENT_IN)
   gm_Print("component", 0);
   vdd_SetVideoInput(VDD_1,VDD_INTYP_COMPONENT);
#endif
#if (DEFAULT_DIP_DAUGHTER_BOARD_IN == VPC3230_COMPOSITE_IN)
   gm_Print("composite", 0);
   vdd_SetVideoInput(VDD_1,VDD_INTYP_COMPOSITE2);
#endif

   vdd_ConfigVideoMode(VDD_1,VDD_MODE_NTSC);

#endif



#ifdef ANX9021_DEV_ID
   if (ANX9021_IsConnected())
   {
      ANX9021_Init();
      Bt_HDMI_Connected = TRUE;
      gm_Printf("HDMI ANX9021 module initialized",0);
   }
   else
   {
      gm_Printf("HDMI ANX9021 module absent",0);
   }
#endif

*/

#endif
}

//******************************************************************************
//
// FUNCTION       : BOOL IsHDMI_Connected(void)
//
// USAGE          : Return if HDMI module is connected
//
// INPUT          : None
//
// OUTPUT         : TRUE or FALSE
//
// GLOBALS        : None
//
// USED_REGS      : None
//
// PRE_CONDITION  : None
//
// POST_CONDITION : None
//
//******************************************************************************
BOOL IsHDMI_Connected(void)
{
#ifdef ANX9021_DEV_ID
   return Bt_HDMI_Connected;
#else
   return TRUE;//just to be safe
#endif
}

//******************************************************************************
//
// FUNCTION       : BOOL IsDVI_Connected(void)
//
// USAGE          : Return if DVI module is connected
//
// INPUT          : None
//
// OUTPUT         : TRUE or FALSE
//
// GLOBALS        : None
//
// USED_REGS      : None
//
// PRE_CONDITION  : None
//
// POST_CONDITION : None
//
//******************************************************************************
BOOL IsDVI_Connected(void)
{
   return TRUE;//just to be safe
}

