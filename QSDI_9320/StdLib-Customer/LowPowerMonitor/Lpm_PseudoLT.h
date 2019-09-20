#ifndef __LPM_PSEUDOLT_H__
#define __LPM_PSEUDOLT_H__

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
// MODULE:      Lpm_PseudoLT.h
//
// USAGE:       
//
//******************************************************************************

void Lpm_PseudoAuxNativeRequest(BYTE B_Request, DWORD DW_Addr, BYTE  B_Len, AUX_CH B_AuxCh);


    #define DPRX_DPCD_MAX_LINK_RATE               0x0001
    #define DPRX_DPCD_MAX_LANE_COUNT              0x0002
    #define DPRX_DPCD_MAX_DOWNSPREAD              0x0003
    #define DPRX_DPCD_NORP                        0x0004
    #define DPRX_DPCD_TX_DWNSTRMPRT_PRSNT         0x0005
    #define DPRX_DPCD_MAIN_LINK_CH_CODE           0x0006
    #define DPRX_DPCD_DWNSTRM_PORT_COUNT          0x0007
    #define DPRX_DPCD_P0_CAPBL0                   0x0008
    #define DPRX_DPCD_P0_CAPBL1                   0x0009
    #define DPRX_DPCD_P1_CAPBL0                   0x000A
    #define DPRX_DPCD_P1_CAPBL1                   0x000B
    #define DPRX_DPCD_DWNSTRM_P0_CAPBLTY          0x0080
    #define DPRX_DPCD_DWNSTRM_P1_CAPBLTY          0x0081
    #define DPRX_DPCD_DWNSTRM_P2_CAPBLTY          0x0082
    #define DPRX_DPCD_DWNSTRM_P3_CAPBLTY          0x0083
    #define DPRX_DPCD_DWNSTRM_P4_CAPBLTY          0x0084
    #define DPRX_DPCD_DWNSTRM_P5_CAPBLTY          0x0085
    #define DPRX_DPCD_DWNSTRM_P6_CAPBLTY          0x0086
    #define DPRX_DPCD_DWNSTRM_P7_CAPBLTY          0x0087
    #define DPRX_DPCD_DWNSTRM_P8_CAPBLTY          0x0088
    #define DPRX_DPCD_DWNSTRM_P9_CAPBLTY          0x0089
    #define DPRX_DPCD_DWNSTRM_P10_CAPBLTY         0x008A
    #define DPRX_DPCD_DWNSTRM_P11_CAPBLTY         0x008B
    #define DPRX_DPCD_DWNSTRM_P12_CAPBLTY         0x008C
    #define DPRX_DPCD_DWNSTRM_P13_CAPBLTY         0x008D
    #define DPRX_DPCD_DWNSTRM_P14_CAPBLTY         0x008E
    #define DPRX_DPCD_DWNSTRM_P15_CAPBLTY         0x008F
    #define DPRX_DPCD_LINK_BW_SET                 0x0100
    #define DPRX_DPCD_LANE_COUNT_SET              0x0101
    #define DPRX_DPCD_TRAINING_PATRN_SET          0x0102
    #define DPRX_DPCD_TRAINING_L0_SET             0x0103
    #define DPRX_DPCD_TRAINING_L1_SET             0x0104
    #define DPRX_DPCD_TRAINING_L2_SET             0x0105	
    #define DPRX_DPCD_TRAINING_L3_SET             0x0106	
    #define DPRX_DPCD_DOWNSPREAD_CTRL             0x0107	
    #define DPRX_DPCD_MAIN_LINK_CHANNEL_CODING_SET 0x0108	
    #define DPRX_DPCD_SINK_STATUS0                0x0200	
    #define DPRX_DPCD_DEV_SRVC_IRQ_VECTR          0x0201	
    #define DPRX_DPCD_L1L0_STATUS                 0x0202	
    #define DPRX_DPCD_L3L2_STATUS                 0x0203	
    #define DPRX_DPCD_LANE_ALIGN_STATUS           0x0204	
    #define DPRX_DPCD_SINK_STATUS2                0x0205	
    #define DPRX_DPCD_L1L0_ADJ_REQ                0x0206	
    #define DPRX_DPCD_L3L2_ADJ_REQ                0x0207	
    #define DPRX_DPCD_L0_TRAINING_SCORE           0x0208	
    #define DPRX_DPCD_L1_TRAINING_SCORE           0x0209
    #define DPRX_DPCD_L2_TRAINING_SCORE           0x020A
    #define DPRX_DPCD_L3_TRAINING_SCORE           0x020B	
    #define DPRX_DPCD_L0_SYM_ERROR_COUNT_0        0x0210	
    #define DPRX_DPCD_L0_SYM_ERROR_COUNT_1        0x0211	
    #define DPRX_DPCD_L1_SYM_ERROR_COUNT_0        0x0212	
    #define DPRX_DPCD_L1_SYM_ERROR_COUNT_1        0x0213
    #define DPRX_DPCD_L2_SYM_ERROR_COUNT_0        0x0214
    #define DPRX_DPCD_L2_SYM_ERROR_COUNT_1        0x0215
    #define DPRX_DPCD_L3_SYM_ERROR_COUNT_0        0x0216
    #define DPRX_DPCD_L3_SYM_ERROR_COUNT_1        0x0217
    #define DPRX_DPCD_TEST_REQ                    0x0218
    #define DPRX_DPCD_TEST_LINK_RATE              0x0219
    #define DPRX_DPCD_TEST_LANE_COUNT             0x0220
    #define DPRX_DPCD_TEST_PATTERN                0x0221
    #define DPRX_DPCD_TEST_H_TOTAL_H              0x0222
    #define DPRX_DPCD_TEST_H_TOTAL_L              0x0223
    #define DPRX_DPCD_TEST_V_TOTAL_H              0x0224
    #define DPRX_DPCD_TEST_V_TOTAL_L              0x0225
    #define DPRX_DPCD_TEST_H_START_H              0x0226
    #define DPRX_DPCD_TEST_H_START_L              0x0227
    #define DPRX_DPCD_TEST_V_START_H              0x0228
    #define DPRX_DPCD_TEST_V_START_L              0x0229
    #define DPRX_DPCD_TEST_H_SYNC_H               0x022A
    #define DPRX_DPCD_TEST_H_SYNC_L               0x022B
    #define DPRX_DPCD_TEST_V_SYNC_H               0x022C
    #define DPRX_DPCD_TEST_V_SYNC_L               0x022D
    #define DPRX_DPCD_TEST_H_WIDTH_H              0x022E
    #define DPRX_DPCD_TEST_H_WIDTH_L              0x022F
    #define DPRX_DPCD_TEST_V_HEIGHT_H             0x0230
    #define DPRX_DPCD_TEST_V_HEIGHT_L             0x0231
    #define DPRX_DPCD_TEST_MISC_0                 0x0232
    #define DPRX_DPCD_TEST_MISC_1                 0x0233
    #define DPRX_DPCD_TEST_RFRSH_NUMERATOR        0x0234
    #define DPRX_DPCD_TEST_CRC_R_CR_0             0x0240
    #define DPRX_DPCD_TEST_CRC_R_CR_1             0x0241
    #define DPRX_DPCD_TEST_CRC_G_Y_0              0x0242
    #define DPRX_DPCD_TEST_CRC_G_Y_1              0x0243
    #define DPRX_DPCD_TEST_CRC_B_CB_0             0x0244
    #define DPRX_DPCD_TEST_CRC_B_CB_1             0x0245
    #define DPRX_DPCD_TEST_SINK_MISC              0x0246
    #define DPRX_DPCD_PHY_TEST_PATTERN            0x0248
    #define DPRX_DPCD_TEST_RESPONSE               0x0260
    #define DPRX_DPCD_TEST_EDID_CHEKSUM           0x0261
    #define DPRX_DPCD_TEST_SINK                   0x0270
    #define DPRX_DPCD_SOURCE_IEEE_OUI_0           0x0300
    #define DPRX_DPCD_SOURCE_IEEE_OUI_1           0x0301
    #define DPRX_DPCD_SOURCE_IEEE_OUI_2           0x0302
    #define DPRX_DPCD_SINK_IEEE_OUI_0             0x0400
    #define DPRX_DPCD_SINK_IEEE_OUI_1             0x0401
    #define DPRX_DPCD_SINK_IEEE_OUI_2             0x0402
    #define DPRX_DPCD_BRANCH_IEEE_OUI_0           0x0500
    #define DPRX_DPCD_BRANCH_IEEE_OUI_1           0x0501
    #define DPRX_DPCD_BRANCH_IEEE_OUI_2           0x0502
    #define DPRX_DPCD_SINK_SET_POWER              0x0600


#endif //__LPM_PSEUDOLT_H__
