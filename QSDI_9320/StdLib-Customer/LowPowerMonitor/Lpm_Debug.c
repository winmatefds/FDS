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
// MODULE:      Lpm_Debug.c
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
#include <mem.h>
#include "embedded.h"


//#define SHOW_APPSTEST_LIST

//******************************************************************************
//  G L O B A L    V A R I A B L E S
//******************************************************************************
// precomputed definitions of checksum start values used to speed up the computation 
#define INITRxCK   (DDC2B_SRC_ADDRESS ^ gB_DDC2BiDevId)
#define INITTxCK   ((DDC2B_SRC_ADDRESS-1) ^ gB_DDC2BiDevId)

#define MaxDebugParams    4
long DebugParam[MaxDebugParams]={0};

const BYTE AckPkt[] =
{
    3,   CMD_ACK
};
const BYTE NakPkt[] =
{
    3,   CMD_NAK
};

debugCmd_t *gDebugCmdPtr = NULL_PTR;
BYTE gB_GprobeCmdSet   = 0xC2;
WORD gW_ManufacturerID   = 0;

BYTE gB_CheckSRAMLast16KB = 0;
BYTE DebugOnly = FALSE;


//****************************************************************************** 
//  E X T E R N    V A R I A B L E S 
//****************************************************************************** 
extern BYTE gB_DDC2BiDevId;
extern BYTE LpmPwrState;


//****************************************************************************** 
//  S T A T I C    V A R I A B L E S 
//****************************************************************************** 
static WORD W_BurstAddr;
static BYTE far* gBurstPtr;


//****************************************************************************** 
//  S T A T I C    F U N C T I O N    P R O T O T Y P E S 
//****************************************************************************** 
static void ReadRamRegister(debugCmd_t far* Bp_MessageBuff);
static void ReadRamRegisterEx(debugCmd_t far* Bp_MessageBuff);
static BYTE ReadRamBurstEx(HANDLER_USE_t B_whichHandler, debugCmd_t far* Bp_MessageBuff);
static void WriteRamRegister(debugCmd_t far* Bp_MessageBuff);
static void WriteRamRegisterEx(debugCmd_t far* Bp_MessageBuff);
static void WriteRAMBurst(debugCmd_t far* Bp_MessageBuff);
static void WriteNextRAMBurst(debugCmd_t far* Bp_MessageBuff);
static void WriteRamBurstEx(debugCmd_t far* Bp_MessageBuff);
#if VAR_REGISTER_RW_SUPPORT
static BYTE ReadVarSizeRegisters(debugCmd_t far* Bp_MessageBuff);
static BYTE WriteVarSizeRegisters(debugCmd_t far* Bp_MessageBuff);
#endif

//******************************************************************************
//
//  C O D E
//
//******************************************************************************
//***************************************************************************** 
// FUNCTION     : PARSER_STATUS_t Lpm_SystemDebug(void) 
// USAGE        : Gprobe client, Receive commands from windows based debugger 
//                such as read/write gm registers, dump memory, 
//                and dump data structures. To keep  the code size 
//                down, assume that windows debugger does all user 
//                display formatting. SystemDebug just sends or 
//                receives raw data. 
// INPUT        : 
// OUTPUT       : NO_MSG_PARSED   = No command message received 
//                :   MSG_PARSED      = Command has been handled by IROM 
//                UNKNOWN_MSG_CMD = Unknown to IROM, let XROM to handle 
// GLOBALS      : 
// USED_REGS    : 
//***************************************************************************** 
void Lpm_SystemDebug(void)
{
#ifdef UART_DEBUG_SUPPORT
    PARSER_STATUS_t B_Status;
    if (IsSerialMsgCompleted() == TRUE)                      // Check whether packet OK 
    {
        //gm_Print("parsing command",0); 
        Lpm_StopRcvMsg();                               // Stop receive anymore data

        B_Status = Lpm_GprobeCmdParser(SERIAL_USE, gDebugCmdPtr);  // Process command 

        if (B_Status != UNKNOWN_MSG_CMD)
            Lpm_StartRcvMsg();                           // Re-start receive 
        return; // B_Status;
    }
//    return NO_MSG_PARSED;
#endif
}

//***************************************************************************** 
// FUNCTION     : PARSER_STATUS_t Lpm_GprobeCmdParser(HANDLER_USE_t B_whichHandler, debugCmd_t* Bp_MessageBuff) 
// USAGE        : Gprobe command parser, function decodes the received command 
//                from Gprobe and execute the command accordingly. 
//                The Serial and DDC2Bi comm. handlers are using this function 
// INPUT        : B_whichHandler - which channel to be used
//                Bp_MessageBuff - pointer to the buffer of input message
// OUTPUT       : MSG_PARSED      = Command has been handled by IROM 
//                UNKNOWN_MSG_CMD = Unknown to IROM, let XROM to handle 
// GLOBALS      : gDebugCmdPtr 
// USED_REGS    : 
//***************************************************************************** 
PARSER_STATUS_t Lpm_GprobeCmdParser(HANDLER_USE_t B_whichHandler, debugCmd_t far* Bp_MessageBuff)
{
    PARSER_STATUS_t PS_Status = MSG_PARSED;
    BYTE   B_SendAckNackData = SND_ACK;

    // Adjust gDebugCmdPtr for corresponding channel. For UART it is already adjusted correctely. 
    if(B_whichHandler != SERIAL_USE)
    {
        Bp_MessageBuff += Start_of_GprobeCmd_in_DDC2BiMsg;
    }

    switch(Bp_MessageBuff[1])
    {
        case CMD_gmRead:
         ReadRamRegister(Bp_MessageBuff);    // read Ram & Registers
            B_SendAckNackData = SND_DATA;
            break;
      case CMD_gmRead2: //---0x50 read  host reg extended (DWORD address)-----
         ReadRamRegisterEx(Bp_MessageBuff);   // read Ram & Registers with DWORD addr               
         B_SendAckNackData = SND_DATA;
         break;    
      case CMD_RAMRead2: //---0x52, burst read RAM buffer (DWORD address)----------
         B_SendAckNackData = ReadRamBurstEx(B_whichHandler, Bp_MessageBuff);
         break; 
        case CMD_gmWrite:
         WriteRamRegister(Bp_MessageBuff);   // write Ram & Register 
         //B_SendAckNackData = SND_ACK;        // Send response 
         break;
      case CMD_gmWrite2: //---0x51-write host reg extended (DWORD address)--------------                  
         WriteRamRegisterEx(Bp_MessageBuff);   // write Ram & Register             
            //B_SendAckNackData = SND_ACK;                          // Send response 
            break;
        case CMD_BurstRAMWr:
         WriteRAMBurst(Bp_MessageBuff);                           // write RAM buffer 
            //B_SendAckNackData = SND_ACK;                          // Send response 
            break;
        case CMD_BurstNextRAMWr:
         WriteNextRAMBurst(Bp_MessageBuff);                       // write burst RAM buffer 
            //B_SendAckNackData = SND_ACK;                          // Send response 
            break;
      case CMD_RAMWrite2: //---0x53, burst write RAM buffer (DWORD address)----------  
         WriteRamBurstEx(Bp_MessageBuff);
         break;
			
      case CMD_TurnOffSwitcher: //---Debug mode on--------------------------------------------
         DebugOnly = TRUE;                // Enter debug mode
         break;
      case CMD_TurnOnSwitcher: //---Debug mode off--------------------------------------------
         DebugOnly = FALSE;                //Exit debug mode
         break;			

      case CMD_WrNvramBlock:            // write Nvram from RAM buffer
      {
         WORD W_Address = ((WORD)Bp_MessageBuff[4] << 8) | (WORD)Bp_MessageBuff[5];
         BYTE far* Bp_Data = (BYTE far*)((((BYTE far*)Bp_MessageBuff)[2] << 8) | ((BYTE far*)Bp_MessageBuff)[3]);
         WORD W_SizeInBytes = ((WORD)Bp_MessageBuff[6] << 8) | (WORD)Bp_MessageBuff[7];
         
         if(Lpm_NvramWriteBuffer(W_Address, Bp_Data, W_SizeInBytes) != OK)
         {
            //gm_Print("--Write Lpm Nvram Failed!!",0);
            B_SendAckNackData = SND_NAK;
         }
         else
         {
            //gm_Print("--Write Lpm Nvram OK!",0);
            B_SendAckNackData = SND_ACK;
         }
         break;
      }
      case CMD_RdNvram:                        // Read data packet from nvram
      {
         WORD W_Address = ((WORD)Bp_MessageBuff[2] << 8) | (WORD)Bp_MessageBuff[3];
         BYTE far* Bp_Data = (BYTE far*)Bp_MessageBuff+2;         
         WORD W_SizeInBytes = (WORD)Bp_MessageBuff[4];

         // Read NVRAM data into buffer
         if(Lpm_NvramReadBuffer(W_Address, Bp_Data, W_SizeInBytes) != OK)
         {
            //gm_Print("Read Lpm Nvram Failed!!",0);
            B_SendAckNackData = SND_NAK;
         }
         else
         {
            //gm_Print("Read Lpm Nvram OK!",0);
            Bp_MessageBuff[0] = W_SizeInBytes + 3;
            B_SendAckNackData = SND_DATA;
         }
         break;
      }
		#if VAR_REGISTER_RW_SUPPORT
        case CMD_VarLenRegRead:
            B_SendAckNackData = ReadVarSizeRegisters(Bp_MessageBuff);
            break;
        case CMD_VarLenRegWrite:
            B_SendAckNackData = WriteVarSizeRegisters(Bp_MessageBuff);
            break;
		#endif

      case CMD_SetParam:
          if( (Bp_MessageBuff[2]-1) < MaxDebugParams)
          {
              BYTE *sNum, *dNum;
              dNum = (BYTE *)(&DebugParam[Bp_MessageBuff[2]-1]);
              sNum = (BYTE *)(&Bp_MessageBuff[3]);
              dNum[0] = sNum[3];
              dNum[1] = sNum[2];
              dNum[2] = sNum[1];
              dNum[3] = sNum[0];
          }
          break;
        case CMD_AppsTest:
            Lpm_SendAckOrNack(SND_ACK, B_whichHandler, Bp_MessageBuff);
            Lpm_AppstestCmdParser(B_whichHandler,Bp_MessageBuff);
            return PS_Status;
		
        default:
            PS_Status = UNKNOWN_MSG_CMD;
			break;
    }

    if(PS_Status == UNKNOWN_MSG_CMD)
    {
        return PS_Status;
    }

    switch(B_SendAckNackData)                             // Send response 
    {
        case SND_ACK:
            Lpm_SendAckOrNack(SND_ACK, B_whichHandler, Bp_MessageBuff);
            break;
        case SND_DATA:
            Lpm_WriteCmdPkt(Bp_MessageBuff, B_whichHandler);
            break;
        case SND_NOTHING:
            break;
        case SND_NAK:
        default:
            Lpm_SendAckOrNack(SND_NAK, B_whichHandler, Bp_MessageBuff);
            break;
    };

    return PS_Status;	
}

