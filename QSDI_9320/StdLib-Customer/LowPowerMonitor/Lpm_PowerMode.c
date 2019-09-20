/*
	$Workfile:   Lpm_PowerMode.c  $
	$Revision: 1.91 $
	$Date	 :     $
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
//=============================================================================
//
// MODULE:      Lpm_PowerMode.c
//
// USAGE:       
//
//******************************************************************************

//******************************************************************************
//
//  I N C L U D E    F I L E S
//
//******************************************************************************
#include ".\LowPowerMonitor\Lpm_All.h"
#include <mem.h>


gmt_Event2PwrHdr Event2PwrHdr = LPM_EVNT2PWR_NONE;
BYTE LastLpmPwrStatus = LPM_PowerOffMode;
BYTE LpmPwrState = LPM_NormalPowerMode;
#if LPM_FORCE_POWER_DOWN
BYTE IsDcOffEvent = FALSE;
BYTE IsForceDcOff = FALSE;
#endif

#ifdef POWER_MODE_HANDLE

//******************************************************************************
//
//  C O D E
//
//******************************************************************************
void Lpm_PowerModeInit(void)
{
	LpmPwrState = LPM_NormalPowerMode;
	Event2PwrHdr = LPM_EVNT2PWR_NONE;
	LastLpmPwrStatus = LPM_PowerOffMode;	
}

void Lpm_SetPowerKeyEvent(void)
{
	Event2PwrHdr |= LPM_EVNT2PWR_PWRKEY;
   WakeupFrom = LPM_WAKEUP_KEYPAD;
}

#ifdef ANY_KEY_WAKEUP
void Lpm_SetAnyKeyWakeEvent(void)
{
   gm_Print("------ Got Key Wake Up Event!", 0);
	Event2PwrHdr |= LPM_EVNT2PWR_ANASYNC_DETECT;
   WakeupFrom = LPM_WAKEUP_KEYPAD;
}
#endif

BOOL Lpm_PowerModeStateChange(void)
{
   BOOL IsPwrStateChange = FALSE;

   
	if(Event2PwrHdr & LPM_EVNT2PWR_PWRKEY)
	{
		if(LpmPwrState == LPM_LowPowerMode) //LowPower => PowerOff
      {    
			LpmPwrState = LPM_PowerOffMode;
         IsPwrStateChange = TRUE;
      }
		else if(LpmPwrState == LPM_PowerOffMode)  //PowerOff => NormalPower
      {    
			LpmPwrState = LPM_NormalPowerMode;
         IsPwrStateChange = TRUE;
      }
		else if(LpmPwrState == LPM_NormalPowerMode)  //NormalPower => PowerOff
      {    
			LpmPwrState = LPM_PowerOffMode;	
         IsPwrStateChange = TRUE;
      }		
	}
	else if( Event2PwrHdr & (LPM_EVNT2PWR_SG_DETECT_MASK
                          | LPM_EVNT2PWR_HPD_DETECT
                          | LPM_EVNT2PWR_PD_DETECT) ) 
	{
		if (LpmPwrState == LPM_LowPowerMode)
		{
			LpmPwrState = LPM_NormalPowerMode;	
         IsPwrStateChange = TRUE;
		}    
	}
   else if(Event2PwrHdr & LPM_EVNT2PWR_SLEEP_NOTICE)
   {
      if (LpmPwrState == LPM_NormalPowerMode)
      {
			LpmPwrState = LPM_LowPowerMode;
         IsPwrStateChange = TRUE;
      }
   }
   else if(Event2PwrHdr & LPM_EVNT2PWR_POWER_ON_NOTICE)
   {
      if (LpmPwrState != LPM_NormalPowerMode)
      {
			LpmPwrState = LPM_NormalPowerMode;
         IsPwrStateChange = TRUE;
      }
   }

   Event2PwrHdr = LPM_EVNT2PWR_NONE;

   return IsPwrStateChange;
}

void Lpm_PowerModeHandler(void)
{

   WORD  Message[5];
   if (gm_MessageReceived(gmd_MESSAGE_POWER, &Message[0], sizeof(Message))  )
   {	
      SigDetPortNum = (BYTE)(Message[0]>>8);   
      //max sig detect port number: 1 ~ MAX_SIGDET_PORT_NUM
      if(SigDetPortNum > MAX_SIGDET_PORT_NUM)
         SigDetPortNum = MAX_SIGDET_PORT_NUM;
      else if(SigDetPortNum < 1)
         SigDetPortNum = 1;

      _fmemset(CurInputPort, 0xFF, MAX_SIGDET_PORT_NUM*2);
      _fmemcpy(CurInputPort, &Message[1], SigDetPortNum*2);
      Lpm_ExternalSwitchCheck();
      
      //system No sync or power key press 
      if ((Message[0] & 0xFF) == Pwr_Sleep)		//Got notice that Mission is going to sleep mode
      {
      gm_ClearRegBitsWord(LPM_GPOUTPUT_1, LPM_GPIO19_OUT);
         Event2PwrHdr |= LPM_EVNT2PWR_SLEEP_NOTICE;
         gm_Print("----Got Mission sleep command",0);
         gm_Print("----Sig Detect Port Number = %d", SigDetPortNum);
      }
      else if((Message[0] & 0xFF) == Pwr_Off)		//Got notice that Mission is going to power off mode  
      {
      gm_ClearRegBitsWord(LPM_GPOUTPUT_1, LPM_GPIO19_OUT);
         Event2PwrHdr |= LPM_EVNT2PWR_PWRKEY;
         #if LPM_FORCE_POWER_DOWN
         IsDcOffEvent = FALSE;
         #endif
         gm_Print("----Got Mission power off command",0);
      }
   }
/*
	// 20130204
	if(BoardType == BOARD_4K2K)
   {
   	if(ChipID4k2K == CHIPID_4K2K_FE)
   	{	
   		// Check Mission poer, into this routine when Mission lost power
 		
   		if(!IsMissionPowerON()) // Mission get power off
   		{
   			WORD  Message[5];
				
   			SigDetPortNum = MAX_SIGDET_PORT_NUM;
   			_fmemset(CurInputPort, 0xFF, MAX_SIGDET_PORT_NUM*2);
      		_fmemcpy(CurInputPort, &Message[1], SigDetPortNum*2);
      		Lpm_ExternalSwitchCheck();

				// Mission lost power by DC off event
	   		if(IsDCOffMode())	
	   		{
	   			if(LpmPwrState == LPM_NormalPowerMode)
					{
						gm_Print("DC OFF from ON ... ", 0);
						Event2PwrHdr |= LPM_EVNT2PWR_PWRKEY;
					}
					else if(LpmPwrState == LPM_LowPowerMode)
					{
						gm_Print("DC OFF from Sleep ... ", 0);
						Event2PwrHdr |= LPM_EVNT2PWR_PWRKEY;
					}
	   		}
				else	// Mission lost power by No Sync event
				{
					if((LpmPwrState == LPM_NormalPowerMode) && !(gm_ReadRegWord(LPM_GPOUTPUT_1) & LPM_GPIO19_OUT))
	   			{ 
	   				gm_Print("Go to Low Power ", 0);
	   				Event2PwrHdr |= LPM_EVNT2PWR_SLEEP_NOTICE;
	   			}
				}
   		}
			else // Mission get power on event
			{
				// DC on
				if(LpmPwrState == LPM_PowerOffMode)
				{
					Event2PwrHdr |= LPM_EVNT2PWR_PWRKEY;
					gm_Print("DC ON from Off ... ", 0);
				}
				else if(LpmPwrState == LPM_LowPowerMode) // Sync on
				{
					gm_Print("Wake up ... ", 0);
					Event2PwrHdr |= LPM_EVNT2PWR_POWER_ON_NOTICE;
				}

				// System power, clear power up request by FE side and clear reset overall bit for wake up
				if(gm_ReadRegWord(LPM_GPOUTPUT_1) & LPM_GPIO19_OUT)
				{
					gm_ClearRegBitsWord(LPM_GPOUTPUT_1, LPM_GPIO19_OUT);
					//gm_ClearRegBitsWord(LPM_MICRO_CONTROL, LPM_RST_OVR);  //Clear RST_OVR to avoid reset key can't work
				}
			} 
   	}

		if(ChipID4k2K == CHIPID_4K2K_R)
   	{
   		// Set GPIO 0 to "1", if R side press DC off during loe power mode
   		if(!IsMissionPowerON() && (LpmPwrState == LPM_PowerOffMode) && !(gm_ReadRegWord(LPM_GPOUTPUT_0) & GPIO0_OUT))
   		{
   			gm_SetRegBitsWord(LPM_GPOUTPUT_0, GPIO0_OUT);
   		}

			// Power up system, if got request from FE (HDMI port sync on)
   		if(LpmPwrState == LPM_LowPowerMode)
   		{
   			if(gm_ReadRegWord(LPM_GPINPUT_1) & LPM_GPIO19_IN)
   			{
   				//gm_Print("R got power on event from FE %d", 1);
					Event2PwrHdr |= LPM_EVNT2PWR_POWER_ON_NOTICE;
   			}
   		}
		}
	}
*/
   #if LPM_FORCE_POWER_DOWN
   if((IsDcOffEvent == TRUE) && Lpm_TimerTimeOut(PWROFF_1ms_TMR))
   {      
      Event2PwrHdr |= LPM_EVNT2PWR_PWRKEY;
      IsDcOffEvent = FALSE;
      gm_Print("----Wait Mission power off command timeout!",0);
   }
   #endif

   //===================================================
   //   get LPM power state by event 
   //===================================================
   //Place Power related events process here
   //New Power state will be set according to the received events
   if(Event2PwrHdr == LPM_EVNT2PWR_NONE)
      return;
   else  //Power event occurs
   {
      if(Lpm_PowerModeStateChange() == FALSE)
         return;
   }

   switch (LpmPwrState)
   {
      case 	LPM_NormalPowerMode:		//LPM OCM clock is TCLK 27MHz from Mission
      {  
         Lpm_RedLedCtrl(0);
         //Disable i2c slave till mission i2c slave init to avoid stretching out (some source can't handle stretching)
         //DO NOT disable I2C slave1 when Lpm I2C master is used!!
         gm_ClearRegBitsWord(I2C_SLAVE2_CTRL1, I2C_SLAVE2_EN);
         gm_ClearRegBitsWord(I2C_SLAVE3_CTRL1, I2C_SLAVE3_EN);
         gm_ClearRegBitsWord(I2C_SLAVE4_CTRL1, I2C_SLAVE4_EN);

         //-- MISSION_MICRO_PWR_DOWN is gating both LPM_WARM_RESET and LPM_RST_OVR
         gm_ClearRegBitsWord(LPM_MICRO_CONTROL, MISSION_MICRO_PWR_DOWN); 
         Lpm_WarmReset(1);  //Start Warm Reset
         gm_ClearRegBitsWord(LPM_PD_CONTROL,MAIN_PSU_SHDN);	//Power up Mission		
         Lpm_Delay1ms(WAIT_MAIN_PWR_STABLE_DELAY);   
         
         Lpm_PowerUpLpmDevices();
         Lpm_SharedPBusSwitch(BUS_LPM);         
         Lpm_SignalDetectStop(); 
         Lpm_PowerModeCsmMuxSel(LPM_NormalPowerMode);
         if(!((BoardType==BOARD_4K2K) && (ChipID4k2K != CHIPID_4K2K_R)))
         {
            Lpm_NvramWriteByte(NVRAM_OFFSET_PWR_MODE, PWR_SW_ON);
         }
          
         //Lpm_HpdControl(LPM_NormalPowerMode);
         Lpm_WarmReset(0);  //Clear Warm Reset                   

         Lpm_SharedPBusSwitch(BUS_MISSION);     //Mission get back PBus Own 
         PBusReqLock = FALSE;
         Lpm_Delay1ms(WAIT_TCLK_STABLE_DELAY);
         //Lpm_Delay1ms(50);

         #if (LPM_OCMCLK_DEFAULT != 0)
         Lpm_ClockSwitch2TCLK();  
         #else
         Lpm_ClockSwitch2RCOSC(TCLK_DIV_0);
         #endif  
         gm_RegMsgInit();         
         Lpm_TmdsSwitcherControl(LPM_NormalPowerMode);

         //------------ Main Handlers Switching ------------
         #ifdef I2C_SLAVE_SUPPORT
         Lpm_MainHandlersTurn(LpmHdrID_I2cSlave, FALSE);
         #endif                                                   
         #ifdef DDCCI_SUPPORT
         Lpm_MainHandlersTurn(LpmHdrID_DDC, FALSE);
         #endif
         #ifdef TCLK_TIMER_SUPPORT
         Lpm_MainHandlersTurn(LpmHdrID_TclkTimer, TRUE);
         #endif
         #ifdef SIGNALING_DETECT
         Lpm_MainHandlersTurn(LpmHdrID_SgDeTect, FALSE);  
         #endif
         gm_Print("****LPM_NormalPowerMode",0);

      }
      break;

      case 	LPM_LowPowerMode:	//LPM OCM clock is from RCOSC with divider 4. (~6.75Mhz)
      {        
         Lpm_RedLedCtrl(1);
         Lpm_ClockSwitch2RCOSC(TCLK_DIV_2);

         Lpm_SharedPBusSwitch(BUS_LPM);		//LPM get back PBus Own 
         PBusReqLock = TRUE;
         Lpm_ResetLpmDevices(BUS_LPM);     //Reset due to LPM clock is changed to RC-OSC.

         Lpm_HpdControl(LPM_LowPowerMode);           
         Lpm_PowerModeCsmMuxSel(LPM_LowPowerMode);       

         Lpm_PowerDownLpmDevices(LPM_LowPowerMode);           
         Lpm_Delay1ms(1);
         Lpm_PowerDownMission();
         Lpm_Delay1ms(1);

         Lpm_SignalDetectInit();         

         //------------ Main Handlers Switching ------------
         #ifdef I2C_SLAVE_SUPPORT
         Lpm_MainHandlersTurn(LpmHdrID_I2cSlave, TRUE);
         #endif				
         #ifdef DDCCI_SUPPORT
         Lpm_MainHandlersTurn(LpmHdrID_DDC, TRUE);
         #endif
         #ifdef TCLK_TIMER_SUPPORT
         Lpm_MainHandlersTurn(LpmHdrID_TclkTimer, TRUE);
         #endif
         #ifdef SIGNALING_DETECT
         Lpm_MainHandlersTurn(LpmHdrID_SgDeTect, TRUE);
         #endif
         gm_Print("****LPM_LowPowerMode",0);
      }
      break;

      case 	LPM_PowerOffMode: //LPM OCM clock is from RCOSC with divider 4. (~6.75Mhz)
      {
         Lpm_RedLedCtrl(0);
         Lpm_HpdControl(LPM_PowerOffMode);
         Lpm_ForceDpHpdLow(); //Avoid DPTx reading EDID in power off mode
         if(!((BoardType==BOARD_4K2K) && (ChipID4k2K != CHIPID_4K2K_R)))
         {
            Lpm_NvramWriteByte(NVRAM_OFFSET_PWR_MODE, PWR_SW_OFF);
         }
         Lpm_ClockSwitch2RCOSC(TCLK_DIV_2);
       
         Lpm_SharedPBusSwitch(BUS_LPM);   //LPM get back PBus Own 
         PBusReqLock = TRUE;
         Lpm_ResetLpmDevices(BUS_LPM);     //Reset due to LPM clock is changed to RC-OSC.
         
         Lpm_PowerModeCsmMuxSel(LPM_PowerOffMode);
         Lpm_PowerDownLpmDevices(LPM_PowerOffMode); 
         Lpm_TmdsSwitcherControl(LPM_PowerOffMode);
         Lpm_Delay1ms(1);
         Lpm_PowerDownMission();

         #ifdef SIGNALING_DETECT
         Lpm_SignalDetectStop();         
         #endif          

         //------------ Main Handlers Switching ------------
         #ifdef I2C_SLAVE_SUPPORT
         Lpm_MainHandlersTurn(LpmHdrID_I2cSlave, TRUE);
         #endif				
         #ifdef DDCCI_SUPPORT
         Lpm_MainHandlersTurn(LpmHdrID_DDC, TRUE);
         #endif
         #ifdef TCLK_TIMER_SUPPORT
         Lpm_MainHandlersTurn(LpmHdrID_TclkTimer, TRUE);
         #endif
         #ifdef SIGNALING_DETECT
         Lpm_MainHandlersTurn(LpmHdrID_SgDeTect, FALSE);
         #endif
         gm_Print("****LPM_PowerOffMode",0);
      }
      break;			
   }
}

