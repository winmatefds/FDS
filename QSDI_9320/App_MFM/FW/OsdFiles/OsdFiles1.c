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
// MODULE:    OsdFiles.c
//
// USAGE:     OSD files container to incapsulate differences between Display ond TV OSD 
//
//******************************************************************************
#include "System\all.h"
#ifdef ST_4K2K_93xx_BOARD
    #include "OsdFiles_4k2k\Dynfonts.c"
    #include "OsdFiles_4k2k\osd_act.c"
#else
//Mars board

    #include "OsdFiles\Dynfonts.c"
    #include "OsdFiles\osd_act.c"
//    #include "OsdFiles\osd_cc.c"
#endif

//*********************************  END  **************************************
