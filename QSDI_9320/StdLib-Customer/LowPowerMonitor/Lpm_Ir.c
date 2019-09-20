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
// MODULE:      Lpm_Ir.c
//
// USAGE:       
//
//******************************************************************************
#include ".\LowPowerMonitor\Lpm_All.h"
#include <mem.h>

#ifdef IR_SUPPORT

static gmt_Ir * gmp_Ir;
extern BYTE IrPowerKey;

//******************************************************************************
//
// FUNCTION:		BOOL Lpm_IrHandler(void)
//
// USAGE:			This function decodes the IR commands, queues them on the OSD
//					queue, and returns TRUE if the power on/off command has been
//					received; returns FALSE if the power on/off command has not been
//					received.
//
// INPUT:			None
//
// OUTPUT:			TRUE - Power on/off command received
//					FALSE - Power on/off command has not been received
//
// GLOBALS:			CopiedKeypad_IR		(R)
//					B_PrevKey			(R/w)
//					DW_PrevKeyTime		(R/W)
//					CopiedIrProtocol	(R)
//
// USED_REGS:		None
//
// PRE_CONDITION: 
//
// POST_CONDITION: 
//
//******************************************************************************
extern BOOL powerUpRequest;
void Lpm_IrHandler(void)
{		
   if (gmp_Ir->valid == OK)
   {
      BYTE buf[3];
	  
      buf[0] = (BYTE)gmp_Ir->cmd;
      buf[1] = (BYTE)gmp_Ir->key;
      buf[2] = (BYTE)gmp_Ir->index;
      gmp_Ir->valid = ERR_NO_DATA;


	if(gmp_Ir->key==202)
	buf[1] =136;

      if (!IsMissionPoweredUp())
      {
         if ((gmp_Ir->key == 202) || (gmp_Ir->key == 136)) 
            Lpm_SetPowerKeyEvent();
         else 
         {
            if (Lpm_PowerModeGetState() == LPM_LowPowerMode)
               #ifdef ANY_KEY_WAKEUP
               Lpm_SetAnyKeyWakeEvent();
               #endif
         }
      } 
      else {		
        gm_MessageSend(gmd_MESSAGE_IR, buf, sizeof(buf));
      }
   }
}

//******************************************************************************
//
// FUNCTION:	void far Lpm_IrIsr(void)
//
// USAGE:		The default IR Interrupt Service Routine (ISR).  Gets data from 
//				hardware, passes bit timing to the appropriate Protocol State machine
//				(RECS80, RC5, etc.) Populates local driver variables.
//
// INPUT:		W_IrqStatus - Unused at this time
//
// OUTPUT:		None
//
// GLOBALS:		gmp_Ir->valid - Boolean indicating Valid IR Data
//              gmp_Ir->cmd - Holds Command/Address code
//              gmp_Ir->key - Holds raw Keycode
//              gmp_Ir->index - Holds Index of structure that decoded the packet.
//
// USED_REGS:	None
//                                         |-- B_IrWatchdog --|
//
//  ----+  +--+ +-+ +-+ +-+    +-+ +-+ +-+ +-------------//-------------+
//      |  |  | | | | | | |    | | | | | | |                            |
//      |  |  | | | | | | |    | | | | | | |                            |
//      +--+  +-+ +-+ +-+ +-//-+ +-+ +-+ +-+                            +--+
//
//      |-------------- Burst -------------|----------- Idle -----------|
//
//      |--------------------------- Period ----------------------------|
//
//
// RECS80 State Machine.
//
//     | L | H |T| 1 | |0| |0|  //
//     
// ----+   +---+ +---+ +-+ +-+  // 
//     |   |   | |   | | | | | 
//     |   |   | |   | | | | | 
//     +---+   +-+   +-+ +-+ +- //
//                  L = Start Low
//                  H = Start High
//                  T = Trough
//                  1 = Logic 1
//                  0 = Logic 0
//
// RC5 State Machine
//
//   | S | S | T | ? | ? | ? // | E |
//     
// --+-+ +-+ +-+ + +-+ +-+ +-+ + // + +-----//
//     | | | | |   | | | | | |        |
//     | | | | |   | | | | | |        |
//     +-+ +-+ +-+-+ +-+ +-+ +-+ // +-+
//                  S = Start Bit = 1 (11b = Normal Key, 10b = +64)
//                  T = Toggle Bit = Any
//                  ? = Any
//                  E = End Packet Forced after last activity
//
// PRE_CONDITION  : 
//
// POST_CONDITION : 
//
//******************************************************************************
#define IR_POLARITY_MASK 0x80
#define IR_BIT_WIDTH_MASK 0x7F