void Lpm_PowerModeSetEvent2PwrHdr(gmt_Event2PwrHdr SetEvent)
{
	Event2PwrHdr |= SetEvent;
}

BYTE Lpm_PowerModeGetState(void)
{
	return LpmPwrState;
}

//******************************************************************************
// FUNCTION:   void Lpm_PowerModeCsmMuxSel(BYTE PwrMode)
// USAGE:      Select CSM Mux for each power mode
// INPUT:      None
// OUTPUT:     None
// GLOBALS:    None
// USED_REGS:  LPM_CSM_XX_CTRL
//******************************************************************************
void Lpm_PowerModeCsmMuxSel(BYTE PwrMode)
{
   if(PwrMode == LPM_NormalPowerMode)
   {
      //Recovery the CSM Mux control values
      gm_WriteRegWord(LPM_CSM_I2C_MUX_CTRL, CsmI2cMuxCtrlBackup);
      gm_WriteRegWord(LPM_CSM_HPD_OUT_CTRL, CsmHpdOutCtrlBackup);
      gm_WriteRegWord(LPM_CSM_HPD_PD_IN_CTRL, CsmHpdPdInCtrlBackup);
      gm_WriteRegWord(LPM_CSM_HPD_PD_IN_TRIG, 0xFFFF);
   }
   else if((PwrMode == LPM_LowPowerMode) || (PwrMode == LPM_PowerOffMode))
   {
      CsmI2cMuxCtrlBackup = gm_ReadRegWord(LPM_CSM_I2C_MUX_CTRL);
      CsmHpdOutCtrlBackup = gm_ReadRegWord(LPM_CSM_HPD_OUT_CTRL);
      CsmHpdPdInCtrlBackup = gm_ReadRegWord(LPM_CSM_HPD_PD_IN_CTRL);

      //for HDMI, DVI ports. the CSM setting is no impact for VGA port
      Lpm_CsmI2CMuxSelect(MUX_DDC2_LpmI2cSlv2_MisHdmiCp); //default
      //Disconnect Mission HDMI/DVI device and no longer impact SDA/SCL lines
      //Avoid Graphic card read HDMI/DVI EDID fails in LPM mode
      gm_ClearRegBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_DVI_I2C_MUX_SEL|LPM_HDMI_I2C_MUX_SEL);

      if(Lpm_IsWakeupPort(Port_DP_A))
      {
         #if (LPM_EDID_ACCESS_SEL == 0)
         gm_SetRegBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_AUX_SEL);
         #elif(LPM_EDID_ACCESS_SEL == 1)
         Lpm_CsmI2CMuxSelect(MUX_DDC1_LpmDpC0Aux2I2c_ExtDev);
         #elif(LPM_EDID_ACCESS_SEL == 2)
         Lpm_CsmI2CMuxSelect(MUX_DDC1_LpmDpC0Aux2I2c_I2cSlv1);
         #endif
      }
      if(Lpm_IsWakeupPort(Port_DP_B))
      {
         #if (LPM_EDID_ACCESS_SEL == 0)
         gm_SetRegBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_AUX_SEL);
         #elif(LPM_EDID_ACCESS_SEL == 1)
         Lpm_CsmI2CMuxSelect(MUX_DDC3_LpmDpC2Aux2I2c_ExtDev);
         #elif(LPM_EDID_ACCESS_SEL == 2)
         Lpm_CsmI2CMuxSelect(MUX_DDC3_LpmDpC2Aux2I2c_I2cSlv3);
         #endif
      }      
   }
}