//***************************************************************************** 
// FUNCTION     : BYTE Lpm_AppstestCmdParser(HANDLER_USE_t B_whichHandler, debugCmd_t* Bp_MessageBuff)
// USAGE        : Appstest command parser, function decodes the received appstest case 
//                from Gprobe and execute the command accordingly. 
// INPUT        : B_whichHandler - which channel to be used
//                Bp_MessageBuff - pointer to the buffer of input message
// OUTPUT       : B_SendAckNackData : SND_ACK/SND_DATA/SND_NAK
// GLOBALS      : 
// USED_REGS    : 
//***************************************************************************** 
BYTE Lpm_AppstestCmdParser(HANDLER_USE_t B_whichHandler, debugCmd_t far* Bp_MessageBuff)
{
    BYTE B_SendAckNackData = SND_ACK;
    UNUSED_VAR(B_whichHandler);

    switch(Bp_MessageBuff[2])
    {
        case 0:
         #ifdef SHOW_APPSTEST_LIST
         gm_Print("  0 = Supported appstest case list", 0);
         gm_Print("  1 = Lpm SRAM memories switch(First 32KB/Last 16KB)", 0);
         gm_Print("  2 = Lpm OCM clock switch", 0);
         gm_Print("      Param[0] = 0:LPM RC-OSC  1:MISSION TCLK  2:MISSION OCLK", 0);         
         gm_Print("  8 = Select CSM Mux routing", 0);  
         gm_Print("      Param[0] = Mux Select", 0);          
         gm_Print("  9 = Set LPM_I2C_M_SEL to route CSM I2C master path", 0);  
         gm_Print("      Param[0] = 0:Set / 1:Clear LPM_I2C_M_SEL", 0);  
         gm_Print(" 45 = Check Cable Status", 0);
         gm_Print(" 53 = Read DPCD memory", 0);  
         gm_Print("      Param[0] = AUX channel", 0);
         gm_Print("      Param[1] = DPCD Address", 0);
         gm_Print("      Param[2] = Read Length", 0);
         gm_Print(" 54 = Write DPCD memory (Byte)", 0);  
         gm_Print("      Param[0] = AUX channel", 0);
         gm_Print("      Param[1] = DPCD Address", 0);
         gm_Print("      Param[2] = Write Data", 0);
         gm_Print(" 55 = DPCD RW through OCM2AUX_BUS registers", 0);  
         gm_Print("      Param[0] = AUX channel", 0);
         gm_Print("      Param[1] = Is Write?", 0);
         gm_Print("      Param[2] = DPCD Address", 0);
         gm_Print(" 60 = Check LPM Nvram (ACK) through I2C Master", 0);
         gm_Print(" 81 = Set Mission Power Up/Down without Warm Reset(Mission ON)", 0);  
         gm_Print("      Param[0] = LPM Power State", 0);
         gm_Print(" 82 = Set Mission Power Up/Down with Warm Reset", 0);  
         gm_Print("      Param[0] = LPM Power State", 0);
         gm_Print(" 83 = Set Power Event to Lpm_PowerModeHandler()", 0);  
         gm_Print("      Param[0] = Power Event", 0);
         gm_Print("100 = Used to validate LPM Tclk Timer's accurancy", 0); 
         gm_Print("101 = Check LPM stack usage status", 0); 
         gm_Print("245 = Set PBus Reqest Lock", 0);  
         gm_Print("      Param[0] = PBusReqLock by LPM", 0);
         gm_Print("246 = Set PBus Ownership", 0);  
         gm_Print("      Param[0] = PBusOwner (0:Mission  1:LPM)", 0);
         #endif
                break;
      
      case 1:  //Lpm SRAM memories switcher (First 32KB/ Last 16KB)
         if(DebugParam[0])
         {
            gB_CheckSRAMLast16KB = 1;
            gm_Print("--Last 16KB LPM SRAM checking mode!", 0);
            gm_Print("--Please DO NOT RW LPM registers in this mode!!", 0);         
         }
         else
         {
            gB_CheckSRAMLast16KB = 0;
            gm_Print("--Normal LPM SRAM/Register access mode!", 0);
            gm_Print("--For LPM SRAM, only first 32KB can be RW!!", 0);
         }
         break;

      case 2:  //Lpm OCM clock switch
         if(DebugParam[0]==0) //from LPM RC-OSC 
         {
            DWORD RcOscFreq = SYSTEM_TCLK_FREQ;
            //BYTE ClkDiv = (BYTE)DebugParam[2];  //Clock Divider = 1, 2, 4, ...128           
            RcOscTrim = (BYTE)DebugParam[1]; // 3bits, value from 0 - 7
            //if(ClkDiv < 1) ClkDiv = 1;
            //RcOscFreq = LpmRcOscClk/ClkDiv;
            gm_WriteRegWord(LPM_RCOSC,(RcOscTrim<<LPM_RC_OSC_TRIM_SHIFT)|LPM_RC_OSC_EN);
            gm_WriteRegWord(LPM_CLOCK_CTRL, 2<<LPM_OCMCLK_SEL_SHIFT);//RC-OSC drives LPM_OCMCLK
            #ifdef UART_DEBUG_SUPPORT
            Lpm_UartInit(RcOscFreq, UART_BAUD_RATE, Ba_CommBufUart0, sizeof(Ba_CommBufUart0));
            #endif
            Lpm_TimerInit(RcOscFreq);

            //gm_Print("RC-OSC Trim = 0x%x", RcOscTrim);
            //gm_Print("RC-OSC Clock = %d000Hz", (WORD)(LpmRcOscClk/1000));
            //gm_Print("OCM Clock = %d000Hz", (WORD)(RcOscFreq/1000));
         }
         else if(DebugParam[0]==1)  //from Mission TCLK 27MHz
         {
            gm_WriteRegWord(LPM_CLOCK_CTRL, LPM_TCLK_SEL);//0x01
            #ifdef UART_DEBUG_SUPPORT
            Lpm_UartInit(SYSTEM_TCLK_FREQ, UART_BAUD_RATE, Ba_CommBufUart0, sizeof(Ba_CommBufUart0));
            #endif
            Lpm_TimerInit(SYSTEM_TCLK_FREQ);
         }
         else if(DebugParam[0]==2)  //from Mission OCLK 200MHz
         {
            gm_WriteRegWord(LPM_CLOCK_CTRL, LPM_OCMCLK_SEL);//0x06
            #ifdef UART_DEBUG_SUPPORT
            Lpm_UartInit(MISSION_OCMCLK_FREQ, UART_BAUD_RATE, Ba_CommBufUart0, sizeof(Ba_CommBufUart0));
            #endif
            Lpm_TimerInit(MISSION_OCMCLK_FREQ);
         }
         break;

      case 8:  //Select CSM Mux routing
      {
         I2cMuxSelect MuxSel = (I2cMuxSelect)DebugParam[0];
         Lpm_CsmI2CMuxSelect(MuxSel);
         break;
      }
      case 9:  //Set LPM_I2C_M_SEL value for routing I2C master path CSM 
      {
         if(DebugParam[0]==0)
            gm_ClearRegBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_I2C_M_SEL);
         else
            gm_SetRegBitsWord(LPM_CSM_I2C_MUX_CTRL, LPM_I2C_M_SEL);
            
         break;
      }

      case 45: //Check Cable Status
      {
         if(LpmPwrState == LPM_NormalPowerMode)
         {
            gm_Print("Can't get CPHY 0/2 cable status(PBus) in Normal Power Mode!", 0);
            gm_Print("Please check in Mission with appstest 45", 0);
            gm_Print("PHY1 Cable Status = %d", Lpm_IsCPhyCablePlugged(CPhyNum_1));
            gm_Print("VGA  Cable Status = %d", Lpm_IsVGACablePlugged());            
         }
         else
         {
            gm_Print("PHY0 Cable Status = %d", Lpm_IsCPhyCablePlugged(CPhyNum_0));
            gm_Print("PHY1 Cable Status = %d", Lpm_IsCPhyCablePlugged(CPhyNum_1));
            gm_Print("PHY2 Cable Status = %d", Lpm_IsCPhyCablePlugged(CPhyNum_2));
            gm_Print("VGA  Cable Status = %d", Lpm_IsVGACablePlugged());
         }
         break;
      }

      case 53:  //Read DPCD memory
      {
         BYTE B_Data, i;
         AUX_CH AuxCh = (AUX_CH)DebugParam[0];
         WORD W_DpcdAddr = (WORD)DebugParam[1];
         WORD W_Len = (WORD)DebugParam[2];
         DWORD DpcdBaseAddr; 

         if(AuxCh == AUX_CH_C2)
            DpcdBaseAddr = 0x1B000UL;
         else  //AUX_CH_C0
            DpcdBaseAddr = 0x1F000UL;

         //gm_Print("DP AUX CH = %d", AuxCh);
         //gm_Print("DPCD Addr = 0x%x", W_DpcdAddr);
         //gm_Print("DPCD Length = 0x%x", W_Len);            
         for(i=0; i<W_Len; i++)
         {
            #if 1
            DWORD DW_Addr = DpcdBaseAddr + W_DpcdAddr + i;
            B_Data = *LPM_LINEAR_TO_FPTR_B(DW_Addr);   
            #else
            B_Data = peekb(DpcdBaseAddr>>LPM_SEGBIT2SHIFT, W_DpcdAddr+i);
            #endif
            gm_Print("DPCD Data = 0x%x", B_Data);
         }
      break;
      }

      case 54:  //Write DPCD memory byte
      { 
         AUX_CH AuxCh = (AUX_CH)DebugParam[0];
         WORD W_DpcdAddr = (WORD)DebugParam[1];
         BYTE B_Data = (BYTE)DebugParam[2];
         DWORD DpcdBaseAddr, DW_Addr; 

         if(AuxCh == AUX_CH_C2)
            DpcdBaseAddr = 0x1B000UL;
         else  //AUX_CH_C0
            DpcdBaseAddr = 0x1F000UL;

         DW_Addr = DpcdBaseAddr + W_DpcdAddr;
         *LPM_LINEAR_TO_FPTR_B(DW_Addr) = B_Data;
         break;
      } 
      
      #ifdef AUX_RX_SUPPORT
      case 55:  //DPCD read/write through OCM2AUX_BUS registers
      {
         AUX_CH AuxCh = (AUX_CH)DebugParam[0];  
         BYTE IsWrite = (BYTE)DebugParam[1];
         DWORD DpcdAddr = (DWORD)DebugParam[2];

         if(IsWrite) //Write DPCD
         {
            BYTE Data = (BYTE)DebugParam[3];
            Lpm_AuxRxBusWriteByte(DpcdAddr, Data, AuxCh);
         }
         else      //Read DPCD
         {
            BYTE Data;
            Data = Lpm_AuxRxBusReadByte(DpcdAddr, AuxCh);
            gm_Print("DPCD Data = 0x%x", Data);
      }
      break;         
      }
      #endif

      #ifdef I2C_MASTER_SUPPORT
      case 60:  //Check LPM Nvram (ACK) through I2C Master
         Lpm_I2CMasterCsmSel(1);
         
         //Check EEPROM & I2C path
         if (Lpm_NvramCheckDevice(0) == ERR_BUSY)     // Wait for device available
            gm_Print("Nvram Check ERR",0);
         else
            gm_Print("Nvram Check OK",0);

         Lpm_I2CMasterCsmSel(0);
         break;
      #endif

      case 81:  //Lpm Mission PowerDown/ Power Up without Warm Reset (Mission is alive)

         Lpm_Delay1ms(2);  //delay for returning ACK
         if(DebugParam[0] == LPM_NormalPowerMode)  // 0: Enter normal power mode
         {
            LpmPwrState = LPM_NormalPowerMode;
            //gm_ClearRegBitsWord(LPM_PD_CONTROL, MAIN_PSU_SHDN);	         
            Lpm_PowerUpLpmDevices();

            //Lpm_WarmReset();  //Start Warm Reset
            Lpm_PowerModeCsmMuxSel(LPM_NormalPowerMode);
            #ifdef I2C_SLAVE_SUPPORT
            Lpm_MainHandlersTurn(LpmHdrID_I2cSlave, FALSE);
            #endif
            #ifdef DDCCI_SUPPORT
            Lpm_MainHandlersTurn(LpmHdrID_DDC, FALSE);
            #endif
            Lpm_SharedPBusSwitch(BUS_MISSION);		//Mission get back PBus Own 
            PBusReqLock = FALSE;
            Lpm_Delay1ms(1);

            #if (LPM_OCMCLK_DEFAULT != 0)
            Lpm_ClockSwitch2TCLK();  
            #else
            Lpm_ClockSwitch2RCOSC(TCLK_DIV_0);
            #endif

            gm_Print("--Enter Normal Power Mode...LPM OCM Clock = 27MHz!", 0);
         }
         else if(DebugParam[0] == LPM_LowPowerMode) // 1: Enter low power mode
         {
            gmt_LPM_OCMCLK_SEL Lpm_OcmClkSel = (gmt_LPM_OCMCLK_SEL)DebugParam[1];

            LpmPwrState = LPM_LowPowerMode;
            Lpm_ClockSwitch2RCOSC(Lpm_OcmClkSel);

            Lpm_SharedPBusSwitch(BUS_LPM);   //LPM get back PBus Own 
            PBusReqLock = TRUE;
            Lpm_ResetLpmDevices(BUS_LPM);     //Reset due to LPM clock is changed to RC-OSC.

            Lpm_PowerModeCsmMuxSel(LPM_LowPowerMode);
            #ifdef I2C_SLAVE_SUPPORT
            Lpm_MainHandlersTurn(LpmHdrID_I2cSlave, TRUE);
            #endif
            #ifdef DDCCI_SUPPORT
            Lpm_MainHandlersTurn(LpmHdrID_DDC, TRUE);
            #endif
            Lpm_PowerDownLpmDevices(LPM_LowPowerMode);           
            //gm_SetRegBitsWord(LPM_PD_CONTROL, MAIN_PSU_SHDN);   //Power down Mission
            gm_Print("--Enter Low Power Mode! (Mission is NOT powered down)", 0);      
         }
         break;

      case 82:  //Lpm Mission PowerDown/ Power Up with Warm Reset

         Lpm_Delay1ms(2);  //delay for returning ACK
         if(DebugParam[0] == LPM_NormalPowerMode)  // 0: Enter normal power mode
         {
            LpmPwrState = LPM_NormalPowerMode;
            Lpm_RedLedCtrl(0);
            //-- MISSION_MICRO_PWR_DOWN is gating both LPM_WARM_RESET and LPM_RST_OVR
            gm_ClearRegBitsWord(LPM_MICRO_CONTROL, MISSION_MICRO_PWR_DOWN); 
            Lpm_WarmReset(1);  //Start Warm Reset
            gm_ClearRegBitsWord(LPM_PD_CONTROL,MAIN_PSU_SHDN);	//Power up Mission		
            Lpm_Delay1ms(WAIT_MAIN_PWR_STABLE_DELAY);   
            
            Lpm_SharedPBusSwitch(BUS_LPM);
            Lpm_PowerUpLpmDevices();
            Lpm_SignalDetectStop(); 
            Lpm_PowerModeCsmMuxSel(LPM_NormalPowerMode);
            Lpm_NvramWriteByte(NVRAM_OFFSET_PWR_MODE, PWR_SW_ON);
            //Lpm_HpdControl(LPM_NormalPowerMode);
            Lpm_WarmReset(0);  //Clear Warm Reset                   

            Lpm_SharedPBusSwitch(BUS_MISSION);     //Mission get back PBus Own 
            PBusReqLock = FALSE;
            Lpm_Delay1ms(WAIT_TCLK_STABLE_DELAY);

            #if (LPM_OCMCLK_DEFAULT != 0)
            Lpm_ClockSwitch2TCLK();  
            #else
            Lpm_ClockSwitch2RCOSC(TCLK_DIV_0);
            #endif  
            gm_RegMsgInit();         

            //------------ Main Handlers Switching ------------
            #ifdef I2C_SLAVE_SUPPORT
            Lpm_MainHandlersTurn(LpmHdrID_I2cSlave, FALSE);
            #endif                                                   
            #ifdef DDCCI_SUPPORT
            Lpm_MainHandlersTurn(LpmHdrID_DDC, FALSE);
            #endif
            #ifdef TCLK_TIMER_SUPPORT
            Lpm_MainHandlersTurn(LpmHdrID_TclkTimer, TRUE);
            #endif
            #ifdef SIGNALING_DETECT
            Lpm_MainHandlersTurn(LpmHdrID_SgDeTect, FALSE);  
            #endif                          
            gm_Print("--Enter Normal Power Mode...LPM OCM Clock = 27MHz!", 0);
         }
         else if(DebugParam[0] == LPM_LowPowerMode) // 1: Enter low power mode
         {
            gmt_LPM_OCMCLK_SEL Lpm_OcmClkSel = (gmt_LPM_OCMCLK_SEL)DebugParam[1];

            LpmPwrState = LPM_LowPowerMode;            
            Lpm_RedLedCtrl(1);
            Lpm_ClockSwitch2RCOSC(Lpm_OcmClkSel);
            
            Lpm_SharedPBusSwitch(BUS_LPM);      //LPM get back PBus Own 
            PBusReqLock = TRUE;
            Lpm_ResetLpmDevices(BUS_LPM);     //Reset due to LPM clock is changed to RC-OSC.
            
            Lpm_HpdControl(LPM_LowPowerMode);           
            Lpm_PowerModeCsmMuxSel(LPM_LowPowerMode);       
            
            Lpm_PowerDownLpmDevices(LPM_LowPowerMode);           
            Lpm_Delay1ms(1);
            Lpm_PowerDownMission();
            Lpm_Delay1ms(1);
            
            Lpm_SignalDetectInit();         
            
            //------------ Main Handlers Switching ------------
            #ifdef I2C_SLAVE_SUPPORT
            Lpm_MainHandlersTurn(LpmHdrID_I2cSlave, TRUE);
            #endif            
            #ifdef DDCCI_SUPPORT
            Lpm_MainHandlersTurn(LpmHdrID_DDC, TRUE);
            #endif
            #ifdef TCLK_TIMER_SUPPORT
            Lpm_MainHandlersTurn(LpmHdrID_TclkTimer, TRUE);
            #endif
            #ifdef SIGNALING_DETECT
            Lpm_MainHandlersTurn(LpmHdrID_SgDeTect, TRUE);
            #endif
            gm_Print("--Enter Low Power Mode!", 0);      
         }
      break;

      case 83:  //Set Power Event, the event will be processed by Lpm_PowerModeHandler()

         if(DebugParam[0] == 0)
         {
            gm_Print("Power Event List:", 0);
            gm_Print("1: LPM_EVNT2PWR_PWRKEY", 0);
            gm_Print("2: LPM_EVNT2PWR_AUXWAKE_DETECT", 0);
            gm_Print("3: LPM_EVNT2PWR_ANASYNC_DETECT", 0);
            gm_Print("4: LPM_EVNT2PWR_HPD_DETECT", 0);
            gm_Print("5: LPM_EVNT2PWR_PD_DETECT", 0);
            gm_Print("6: LPM_EVNT2PWR_SLEEP_NOTICE", 0);
            gm_Print("7: LPM_EVNT2PWR_POWER_ON_NOTICE", 0);
         }
         else
         {
            gmt_Event2PwrHdr PwrEvent = LPM_EVNT2PWR_NONE;
            switch(DebugParam[0])
            {
               case 1:
                  PwrEvent = LPM_EVNT2PWR_PWRKEY;
                  break;
               case 2:
                  PwrEvent = LPM_EVNT2PWR_AUXWAKE_DETECT;
                  break;
               case 3:
                  PwrEvent = LPM_EVNT2PWR_TMDS_DETECT;
                  break;
               case 4:
                  PwrEvent = LPM_EVNT2PWR_HPD_DETECT;
                  break;
               case 5:
                  PwrEvent = LPM_EVNT2PWR_PD_DETECT;
                  break;
               case 6:
                  PwrEvent = LPM_EVNT2PWR_SLEEP_NOTICE;
                  break;
               case 7:
                  PwrEvent = LPM_EVNT2PWR_POWER_ON_NOTICE;
                  break;
            }
            Lpm_PowerModeSetEvent2PwrHdr(PwrEvent);
            gm_Print("Set Power Event = %d", (BYTE)DebugParam[0]);
         }
         break;

      case 100:   //Validate the accurancy of LPM Tclk Timer
      {
         #ifdef TCLK_TIMER_SUPPORT 
         BYTE B_Cnt = 0;
         DWORD DW_TimeTemp = 0;
         //gm_SetRegBitsWord(LPM_TCLK_TIMER_CTRL, TCLK_TIMER_TEST_INT); //Force TCLK Timer Interrupt
         gm_ClearRegBitsWord(LPM_GPOUTPUT_1, LPM_GPIO20_OUT);
         gm_SetRegBitsWord(LPM_GPIO_DIRCTRL2, LPM_GPIO20_IO);
         gm_SetRegBitsWord(LPM_GPIO_ENABLE2, LPM_GPIO20_EN);

         gm_Print("Validating LPM TCLK Timer...in 10sec",0);
         
         while(B_Cnt <= 10)   //Validate 10sec
         {
            DWORD DW_Now = Lpm_TclkTimerGetTime();

            if(DW_TimeTemp != DW_Now)
            {
               DW_TimeTemp = DW_Now;
               B_Cnt++;
            }
            
            if (DW_Now % 2)
               gm_SetRegBitsWord(LPM_GPOUTPUT_1, LPM_GPIO20_OUT);   //output GPIO20 to high
            else
               gm_ClearRegBitsWord(LPM_GPOUTPUT_1, LPM_GPIO20_OUT); //output GPIO20 to low

            #ifdef BROWN_OUT_SUPPORT
            Lpm_BrownOutReTrigger();
            #endif
         }  
         gm_Print("LPM TCLK Timer validation complete!",0);
         #endif
         break;
      }
      
      case 101:   //Check LPM stack usage status
      {
         WORD W_Cnt = 0;
         DWORD DW_Addr;
         
         for(DW_Addr=LPM_STACK_STOP; DW_Addr<LPM_STACK_START; DW_Addr+=2, W_Cnt+=2)
         {
            if(*LPM_LINEAR_TO_FPTR_W(DW_Addr) != LPM_STACK_MARK)
               break;               
         }

         if(W_Cnt < 100)
         {
            gm_Print("  !!!!!!!!!!!!!!!!!!!!!!!!", 0);
            gm_Print("  !  STACK in DANGEROUS  !", 0);
            gm_Print("  !!!!!!!!!!!!!!!!!!!!!!!!", 0);         
         }
         gm_Print("  ** Min LPM Stack Free Size = %d bytes", W_Cnt);         
         gm_Print("  ** Max LPM Stack Used Size = %d bytes", (WORD)LPM_STACK_SIZE-W_Cnt);
         break;
      }
      
      case 245:  //Set PBus Req Lock, PBusReqLock=1 means PBus can be controlled by LPM manually
         PBusReqLock = (BYTE)DebugParam[0];
         break;

      case 246:   //Set PBus ownership manually
      {
         gmt_BusOwn BusOwn = (gmt_BusOwn)DebugParam[0];
         Lpm_SharedPBusSwitch(BusOwn);   
         PBusReqLock = TRUE;            
         break;
      }
      
        default:
         gm_Print("??Unsupported appstest case!", 0);
                break;
    }

    return B_SendAckNackData;
}

