/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2015 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"


/* USER CODE BEGIN Includes */
#include "typedef.h"
#include "hdmirx.h"
#include "IT626X_drv.h"
#include "IT626X_sys.h"
#include "stm32f1xx_hal_tim_ex.h"

#include <string.h>
#include <stdio.h>
//#include "IT626X_sys.h"
/* USER CODE END Includes */

#define Input_Select	161
#define QSDI_Status	240
#define Filp_Select 	260

#define Output_Select	 290
#define Configuration	291
#define Device_Detect 430
#define Device_Detect_Reply	440


#define Input_DVI	1
#define Input_VGA	2
#define Input_SDI 	3
#define Input_CVBS	4
#define Input_DP	 	5


#define Output_720P		1
#define Output_1080I		2
#define Output_1080P	 	3









#define CAN_ 1
/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan;

I2C_HandleTypeDef hi2c2;


extern _XDATA HDMI_VTiming s_CurrentVM ;

//SPI_HandleTypeDef hspi1;
TIM_HandleTypeDef htim2;
UART_HandleTypeDef huart4;
UART_HandleTypeDef huart1;
/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
int xCntSum, xCntCnt ;

static unsigned int UserPrefCurrentInput;
static unsigned int UserPrefCurrentInput_GPIO;
static unsigned int UserOutput_MODE;
static   unsigned int  Qsdi_time; 
static unsigned int UserPrefCurrentFilp;
//static unsigned int UserPrefScaling;
//static unsigned int UserPrefPattern;


static unsigned int UserLED_Setting;
static unsigned int HDMI_VState;
#define DUMP_TIME 20

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN_Init(void);
static void MX_I2C2_Init(void);
//static void MX_SPI1_Init(void);
static void MX_TIM2_Init(void);
static void MX_UART4_Init(void);
static void MX_USART1_UART_Init(void);
int dump_vidmode(void);
/*
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)

PUTCHAR_PROTOTYPE
{
HAL_UART_Transmit(&huart1 , (uint8_t *)&ch, 1, 0xFFFF);
return ch;
}
*/
/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */


void Input_Selset_GPIO(void)
{

#if 0
 	if((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9)==0)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2)==0) &&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1)==0)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)==1)&&UserPrefCurrentInput_GPIO !=Input_VGA )//VGA
	{
		delay1ms(50);
		if((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9)==0)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2)==0) &&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1)==0)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)==1)&&UserPrefCurrentInput_GPIO !=Input_VGA )//VGA
			{
				UserPrefCurrentInput=Input_VGA;
				UserPrefCurrentInput_GPIO=Input_VGA;
				USART1_printf( USART1," UserPrefCurrentInput %x   \r\n",UserPrefCurrentInput); 
			}
		

	}
	else if((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9)==0)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2)==0) &&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1)==1)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)==0)&&UserPrefCurrentInput_GPIO !=Input_DVI )//HDMI
	{
		delay1ms(50);
		if((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9)==0)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2)==0) &&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1)==1)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)==0)&&UserPrefCurrentInput_GPIO !=Input_DVI )//HDMI
			{	
				UserPrefCurrentInput=Input_DVI;
				UserPrefCurrentInput_GPIO=Input_DVI;
				USART1_printf( USART1," UserPrefCurrentInput %x   \r\n",UserPrefCurrentInput); 
			}

	}
	 else if((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9)==1)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2)==0) &&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1)==0)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)==0)&&UserPrefCurrentInput_GPIO !=Input_DP )//DP
	{
		delay1ms(50);
		if((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9)==1)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2)==0) &&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1)==0)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)==0)&&UserPrefCurrentInput_GPIO !=Input_DP )//DP
			{
				UserPrefCurrentInput=Input_DP;
				UserPrefCurrentInput_GPIO=Input_DP;
				USART1_printf( USART1," UserPrefCurrentInput %x   \r\n",UserPrefCurrentInput); 
			}

	}

	else if((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9)==0)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2)==1) &&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1)==0)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)==0)&&UserPrefCurrentInput_GPIO !=Input_SDI )//SDI
	{
		delay1ms(50);
		if((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9)==0)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2)==1) &&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1)==0)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)==0)&&UserPrefCurrentInput_GPIO !=Input_SDI )//SDI
			{	
				UserPrefCurrentInput=Input_SDI;
				UserPrefCurrentInput_GPIO=Input_SDI;
				USART1_printf( USART1," UserPrefCurrentInput %x   \r\n",UserPrefCurrentInput); 
			}

	}
	 else if((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9)==0)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2)==0) &&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1)==0)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)==0)&&UserPrefCurrentInput_GPIO !=Input_CVBS )//cvbs
	{
		delay1ms(50);
		 if((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9)==0)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2)==0) &&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1)==0)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)==0)&&UserPrefCurrentInput_GPIO !=Input_CVBS )//cvbs
			{		
				UserPrefCurrentInput=Input_CVBS;
				UserPrefCurrentInput_GPIO=Input_CVBS;
				USART1_printf( USART1," UserPrefCurrentInput %x   \r\n",UserPrefCurrentInput); 
			}
	}
