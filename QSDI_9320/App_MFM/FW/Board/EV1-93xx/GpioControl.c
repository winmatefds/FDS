/*
	$Workfile:   GpioControl.c  $
	$Revision: 1.65 $
	$Date: 2013/01/18 08:54:18 $
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
// MODULE:      GpioControl.c
//
// USAGE:       This module contains RD2_32626H_BOARD specific GPIO handling functions
//
//******************************************************************************
#include <embedded.h>
//******************************************************************************
//  L O C A L    D E F I N I T I O N S
//******************************************************************************

//
// I2C SCL FREQ (100KHz)
//
#define I2C_100K_FREQ		(100000L)
#define I2C_100K_SCALE		(((gDW_SystemOcmClk / I2C_100K_FREQ) / 5) - 2)

typedef enum
{
   CPhyType_None,
   CPhyType_DVI,
   CPhyType_HDMI,
   CPhyType_DP
}gmt_CPhyType;

//******************************************************************************
//  E X T E R N S
//******************************************************************************


//******************************************************************************
//
// FUNCTION       : void InitSystemGpio(void)
//
// USAGE          : Call individual functions to initialize the system board control GPIO.
//
// INPUT          :
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
void InitSystemGpio(void)
{
   // Mission GPIO control
   // GPIO1: SCBL	GPIO4: Dual Flash switch
//	gm_SetRegBitsWord(GPIO_0_ENABLE,(BIT1 | BIT4));
//	gm_SetRegBitsWord(GPIO_0_DIRCTRL,(BIT1 | BIT4));

   // Enable SCBL on GPIO1_PWM14
   gm_SetRegBitsWord(MFP_SPARE2,(BIT1 | BIT4));

   // GPIO77(13): UDO0	GPIO78(14): UDO0		GPIO79(15): LED_Green
   gm_SetRegBitsWord(GPIO_4_DIRCTRL, GPIO79_IO);
   gm_SetRegBitsWord(GPIO_4_ENABLE, GPIO79_EN);

   // GPIO80(Bit0): DIN connect detect	GPIO81(Bit1): AUD_Main_Mute_Ctrl
   // GPIO82(Bit2): Audio value ctrl		GPIO83(Bit3): audio board INT in
   // GPIO84(Bit4): Panel backlight ctrl	GPIO85(Bit5): HDMI board INT in
   // GPIO86(Bit6): Not use			GPIO87(Bit7): I2S Dout0
   // GPIO88(Bit8): /SRESET			GPIO89(Bit9): I2S Dout3
   gm_SetRegBitsWord(GPIO_5_ENABLE, (GPIO80_EN | GPIO81_EN | GPIO83_EN | GPIO85_EN | GPIO88_EN));  
   gm_SetRegBitsWord(GPIO_5_DIRCTRL, (GPIO81_IO | GPIO88_IO));

#ifdef USE_SECOND_UART
   // UART1 share pins (GPIO 79 & 80) with green LED and DIP connect detect
   gm_ClearRegBitsWord(GPIO_4_ENABLE, GPIO79_EN);
   gm_ClearRegBitsWord(GPIO_5_ENABLE, GPIO80_EN);
#endif   

#ifdef EXTERNAL_DECODER
   gm_SetRegBitsWord(GPOUTPUT5,GPIO88_OUT);
#endif

   // GPIO92 SPI-flash Hold. Enable GPIO function to override ENABLE_30BIT_OUTPUT 
   gm_SetRegBitsWord(GPIO_5_ENABLE,GPIO92_EN);
   gm_ClearRegBitsWord(GPIO_5_DIRCTRL,GPIO92_IO);

   // Backlight control
   gm_SetRegBitsWord(PWM3_CONFIG, PWM3_GPIOn_EN);

   // OCM_INT1 init
   gm_SetRegBitsWord(MFP_MISC_ALT_PINS, MFP_OCM_INT1_EN);

   gm_SetRegBitsWord(LPM_GPIO_ENABLE1, BIT0);

   gm_SetRegBitsWord(LPM_GPOUTPUT_1,LPM_GPIO20_OUT);
   gm_SetRegBitsWord(LPM_GPIO_DIRCTRL2, LPM_GPIO20_IO);
   gm_SetRegBitsWord(LPM_GPIO_ENABLE2, LPM_GPIO20_EN);

   // Cable detect / Source (Host) power detect / HPD (Hot plug) configuration
   // HPD1 (Bit2-0)
   //	000: 0(TRI)	001: DP0_HPD	010: HDMI_HPD	011: DP0_HPD(if PD1=1)	100: HDMI_HPD(if PD1=1)
   //	101: DP0_HPD(TRI, if DP1_HPD=1)	110: HDMI_HPD(TRI, if HDMI_HPD=1)
   // HPD2 (Bit6-4)
   //	000: 0(TRI)	001: DVI_HPD		010: HDMI_HPD	011: DP0_HPD(if PD1=1)	100: HDMI_HPD(if PD1=1)
   //	101: DVI_HPD(TRI, if DVI_HPD=1)	110: HDMI_HPD(TRI, if HDMI_HPD=1)
   // HPD3 (Bit10-8)
   //	000: 0(TRI)	001: DP1_HPD	010: HDMI_HPD	011: DP0_HPD(if PD1=1)	100: HDMI_HPD(if PD1=1)
   //	101: DP1_HPD(TRI, if DP1_HPD=1)	110: HDMI_HPD(TRI, if HDMI_HPD=1)

   #ifdef EV3_93xx_BOARD // ToDo: Check later
   #if (FEATURE_DISPLAY_PORT == ENABLE) && ENABLE_HPD_OVERRIDED
   gm_WriteRegWord(LPM_CSM_HPD_OUT_CTRL,0x0737);	// 0000 0111 0001 0111
   #else
   gm_WriteRegWord(LPM_CSM_HPD_OUT_CTRL,0x0131);	// 0000 0001 0001 0001
   #endif
   gm_WriteRegWord(LPM_CSM_HPD_PD_IN_CTRL,0x009F); // 0000 0000 1111 1111
   
   #else //EV1_93xx_BOARD
   #if (FEATURE_DISPLAY_PORT == ENABLE) && ENABLE_HPD_OVERRIDED
   gm_WriteRegWord(LPM_CSM_HPD_OUT_CTRL,0x0727);	// 0000 0111 0010 0111
   #else
   gm_WriteRegWord(LPM_CSM_HPD_OUT_CTRL,0x0121);	// 0000 0001 0010 0001
   #endif
   gm_WriteRegWord(LPM_CSM_HPD_PD_IN_CTRL,0x00FF); // 0000 0000 1111 1111
   #endif

   if (gm_ReadRegWord(LPM_MISSION_STATUS) & LP_SRAM_LOADED)
   {
      //LPM_C register can't RW by Mission directly after LPM starts up
      WORD msg[3] = {Clear_bit, LPM_GPIO_ENABLE1, LPM_GPIO8_EN|LPM_GPIO9_EN|LPM_GPIO10_EN};//3: clear bits
      gm_MessageSend(gmd_MESSAGE_LPM_GPIO_CTRL, &msg, sizeof(msg));
   }
}

//******************************************************************************
//
// FUNCTION       : void GpioSoftReset(WORD W_Delay)
//
// USAGE          : Initializes the soft reset (SRESET) GPIO
//
// INPUT          : W_Delay - number of milliseconds to be delayed in the function
//
// OUTPUT         : None
//
// GLOBALS        : None
//
// USED_REGS      : IO_EXP_PCF8575 (R/W)
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void GpioSoftReset(WORD W_Delay)
{
   // Mars is not using external IO
   UNUSED_VAR(W_Delay);
}

//******************************************************************************
//
// FUNCTION       : void I2C_Init(void)
//
// USAGE          : Initializes the I2C port
//
// INPUT          : None
//
// OUTPUT         : None
//
// GLOBALS        : None
//
// USED_REGS      : I2C_MST_CTR			(W)
//                  I2C_MST_CLK_SCALE	(W)
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void I2C_Init(void)
{
   // Master I2C on MSTR0
   gm_WriteRegWord(I2C_MST_CTR, I2C_EN | I2C_SDA_SCL0_EN | I2C_SDA_SCL1_EN);
   gm_WriteRegWord(I2C_MST_CLK_SCALE, I2C_100K_SCALE);
}

//******************************************************************************
//
// FUNCTION       : void GpioLed1Keypad(BOOL B_State)
//
// USAGE          : Writes to the registers to turn on or off LED1
//
// INPUT          : B_State - TRUE or FALSE
//
// OUTPUT         : None
//
// GLOBALS        : None
//
// USED_REGS      : IO_EXP_PCF8575 (R/W)
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void GpioLed1Keypad(BOOL B_State)
{
   //gm_SetRegBitsWord(GPIO_4_ENABLE, BIT15);
   //gm_SetRegBitsWord(GPIO_4_DIRCTRL, BIT15);
   if (B_State == TRUE)
   {
      gm_ClearRegBitsWord(GPOUTPUT4, BIT15);
   }
   else
   {
      gm_SetRegBitsWord(GPOUTPUT4, BIT15);
   }
}

//******************************************************************************
//
// FUNCTION       : void GpioLed2Keypad(BOOL B_State)
//
// USAGE          : Writes to the registers to turn on or off LED2
//
// INPUT          : B_State - TRUE or FALSE
//
// OUTPUT         : None
//
// GLOBALS        : None
//
// USED_REGS      : IO_EXP_PCF8575 (R/W)
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void GpioLed2Keypad(BOOL B_State)
{
   WORD msg_data[3];

   msg_data[1] = LPM_GPOUTPUT_1;
   msg_data[2] = LPM_GPIO20_OUT;

   if (B_State == TRUE)
   {
      msg_data[0] = Clear_bit;
      gm_MessageSend(gmd_MESSAGE_LPM_GPIO_CTRL, msg_data, sizeof(msg_data));
   }
   else
   {
      msg_data[0] = Set_bit;
      gm_MessageSend(gmd_MESSAGE_LPM_GPIO_CTRL, msg_data, sizeof(msg_data));
   }
}

//******************************************************************************
//
// FUNCTION       : void GpioLedFilmMode1(BYTE B_State)
//
// USAGE          : Turns on or off LED_MADI
//
// INPUT          : B_State - TRUE or FALSE
//
// OUTPUT         : None
//
// GLOBALS        : None
//
// USED_REGS      : IO_EXP_PCF8575 (R/W)
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void GpioLedFilmMode1(BYTE B_State)
{
   UNUSED_VAR(B_State);
}

//******************************************************************************
//
// FUNCTION       : void GpioLedFilmMode0(BYTE B_State)
//
// USAGE          : Turns on or off AFP_LED
//
// INPUT          : B_State - TRUE OR FALSE
//
// OUTPUT         : None
//
// GLOBALS        : None
//
// USED_REGS      : IO_EXP_PCF8575 (R/W)
//
// PRE_CONDITION  :
//
// POST_CONDITION :
//
//******************************************************************************
void GpioLedFilmMode0(BYTE B_State)
{
   UNUSED_VAR(B_State);
}

//******************************************************************************
//
// USAGE          : This is function which is called out of "SelectInput"
//                  procedure to configure board specific input parameters .
//                  This is function is being called through a pointer in the ports
//                  attributes table and provides flexibiliy with different functions
//                  associated with different ports.
//
// INPUT          : B_Channel - Current input channel
//                  B_InputConnector - Input connector to be connected to the channel
//
//******************************************************************************
#pragma argsused
void SelectInputExternal(gmt_PHY_CH B_Channel, gmt_UserDefConnNames B_InputConnector)
{
   //--------- HDCP port switch -----------
   if ((UserPrefCurrentInputMain == ALL_BOARDS_VGA1) || (UserPrefCurrentInputPip == ALL_BOARDS_VGA1))
   {
      gm_WriteRegWord(HW_SERIAL_SLAVE, 0x100); // VGA
   }
   else
   {
      gm_WriteRegWord(HW_SERIAL_SLAVE, 0x200); // DVI or HDMI
   }

//    if ((UserPrefCurrentInputPip == ALL_BOARDS_HDMI) || (UserPrefCurrentInputMain == ALL_BOARDS_HDMI))
   {
#if CHECK_ITEM
#pragma message "ToDo:Temporary set for Hdmi Hdcp, need to verify later"
#endif
      gm_WriteRegWord(MFP_SPARE1, 0x0); // HDMI SLV2
   }

   //------ DDC2Bi port switch ---------
   {
      // Use CSM I2C Mux to route I2C slave pins to DDC
      static TDdc2bi ddc2biData;
      if (UserPrefCurrentInputMain == ALL_BOARDS_VGA1)
      {
         gm_DDC2Bi_Init(DDC2BI_CHANNEL0, &ddc2biData);
         gm_CsmI2CMuxSelect(MUX_DDC4_LpmI2cSlv4);
      }
      else // default set to HDMI/DVI
      {
         gm_DDC2Bi_Init(DDC2BI_CHANNEL1, &ddc2biData);
         //if(UserPrefCurrentInputMain == ALL_BOARDS_HDMI)
         //gm_CsmI2CMuxSelect(MUX_DDC2_MisI2cSlv2);
#if CHECK_ITEM
#pragma message "ToDo:Temporary set for Hdmi Hdcp, need to verify later"
#endif

#ifdef EV3_93xx_BOARD
   		gm_CsmI2CMuxSelect(MUX_DDC2_LpmI2cSlv2_MisDviCp);
#else
         gm_CsmI2CMuxSelect(MUX_DDC2_LpmI2cSlv2_MisHdmiCp);
#endif
         //else if(UserPrefCurrentInputMain == ALL_BOARDS_DVI1)
         //	gm_CsmI2CMuxSelect(MUX_DDC3_MisI2cSlv3);
      }
   }

}

#define LBADC_DELTA           4 // Tolerance
#define LBADC_DVI_CABLE     210 // ADC value for only DVI cable plugged
#define LBADC_VGA_CABLE     158 // ADC value for only VGA cable plugged
#define LBADC_DVI_VGA_CABLE 126 // ADC value for both DVI and VGA cable plugged
#define LBADC_EQUAL(a,b) ((a) < (b)+LBADC_DELTA) && ((a) >  (b)-LBADC_DELTA)
extern BYTE LPM_CableStatus[8];
void UpdateCableStatus(CableInfo * CableUpdate)
{

   // DP1.2 HDMI DP1.1
   // LPM_CableStatus[4]: LPM LB ADC 2
   // LPM_CableStatus[6]: LPM LB ADC 3

   // DP get cable status by mission
   if (!(gm_ReadRegByte(LPM_CSM_HPD_PD_IN_STATUS) & PD1_IN_STATUS))
   {
      CableUpdate->PHY1_CableStatus = TRUE;
   }
   else
   {
      CableUpdate->PHY1_CableStatus = FALSE;
   }

   if (gm_ReadRegByte(LPM_CSM_HPD_PD_IN_STATUS) & PD2_IN_STATUS)
   {
      CableUpdate->PHY1_SourcePowerStatus = TRUE;
   }
   else
   {
      CableUpdate->PHY1_SourcePowerStatus = FALSE;
   }

   // HDMI
   if (LBADC_EQUAL(LPM_CableStatus[4],LBADC_DVI_CABLE) || LBADC_EQUAL(LPM_CableStatus[4],LBADC_DVI_VGA_CABLE))
      CableUpdate->PHY2_CableStatus = TRUE;
   else
      CableUpdate->PHY2_CableStatus = FALSE;

   CableUpdate->PHY2_SourcePowerStatus = 0;

   // DP 1.1
   if (!(gm_ReadRegByte(LPM_CSM_HPD_PD_IN_STATUS) & PD3_IN_STATUS))
   {
      CableUpdate->PHY3_CableStatus = TRUE;
   }
   else
   {
      CableUpdate->PHY3_CableStatus = FALSE;
   }

   if (gm_ReadRegByte(LPM_CSM_HPD_PD_IN_STATUS) & PD4_IN_STATUS)
   {
      CableUpdate->PHY3_SourcePowerStatus = TRUE;
   }
   else
   {
      CableUpdate->PHY3_SourcePowerStatus = FALSE;
   }

   // VGA
   if (LBADC_EQUAL(LPM_CableStatus[4],LBADC_VGA_CABLE) || LBADC_EQUAL(LPM_CableStatus[4],LBADC_DVI_VGA_CABLE))
      CableUpdate->VGA_CableStatus = TRUE;
   else
      CableUpdate->VGA_CableStatus = FALSE;

   CableUpdate->VGA_SourcePowerStatus = 0;

}


void LPM_GPIO_Config(void)
{
#if 1
   //=============================================================
   // EV1 Board LPM GPIO Config
   // TYPE :
   //		00: Special function
   //		01: GPIO_IN
   //		10: GPIO_OUT
   //		11: Not Use pin
   //=============================================================
   //LPM GPIO control, shared PBus
   // GPIO0(Bit0): ST sensor INT 1  GPIO1: VGA DDC SDA   GPIO2: VGA DDC SCL GPIO3: ST power module ctrl
   // GPIO4: CEC  GPIO5: CP2 DP Cable det GPIO6: CP0 DP Power det
   // GPIO7: CP0 cable det GPIO8: CP2 DP HPD GPIO9: CP1 HDMI HPD
   // GPIO10: CP0 DP HPD   GPIO11: Not use   GPIO12: Not use
   // GPIO13: CP1 HDMI DDC SDA   GPIO14: CP1 HDMI DDC SCL
   // GPIO15(bit0): LPM master I2C SCL // GPIO16: LPM master I2C SDA
   // GPIO17: DP TX HPD in GPIO18: Mission power ctr
   // GPIO19: Not use      GPIO20: LED Amber
   //=============================================================

   // 00: Special function 01: GPIO_IN 10:GPIO_OUT 11: Not Use pin
   //GPIO0 - GPIO3   
   gm_WriteRegWord(MISSION_LPM_DATA_OUT_1, 0x00);
   //GPIO4 - GPIO7   
   gm_WriteRegWord(MISSION_LPM_DATA_OUT_2, 0x00);
   //GPIO8 - GPIO11  
   gm_WriteRegWord(MISSION_LPM_DATA_OUT_3, 0x00);
   //GPIO12 - GPIO15 
   gm_WriteRegWord(MISSION_LPM_DATA_OUT_4, 0x00);
   //GPIO16 - GPIO19
   gm_WriteRegWord(MISSION_LPM_DATA_OUT_5, 0xC0);
   //GPIO20    
   gm_WriteRegWord(MISSION_LPM_DATA_OUT_6, 0x02);
   //GPADC0 - GPADC3
   gm_WriteRegWord(MISSION_LPM_DATA_OUT_7, 0x03);

   //LPM_GPADC control
   // ADC_AIN0: Keypad_in1 ADC_AIN2: Keypad2 ADC_AIN3: VGA / DVI cable det
   // ADC_AIN4: Not use

   //Power Key deifinition ADC channel (0 - 3)
   gm_WriteRegWord(MISSION_LPM_DATA_OUT_8, 0x00|(LPM_FORCE_POWER_DOWN<<8));
   //Power Key deifinition ADC value (0-255)
   gm_WriteRegWord(MISSION_LPM_DATA_OUT_9, 0x26);
   //ST touch sensor deifinition SLV channel (1 - 4) 0: No Touch sensor
   gm_WriteRegWord(MISSION_LPM_DATA_OUT_10, 0x01); //SLV1
   //ST touch sensor deifinition INT GPIO (0 - 20)
   gm_WriteRegWord(MISSION_LPM_DATA_OUT_11, 0x00); //GPIO0

   //Send board info to LPM directly
   gm_WriteRegWord(MISSION_LPM_DATA_OUT_12, GetCurrentBoardInfo());

   gm_WriteRegWord(MISSION_LPM_DATA_OUT_13, GetIrPowerKeyVal(Key_KEY_PHYSICAL_POWER));

   // Send Product ID and Revision 
   gm_WriteRegWord(MISSION_LPM_DATA_OUT_14, gm_ReadRegWord(PRODUCT_ID));
   gm_WriteRegWord(MISSION_LPM_DATA_OUT_15, gm_ReadRegWord(PRODUCT_REV));

   //Finally, fill IPC message ID to the first WORD to be recognized by LPM!!
   gm_WriteRegWord(MISSION_LPM_DATA_OUT_0, gmd_MESSAGE_LPM_GPIO_PINOUT);
         
#else
   BYTE LPMGPIO_Config[13];

   //=============================================================
   // EV1 Board LPM GPIO Config
   // TYPE :
   //		00: Special function
   //		01: GPIO_IN
   //		10: GPIO_OUT
   //		11: Not Use pin
   //=============================================================

   //GPIO3 - GPIO0	==>	00 00 00 00=>0x40
   //GPIO0:ST touch sensor device INT_IN
   //GPIO1:LPM UART UDO or SLV4 SDA
   //GPIO2:LPM UART UDI or SLV4 SCL
   //GPIO3:CP2 AUX_N HOST POWER DETECT
   LPMGPIO_Config[0] = 0x00;

   //GPIO7 - GPIO4	==>	00 00 00 00=>0x00
   //GPIO4:CEC
   //GPIO5:CP2 AUX_P CABLE DETECT
   //GPIO6:CP0 AUX_N HOST POWER DETECT
   //GPIO7:CP0 AUX_P CABLE DETECT
   LPMGPIO_Config[1] = 0x00;

   //GPIO11 - GPIO8	  ==> 11 00 00 00 ==>0xc0
   //GPIO08:HPD3 set to 00 due to PBus own is Mission
   //GPIO09:HPD2 set to 00 due to PBus own is Mission
   //GPIO10:HPD1 set to 00 due to PBus own is Mission
   //GPIO11:SLV3 SCL
   LPMGPIO_Config[2] = 0x00;

   //GPIO15 - GPIO12  ==>00 00 00 00 ==>0x00
   //GPIO12:SLV3_SDA
   //GPIO13:SLV2_SCL
   //GPIO14:SLV2_SDA
   //GPIO15:LPM Master I2C SCL or SLV1 SCL
   LPMGPIO_Config[3] = 0x00;

   //GPIO19 - GPIO16 ==>11 00 00 00 ==>0xc0
   //GPIO16:LPM Master I2C SDA or SLV1 SDA
   //GPIO17:DPTX HPD
   //GPIO18:MAIN POWER ON
   //GPIO19:NO Used
   LPMGPIO_Config[4] = 0xc0;

   //GPIO20  ==>	00 00 00 10 ==> 0x02
   //GPIO20:LED Amber
   LPMGPIO_Config[5] = 0x02;

   //How many gpADC channel use  ==> 00 00 00 11 ==>0x03
   //GPADC0 : Keypad
   //GPADC1 : Keypad
   //GPADC2 : HDMI & VGA Cable detect
   //GPADC3 : No Used
   LPMGPIO_Config[6] = 0x03;

   //Power Key deifinition ADC channel (0 - 3)
   LPMGPIO_Config[7] = 0x00;

   //Power Key deifinition ADC value (0-255)
   LPMGPIO_Config[8] = 0x26;

   //ST touch sensor deifinition SLV channel (1 - 4) 0: No Touch sensor
   LPMGPIO_Config[9] = 0x01;	// SLV1

   //ST touch sensor deifinition INT GPIO (0 - 20)
   LPMGPIO_Config[10] = 0x00;	// GPIO0

   //Send board info to LPM directly
   LPMGPIO_Config[11] = GetCurrentBoardInfo();

   LPMGPIO_Config[12] = GetIrPowerKeyVal(Key_KEY_PHYSICAL_POWER);

   gm_MessageSend(gmd_MESSAGE_LPM_GPIO_PINOUT, &LPMGPIO_Config[0], 13);
#endif   
}

#if (FEATURE_DISPLAY_PORT == ENABLE) && ENABLE_HPD_OVERRIDED
void ForceDpHpd2Low(gmt_DPRX_DID b_DID, BOOL b_Set2Low)
{
   if (b_DID==DID_DPRX0)
   {
      if (b_Set2Low==TRUE)
         gm_SetRegBitsByte(LPM_CSM_HPD_OUT_CTRL,HPD1_SOURCE_SEL);
      else
         gm_ClearAndSetBitsByte(LPM_CSM_HPD_OUT_CTRL,HPD1_SOURCE_SEL,0x1);
   }
   else if (b_DID==DID_DPRX1)
   {
      if (b_Set2Low==TRUE)
         gm_SetRegBitsByte(LPM_CSM_HPD_OUT_CTRL,HPD3_SOURCE_SEL);
      else
         gm_ClearAndSetBitsWord(LPM_CSM_HPD_OUT_CTRL,HPD3_SOURCE_SEL,0x100);
   }
}
#endif

/*********************************  END  **************************************/