//***************************************************************************** 
// FUNCTION     : BYTE IsSerialMsgCompleted(void) 
// USAGE        : Checks for new command in the serial buffer. 
//                The function checks for a new compelte command 
//                in the serial input buffer and if the command 
//                is incomplete checks for time out. If time out 
//                is detected it resynchronize the serial link and 
//                exits. 
// INPUT        : 
// OUTPUT       : TRUE  = if a command was found in the serial receive buffer 
//                FALSE = if no command. 
// GLOBALS      : gBa_CommBuf, gW_RcvCount, gDebugCmdPtr 
// USED_REGS    : 
//***************************************************************************** 
#ifdef UART_DEBUG_SUPPORT
BYTE IsSerialMsgCompleted(void)
{
    BYTE len, checksum;
    WORD i;

    // 
    // Check total packet length received 
    // 
    len = Ba_CommBufUart0[0];                     // packet length 
    if((uart0RcvCount > 0) && (uart0RcvCount >= len) && (len > 0) )   // Packet received ? 
    {                                    // Yes 
        // 
        //   prepare for the next receive message 
        // 
        uart0RcvCount = 0;                      // reset for next packet 
        gDebugCmdPtr = (debugCmd_t*)Ba_CommBufUart0;   // point to beginning of cmd packet 

        // 
        //   verify the message correctness 
        // 
        checksum = 0;                              // Init checksum 
        for (i=0; i<len; i++)                       // Calculate packet checksum 
        {
            checksum += Ba_CommBufUart0[i];
        }
        if (checksum == 0)                         // Checksum correct 
        {
            return TRUE;                      // Yes 
        }
        else
        {
            Lpm_WriteCmdPkt((BYTE far*)NakPkt, SERIAL_USE);// Send NAK 
            
            return FALSE;                          // No 
        }
    }
    else  // we're still waiting for packet to arrive 
    {
        // 
        // If a packet is pending but we've been waiting too long, resync 
        // 
        if((uart0RcvCount > 0) && Lpm_TimerTimeOut(UART_1ms_TMR))
        {
            uart0RcvCount = 0;                      // reset for next packet 
        }

        return FALSE;
    }
}
#endif

