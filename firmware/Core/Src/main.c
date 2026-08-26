/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "ds18b20.h"
#include <string.h>
#include "wire.h"
#include "a4988.h"
#include "esp8266.h"
#include "ili9341.h"
#include "max6675.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
ESP8266_Handle_t esp8266;

volatile uint8_t enter_button_pressed = 0;
volatile uint32_t enter_button_last_tick = 0;

int16_t encoder_previous = 0;
int16_t encoder_current = 0;
int16_t encoder_delta = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//int _write(int file, char *ptr, int len)
//{
//    HAL_UART_Transmit(&huart2, (uint8_t*)ptr, len, HAL_MAX_DELAY);
//    return len;
//}

int _write(int file, char *ptr, int len)
{
    (void)file;

    HAL_UART_Transmit(
        &huart2,
        (uint8_t *)ptr,
        len,
        100
    );

    return len;
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
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  MX_TIM6_Init();
  MX_USART1_UART_Init();
  MX_TIM3_Init();
  MX_SPI3_Init();
  MX_SPI2_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */

  HAL_TIM_Base_Start(&htim1);
  uint8_t rom[8];
  float temp;
  char line[20];


//  ESP8266_Result_t esp_result;
//
//  esp_result = ESP8266_Init(
//      &esp8266,
//      &huart1,
//      &huart2
//  );
//
//  if (esp_result == ESP8266_OK)
//  {
//      printf("ESP8266: inicjalizacja OK\r\n");
//  }
//  else
//  {
//      printf("ESP8266: inicjalizacja ERROR\r\n");
//  }
//
//  HAL_Delay(2000);
//
//  ESP8266_SendCommand(
//      &esp8266,
//      "AT+CWMODE=1"
//  );
//
//  HAL_Delay(1000);
//
//  ESP8266_ScanNetworks(&esp8266);

//  uint32_t encoder_last_print_time = 0;
//  int16_t encoder_last_printed = 0;
//
//  HAL_TIM_Encoder_Start(
//      &htim3,
//      TIM_CHANNEL_ALL
//  );
//
//  encoder_previous =
//      (int16_t)__HAL_TIM_GET_COUNTER(&htim3);
//
//  printf("Test enkodera uruchomiony\r\n");

//  ILI9341_Init();
//
//  ILI9341_FillScreen(
//      ILI9341_RED
//  );
//
//  HAL_Delay(1000);
//
//  ILI9341_FillScreen(
//      ILI9341_GREEN
//  );
//
//  HAL_Delay(1000);
//
//  ILI9341_FillScreen(
//      ILI9341_BLUE
//  );

  if (ds18b20_init() != HAL_OK) {
    Error_Handler();
  }

  uint8_t ds1[DS18B20_ROM_CODE_SIZE];

  if (ds18b20_read_address(ds1) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	    ds18b20_start_measure(NULL);
	    HAL_Delay(750);
	    float temp = ds18b20_get_temp(NULL);
	    if (temp >= 80.0f)
	      printf("Sensor error...\n");
	    else
	      printf("T1 = %.1f*C\n", temp);


//	  uint32_t now = HAL_GetTick();
//
//	      encoder_current =
//	          (int16_t)__HAL_TIM_GET_COUNTER(&htim3);
//
//	      encoder_delta =
//	          encoder_current - encoder_previous;
//
//	      if (encoder_delta != 0)
//	      {
//	          encoder_previous = encoder_current;
//	      }
//
//	      if ((now - encoder_last_print_time) >= 100)
//	      {
//	          if (encoder_current != encoder_last_printed)
//	          {
//	              printf(
//	                  "CNT = %d\r\n",
//	                  encoder_current
//	              );
//
//	              encoder_last_printed =
//	                  encoder_current;
//	          }
//
//	          encoder_last_print_time = now;
//	      }
//
//
//	      if (enter_button_pressed)
//	      {
//	          enter_button_pressed = 0;
//
//	          printf("ENTER - przycisk wcisniety\r\n");
//	      }
//
//	      HAL_Delay(1);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 50;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == ENTER_button_Pin)
    {
        uint32_t now = HAL_GetTick();

        if ((now - enter_button_last_tick) > 50)
        {
            enter_button_pressed = 1;
            enter_button_last_tick = now;
        }
    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
