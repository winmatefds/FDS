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
// MODULE: 		Lpm_Nvram.c
//
// USAGE : 	Standard low level NVRAM read / write functions to
//          	accommodate 4K i2c NVRAM
//				(Use 2 bytes of address)
//
// Public functions defined in this module:
//
//          Lpm_NvramCheckDevice,
//          Lpm_NvramReadByte,
//          Lpm_NvramWriteByte,
//          Lpm_NvramReadBuffer,
//          Lpm_NvramWriteBuffer,
//          NVRam_GetSize
//
//******************************************************************************

//******************************************************************************
//  I N C L U D E    F I L E S
//******************************************************************************
#include ".\LowPowerMonitor\Lpm_All.h"


#ifdef I2C_MASTER_SUPPORT

//******************************************************************************
//  G L O B A L    V A R I A B L E S
//******************************************************************************

#define DEVICE_CHECK_LIMIT    255      // number of times to check

BYTE gB_NVRamError;                    // NVram error bit indicator

//******************************************************************************
//  C O D E
//******************************************************************************

//******************************************************************************
//
// FUNCTION       : gmt_RET_STAT Lpm_NvramCheckDevice(WORD W_RegAddr)
//
// USAGE          : Check if device present at i2c address by sending START
//                  to the device address.
//
// INPUT          : W_RegAddr - dummy variable
//
// OUTPUT         : OK 		- device acknowledges
//                  ERR_BUSY - device is busy
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
gmt_RET_STAT far Lpm_NvramCheckDevice(WORD W_RegAddr)
{
   BYTE B_StartLimit = DEVICE_CHECK_LIMIT;

   W_RegAddr++;								// NOT used

   while(B_StartLimit--)
   {
      if (Lpm_I2CMasterCheck(0, NVRAM_DEVICE) == gmd_PASS)
         return (OK);
   }
   //gm_Print("*** NVRAM timeout error ***\n\n",0);
   return (ERR_BUSY);
}

//******************************************************************************
//
// FUNCTION       : BYTE Lpm_NvramReadByte(WORD W_RegAddr)
//
// USAGE          : Read a byte from specified address in NVRAM.
//                  The function sends the page address and data
//                  address within that page. After that indicates
//                  a read from the same page and receives the data
//                  byte.
//
// INPUT          : W_RegAddr - NVRAM register address
//
// OUTPUT         : Value if operation is successful or 0 if failed.
//
// GLOBALS        : gB_NVRamError (W)
//
// USED_REGS      : None
//
// PRE_CONDITION  : 
//
// POST_CONDITION : 
//
//******************************************************************************
BYTE far Lpm_NvramReadByte(WORD W_RegAddr)
{
   BYTE B_Data,Ba_RegAddr[2];
   gmt_RET_STAT W_Result;

   #if(NVRAM_ADDR_LEN == 1)
   Ba_RegAddr[0] = (BYTE)(W_RegAddr);
   #else
   Ba_RegAddr[0] = (BYTE)(W_RegAddr >> 8);		// Upper WORD sub-address
   Ba_RegAddr[1] = (BYTE)(W_RegAddr);			// Lower WORD sub-address
   #endif

   Lpm_I2CMasterCsmSel(1);

   if (Lpm_NvramCheckDevice(0) == ERR_BUSY)		// Wait for device available
   {
      Lpm_I2CMasterCsmSel(0);
      return (ERR_BUSY);
   }

   W_Result = Lpm_I2CMasterReadAddrBuffer(NVRAM_DEVICE, Ba_RegAddr, NVRAM_ADDR_LEN, &B_Data, 1);

   Lpm_I2CMasterCsmSel(0);

   if (W_Result == OK)							// check result
   {                                           // OK
      gB_NVRamError = 0;						// Clear error
      return (B_Data);						// Return read data
   }
   else
   {
      gB_NVRamError = 1;						// Set error
      return (0);								// Return dummy
   }
}