//***************************************************************************** 
// FUNCTION     : void ReadHostRegister(HANDLER_USE_t B_whichHandler) 
// USAGE        : Reads Host register.  This function reads the 
//                register indicated in received command packet and sends the 
//                value to GProbe. 
// INPUT        : B_whichHandler  = SERIAL_USE or DDC2BI_USE 
// OUTPUT       : 
// GLOBALS      : gDebugCmdPtr 
// USED_REGS    : any register indicated by the command. 
//***************************************************************************** 
static void ReadRamRegister(debugCmd_t far* Bp_MessageBuff)
{
    WORD W_Addr, W_Data;

    // Get host register address and read register value 
    W_Addr = ((((ReadBrCmd_t far*)Bp_MessageBuff)->addr_hi) << 8) |
    ((ReadBrCmd_t far*)Bp_MessageBuff)->addr_lo;

   if(gB_CheckSRAMLast16KB)
   {
      //LPM 48KB SRAM is actually 32KB + 16KB
      //First 32KB is F4000h - FBFFFh (or mirror mapping to 0h - 7FFFh)
      //Last 16KB is FC000h - FFFFFh (NO mirror mapping)      
      #if 1
      DWORD DW_Addr = 0xF4000UL + W_Addr;
      W_Data = *LPM_LINEAR_TO_FPTR_W(DW_Addr);
      #else
      BYTE B_Data[2];    
      B_Data[0] = peekb(0xF400, W_Addr);
      B_Data[1] = peekb(0xF400, W_Addr+1);  
      W_Data = ((WORD)B_Data[1]<<8) | B_Data[0];
      #endif      
   }
   else
   {
      W_Data = gm_ReadRegWord(W_Addr);
   }

   // Format packet length and register values 
   ((RdBrCmdPkt_t far*)Bp_MessageBuff)->length   = 7;
   ((RdBrCmdPkt_t far*)Bp_MessageBuff)->value_hi = W_Data >> 8;
   ((RdBrCmdPkt_t far*)Bp_MessageBuff)->value_lo = (BYTE)W_Data;
}

