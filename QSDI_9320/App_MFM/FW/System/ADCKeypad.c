/*
	$Workfile:   ADCKeypad.c  $
	$Revision: 1.24 $
	$Date: 2013/05/06 03:21:14 $
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
//
//		---
//
//		MODULE:		ADCKeypad.c
//
//		USAGE:		ADC Keypad related functions
//
//******************************************************************************
//******************************************************************************
//  I N C L U D E    F I L E S
//******************************************************************************
#include "system\all.h"
#include <math.h>
#if (FEATURE_LOW_POWER_MONITOR == ENABLE)
#include "StdLibInc\gm_LPM_IPC.h"
#endif

#define DEBUG_MSG 1
#if DEBUG_MSG
#define msg(a,b) gm_Print(a,b)
#define msg_x(a,b,c) gm_Print(a,b,c)
#else
#define msg(a,b)
#define msg_x(a,b,c)
#endif

#define ADC_KBD_NOISE_FILTER	0x10	// Up to 0.2 volts is keypad noise.

enum
{
   ADC_KEY1,
   ADC_KEY2,
   ADC_KEY3,
   MAX_ADC_KEY
};

enum
{
#ifdef PORT_GPIO_USED
   GPIO_KEY,
#endif
#ifdef PORT_ADC_1_USED
   ADC1_KEY,
#endif
#ifdef PORT_ADC_2_USED
   ADC2_KEY,
#endif
#ifdef PORT_ADC_3_USED
   ADC3_KEY,
#endif
#ifdef PORT_IR_USED
   IR_KEY,
#endif
   MAX_KEY_TYPE
};

/*ChengLin 20130315: Avoid OSD Menu Showing  during Splash Screen*/
#ifdef ST_4K2K_93xx_BOARD
extern BOOL AllowKeypadAction;
#if ENABLE_PBP_ATHENA_L_DEEP_SLEEP_CHECK               
extern BOOL AthenaRIgnoreSleep_PBP;
#endif
#endif

extern void GpioLed1Keypad(BOOL B_Set);
extern void GpioLed2Keypad(BOOL B_Set);

#if (defined(PORT_ADC_1_USED) || defined(PORT_ADC_2_USED) || defined(PORT_ADC_3_USED) \
    ||(defined(WBK_FUNCTION_GETADCVALUE_USED) && !defined(LPM_PERMANENTLY_CONTROLS_PERIPHERAL)))
BYTE B_AdcKeyData[NUM_KEYPAD_CH];
BYTE B_AdcData[NUM_KEYPAD_CH];
#endif

#ifdef WBK_FUNCTION_GETADCVALUE_USED		// WB4
static BYTE B_AdcChanNum = 0;
static BYTE B_AdcDriversIndex[NUM_KEYPAD_CH];
#endif

static struct
{
   gmt_LED_COLOR	LED_Color;
   gmt_LED_ATTRIB	LED_Attrib;
   gmt_LED_STATUS	LED_Status;
} LEDStruct;

#if (defined(PORT_ADC_1_USED) | defined(PORT_ADC_2_USED) | defined(PORT_ADC_3_USED))

