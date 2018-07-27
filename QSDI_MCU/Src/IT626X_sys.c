///*****************************************
//  Copyright (C) 2009-2015
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <IT626X_sys.c>
//   @author Jau-chih.Tseng@ite.com.tw
//   @date   2015/03/11
//   @fileversion: IT6263_SAMPLE_V209
//******************************************/

///////////////////////////////////////////////////////////////////////////////
// This is the sample program for CAT6611 driver usage.
///////////////////////////////////////////////////////////////////////////////

//#include "hdmitx.h"

#include "Access.h"
#include "typedef.h"
#include "stm32f1xx_hal.h"
 #include <stdlib.h>
#include "IT626X_drv.h"
#include "hdmirx.h"
 #include "IT626X_sys.h"
#define INPUT_SIGNAL_TYPE 0 // 24 bit sync seperate

#define DEBUG_MSG 0

#define I2S 0
#define SPDIF 1

_IDATA INSTANCE InstanceData =
{


    0,      // BYTE I2C_DEV ;
    0x98,    // BYTE I2C_ADDR ;

    /////////////////////////////////////////////////
    // Interrupt Type
    /////////////////////////////////////////////////
    0x40,      // BYTE bIntType ; // = 0 ;
    /////////////////////////////////////////////////
    // Video Property
    /////////////////////////////////////////////////
    INPUT_SIGNAL_TYPE ,// BYTE bInputVideoSignalType ; // for Sync Embedded,CCIR656,InputDDR

    /////////////////////////////////////////////////
    // Audio Property
    /////////////////////////////////////////////////
    SPDIF, // BYTE bOutputAudioMode ; // = 0 ;
    FALSE , // BYTE bAudioChannelSwap ; // = 0 ;
    0x0f, // BYTE bAudioChannelEnable ;
    AUDFS_48KHz ,// BYTE bAudFs ;
    0, // unsigned long TMDSClock ;
    FALSE, // BYTE bAuthenticated:1 ;
    FALSE, // BYTE bHDMIMode: 1;
    FALSE, // BYTE bIntPOL:1 ; // 0 = Low Active
    FALSE, // BYTE bHPD:1 ;

};

_IDATA BYTE HPDStatus = FALSE;
_IDATA BYTE HPDChangeStatus = FALSE;
BOOL ReGenTimingEnable=FALSE;
BOOL bChangeMode = FALSE ;
BOOL bForceCTS = FALSE;
_IDATA BYTE OutputColorDepth = 24 ;
BOOL bHDMIMode, bAudioEnable ;
_IDATA BYTE VIC ; // 480p60
_IDATA ULONG VideoPixelClock ;
_IDATA BYTE pixelrep ; // no pixelrepeating
_IDATA HDMI_Aspec aspec ;
_IDATA BYTE HDMI3DFormat = 0x00 ; // 3D Format
_IDATA BYTE bOutputColorMode = F_MODE_RGB444 ;
_IDATA BYTE bInputColorMode = F_MODE_RGB444;
_IDATA Audio_InfoFrame AudioInfo ;
_IDATA AVI_InfoFrame AviInfo;
_IDATA HDMI_Colorimetry Colorimetry ;
_IDATA VendorSpecific_InfoFrame VS_Info;
 static RX_CAP RxCapability ;
 _XDATA  BYTE cBuf[128] ;
_IDATA BYTE iVideoModeSelect=0 ;

void HDMITX_ChangeDisplayOption(HDMI_Video_Type VideoMode, HDMI_OutputColorMode OutputColorMode);
void InitIT626X_Instance(void);
void HDMITX_SetOutput(void);
void HDMITX_DevLoopProc(void);
void ConfigAudioInfoFrm(void);
void ConfigAVIInfoFrame(BYTE VIC, BYTE pixelrep);
BYTE ParseEDID(void);
static BOOL ParseCEAEDID(BYTE *pCEAEDID);