static void ReadRamRegisterEx(debugCmd_t far* Bp_MessageBuff)
{
   WORD data;
   TFarAddr addr;
   
   addr.w.offs = (((WORD)((ReadBr2Cmd_t far*)Bp_MessageBuff)->addr_1) << 8) |
     ((ReadBr2Cmd_t far*)Bp_MessageBuff)->addr_0;
   addr.w.segm = (((WORD)((ReadBr2Cmd_t far*)Bp_MessageBuff)->addr_3) << 8) |
     ((ReadBr2Cmd_t far*)Bp_MessageBuff)->addr_2;                  
   addr.w.segm <<= (16 - LPM_SEGBIT2SHIFT);  //20-bit mode
   data = (WORD)(*(volatile WORD far *)addr.pt);            
   ((RdBr2CmdPkt_t far*)Bp_MessageBuff)->length = sizeof(RdBr2CmdPkt_t) + 1;
   ((RdBr2CmdPkt_t far*)Bp_MessageBuff)->value_3 = ((RdBr2CmdPkt_t far*)Bp_MessageBuff)->value_2 = 0;
   ((RdBr2CmdPkt_t far*)Bp_MessageBuff)->value_1 = (BYTE)(data >> 8);
   ((RdBr2CmdPkt_t far*)Bp_MessageBuff)->value_0 = (BYTE)data;
}