#else


 	if((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9)==1)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2)==1) &&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1)==1)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)==0)&&(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_15)==1) &&UserPrefCurrentInput_GPIO !=Input_VGA )//VGA
	{
		delay1ms(50);
		if((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9)==1)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2)==1) &&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1)==1)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)==0)&&(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_15)==1) &&UserPrefCurrentInput_GPIO !=Input_VGA )//VGA
			{
				UserPrefCurrentInput=Input_VGA;
				UserPrefCurrentInput_GPIO=Input_VGA;
				USART1_printf( USART1," UserPrefCurrentInput %x   \r\n",UserPrefCurrentInput); 
			}
		

	}
	else if((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9)==1)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2)==1) &&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1)==0)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)==1)&&(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_15)==1) &&UserPrefCurrentInput_GPIO !=Input_DVI )//HDMI
	{
		delay1ms(50);
		if((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9)==1)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2)==1) &&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1)==0)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)==1)&&(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_15)==1) &&UserPrefCurrentInput_GPIO !=Input_DVI )//HDMI
			{	
				UserPrefCurrentInput=Input_DVI;
				UserPrefCurrentInput_GPIO=Input_DVI;
				USART1_printf( USART1," UserPrefCurrentInput %x   \r\n",UserPrefCurrentInput); 
			}

	}
	 else if((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9)==0)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2)==1) &&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1)==1)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)==1)&&(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_15)==1) &&UserPrefCurrentInput_GPIO !=Input_DP )//DP
	{
		delay1ms(50);
		if((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9)==0)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2)==1) &&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1)==1)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)==1)&&(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_15)==1) &&UserPrefCurrentInput_GPIO !=Input_DP )//DP
			{
				UserPrefCurrentInput=Input_DP;
				UserPrefCurrentInput_GPIO=Input_DP;
				USART1_printf( USART1," UserPrefCurrentInput %x   \r\n",UserPrefCurrentInput); 
			}

	}

	else if((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9)==1)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2)==0) &&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1)==1)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)==1)&&(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_15)==1) &&UserPrefCurrentInput_GPIO !=Input_SDI )//SDI
	{
		delay1ms(50);
		if((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9)==1)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2)==0) &&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1)==1)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)==1)&&(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_15)==1) &&UserPrefCurrentInput_GPIO !=Input_SDI )//SDI
			{	
				UserPrefCurrentInput=Input_SDI;
				UserPrefCurrentInput_GPIO=Input_SDI;
				USART1_printf( USART1," UserPrefCurrentInput %x   \r\n",UserPrefCurrentInput); 
			}

	}
	 else if((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9)==1)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2)==1) &&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1)==1)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)==1)&&(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_15)==0) &&UserPrefCurrentInput_GPIO !=Input_CVBS )//cvbs
	{
		delay1ms(50);
		 if((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9)==1)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2)==1) &&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1)==1)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)==1)&&(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_15)==0) &&UserPrefCurrentInput_GPIO !=Input_CVBS )//cvbs
			{		
				UserPrefCurrentInput=Input_CVBS;
				UserPrefCurrentInput_GPIO=Input_CVBS;
				USART1_printf( USART1," UserPrefCurrentInput %x   \r\n",UserPrefCurrentInput); 
			}
	}
#endif
}


void Input_Selset_Rs232(void)
{


BYTE CheckSum=0;
BYTE Input_temp=0;
BYTE Filp_temp=0;
BYTE LED_Setting_temp=0;
BYTE Scaling_temp=0;
BYTE Pattern_temp=0;





    switch(UserPrefCurrentInput)
    {
        case Input_VGA:	
		Input_temp=0;
		break;

        case Input_DVI:
		Input_temp=7;
            break;

        case Input_DP:
		Input_temp=8;	
            break;

        case Input_CVBS:
		Input_temp=2;	
            break;

        case Input_SDI:
		Input_temp=3;	
            break;
    }



    switch(UserPrefCurrentFilp)
    {
        case 0:
		Filp_temp=0;	
		break;

        case 1:
		Filp_temp=3;
            break;
			
    }

		if(!UserLED_Setting)
		{
				LED_Setting_temp=0;
			}
		else
			{
				LED_Setting_temp=1;
		}




if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_4)) //SWITCH_#1 Stretch to fit / No Stretch
{
Scaling_temp=1;
}
else
{
Scaling_temp=0;
}


