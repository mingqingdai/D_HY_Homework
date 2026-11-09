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
#include "i2c.h"
#include "stm32f1xx_hal_uart.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
#include <string.h>
#include "drv8833.h"
#include "font.h"
#include <stdio.h>
#include "oled.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define COUNT_MID 20



/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

  int count = 0;
  int speed = 0;
  int displaySpeed = 1;
  char message[20]="";

  uint8_t rxBuffer; // 接收缓冲区


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart==&huart2) // 检查是否是 USART2
    {
        // 处理接收到的数据
        // 这里可以根据需要处理 rxBuffer 中的数据
        // 例如，将其存储到一个队列中，或者直接处理
      if(rxBuffer == '1')
      {
        
        __HAL_TIM_SET_COUNTER(&htim1, 16);
      } 
      else if(rxBuffer == '2')
      {
        __HAL_TIM_SET_COUNTER(&htim1, 12);
      }
      else if(rxBuffer == '3')
      {
        __HAL_TIM_SET_COUNTER(&htim1, 8);
      }
      else if(rxBuffer == '4')
      {
        __HAL_TIM_SET_COUNTER(&htim1, 4);
      }
      else if(rxBuffer == '5')
      {
        __HAL_TIM_SET_COUNTER(&htim1, 0);
      }
      else if(rxBuffer == '0')
      {
        __HAL_TIM_SET_COUNTER(&htim1, 20);
      }


        // 重新启动接收中断，以便接收下一个字节
        HAL_UART_Receive_IT(&huart2, &rxBuffer, 1);
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
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  HAL_Delay(20);
  OLED_Init();

  DRV8833_Init();
  HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_ALL);
  __HAL_TIM_SET_COUNTER(&htim1, COUNT_MID);

HAL_UART_Receive_IT(&huart2, &rxBuffer, 1); // 启动 USART2 的接收中断


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  
    OLED_NewFrame();// 开启新的一帧显示

    count = (int)__HAL_TIM_GET_COUNTER(&htim1);// 获取编码器计数值

    if(count > 60000)// 编码器计数值溢出，限制在最大位置
    {
      count=COUNT_MID*2;
      __HAL_TIM_SET_COUNTER(&htim1, COUNT_MID*2);
    }
    else if(count < 0)
    {
      // 编码器反向越过 0 时，限制在最小位置
      count=0;
      __HAL_TIM_SET_COUNTER(&htim1, 0);// 将计数器设置为 0
    }
    else if(count>COUNT_MID*2)// 编码器计数值超过最大位置，限制在最大位置
    {
      count=COUNT_MID*2;// 将计数器设置为最大位置
      __HAL_TIM_SET_COUNTER(&htim1, COUNT_MID*2);// 将计数器设置为最大位置
    }
    if(count<COUNT_MID)// 编码器计数值小于中间位置，限制在最小位置
    {
      speed=(COUNT_MID-count)*100/COUNT_MID;
      DRV8833_Backward(speed);
    }
    else
    {
      speed=(count-COUNT_MID)*100/COUNT_MID;
      DRV8833_Forward(speed);
    }

    displaySpeed = speed;
    if (displaySpeed < 1)
    {
      displaySpeed = 0;
    }
    else if (displaySpeed > 100)
    {
      displaySpeed = 100;
    }

    sprintf(message, "speed:%d", displaySpeed);
    OLED_PrintString(13, 0, message, &font16x16, OLED_COLOR_NORMAL);
    OLED_DrawRectangle(13, 25, 101, 12, OLED_COLOR_NORMAL);
    OLED_DrawFilledRectangle(13, 26, displaySpeed, 11, OLED_COLOR_NORMAL);





    OLED_ShowFrame();
    HAL_Delay(100);





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
