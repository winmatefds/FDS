#ifndef __CEA_861__
#define __CEA_861__
//Color space values of AVI InfoFrame (see CEA-861-D ,table  8)
typedef enum
{
    gmd_CEA861_AVI_Y_RGB = 0,
    gmd_CEA861_AVI_Y_YUV422,
    gmd_CEA861_AVI_Y_YUV444,
    gmd_CEA861_AVI_Y_RSVD
} gmt_CEA861_AVI_Y;

//Active information present field of AVI InfoFrame (CEA-861-D, table 8)
typedef enum
{
    gmd_CEA861_AVI_A_NODATA = 0,
    gmd_CEA861_AVI_A_DATA_VALID
} gmt_CEA861_AVI_A;

//Bar info data valid field of AVI InfoFrame (CEA-861-D, table 8)
typedef enum
{
    gmd_CEA861_AVI_B_NO_BAR = 0,
    gmd_CEA861_AVI_B_V_BAR,
    gmd_CEA861_AVI_B_H_BAR,
    gmd_CEA861_AVI_B_HV_BAR
} gmt_CEA861_AVI_B;

//Scan information field of AVI InfoFrame (CEA-861-D, table 8)
typedef enum
{
    gmd_CEA861_AVI_S_NO_DATA = 0,
    gmd_CEA861_AVI_S_OVERSCAN_TV,
    gmd_CEA861_AVI_S_UNDERSCAN,
    gmd_CEA861_AVI_S_RSVD
} gmt_CEA861_AVI_S;

//Colorimetry field of AVI InfoFrame (CEA-861-D, table 9)
typedef enum
{
    gmd_CEA861_AVI_C_NO_DATA = 0,
    gmd_CEA861_AVI_C_ITU_BT_601,
    gmd_CEA861_AVI_C_BT_709,
    gmd_CEA861_AVI_C_EXTENDED
} gmt_CEA861_AVI_C;

//Picture aspect ratio field (4:3, 16:9) of AVI InfoFrame (CEA-861-D, table 9)
typedef enum
{
    gmd_CEA861_AVI_M_NO_DATA = 0,
    gmd_CEA861_AVI_M_4_3,
    gmd_CEA861_AVI_M_16_9,
    gmd_CEA861_AVI_M_RSVD
} gmt_CEA861_AVI_M;

//Non-uniform picture scaling field (4:3, 16:9) of AVI InfoFrame (CEA-861-D, table 11)
typedef enum
{
    gmd_CEA861_AVI_SC_NONE = 0,
    gmd_CEA861_AVI_SC_H,
    gmd_CEA861_AVI_SC_V,
    gmd_CEA861_AVI_SC_HV,
} gmt_CEA861_AVI_SC;

//Audio coding type (CEA-861-D, table 17)
typedef enum
{
    gmd_CEA861_AUDIO_CT_NONE = 0,
    gmd_CEA861_AUDIO_CT_PCM,
    gmd_CEA861_AUDIO_CT_AC3,
    gmd_CEA861_AUDIO_CT_MPEG1,
    gmd_CEA861_AUDIO_CT_MP3,
    gmd_CEA861_AUDIO_CT_MPEG2,
    gmd_CEA861_AUDIO_CT_AAC,
    gmd_CEA861_AUDIO_CT_DTS,
    gmd_CEA861_AUDIO_CT_ATRAC,
    gmd_CEA861_AUDIO_CT_DSD,    // One Bit Audio
    gmd_CEA861_AUDIO_CT_DOLBY_DIGITAL_PLUS,
    gmd_CEA861_AUDIO_CT_DTS_HD,
    gmd_CEA861_AUDIO_CT_MAT,
    gmd_CEA861_AUDIO_CT_DST,
    gmd_CEA861_AUDIO_CT_WMA_PRO,
    gmd_CEA861_AUDIO_CT_RSVD
} gmt_CEA861_AUDIO_CT;
#endif //#ifndef __CEA_861__