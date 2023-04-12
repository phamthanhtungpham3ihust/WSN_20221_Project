/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"
#include "math.h"
#include "DS18B20.h"
#include "stdio.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum option_t{
	TRANSPARENT,
	POINT2POINT
} option_t;

typedef enum mode_lora {
	  General_Mode,
	  Power_Saving_Mode,
	  Wake_up_Mode,
	  Sleep_Mode
} mode_lora;

typedef enum Trang_thai{
	FAIL,
	SUCCEED
} Trang_thai;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
 TIM_HandleTypeDef htim4;
UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
  float temp;
	DS18B20_Name DS1;
	int temp_tenth, temp_unit, temp_decimal;
  char data[4];
  char data1[]="NO1";
	uint8_t flag=0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM4_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// Function that changing mode of module Lora 
Trang_thai change_mode(GPIO_TypeDef *GPIOx_AUX, uint16_t GPIO_Pin_AUX,
					   GPIO_TypeDef *GPIOx_M0, uint16_t GPIO_Pin_M0,
					   GPIO_TypeDef *GPIOx_M1, uint16_t GPIO_Pin_M1, mode_lora mode)
{
	GPIO_PinState read_pin_AUX = HAL_GPIO_ReadPin(GPIOx_AUX,GPIO_Pin_AUX);
	while(read_pin_AUX==GPIO_PIN_RESET){

	};
	HAL_Delay(5);
	if(read_pin_AUX==GPIO_PIN_SET){
		switch(mode)
			{
			case General_Mode:
				HAL_GPIO_WritePin(GPIOx_M0,GPIO_Pin_M0 , GPIO_PIN_RESET);
			  HAL_GPIO_WritePin(GPIOx_M1, GPIO_Pin_M1, GPIO_PIN_RESET);
			HAL_Delay(10);
			break;
			case Power_Saving_Mode:
				HAL_GPIO_WritePin(GPIOx_M0, GPIO_Pin_M0, GPIO_PIN_SET);
			  HAL_GPIO_WritePin(GPIOx_M1,GPIO_Pin_M1 , GPIO_PIN_RESET);
			HAL_Delay(10);
			break;
			case Wake_up_Mode:
				HAL_GPIO_WritePin(GPIOx_M0, GPIO_Pin_M0, GPIO_PIN_RESET);
			  HAL_GPIO_WritePin(GPIOx_M1,GPIO_Pin_M1 , GPIO_PIN_SET);
			HAL_Delay(10);
			break;
			case Sleep_Mode:
				HAL_GPIO_WritePin(GPIOx_M0,GPIO_Pin_M0 , GPIO_PIN_SET);
			  HAL_GPIO_WritePin(GPIOx_M1, GPIO_Pin_M1, GPIO_PIN_SET);
			HAL_Delay(10);
			break;
			default:
				HAL_GPIO_WritePin(GPIOx_M0,GPIO_Pin_M0 , GPIO_PIN_RESET);
			  HAL_GPIO_WritePin(GPIOx_M1,GPIO_Pin_M1 , GPIO_PIN_RESET);
			HAL_Delay(10);
				break;
		}
	};

	while(read_pin_AUX == GPIO_PIN_RESET)
	{

	};
	HAL_Delay(5);
if (read_pin_AUX	==GPIO_PIN_SET)
	{
		return SUCCEED;
	}
	else
	{
		return FAIL;
	}

}
// function that confige parameter for module Lora
void config_lora(UART_HandleTypeDef *huart, int baudrate,
					 uint8_t add_high_byte, uint8_t add_low_byte, option_t option)
{
	uint8_t SPEED;
	uint8_t OPTION;
	uint8_t ADDH = add_high_byte;
	uint8_t ADDL = add_low_byte;


	/* 433MHz */
	uint8_t CHAN = 0x17;
	switch(baudrate)
	{
		case 1200:
			SPEED = 0x02;
			break;
		case 2400:
			SPEED = 0x0A;
			break;
		case 4800:
			SPEED = 0x12;
			break;
		case 9600:
			SPEED = 0x1A;
			break;
		case 19200:
			SPEED = 0x22;
			break;
		case 38400:
			SPEED = 0x2A;
			break;
		case 57600:
			SPEED = 0x32;
			break;
		case 115200:
			SPEED = 0x3A;
			break;
		default:
			SPEED = 0x1A;
	};

	switch(option)
	{
		case TRANSPARENT:
			OPTION = 0x40 ;
			break;
		case POINT2POINT:
			OPTION = 0xC0;
			break;
		default:
			OPTION = 0x40;
		break;
	};

	uint8_t config_cmd[] = {0xC0, ADDH, ADDL, SPEED, CHAN, OPTION};

	/* Configure */

	HAL_UART_Transmit(huart, config_cmd, sizeof(config_cmd), 100);
	HAL_Delay(150);
}

