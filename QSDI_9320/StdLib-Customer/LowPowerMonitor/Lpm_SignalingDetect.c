/*
	$Workfile:   Lpm_SignalingDetect.c  $
	$Revision: 1.60 $
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
// MODULE:      Lpm_SignalingDetect.c
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


#ifdef SIGNALING_DETECT

extern gmt_CPhyType CPhyType[];
extern gmt_Event2PwrHdr Event2PwrHdr;
extern BYTE IsDpSrcPwrWakeup[DPRX_NUM];
extern BYTE IsHdmiDviSrcPwrWakeup[CPhyNum_Num];
extern BYTE IsVgaSrcPwrWakeup;


BYTE ASDECT_State = LPM_ASDET_STOP; 


//******************************************************************************
//
//  C O D E
//
//******************************************************************************
void Lpm_SignalDetectInit(void)
{

   //------ Init for Analog Sync control
   if( Lpm_IsWakeupPort(Port_VGA) )
   {
      ASDECT_State = LPM_ASDET_SS_SETUP;
      //Fix for VGA post screen: Polling VGA SrcPwr for wake up to shorten display time
      if(Lpm_IsVGASrcPwrValid() == FALSE)  //only RD3 check VGA SrcPwr, other board returns TRUE 
      {
         IsVgaSrcPwrWakeup = TRUE;
         //gm_Print("VGA Source Power Off",0);
      }
   }

   //------ Init for TMDS Clock signaling control
   gm_WriteRegWord(CPHY_TMDSCK_DET_CTRL, LPM_CLK_MS_MSK|LPM_CLK_MS_EN
                                        |(CLK_MS_REF_TCLK_DIV32<<LPM_CLK_MS_REF_DIV_SHIFT));

   //------ Init for Aux Autowake IRQ
   gm_WriteRegDWord(LPM_C0_AUX_IRQ_STATUS, DPRX_AUX2OCM_REQ_RECEPTN_STS|DPRX_AUX2I2C_REQ_RECEPTN_STS|DPRX_AUX_AUTO_WAKE_STS);
   gm_ClearRegBitsWord(LPM_C0_AUX_IRQ_CTRL, DPRX_AUX2OCM_REQ_IRQ_EN|DPRX_AUX2I2C_REQ_IRQ_EN);
   #ifdef AUX_RX_C2_SUPPORT 
   gm_WriteRegDWord(LPM_C2_AUX_IRQ_STATUS, DPRX_AUX2OCM_REQ_RECEPTN_STS|DPRX_AUX2I2C_REQ_RECEPTN_STS|DPRX_AUX_AUTO_WAKE_STS);
   gm_ClearRegBitsWord(LPM_C2_AUX_IRQ_CTRL, DPRX_AUX2OCM_REQ_IRQ_EN|DPRX_AUX2I2C_REQ_IRQ_EN);   
   #endif

   //Lpm_InterruptEnable(gmd_CSM_HPD_EDPD);
   Lpm_InterruptEnable(gmd_AUTOWAKE);

   if( Lpm_IsWakeupPort(Port_HDMI) || Lpm_IsWakeupPort(Port_DVI) )
   {
      if( Lpm_IsWakeupTmdsCPhy(CPhyNum_0) )
      {         
         Lpm_TmdsClkDetectInit(CPhyNum_0);
         //Fix for Hdmi/Dvi post screen: Polling Hdmi/Dvi SrcPwr for wake up to shorten display time
         if(Lpm_IsHdmiDviSrcPwrValid(CPhyNum_0) == FALSE) //only RD3 check Hdmi/Dvi SrcPwr, other board returns TRUE          
         {
            IsHdmiDviSrcPwrWakeup[CPhyNum_0]= TRUE;
            //gm_Print("HDMI/DVI (CPHY0) Source Power Off",0);
         }  
      }
      if( Lpm_IsWakeupTmdsCPhy(CPhyNum_1) )
      {      
         Lpm_TmdsClkDetectInit(CPhyNum_1);
         //Fix for Hdmi/Dvi post screen: Polling Hdmi/Dvi SrcPwr for wake up to shorten display time
         if(Lpm_IsHdmiDviSrcPwrValid(CPhyNum_1) == FALSE) //only RD3 check Hdmi/Dvi SrcPwr, other board returns TRUE 
         {
            IsHdmiDviSrcPwrWakeup[CPhyNum_1] = TRUE;
            //gm_Print("HDMI/DVI (CPHY1) Source Power Off",0);
         }
      }   
      if( Lpm_IsWakeupTmdsCPhy(CPhyNum_2) )
      {   
         Lpm_TmdsClkDetectInit(CPhyNum_2);
         if(Lpm_IsHdmiDviSrcPwrValid(CPhyNum_2) == FALSE) //only RD3 check Hdmi/Dvi SrcPwr, other board returns TRUE 
         //Fix for Hdmi/Dvi post screen: Polling Hdmi/Dvi SrcPwr for wake up to shorten display time
         {
            IsHdmiDviSrcPwrWakeup[CPhyNum_2] = TRUE;
            //gm_Print("HDMI/DVI (CPHY2) Source Power Off",0);
         }        
      }
      
   #ifdef TMDS_CLKDET_BY_AUTOWAKE_IRQ
   if(Lpm_IsWakeupPort(Port_HDMI))  
      Lpm_AutoWakePortSetup(Port_HDMI);
   else if(Lpm_IsWakeupPort(Port_DVI)) 
      Lpm_AutoWakePortSetup(Port_DVI);      
   #endif      
   }

   //gm_WriteRegByte(CPHY_TMDSCK_DET_RESULT, 0xFF);//clear TMDS detect results; no need for cut2   

   //------ Init for DP Aux signaling control
   if(Lpm_IsWakeupPort(Port_DP_A)) 
   { 
      Lpm_AutoWakePortSetup(Port_DP_A);
      //Fix for DP post screen: Polling DP SrcPwr for wake up to shorten display time
      if(Lpm_IsDPRxSrcPwrValid(DPRX_C0) == FALSE)
         IsDpSrcPwrWakeup[DPRX_C0] = TRUE;
   }
   #ifdef AUX_RX_C2_SUPPORT 
   if(Lpm_IsWakeupPort(Port_DP_B)) 
   {      
      Lpm_AutoWakePortSetup(Port_DP_B);
      //Fix for DP post screen: Polling DP SrcPwr for wake up to shorten display time
      if(Lpm_IsDPRxSrcPwrValid(DPRX_C2) == FALSE) 
         IsDpSrcPwrWakeup[DPRX_C2] = TRUE;
   }
   #endif
  
}

void Lpm_SignalDetectStop(void)
{
   ASDECT_State = LPM_ASDET_STOP;   
   
   gm_WriteRegWord(CPHY_TMDSCK_DET_CTRL, 0);

   if(Lpm_PowerModeGetState() == LPM_NormalPowerMode)   
   {
      gm_WriteRegWord(LPM_COMBO0_CONTROL, 0);
      gm_WriteRegWord(LPM_COMBO1_CONTROL, 0);
      gm_WriteRegWord(LPM_COMBO2_CONTROL, 0);   
      gm_WriteRegWord(LPM_AUTOWAKE_CTRL, 0);
      gm_ClearRegBitsWord(LPM_C0_AUX_CONTROL, DPRX_AUX_AUTO_WAKE_ADDR);
      gm_ClearRegBitsWord(LPM_C2_AUX_CONTROL, DPRX_AUX_AUTO_WAKE_ADDR); 
      gm_ClearRegBitsWord(LPM_C0_AUX_IRQ_CTRL, DPRX_AUX2OCM_REQ_IRQ_EN|DPRX_AUX2I2C_REQ_IRQ_EN);
      
      Lpm_InterruptDisable(gmd_AUTOWAKE);        
   }
   else  //LPM_PowerOffMode
   {   
      //Lpm_InterruptDisable(gmd_AUTOWAKE);
      
      //------ Init for DP Aux for I2C over Aux event
      //if(Lpm_IsWakeupPort(Port_DP_A)) 
      {
         Lpm_AutoWakePortSetup(Port_DP_A);
         Lpm_InterruptEnable(gmd_AUTOWAKE);          
      }     
      #ifdef AUX_RX_C2_SUPPORT 
      //if(Lpm_IsWakeupPort(Port_DP_B)) 
      {
         Lpm_AutoWakePortSetup(Port_DP_B);
         Lpm_InterruptEnable(gmd_AUTOWAKE);          
      }
      #endif              
   } 

   //Lpm_InterruptDisable(gmd_CSM_HPD_EDPD);    
   Lpm_InterruptDisable(gmd_TMDSCLK);   
}

void Lpm_SignalDetectHandler(void)
{
   #ifdef VGA_SRC_PWR_WAKEUP
   if(IsVgaSrcPwrWakeup == TRUE)
   {   
      if(Lpm_IsVGACablePlugged() && Lpm_IsVGASrcPwrValid())
      {
         if(Lpm_IsWakeupPort(Port_VGA))
         {
            Lpm_ClockSwitch2RCOSC(TCLK_DIV_0);  //Raise OCM clock to 27MHz 
            IsVgaSrcPwrWakeup = FALSE;
            IsSyncDetected = TRUE;
            WakeupFrom = LPM_WAKEUP_ASDET;
            Event2PwrHdr |= LPM_EVNT2PWR_ANASYNC_DETECT;  
            //gm_Print("VGA Src Power Detected!",0);
         }
      }
   }
   #endif

   //------ Analog Sync detection ------
   if(Lpm_IsVGACablePlugged())
   {
      if(Lpm_AnalogSyncDetect() == LPM_EVNT2PWR_ANASYNC_DETECT)
      {
         Lpm_ClockSwitch2RCOSC(TCLK_DIV_0);  //Raise OCM clock to 27MHz 
         ASDECT_State = LPM_ASDET_STOP;
         Event2PwrHdr |= LPM_EVNT2PWR_ANASYNC_DETECT;  
      }   
   }
      
   #ifndef TMDS_CLKDET_BY_AUTOWAKE_IRQ
   #ifdef HDMI_DVI_SRC_PWR_WAKEUP
   if(IsHdmiDviSrcPwrWakeup[CPhyNum_0] == TRUE)
   {
      if( Lpm_IsWakeupPort(Port_HDMI) || Lpm_IsWakeupPort(Port_DVI) )
      {
         if( Lpm_IsWakeupTmdsCPhy(CPhyNum_0) )
         {
            if(Lpm_IsCPhyCablePlugged(CPhyNum_0) && Lpm_IsCPhySrcPwrValid(CPhyNum_0))
            {
               Lpm_ClockSwitch2RCOSC(TCLK_DIV_0);  //Raise OCM clock to 27MHz 
               IsHdmiDviSrcPwrWakeup[CPhyNum_0] = FALSE;
               IsSyncDetected = TRUE;
               WakeupFrom = LPM_WAKEUP_CPHY0;
               Event2PwrHdr |= LPM_EVNT2PWR_TMDS_DETECT; 
               //gm_Print("CPHY0 Src Power Detected!",0);
            }
         }
      }
   }
   else if(IsHdmiDviSrcPwrWakeup[CPhyNum_1] == TRUE)
   {
      if( Lpm_IsWakeupPort(Port_HDMI) || Lpm_IsWakeupPort(Port_DVI) )
      {   
         if( Lpm_IsWakeupTmdsCPhy(CPhyNum_1) )
         {
            if(Lpm_IsCPhyCablePlugged(CPhyNum_1) && Lpm_IsCPhySrcPwrValid(CPhyNum_1))
            {         
               Lpm_ClockSwitch2RCOSC(TCLK_DIV_0);  //Raise OCM clock to 27MHz 
               IsHdmiDviSrcPwrWakeup[CPhyNum_1] = FALSE;
               IsSyncDetected = TRUE;
               WakeupFrom = LPM_WAKEUP_CPHY1;
               Event2PwrHdr |= LPM_EVNT2PWR_TMDS_DETECT; 
               //gm_Print("CPHY1 Src Power Detected!",0);
            }
         } 
      }
   }   
   else if(IsHdmiDviSrcPwrWakeup[CPhyNum_2] == TRUE)
   {  
      if( Lpm_IsWakeupPort(Port_HDMI) || Lpm_IsWakeupPort(Port_DVI) )
      {    
         if( Lpm_IsWakeupTmdsCPhy(CPhyNum_2) )
         {
            if(Lpm_IsCPhyCablePlugged(CPhyNum_2) && Lpm_IsCPhySrcPwrValid(CPhyNum_2))
            {         
               Lpm_ClockSwitch2RCOSC(TCLK_DIV_0);  //Raise OCM clock to 27MHz 
               IsHdmiDviSrcPwrWakeup[CPhyNum_2] = FALSE;
               IsSyncDetected = TRUE;
               WakeupFrom = LPM_WAKEUP_CPHY2;
               Event2PwrHdr |= LPM_EVNT2PWR_TMDS_DETECT; 
               //gm_Print("CPHY2 Src Power Detected!",0);
            }
         }         
      }
   }
   #endif
     
   //------ HDMI,DVI Sync detection ------
   if(Lpm_DigitalTmdsClkDetect() == LPM_EVNT2PWR_TMDS_DETECT)
   {
      Lpm_ClockSwitch2RCOSC(TCLK_DIV_0);  //Raise OCM clock to 27MHz 
      gm_WriteRegWord(LPM_IRQ_STATUS, LPM_TMDSCK_IRQ);
      Event2PwrHdr |= LPM_EVNT2PWR_TMDS_DETECT;  
   }
   #endif

   #ifdef DP_SRC_PWR_WAKEUP
   //Fix for DP post screen: Polling DP SrcPwr for wake up to shorten display time
   if(IsDpSrcPwrWakeup[DPRX_C0] == TRUE)
   {
      if(Lpm_IsDPRxCablePlugged(DPRX_C0) && Lpm_IsDPRxSrcPwrValid(DPRX_C0))
      {
         if(Lpm_IsWakeupPort(Port_DP_A))
         {
            Lpm_ClockSwitch2RCOSC(TCLK_DIV_0);  //Raise OCM clock to 27MHz 
            IsDpSrcPwrWakeup[DPRX_C0] = FALSE;
            IsSyncDetected = TRUE;
            WakeupFrom = LPM_WAKEUP_CPHY0;
            Event2PwrHdr |= LPM_EVNT2PWR_AUXWAKE_DETECT; 
            //gm_Print("DP Src Power Detected!",0);
         }
      }
   }
   #ifdef AUX_RX_C2_SUPPORT
   else if(IsDpSrcPwrWakeup[DPRX_C2] == TRUE)
   {
      if(Lpm_IsDPRxCablePlugged(DPRX_C2) && Lpm_IsDPRxSrcPwrValid(DPRX_C2))
      {
         if(Lpm_IsWakeupPort(Port_DP_B))
         {      
            Lpm_ClockSwitch2RCOSC(TCLK_DIV_0);  //Raise OCM clock to 27MHz 
            IsDpSrcPwrWakeup[DPRX_C2] = FALSE;
            IsSyncDetected = TRUE;
            WakeupFrom = LPM_WAKEUP_CPHY2;
            Event2PwrHdr |= LPM_EVNT2PWR_AUXWAKE_DETECT;    
         }
      }
   }
   #endif
   #endif
}

void Lpm_TmdsClkDetectInit(gmt_CPhyNum CPhyNum)
{
   #define CPHY_RX_RTERM      0x5  //0xF //The value for LPM2Cx_RX_RTERM

   if(CPhyNum == CPhyNum_0)
   {
      gm_WriteRegWord(LPM_COMBO0_CONTROL, LPM2C0_OVR_SMP|(CPHY_RX_RTERM<<LPM2C0_RX_RTERM_SHIFT));
      //gm_SetRegBitsWord(LPM_COMBO0_CONTROL, LPM2C0_RTERM_ENB);//Disable CPhy Termination         
      gm_ClearRegBitsWord(LPM_SPARE1, LPM2C0_RXL0_HDMI_DET_CLR
         |(TMDS_DET_THRES << LPM2C0_HDMI_CLK_DET_THRES_SHIFT));
      gm_SetRegBitsWord(LPM_COMBO0_CONTROL, LPM2C0_CLK_OUT_EN);
   }
   else if(CPhyNum == CPhyNum_1)
   {
      gm_WriteRegWord(LPM_COMBO1_CONTROL, LPM2C1_OVR_SMP|(CPHY_RX_RTERM<<LPM2C1_RX_RTERM_SHIFT));
      //gm_SetRegBitsWord(LPM_COMBO1_CONTROL, LPM2C1_RTERM_ENB);//Disable CPhy Termination         
      gm_ClearRegBitsWord(LPM_SPARE1, LPM2C1_RXL0_HDMI_DET_CLR
         |(TMDS_DET_THRES << LPM2C1_HDMI_CLK_DET_THRES_SHIFT));         
      gm_SetRegBitsWord(LPM_COMBO1_CONTROL, LPM2C1_CLK_OUT_EN);
   }
   else if(CPhyNum == CPhyNum_2)
   {
      gm_WriteRegWord(LPM_COMBO2_CONTROL, LPM2C2_OVR_SMP|(CPHY_RX_RTERM<<LPM2C2_RX_RTERM_SHIFT));
      //gm_SetRegBitsWord(LPM_COMBO2_CONTROL, LPM2C2_RTERM_ENB);//Disable CPhy Termination         
      gm_ClearRegBitsWord(LPM_SPARE1, LPM2C2_RXL0_HDMI_DET_CLR
         |(TMDS_DET_THRES << LPM2C2_HDMI_CLK_DET_THRES_SHIFT));         
      gm_SetRegBitsWord(LPM_COMBO2_CONTROL, LPM2C2_CLK_OUT_EN);  
   }   
}

gmt_Event2PwrHdr Lpm_DigitalTmdsClkDetect(void)
{
   gmt_Event2PwrHdr PortStatus = LPM_EVNT2PWR_NONE;

   if( Lpm_IsWakeupPort(Port_HDMI) || Lpm_IsWakeupPort(Port_DVI) )
   {
      //------------------------ CPHY 0 ------------------------
      if( Lpm_IsWakeupTmdsCPhy(CPhyNum_0) )         
      {
         if( Lpm_IsHdmiDviCablePlugged(CPhyNum_0) )
         {
         gm_ClearRegBitsWord(CPHY_TMDSCK_DET_CTRL, LPM_CLK_MS_SEL);    
         if (gm_ReadRegByte(CPHY_TMDSCK_DET_RESULT) & CPHY0_TMDSCK_STATUS)
         //if (gm_ReadRegByte(CPHY_TMDSCK_DET_RESULT)&(CPHY0_TMDSCK_STATUS|LPM_CPHY_LCLK_PRSNT))
         {
            gm_WriteRegByte(CPHY_TMDSCK_DET_RESULT, CPHY0_TMDSCK_STATUS|LPM_CPHY_LCLK_PRSNT);
            PortStatus = LPM_EVNT2PWR_TMDS_DETECT;
            IsSyncDetected = TRUE;
            WakeupFrom = LPM_WAKEUP_CPHY0;

				// 02130204, FE detect TMDS clk, send power up request to R side
				if((BoardType == BOARD_4K2K) && (ChipID4k2K == CHIPID_4K2K_FE))
				{
					gm_SetRegBitsWord(LPM_GPOUTPUT_1, LPM_GPIO19_OUT);
				}
            gm_Print(" CPhy0 - TMDS detect !!",0);
         }
         }
      }
      //------------------------ CPHY 1 ------------------------ 
      if( Lpm_IsWakeupTmdsCPhy(CPhyNum_1) )    
      {
         if( Lpm_IsHdmiDviCablePlugged(CPhyNum_1) )
         {      
         gm_ClearAndSetBitsWord(CPHY_TMDSCK_DET_CTRL, LPM_CLK_MS_SEL, 1<<LPM_CLK_MS_SEL_SHIFT);  
         if (gm_ReadRegByte(CPHY_TMDSCK_DET_RESULT) & CPHY1_TMDSCK_STATUS)
         //if (gm_ReadRegByte(CPHY_TMDSCK_DET_RESULT)&(CPHY1_TMDSCK_STATUS|LPM_CPHY_LCLK_PRSNT))
         {
              gm_SetRegBitsWord(LPM_SPARE1, LPM2C1_RXL0_HDMI_DET_CLR);
              Lpm_Delay1ms(1);
              gm_ClearRegBitsWord(LPM_SPARE1, LPM2C1_RXL0_HDMI_DET_CLR);
              Lpm_Delay1ms(1);
              if (Lpm_IsHdmiDviCablePlugged(CPhyNum_1) && (gm_ReadRegByte(CPHY_TMDSCK_DET_RESULT) & CPHY1_TMDSCK_STATUS))
              {
              gm_WriteRegByte(CPHY_TMDSCK_DET_RESULT, CPHY1_TMDSCK_STATUS|LPM_CPHY_LCLK_PRSNT);
              PortStatus = LPM_EVNT2PWR_TMDS_DETECT;  
              IsSyncDetected = TRUE;
              WakeupFrom = LPM_WAKEUP_CPHY1;
              gm_Print(" CPhy1 - TMDS detect !!",0);
              }	
         }
         }
      }
      //------------------------ CPHY 2 ------------------------
      if( Lpm_IsWakeupTmdsCPhy(CPhyNum_2) )        
      {
         if( Lpm_IsHdmiDviCablePlugged(CPhyNum_2) )
         {      
         gm_ClearAndSetBitsWord(CPHY_TMDSCK_DET_CTRL, LPM_CLK_MS_SEL, 2<<LPM_CLK_MS_SEL_SHIFT);        
         if (gm_ReadRegByte(CPHY_TMDSCK_DET_RESULT) & CPHY2_TMDSCK_STATUS)
         //if (gm_ReadRegByte(CPHY_TMDSCK_DET_RESULT)&(CPHY2_TMDSCK_STATUS|LPM_CPHY_LCLK_PRSNT))
         {
            gm_WriteRegByte(CPHY_TMDSCK_DET_RESULT, CPHY2_TMDSCK_STATUS|LPM_CPHY_LCLK_PRSNT);
            PortStatus = LPM_EVNT2PWR_TMDS_DETECT;  
            IsSyncDetected = TRUE;
            WakeupFrom = LPM_WAKEUP_CPHY2;
            gm_Print(" CPhy2 - TMDS detect !!",0);
         }
      }
   }
   }
   return PortStatus;
}


//check Analog sync by main loop
// 1 main loop about 6~7ms 
gmt_Event2PwrHdr Lpm_AnalogSyncDetect(void)
{
   gmt_Event2PwrHdr PortStatus = LPM_EVNT2PWR_NONE;
   WORD Status;

   if (ASDECT_State == LPM_ASDET_STOP)	//No need to check if state not trigger 
      return PortStatus;

   if (Lpm_IsWakeupPort(Port_VGA)==FALSE)
      return PortStatus;

   //gm_Print("---ASDECT_State %d",ASDECT_State);	
   switch ( ASDECT_State )
   {
      case LPM_ASDET_CS_SETUP:
         gm_WriteRegWord(LPM_ASYNC_DET_CTRL, 0x0000);	//reset ASDET0~ASDET3's  result 
         gm_WriteRegWord(LPM_ASYNC_CLK_CTRL, 0x3003);//0xC003 //reset ASDET0~ASDET3's  result 	  		
         //ASDET0 -SS_HS 		
         gm_WriteRegWord(LPM_ASDET0_COUNT_MAX, 0xEC);		
         gm_WriteRegWord(LPM_ASDET0_COUNT_MIN, 0x1E);			
         gm_WriteRegWord(LPM_ASDET0_SUCCESS_COUNT, 0x08);	
         //ASDET3 -CS_VS 
         gm_WriteRegWord(LPM_CSYNC_INT_TH, 0x28A);				
         gm_WriteRegWord(LPM_ASDET3_COUNT_MAX, 0xA5);//0xA5		
         gm_WriteRegWord(LPM_ASDET3_COUNT_MIN, 0x30);//0x42
         gm_WriteRegWord(LPM_ASDET3_SUCCESS_COUNT, 0x08);//0x04	
         gm_WriteRegWord(LPM_ASYNC_DET_CTRL, 0x109); //0x8109	
         //Lpm_Delay1ms(100);		//Wait for Integrator extract VS from Csync  

         ASDECT_State = LPM_ASDET_CS_CHECK; 
         Lpm_TimerStart(ASYNC_1ms_TMR, CSYNC_CHECK_TIME);
         break;

      case LPM_ASDET_CS_CHECK:
         if(Lpm_TimerTimeOut(ASYNC_1ms_TMR) == FALSE)
         {  
            Status = gm_ReadRegByte(LPM_ASYNC_COUNT_RESULT)&(LPM_ASDET0_RES|LPM_ASDET3_RES);
            if(Status == (LPM_ASDET0_RES|LPM_ASDET3_RES))
            {
               gm_Print("---CSync OK",0);
               PortStatus = LPM_EVNT2PWR_ANASYNC_DETECT;
               IsSyncDetected = TRUE;
               WakeupFrom = LPM_WAKEUP_ASDET;
               ASDECT_State = LPM_ASDET_CS_SETUP;
            }
         }
         else
         {
            ASDECT_State = LPM_ASDET_SS_SETUP;
         }
         break;

      case LPM_ASDET_SS_SETUP:
         gm_WriteRegWord(LPM_ASYNC_DET_CTRL, 0x0000);	//reset ASDET0~ASDET3's  result 
         gm_WriteRegWord(LPM_ASYNC_CLK_CTRL, 0x00C3);	//reset ASDET0~ASDET3's  result 	  	
         gm_WriteRegWord(LPM_CSYNC_INT_TH, 0x0);		  		
         //ASDET0 -SS_HS 		
         gm_WriteRegWord(LPM_ASDET0_COUNT_MAX, 0xEC);		
         gm_WriteRegWord(LPM_ASDET0_COUNT_MIN, 0x1E);			
         gm_WriteRegWord(LPM_ASDET0_SUCCESS_COUNT, 0x08);	
         //ASDET1 -SS_VS 
         gm_WriteRegWord(LPM_ASDET1_COUNT_MAX, 0xA5);
         gm_WriteRegWord(LPM_ASDET1_COUNT_MIN, 0x30);
         gm_WriteRegWord(LPM_ASDET1_SUCCESS_COUNT, 0x08);
         gm_WriteRegWord(LPM_ASYNC_DET_CTRL, 0x0003);	
		
         ASDECT_State = LPM_ASDET_SS_CHECK;         
         Lpm_TimerStart(ASYNC_1ms_TMR, SSYNC_CHECK_TIME);
         break;

      case LPM_ASDET_SS_CHECK:
         if(Lpm_TimerTimeOut(ASYNC_1ms_TMR) == FALSE)
         {  
            Status = gm_ReadRegByte(LPM_ASYNC_COUNT_RESULT)&(LPM_ASDET0_RES|LPM_ASDET1_RES);
            if(Status == (LPM_ASDET0_RES|LPM_ASDET1_RES))
            {
               gm_Print("---SSync OK",0);
               PortStatus = LPM_EVNT2PWR_ANASYNC_DETECT;
               IsSyncDetected = TRUE;
               WakeupFrom = LPM_WAKEUP_ASDET;
               ASDECT_State = LPM_ASDET_SS_SETUP;
            }
         }
         else
         {
            ASDECT_State = LPM_ASDET_SOG_SETUP;
         }
         break;

      case LPM_ASDET_SOG_SETUP:
         gm_WriteRegWord(LPM_ASYNC_DET_CTRL, 0x0000);	//reset ASDET0~ASDET3's result 
         gm_WriteRegWord(LPM_ASYNC_CLK_CTRL, 0x30C0);//0x3040	   		
         //ASDET1 -SOG_VS 
         gm_WriteRegWord(LPM_CSYNC_INT_TH, 0x28A);
         gm_WriteRegWord(LPM_ASDET1_COUNT_MAX, 0xFF);    // Chnage to 0xFF to support Vsync 24Hz
         gm_WriteRegWord(LPM_ASDET1_COUNT_MIN, 0x30);
         gm_WriteRegWord(LPM_ASDET1_SUCCESS_COUNT, 0x08);	
         //ASDET3 -SOG - HS 
         gm_WriteRegWord(LPM_ASDET3_COUNT_MAX, 0xEC);		
         gm_WriteRegWord(LPM_ASDET3_COUNT_MIN, 0x1E);
         gm_WriteRegWord(LPM_ASDET3_SUCCESS_COUNT, 0x08);	
         gm_WriteRegWord(LPM_ASYNC_DET_CTRL, 0x41AF);

         ASDECT_State = LPM_ASDET_SOG_CHECK;        
         Lpm_TimerStart(ASYNC_1ms_TMR, SOG_CHECK_TIME);         
         break;			

      case LPM_ASDET_SOG_CHECK:
         if(Lpm_TimerTimeOut(ASYNC_1ms_TMR) == FALSE)
         {  
            Status = gm_ReadRegByte(LPM_ASYNC_COUNT_RESULT)&(LPM_ASDET1_RES|LPM_ASDET3_RES);
            if(Status == (LPM_ASDET1_RES|LPM_ASDET3_RES))
            {
               gm_Print("---SOG OK",0);
               PortStatus = LPM_EVNT2PWR_ANASYNC_DETECT;
               IsSyncDetected = TRUE;
               WakeupFrom = LPM_WAKEUP_ASDET;
               ASDECT_State = LPM_ASDET_SOG_SETUP;
            }
         }
         else
         {
            ASDECT_State = LPM_ASDET_SOY_SETUP;
         }
         break;

      case LPM_ASDET_SOY_SETUP:
         gm_WriteRegWord(LPM_ASYNC_DET_CTRL, 0x0000);	//reset ASDET0~ASDET3's result 
         gm_WriteRegWord(LPM_ASYNC_CLK_CTRL, 0x3300); //0xC300	   		
         //ASDET2 -SOY_HS 
         gm_WriteRegWord(LPM_ASDET2_COUNT_MAX, 0xEC);		
         gm_WriteRegWord(LPM_ASDET2_COUNT_MIN, 0x1E);
         gm_WriteRegWord(LPM_ASDET2_SUCCESS_COUNT, 0x08);	
         //ASDET3 -SOY_VS 
         gm_WriteRegWord(LPM_CSYNC_INT_TH, 0x28A);			
         gm_WriteRegWord(LPM_ASDET3_COUNT_MAX, 0xA5);//0xA5		
         gm_WriteRegWord(LPM_ASDET3_COUNT_MIN, 0x30);//0x42
         gm_WriteRegWord(LPM_ASDET3_SUCCESS_COUNT, 0x08);	
         gm_WriteRegWord(LPM_ASYNC_DET_CTRL, 0xC14C);

         ASDECT_State = LPM_ASDET_SOY_CHECK;        
         Lpm_TimerStart(ASYNC_1ms_TMR, SOY_CHECK_TIME); 
         break;			

      case LPM_ASDET_SOY_CHECK:
         if(Lpm_TimerTimeOut(ASYNC_1ms_TMR) == FALSE)
         {  
            Status = gm_ReadRegByte(LPM_ASYNC_COUNT_RESULT)&(LPM_ASDET2_RES|LPM_ASDET3_RES);
            if(Status == (LPM_ASDET2_RES|LPM_ASDET3_RES))
            {
               gm_Print("---SOY OK",0);
               PortStatus = LPM_EVNT2PWR_ANASYNC_DETECT;
               IsSyncDetected = TRUE;
               WakeupFrom = LPM_WAKEUP_ASDET;
               ASDECT_State = LPM_ASDET_SOY_SETUP;
            }
         }
         else
         {
            ASDECT_State = LPM_ASDET_CS_SETUP;
         }
         break;			

      default:
         PortStatus = LPM_EVNT2PWR_NONE;
         break;			
   }
   
   return PortStatus;
}

void Lpm_AutoWakePortSetup(gmt_PortChannel B_Port)
{
   if(B_Port == Port_DP_A)
   {
      if(Lpm_PowerModeGetState() == LPM_LowPowerMode)
      {
         //gm_SetRegBitsWord(LPM_AUTOWAKE_CTRL, LPM_AUX_TCLK_EN);
         gm_SetRegBitsWord(LPM_AUTOWAKE_CTRL, AUTOWAKE_CH_CPHY0<<LPM_AUTOWAKE_AUX_TRG_SHIFT);
         gm_SetRegBitsWord(LPM_AUTOWAKE_CTRL, AUTOWAKE_CH_CPHY0<<AUTOWAKE_AUX2OCM_TRG_SHIFT);
         gm_WriteRegWord(LPM_AUTOWAKE_STATUS, PD_DETECT_AUTOWAKE_STATUS|HPD_IN_DETECT_AUTOWAKE_STATUS);
         //Access any DPCD address triggers AUTOWAKE
         gm_SetRegBitsWord(LPM_C0_AUX_CONTROL, DPRX_AUX_AUTO_WAKE_ADDR);
         //To do :  Board define setting on Host Power detection  
         //DPRX0 cable detect, source power detect, HPD
         // bit[6:0] = pins {HPD3, HPD2, HPD1, EDPD4, EDPD3, EDPD2, EDPD1}
         //if(DpRxDetPins[0])
         //   gm_SetRegBitsWord(LPM_AUTOWAKE_CTRL, (WORD)(DpRxDetPins[0]&0xF)<<AUTOWAKE_PD1_TRG_SHIFT);
      }
      gm_SetRegBitsWord(LPM_C0_AUX_STROBE, DPRX_AUX_AUTO_WAKEUP_EN);
   }
   #ifdef AUX_RX_C2_SUPPORT    
   else if(B_Port == Port_DP_B)
   {
      if(Lpm_PowerModeGetState() == LPM_LowPowerMode)
      {
         //gm_SetRegBitsWord(LPM_AUTOWAKE_CTRL, LPM_AUX_TCLK_EN);			
         gm_SetRegBitsWord(LPM_AUTOWAKE_CTRL, AUTOWAKE_CH_CPHY2<<LPM_AUTOWAKE_AUX_TRG_SHIFT);
         gm_SetRegBitsWord(LPM_AUTOWAKE_CTRL, AUTOWAKE_CH_CPHY2<<AUTOWAKE_AUX2OCM_TRG_SHIFT);
         gm_WriteRegWord(LPM_AUTOWAKE_STATUS, PD_DETECT_AUTOWAKE_STATUS|HPD_IN_DETECT_AUTOWAKE_STATUS);
         //Access any DPCD address triggers AUTOWAKE
         gm_SetRegBitsWord(LPM_C2_AUX_CONTROL, DPRX_AUX_AUTO_WAKE_ADDR);
         //To do :  Board define setting on Host Power detection  
         //DPRX2 cable detect, source power detect, HPD
         // bit[6:0] = pins {HPD3, HPD2, HPD1, EDPD4, EDPD3, EDPD2, EDPD1}
         //if(DpRxDetPins[1])
         //   gm_SetRegBitsWord(LPM_AUTOWAKE_CTRL, (WORD)(DpRxDetPins[1]&0xF)<<AUTOWAKE_PD1_TRG_SHIFT);
      }
      gm_SetRegBitsWord(LPM_C2_AUX_STROBE, DPRX_AUX_AUTO_WAKEUP_EN);
   }
   #endif
   #ifdef TMDS_CLKDET_BY_AUTOWAKE_IRQ
   else if( (B_Port == Port_HDMI) || (B_Port == Port_DVI) )
   {
      if(Lpm_PowerModeGetState() == LPM_LowPowerMode)
      {
         gm_SetRegBitsWord(LPM_AUTOWAKE_CTRL, TMDS_AUTOWAKE_CKDET_TRG);
         gm_WriteRegWord(LPM_AUTOWAKE_STATUS, TMDS_CKDET_AUTOWAKE_STATUS); 
         Lpm_InterruptEnable(gmd_TMDSCLK); 
      }    
   }
   #endif
   #if 0
   else if( B_Port == Port_VGA )
   {
      gm_SetRegBitsWord(LPM_AUTOWAKE_CTRL, ANA_SYNCDET_AUTOWAKE_TRG);
      gm_WriteRegWord(LPM_AUTOWAKE_STATUS, ANA_SYNCDET_AUTOWAKE_STATUS); 
   }
   #endif
}

void far Lpm_AutoWakeISR(gmt_PortChannel B_Port)
{
   if( (B_Port == Port_DP_A) && Lpm_IsWakeupPort(Port_DP_A) )
   {
      Lpm_ClockSwitch2RCOSC(TCLK_DIV_0);  //Raise OCM clock to 27MHz 
      IsSyncDetected = TRUE;
      WakeupFrom = LPM_WAKEUP_CPHY0;
      Event2PwrHdr |= LPM_EVNT2PWR_AUXWAKE_DETECT;
   }
   else if( (B_Port == Port_DP_B) && Lpm_IsWakeupPort(Port_DP_B) ) 
   {
      Lpm_ClockSwitch2RCOSC(TCLK_DIV_0);  //Raise OCM clock to 27MHz 
      IsSyncDetected = TRUE;
      WakeupFrom = LPM_WAKEUP_CPHY2;
      Event2PwrHdr |= LPM_EVNT2PWR_AUXWAKE_DETECT;
   }
   #ifdef TMDS_CLKDET_BY_AUTOWAKE_IRQ
   else if( (B_Port == Port_HDMI) || (B_Port == Port_DVI) )
   {
      if(Lpm_DigitalTmdsClkDetect() == LPM_EVNT2PWR_TMDS_DETECT)
      {
         Lpm_ClockSwitch2RCOSC(TCLK_DIV_0);  //Raise OCM clock to 27MHz 
         Event2PwrHdr |= LPM_EVNT2PWR_TMDS_DETECT;  
      }
   }
   #endif
}

BYTE Lpm_IsWakeupPort(gmt_PortChannel B_Port)
{
   BYTE Status = FALSE;

   if(Lpm_IsAllPortWakeup())
      return TRUE;
   
   switch(B_Port)
   {
      case Port_DVI:
         if(  ((CurInputPort[0]>=ALL_BOARDS_DVI1)&&(CurInputPort[0]<=ALL_BOARDS_DVI2)&&(CurInputPort[1]<CPhyNum_Num)) 
            ||((CurInputPort[2]>=ALL_BOARDS_DVI1)&&(CurInputPort[2]<=ALL_BOARDS_DVI2)&&(CurInputPort[3]<CPhyNum_Num))
            ||((CurInputPort[4]>=ALL_BOARDS_DVI1)&&(CurInputPort[4]<=ALL_BOARDS_DVI2)&&(CurInputPort[5]<CPhyNum_Num))
            ||((CurInputPort[6]>=ALL_BOARDS_DVI1)&&(CurInputPort[6]<=ALL_BOARDS_DVI2)&&(CurInputPort[7]<CPhyNum_Num)) ) 
            Status = TRUE;
         break;
      case Port_HDMI:
         if(  ((CurInputPort[0]>=ALL_BOARDS_HDMI)&&(CurInputPort[0]<=ALL_BOARDS_HDMI3)&&(CurInputPort[1]<CPhyNum_Num))
            ||((CurInputPort[2]>=ALL_BOARDS_HDMI)&&(CurInputPort[2]<=ALL_BOARDS_HDMI3)&&(CurInputPort[3]<CPhyNum_Num))
            ||((CurInputPort[4]>=ALL_BOARDS_HDMI)&&(CurInputPort[4]<=ALL_BOARDS_HDMI3)&&(CurInputPort[5]<CPhyNum_Num))
            ||((CurInputPort[6]>=ALL_BOARDS_HDMI)&&(CurInputPort[6]<=ALL_BOARDS_HDMI3)&&(CurInputPort[7]<CPhyNum_Num)) ) 
            Status = TRUE;            
         break;

      case Port_DP_A:
         /*ChengLin 20130419: Separate 4K2K board Normal DP and Mini DP wake up detection*/         
        if((BoardType==BOARD_4K2K) && (ChipID4k2K == CHIPID_4K2K_R))
         {
            if(gm_ReadRegWord(LPM_GPOUTPUT_0)& LPM_GPIO14_OUT) //Normal DP
            {
               if( ((CurInputPort[0]==ALL_BOARDS_DP1)&&(CurInputPort[1]==CPhyNum_0))
                  ||((CurInputPort[2]==ALL_BOARDS_DP1)&&(CurInputPort[3]==CPhyNum_0))
                  ||((CurInputPort[4]==ALL_BOARDS_DP1)&&(CurInputPort[5]==CPhyNum_0))
                  ||((CurInputPort[6]==ALL_BOARDS_DP1)&&(CurInputPort[7]==CPhyNum_0)) )
                  Status = TRUE;  
            }
            else //Mini DP
            {         
               if( ((CurInputPort[0]==ALL_BOARDS_DP3)&&(CurInputPort[1]==CPhyNum_0))
                  ||((CurInputPort[2]==ALL_BOARDS_DP3)&&(CurInputPort[3]==CPhyNum_0))
                  ||((CurInputPort[4]==ALL_BOARDS_DP3)&&(CurInputPort[5]==CPhyNum_0))
                  ||((CurInputPort[6]==ALL_BOARDS_DP3)&&(CurInputPort[7]==CPhyNum_0)) )
                  Status = TRUE;  
            }
         }
         else
         {
            if( ((CurInputPort[0]==ALL_BOARDS_DP1)&&(CurInputPort[1]==CPhyNum_0))
               ||((CurInputPort[2]==ALL_BOARDS_DP1)&&(CurInputPort[3]==CPhyNum_0))
               ||((CurInputPort[4]==ALL_BOARDS_DP1)&&(CurInputPort[5]==CPhyNum_0))
               ||((CurInputPort[6]==ALL_BOARDS_DP1)&&(CurInputPort[7]==CPhyNum_0)) )
               Status = TRUE;  
         }
         break;         
      case Port_DP_B:
         if( ((CurInputPort[0]==ALL_BOARDS_DP2)&&(CurInputPort[1]==CPhyNum_2))
            ||((CurInputPort[2]==ALL_BOARDS_DP2)&&(CurInputPort[3]==CPhyNum_2))
            ||((CurInputPort[4]==ALL_BOARDS_DP2)&&(CurInputPort[5]==CPhyNum_2))
            ||((CurInputPort[6]==ALL_BOARDS_DP2)&&(CurInputPort[7]==CPhyNum_2)) )
            Status = TRUE;         
         break;
         
      case Port_VGA:
         if( (CurInputPort[0]==ALL_BOARDS_VGA1)||(CurInputPort[2]==ALL_BOARDS_VGA1)
            ||(CurInputPort[4]==ALL_BOARDS_VGA1)||(CurInputPort[6]==ALL_BOARDS_VGA1))
            Status = TRUE;
         break;

      default:
         break;
   }

   return Status;
}

