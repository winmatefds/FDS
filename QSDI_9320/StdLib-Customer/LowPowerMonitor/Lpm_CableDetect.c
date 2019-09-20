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
// MODULE:      Lpm_CableDetect.c
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
#include <stdlib.h>
#include <embedded.h>

#ifdef CABLE_DETECT_SUPPORT

static WORD LastTime = 0;


void Lpm_CableDetectHandlerInit(void)
{
   LbadcCableNowState[0] = 0;
   LbadcCableNowState[1] = 0;
   LastTime	= 0;

   #ifdef PD_MONITORED_BY_LPM_GPIO	
   LpmGpioLastState[0] = 0;
   LpmGpioNowState[0]  = 0;		
   LpmGpioLastState[1] = 0;
   LpmGpioNowState[1]  = 0;
   #endif
}

void Lpm_CableDetectHandler(void)
{
   WORD NowTime = gm_W_GetSystemTime();
   BOOL IsUpdate = FALSE;

   if (abs(NowTime - LastTime) >= CableHandlerPeriod) 
   {			 
      LastTime = NowTime;	

      #ifdef PD_MONITORED_BY_LPM_GPIO
      LpmGpioNowState[0]= gm_ReadRegWord(LPM_GPINPUT_0) & LpmGpio0CheckMask;
      LpmGpioNowState[1]= gm_ReadRegWord(LPM_GPINPUT_1) & LpmGpio1CheckMask;

      //Check Gpio 
      if ((LpmGpioNowState[0]!=LpmGpioLastState[0]) || (LpmGpioNowState[1]!=LpmGpioLastState[1]) )
      {
         LpmGpioLastState[0] = LpmGpioNowState[0];
         LpmGpioLastState[1] = LpmGpioNowState[1];	
         gm_Print("---GPIO cable update",0);
         IsUpdate = TRUE;
      }
      #endif		
      
      //Update cable status if get request from ADC2 or ADC3 
      if ( CableUpdateRequest == TRUE)
      {
         //gm_Print("----Cable ADC2 update status %d",LbadcCableNowState[0]);			
         //gm_Print("----Cable ADC3 update status %d",LbadcCableNowState[1]);				
         CableUpdateRequest = FALSE;
         IsUpdate = TRUE;
      }	

      if (IsUpdate)
      {
         WORD msg[4];
         #ifdef PD_MONITORED_BY_LPM_GPIO			
         msg[0] = LpmGpioNowState[0];
         msg[1] = LpmGpioNowState[1];			
         #endif
         msg[2] = LbadcCableNowState[0];			
         msg[3] = LbadcCableNowState[1];			

         if (gm_MessageSend(gmd_MESSAGE_CableStatus, &msg[0], sizeof(msg))==FALSE )	
         {
            gm_Print("----send Cable status failed !",0); 
         }
         //else
         {
         //   gm_Print("----send Cable status OK !",0);
         }
      }	
   }
}

BYTE Lpm_IsDPRxSrcPwrValid(DPRX_SEL DPRxSel)
{
   #ifdef CHECK_DP_SRC_PWR
   /*ChengLin 20130419: Separate 4K2K board Normal DP and Mini DP Source Power Pin*/    
   if((BoardType==BOARD_4K2K) && (ChipID4k2K == CHIPID_4K2K_R))
   {  
      if(DPRxSel == DPRX_C0)
      {
         if(gm_ReadRegWord(LPM_GPOUTPUT_0)& LPM_GPIO14_OUT) //Normal DP
         {        
            if(gm_ReadRegByte(LPM_CSM_HPD_PD_IN_STATUS) & PD2_IN_STATUS)   //source power valid
               return TRUE;
            else
               return FALSE;
         }
         else //Mini DP
         {          
            if(gm_ReadRegByte(LPM_CSM_HPD_PD_IN_STATUS) & PD4_IN_STATUS)   //source power valid
               return TRUE;
            else
               return FALSE;
         }
      }
      else
      {
         if(gm_ReadRegByte(LPM_CSM_HPD_PD_IN_STATUS) & DpRxSrcPwrPin[DPRxSel])   //source power valid
            return TRUE;
         else
            return FALSE;
      }
   }
   else
   {
      if(gm_ReadRegByte(LPM_CSM_HPD_PD_IN_STATUS) & DpRxSrcPwrPin[DPRxSel])   //source power valid
         return TRUE;
      else
         return FALSE;
   }
   #else
   UNUSED_VAR(DPRxSel);
   return TRUE;
   #endif
}

