#ifndef _MODEHANDLER_HPP_
#define _MODEHANDLER_HPP_
// modehandler.h //




 #define BITFIELD                WORD
#define gmd_AUD_TRUE            TRUE
#define BITFIELD_BYTE_SIZE      2
#define SAP_STEREO_LIMIT        0x20

typedef struct
{
   WORD  W_InMatrix      : 4;
}gmt_ProcDescType;

#define  gmt_AUDIO_SOUND_STD gme_AudioSoundStandard

#define  gmd_AUD_FALSE FALSE
#define  FIX_FOR_NICAM_MONO
#define MAX_NUM_OUTPUT_CHANNELS  4





typedef enum
{
   INST_STATE_OFF,
   INST_STATE_INIT,
   INST_STATE_MODE_CHANGE,
   INST_STATE_UNSTABLE,
   INST_STATE_DETERMINE_MODE,
   INST_STATE_STABLE,
}InstanceState_t;


typedef enum
{
   AUD_TYPE_MX_MONO      = 0x0,
   AUD_TYPE_MX_STEREO    = 0x1,
   AUD_TYPE_MX_SIF_AUX   = 0x2, // Source is SIF aux
}AudType_SIF;

typedef enum
{
gmd_SAP_DUAL_B = 0,
gmd_NON_SAP_DUAL_B = 1,
} gmt_SAP_DUALB_SELECT;

#define SIF_CTRL SIF_CTRL_0
 #define SIF_DEMOD_STEREO SIF_DEMOD_STEREO_0
 #define SIF_DEMOD_SAP_DUAL SIF_DEMOD_SAP_DUAL_0

 #define SIF_MIXER1_PHASE_STEP0 SIF_MIXER1_PHASE_STEP0_0
 #define SIF_MIXER1_PHASE_STEP1 SIF_MIXER1_PHASE_STEP1_0
 #define SIF_MIXER2_PHASE_STEP0 SIF_MIXER2_PHASE_STEP0_0
 #define SIF_MIXER2_PHASE_STEP1 SIF_MIXER2_PHASE_STEP1_0
 #define SIF_MIXER2_PHASE_STEP2 SIF_MIXER2_PHASE_STEP2_0

 //WORK ON THIS..(We have to take this out from here....
 //#define SIF_OUT_AUX_SEL                        0xc
 #define SIF_CTRL_NICAM_ANA_AUX_SELECT          0x00
 #define SIF_CTRL_A2_SAP_AUX_SELECT             0x04
 #define SIF_CTRL_BTSC_SAP_AUX_SELECT           0x08
 //#define SIF_OUT_MAIN_SEL                       0x2
//#endif
//********************************
// Features:
//********************************
#define USE_NICAM_DIGITAL                    1 // disabled until nicam digital is supported in MPE
#define CALIBRATE_SIF_ADC_ON_FIRST_CHANNEL   0 // will calibrate sif adc on firt good channel.	

//#define NEW_AUDIO_API_FOR_SIF_HANDLING

void SIFMODE_HANDLER(gmt_Instance audInst);
void mode_update(void);

#ifdef NEW_AUDIO_API_FOR_SIF_HANDLING
void setDemodConfigVal(gmAud_DEMOD_CONFIG_VECTOR val);
BOOL isMutePrimaryChOnCarrierLossEnabled(void);
BOOL isMuteSecendaryChOnCarrierLossEnabled(void);
BOOL isHighDeviationOnFMEnabled(void);
BOOL isAutoSoundStandardDetectionEnabled(void);
BOOL isAutoFallbackToMonoEnabled(void);
#endif // NEW_AUDIO_API_FOR_SIF_HANDLING

#endif