/*
////////////////////////////////////////////////////////////////////////////////
// EDID
////////////////////////////////////////////////////////////////////////////////
 //


_IDATA USHORT LastInputPclk;
////////////////////////////////////////////////////////////////////////////////
// Program utility.
////////////////////////////////////////////////////////////////////////////////

// 2012/08/16 modified by jjtseng
#ifdef SUPPORT_HDCP
static BYTE bAuthenticated = FALSE ;
#endif


// _IDATA BYTE bInputColorMode = F_MODE_YUV422 ;
// _IDATA BYTE bInputColorMode = F_MODE_YUV444 ;
// _IDATA BYTE bOutputColorMode = F_MODE_YUV422 ;
// _IDATA BYTE bOutputColorMode = F_MODE_YUV444 ;




//_IDATA BYTE bAudioSampleFreq = INPUT_SAMPLE_FREQ ;



////////////////////////////////////////////////////////////////////////////////
// Function Body.
////////////////////////////////////////////////////////////////////////////////



*/

typedef enum VindorName {
    DELL_2408WPF = 0 ,
    UNKNOW_Monitor
} VindorName ;

struct EDID_Vindor {
    VindorName Name;
    BYTE  VIDC[8];
};
_CODE struct EDID_Vindor ForceCTS_EDID_Vindeo[]=
{
    {DELL_2408WPF,{0x10,0xAC,0x2C,0xA0,0x53,0x33,0x47,0x31 }},
};


BOOL AudeoModeDetect(void)
{
    SetupAudioChannel(bAudioEnable);
    return  TRUE;
}


BOOL VideoModeDetect(void)
{
    static BOOL ForceUpdate=TRUE;

    if(CheckLVDS()==FALSE)
    {
        ForceUpdate=TRUE;

        return FALSE;
    }
    // VIC = HDMI_1080p60;
    if(ForceUpdate)
    {
    #if Debug_message
		HDMITX_DEBUG_PRINTF(("VideoModeDetect()\n")) ;
	#endif
	    VIC = HDMI_Unkown ; // turn off Regentiming at first.
        ForceUpdate=FALSE;
#ifdef SUPPORT_REGEN_TIMING
        if(ReGenTiming(VIC))
        {
            ReGenTimingEnable=TRUE;
					 USART1_printf( USART1," ReGenTimingEnable   TRUE  \r\n",0); 

           // HDMITX_DEBUG_PRINTF(("USE RE GEN TIMING FORMAT %d\n",(int)VIC));
        }
        else
        {
            ReGenTimingEnable=FALSE;
			USART1_printf( USART1," ReGenTimingEnable   TRUE  \r\n",0); 
        //    HDMITX_DEBUG_PRINTF(("NO RE GEN TIMING MTACH %d\n",(int)VIC));
        }
#else
		VIC = HDMITX_DetectVIC() ;
#endif
  //      HDMITX_ChangeDisplayOption(HDMI_Unkown,HDMI_RGB444);
#if Debug_message
        HDMITX_DEBUG_PRINTF(("VIC = %d\n",(WORD)VIC));
        HDMITX_DEBUG_PRINTF(("H TOTAL = %d\n",GetHTotal()));
        HDMITX_DEBUG_PRINTF(("V TOTAL = %d\n",GetVTotal()));
        HDMITX_DEBUG_PRINTF(("H_Active = %d\n",GetHActive()));
        HDMITX_DEBUG_PRINTF(("V_Active = %d\n",GetVActive()));
        HDMITX_DEBUG_PRINTF(("PCLK = %u0,000\n",(WORD)(GetInputPclk()/10000)));
#endif
    }
    return TRUE;
	
}

void InitIT626X_Instance(void)
{
  HDMITX_InitInstance(&InstanceData);
	HPDStatus = FALSE;
	HPDChangeStatus = FALSE;

}