//******************************************************************************
// FUNCTION:   void Lpm_PowerDownLpmDevices(void)
// USAGE:      Power Down LPM device as entering Low power or Power off mode
// INPUT:      None
// OUTPUT:     None
// GLOBALS:    None
// USED_REGS:  LPM_PD_CONTROL
//******************************************************************************
void Lpm_PowerDownLpmDevices(BYTE PwrMode)
{
   if(PwrMode == LPM_LowPowerMode)
   {
      gm_SetRegBitsWord(LPM_SPARE1, LPM2C0_RXL0_HDMI_DET_CLR|LPM2C1_RXL0_HDMI_DET_CLR
         |LPM2C2_RXL0_HDMI_DET_CLR|LPM_RXAUX_RXPD_CPHY1); //clear for TMDS sensitive noise issue
      Lpm_Delay1ms(1);
      
      if((ProductRev & 0xFF) == 0x01)  //cut2.0
      {
      //Cut2 issue...LPM2Cx_PD is inverted. Must keep CPHY0,2 power on for DP wakeup detection!
      if( Lpm_IsWakeupPort(Port_DP_A))
         gm_SetRegBitsWord(LPM_PD_CONTROL,LPM2C1_PD|LPM2C2_PD|LPM2AFE_PD|LPM_SP1_PD);
      else if(Lpm_IsWakeupPort(Port_DP_B))
         gm_SetRegBitsWord(LPM_PD_CONTROL,LPM2C0_PD|LPM2C1_PD|LPM2AFE_PD|LPM_SP1_PD);
      else
         gm_SetRegBitsWord(LPM_PD_CONTROL,LPM2C0_PD|LPM2C1_PD|LPM2C2_PD|LPM2AFE_PD|LPM_SP1_PD);
      }
      else  
      {
         gm_SetRegBitsWord(LPM_PD_CONTROL,LPM2C0_PD|LPM2C1_PD|LPM2C2_PD|LPM2AFE_PD|LPM_SP1_PD);
      }
         
      //gm_SetRegBitsWord(LPM_COMBO0_CONTROL, LPM2C0_RXAUX_TXPD);
      //gm_SetRegBitsWord(LPM_COMBO2_CONTROL, LPM2C2_RXAUX_TXPD); 
      gm_SetRegBitsWord(LPM_AFE_CTRL,LPM_AFE_EN_ST_ASE|LPM_AFE_EN_ST_ASE_HS); 
   }
   else if(PwrMode == LPM_PowerOffMode)
   {
      BYTE RxTerm[CPhyNum_Num] = {0xF, 0xF, 0xF};
      if(CPhyType[CPhyNum_0] == CPhyType_DP)
         RxTerm[CPhyNum_0] = 0xD;
      if(CPhyType[CPhyNum_2] == CPhyType_DP)
         RxTerm[CPhyNum_2] = 0xD;     
      
      gm_SetRegBitsWord(LPM_PD_CONTROL,LPM2C0_PD|LPM2C1_PD|LPM2C2_PD|LPM2AFE_PD|LPM_SP1_PD);
      gm_WriteRegWord(LPM_COMBO0_CONTROL, LPM2C0_RTERM_ENB|(RxTerm[CPhyNum_0]<<LPM2C0_RX_RTERM_SHIFT));//Adjust RTERM to avoid receiving error Aux data         
      gm_WriteRegWord(LPM_COMBO1_CONTROL, LPM2C1_RTERM_ENB|(RxTerm[CPhyNum_1]<<LPM2C1_RX_RTERM_SHIFT));
      gm_WriteRegWord(LPM_COMBO2_CONTROL, LPM2C2_RTERM_ENB|(RxTerm[CPhyNum_2]<<LPM2C2_RX_RTERM_SHIFT)); 
      gm_WriteRegWord(LPM_AFE_CTRL, 0);

      //Disabled to support Lpm Aux EDID reply
      //gm_ClearRegBitsWord(LPM_C0_AUX_CONTROL, DPRX_AUX_CLK_EN);
      //gm_ClearRegBitsWord(LPM_C2_AUX_CONTROL, DPRX_AUX_CLK_EN);   
      //gm_SetRegBitsWord(LPM_C0_AUX_CONTROL, DPRX_AUX_POWER_DOWN);
      //gm_SetRegBitsWord(LPM_C2_AUX_CONTROL, DPRX_AUX_POWER_DOWN);      
   }
}