BYTE Lpm_IsDPRxCablePlugged(DPRX_SEL DPRxSel)
{  
   #ifdef CHECK_DP_CABLE_STS
   /*ChengLin 20130419: Separate 4K2K board Normal DP and Mini DP Cable Detection Pin*/     
   if((BoardType==BOARD_4K2K) && (ChipID4k2K == CHIPID_4K2K_R))
   {
      if(DPRxSel == DPRX_C0)
      {
         if(gm_ReadRegWord(LPM_GPOUTPUT_0)& LPM_GPIO14_OUT) //Normal DP
         {
            if(gm_ReadRegByte(LPM_CSM_HPD_PD_IN_STATUS) & PD1_IN_STATUS)   //cable unplugged
               return FALSE;
            else
               return TRUE;
         }
         else //Mini DP
         {
            if(gm_ReadRegByte(LPM_CSM_HPD_PD_IN_STATUS) & PD3_IN_STATUS)   //cable unplugged
               return FALSE;
            else
               return TRUE;
         }
      }
      else
      {
         if(gm_ReadRegByte(LPM_CSM_HPD_PD_IN_STATUS) & DpRxCblDetPin[DPRxSel])   //cable unplugged
            return FALSE;
         else
            return TRUE;
      }
   }
   else
   {
      if(gm_ReadRegByte(LPM_CSM_HPD_PD_IN_STATUS) & DpRxCblDetPin[DPRxSel])   //cable unplugged
         return FALSE;
      else
         return TRUE;
   }
   #else
   UNUSED_VAR(DPRxSel);
   return TRUE;
   #endif
}

BYTE Lpm_IsCPhySrcPwrValid(gmt_CPhyNum CPhyNum)
{
   #ifdef CHECK_CPHY_SRC_PWR
   BYTE SrcPwrDet = FALSE;

   if(CPhyNum == CPhyNum_0)
   {
      if((gm_ReadRegByte(LPM_CSM_HPD_PD_IN_STATUS) & PD2_IN_STATUS) == 0)
         SrcPwrDet = TRUE;    
   }
   else if(CPhyNum == CPhyNum_1)
   {
      if(BoardType == BOARD_RD3)
      {   
         if( LBADC_EQUAL(LbadcCableNowState[0],RD3_LBADC_DVI_CBL5V) 
          || LBADC_EQUAL(LbadcCableNowState[0],RD3_LBADC_DVI_CBL5V_VGA_CBL)
          || LBADC_EQUAL(LbadcCableNowState[0],RD3_LBADC_DVI_CBL5V_VGA_CBL5V) )
            SrcPwrDet = TRUE;  
      }
      else
         SrcPwrDet = TRUE;
   }
   else if(CPhyNum == CPhyNum_2)
   {
      if(BoardType == BOARD_RD3)
      {     
         if( LBADC_EQUAL(LbadcCableNowState[1],RD3_LBADC_HDMI_CBL5V) )
            SrcPwrDet = TRUE;     
      }
      else if(BoardType == BOARD_EV1)
      {
         if((gm_ReadRegByte(LPM_CSM_HPD_PD_IN_STATUS) & PD4_IN_STATUS) == 0)
            SrcPwrDet = TRUE;  
      } 
      else
         SrcPwrDet = TRUE;
   } 
   return SrcPwrDet;
   
   #else
   UNUSED_VAR(CPhyNum);
   return TRUE;
   #endif   
}