void
HDMITX_SetOutput(void)
{
    BYTE uc;
    BYTE level ;
    unsigned long cTMDSClock = VideoPixelClock*(pixelrep+1);
    DisableAudioOutput();
    uc = HDMITX_ReadI2C_Byte(0xc1);
    switch(uc & 0x70)
    {
    case 0x50:
        cTMDSClock *= 5 ;
        cTMDSClock /= 4 ;
        break ;
    case 0x60:
        cTMDSClock *= 3 ;
        cTMDSClock /= 2 ;
    }

    if( cTMDSClock>80000000L )
    {
        level = PCLK_HIGH ;
    }
    else if(cTMDSClock>20000000L)
    {
        level = PCLK_MEDIUM ;
    }
    else
    {
        level = PCLK_LOW ;
    }

#ifdef SUPPORT_HDCP
	EnableHDCP(FALSE);
    bAuthenticated = FALSE ;
#endif
	SetOutputColorDepthPhase(OutputColorDepth,0);
	SetupVideoInputSignal(InstanceData.bInputVideoSignalType);
	EnableVideoOutput(level,F_MODE_RGB444,F_MODE_RGB444 ,(BYTE)bHDMIMode);
    if( bHDMIMode )
    {
        ConfigAVIInfoFrame(VIC, pixelrep);
        // ConfigfHdmiVendorSpecificInfoFrame(Side_by_Side);
        ConfigfHdmiVendorSpecificInfoFrame(HDMI3DFormat);
        // ConfigfHdmiVendorSpecificInfoFrame(Frame_Packing);
        // ConfigfHdmiVendorSpecificInfoFrame(Top_and_Botton);
        // #ifdef SUPPORT_HDCP
        //         EnableHDCP(TRUE);
        // #endif
		if( bAudioEnable )
		{
/*	leo ?????
#ifdef SUPPORT_HBR_AUDIO
            EnableHDMIAudio(T_AUDIO_HBR, FALSE, 768000L,OUTPUT_CHANNEL,NULL,cTMDSClock);
#else
		// 2012/07/10 modified by jjtseng
		// found by Tranmin, fix.
        // #ifndef SUPPORT_I2S_AUDIO
        #ifdef SUPPORT_I2S_AUDIO
            EnableHDMIAudio(T_AUDIO_LPCM, FALSE, INPUT_AUDIO_SAMPLE_FREQ,OUTPUT_CHANNEL,NULL,cTMDSClock);
        #else
            EnableHDMIAudio(T_AUDIO_LPCM, TRUE, INPUT_AUDIO_SAMPLE_FREQ,OUTPUT_CHANNEL,NULL,cTMDSClock);
        #endif
#endif
*/
            ConfigAudioInfoFrm();
		}
    // 2012/08/16 modified by jjtseng
    #ifdef SUPPORT_HDCP
        SetAVMute(TRUE);
    #else
        SetAVMute(FALSE);
    #endif
    //~jjtseng 2012/08/16
    }
	else
	{

		EnableAVIInfoFrame(FALSE ,NULL);
        EnableVSInfoFrame(FALSE,NULL);
        SetAVMute(FALSE);
	}
    bChangeMode = FALSE ;
#if 0
    while(1)
    {
        uc=(LVDS_ReadI2C_Byte(0x0b)&(1<<6));
        if(uc != (1<<6))
            LVDS_OrREG_TX_Byte(0x0b,(1<<6));

        uc=(LVDS_ReadI2C_Byte(0x09)&(0x03));
        if(uc != 0x03)
            LVDS_OrREG_TX_Byte(0x09,(0x01));
    }
#endif
}
void MuteUpdata()
{
    if(GetAVMute())
    {
        if(GetVideoStatus())
        {
            HDMITX_AndREG_Byte(0x04,~(0x14));
            SetAVMute(FALSE);
        }
    }
}