//******************************************************************************
// FUNCTION:   void Lpm_PowerUpLpmDevices(void)
// USAGE:      Power Up LPM device as entering Normal power mode
// INPUT:      None
// OUTPUT:     None
// GLOBALS:    None
// USED_REGS:  LPM_PD_CONTROL
//******************************************************************************
void Lpm_PowerUpLpmDevices(void)
{
   #if 0
   gm_ClearRegBitsWord(LPM_PD_CONTROL,LPM2C0_PD|LPM2C1_PD|LPM2C2_PD|LPM2AFE_PD|LPM_SP1_PD);
   //gm_ClearRegBitsWord(LPM_COMBO0_CONTROL, LPM2C0_RXAUX_TXPD);
   //gm_ClearRegBitsWord(LPM_COMBO2_CONTROL, LPM2C2_RXAUX_TXPD); 
   gm_WriteRegWord(LPM_AFE_CTRL, 0);
   gm_SetRegBitsWord(LPM_C0_AUX_CONTROL, DPRX_AUX_CLK_EN);
   gm_SetRegBitsWord(LPM_C2_AUX_CONTROL, DPRX_AUX_CLK_EN);
   gm_ClearRegBitsWord(LPM_C0_AUX_CONTROL, DPRX_AUX_POWER_DOWN);      
   gm_ClearRegBitsWord(LPM_C2_AUX_CONTROL, DPRX_AUX_POWER_DOWN);  
   #else
   gm_ClearAndSetBitsWord(LPM_C0_AUX_CONTROL, DPRX_AUX_POWER_DOWN, DPRX_AUX_CLK_EN);
   gm_ClearAndSetBitsWord(LPM_C2_AUX_CONTROL, DPRX_AUX_POWER_DOWN, DPRX_AUX_CLK_EN);   
   gm_WriteRegWord(LPM_COMBO0_CONTROL, 0);
   gm_WriteRegWord(LPM_COMBO1_CONTROL, 0);
   gm_WriteRegWord(LPM_COMBO2_CONTROL, 0);
   gm_WriteRegWord(LPM_ASYNC_DET_CTRL, 0); 
   gm_WriteRegWord(LPM_AFE_CTRL, 0);
   gm_WriteRegWord(CPHY_TMDSCK_DET_CTRL, 0);
   gm_WriteRegWord(LPM_SPARE1, 0);
   gm_WriteRegWord(LPM_PD_CONTROL, 0);   
   #endif
}