BYTE Lpm_IsAllPortWakeup(void)
{
   if(SigDetPortNum == MAX_SIGDET_PORT_NUM)
      return TRUE;
   else 
      return FALSE;
}

BYTE Lpm_IsWakeupTmdsCPhy(gmt_CPhyNum CPhyNum)
{
   if( (CurInputPort[1] == CPhyNum) || (CurInputPort[3] == CPhyNum)
      ||(CurInputPort[5] == CPhyNum) || (CurInputPort[7] == CPhyNum) )  
      return TRUE;
   else
      return FALSE;
}

void Lpm_ExternalSwitchCheck(void)
{
   gmt_CPhyNum CPhyNum;

   for(CPhyNum=CPhyNum_0;CPhyNum<CPhyNum_Num;CPhyNum++)
   {
      BYTE count;
      BYTE i;

      count=0;
      for(i=0;i<SigDetPortNum;i++)
      {
         if(CurInputPort[(i<<1)+1] == CPhyNum) // CPhy number 
         {
            count++;
         }
      }

      if(count>1)
      {
         ExternalSwitch = CPhyNum;
         return;
      }
   }
   ExternalSwitch = 0xFF;
   return;
}

BYTE Lpm_GetWakeupExtraInfo(gmt_LPM_Wakeup_Detect CPhyNum)
{
   if(CPhyNum >= LPM_WAKEUP_CPHY0 && CPhyNum <= LPM_WAKEUP_CPHY2)
   {
      BYTE i;
      for(i=0;i<SigDetPortNum;i++)
      {
         if(CurInputPort[(i<<1)+1] == CPhyNum) // CPhy number 
         {
            if(ExternalSwitch != 0xFF)
            {
               BYTE WakeupExtraInfo;
               //Modify this code to find port number for external switch mapping to the CPhy number.
               WakeupExtraInfo = CurInputPort[(i<<1)];
               return WakeupExtraInfo;
            }
            else
            {
               return (CurInputPort[(i<<1)]); // Port number
            }
         }
      }
   }
   return 0;
}
#else
void Lpm_SignalDetectInit(void)
{
}
void Lpm_SignalDetectStop(void)
{
}
void far Lpm_AutoWakeISR(gmt_PortChannel B_Port)
{
   UNUSED_VAR(B_Port);
}
BYTE Lpm_IsWakeupPort(gmt_PortChannel B_Port)
{
   UNUSED_VAR(B_Port);
   return TRUE;
}
#endif