void
HDMITX_DevLoopProc()
{
    //~jjtseng 2012/08/16
    static BYTE LoopCnt=0;
    CheckHDMITX(&HPDStatus,&HPDChangeStatus);
    if( HPDChangeStatus )
    {
    // 2012/08/16 modified by jjtseng
    #ifdef SUPPORT_HDCP
        bAuthenticated = FALSE ;
    #endif
    //~jjtseng 2012/08/16
        LoopCnt=0;
        if( HPDStatus )
        {
     //       ParseEDID(); //LEO
            bOutputColorMode = F_MODE_RGB444;

			if( RxCapability.ValidHDMI )
			{
				bHDMIMode = TRUE ;

				if(RxCapability.VideoMode & (1<<6))
				{
					bAudioEnable = TRUE ;
				}
			}
			else
			{
				bHDMIMode = FALSE ;
				bAudioEnable = FALSE ;
			}
#if Debug_message
    		HDMITX_DEBUG_PRINTF(("HPD change HDMITX_SetOutput();\n"));
#endif
			USART1_printf( USART1," HPD change HDMITX_SetOutput",0); 

            bChangeMode=TRUE;
            //HDMITX_SetOutput();//if system have use VideoModeDetect function

        }
        else
        {
            // unplug mode, ...
#if Debug_message
    		HDMITX_DEBUG_PRINTF(("HPD OFF DisableVideoOutput()\n"));
#endif


			USART1_printf( USART1," HPD OFF DisableVideoOutput",0); 

            DisableAudioOutput();
            DisableVideoOutput();
			
            #ifdef SUPPORT_HDCP
            EnableHDCP(FALSE);
            #endif
            bAudioEnable = FALSE ;
        }
    }
    else // no stable but need to process mode change procedure
    {

        LoopCnt++;
        CheckAudioVideoInput();
        if(0==(LoopCnt&0x0F) || bChangeMode)
        {
            if(FALSE==VideoModeDetect())
                {bChangeMode=TRUE;return;}
        }
        //===================================
        if(0==(LoopCnt&0x03))
            {AudeoModeDetect();}

        if(bChangeMode && HPDStatus)
            {HDMITX_SetOutput();}

    // 2012/08/16 modified by jjtseng
        #ifdef SUPPORT_HDCP
        if( HPDStatus && bAuthenticated == FALSE )
        {
            bAuthenticated = EnableHDCP(TRUE);
            if( bAuthenticated == TRUE )
            {
                SetAVMute(FALSE) ;
            }
        }
        #endif
    //~jjtseng 2012/08/16

    }
}