void Lpm_WarmReset(BYTE IsEnable)
{
   if(IsEnable)
   {
      //Only reset Mission side, LPM code will not be loaded!
      gm_SetRegBitsWord(LPM_MICRO_CONTROL, LPM_WARM_RESET);
      gm_SetRegBitsWord(LPM_MICRO_CONTROL, LPM_RST_OVR);
   }
   else
   {
      //gm_ClearRegBitsWord(LPM_MICRO_CONTROL, LPM_RST_OVR);
      gm_ClearRegBitsWord(LPM_MICRO_CONTROL, LPM_WARM_RESET);
   }
}

void Lpm_HpdControl(BYTE PwrMode)
{
   //Change HPD pin to GPIO before entering sleep mode to fix sometimes can't wakeup issue
   //For power off mode, HPD should also be kept in high for source to read HDCP...
   if ((PwrMode == LPM_LowPowerMode) || (PwrMode == LPM_PowerOffMode))
   {
      if((BoardType == BOARD_RD1) || (BoardType == BOARD_RD2))
      {  
         //Change DP HPD to LPM_GPIO to maintain HPD level
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO10_IO);         
         gm_SetRegBitsWord(LPM_GPOUTPUT_0, LPM_GPIO10_OUT);
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO10_EN);

         //Change HDMI/DVI HPD to LPM_GPIO to maintain HPD level
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO8_IO|LPM_GPIO9_IO);
         gm_SetRegBitsWord(LPM_GPOUTPUT_0, LPM_GPIO8_OUT|LPM_GPIO9_OUT);
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO8_EN|LPM_GPIO9_EN);
      }
      else if(BoardType == BOARD_RD3)
      {  
         //Change DP HPD to LPM_GPIO to maintain HPD level
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO10_IO);         
         gm_SetRegBitsWord(LPM_GPOUTPUT_0, LPM_GPIO10_OUT);
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO10_EN);

         //Change HDMI/DVI HPD to LPM_GPIO to maintain HPD level
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO8_IO|LPM_GPIO9_IO);
         gm_ClearRegBitsWord(LPM_GPOUTPUT_0, LPM_GPIO8_OUT|LPM_GPIO9_OUT); //Invert for EV3
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO8_EN|LPM_GPIO9_EN);
      }      
      else if(BoardType == BOARD_EV1)
      {
         //Change DP HPD to LPM_GPIO to maintain HPD level
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO10_IO);         
         gm_SetRegBitsWord(LPM_GPOUTPUT_0, LPM_GPIO10_OUT);
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO10_EN);
         #ifdef AUX_RX_C2_SUPPORT
         //Keep DP_B HPD as high in LPM mode
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO8_IO);         
         gm_SetRegBitsWord(LPM_GPOUTPUT_0, LPM_GPIO8_OUT);
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO8_EN);          
         #endif
         
         //Change HDMI/DVI HPD to LPM_GPIO to maintain HPD level
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO9_IO);
         gm_SetRegBitsWord(LPM_GPOUTPUT_0, LPM_GPIO9_OUT);
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO9_EN);
      } 
      else if(BoardType == BOARD_EV2)
      {  
         //Change HDMI/DVI HPD to LPM_GPIO to maintain HPD level
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO8_IO|LPM_GPIO9_IO|LPM_GPIO10_IO);
         gm_SetRegBitsWord(LPM_GPOUTPUT_0, LPM_GPIO8_OUT|LPM_GPIO9_OUT|LPM_GPIO10_OUT);
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO8_EN|LPM_GPIO9_EN|LPM_GPIO10_EN);
      }
      else if(BoardType == BOARD_EV3)
      {
         //Change DP HPD to LPM_GPIO to maintain HPD level
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO10_IO);         
         gm_SetRegBitsWord(LPM_GPOUTPUT_0, LPM_GPIO10_OUT);
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO10_EN);
         
         //Change Dual-DVI HPD to LPM_GPIO to maintain HPD level
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO9_IO);
         gm_ClearRegBitsWord(LPM_GPOUTPUT_0, LPM_GPIO9_OUT); //Invert for EV3
         gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO9_EN);
      }       
   }

   //For Normal Power mode...HPD pin recovery in InitSystemGpio() by Mission to avoid HPD level drop!!

}