// if(VState ==9 ||VState ==8)	
if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_7)) //
{
Pattern_temp=0;
}
else
{

if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_8)) 
{
Pattern_temp=1;

}
else
{
Pattern_temp=2;
}

}




CheckSum=0x00-Input_temp-Filp_temp-LED_Setting_temp-Scaling_temp-Pattern_temp-9-0x33;

delay1ms(100);

            	USART_SendData(UART4, 0x09);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x33);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x00);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, Input_temp);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, Filp_temp);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, LED_Setting_temp);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, Scaling_temp);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, Pattern_temp);
		while((UART4->SR&0X40)==0);	
		USART_SendData(UART4, CheckSum);
		while((UART4->SR&0X40)==0);



delay1ms(200);
//delay1ms(200);



}

/*
void UserOutput_GPIO(void)
{


	 if((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_5))&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_6)))
	 	{

		//	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8, GPIO_PIN_RESET);//
		//	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_7, GPIO_PIN_SET);//
			UserOutput_MODE=Output_1080P;

	 }
	 else if((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_5))||(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_6)))
	 	{
		//	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8, GPIO_PIN_RESET);//
		//	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_7, GPIO_PIN_RESET);//
			UserOutput_MODE=Output_1080I;

	 	}
	else
		{

		//	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8, GPIO_PIN_RESET);//
		//	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_7, GPIO_PIN_RESET);//
			UserOutput_MODE=Output_720P;
		}




}
*/


#if CAN_
static CanTxMsgTypeDef        TxMessage;
static CanRxMsgTypeDef        RxMessage;
#endif
/* USER CODE END 0 */
#if CAN_
HAL_StatusTypeDef CAN_Polling(void)
{
 CAN_FilterConfTypeDef  sFilterConfig;

  uint32_t Qsdi_id; 

  hcan.pTxMsg = &TxMessage;
  hcan.pRxMsg = &RxMessage;


//Qsdi_id=~(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0)|HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_3)<<1|HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_2)<<2|HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_1)<<3|HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_0)<<4
//		|HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_15)<<5|HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_14)<<6|HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_13)<<7);

//Qsdi_id=(~ HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_1)| ~ HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_2)<<1| ~ HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_3)<<2| ~ HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0)<<3
//		|~ HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_13)<<4|~ HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_14)<<5|~ HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_15)<<6|~ HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_0)<<7);

Qsdi_id=(~(HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_1)|HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_2)<<1|HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_3)<<2|HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0)<<3
		|HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_13)<<4|HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_14)<<5|HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_15)<<6|HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_0)<<7));


  #if 1
  //##-2- Configure the CAN Filter ###########################################
  sFilterConfig.FilterNumber = 0;
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
//  sFilterConfig.FilterIdHigh = (0xf0<<3)>>16;
//  sFilterConfig.FilterIdLow =(0xf0<<3)|CAN_ID_EXT|CAN_RTR_DATA | Qsdi_id;
//  sFilterConfig.FilterMaskIdHigh = 0xFFFF;
//  sFilterConfig.FilterMaskIdLow = 0xFFFF;
  //sFilterConfig.FilterMaskIdLow = 0x00ff;
 // sFilterConfig.FilterMaskIdLow = Qsdi_id & 0x00ff;
  
//    sFilterConfig.FilterIdHigh = 0x0000;
//  sFilterConfig.FilterIdLow =0x0000;
//  sFilterConfig.FilterMaskIdHigh = 0x0000;
//sFilterConfig.FilterMaskIdLow = 0x0000;
/*
  sFilterConfig.FilterIdHigh = (Qsdi_id<<3)>>16;;
  sFilterConfig.FilterIdLow =(Qsdi_id<<3)|CAN_ID_EXT|CAN_RTR_DATA ;;
  sFilterConfig.FilterMaskIdHigh = 0x0000;
  sFilterConfig.FilterMaskIdLow = (0xff<<3);
*/
sFilterConfig.FilterIdHigh = (Qsdi_id<<23)>>16;
  sFilterConfig.FilterIdLow =(Qsdi_id<<23)|CAN_ID_EXT|CAN_RTR_DATA ;;
  sFilterConfig.FilterMaskIdHigh = (0xff<<4);
  sFilterConfig.FilterMaskIdLow = 0x0000;




  sFilterConfig.FilterFIFOAssignment = 0;
  sFilterConfig.FilterActivation = ENABLE;
  sFilterConfig.BankNumber = 14;

  if (HAL_CAN_ConfigFilter(&hcan, &sFilterConfig) != HAL_OK)
  {
    //Filter configuration Error
    //Error_Handler();
  }



  sFilterConfig.FilterNumber = 1;
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;