void
HDMITX_ChangeDisplayOption(HDMI_Video_Type OutputVideoTiming, HDMI_OutputColorMode OutputColorMode)
{
   //HDMI_Video_Type  t=HDMI_480i60_16x9;
  #if Debug_message 
   HDMITX_DEBUG_PRINTF(("HDMITX_ChangeDisplayOption(%d)\n",  (int)OutputVideoTiming)) ;
  #endif
   HDMI3DFormat = 0x00 ;
    switch(OutputVideoTiming)
	{
    case HDMI_640x480p60:
        VIC = 1 ;
        VideoPixelClock = 25000000L ;
        pixelrep = 0 ;
        aspec = HDMI_4x3 ;
        Colorimetry = HDMI_ITU601 ;
        break ;
    case HDMI_480p60:
        VIC = 2 ;
        VideoPixelClock = 27000000L ;
        pixelrep = 0 ;
        aspec = HDMI_4x3 ;
        Colorimetry = HDMI_ITU601 ;
        break ;
    case HDMI_480p60_16x9:
        VIC = 3 ;
        VideoPixelClock = 27000000L ;
        pixelrep = 0 ;
        aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU601 ;
        break ;
    case HDMI_720p60:
        VIC = 4 ;
        VideoPixelClock = 74250000L ;
        pixelrep = 0 ;
        aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU709 ;
        break ;

    case HDMI_1080i60:
        VIC = 5 ;
        VideoPixelClock = 74250000L ;
        pixelrep = 0 ;
        aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU709 ;
        break ;
    case HDMI_480i60:
        VIC = 6 ;
        VideoPixelClock = 13500000L ;
        pixelrep = 1 ;
        aspec = HDMI_4x3 ;
        Colorimetry = HDMI_ITU601 ;
        break ;
    case HDMI_480i60_16x9:
        VIC = 7 ;
        VideoPixelClock = 13500000L ;
        pixelrep = 1 ;
        aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU601 ;
        break ;
    case HDMI_1080p60:
        VIC = 16 ;
        VideoPixelClock = 148500000L ;
        pixelrep = 0 ;
        aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU709 ;
        break ;
    case HDMI_576p50:
        VIC = 17 ;
        VideoPixelClock = 27000000L ;
        pixelrep = 0 ;
        aspec = HDMI_4x3 ;
        Colorimetry = HDMI_ITU601 ;
        break ;
    case HDMI_576p50_16x9:
        VIC = 18 ;
        VideoPixelClock = 27000000L ;
        pixelrep = 0 ;
        aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU601 ;
        break ;
    case HDMI_720p50:
        VIC = 19 ;
        VideoPixelClock = 74250000L ;
        pixelrep = 0 ;
        aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU709 ;
        break ;

    case HDMI_1080i50:
        VIC = 20 ;
        VideoPixelClock = 74250000L ;
        pixelrep = 0 ;
        aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU709 ;
        break ;
    case HDMI_576i50:
        VIC = 21 ;
        VideoPixelClock = 13500000L ;
        pixelrep = 1 ;
        aspec = HDMI_4x3 ;
        Colorimetry = HDMI_ITU601 ;
        break ;
    case HDMI_576i50_16x9:
        VIC = 22 ;
        VideoPixelClock = 13500000L ;
        pixelrep = 1 ;
        aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU601 ;
        break ;
    case HDMI_1080p50:
        VIC = 31 ;
        VideoPixelClock = 148500000L ;
        pixelrep = 0 ;
        aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU709 ;
        break ;
    case HDMI_1080p24:
        VIC = 32 ;
        VideoPixelClock = 74250000L ;
        pixelrep = 0 ;
        aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU709 ;
        break ;

    case HDMI_1080p25:
        VIC = 33 ;
        VideoPixelClock = 74250000L ;
        pixelrep = 0 ;
        aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU709 ;
        break ;
    case HDMI_1080p30:
        VIC = 34 ;
        VideoPixelClock = 74250000L ;
        pixelrep = 0 ;
        aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU709 ;
        break ;
    default:
        VIC = 0 ;
        VideoPixelClock = GetInputPclk();
        pixelrep = 0 ;
        aspec = HDMI_4x3 ;
        Colorimetry = HDMI_ITU601 ;
        break ;
        //bChangeMode = FALSE ;
        //return ;
    }
    bOutputColorMode = F_MODE_RGB444 ;

    if( Colorimetry == HDMI_ITU709 )
    {
        bInputColorMode |= F_MODE_ITU709 ;
    }
    else
    {
        bInputColorMode &= ~F_MODE_ITU709 ;
    }

    if( Colorimetry != HDMI_640x480p60)
    {
        bInputColorMode |= F_MODE_16_235 ;
    }
    else
    {
        bInputColorMode &= ~F_MODE_16_235 ;
    }

    bChangeMode = TRUE ;
}



