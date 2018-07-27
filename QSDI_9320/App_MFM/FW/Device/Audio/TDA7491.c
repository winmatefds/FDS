
#include "System\all.h"

#if (FEATURE_AUDIO == ENABLE)

#if (FEATURE_TDA7491 == ENABLE)
void TDA7491_GPIO_SET(gmt_GPIO_ID index, BOOL set)
{
   switch (index)
   {
   default:
      break;
	  
   case GPIO_81:
      gm_SetRegBitsWord(GPIO_5_ENABLE,       GPIO81_EN);
      gm_SetRegBitsWord(GPIO_5_DIRCTRL,      GPIO81_IO);
      gm_ClearRegBitsWord(GPIO_5_OPENDRAIN_EN, GPIO81_OD);     // 3.3v
      gm_ClearAndSetBitsWord(GPOUTPUT5, GPIO81_OUT, ((set) ? 0 : GPIO81_OUT));
      break;

   case GPIO_82:
      gm_SetRegBitsWord(GPIO_5_ENABLE,       GPIO82_EN);
      gm_SetRegBitsWord(GPIO_5_DIRCTRL,      GPIO82_IO);
      gm_ClearRegBitsWord(GPIO_5_OPENDRAIN_EN, GPIO82_OD);     // 3.3v
      gm_ClearAndSetBitsWord(GPOUTPUT5, GPIO82_OUT, ((set) ? 0 : 0));
      break;

   case GPIO_83:
  //   gm_SetRegBitsWord(GPIO_5_ENABLE,       GPIO83_EN);
 //     gm_SetRegBitsWord(GPIO_5_DIRCTRL,      GPIO83_IO);
   //   gm_ClearRegBitsWord(GPIO_5_OPENDRAIN_EN, GPIO83_OD);     // 3.3v
  //    gm_ClearAndSetBitsWord(GPOUTPUT5, GPIO83_OUT, ((set) ? 0 : 0));
      break;

   case GPIO_86:
      gm_SetRegBitsWord(GPIO_5_ENABLE,       GPIO86_EN);
      gm_SetRegBitsWord(GPIO_5_DIRCTRL,      GPIO86_IO);
      gm_ClearRegBitsWord(GPIO_5_OPENDRAIN_EN, GPIO86_OD);     // 3.3v
      gm_ClearAndSetBitsWord(GPOUTPUT5, GPIO86_OUT, ((set) ? GPIO86_OUT : 0));
      break;

   case GPIO_91:
	gm_SetRegBitsWord(GPIO_5_ENABLE, 		GPIO91_EN);
	gm_SetRegBitsWord(GPIO_5_DIRCTRL,		GPIO91_IO);
	gm_ClearRegBitsWord(GPIO_5_OPENDRAIN_EN, GPIO91_OD);		// 3.3v
       gm_ClearAndSetBitsWord(GPOUTPUT5, GPIO91_OUT, ((set) ? GPIO91_OUT : 0));
	break;
		
   }
}

gmt_TDA7491_PIN_INFO * TDA7491_GetPinInfo(void)
{
   static gmt_TDA7491_PIN_INFO info =
   {
      GPIO_81, GPIO_86, GPIO_82, GPIO_83
   };

   return (gmt_TDA7491_PIN_INFO *) &info;
}

void TDA7491_Pin_Config(void)
{
   switch (Get_Behavior_ID())
   {
   case BEHAVIOR_ID_93xx_RD1:
   case BEHAVIOR_ID_93xx_RD1_REV_B:
   case BEHAVIOR_ID_93xx_RD2:
   case BEHAVIOR_ID_93xx_RD3:
      TDA7491_GetPinInfo()->PIN_MUTE   = GPIO_81;
      TDA7491_GetPinInfo()->PIN_STBY   = GPIO_86;
      TDA7491_GetPinInfo()->PIN_GAIN_0 = GPIO_82;
      TDA7491_GetPinInfo()->PIN_GAIN_1 = GPIO_83;
      break;

   case BEHAVIOR_ID_93xx_ST_4K2K_L:
      TDA7491_GetPinInfo()->PIN_MUTE   = GPIO_81;
      TDA7491_GetPinInfo()->PIN_STBY   = GPIO_86;
      TDA7491_GetPinInfo()->PIN_GAIN_0 = GPIO_82;
      TDA7491_GetPinInfo()->PIN_GAIN_1 = GPIO_91;
      break;
   }
}

void TDA7491_STBY(BOOL stby)
{
   // AUD_STBY
   // 0 : stabdby on
   // 1 : standby off

   if (stby)
   {
      TDA7491_GPIO_SET(TDA7491_GetPinInfo()->PIN_STBY, FALSE);
   }
   else
   {
      TDA7491_GPIO_SET(TDA7491_GetPinInfo()->PIN_STBY, TRUE);
   }
}

void TDA7491_MUTE(BOOL mute)
{
   // "Mode Selection" "STBY" "MUTE"
   //    Standby        L      X (don't care)
   //    Mute           H      L
   //    Play           H      H

   // Main mute
   // 0 : mute on
   // 1 : mute off

   if (mute)
   {
      TDA7491_GPIO_SET(TDA7491_GetPinInfo()->PIN_MUTE, FALSE);
   }
   else
   {
      TDA7491_GPIO_SET(TDA7491_GetPinInfo()->PIN_MUTE, TRUE);
   }
}

