/*
   $Workfile:   Lpm_Keypad.c  $
   $Revision: 1.33 $
   $Date: 2012/10/08 09:28:12 $
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
// MODULE:      Lpm_Keypad.c
//
// USAGE:       Used in Lpm_Main.c to provide control of low bandwidth ADC and GPIO key by LPM micro
//
//******************************************************************************
#include ".\LowPowerMonitor\Lpm_All.h"
#include <stdlib.h>
#include <embedded.h>


#ifdef KEYPAD_SUPPORT

#pragma pack(push)
#pragma pack(1)
typedef struct
{
    BYTE timeOut; // =5ms, 5*4channels = 20ms debounce time
    BYTE treshold; // =4 by default;
    BYTE numChannels; // =4 by default, 6 - maximum
    BYTE offChannel; //=0xFF - no selected power on button (any key)
    BYTE offValue; //=24 PowerOn key code
} gmt_LbwAdcSettings;
#pragma pack(pop)

static gmt_LbwAdcSettings settings;
static BYTE curChannel;
static WORD timeX;
static BYTE valHistory[4][2]; //maximum number of channels
#if LPM_FORCE_POWER_DOWN
extern BYTE IsDcOffEvent;
extern BYTE IsForceDcOff;
#endif


//******************************************************************************
//
// USAGE:     Handle keypad depressed events to pass key to Mission or do power up process
//
// INPUT:      None
//
// OUTPUT:   TRUE - specific key to power on/off has been depressed
//            	FALSE - specific key to power on/off has not been depressed
//
// GLOBALS:      None
//
// USED_REGS:   None
//
// PRE_CONDITION  : 
//
// POST_CONDITION : 
//
//******************************************************************************
extern BOOL powerUpRequest;

void Lpm_KeypadHandler(void)
{  
   WORD w = gm_W_GetSystemTime();

   if (((w - timeX) >= settings.timeOut) )
   //			&& (gm_ReadRegWord(LPM_GPADC_RESULT)& CONV_DONE))
   {			 
      timeX = w;			  
      gm_ClearRegBitsWord(LPM_GPADC_CTRL, GPADC_EN |GPADC_INPUT_SELECT); //clear channel number and START_CONV bit			
      {	  
      #pragma pack(push)
      #pragma pack(1)
      typedef struct 
      {
         BYTE ch; 
         BYTE val;	//gpADC use 10Bits to convert ADC 
      } t_LbwAdcMsg;
      #pragma pack(pop)

      t_LbwAdcMsg msg;
      msg.val = (gm_ReadRegWord(LPM_GPADC_RESULT) & GPADC_DATA)/4; // Read value from 10bit gpADC and sample to 8bit.
      msg.ch = curChannel & 0x7f;   
         
      if (++curChannel & 0x80) //to skip first (valHistory-initialization) steps.
      {
         valHistory[msg.ch][0] = valHistory[msg.ch][1] = msg.val;
         if (curChannel >= 0x80+settings.numChannels)
            curChannel = 0;
      }
      else 
      {
         if ((abs(msg.val -valHistory[msg.ch][0]) > settings.treshold)
         &&(abs(msg.val -valHistory[msg.ch][1]) <= settings.treshold))
         {
            if (IsMissionPoweredUp())
            {
               if (msg.ch == 2)	//save cable ADC2 & ADC3 status and report to mission on CableDetectHandler 
               {
                  gm_Print("----ADC2 update %d",msg.val);
                  LbadcCableNowState[0] = msg.val;
                  CableUpdateRequest = TRUE;
               }
               else if (msg.ch == 3)
               {
                  gm_Print("----ADC3 update %d",msg.val);
                  LbadcCableNowState[1] = msg.val;
                  CableUpdateRequest = TRUE;							
               }
               else	
               {
                  //Update keypad ADC0 & ADC1 to mission immediately
                  if (msg.ch==0)	//save cable ADC2 & ADC3 status and report to mission on CableDetectHandler 
                     gm_Print("----ADC0 update %d",msg.val);
                  else
                     gm_Print("----ADC1 update %d",msg.val);
                  						
                  if (gm_MessageSend(gmd_MESSAGE_LBADC, &msg, sizeof(msg))==FALSE)
                     gm_Print("===Key message can't send ",0);

                  #if LPM_FORCE_POWER_DOWN
                  if (IsForceDcOff == TRUE && Lpm_KeypadGetKey(msg.ch, msg.val) == Key_POWER)
                  {
                     Lpm_TimerStart(PWROFF_1ms_TMR, LPM_DC_OFF_TIMEOUT);
                     IsDcOffEvent = TRUE;
                  }
                  #endif
               }
            }
            #ifdef POWER_MODE_HANDLE
            else
            {
               gmt_gpADCKeys GetKey = Lpm_KeypadGetKey(msg.ch, msg.val);
               
               if (msg.ch==0)	//save cable ADC2 & ADC3 status and report to mission on CableDetectHandler 
                  gm_Print("----ADC0 update %d",msg.val);
               else if (msg.ch == 1)
                  gm_Print("----ADC1 update %d",msg.val);           
               else if (msg.ch == 2)	
               {
                  gm_Print("----ADC2 update %d",msg.val);
                  LbadcCableNowState[0] = msg.val;
               }
               else if (msg.ch == 3)	
               {
                  gm_Print("----ADC3 update %d",msg.val);
                  LbadcCableNowState[1] = msg.val;
               }

               //gm_Print("----GetKey = %d",GetKey);

               if ((GetKey == Key_POWER) || (settings.offChannel == 0xFF))
                  Lpm_SetPowerKeyEvent();
               #ifdef ANY_KEY_WAKEUP
               else if ((GetKey > Key_POWER) && (GetKey < Key_NUM))
               {
                  if(Lpm_PowerModeGetState() == LPM_LowPowerMode)
                     Lpm_SetAnyKeyWakeEvent();
               }
               #endif   //#ifdef ANY_KEY_WAKEUP       
            }
            #endif   //#ifdef POWER_MODE_HANDLE
         }					 
         else  //Update cable status, for Mission's inquary @AC/DC power on
         {
            if (msg.ch == 2)  //save cable ADC2 & ADC3 status and report to mission on CableDetectHandler 
            {
            #if 0
            //[VGA,DVI] = {[0,0],[1,0],[0,1],[1,1]},  ADC2_VAL = {255, 210, 158, 126}
            if(  (abs(msg.val - 255)<= settings.treshold)
               ||(abs(msg.val - 210)<= settings.treshold)
               ||(abs(msg.val - 158)<= settings.treshold)
               ||(abs(msg.val - 126)<= settings.treshold) )
            #endif           
               LbadcCableNowState[0] = msg.val;
            }
            else if (msg.ch == 3)
            {
               LbadcCableNowState[1] = msg.val;
            }         
         }
         valHistory[msg.ch][0] = valHistory[msg.ch][1];
         valHistory[msg.ch][1] = msg.val;

         #if 1
         curChannel %= settings.numChannels;
         #else
         if (gm_ReadRegByte(0xdc66)==0)
            curChannel %= settings.numChannels; //ch=0,1,2,3
         else
            curChannel = gm_ReadRegByte(0xdc66)-1;
         #endif
      }
      }
   }
   gm_SetRegBitsWord(LPM_GPADC_CTRL, ((curChannel << 2) & GPADC_INPUT_SELECT)|GPADC_EN ); 
}
//******************************************************************************
//
// USAGE:     Init power key (GPIO) and non-power key (LbwAdc) of keypad
//
// INPUT:      None
//
// OUTPUT:   None
//
// GLOBALS:      None
//
// USED_REGS:   None
//
// PRE_CONDITION  : 
//
// POST_CONDITION : 
//
//******************************************************************************
void Lpm_KeypadInit(void)
{
   // ---------------------------------------------------------
   // The ADC default values are board by board
   // These settings are put in OTP memory from 0x270C0 by Mission side
   // So that the Lpm_KeypadLbwAdcInit() is moved to Lpm_MfpInit()
   //----------------------------------------------------------
   //Lpm_KeypadLbwAdcInit(4, 6, 4, 0, 24); //move to Lpm_MfpInit, numChannels, offChannel & offValue are sent from Mission

   //Config a GPIO pin to power key
   gm_SetRegBitsWord(LPM_GPADC_GPIO_CONTROL,GPADC_ADC0IN_DIS|GPADC_ADC1IN_DIS|GPADC_ADC2IN_DIS|GPADC_ADC3IN_DIS);

   gm_WriteRegWord(LPM_GPADC_CTRL,0x5AB);
}

//******************************************************************************
void Lpm_KeypadLbwAdcInit(BYTE timeOut, BYTE treshold, BYTE numChannels, BYTE offChannel, BYTE offValue)
{
   settings.timeOut = timeOut;
   settings.treshold = treshold;
   settings.numChannels = numChannels;
   settings.offChannel = offChannel;
   settings.offValue = offValue;

   curChannel = 0x80;
   CableUpdateRequest = FALSE;	
   timeX = gm_W_GetSystemTime();
}

gmt_gpADCKeys Lpm_KeypadGetKey(BYTE AdcCh, BYTE AdcVal)
{
   gmt_gpADCKeys GetKey = Key_NONE;

#if 0
	if ((settings.offChannel == AdcCh) && (AdcVal >150))		// Power
	  GetKey = Key_POWER;
#else

   if ( (settings.offChannel == AdcCh) && (abs(AdcVal -settings.offValue) <= settings.treshold) )
     GetKey = Key_POWER; 
   else if ((1 == AdcCh) && (abs(AdcVal -253) <= settings.treshold))    //Menu
      GetKey = Key_MENU;
   else if ((1 == AdcCh) && (abs(AdcVal -175) <= settings.treshold))    //Enter
      GetKey = Key_ENTER;
   else if ((0 == AdcCh) && (abs(AdcVal -111) <= settings.treshold))    //KeyTone  
      GetKey = Key_KEYTONE;
   else if ((0 == AdcCh) && (abs(AdcVal - 75) <= settings.treshold))    //Auto
      GetKey = Key_AUTO;
   else if ((0 == AdcCh) && (abs(AdcVal -234) <= settings.treshold))    //Swtich
      GetKey = Key_SWITCH;
   else if ((0 == AdcCh) && (abs(AdcVal -205) <= settings.treshold))    //PIP
      GetKey = Key_PIP;
   else if ((0 == AdcCh) && (abs(AdcVal -175) <= settings.treshold))    //Source
      GetKey = Key_SOURCE;
   else if ((1 == AdcCh) && (abs(AdcVal -111) <= settings.treshold))    //UP
      GetKey = Key_UP;
   else if ((1 == AdcCh) && (abs(AdcVal -234) <= settings.treshold))    //Down
      GetKey = Key_DOWN;
   else if ((1 == AdcCh) && (abs(AdcVal - 75) <= settings.treshold))    //Right
      GetKey = Key_RIGHT;
   else if ((1 == AdcCh) && (abs(AdcVal -144) <= settings.treshold))    //Left
      GetKey = Key_LEFT;
   else if ((0 == AdcCh) && (abs(AdcVal -144) <= settings.treshold))    //Channel Up
      GetKey = Key_CHANNEL_UP;
   else if ((1 == AdcCh) && (abs(AdcVal - 38) <= settings.treshold))    //Channel Down
      GetKey = Key_CHANNEL_DOWN;
   else if ((0 == AdcCh) && (abs(AdcVal -253) <= settings.treshold))    // Violume Up
      GetKey = Key_VOLUME_UP;
   else if ((1 == AdcCh) && (abs(AdcVal -205) <= settings.treshold))    // Violume Down
      GetKey = Key_VOLUME_DOWN;
#endif
   return GetKey;
}

#endif