void
ConfigAVIInfoFrame(BYTE VIC, BYTE pixelrep)
{
//     AVI_InfoFrame AviInfo;
#if Debug_message
	HDMITX_DEBUG_PRINTF(("ConfigAVIInfoFrame(%d)\n",(int)VIC)) ;
#endif
    AviInfo.pktbyte.AVI_HB[0] = AVI_INFOFRAME_TYPE|0x80 ;
    AviInfo.pktbyte.AVI_HB[1] = AVI_INFOFRAME_VER ;
    AviInfo.pktbyte.AVI_HB[2] = AVI_INFOFRAME_LEN ;

    AviInfo.pktbyte.AVI_DB[0] = (0<<5)|(1<<4); // RGB Only

    AviInfo.pktbyte.AVI_DB[1] = 8 ;
    if(VIC )
        AviInfo.pktbyte.AVI_DB[1] |= (aspec != HDMI_16x9)?(1<<4):(2<<4); // 4:3 or 16:9
    else
        AviInfo.pktbyte.AVI_DB[1] &=(0xcf);

    AviInfo.pktbyte.AVI_DB[1] |= (Colorimetry != HDMI_ITU709)?(1<<6):(2<<6); // 4:3 or 16:9
    AviInfo.pktbyte.AVI_DB[2] = 0 ;
    AviInfo.pktbyte.AVI_DB[3] = VIC ;
    AviInfo.pktbyte.AVI_DB[4] =  pixelrep & 3 ;
    AviInfo.pktbyte.AVI_DB[5] = 0 ;
    AviInfo.pktbyte.AVI_DB[6] = 0 ;
    AviInfo.pktbyte.AVI_DB[7] = 0 ;
    AviInfo.pktbyte.AVI_DB[8] = 0 ;
    AviInfo.pktbyte.AVI_DB[9] = 0 ;
    AviInfo.pktbyte.AVI_DB[10] = 0 ;
    AviInfo.pktbyte.AVI_DB[11] = 0 ;
    AviInfo.pktbyte.AVI_DB[12] = 0 ;

    EnableAVIInfoFrame(TRUE, (unsigned char *)&AviInfo);
}


void ConfigfHdmiVendorSpecificInfoFrame(BYTE _3D_Stru)
{
    VS_Info.pktbyte.VS_HB[0] = VENDORSPEC_INFOFRAME_TYPE|0x80;
    VS_Info.pktbyte.VS_HB[1] = VENDORSPEC_INFOFRAME_VER;

    VS_Info.pktbyte.VS_DB[1] = 0x03;
    VS_Info.pktbyte.VS_DB[2] = 0x0C;
    VS_Info.pktbyte.VS_DB[3] = 0x00;
    VS_Info.pktbyte.VS_DB[4] = 0x40;
    switch(_3D_Stru)
    {
    case Side_by_Side:
        VS_Info.pktbyte.VS_HB[2] = 6;
        VS_Info.pktbyte.VS_DB[5] = (_3D_Stru<<4);
        VS_Info.pktbyte.VS_DB[6] = 0x00;
        break;

    case Top_and_Botton:
    case Frame_Packing:
        VS_Info.pktbyte.VS_HB[2] = 5;
        VS_Info.pktbyte.VS_DB[5] = (_3D_Stru<<4);
        break ;
    default:
        EnableVSInfoFrame(FALSE,NULL);
        return;
    }

    EnableVSInfoFrame(TRUE,(BYTE *)&VS_Info);
}

////////////////////////////////////////////////////////////////////////////////
// Function: ConfigAudioInfoFrm
// Parameter: NumChannel, number from 1 to 8
// Return: ER_SUCCESS for successfull.
// Remark: Evaluate. The speakerplacement is only for reference.
//         For production, the caller of SetAudioInfoFrame should program
//         Speaker placement by actual status.
// Side-Effect:
////////////////////////////////////////////////////////////////////////////////

void
ConfigAudioInfoFrm(void)
{
    int i ;
#if 1//Debug_message
    USART1_printf(USART1,"ConfigAudioInfoFrm\n",0);
#endif
    AudioInfo.pktbyte.AUD_HB[0] = AUDIO_INFOFRAME_TYPE ;
    AudioInfo.pktbyte.AUD_HB[1] = 1 ;
    AudioInfo.pktbyte.AUD_HB[2] = AUDIO_INFOFRAME_LEN ;
    AudioInfo.pktbyte.AUD_DB[0] = 1 ;
    for( i = 1 ;i < AUDIO_INFOFRAME_LEN ; i++ )
    {
        AudioInfo.pktbyte.AUD_DB[i] = 0 ;
    }
    //AudioInfo.pktbyte.AUD_DB[3] = 0x1f ;
    EnableAudioInfoFrame(TRUE, (unsigned char *)&AudioInfo);
}