void TDA7491_GAIN(BYTE level)
{
   // "GAIN0" "GAIN1" "Nominal gain, Gv (dB)"
   //   0       0       20
   //   0       1       26
   //   1       0       30
   //   1       1       32

level=0;
/*

   if(UserPrefCurrentInputMain == ALL_BOARDS_CVBS1 ||UserPrefCurrentInputMain == ALL_BOARDS_SVIDEO1 )
   	{
      gm_SetRegBitsWord(GPIO_3_ENABLE,       GPIO59_EN);
      gm_SetRegBitsWord(GPIO_3_DIRCTRL,      GPIO59_IO);
      gm_ClearRegBitsWord(GPIO_3_OPENDRAIN_EN, GPIO59_OD);     // 3.3v
	gm_ClearRegBitsWord(GPOUTPUT3, GPIO59_OUT);


      gm_SetRegBitsWord(GPIO_3_ENABLE,       GPIO60_EN);
      gm_SetRegBitsWord(GPIO_3_DIRCTRL,      GPIO60_IO);
      gm_ClearRegBitsWord(GPIO_3_OPENDRAIN_EN, GPIO60_OD);     // 3.3v
	gm_ClearRegBitsWord(GPOUTPUT3,      GPIO60_OUT);


      gm_SetRegBitsWord(GPIO_3_ENABLE,       GPIO61_EN);
      gm_SetRegBitsWord(GPIO_3_DIRCTRL,      GPIO61_IO);
      gm_ClearRegBitsWord(GPIO_3_OPENDRAIN_EN, GPIO61_OD);     // 3.3v  
	gm_SetRegBitsWord(GPOUTPUT3, GPIO61_OUT);
   	}
   else
   	{


      gm_SetRegBitsWord(GPIO_3_ENABLE,       GPIO59_EN);
      gm_SetRegBitsWord(GPIO_3_DIRCTRL,      GPIO59_IO);
      gm_ClearRegBitsWord(GPIO_3_OPENDRAIN_EN, GPIO59_OD);     // 3.3v
	gm_SetRegBitsWord(GPOUTPUT3, GPIO59_OUT);


      gm_SetRegBitsWord(GPIO_3_ENABLE,       GPIO60_EN);
      gm_SetRegBitsWord(GPIO_3_DIRCTRL,      GPIO60_IO);
      gm_ClearRegBitsWord(GPIO_3_OPENDRAIN_EN, GPIO60_OD);     // 3.3v
	gm_ClearRegBitsWord(GPOUTPUT3,      GPIO60_OUT);


      gm_SetRegBitsWord(GPIO_3_ENABLE,       GPIO61_EN);
      gm_SetRegBitsWord(GPIO_3_DIRCTRL,      GPIO61_IO);
      gm_ClearRegBitsWord(GPIO_3_OPENDRAIN_EN, GPIO61_OD);     // 3.3v  
	gm_ClearRegBitsWord(GPOUTPUT3, GPIO61_OUT);
   }
   */
   
   switch (level)
   {
   default:
   case 0:
      TDA7491_GPIO_SET(TDA7491_GetPinInfo()->PIN_GAIN_0, FALSE);
      TDA7491_GPIO_SET(TDA7491_GetPinInfo()->PIN_GAIN_1, FALSE);
      break;

   case 1:
      TDA7491_GPIO_SET(TDA7491_GetPinInfo()->PIN_GAIN_0, FALSE);
      TDA7491_GPIO_SET(TDA7491_GetPinInfo()->PIN_GAIN_1, TRUE);
      break;

   case 2:
      TDA7491_GPIO_SET(TDA7491_GetPinInfo()->PIN_GAIN_0, TRUE);
      TDA7491_GPIO_SET(TDA7491_GetPinInfo()->PIN_GAIN_1, FALSE);
      break;

   case 3:
      TDA7491_GPIO_SET(TDA7491_GetPinInfo()->PIN_GAIN_0, TRUE);
      TDA7491_GPIO_SET(TDA7491_GetPinInfo()->PIN_GAIN_1, TRUE);
      break;
   }

   // Print("AUDIO - TDA7491_GAIN(%d)", level);
}
#endif

void TDA7491_ENABLE(void)
{
#if (FEATURE_TDA7491 == ENABLE)
   // Print("AUDIO - TDA7491_ENABLE()");

   // power-on and then play
   TDA7491_STBY(FALSE);
   TDA7491_MUTE(FALSE);
#endif
}

void TDA7491_DISABLE(void)
{
#if (FEATURE_TDA7491 == ENABLE)
   // mute and then standby
   TDA7491_MUTE(TRUE);
   TDA7491_STBY(TRUE);
#endif
}

void TDA7491_Initialize(void)
{
#if (FEATURE_TDA7491 == ENABLE)
   // Print("AUDIO - TDA7491_Initialize()");

   TDA7491_Pin_Config();
   TDA7491_DISABLE();
   TDA7491_GAIN(0);
#endif
}

#endif
