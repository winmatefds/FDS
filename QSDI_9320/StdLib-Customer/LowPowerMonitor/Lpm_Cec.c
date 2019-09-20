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
// MODULE:      Lpm_Cec.c
//
// USAGE:       
//
//******************************************************************************

//******************************************************************************
//  I N C L U D E    F I L E S
//******************************************************************************
#include ".\LowPowerMonitor\Lpm_All.h"


#ifdef CEC_SUPPORT

#ifdef _DEBUG
#define  msg_i(a,b)        	 	gm_Print(a,b)
#else
#define  msg_i(a,b)
#endif


#define CEC_REG_BASE_ADDR 0x0DC66


#define xADVANCED_CEC_FEATURES
#define xLOG_ADDR_ALLOC_FEATURE

#if 0
typedef enum
{
    CEC_MSG_FEATURE_ABORT	            = 0x00,  // <Feature Abort> message
	CEC_MSG_IMAGE_VIEW_ON	            = 0x04,	// <Image View On> message
	CEC_MSG_TEXT_VIEW_ON	            = 0x0D,	// <Text View On> message
	CEC_MSG_STANDBY                     = 0x36,	// <Standby> message
    CEC_MSG_GIVE_OSD_NAME               = 0x46,  // <Give OSD Name> message
    CEC_MSG_GIVE_PHYSICAL_ADDRESS       = 0x83,  // <Give Physical Address> message
    CEC_MSG_GIVE_DEVICE_VENDOR_ID       = 0x8C,  // <Give Device Vendor ID>
    CEC_MSG_GIVE_DEVICE_POWER_STATUS    = 0x8F,  // <Give Device Power Status>
    CEC_MSG_GET_MENU_LANGUAGE           = 0x91,  // <Get Menu Language>
    CEC_MSG_GET_CEC_VERSION             = 0x9F,  // <Get CEC version>
    CEC_MSG_ABORT                       = 0xFF   // <Abort> message
} gmt_MNGR_CEC_FEATURES_ENUM;
#endif

typedef enum
{
    CEC_IPC_DATA,
    CEC_IPC_FRAME_STATUS,
    CEC_IPC_BUS_STATUS,
    CEC_IPC_SET_LOG_ADDR,
    CEC_IPC_GET_TIME,
    CEC_IPC_CONFIG_DATA,
    MAX_CEC_ID
}gmt_CEC_IPC_ID;

BYTE B_CecFrameStatusLpm;
BYTE B_CecBusStatusLpm;

#define SIZE_OF_CEC_RX_BUFFER   40

BYTE Ba_Buffer[20];
BYTE Ba_CecRxData[SIZE_OF_CEC_RX_BUFFER];

#ifdef ADVANCED_CEC_FEATURES

#define CEC_CFG_PH_ADDR         0
#define CEC_CFG_LANGUAGE        4
#define CEC_CFG_VERSION         8
#define CEC_CFG_PWR_STAT       10
#define CEC_CFG_VENDOR_ID      12
#define CEC_CFG_OSD_NAME       16

#define CEC_CONFIG_DATA_SIZE    31

BYTE Ba_CecCfgData[CEC_CONFIG_DATA_SIZE];

#endif

#ifdef LOG_ADDR_ALLOC_FEATURE
void gm_HPD_Handler(void);
#endif