sFilterConfig.FilterIdHigh =0x0000;
  sFilterConfig.FilterIdLow =CAN_ID_EXT|CAN_RTR_DATA ;;
  sFilterConfig.FilterMaskIdHigh = (0xff<<4);
  sFilterConfig.FilterMaskIdLow = 0x0000;




  sFilterConfig.FilterFIFOAssignment = 0;
  sFilterConfig.FilterActivation = ENABLE;
  sFilterConfig.BankNumber = 14;

  if (HAL_CAN_ConfigFilter(&hcan, &sFilterConfig) != HAL_OK)
  {
    //Filter configuration Error
    //Error_Handler();
  }






  
#endif
  /*##-3- Configure Transmission process #####################################*/
 #if 0
  hcan.pTxMsg->StdId = 0x321;
  hcan.pTxMsg->ExtId = 0x00;
  hcan.pTxMsg->RTR = CAN_RTR_DATA;
  hcan.pTxMsg->IDE = CAN_ID_EXT;
  hcan.pTxMsg->DLC = 8;
#endif
/*
  if(HAL_CAN_Transmit(&hcan, 100) != HAL_OK)//那?10是超????位ms
  {
    //[i] Transmition Error [/i]/
		//HAL_GPIO_WritePin (GPIOF,GPIO_PIN_7,GPIO_PIN_RESET );
		//while(1);
		USART1_printf( USART1," HAL_CAN_Transmit   HAL_ERROR \r\n",0);

		return HAL_ERROR;
  }
  
  if(HAL_CAN_GetState(&hcan) != HAL_CAN_STATE_READY)
  {
  USART1_printf( USART1," HAL_CAN_Transmit   HAL_ERROR \r\n",0);
    return HAL_ERROR;
  }

*/

  //[i][size=16]-4- Start the Reception process [/size]################################[size=16]#[size=16]#[/i]/[/size][/size]
  if(HAL_CAN_Receive(&hcan, CAN_FIFO0,400) != HAL_OK)//上面定?了FIFO0
  {
    //[i] Reception Error [/i]/

		//HAL_GPIO_WritePin (GPIOF,GPIO_PIN_7,GPIO_PIN_RESET );
		//while(1);
		//USART1_printf( USART1," HAL_CAN_Receive   HAL_ERROR \r\n",0);
		return HAL_ERROR;
  }
  else
  	{
  if(HAL_CAN_GetState(&hcan) != HAL_CAN_STATE_READY)
  {
  		//USART1_printf( USART1,"HAL_CAN_GetState   HAL_ERROR \r\n",0);

    return HAL_ERROR;
  }
  /*
  if (hcan.pRxMsg->ExtId != 0x321)//接收到的?据?示符
  {
  USART1_printf( USART1,"StdId   HAL_ERROR \r\n",0);
    return HAL_ERROR;  
  }
*/
  if (hcan.pRxMsg->IDE != CAN_ID_EXT)//接收到的???据?型
  {
  USART1_printf( USART1,"CAN_ID_STD   HAL_ERROR \r\n",0);
    return HAL_ERROR;
  }

  if (hcan.pRxMsg->DLC != 8)//接收到的?据?度
  {
     USART1_printf( USART1,"DLC   HAL_ERROR \r\n",0);

    return HAL_ERROR;  
  }
/*
  if ((hcan.pRxMsg->Data<<8|RxMessage.Data) != 0xCAFE)//接收到的?据比?
  {
    return HAL_ERROR;
  }
  */
  USART1_printf( USART1,"hcan.pRxMsg->Data[0] %x \r\n",hcan.pRxMsg->Data[0] );
  return HAL_OK; //[i] Test Passed [/i]/
  	}
}
#endif