void Lpm_ForceDpHpdLow(void)
{
   //Force HPD to low after detecting Aux signaling to avoid DPTx continue LT or Aux transactions
   if((BoardType == BOARD_RD1) || (BoardType == BOARD_RD2)
      ||(BoardType == BOARD_RD3) || (BoardType == BOARD_EV3))
   {
      gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO10_IO);         
      gm_ClearRegBitsWord(LPM_GPOUTPUT_0, LPM_GPIO10_OUT);
      gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO10_EN);
   }
   else if(BoardType == BOARD_EV1)
   {
      gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO10_IO);         
      gm_ClearRegBitsWord(LPM_GPOUTPUT_0, LPM_GPIO10_OUT);
      gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO10_EN);
      #ifdef AUX_RX_C2_SUPPORT
      gm_SetRegBitsWord(LPM_GPIO_DIRCTRL1, LPM_GPIO8_IO);         
      gm_ClearRegBitsWord(LPM_GPOUTPUT_0, LPM_GPIO8_OUT);
      gm_SetRegBitsWord(LPM_GPIO_ENABLE1, LPM_GPIO8_EN);          
      #endif
   }  
   //No DP port in BOARD_EV2
}

//******************************************************************************
// FUNCTION:   void Lpm_PowerDownMission(void)
// USAGE:      Power Down Mission as entering Low power or Power off mode
// INPUT:      None
// OUTPUT:     None
// GLOBALS:    None
// USED_REGS:  LPM_MICRO_CONTROL, LPM_PD_CONTROL
//******************************************************************************
void Lpm_PowerDownMission(void)
{
   //gm_SetRegBitsWord(LPM_MICRO_CONTROL, LPM_WARM_RESET|LPM_RST_OVR);         
   gm_SetRegBitsWord(LPM_MICRO_CONTROL, MISSION_MICRO_PWR_DOWN);
   gm_SetRegBitsWord(LPM_PD_CONTROL, MAIN_PSU_SHDN);   //Power down Mission
}