// ISR of UART
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  // Check received data
   if(strcmp(data, data1) == 0){
    // First request of Gateway
		 if(flag==0){
      // Read Sensor
				temp=DS18B20_ReadTemp(&DS1);
				temp_tenth=(int)((temp)/10);
				temp_unit =(int)((int)temp%10);
				temp_decimal=(int)((temp-temp_tenth*10-temp_unit)*10);
				HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
      // Create message
				uint8_t TX_Buffer[]={0x00, 0x20, 0x17, temp_tenth+48, temp_unit+48, temp_decimal+48};
      // Transmit to module Lora
				HAL_UART_Transmit(huart, TX_Buffer, sizeof(TX_Buffer),100);
				flag=1;
		 }
     // Sencond request of Gateway (when message is missed in first time)
	   else if(flag==1){
				uint8_t TX_Buffer[]={ 0x00, 0x20, 0x17, temp_tenth+48, temp_unit+48, temp_decimal+48};
				HAL_UART_Transmit(huart, TX_Buffer, sizeof(TX_Buffer),100);
				flag=0;
		}
	 }
	 else{
		 flag=0;
	 }
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
	Trang_thai state;
  // Change to Sleep mode to configure
	state = change_mode(GPIOA, GPIO_PIN_3, GPIOA,GPIO_PIN_1, GPIOA, GPIO_PIN_2, Sleep_Mode);
	if(state==FAIL){
		state = change_mode(GPIOA, GPIO_PIN_3, GPIOA,GPIO_PIN_1, GPIOA, GPIO_PIN_2, Sleep_Mode);
	}
  // Configure address and channel
	config_lora(&huart1,9600,0x00, 0x01, POINT2POINT);
  // Change to General mode
	state = change_mode(GPIOA, GPIO_PIN_3, GPIOA,GPIO_PIN_1, GPIOA, GPIO_PIN_2, General_Mode);
  // Initilize sensor
	DS18B20_Init(&DS1, &htim4, DS18B20_GPIO_Port, DS18B20_Pin);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // Enable UART interrupt
	  HAL_UART_Receive_IT(&huart1, data, 3);
    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */

		/*
  temp=DS18B20_ReadTemp(&DS1);
	temp_tenth=(int)((temp)/10);
	temp_unit =(int)((int)temp%10);
	temp_decimal=(int)((temp-temp_tenth*10-temp_unit)*10);
	HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
	uint8_t TX_Buffer[]={0x00, 0x20, 0x17, temp_tenth+48, temp_unit+48, temp_decimal+48};
	HAL_UART_Transmit(&huart1, TX_Buffer, sizeof(TX_Buffer),100);
	HAL_Delay(5000);
  }
		*/
  /* USER CODE END 3 */
}
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 72-1;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 65535;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, M0_Lora_Pin|M1_Lora_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(DS18B20_GPIO_Port, DS18B20_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : M0_Lora_Pin M1_Lora_Pin */
  GPIO_InitStruct.Pin = M0_Lora_Pin|M1_Lora_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : AUX_Lora_Pin */
  GPIO_InitStruct.Pin = AUX_Lora_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(AUX_Lora_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : DS18B20_Pin */
  GPIO_InitStruct.Pin = DS18B20_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DS18B20_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