int main(void)
{
  /* USER CODE BEGIN 1 */
   uint32_t Qsdi_Cammand; 
   uint32_t Qsdi_id1; 

  /* USER CODE END 1 */
#if 0//CAN_
  HAL_StatusTypeDef CAN_Status;
  
#endif

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();


  /* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_CAN_Init();
	MX_I2C2_Init();

	  MX_TIM2_Init();
	//  MX_SPI1_Init();
	MX_UART4_Init();
	MX_USART1_UART_Init();
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0, GPIO_PIN_SET); // 
	delay1ms(800);

  /* USER CODE BEGIN 2 */
  #if 1
 

	InitIT626X_Instance();
	InitIT626X();

	delay1ms(800);

#if 1//6605

   	SelectHDMIPort(CAT_HDMI_PORTA);
	InitHDMIRX(TRUE);
	HDMITX_ChangeDisplayOption(HDMI_1080p60,HDMI_YUV422);
	Turn_HDMIRX(ON) ;//leo

#endif

	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_2, GPIO_PIN_RESET);
	USART_SendData(UART4, 0x05);
	while((UART4->SR&0X40)==0);
	USART_SendData(UART4, 0x40);
	while((UART4->SR&0X40)==0);
	USART_SendData(UART4, 0x04);
	while((UART4->SR&0X40)==0);
	USART_SendData(UART4, 0x06);
	while((UART4->SR&0X40)==0);
	USART_SendData(UART4, 0xb1);
	while((UART4->SR&0X40)==0);
	delay1ms(300);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_15, GPIO_PIN_SET);//audio i2s
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14, GPIO_PIN_RESET);//audio i2s
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_6, GPIO_PIN_SET);//audio i2s
	    xCntCnt = 0 ;
	    xCntSum = 0 ;	
		



#if 0
 	if((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9)==0)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2)==0) &&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1)==0)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)==1) )//VGA
	{

		UserPrefCurrentInput=Input_VGA;
		UserPrefCurrentInput_GPIO=Input_VGA;
	}
	else if((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9)==0)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2)==0) &&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1)==1)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)==0) )//HDMI
	{
		UserPrefCurrentInput=Input_DVI;
		UserPrefCurrentInput_GPIO=Input_DVI;
	}
	 else if((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9)==1)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2)==0) &&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1)==0)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)==0))//DP
	{
		UserPrefCurrentInput=Input_DP;
		UserPrefCurrentInput_GPIO=Input_DP;
	}

	else if((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9)==0)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2)==1) &&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1)==0)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)==0) )//SDI
	{
		
		UserPrefCurrentInput=Input_SDI;
		UserPrefCurrentInput_GPIO=Input_SDI;
		

	}
	 else if((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9)==0)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2)==0) &&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1)==0)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)==0) )//cvbs
	{
		UserPrefCurrentInput=Input_CVBS;
		UserPrefCurrentInput_GPIO=Input_CVBS;
	}
	
	 else
	 	{

		UserPrefCurrentInput=Input_VGA;
		UserPrefCurrentInput_GPIO=Input_VGA;
	}
#else


 	if((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9)==1)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2)==1) &&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1)==1)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)==0) &&(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_15)==1))//VGA
	{

		UserPrefCurrentInput=Input_VGA;
		UserPrefCurrentInput_GPIO=Input_VGA;
	}
	else if((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9)==1)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2)==1) &&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1)==0)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)==1) &&(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_15)==1))//HDMI
	{
		UserPrefCurrentInput=Input_DVI;
		UserPrefCurrentInput_GPIO=Input_DVI;
	}
	 else if((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9)==0)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2)==1) &&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1)==1)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)==1)&&(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_15)==1))//DP
	{
		UserPrefCurrentInput=Input_DP;
		UserPrefCurrentInput_GPIO=Input_DP;
	}

	else if((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9)==1)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2)==0) &&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1)==1)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)==1)&&(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_15)==1) )//SDI
	{
		
		UserPrefCurrentInput=Input_SDI;
		UserPrefCurrentInput_GPIO=Input_SDI;
		

	}
	 else if((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9)==1)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2)==1) &&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1)==1)&&(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)==1)&&(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_15)==0) )//cvbs
	{
		UserPrefCurrentInput=Input_CVBS;
		UserPrefCurrentInput_GPIO=Input_CVBS;
	}
	
	 else
	 	{

		UserPrefCurrentInput=Input_VGA;
		UserPrefCurrentInput_GPIO=Input_VGA;
	}
#endif


//USART1_printf( USART1," UserPrefCurrentInput %x   \r\n",UserPrefCurrentInput); 

	//UserOutput_MODE=Output_720P;
	UserLED_Setting=ENABLE;
	UserTime_Interval=0;
	UserTime_1=0;
	HDMI_VState=0;
	Qsdi_time=3;
	Input_Selset_GPIO();
	Input_Selset_Rs232();
 	//UserOutput_GPIO();



		
  /* USER CODE END 2 */