//******************************************************************************
//
// FUNCTION       : gmt_RET_STAT Lpm_NvramWriteByte(WORD W_RegAddr, BYTE B_Data)
//
// USAGE          : Write a byte to specified address in NVRAM.
//                  The function sends the page address and data
//                  address within that page. After that indicates
//                  a write at the same page and sends the data
//                  byte.
//
// INPUT          : W_RegAddr - register address
//                  B_Data 	  - register data
//
// OUTPUT         : OK 		  = command sucessful
//                  otherwise = error
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
gmt_RET_STAT far Lpm_NvramWriteByte(WORD W_RegAddr, BYTE B_Data)
{
   BYTE Ba_RegAddr[2];
   gmt_RET_STAT W_Result;

   #if(NVRAM_ADDR_LEN == 1)
   Ba_RegAddr[0] = (BYTE)(W_RegAddr);
   #else
   Ba_RegAddr[0] = (BYTE)(W_RegAddr >> 8);		// Upper WORD sub-address
   Ba_RegAddr[1] = (BYTE)(W_RegAddr);			// Lower WORD sub-address
   #endif

   Lpm_I2CMasterCsmSel(1);

   if (Lpm_NvramCheckDevice(0) == ERR_BUSY)		// Wait for device available
   {
      Lpm_I2CMasterCsmSel(0);
      return (ERR_BUSY);
   }

   // 20100204 #1, For new RD board hang issue, show massage for debug.
   Lpm_TimerStart(NVRAM_1ms_TMR, 500);
   //
   // Write to EEPROM and make sure success
   //
   do
   {
      W_Result = Lpm_I2CMasterWriteAddrBuffer(NVRAM_DEVICE, Ba_RegAddr, NVRAM_ADDR_LEN, &B_Data, 1);

      if (Lpm_TimerTimeOut(NVRAM_1ms_TMR))	//500ms for timeout
      {
         //gm_Print("NVRAM error, system stop %d", 1);
         break;
      }

   }while (W_Result != OK);

   Lpm_I2CMasterCsmSel(0);

   return (W_Result);							// Return status
}

//******************************************************************************
//
// FUNCTION       : gmt_RET_STAT  Lpm_NvramReadBuffer(WORD W_RegAddr, BYTE *Bp_Buf, WORD W_Length)
//
// USAGE          : Read a buffer from specified address in NVRAM.
//                  The function checks first if the block length
//                  is not zero, if the data length does not exceed
//                  the total nvram capacity and if everything is OK
//                  reads the data block.
//
// INPUT          : W_RegAddr - register address
//                  Bp_Buf	  - Buffer pointer
//                  W_Length  - Number of byte to read
//
// OUTPUT         : OK 		  = successful
//                  otherwise = failed
//
// GLOBALS        : gB_NVRamError (W)
//
// USED_REGS      : None
//
// PRE_CONDITION  : 
//
// POST_CONDITION : 
//
//******************************************************************************
gmt_RET_STAT far Lpm_NvramReadBuffer(WORD W_RegAddr, BYTE far*Bp_Buf, WORD W_Length)
{
   gmt_RET_STAT W_Result;
   BYTE Ba_RegAddr[2];

   #if(NVRAM_ADDR_LEN == 1)
   Ba_RegAddr[0] = (BYTE)(W_RegAddr);
   #else
   Ba_RegAddr[0] = (BYTE)(W_RegAddr >> 8);		// Upper WORD sub-address
   Ba_RegAddr[1] = (BYTE)(W_RegAddr);			// Lower WORD sub-address
   #endif

   Lpm_I2CMasterCsmSel(1);

   if (Lpm_NvramCheckDevice(0) == ERR_BUSY)		// Wait for device available
   {
      Lpm_I2CMasterCsmSel(0);
      return (ERR_BUSY);
   }

   W_Result = Lpm_I2CMasterReadAddrBuffer(NVRAM_DEVICE, Ba_RegAddr, NVRAM_ADDR_LEN, Bp_Buf, W_Length);

   Lpm_I2CMasterCsmSel(0);

   if (W_Result == OK)
      gB_NVRamError = 0;						// Clear error
   else
      gB_NVRamError = 1;						// Set error
      
   return (W_Result);							// Return read data
}