static BYTE ReadRamBurstEx(HANDLER_USE_t B_whichHandler, debugCmd_t far* Bp_MessageBuff)
{
   TFarAddr addr;
   BYTE i; 
   WORD length;
   BYTE far* ptr;
   BYTE B_SendAckNackData;

   addr.w.offs = (((WORD)((RamReadCmd_t far*)Bp_MessageBuff)->addr_1) << 8) |
   ((RamReadCmd_t far*)Bp_MessageBuff)->addr_0;
   addr.w.segm = (((WORD)((RamReadCmd_t far*)Bp_MessageBuff)->addr_3) << 8) |
   ((RamReadCmd_t far*)Bp_MessageBuff)->addr_2;                  
   addr.w.segm <<= (16 - LPM_SEGBIT2SHIFT);  //20-bit mode

   length = ((RamReadCmd_t far*)Bp_MessageBuff)->count_0 + offs_strct(RamReadResp_t, value) + 1;
   if ( ((RamReadCmd_t far*)Bp_MessageBuff)->count_3 |
         ((RamReadCmd_t far*)Bp_MessageBuff)->count_2 |
         ((RamReadCmd_t far*)Bp_MessageBuff)->count_1 |
         (length > ((B_whichHandler == SERIAL_USE) ? SIO_RxBufLen : I2C_SLAVE_BUFFER_SIZE)))
   {
      B_SendAckNackData = SND_NAK;
   }
   else 
   {
      ((RamReadResp_t far*)Bp_MessageBuff)->length = length;

      ptr = &(((RamReadResp_t far*)Bp_MessageBuff)->value[0]);
      for(i = length - offs_strct(RamReadResp_t, value) - 1; i; i--)
         *ptr++ = *addr.pt++;                     

      B_SendAckNackData = SND_DATA;
   }
   
   return B_SendAckNackData;
}

//***************************************************************************** 
// FUNCTION     : static void WriteHostRegister(void) 
// USAGE        : Writes to host register.  This function writes the 
//                register with the value indicated in received command packet 
//                from GProbe. 
// INPUT        : 
// OUTPUT       : 
// GLOBALS      : gDebugCmdPtr 
// USED_REGS    : any register indicated by the command. 
//***************************************************************************** 
static void WriteRamRegister(debugCmd_t far* Bp_MessageBuff)
{
    WORD W_Addr, W_Data;

    // Write host register value 
    W_Addr  = (((WORD)((WriteBrCmd_t far*)Bp_MessageBuff)->addr_hi) << 8) |
    ((WriteBrCmd_t*)Bp_MessageBuff)->addr_lo;
	
    //W_Data  = (((WriteBrCmd_t*)gDebugCmdPtr)->value_hi << 8 ); //5221 using byte oriented registers for debugging 
	W_Data	= (((WORD)((WriteBrCmd_t far*)Bp_MessageBuff)->value_hi) << 8 );
	W_Data |=  ((WriteBrCmd_t far*)Bp_MessageBuff)->value_lo;
	
   if(gB_CheckSRAMLast16KB)
   {
      //LPM 48KB SRAM is actually 32KB + 16KB
      //First 32KB is F4000h - FBFFFh (or mirror mapping to 0h - 7FFFh)
      //Last 16KB is FC000h - FFFFFh (NO mirror mapping)
      #if 1
      DWORD DW_Addr = 0xF4000UL + W_Addr;
      *LPM_LINEAR_TO_FPTR_W(DW_Addr) = W_Data;
      #else
      pokeb(0xF400, W_Addr,  (BYTE)W_Data);
      pokeb(0xF400, W_Addr+1,(BYTE)(W_Data>>8));
      #endif
   }
   else
   {
      gm_WriteRegWord(W_Addr, W_Data); 
   }
}

static void WriteRamRegisterEx(debugCmd_t far* Bp_MessageBuff)
{
   WORD data;
   TFarAddr addr;
   
   addr.w.offs = (((WORD)((WriteBr2Cmd_t far*)Bp_MessageBuff)->addr_1) << 8) |
     ((WriteBr2Cmd_t far*)Bp_MessageBuff)->addr_0;
   addr.w.segm = (((WORD)((WriteBr2Cmd_t far*)Bp_MessageBuff)->addr_3) << 8) |
     ((WriteBr2Cmd_t far*)Bp_MessageBuff)->addr_2;                      
   data  = (((WORD)((WriteBr2Cmd_t far*)Bp_MessageBuff)->value_1) << 8 ) |
     ((WriteBr2Cmd_t far*)Bp_MessageBuff)->value_0;
   
   addr.w.segm <<= (16 - LPM_SEGBIT2SHIFT);  //20-bit mode
   (*(volatile WORD far *)(addr.pt)) = data; 
}

static void WriteRamBurstEx(debugCmd_t far* Bp_MessageBuff)
{
   TFarAddr addr;
   BYTE i;
   
   addr.w.offs = (((WORD)((BurstWrRAM2Cmd_t far*)Bp_MessageBuff)->addr_1) << 8) |
      ((BurstWrRAM2Cmd_t far*)Bp_MessageBuff)->addr_0;
   addr.w.segm = (((WORD)((BurstWrRAM2Cmd_t far*)Bp_MessageBuff)->addr_3) << 8) |
      ((BurstWrRAM2Cmd_t far*)Bp_MessageBuff)->addr_2;      
   addr.w.segm <<= (16 - LPM_SEGBIT2SHIFT);  //20-bit mode
   
   gBurstPtr = addr.pt;
   
   addr.pt = &(((BurstWrRAM2Cmd_t far*)Bp_MessageBuff)->value[0]);
   // 7 = length byte + command byte + crc byte + 4 addr bytes
   for(i = ((BurstWrRAM2Cmd_t far*)Bp_MessageBuff)->length - 7; i; i--)
      *gBurstPtr++ = *addr.pt++; 

}

//***************************************************************************** 
// FUNCTION     : void WriteRAMBurst(void) 
// USAGE        : This function takes the data packet received 
//                in the serial buffer and puts it in a bigger 
//                RAM buffer starting at the address indicated 
//                in the command. The address is copied into 
//                the BurstAddr global, and used also by the 
//                WrNextRAMBurst function also. 
// INPUT        : Bp_MessageBuff - buffer address 
// OUTPUT       : 
// GLOBALS      : BurstAddr, gDebugCmdPtr 
// USED_REGS    : 
//***************************************************************************** 
static void WriteRAMBurst(debugCmd_t far* Bp_MessageBuff)
{
   BYTE i;
   DWORD DW_Addr;

   W_BurstAddr = (((WORD)((BurstWrRAMCmd_t far*)Bp_MessageBuff)->addr_hi) << 8) |
   ((BurstWrRAMCmd_t far*)Bp_MessageBuff)->addr_lo;

   // 
   // 5 = length byte + command byte + crc byte + 1 addr word 
   // 
   for(i=0; i<(((BurstWrRAMCmd_t far*)Bp_MessageBuff)->length - 5); i++)
   {
      if(gB_CheckSRAMLast16KB)
      {   
         DW_Addr = 0xF4000UL + W_BurstAddr;
         *LPM_LINEAR_TO_FPTR_B(DW_Addr) = ((BurstWrRAMCmd_t far*)Bp_MessageBuff)->value[i];
         W_BurstAddr++;
      }
      else
      {
         *(BYTE *)(W_BurstAddr++) = ((BurstWrRAMCmd_t far*)Bp_MessageBuff)->value[i];
      }
   }
}