#endif
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) 
  {
#if CAN_



Qsdi_id1=(~(HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_1)|HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_2)<<1|HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_3)<<2|HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0)<<3
		|HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_13)<<4|HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_14)<<5|HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_15)<<6|HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_0)<<7));

Qsdi_id1=0x00ff & Qsdi_id1;


//USART1_printf( USART1,"Qsdi_id1= %x \r\n",Qsdi_id1 );


if(HDMI_VState)
if(HAL_OK==CAN_Polling())
{

Qsdi_Cammand=(hcan.pRxMsg->ExtId & 0x3ff00)>>8;


    switch(Qsdi_Cammand)
    {
        case Input_Select://161
			
		if(((hcan.pRxMsg->Data[1])>0)&&((hcan.pRxMsg->Data[1])<6))		
			UserPrefCurrentInput=hcan.pRxMsg->Data[1];


	break;
/*
        case Filp_Select://260
			
	//	if((hcan.pRxMsg->Data[1])<2)
			UserPrefCurrentFilp=hcan.pRxMsg->Data[1];


	break;

*/
        case Output_Select://290

	//	if(((hcan.pRxMsg->Data[0])>0)&&((hcan.pRxMsg->Data[0])<4))		
			UserOutput_MODE=hcan.pRxMsg->Data[0];


//720p = 1
//1080p30 = 2
//1080p60 = 3
	if(UserOutput_MODE==Output_720P)
		{
            	USART_SendData(UART4, 0x05);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x40);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x90);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x01);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x2a);
		while((UART4->SR&0X40)==0);
		delay1ms(50);


            	USART_SendData(UART4, 0x05);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x40);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x90);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x01);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x2a);
		while((UART4->SR&0X40)==0);
		delay1ms(50);

		}
	else if (UserOutput_MODE==Output_1080I)
		{
            	USART_SendData(UART4, 0x05);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x40);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x90);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x02);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x29);
		while((UART4->SR&0X40)==0);

		delay1ms(50);
            	USART_SendData(UART4, 0x05);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x40);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x90);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x02);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x29);
		while((UART4->SR&0X40)==0);
		delay1ms(50);
		}
		else if (UserOutput_MODE==Output_1080P)
		{
            	USART_SendData(UART4, 0x05);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x40);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x90);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x03);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x28);
		while((UART4->SR&0X40)==0);
		delay1ms(50);


            	USART_SendData(UART4, 0x05);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x40);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x90);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x03);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x28);
		while((UART4->SR&0X40)==0);
		delay1ms(50);

		}	

            	USART_SendData(UART4, 0x05);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x40);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x04);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x06);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0xb1);
		while((UART4->SR&0X40)==0);
		delay1ms(50);

            	USART_SendData(UART4, 0x05);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x40);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x04);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x06);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0xb1);
		while((UART4->SR&0X40)==0);


		delay1ms(800);
		delay1ms(800);
		delay1ms(800);
		delay1ms(800);
		delay1ms(800);
		delay1ms(800);
		delay1ms(800);
		delay1ms(800);
		delay1ms(800);
		delay1ms(800);



	break;



        case Configuration://291

		//if(((hcan.pRxMsg->Data[0])>=0)&&((hcan.pRxMsg->Data[0])<=15))		
			Qsdi_time=hcan.pRxMsg->Data[0];

		if((hcan.pRxMsg->Data[1])<=1)	
			UserLED_Setting=hcan.pRxMsg->Data[1];

	
		UserPrefCurrentFilp=hcan.pRxMsg->Data[2];

		UserTime_Interval=0;
		



	break;




        case Device_Detect:// 430
		hcan.pTxMsg->ExtId =Device_Detect_Reply<<8 | Qsdi_id1;
		TxMessage.Data[0] = 0x01;
		TxMessage.Data[1] = 0x02;
		TxMessage.Data[2] = 0x00;
		TxMessage.Data[3] = 0x00;
		TxMessage.Data[4] = 0x00;
		TxMessage.Data[5] = 0x00;
		TxMessage.Data[6] = 0x00;
		TxMessage.Data[7] = 0x00;
		//CAN_Status = HAL_CAN_Transmit(&hcan,100);
		HAL_CAN_Transmit(&hcan,100);
		//USART1_printf( USART1," CAN_Status %x   \r\n",CAN_Status); 
	break;

    	}

}
else
{




}

	delay1ms(30);