//******************************************************************************
//
// FUNCTION       : static KEY_INDEX TranslateAdcToKey (BYTE B_KeyPort, BYTE B_Value)
//
// USAGE          : Compares ADC value with table code by the method binary tree
//
// INPUT          : B_KeyPort - ADC key port number
//                     (ADC_KEY1, ADC_KEY2, ADC_KEY3)
//                  B_Value
//
// OUTPUT         : KEY_INDEX - id pushed key
//                  If none of the key is pressed, return Key_None
//
// GLOBALS        : NUMBER_OF_ADC_1Values (RO) - number of members in array ADC_1Value
//                  ADC_1Values 			   (RO)- a struct to map ADC code to id key.
//                  NUMBER_OF_ADC_2Values (RO) - number of members in array ADC_2Value
//                  ADC_1Values 			   (RO)- a struct to map ADC code to id key.
//                  NUMBER_OF_ADC_3Values (RO) - number of members in array ADC_3Value
//                  ADC_1Values 			   (RO)- a struct to map ADC code to id key.
//
// USED_REGS      : None
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
static KEY_INDEX TranslateAdcToKey (BYTE B_KeyPort, BYTE B_Value)
{
   SBYTE  SB_Result;
   SWORD  B_Offset, B_Max, B_Min = 0;
   ST_ADC ROM	*ADC_Values;

#ifdef PORT_ADC_1_USED
   if (B_KeyPort == ADC_KEY1)
   {
      B_Max = NUMBER_OF_ADC_1Values - 1;
      ADC_Values = ADC_1Values;
   }
#endif
#ifdef PORT_ADC_2_USED
   if (B_KeyPort == ADC_KEY2)
   {
      B_Max = NUMBER_OF_ADC_2Values - 1;
      ADC_Values = ADC_2Values;
   }
#endif
#ifdef PORT_ADC_3_USED
   if (B_KeyPort == ADC_KEY3)
   {
      B_Max = NUMBER_OF_ADC_3Values - 1;
      ADC_Values = ADC_3Values;
   }
#endif
   if (B_Value > ADC_KBD_NOISE_FILTER)		//is key press present?
   {
      // Do a binary tree search of all the ADC1 values until we find the
      // real key value where our read value falls into the range of and
      // return this real value
      // If B_Min goes past B_Max, then no corresponding code was found so
      // we return Key_None.
      while (B_Min <= B_Max)
      {
         B_Offset = ((WORD)B_Max + B_Min) / 2;
         // Check if the value falls within the range of the current ADC
         // value we are pointing at.
         if (abs(B_Value - ADC_Values[B_Offset].Code) <= ADC_DELTA)
            return(ADC_Values[B_Offset].KeyIndex);
         else if (ADC_Values[B_Offset].Code > B_Value)
            B_Max = B_Offset - 1;
         else
            B_Min = B_Offset + 1;
      }
   }
   return Key_None;
}

//******************************************************************************
//
// FUNCTION       : BYTE ScanNextAdcKey (void)
//
// USAGE          : Scans the next ADC channel.
//
// INPUT          : None
//
// OUTPUT         : TRUE - ADC key available
//                  FALSE - no ADC key available
//
// GLOBALS        : NUMBER_OF_ADC_1Values		(RO)
//                  NUMBER_OF_ADC_2Values		(RO)
//                  NUMBER_OF_ADC_3Values		(RO)
//                  B_AdcData					(R/W)
//                  B_AdcKeyData				(W)
//
// USED_REGS      : None
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
BYTE ScanNextAdcKey (void)
{
   static BYTE B_AdcChannel = 2;
   BYTE B_Temp;

   // select next available ADC channel (one at a time - cycle through the defined ports)
   do
   {
      if (	++B_AdcChannel > 2)
         B_AdcChannel = 0;

      if (	B_AdcChannel == 0 && NUMBER_OF_ADC_1Values ||
            B_AdcChannel == 1 && NUMBER_OF_ADC_2Values ||
            B_AdcChannel == 2 && NUMBER_OF_ADC_3Values)
         break;

   } while (TRUE);

   // program ADC select bits, scan and debounce specified channel
   B_Temp = KeyFilter(B_AdcChannel,gm_ScanAdcKey ((BIT1 << B_AdcChannel) & (BIT2|BIT3)));

   // If difference from emitting value exceeds threshold, then update emitting value
   if (abs((SWORD) B_Temp-B_AdcData[B_AdcChannel]) > ADC_DELTA/2)
   {
      B_AdcData[B_AdcChannel] = B_Temp;
      B_AdcKeyData[0] = TranslateAdcToKey(0,B_AdcData[0]);
      B_AdcKeyData[1] = TranslateAdcToKey(1,B_AdcData[1]);
      B_AdcKeyData[2] = TranslateAdcToKey(2,B_AdcData[2]);
      return TRUE;
   }

   return FALSE;
}
#endif