void Lpm_IrIsr(void)//derived from gmp_IrIsr()
{
	WORD W_IsrData = gm_ReadRegWord(IR_STATUS_DATA);
	if (W_IsrData & IR_DATA_OVERRUN) //Get Data from IR decoder
	{  // Reset FIFO on Overrun
    	gm_SetRegBitsWord(IR_CONFIG, CLEAR_BUFFERS);
      	return;
   	}
   	if ((W_IsrData & DATA_AVAILABLE) == 0) 
   	{
   		return; //no data
   	}  

   	// Application Callback
   	if (gmp_Ir->P_IrCallBack != NULL_PTR)
   	{
    	(*(gmp_Ir->P_IrCallBack))();
      	return;
   	}

   	{  // timeout to divide parcels
    	DWORD DW_Now = gm_GetSystemTime();

		if ((DW_Now - gmp_Ir->expTime) > (gmp_Ir->P_IrProtocol->B_Period / 2))
      	{     
        	BYTE j = gmp_Ir->P_IrProtocol->B_NumPackets;
         	while (j)
            	gmp_Ir->packetStates[j--].B_State = IR_STATE_START0;
      	}
      	gmp_Ir->expTime = DW_Now;
   	}

   {  // Apply the Bit Information to each Packet Type in the Definition
      BYTE j;     
      for (j = 0; j < gmp_Ir->P_IrProtocol->B_NumPackets; j++)
      {
         gmt_IrPacket const far* P_Packet = &(gmp_Ir->P_IrProtocol->B_Packets[j]);
         gmt_IrPacketState * P_PacketState = &(gmp_Ir->packetStates[j]);                
         BYTE B_BitWidth = (BYTE)(W_IsrData & IR_BIT_WIDTH_MASK); // Raw Data
         BOOL isPolarityHigh = W_IsrData & IR_POLARITY_MASK; 
            
         if (gmp_Ir->P_IrProtocol->B_Protocol == IR_PROTOCOL_RECS80)
         {
            //status = (*gFp_IrRECS80StateMachine)(&W_Cmd, &W_Data, j);

            // Advance Packet State for 0 Length Start Packet Types
            if ((P_PacketState->B_State == IR_STATE_START0) 
               && (P_Packet->B_Start0Min == 0) 
               && (P_Packet->B_Start0Max == 0))
            {
               P_PacketState->B_State = IR_STATE_START1;
            }

            if ((P_PacketState->B_State == IR_STATE_START1) 
               && (P_Packet->B_Start1Min == 0) 
               && (P_Packet->B_Start1Max == 0))
            {
               P_PacketState->B_State = IR_STATE_TROUGH;
            }
            
            if (P_PacketState->B_State == IR_STATE_START0) // RECS80 - Preamble - Start Lo
            {
               if ((!isPolarityHigh)
                  && (B_BitWidth >= P_Packet->B_Start0Min)
                  && (B_BitWidth <= P_Packet->B_Start0Max))
                  P_PacketState->B_State = IR_STATE_START1;
            }
            else if (P_PacketState->B_State == IR_STATE_START1) // RECS80 - Preamble - Start Hi
            {
               P_PacketState->B_State = ( isPolarityHigh && (B_BitWidth >= P_Packet->B_Start1Min) &&
                  (B_BitWidth <= P_Packet->B_Start1Max)) ? IR_STATE_TROUGH : IR_STATE_START0;
            }
            else if (P_PacketState->B_State ==IR_STATE_TROUGH) // RECS80 - Preamble - Trough
            {  // low state following the start lo and start hi bits.
               if ( (!isPolarityHigh) && (B_BitWidth >= P_Packet->B_TroughMin) && (B_BitWidth <= P_Packet->B_TroughMax))
               {
                  P_PacketState->B_State = IR_STATE_PAYLOAD; 
                  P_PacketState->B_StateCnt = 0; //init fields
                  P_PacketState->D_Payload = 0;       
                  P_PacketState->B_SpecialKey = 0;
                  P_PacketState->W_Address = 0;
               }
               else
                  P_PacketState->B_State = IR_STATE_START0;
            }
            else if (P_PacketState->B_State == IR_STATE_PAYLOAD) // RECS80 - Payload
            {  // Hi/Lo states following the Preamble
               // Start Hi in Payload area (Separater for Global Data)
               if ((P_Packet->B_Flags & IR_FLAG_MULT_START) && isPolarityHigh
                  && (B_BitWidth >= P_Packet->B_Start1Min) && (B_BitWidth <= P_Packet->B_Start1Max))
               {  // Aquire Address early
                  P_PacketState->W_Address = (WORD)((P_PacketState->D_Payload & P_Packet->D_AddrMask) >> P_Packet->B_AddrShift);
               }
               // Bits are High
               else if (isPolarityHigh)
               {
                  // '0'
                  if ((B_BitWidth >= P_Packet->B_Data0Min) && (B_BitWidth <= P_Packet->B_Data0Max))
                  {
                     P_PacketState->D_Payload = P_PacketState->D_Payload << 1;
                     ++P_PacketState->B_StateCnt;
                  }
                  // '1'
                  else if ((B_BitWidth >= P_Packet->B_Data1Min) && (B_BitWidth <= P_Packet->B_Data1Max))
                  {
                     P_PacketState->D_Payload = (P_PacketState->D_Payload << 1) | 0x00000001L;
                     ++P_PacketState->B_StateCnt;
                  }
                  // Error
                  else
                  {
                     P_PacketState->B_State = IR_STATE_START0;
                  }
               }
               // Trough between Bits
               else
               {
                  if ((B_BitWidth < P_Packet->B_TroughMin) || (B_BitWidth > P_Packet->B_TroughMax))
                     P_PacketState->B_State = IR_STATE_START0;
               }
            }  

            // Payload Complete. RECS80 - Termination
            if ((P_PacketState->B_State == IR_STATE_PAYLOAD) && (P_PacketState->B_StateCnt >= P_Packet->B_PayloadCount))
               P_PacketState->B_State = IR_STATE_TERM;
            
         }

         else //if (gmp_Ir->P_IrProtocol->B_Protocol == IR_PROTOCOL_RC5)
         {
            BYTE nbits; //could be maximum 3 bits in one interrupt
            if ((P_PacketState->B_State == IR_STATE_START0) || // Waiting to Start (Allow stretched intervals)
               (  (B_BitWidth >= P_Packet->B_Data0Min) && // Single Transitions
                  (B_BitWidth <= P_Packet->B_Data0Max)))
               nbits = 2;
            else if ((B_BitWidth >= P_Packet->B_Data1Min) // Double Transitions
               && (B_BitWidth <= P_Packet->B_Data1Max))
               nbits = 3;
            else
            {  //restart
               P_PacketState->B_State = IR_STATE_START0;             
               nbits = 2;              
            }

            while(nbits--)
            {
               //status = (*gFp_IrRC5StateMachine)(&W_Cmd, &W_Data, j);             
               
               // RC5 state machine is based purely on the polarity.
               if (P_PacketState->B_State == IR_STATE_START0) // RC5 - Preamble
               {
                  if (isPolarityHigh)
                     P_PacketState->B_State = IR_STATE_START1;
                  P_PacketState->B_SpecialKey = 0;
               }
               else if (P_PacketState->B_State == IR_STATE_START1) // RC5 - Preamble
               {
                  P_PacketState->B_State = isPolarityHigh ? IR_STATE_START0 : IR_STATE_START2;
               }
               else if (P_PacketState->B_State == IR_STATE_START2) // RC5 - Preamble
               {
                  P_PacketState->B_State = IR_STATE_START3;
                  if (!isPolarityHigh) 
                     P_PacketState->B_SpecialKey = 64;
               }
               else if (P_PacketState->B_State == IR_STATE_START3) // RC5 - Preamble
               {
                  P_PacketState->B_State = IR_STATE_PAYLOAD;
                  P_PacketState->B_StateCnt = 0; //init field
                  P_PacketState->D_Payload = 0; //init field
                  P_PacketState->W_Address = 0; //init field
               }
               else if (P_PacketState->B_State == IR_STATE_PAYLOAD) // RC5 - Payload
               {
                  if ((P_PacketState->B_StateCnt & 1) == 0)
                  {     
                     if (P_Packet->B_Flags & IR_FLAG_REVERSE)
                     {
                        P_PacketState->D_Payload >>= 1;
                        if (isPolarityHigh)
                           P_PacketState->D_Payload |= 0x80000000UL;                            
                     }
                     else 
                     {
                        P_PacketState->D_Payload <<= 1;
                        if (isPolarityHigh)
                           P_PacketState->D_Payload |= 1;   
                     }
                  }

                  // Payload Complete
                  if ((++P_PacketState->B_StateCnt >= P_Packet->B_PayloadCount))
                     P_PacketState->B_State = IR_STATE_TERM;               
               }

               if (P_PacketState->B_State == IR_STATE_TERM)
                  break;
               
               // Terminate Packets (RC5 Doesn't get final Bit)            
               if (nbits==1)
               {
                  if ((P_PacketState->B_State == IR_STATE_PAYLOAD) && 
                     (P_PacketState->B_StateCnt == P_Packet->B_PayloadCount - 1))
                     isPolarityHigh = TRUE;
                  else
                     break;
               }
               
            }           
         }

         // Term - This state is reached if the required number of Addr and Data bits are satisfied            
         if (P_PacketState->B_State == IR_STATE_TERM)
         {
            // Decode
            if (P_PacketState->W_Address == 0)
               P_PacketState->W_Address = (WORD)((P_PacketState->D_Payload & P_Packet->D_AddrMask) >> P_Packet->B_AddrShift);
            
            if ((P_PacketState->W_Address == P_Packet->W_CmdID) || 
               (P_Packet->B_PayloadCount == 0) || 
               (P_Packet->W_CmdID == 0xffff))
            {
               gmp_Ir->valid = OK;           
               gmp_Ir->cmd = P_PacketState->W_Address;
               gmp_Ir->key = (WORD)((P_PacketState->D_Payload & P_Packet->D_DataMask) >> P_Packet->B_DataShift) + 
                  P_PacketState->B_SpecialKey;
               gmp_Ir->index = j; 
               gmp_Ir->expTime = 0;
            }
               
            // Key Identified - Reset all Packets
            for (j = gmp_Ir->P_IrProtocol->B_NumPackets; j;)
               gmp_Ir->packetStates[--j].B_State = IR_STATE_START0;

            return;     
         }        
      }
   }
}