//***************************************************************************** 
// FUNCTION     : void WriteNextRAMBurst(void) 
// USAGE        : Write a internal RAM buffer 
//                This function takes the data packet received in 
//                the serial buffer and puts it in a bigger RAM 
//                buffer starting at the address indicated by 
//                the BurstAddr global. 
// INPUT        : buffer address
// OUTPUT       : 
// GLOBALS      : BurstAddr, gDebugCmdPtr 
// USED_REGS    : 
//***************************************************************************** 
static void WriteNextRAMBurst(debugCmd_t far* Bp_MessageBuff)
{
   BYTE i;
   DWORD DW_Addr;

   // 
   // 3 = length byte + command byte + crc byte 
   // 
   for(i=0; i<(((BurstWrNextRAMCmd_t far*)Bp_MessageBuff)->length - 3); i++)
   {
      if(gB_CheckSRAMLast16KB)
      {   
         DW_Addr = 0xF4000UL + W_BurstAddr;
         *LPM_LINEAR_TO_FPTR_B(DW_Addr) = ((BurstWrRAMCmd_t far*)Bp_MessageBuff)->value[i];
         W_BurstAddr++;
      }
      else
      {    
         *(BYTE *)(W_BurstAddr++) = ((BurstWrNextRAMCmd_t far*)Bp_MessageBuff)->value[i];
      }
   }
}

#if VAR_REGISTER_RW_SUPPORT
//***************************************************************************** 
// FUNCTION     : static byte ReadVarSizeRegisters(debugCmd_t* Bp_MessageBuff) 
// USAGE        : 
// INPUT        : Bp_MessageBuff 
// OUTPUT       : 
// GLOBALS      : 
// USED_REGS    : any register indicated by the command. 
//***************************************************************************** 
static BYTE ReadVarSizeRegisters(debugCmd_t far* Bp_MessageBuff)
{
    BYTE B_AddrSz, B_Len2Handle;
    BYTE far *PB_Src, far *PB_Dst;
    BYTE B_CurRegSz;

    B_AddrSz = ((TVarLenReg_Read*)Bp_MessageBuff)->AddressLength;

    if((B_AddrSz != 3) &&  (B_AddrSz != 2))
    {
        return SND_NAK;
    }

    B_Len2Handle = ((TVarLenReg_Read*)Bp_MessageBuff)->MessageLength - 4;

    PB_Src = ((TVarLenReg_Read*)Bp_MessageBuff)->Regs;
    PB_Dst = &(((TVarLenReg_Read*)Bp_MessageBuff)->AddressLength);

    ((TVarLenReg_Read*)Bp_MessageBuff)->MessageLength = 3;

    for(; B_Len2Handle >= B_AddrSz + 1; B_Len2Handle -= B_AddrSz + 1)
    {
        DWORD DW_Addr;
        BYTE far* Bp_Addr;
        BYTE B_TmpCtr;
        //Get Addr 
        if(B_AddrSz == 2)
        {
            DW_Addr = ((WORD)PB_Src[0] << 8) | PB_Src[1];
        }
        else if(B_AddrSz == 3)
        {
            DW_Addr = ((DWORD)PB_Src[0] << 16) | ((DWORD)PB_Src[1] << 8) | PB_Src[2];
        }

        B_CurRegSz = *(PB_Src + B_AddrSz);
        PB_Src++;

        //Read value 
        Bp_Addr = LPM_LINEAR_TO_FPTR_B(DW_Addr);
        for(B_TmpCtr = B_CurRegSz; B_TmpCtr; B_TmpCtr--)
        {
            *(PB_Dst) = *(Bp_Addr + B_TmpCtr-1);
            PB_Dst++;
        }
        // _fmemcpy(PB_Dst, Bp_Addr, B_CurRegSz); 

        ((TVarLenReg_Read*)gDebugCmdPtr)->MessageLength += B_CurRegSz;
    }

    return SND_DATA;
}

//***************************************************************************** 
// FUNCTION     : static byte WriteVarSizeRegisters(debugCmd_t* Bp_MessageBuff) 
// USAGE        : 
// INPUT        : Bp_MessageBuff 
// OUTPUT       : 
// GLOBALS      : 
// USED_REGS    : any register indicated by the command. 
//***************************************************************************** 
static BYTE WriteVarSizeRegisters(debugCmd_t far* Bp_MessageBuff)
{
    BYTE B_AddrSz, B_Len2Handle;
    BYTE far *PB_Src;
    BYTE B_CurRegSz;
    DWORD DW_Addr;
    BYTE far* Bp_Addr;

    B_AddrSz = ((TVarLenReg_Write*)Bp_MessageBuff)->AddressLength;
    B_Len2Handle = ((TVarLenReg_Write*)Bp_MessageBuff)->MessageLength - 4;
    PB_Src = ((TVarLenReg_Write*)Bp_MessageBuff)->Regs;
    while(B_Len2Handle)
    {
        BYTE B_TmpCtr;
        if(B_AddrSz == 2)
        {
            DW_Addr = ((WORD)PB_Src[0] << 8) | PB_Src[1];
        }
        else
        if(B_AddrSz == 3)
        {
            DW_Addr = ((DWORD)PB_Src[0] << 16) | ((DWORD)PB_Src[1] << 8) | PB_Src[2];
        }
        else
        {
            return SND_NAK;
        }
        PB_Src += B_AddrSz;
        B_CurRegSz = *(PB_Src);
        Bp_Addr = LPM_LINEAR_TO_FPTR_B(DW_Addr);
        PB_Src++;
        for(B_TmpCtr = B_CurRegSz; B_TmpCtr; B_TmpCtr--)
        {
            *(Bp_Addr + B_TmpCtr-1) = *(PB_Src);
            PB_Src++;
        }
        B_Len2Handle -= B_CurRegSz + B_AddrSz + 1;
    }
    return SND_ACK;
}
#endif

//***************************************************************************** 
// FUNCTION     : void Lpm_SendAckOrNack(BYTE B_Ack, BYTE B_whichHandler) 
// USAGE        : Send ACK or NACK message to master 
// INPUT        : B_Ack - Send ACK if set to SND_ACK ; Send NACK if set to SND_NAK
//                B_whichHandler - which channel to be used 
// OUTPUT       : 
// GLOBALS      : 
// USED_REGS    : 
//***************************************************************************** 
void Lpm_SendAckOrNack(BYTE B_Ack, BYTE B_whichHandler, debugCmd_t far* Bp_MessageBuff)
{
	
    if(B_whichHandler == SERIAL_USE)
    {
        if(B_Ack == SND_ACK)
        {
            Lpm_WriteCmdPkt((BYTE far*)AckPkt, SERIAL_USE);
        }
        else
        {
            Lpm_WriteCmdPkt((BYTE far*)NakPkt, SERIAL_USE);
        }
        return;
    }

    if(B_Ack == SND_ACK)
        _fmemcpy((BYTE far*)Bp_MessageBuff, (BYTE far*)AckPkt, 3);
    else
        _fmemcpy((BYTE far*)Bp_MessageBuff, (BYTE far*)NakPkt, 3);

    Lpm_WriteCmdPkt((BYTE far *)Bp_MessageBuff, B_whichHandler);
}