extern gmt_PwrStateType PwrState;
BOOL IsLockKeypadLightControl(void)
{
   return (PwrState == Pwr_Transition ? TRUE : FALSE);
}
//******************************************************************************
//
// FUNCTION       : void KeypadLightControl(BYTE B_Control)
//
// USAGE          : Controls the keypad LED
//
// INPUT          : B_Control
//
// OUTPUT         : None
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
void KeypadLightControl(gmt_LED_COLOR B_Color, gmt_LED_ATTRIB B_Attrib)
{
   LEDStruct.LED_Attrib = B_Attrib;

   // Save LED status (active or inactive)
   if (B_Color != KEYPAD_LED_OFF)
      LEDStruct.LED_Status = LED_ON;
   else
      LEDStruct.LED_Status = LED_OFF;

   if(!IsLockKeypadLightControl())
   {
      switch (B_Color)
      {
         case KEYPAD_LED_OFF:
            GpioLed1Keypad(FALSE);
            GpioLed2Keypad(FALSE);
            break; 

         case KEYPAD_LED_RED:
            GpioLed1Keypad(TRUE);
            GpioLed2Keypad(FALSE);
            break;

         case KEYPAD_LED_GREEN:
            GpioLed1Keypad(TRUE); //on
            GpioLed2Keypad(TRUE); //off
            break;

         case KEYPAD_LED_ORANGE:
            GpioLed1Keypad(TRUE);
            GpioLed2Keypad(FALSE);
            break;

         default:
            break;
      }
   }
/*
   if (B_Attrib == LED_BLINKING)
   {
      gm_TimerStart(BLINKING_LED_TMR, 3); // 300msec

      if (B_Color != KEYPAD_LED_OFF)
         LEDStruct.LED_Color = B_Color;
   }
*/
}
//******************************************************************************
//
// FUNCTION       :
//
// USAGE          :
//
// INPUT          :
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
void BlinkingHandler(void)
{
   if ((LEDStruct.LED_Attrib == LED_BLINKING) && (gm_TimerCheck(BLINKING_LED_TMR) == TIMER_TMO))
   {
      if (LEDStruct.LED_Status == LED_OFF)
         KeypadLightControl(LEDStruct.LED_Color, LED_BLINKING);
      else
         KeypadLightControl(KEYPAD_LED_OFF, LED_BLINKING);
   }
}
#ifdef WBK_FUNCTION_GETADCVALUE_USED		// WB4
//******************************************************************************
//
// FUNCTION       : BYTE InitADCDriver( BYTE B_Index )
//
// USAGE          : Initializes ADC driver
//
// INPUT          : B_Index - index of this keypad in WBK_Keypads[].
//
// OUTPUT         : TRUE
//
// GLOBALS        : WBK_Keypads[]
//
// USED_REGS      : None
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
#ifdef LPM_PERMANENTLY_CONTROLS_PERIPHERAL
#if 0
#pragma argsused
BYTE InitADCDriver( BYTE B_Index )		// WB4
{
#pragma pack(push)
#pragma pack(1)
   struct
   {
      BYTE timeOut; // =5ms, 5*4channels = 20ms debounce time
      BYTE treshold; // =8
      BYTE numChannels; // =4 by default, 6 - maximum
      BYTE offChannel; //=0xFF - no selected power on button (any key)
      BYTE offValue; //=24 PowerOn key code
   } settings = {5, 8, 4, 0, 24}; // could be taken from WB project.
#pragma pack(pop)
   gm_MessageSend(gmd_MESSAGE_LBADC, &settings, sizeof(settings));
   return TRUE;
}
#else
BYTE InitADCDriver( BYTE B_Index )		// WB4
{
   UNUSED_VAR(B_Index);
   // Set power key definition in GpioControl.c by each board.
   // LPM read table and config GPADC and GPIO from OTP free memory.
   // Return OK to OSD.
   return TRUE;
}
#endif
#else
BYTE InitADCDriver( BYTE B_Index )		// WB4
{
//   B_AdcDriversIndex[B_AdcChanNum] = B_Index;
//   B_AdcChanNum++;
   gm_SetRegBitsWord(LPM_GPADC_GPIO_CONTROL,GPADC_ADC0IN_DIS|GPADC_ADC1IN_DIS|GPADC_ADC2IN_DIS|GPADC_ADC3IN_DIS);
   gm_WriteRegWord(LPM_GPADC_CTRL,0x5AB);
   UNUSED_VAR(B_Index);
   return TRUE;
}

#endif