//******************************************************************************
//
// FUNCTION       : gmt_RET_STAT  Lpm_NvramWriteBuffer(WORD W_RegAddr, BYTE *Bp_Buf, WORD W_Length)
//
// USAGE          : Write a buffer to specified address in NVRAM.
//                  The function checks first if the block length
//                  is not zero, if the data length does not exceed
//                  the total nvram capacity and if everything is OK,
//                  writes the data block.
//
// INPUT          : W_RegAddr - register address
//                  Bp_Buf	  - Buffer pointer
//                  W_Length  - Number of byte to read
//
// OUTPUT         : OK 		  = successful
//                  otherwise = failed
//
// GLOBALS        : gB_NVRamError  (W)
//
// USED_REGS      : None
//
// PRE_CONDITION  : 
//
// POST_CONDITION : 
//
//******************************************************************************
gmt_RET_STAT far Lpm_NvramWriteBuffer(WORD W_RegAddr, BYTE far*Bp_Buf, WORD W_Length)
{
   BYTE B_Count, Ba_RegAddr[2], B_PageSize;
   gmt_RET_STAT W_Result;

   if (Bp_Buf == 0 || W_Length == 0)			// Input parameter OK ?
      return ERR_PARAM;						
   							
   gB_NVRamError = 0;							// Clear error
   W_Result = ERR_WRITE;						// Set error flag


   Lpm_I2CMasterCsmSel(1);
   //
   // Write buffer of data
   //
   while(W_Length)								// Data still available
   {
      //
      // Number of bytes available in current page.
      //
      B_PageSize = NVRAM_PAGE_SIZE - (W_RegAddr % NVRAM_PAGE_SIZE);
      //
      // Is current page has enough byte for the buffer.
      //
      if (W_Length > B_PageSize)
      {
         //
         // Open new page?
         //
         if (B_PageSize == NVRAM_PAGE_SIZE)		// No
         {
            //
            // CASE3: The rest of the buffer is more than a page size.
            //
            B_Count = NVRAM_PAGE_SIZE;    	    // Yes
         }
         else
         {
            //
            // CASE2: Current page has not enough byte for the buffer,
            //        write some data in this page and the rest in other
            //		  page(s).
            //
            B_Count = B_PageSize;				// No
         }
      }
      else
      {
         //
         // CASE1: Current page has enough byte for the buffer.
         // CASE4: Finished up the rest of the buffer.
         //
         B_Count = W_Length;						// Yes
      }

      #if(NVRAM_ADDR_LEN == 1)
      Ba_RegAddr[0] = (BYTE)(W_RegAddr);
      #else
      Ba_RegAddr[0] = (BYTE)(W_RegAddr >> 8);		// Upper WORD sub-address
      Ba_RegAddr[1] = (BYTE)(W_RegAddr);			// Lower WORD sub-address
      #endif

      //
      // Write to EEPROM and make sure success
      //
      if (Lpm_NvramCheckDevice(0) == ERR_BUSY)		// Wait for device available
      {
         Lpm_I2CMasterCsmSel(0);
            return (ERR_BUSY);
      }


      Lpm_TimerStart(NVRAM_1ms_TMR, 500);	//500ms for timeout
      do
      {
         W_Result = Lpm_I2CMasterWriteAddrBuffer(NVRAM_DEVICE, Ba_RegAddr, NVRAM_ADDR_LEN,
                                                Bp_Buf, B_Count);

      if (Lpm_TimerTimeOut(NVRAM_1ms_TMR))
      {
         //gm_Print("NVRAM error, system stop %d", 2);
         break;
      }

      }while (W_Result != OK);

      Bp_Buf	 += B_Count; 						// Adjust pointer to B_Count
      W_RegAddr += B_Count;						// Adjust register address
      W_Length  -= B_Count;						// Adjust buffer count
   }

   Lpm_I2CMasterCsmSel(0);

   return (W_Result);
}

#else
gmt_RET_STAT far Lpm_NvramCheckDevice(WORD W_RegAddr)
{
   UNUSED_VAR(W_RegAddr);
   return ERR_BUSY;
}
BYTE far Lpm_NvramReadByte(WORD W_RegAddr)
{
   UNUSED_VAR(W_RegAddr);
   return 0;
}
gmt_RET_STAT far Lpm_NvramWriteByte(WORD W_RegAddr, BYTE B_Data)
{
   UNUSED_VAR(W_RegAddr);
   UNUSED_VAR(B_Data);   
   return ERR_BUSY;
}
gmt_RET_STAT far Lpm_NvramReadBuffer(WORD W_RegAddr, BYTE far*Bp_Buf, WORD W_Length)
{
   UNUSED_VAR(W_RegAddr);
   UNUSED_VAR(Bp_Buf); 
   UNUSED_VAR(W_Length);
   return ERR_BUSY;
}
gmt_RET_STAT far Lpm_NvramWriteBuffer(WORD W_RegAddr, BYTE far*Bp_Buf, WORD W_Length)
{
   UNUSED_VAR(W_RegAddr);
   UNUSED_VAR(Bp_Buf); 
   UNUSED_VAR(W_Length);
   return ERR_BUSY;
}
#endif	//#ifdef I2C_MASTER_SUPPORT

//**********************************  END  *************************************