BYTE Lpm_IsCPhyCablePlugged(gmt_CPhyNum CPhyNum)
{
   #ifdef CHECK_CPHY_CABLE_STS
   BYTE CblDet = FALSE;
   
   //EV1, EV2, RD1, RD2, RD3 use the same cable detect pin for CPHY0, CPHY1, CPHY2
   if(CPhyNum == CPhyNum_0)
   {
   	// 20130204, For 4Kx2K board, FE HDMI port connect to PHY 0, but use HPD3
   	if((BoardType == BOARD_4K2K) && (ChipID4k2K == CHIPID_4K2K_FE))
   	{
   		if(gm_ReadRegByte(LPM_CSM_HPD_PD_IN_STATUS) & PD3_IN_STATUS)   //cable plugged
            CblDet = TRUE;  
   	}
		else
		{
		   if((gm_ReadRegByte(LPM_CSM_HPD_PD_IN_STATUS) & PD1_IN_STATUS) == 0)   //cable plugged
         	CblDet = TRUE;
		}	
   }
   else if(CPhyNum == CPhyNum_1)
   {
      #ifdef TMDS_CLKDET_BY_AUTOWAKE_IRQ
      CblDet = TRUE; //due to can't get ADC value in time in Autowake ISR
      #else
      if(BoardType == BOARD_EV1)
      {
         if(LBADC_EQUAL(LbadcCableNowState[0],EV1_LBADC_HDMI_CABLE) || LBADC_EQUAL(LbadcCableNowState[0],EV1_LBADC_HDMI_VGA_CABLE))   //cable plugged
            CblDet = TRUE;     
      }
      else if(BoardType == BOARD_EV2)
      {
         if(LBADC_EQUAL(LbadcCableNowState[0],EV2_LBADC_HDMI_B_CABLE) || LBADC_EQUAL(LbadcCableNowState[0],EV2_LBADC_HDMI_B_VGA_CABLE))   //cable plugged
            CblDet = TRUE;     
      }  
      else if(BoardType == BOARD_EV3)
      {
         if(gm_ReadRegByte(LPM_CSM_HPD_PD_IN_STATUS) & PD3_IN_STATUS)   //cable plugged
            CblDet = TRUE;     
      } 
      else if(BoardType == BOARD_RD3)
      {
         if( LBADC_EQUAL(LbadcCableNowState[0],RD3_LBADC_DVI_CBL) || LBADC_EQUAL(LbadcCableNowState[0],RD3_LBADC_DVI_CBL5V)
          || LBADC_EQUAL(LbadcCableNowState[0],RD3_LBADC_DVI_CBL_VGA_CBL) || LBADC_EQUAL(LbadcCableNowState[0],RD3_LBADC_DVI_CBL_VGA_CBL5V)
          || LBADC_EQUAL(LbadcCableNowState[0],RD3_LBADC_DVI_CBL5V_VGA_CBL) || LBADC_EQUAL(LbadcCableNowState[0],RD3_LBADC_DVI_CBL5V_VGA_CBL5V) )
            CblDet = TRUE;     
      }      
      else //if((BoardType == BOARD_RD1) || (BoardType == BOARD_RD2))
      {
         if(LBADC_EQUAL(LbadcCableNowState[0],RD1_LBADC_DVI_CABLE) || LBADC_EQUAL(LbadcCableNowState[0],RD1_LBADC_DVI_VGA_CABLE))   //cable plugged
            CblDet = TRUE;     
      }    
      #endif
   }
   else if(CPhyNum == CPhyNum_2)
   {
      if(BoardType == BOARD_RD3)
      {
         if( LBADC_EQUAL(LbadcCableNowState[1],RD3_LBADC_HDMI_CBL) 
            || LBADC_EQUAL(LbadcCableNowState[1],RD3_LBADC_HDMI_CBL5V) )
            CblDet = TRUE;     
      }
      else
      {
         if((gm_ReadRegByte(LPM_CSM_HPD_PD_IN_STATUS) & PD3_IN_STATUS) == 0)   //cable plugged
            CblDet = TRUE;     
      }
   }
   return CblDet;
   
   #else
   UNUSED_VAR(CPhyNum);
   return TRUE;
   #endif
}