//******************************************************************************
//
// FUNCTION       : BYTE GetADCValue( BYTE B_Index )
//
// USAGE          : ADC driver function. Scans the ADC channel and translates code
//                  to key value.
//
// INPUT          : B_Index - index of this keypad in WBK_Keypads[].
//
// OUTPUT         : TRUE - ADC key available
//                  FALSE - no ADC key available
//
// GLOBALS        : WBK_Keypads[], B_AdcData[]
//
// USED_REGS      : None
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************

#if (FEATURE_SOCIPC == ENABLE)
   #ifdef ST_4K2K_93xx_BOARD 
      #if ENABLE_SOC_IPC_KEYPAD_VIA_QUEUE
      KeypadRecvFormat_t SocIPCKeypadBuf;
      #else //Original
      BOOL g_Keypad_GetValidKeys = FALSE;
      BYTE g_Keypad_Channel = 0x00;
      BYTE g_Keypad_Value = 0x00;
      #endif
   #endif
#endif

#ifdef LPM_PERMANENTLY_CONTROLS_PERIPHERAL
BYTE GetADCValue( BYTE B_Index )		// WB4
{
   #pragma pack(push)
   #pragma pack(1)
   typedef struct
   {
      BYTE ch;
      BYTE val;
   } t_LbwAdcKey;

   #pragma pack(pop)
   t_LbwAdcKey key;
   WBK_KEYPAD_DEFINITION ROM* Sp_Keypad;

   if (!gm_MessageReceived(gmd_MESSAGE_LBADC, &key, sizeof(key)))
      return FALSE;

//if(key.ch==0 && (key.val>30 )&& key.val<45)  //BurnInMode
//	UserPrefBurnInMode=0;


   msg("GetADCValue, key.ch = %d", key.ch);
   msg("GetADCValue, key.val = %d", key.val);

/*ChengLin 20130315: Avoid OSD Menu Showing  during Splash Screen*/
#ifdef ST_4K2K_93xx_BOARD
   if(AllowKeypadAction == FALSE)
      return FALSE;

#if ENABLE_PBP_ATHENA_L_DEEP_SLEEP_CHECK               
   if((AthenaRIgnoreSleep_PBP) && (GetChipID_4K2K()==CHIPID_4K2K_R))
   {
      if(((key.ch == 0) &&((key.val>= 229)&&(key.val<= 239))) //Switch Key
         ||((key.ch == 0) &&((key.val>= 170)&&(key.val<= 180))) //Source Key 
         ||((key.ch == 0) &&((key.val>= 250))) //vol+ Key
         ||((key.ch == 1) &&((key.val>= 199)&&(key.val<= 209))) //vol- Key
         ||((key.ch == 1) &&((key.val>= 250))) //Menu Key
         )
         {
            gm_TimerStart(PBP_SLEEP_CHECK_TIMEOUT_1ms_TMR, PBP_SLEEP_WAIT_TIME_OUT);
            msg("\n<Keypad ADC>PBP_SLEEP_CHECK_TIMEOUT_1ms_TMR Start!!\n",0);
         }
   }
#endif
#endif

#if (FEATURE_3D == ENABLE)
#if (FEATURE_3D_KEY_HANDLER == ENABLE)
   if (gm_3D_KeyEventHandler((BYTE far *) &key))
   {
      EnqueueOsdKey(Key_None);
      return TRUE;
   }
#endif
#endif

   for (B_Index = 0;;)
   {
      Sp_Keypad = WBK_Keypads[B_Index];
      if (Sp_Keypad
            && (Sp_Keypad->GetKeyValue == GetADCValue) //is it LBADC keypad? (but IR)
            && (Sp_Keypad->B_ChanNumber == key.ch)) //chack channel number
         break;

      if (B_Index++>=WB_NUMBER_OF_INPUT_SOURCE)
         return FALSE;
   }

   if ( Sp_Keypad->B_KeysNumber == 0 )
      return FALSE;

   EnqueueOsdKey((key.val <= ADC_KBD_NOISE_FILTER) ? Key_None : //if key released than Key_None
   TranslateCodeToKey(Sp_Keypad->Bp_KeyValues, Sp_Keypad->B_KeysNumber,
      Sp_Keypad->B_CodeLen, Sp_Keypad->B_Delta, key.val));

   return TRUE;
}
#else // Non-LPM_PERMANENTLY_CONTROLS_PERIPHERAL
WORD W_AdcPrevKey = 0xFFFF;
BYTE B_LastKey[NUM_KEYPAD_CH];
BYTE B_EmitKey[NUM_KEYPAD_CH];
BYTE B_Count[NUM_KEYPAD_CH];
BYTE GetADCValue( BYTE B_Index )		// WB4
{
   WBK_KEYPAD_DEFINITION ROM* Sp_Keypad;
   WORD W_PhKey = 0;
   BYTE KeyIndex;
   BYTE B_AdcChannel;

   Sp_Keypad = WBK_Keypads[B_Index];

   if ( Sp_Keypad->B_KeysNumber == 0 )
      return FALSE;

   if(Sp_Keypad->B_ChanNumber == 1)
      B_AdcChannel =1;
   else if(Sp_Keypad->B_ChanNumber == 0)
      B_AdcChannel =0;

   W_PhKey = gm_ScanAdcKey(B_AdcChannel);

   //Stability filter
   //See if the ADC value has changed over a threshhold or not
   if (abs(W_PhKey-B_LastKey[B_AdcChannel]) < ADC_DELTA/2)
   {
      if (~B_Count[B_AdcChannel])
         B_Count[B_AdcChannel] ++;
   }
   else
   {
      B_Count[B_AdcChannel] = 0;
   }
   B_LastKey[B_AdcChannel] = W_PhKey;

   if (B_Count[B_AdcChannel] >= 2)
   {
      B_EmitKey[B_AdcChannel] = W_PhKey;
   }
   W_PhKey = B_EmitKey[B_AdcChannel];

   // If difference from emitting value exceeds threshold, then update emitting value
   if (abs((SWORD) W_PhKey-B_AdcData[B_AdcChannel]) > Sp_Keypad->B_Delta/2)
   {
      B_AdcData[B_AdcChannel] = W_PhKey;
      if (W_PhKey > ADC_KBD_NOISE_FILTER)		//is key press present?
      {
      }
      else
      {
         W_PhKey = 0;
      }
   }

   if(W_PhKey < 30) // Ignore noise
      W_PhKey = 0; 

   if ( W_PhKey == 0 && W_AdcPrevKey == 0)
      return FALSE;

   KeyIndex = TranslateCodeToKey(Sp_Keypad->Bp_KeyValues, Sp_Keypad->B_KeysNumber,
                                 Sp_Keypad->B_CodeLen, Sp_Keypad->B_Delta, W_PhKey);
   EnqueueOsdKey(KeyIndex);
   if (W_AdcPrevKey != W_PhKey)
   {
      //msg_x("GetADCValue KeyIndex = %d, W_PhKey = 0x%X",KeyIndex,W_PhKey);
      W_AdcPrevKey = W_PhKey;
   }
   return TRUE;
}
#endif