//******************************************************************************
//  C O D E
//******************************************************************************
extern BOOL powerUpRequest;
void Lpm_CecHandler(void)
{
	//BOOL isPowerKey = FALSE;
    #ifdef ADVANCED_CEC_FEATURES
    BYTE i;
    #endif

    #ifdef LOG_ADDR_ALLOC_FEATURE
    if (!IsMissionPoweredUp())
    {
        gm_HPD_Handler();

        gm_CecAppLogAddrHandler();
    }
    #endif

    if (!Configuration.PermanentPeripheralControl)
    {
        
        #ifdef ADVANCED_CEC_FEATURES
        if(gm_RegMsgRv(gmd_MESSAGE_CEC, (BYTE far *)&Ba_Buffer[0]))
        {
            if(Ba_Buffer[0] == CEC_IPC_CONFIG_DATA)
            {

                for(i = 0; i < 8; i++)
                {
                    Ba_CecCfgData[Ba_Buffer[1] + i] = Ba_Buffer[i + 2];
                }
            }
        }
        #endif // ADVANCED_CEC_FEATURES

    	if (!(gm_ReadRegWord(LPM_MICRO_CONTROL) & LP_RUN))//make sure pin is connected to LPM
    		return; 	// FALSE;
    }//!PermanentPeripheralControl
	
    if(gm_CecGetFrameStatus() & gmd_CEC_FRAME_RX_DONE)
	{
		//gm_CecClearFrameStatusBits(gmd_CEC_FRAME_RX_DONE);

        msg_i("RX_DONE", 0);

        if(gm_CecGetBusStatus() & gmd_CEC_BUS_FREE)
        {

            if(gm_CecRxFrame(&Ba_Buffer[1], &Ba_Buffer[2], &Ba_Buffer[3], &Ba_Buffer[4]))
            {
                Ba_Buffer[0] = CEC_IPC_DATA;
                Ba_Buffer[3]&= 0x1F;
               /*
                gm_Print("Ba_Buffer 0x%x", Ba_Buffer[0]);
                gm_Print("          0x%x", Ba_Buffer[1]);
                gm_Print("          0x%x", Ba_Buffer[2]);
                gm_Print("          0x%x", Ba_Buffer[3]);
                gm_Print("          0x%x", Ba_Buffer[4]);
                */
                // This part of CEC code need to be implemented (Mission power status)
                switch(Ba_Buffer[4])
                {
                    //case CEC_MSG_IMAGE_VIEW_ON:
                    case CEC_MSG_TEXT_VIEW_ON:
                    //case CEC_MSG_STANDBY:
                     if (!IsMissionPoweredUp())
		               {
		               #ifdef POWER_MODE_HANDLE
                     gm_Print("ACTIVE SOURCE, Going to Turn on Panel !", 0);
		               Lpm_SetPowerKeyEvent();			
		               #endif
                     }

                    break;
                    case CEC_MSG_INACTIVE_SOURCE:
                        gm_Print("INACTIVE SOURCE, Going to Turn off Panel !", 0);
                        Lpm_SetPowerKeyEvent();
                     break;

                    #ifdef ADVANCED_CEC_FEATURES
                    case CEC_MSG_GIVE_OSD_NAME:
                        if (!IsMissionPoweredUp())
                        {
                            gm_CecTxFrame(Ba_Buffer[1], (Ba_CecCfgData[CEC_CFG_OSD_NAME + 15] & 0xF), (BYTE far *)&Ba_CecCfgData[CEC_CFG_OSD_NAME]);
                        }
                        break;

                    case CEC_MSG_GIVE_PHYSICAL_ADDRESS:
                       
                            gm_CecTxFrame(0x0f, 4, (BYTE far *)&Ba_CecCfgData[CEC_CFG_PH_ADDR]);

                        break;

                    case CEC_MSG_GIVE_DEVICE_VENDOR_ID:

                            gm_CecTxFrame(Ba_Buffer[1], 4, (BYTE far *)&Ba_CecCfgData[CEC_CFG_VENDOR_ID]);

                        break;

                    case CEC_MSG_GIVE_DEVICE_POWER_STATUS:
                        if (!IsMissionPoweredUp())
                        {
                            gm_CecTxFrame(Ba_Buffer[1], 2, (BYTE far *)&Ba_CecCfgData[CEC_CFG_PWR_STAT]);
                        }
                        break;

                    case CEC_MSG_GET_MENU_LANGUAGE:
                        if (!IsMissionPoweredUp())
                        {
                            gm_CecTxFrame(Ba_Buffer[1], 4, (BYTE far *)&Ba_CecCfgData[CEC_CFG_LANGUAGE]);
                        }
                        break;

                    case CEC_MSG_GET_CEC_VERSION:
                        if (!IsMissionPoweredUp())
                        {
                            gm_CecTxFrame(Ba_Buffer[1], 2, (BYTE far *)&Ba_CecCfgData[CEC_CFG_VERSION]);
                        }
                        break;
                    #endif

                    default:
                        if (!IsMissionPoweredUp())
                        {// <Feature Abort>

                            Ba_Buffer[2] = CEC_MSG_FEATURE_ABORT;
                            Ba_Buffer[3] = Ba_Buffer[4];
                            Ba_Buffer[4] = 1;
                            gm_CecTxFrame(Ba_Buffer[1], 3, (BYTE far *)&Ba_Buffer[2]);
                        }
                        break;
                }


                if (IsMissionPoweredUp())
                {// Send mesage if Mission is PowerOn
                    gm_RegMsgSd(gmd_MESSAGE_CEC, Ba_Buffer, Ba_Buffer[3] + 4);
                }
            }
            else
            {
                gm_CecClearFrameStatusBits(gmd_CEC_FRAME_RX_DONE);

                msg_i("Rx Empty", 0);
            }
        }
	}

    if (Configuration.PermanentPeripheralControl)
    {
        if (gm_CecGetFrameStatus() != B_CecFrameStatusLpm)
        {
            B_CecFrameStatusLpm = gm_CecGetFrameStatus();

            if (IsMissionPoweredUp())
            {// Send mesage if Mission is PowerOn

                Ba_Buffer[0] = CEC_IPC_FRAME_STATUS;
                Ba_Buffer[1] = B_CecFrameStatusLpm;
                gm_RegMsgSd(gmd_MESSAGE_CEC, Ba_Buffer, 2);
            }
        }

        if (gm_CecGetBusStatus() != B_CecBusStatusLpm)
        {
            B_CecBusStatusLpm = gm_CecGetBusStatus();
            //Ba_Buffer[2] = gm_CecGetBusStatus();

            if (IsMissionPoweredUp())
            {// Send mesage if Mission is PowerOn
                Ba_Buffer[0] = CEC_IPC_BUS_STATUS;
                Ba_Buffer[1] = B_CecBusStatusLpm;
                gm_RegMsgSd(gmd_MESSAGE_CEC, Ba_Buffer, 2);
            }
        }

        if(gm_RegMsgRv(gmd_MESSAGE_CEC, (BYTE far *)&Ba_Buffer[0]))
        {
            switch(Ba_Buffer[0])
            {
                case CEC_IPC_DATA:
                    gm_CecTxFrame(Ba_Buffer[1], Ba_Buffer[2], (BYTE far *)&Ba_Buffer[3]);
                    break;

                case CEC_IPC_FRAME_STATUS:
                    //B_CecFrameStatusLpm = B_Buffer[1];
                    gm_CecClearFrameStatusBits(Ba_Buffer[1]);
                    break;

                case CEC_IPC_SET_LOG_ADDR:
                    gm_CecSetLogAddr(Ba_Buffer[1]);
                    break;

                #ifdef ADVANCED_CEC_FEATURES
                case CEC_IPC_CONFIG_DATA:
                    {
                        for(i = 0; i < 8; i++)
                        {
                            Ba_CecCfgData[Ba_Buffer[1] + i] = Ba_Buffer[i + 2];
                          }
                    }
                    break;
                #endif
            }
        }
    }
    
//	return isPowerKey;
}

void Lpm_CecInit(void)
{
    gmt_CEC_INIT S_InitStructure;
    
    S_InitStructure.W_CecRegBaseAddress         = CEC_REG_BASE_ADDR;
    S_InitStructure.B_NumberOfRetransmissions   = 5;
    S_InitStructure.B_SizeOfRxBuff              = SIZE_OF_CEC_RX_BUFFER;
    S_InitStructure.Bp_CecRxBufferAddress       = &Ba_CecRxData[0];
    S_InitStructure.DW_TClkHz                   = SYSTEM_TCLK_FREQ;

    gm_CecInit(&S_InitStructure);

    B_CecFrameStatusLpm = 0;
    B_CecBusStatusLpm   = 0;
    
    #ifdef LOG_ADDR_ALLOC_FEATURE
    gm_CecAppSetPhysicalAddress(0x00);

    gm_CecAppLogAddrHandlerRequest();
    #endif

}

#ifdef LOG_ADDR_ALLOC_FEATURE
void gm_HPD_Handler(void)
{
    #if 0
    if(gm_GpioSwitch())
    {
        gm_CecAppLogAddrHandlerRequest();
    }
    #endif
}
#endif

#endif