//******************************************************************************
// FUNCTION:   void Lpm_TmdsSwitcherControl(BYTE PwrMode)
// USAGE:      Control TMDS switcher as entering Normal power or Power off mode
// INPUT:      None
// OUTPUT:     None
// GLOBALS:    None
// USED_REGS:  
//******************************************************************************
void Lpm_TmdsSwitcherControl(BYTE PwrMode)
{
   //HDMI switch TMDS_EN (1:Power on/0:Power off)
   if(PwrMode == LPM_PowerOffMode)
   {
      if(BoardType == BOARD_RD3)
         gm_ClearRegBitsWord(LPM_GPOUTPUT_0,LPM_GPIO5_OUT);
      else if(BoardType == BOARD_RD1)
         gm_ClearRegBitsWord(LPM_GPADC_GPOUTPUT,GPADC_GPIO3_OUT);
   }
   else
   {
      if(BoardType == BOARD_RD3) 
         gm_SetRegBitsWord(LPM_GPOUTPUT_0,LPM_GPIO5_OUT);
      else if(BoardType == BOARD_RD1)
         gm_SetRegBitsWord(LPM_GPADC_GPOUTPUT,GPADC_GPIO3_OUT);
   }   
}

// 20130204
BOOL IsMissionPowerON(void)
{
	return gm_ReadRegWord(LPM_MICRO_STATUS) & MISSION_POR;
}

BOOL IsDCOffMode(void)
{
	return gm_ReadRegWord(LPM_GPINPUT_0) & GPIO0_IN;
}
#endif   //#ifdef POWER_MODE_HANDLE