#endif // #ifdef WBK_FUNCTION_GETADCVALUE_USED

#ifdef WBK_FUNCTION_GETGPIOVALUE_USED

#ifdef VIRTUAL_GPIO_KEY
WORD VirtualGPINPUT,VirtualGPIODIRCTRL;
#endif

WORD ROM W_ArrayOfGPIOInputAddress[] =
{
   GPINPUT0, // Channel 0
#ifdef VIRTUAL_GPIO_KEY
   &VirtualGPINPUT // Channel 1
#endif
};
WORD ROM W_ArrayOfGPIODirAddress[] =
{
   GPIO_0_DIRCTRL, // Channel 0
#ifdef VIRTUAL_GPIO_KEY
   &VirtualGPIODIRCTRL // Channel 1
#endif
};

#pragma pack(push)
#pragma pack(1)
typedef struct WBK_ST_KeyConvert
{
   WORD Key_Ph;
   BYTE Key_Log; // Key index
} WBK_ST_KeyConvertType;
#pragma pack(pop)

WORD GetMaskGPIO(WBK_KEYPAD_DEFINITION ROM* Bp_Data)
{
   BYTE B_Count = ((WBK_KEYPAD_DEFINITION ROM*)Bp_Data)->B_KeysNumber;
   WORD W_KeysMask = 0;
   WBK_ST_KeyConvertType ROM* Sp_Masks = (WBK_ST_KeyConvertType ROM*)(((WBK_KEYPAD_DEFINITION ROM*)Bp_Data)->Bp_KeyValues);

   for (; B_Count; B_Count--)
   {
      W_KeysMask |= Sp_Masks->Key_Ph;
      Sp_Masks++;
   }
   return W_KeysMask;
}