/////////////////////////////////////////////////////////////////////
// ParseEDID()
// Check EDID check sum and EDID 1.3 extended segment.
/////////////////////////////////////////////////////////////////////

BYTE
ParseEDID(void)
{
    // collect the EDID ucdata of segment 0
    BYTE CheckSum ;
    BYTE BlockCount ;
    BYTE err ;
    //BYTE bValidCEA  ; //leo ???
    BYTE i ;
    BYTE MoniCnt;
    RxCapability.ValidCEA = FALSE ;
    RxCapability.ValidHDMI = FALSE ;
    bForceCTS = FALSE;
	
//bValidCEA = FALSE ; //leo ???
    GetEDIDData(0, cBuf);


    for( i = 0, CheckSum = 0 ; i < 128 ; i++ )
    {
        CheckSum += cBuf[i] ; CheckSum &= 0xFF ;
    }

			//Eep_Write(0x80, 0x80, EDID_Buf);
	if( CheckSum != 0 )
	{
		return FALSE ;
	}

	if( cBuf[0] != 0x00 ||
	    cBuf[1] != 0xFF ||
	    cBuf[2] != 0xFF ||
	    cBuf[3] != 0xFF ||
	    cBuf[4] != 0xFF ||
	    cBuf[5] != 0xFF ||
	    cBuf[6] != 0xFF ||
	    cBuf[7] != 0x00)
    {
        return FALSE ;
    }
  //  printf("[0x08+0]=%X\n[0x08+1]=%X\n[0x08+2]=%X\n[0x08+3]=%X\n[0x08+4]=%X\n[0x08+5]=%X\n[0x08+6]=%X\n[0x08+7]=%X\n",
 //       (WORD)cBuf[0x08+0],(WORD)cBuf[0x08+1],(WORD)cBuf[0x08+2],(WORD)cBuf[0x08+3],
  //      (WORD)cBuf[0x08+4],(WORD)cBuf[0x08+5],(WORD)cBuf[0x08+6],(WORD)cBuf[0x08+7]);
    for(MoniCnt=0;MoniCnt<UNKNOW_Monitor;MoniCnt++)
    {
        BYTE ccCnt;
        for(ccCnt=0;ccCnt<8;ccCnt++)
        {
            if(cBuf[0x08+ccCnt] != ForceCTS_EDID_Vindeo[MoniCnt].VIDC[ccCnt])
            {
                break;
            }

        }
        if(ccCnt==8)
        {
            bForceCTS = TRUE;
            break;
        }
    }
/*
    if(bForceCTS)
    {//printf("bForceCTS = TRUE\n");
    }
    else
    {//printf("bForceCTS = FALSE\n");
    }
*/
    BlockCount = cBuf[0x7E] ;

    if( BlockCount == 0 )
    {
        return TRUE ; // do nothing.
    }
    else if ( BlockCount > 4 )
    {
        BlockCount = 4 ;
    }

    RxCapability.ValidHDMI = FALSE ;
    // read all segment for test
    for( i = 1 ; i <= BlockCount ; i++ )
    {
        err = GetEDIDData(i, cBuf);

        if( err )
        {
           if( !RxCapability.ValidHDMI && cBuf[0] == 0x2 && cBuf[1] == 0x3 )
            {
                err = ParseCEAEDID(cBuf);
                if( err )
                {

				    if(RxCapability.IEEEOUI==0x0c03)
				    {
				    	//bValidCEA = TRUE ; //leo ?????
					}

                }
            }
        }
    }
    for(i=0;i<128;i++)
        cBuf[i]=0;
    return err ;

}