BYTE Lpm_IsVGASrcPwrValid(void)
{
   #ifdef CHECK_VGA_SRC_PWR
   BYTE SrcPwrDet = FALSE;

   if(BoardType == BOARD_RD3)
   {
      if( LBADC_EQUAL(LbadcCableNowState[0],RD3_LBADC_VGA_CBL5V) 
       || LBADC_EQUAL(LbadcCableNowState[0],RD3_LBADC_DVI_CBL_VGA_CBL5V)
       || LBADC_EQUAL(LbadcCableNowState[0],RD3_LBADC_DVI_CBL5V_VGA_CBL5V) )
         SrcPwrDet = TRUE;     
   }   
   else
      SrcPwrDet = TRUE;
    
   return SrcPwrDet;

   #else
   return TRUE;
   #endif     
}

BYTE Lpm_IsVGACablePlugged(void)
{
   #ifdef CHECK_VGA_CABLE_STS
   BYTE CblDet = FALSE;

   if(BoardType == BOARD_EV1)
   {
      if(LBADC_EQUAL(LbadcCableNowState[0],EV1_LBADC_VGA_CABLE) || LBADC_EQUAL(LbadcCableNowState[0],EV1_LBADC_HDMI_VGA_CABLE))   //cable plugged
         CblDet = TRUE;     
   }
   else if(BoardType == BOARD_EV2)
   {
      if(LBADC_EQUAL(LbadcCableNowState[0],EV2_LBADC_VGA_CABLE) || LBADC_EQUAL(LbadcCableNowState[0],EV2_LBADC_HDMI_B_VGA_CABLE))   //cable plugged
         CblDet = TRUE;     
   }  
   else if(BoardType == BOARD_EV3)
   {
      if(LBADC_EQUAL(LbadcCableNowState[0],EV3_LBADC_VGA_CABLE))   //cable plugged
         CblDet = TRUE;     
   }   
   else if(BoardType == BOARD_RD3)
   {
      if( LBADC_EQUAL(LbadcCableNowState[0],RD3_LBADC_VGA_CBL) || LBADC_EQUAL(LbadcCableNowState[0],RD3_LBADC_VGA_CBL5V)
       || LBADC_EQUAL(LbadcCableNowState[0],RD3_LBADC_DVI_CBL_VGA_CBL) || LBADC_EQUAL(LbadcCableNowState[0],RD3_LBADC_DVI_CBL_VGA_CBL5V)
       || LBADC_EQUAL(LbadcCableNowState[0],RD3_LBADC_DVI_CBL5V_VGA_CBL) || LBADC_EQUAL(LbadcCableNowState[0],RD3_LBADC_DVI_CBL5V_VGA_CBL5V) )
         CblDet = TRUE;     
   } 
   else //if((BoardType == BOARD_RD1) || (BoardType == BOARD_RD2))
   {
      if(LBADC_EQUAL(LbadcCableNowState[0],RD1_LBADC_VGA_CABLE) || LBADC_EQUAL(LbadcCableNowState[0],RD1_LBADC_DVI_VGA_CABLE))   //cable plugged
         CblDet = TRUE;     
   }   
   return CblDet;

   #else
   return TRUE;
   #endif   
}

#else
BYTE Lpm_IsDPRxSrcPwrValid(DPRX_SEL DPRxSel)
{
   UNUSED_VAR(DPRxSel);
   return TRUE;
}
BYTE Lpm_IsDPRxCablePlugged(DPRX_SEL DPRxSel)
{  
   UNUSED_VAR(DPRxSel);
   return TRUE;
}
BYTE Lpm_IsCPhySrcPwrValid(gmt_CPhyNum CPhyNum)
{
   UNUSED_VAR(CPhyNum);
   return TRUE;
}
BYTE Lpm_IsCPhyCablePlugged(gmt_CPhyNum CPhyNum)
{
   UNUSED_VAR(CPhyNum);
   return TRUE;
}
BYTE Lpm_IsVGASrcPwrValid(void)
{
   return TRUE; 
}
BYTE Lpm_IsVGACablePlugged(void)
{
   return TRUE; 
}
#endif
//******************************************************************************