BYTE InitGPIODriver(BYTE B_Index)
{
   WBK_KEYPAD_DEFINITION ROM* Sp_Keypad;
   WORD W_KeysMask;
   WORD W_Address;
   Sp_Keypad = WBK_Keypads[B_Index];
   W_KeysMask = GetMaskGPIO(Sp_Keypad);
   W_Address = W_ArrayOfGPIODirAddress[Sp_Keypad->B_ChanNumber];
   msg("W_Address = 0x%x",W_Address);
   msg("W_KeysMask = 0x%x",W_KeysMask);
   gm_WriteRegWord(W_Address, gm_ReadRegWord(W_Address) & ~W_KeysMask);
   msg("InitGPIODriver(%d)",Sp_Keypad->B_ChanNumber);
   return TRUE;
}

//*****************************************************************************
// FUNCTION     : BYTE GetLGPIOValue (WBK_KEYPAD_DEFINITION ROM* Bp_Data )
// USAGE        : This function reads data from current GPIO
// INPUT        : Bp_Data - pointer to keypad structure
// OUTPUT       : physical GPIO key data
// GLOBALS      : none
// USED_REGS    : none
//*****************************************************************************
#define KEY_POLARITY 1    // polarity for GPIO keypad
WORD GetLGPIOValue(WBK_KEYPAD_DEFINITION ROM* Bp_Data )
{
   WORD W_KeysMask = GetMaskGPIO(Bp_Data);
   BYTE B_Channel = Bp_Data->B_ChanNumber;

   //msg("B_KeysMask 0x%x",W_KeysMask);
#if KEY_POLARITY
   return (gm_ReadRegWord(W_ArrayOfGPIOInputAddress[B_Channel]) & W_KeysMask);
#else
   return (~gm_ReadRegWord(W_ArrayOfGPIOInputAddress[B_Channel]) & W_KeysMask);
#endif    //  KeyPolarity 
}

//*****************************************************************************
// FUNCTION     : BYTE GetGPIOValue (BYTE B_Index)
// USAGE        : This function returns logical data from current GPIO
// INPUT        : B_Index - Keypad/IR channel
// OUTPUT       : logical GPIO key data
// GLOBALS      : none
// USED_REGS    : none
//*****************************************************************************
WORD W_PrevKey = 0xFFFF;
BYTE GetGPIOValue(BYTE B_Index)
{
   WBK_KEYPAD_DEFINITION ROM* Sp_Keypad;
   WORD W_PhKey = 0;
   BYTE KeyIndex;

   Sp_Keypad = WBK_Keypads[B_Index];

   if ( Sp_Keypad->B_KeysNumber == 0 )
      return FALSE;

#ifdef VIRTUAL_GPIO_KEY
   if (gm_TimerCheck(TV_BACKGROUNDAFT_100ms_TMR) == TIMER_TMO)
   {
      VirtualGPINPUT = 0;
      gm_TimerStop(TV_BACKGROUNDAFT_100ms_TMR);
   }
#endif

   W_PhKey = GetLGPIOValue(Sp_Keypad);
   if ( W_PhKey == 0 && W_PrevKey == 0)
      return FALSE;

   KeyIndex = TranslateCodeToKey(Sp_Keypad->Bp_KeyValues, Sp_Keypad->B_KeysNumber,
                                 Sp_Keypad->B_CodeLen, Sp_Keypad->B_Delta, W_PhKey);
   EnqueueOsdKey(KeyIndex);
   if (W_PrevKey != W_PhKey)
   {
      msg_x("GetGPIOValue KeyIndex = %d, W_PhKey = 0x%X",KeyIndex,W_PhKey);
      W_PrevKey = W_PhKey;
   }
   return TRUE;
}

#ifdef VIRTUAL_GPIO_KEY
void SetVirtualGPINPUT(WORD value)
{
   VirtualGPINPUT = value;
   gm_TimerStart(TV_BACKGROUNDAFT_100ms_TMR, 2); // Auto release virtual gpio key timeout 200ms
}
#endif
#endif
//*********************************  END  **************************************

