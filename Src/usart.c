/**
  ******************************************************************************
  * File Name          : USART.c
  * Description        : This file provides code for the configuration
  *                      of the USART instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */
	uint8_t RX_temp;
	uint8_t *RX_buff;
	uint8_t *RX_buff_origin;
	uint16_t RX_len;
	int8_t RX_sta = 0;
/* USER CODE END 0 */

UART_HandleTypeDef huart1;

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */
    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();
  
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration    
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspInit 1 */
	if((RX_buff = (uint8_t *)malloc(RX_BUF_LEN)) == NULL)
		return;
	RX_buff_origin = RX_buff;
  /* USER CODE END USART1_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();
  
    /**USART1 GPIO Configuration    
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX 
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
} 

/* USER CODE BEGIN 1 */
#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the EVAL_COM1 and Loop until the end of transmission */
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
  
  return ch;
}

#define PACK
#ifdef PACK
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);
	
	if(RX_sta == 0){
		RX_buff = RX_buff_origin;
		RX_len = 0x0000;
		memset(RX_buff, 0, RX_BUF_LEN);
	}
	
	*RX_buff = RX_temp;
	RX_buff++;
	switch(RX_sta)
	{
		case(0):
				//ascii(0x01): SOH(start of heading)
			RX_sta = (RX_temp==0x01 && ((RX_buff-1)==RX_buff_origin))? 1:-1;
			break;
		case(1):
			if((RX_buff-1) == (RX_buff_origin+1)){
				RX_len |= (uint16_t)RX_temp << 8;
				RX_sta = 2;
			}
			else  RX_sta = -1;
			break;
		case(2):
			if((RX_buff-1) == (RX_buff_origin+2)){
				RX_len |= (uint16_t)RX_temp;
				RX_sta = 3;
			}
			else RX_sta = -1;
			break;
		case(3):
				//ascii(0x04): EOT(end of transmission)
				//RX_buff_origin+3+RX_len: the address that EOT should be 
			if((RX_buff-1)==(RX_buff_origin+3+RX_len)){
				if(RX_temp == 0x04)
					RX_sta = 4;
				else  RX_sta = -1;
			}
			else  ;		//receive text
			break;
	}
	if(RX_sta == -1){
		RX_sta = 0;
		printf("ERROR!\r\n");
	}
		
	
	HAL_UART_Receive_IT(&huart1, (uint8_t *)&RX_temp, 1);	//turn on uart interrupt again
}
#else
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);
	
	*RX_buff =  RX_temp;
	if((*(RX_buff-1) == '\r') && (*RX_buff == '\n')){
		RX_sta = 1;
		*(RX_buff-1) = '\0';
		RX_buff = RX_buff_origin;
	}
	else
		RX_buff++;
	
	HAL_UART_Receive_IT(&huart1, (uint8_t *)&RX_temp, 1);	//turn on uart interrupt again
}

#endif

void Check_RX_buffer_Initialization(void)
{
	if(RX_buff == NULL || RX_buff_origin == NULL || RX_buff != RX_buff_origin)
		printf("RX_buff initialize failed!\r\n");
	else{
		printf("RX_buff initialize successed!\r\n");
		printf("Memory address is 0x%x \r\n", (unsigned int)RX_buff);
	}

}

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