static BOOL
ParseCEAEDID(BYTE *pCEAEDID)
{
    BYTE offset,End ;
    BYTE count ;
    BYTE tag ;
    int i ;

    if( pCEAEDID[0] != 0x02 || pCEAEDID[1] != 0x03 ) return ER_SUCCESS ; // not a CEA BLOCK.
    End = pCEAEDID[2]  ; // CEA description.
    RxCapability.VideoMode = pCEAEDID[3] ;

	RxCapability.VDOModeCount = 0 ;
    RxCapability.idxNativeVDOMode = 0xff ;

    for( offset = 4 ; offset < End ; )
    {
        tag = pCEAEDID[offset] >> 5 ;
        count = pCEAEDID[offset] & 0x1f ;
        switch( tag )
        {
        case 0x01: // Audio Data Block ;
            RxCapability.AUDDesCount = count/3 ;
            offset++ ;
            for( i = 0 ; i < RxCapability.AUDDesCount ; i++ )
            {
                RxCapability.AUDDes[i].uc[0] = pCEAEDID[offset++] ;
                RxCapability.AUDDes[i].uc[1] = pCEAEDID[offset++] ;
                RxCapability.AUDDes[i].uc[2] = pCEAEDID[offset++] ;
            }

            break ;

        case 0x02: // Video Data Block ;
            //RxCapability.VDOModeCount = 0 ;
            offset ++ ;
            for( i = 0,RxCapability.idxNativeVDOMode = 0xff ; i < count ; i++, offset++ )
            {
            	BYTE VIC ;
            	VIC = pCEAEDID[offset] & (~0x80);
            	// if( FindModeTableEntryByVIC(VIC) != -1 )
            	{
	                RxCapability.VDOMode[RxCapability.VDOModeCount] = VIC ;
	                if( pCEAEDID[offset] & 0x80 )
	                {
	                    RxCapability.idxNativeVDOMode = (BYTE)RxCapability.VDOModeCount ;
	                    iVideoModeSelect = RxCapability.VDOModeCount ;
	                }

	                RxCapability.VDOModeCount++ ;
            	}
            }
            break ;

        case 0x03: // Vendor Specific Data Block ;
            for(i=0;i<count;i++)
            {
                RxCapability.HDMI_VSDB[i]=pCEAEDID[i+offset];
            }
#if Debug_message
            for(i=0;i<count;i++)
            {
                HDMITX_DEBUG_PRINTF(("HDMI_VSDB[%d]= %X\n",i,(WORD)RxCapability.HDMI_VSDB[i]));
            }
#endif

            offset ++ ;
            RxCapability.IEEEOUI = (ULONG)pCEAEDID[offset+2] ;
            RxCapability.IEEEOUI <<= 8 ;
            RxCapability.IEEEOUI += (ULONG)pCEAEDID[offset+1] ;
            RxCapability.IEEEOUI <<= 8 ;
            RxCapability.IEEEOUI += (ULONG)pCEAEDID[offset] ;
            offset += count ; // ignore the remaind.

            if(RxCapability.IEEEOUI==0x0c03)
            {
                RxCapability.ValidHDMI = TRUE ;
            }
            break ;

        case 0x04: // Speaker Data Block ;
            offset ++ ;
            RxCapability.SpeakerAllocBlk.uc[0] = pCEAEDID[offset] ;
            RxCapability.SpeakerAllocBlk.uc[1] = pCEAEDID[offset+1] ;
            RxCapability.SpeakerAllocBlk.uc[2] = pCEAEDID[offset+2] ;
            offset += 3 ;
            break ;
        case 0x05: // VESA Data Block ;
            offset += count+1 ;
            break ;
        case 0x07: // Extended Data Block ;
            offset += count+1 ; //ignore
            break ;
        default:
            offset += count+1 ; // ignore
        }
    }
    RxCapability.ValidCEA = TRUE ;
    return TRUE ;
}