//***************************************************************************** 
// FUNCTION     : void Lpm_WriteCmdPkt(BYTE far *Bp_Buf,  HANDLER_USE_t B_whichHandler) 
// USAGE        : Send a response packet according to the Gprobe debug protocol
//                or prepare a Gprobe message 
//                wrapped in a DDC2Bi response 
// INPUT        : Bp_Buf - Buffer pointer to response message 
//                B_whichHandler - which channel to be used  
// OUTPUT       : 
// GLOBALS      : 
// USED_REGS    : 
//***************************************************************************** 
void Lpm_WriteCmdPkt(BYTE far *Bp_Buf,  HANDLER_USE_t B_whichHandler)
{
    BYTE i,B_Length,B_Checksum;
	#ifdef AUX_RX_SUPPORT
    SW_TransactionStruct_t *SW_TrnsData;
	#endif

   #ifdef UART_DEBUG_SUPPORT
    if(B_whichHandler == SERIAL_USE)
    {
        Lpm_UartWriteCmdPkt(Bp_Buf);
    }
    else
   #endif
    {
        // calculate the checksum and the length of the packet 
        switch(B_whichHandler)
        {
        	#ifdef I2C_SLAVE_SUPPORT
	        case I2C_SLAVE1_USE:
	        case I2C_SLAVE2_USE:
	        case I2C_SLAVE3_USE:
	        case I2C_SLAVE4_USE:
	        {
	           BYTE B_whichSlaveCh = B_whichHandler - I2C_SLAVE1_USE;
	           I2C_SLAVE_TYPE far* Sp_I2CData;
			   
	           Sp_I2CData = Lpm_I2CSlaveGetAddrBuffer(B_whichSlaveCh);
	           B_Checksum = ((DDC2B_SRC_ADDRESS-1) ^ (Sp_I2CData->B_DevID & I2C_ADDRESS_BITS));
	        }
	        break;
			#endif

			#ifdef AUX_RX_SUPPORT
	        case AUX_C0_USE:
	        case AUX_C2_USE:
                {
                    AUX_CH B_AuxCh = AUX_CH_C0;
					
                    if(B_whichHandler == AUX_C2_USE)
			B_AuxCh = AUX_CH_C2;

                    SW_TrnsData = Lpm_AuxRxI2CSwGetAddrModuleData(B_AuxCh);
                    B_Checksum = ((DDC2B_SRC_ADDRESS-1) ^ SW_TrnsData->B_DevID);
                }
                break;
			#endif

            default:
                B_Checksum = INITTxCK;
                break;
        }

        B_Length = (*Bp_Buf);

        Bp_Buf -= Start_of_GprobeCmd_in_DDC2BiMsg; //move back to the beginning of DDC2Bi packet 
        Bp_Buf[0] = (B_Length + 2) | DDC2Bi_CONTROL_STATUS_FLAG;
        // length for DDC2Bi msg 
        Bp_Buf[1] = gB_GprobeCmdSet;            // Specify gProbe command 
        Bp_Buf[2] = (BYTE)gW_ManufacturerID;      // manufacturer id 
        Bp_Buf[3] = (BYTE)(gW_ManufacturerID>>8); // manufacturer id 
        //Bp_Buf[4] = B_Length;                     // length for gprobe msg 

        // Copy response message. It is possibly ACK/NACK template or message that is allready in gDebugCmdPtr 
        _fmemcpy((Bp_Buf + 4), (Bp_Buf + Start_of_GprobeCmd_in_DDC2BiMsg), (B_Length - 1));
        // 
        for (i=0; i < B_Length + 3; i++)            // Process the whole packet 
        {
            B_Checksum ^= Bp_Buf[i];               // Update checksum 
            //gm_Print(" 02 BYTE 0x%X ",Bp_Buf[i]); 
        }
        // 
        // add to not be forced to discriminate the stop condition 
        // 
        Bp_Buf[B_Length + 3] = B_Checksum;
        Bp_Buf[B_Length + 4] = gB_DDC2BiDevId;

        switch(B_whichHandler)
        {
        	#ifdef I2C_SLAVE_SUPPORT
            case I2C_SLAVE1_USE:
            case I2C_SLAVE2_USE:
            case I2C_SLAVE3_USE:
            case I2C_SLAVE4_USE:
            {
                BYTE B_whichSlaveCh = B_whichHandler - I2C_SLAVE1_USE;
                Lpm_I2CSlaveSetReply((B_Length + 5), B_whichSlaveCh);//map to SLAVE_CH enum
            }
                break;
			#endif

			#ifdef AUX_RX_SUPPORT
            case AUX_C0_USE:
            case AUX_C2_USE:
            {
                SW_TrnsData->B_ReplyReady = TRUE;
            }
                break;
			#endif

            default:
                break;
        }
    }
}

#ifdef UART_DEBUG_SUPPORT
//***************************************************************************** 
// FUNCTION     : void Lpm_UartWriteCmdPkt(BYTE far *Bp_Buf) 
// USAGE        : Write packet 
// INPUT        : Bp_Buf - buffer 
// OUTPUT       : 
// GLOBALS      : 
// USED_REGS    : 
//***************************************************************************** 
void Lpm_UartWriteCmdPkt(BYTE far *Bp_Buf)
{
    BYTE i,B_Length,B_Checksum;

    B_Checksum = 0;
    B_Length = (*Bp_Buf)-1;                  // Packet length 

    for (i=0; i < B_Length; i++)            // Process the whole packet 
    {
        B_Checksum += *Bp_Buf;               // Update checksum 
        Lpm_UartTrxChar(*Bp_Buf++);            // Send data 
    }

    B_Checksum = ~B_Checksum + 1;            // Update checksum 
    Lpm_UartTrxChar(B_Checksum);            // Send checksum byte 
}	
#endif

//******************************************************************************
//
// FUNCTION:    void gm_Print (const far char *Bp_Str, WORD W_Value)
//
// USAGE:		This function is a stripped version of printf intended to be
//				used with windows based debugger. It has the following
//				l imitations: It can only send out 1 WORD argument.
// 				If you just want to send a string don't use a conversion factor
//				but still use a dummy argument for value. In order to use the
// 				quoted string method CPU must be Von Newman wired otherwise,
//				the string should be an intialized data string.
//
// 				Examples:
// 					print("The value of MyTestVar in hex is 0x%x\n", MyTestVar);
// 					print("This is just a string, the 0 is ignored.\n",0);
//
// INPUT:       Bp_Str 	- buffer pointer to descriptor
//				W_Value	- variable value
//
// OUTPUT:      None
//
// GLOBALS:		None
//
// USED_REGS:	None
//
//******************************************************************************
void gm_Print (const char far *Bp_Str, WORD W_Value)
{
#ifdef UART_DEBUG_SUPPORT
	char 	ch;
	BYTE 	len, checksum;				// len is length of elements in string + data	                            		
    {   // determine size of packet
        const char far *ps;
    	for (len = 6, ps=Bp_Str; *ps++; len++); // add null terminating byte to length  plus        
    }		// length byte itself plus checksum plus cmd plus 2 byte argument
	checksum = len;						//
	Lpm_UartTrxChar(len);         		// length of printit packet
	Lpm_UartTrxChar(CMD_PRINTIT);		// Command type
	checksum += CMD_PRINTIT;			//
	while((ch = *Bp_Str) != FALSE)				// Send the buffer
	{
		checksum += ch;							// Update checksum
		Lpm_UartTrxChar(*Bp_Str++);     		// Send one char at a time
	}
	Lpm_UartTrxChar(0);							// terminating null to string
	Lpm_UartTrxChar(ch = (W_Value & 0xff)); 	// standardize on little endian
	checksum  += ch;                            // - low byte first
	Lpm_UartTrxChar(ch = (W_Value >> 8) & 0xff);//
	checksum  += ch;                            //
	Lpm_UartTrxChar(~checksum + 1);             // Send checksum

#else
   UNUSED_VAR(Bp_Str);
   UNUSED_VAR(W_Value);
#endif
}