if(Qsdi_time>0)
if(UserTime_Interval>=Qsdi_time)
{
         hcan.pTxMsg->ExtId =QSDI_Status<<8| Qsdi_id1; ;
	  TxMessage.Data[0] = UserOutput_MODE;
	  TxMessage.Data[1] = UserPrefCurrentInput;
	  TxMessage.Data[2] = UserTime_Interval;
	  TxMessage.Data[3] = UserLED_Setting;
	  TxMessage.Data[4] = 0x00;
	  TxMessage.Data[5] = 0x00;
	  TxMessage.Data[6] = 0x00;
	  TxMessage.Data[7] = 0x00;
	 // CAN_Status = HAL_CAN_Transmit(&hcan,100);
	  HAL_CAN_Transmit(&hcan,100);
	 UserTime_Interval=0;


#if 0
if(UserPrefCurrentInput<5)
	UserPrefCurrentInput++;
else
	UserPrefCurrentInput=0;

#endif

	 
}



#endif
	delay1ms(20);

	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9, GPIO_PIN_SET);
	
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_12, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3, GPIO_PIN_SET);
	delay1ms(30);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_12, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3, GPIO_PIN_RESET);


	Input_Selset_GPIO();
	
 	//UserOutput_GPIO();
	Input_Selset_Rs232();
  /* USER CODE END WHILE */
	HDMITX_DevLoopProc();

/*
 USART1_printf( USART1," HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_12) %x   \r\n",HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_12)); 


if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_12) )
{
            	USART_SendData(UART4, 0x05);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x40);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x54);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x01);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x66);
		while((UART4->SR&0X40)==0);
}
else
{
            	USART_SendData(UART4, 0x05);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x40);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x54);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x00);
		while((UART4->SR&0X40)==0);
		USART_SendData(UART4, 0x67);
		while((UART4->SR&0X40)==0);


}

*/
	HDMI_VState=CheckHDMIRX();

	 if(s_CurrentVM.VTotal>1000 && s_CurrentVM.PCLK>125000L)
	 	{
			UserOutput_MODE=Output_1080P;

	 }
	 else if(s_CurrentVM.VTotal<800 && s_CurrentVM.HTotal<1700)

	 	{
			UserOutput_MODE=Output_720P;
	 	}
	else
		{

			UserOutput_MODE=Output_1080I;
		}

  /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{
#if 0
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0);

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
 #else
   RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV2;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0);

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  // SysTick_IRQn interrupt configuration //
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

 #endif
}


//static CanTxMsgTypeDef        TxMessage;  // Static variable declaration
//static CanRxMsgTypeDef        RxMessage;
/* CAN init function */
#if CAN_
void MX_CAN_Init(void)
{
 CAN_FilterConfTypeDef  sFilterConfig;


  hcan.pTxMsg = &TxMessage;
  hcan.pRxMsg = &RxMessage;

  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 8;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SJW = CAN_SJW_1TQ;
  hcan.Init.BS1 = CAN_BS1_8TQ;
  hcan.Init.BS2 = CAN_BS2_7TQ;
  hcan.Init.TTCM = DISABLE;
  hcan.Init.ABOM = DISABLE;
  hcan.Init.AWUM = DISABLE;
  hcan.Init.NART = DISABLE;
  hcan.Init.RFLM = DISABLE;
  hcan.Init.TXFP = DISABLE;


  HAL_CAN_Init(&hcan);



//  HAL_CAN_Init(&hcan);

//16M/8(1+8+7)=125K

  
  #if 1
  //##-2- Configure the CAN Filter ###########################################
  sFilterConfig.FilterNumber = 0;
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
//  sFilterConfig.FilterIdHigh = 0x0000;
//  sFilterConfig.FilterIdLow = 0x0000;
//  sFilterConfig.FilterMaskIdHigh = 0x0000;
//  sFilterConfig.FilterMaskIdLow = 0x0000;
  sFilterConfig.FilterIdHigh = 0x0000;
  sFilterConfig.FilterIdLow = 0x0000;
  sFilterConfig.FilterMaskIdHigh = 0x0000;
  sFilterConfig.FilterMaskIdLow = 0x0000;


  sFilterConfig.FilterFIFOAssignment = 0;
  sFilterConfig.FilterActivation = ENABLE;
  sFilterConfig.BankNumber = 14;

  if (HAL_CAN_ConfigFilter(&hcan, &sFilterConfig) != HAL_OK)
  {
    //Filter configuration Error
    //Error_Handler();
  }
#endif
  /*##-3- Configure Transmission process #####################################*/
  hcan.pTxMsg->StdId = 0x321;
  hcan.pTxMsg->ExtId = 0xf000;
  hcan.pTxMsg->RTR = CAN_RTR_DATA;
  hcan.pTxMsg->IDE = CAN_ID_EXT;
  hcan.pTxMsg->DLC = 8;
  
}