//******************************************************************************
//
// FUNCTION:	WORD far Lpm_IrInit(gmt_IrProtocol const far *pSt_Protocol, void far *p_Ir)
//
// USAGE: 		Initializes the IR driver.  Defines Protocol data structure, programs timing
//				registers, sets ISR vector, and enables the IR interrupt (IRQ1).
//
// INPUT: 		P_Protocol - Pointer to Protocol Definition structure provided by Application.
//				P_Ir - buffer for internal global variables
//
// OUTPUT: 		return size of RAM buffer required to support defined protocol
//
// GLOBALS:		None
//
// USED_REGS:	IRQ1_IR_DECORDER			(WO)
//              IR_CONFIG					(WO)
//              IR_TIMEBASE_MEASURESTEP		(WO)
//              IR_START_PULSE_CMD_END		(WO)
//
// PRE_CONDITION  : 
//
// POST_CONDITION : 
//
//******************************************************************************
void Lpm_IrInit(void *p_Ir)//derived from gm_IrInit()
{
   #define LPM_CUSTOM_CONFIG_ABSOLUTE_ADDR 0xFFE00UL
      gmt_IrProtocolExt far *OCM_Assigned;
      OCM_Assigned = (gmt_IrProtocolExt far *)LPM_LINEAR_TO_FPTR(LPM_CUSTOM_CONFIG_ABSOLUTE_ADDR);
          
      Lpm_InterruptDisable(gmd_IR);
      
      gm_WriteRegWord(IR_CONFIG, OCM_Assigned->W_Config);
      gm_WriteRegWord(IR_TIMEBASE_MEASURESTEP, OCM_Assigned->W_Timing);
      gm_WriteRegWord(IR_START_PULSE_CMD_END, 0x00ff);
      
      memset(p_Ir, 0, sizeof(gmt_Ir));
      gmp_Ir = (gmt_Ir far *)p_Ir;
      gmp_Ir->P_IrProtocol = OCM_Assigned;
      gmp_Ir->valid = ERR_NO_DATA;
         
      
      Lpm_InterruptEnable(gmd_IR);
      gm_SetRegBitsWord(IR_CONFIG, DATA_IRQ_EN);
}
#endif