#else
void MX_CAN_Init(void)
{
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 6;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SJW = CAN_SJW_1TQ;
  hcan.Init.BS1 = CAN_BS1_7TQ;
  hcan.Init.BS2 = CAN_BS2_6TQ;
 
  hcan.Init.TTCM = DISABLE;
  hcan.Init.ABOM = DISABLE;
  hcan.Init.AWUM = DISABLE;
  hcan.Init.NART = DISABLE;
  hcan.Init.RFLM = DISABLE;
  hcan.Init.TXFP = DISABLE;
  HAL_CAN_Init(&hcan);


}

#endif

#if 1
/* TIM2 init function */
void MX_TIM2_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 15;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV4;
  HAL_TIM_Base_Init(&htim2);

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig);

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig);

}
#endif

/* I2C2 init function */
void MX_I2C2_Init(void)
{


  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLED;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLED;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLED;
  HAL_I2C_Init(&hi2c2);




}

/* SPI1 init function */
#if 0
void MX_SPI1_Init(void)
{

  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLED;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
  hspi1.Init.CRCPolynomial = 10;
  HAL_SPI_Init(&hspi1);

}
#endif
/* UART4 init function */
void MX_UART4_Init(void)
{

  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart4);

}


/* USART1 init function */
void MX_USART1_UART_Init(void)
{
__HAL_RCC_USART1_CLK_ENABLE();
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart1);

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
     PA9   ------> USART1_TX
     PA10   ------> USART1_RX
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __GPIOD_CLK_ENABLE();
  __GPIOA_CLK_ENABLE();
  __GPIOC_CLK_ENABLE();
  __GPIOB_CLK_ENABLE();

  /*Configure GPIO pins : PA0 PA1 PA2 PA3 
                           PA4 PA5 PA6 PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3 
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);



  /*Configure GPIO pins : PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_15|GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);






  /*Configure GPIO pins : PC4 PC5 PC6 PC7 
                           PC8 PC9 PC12 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7 
                          |GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);




  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
   					|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);







  /*Configure GPIO pins : PB0 PB1 PB2 PB4 
                           PB5 PB6 PB7 PB8 
                           PB9 PB12*/
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_4 
                         |GPIO_PIN_7|GPIO_PIN_8 
                          |GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins :  PB14 PB15 PB3 */
  GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_15|GPIO_PIN_3 |GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);





/*
  //Configure GPIO pin : PA9 //
  GPIO_InitStruct.Pin = GPIO_PIN_9  |GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  //Configure GPIO pin : PA10 //
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
*/
  /*Configure GPIO pin : PD2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : PB10 PB11 i2c */

GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;	 
HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}




#if 0

void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef *CanHandle)
{
  uint8_t tmp;
  if (1/*(CanHandle->pRxMsg->StdId == 0x321) && (CanHandle->pRxMsg->IDE == CAN_ID_STD) && (CanHandle->pRxMsg->DLC == 2)*/)
  {
    tmp = hcan.pRxMsg->Data[0];
    tmp = hcan.pRxMsg->Data[1];
  }

  /* Receive */
  if (HAL_CAN_Receive_IT(CanHandle, CAN_FIFO0) != HAL_OK)
  {
    // Reception Error
    //Error_Handler();
  }
  
  //tmp = hcan.pRxMsg->Data[0];
  //tmp = hcan.pRxMsg->Data[1];

}

#endif



/* USER CODE BEGIN 4 */
/*
int dump_vidmode(void)
{
    USHORT HActive, VActive ;
    USHORT HTotal, VTotal ;
    ULONG HFreq, VFreq ;
    ULONG PCLK ;

    xCntCnt ++ ;
    xCntSum += getHDMIRXxCnt() ;

    HActive = getHDMIRXHorzActive() ;
    VActive = getHDMIRXVertActive() ;
    HTotal = getHDMIRXHorzTotal() ;
    VTotal = getHDMIRXVertTotal() ;

    PCLK = 27000L * 128;
    PCLK *= (ULONG) xCntCnt ;
    PCLK /= (ULONG) xCntSum ;
    PCLK *= 1000L ; // 100*PCLK ;

    HFreq = PCLK / (ULONG)HTotal ; // HFreq
    VFreq = (HFreq*100) / (ULONG)VTotal ; // VFreq * 100



    PCLK = xCntSum ;
    PCLK *= 100 ;
    PCLK /= xCntCnt ;
    PCLK -= (ULONG)(xCntSum / xCntCnt)*100 ;

}
*/
/* USER CODE END 4 */


#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif




/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
