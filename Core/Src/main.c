/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bsp.h"
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


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  systickConfigure();
//  // 初始化
//	//W25QXX_Init();
//	// 芯片flash大小
//	uint32_t FLASH_SIZE = 8*1024*1024;	// FLASH 大小8M字节
//
//	printf("------------- 读取芯片ID实验 --------------- \r\n");
//	uint16_t Deceive_ID;
//
//	Deceive_ID = W25QXX_ReadID();
//	if (Deceive_ID == 0)
//	{
//		printf("Read Deceive_ID fail \r\n");
//	} else {
//		printf("Deceive_ID is %X \r\n", Deceive_ID); // 显示芯片ID
//	}
//	
//	printf("------------- 读写 字节实验 --------------- \r\n");
//	uint8_t string[] = {"HAOZI TEST"};
//	uint8_t getStringBuf[sizeof(string)] = {"&&&&&&&&&&"};		// 初始值
//	
//	W25QXX_Write(string, 0, sizeof(string));
//	W25QXX_Read(getStringBuf, 0, sizeof(string));
//	if (getStringBuf[0] == '&')
//	{
//		printf("Read string fail \r\n");
//	} else {
//		printf("Read string is: %s \r\n", getStringBuf);
//	}
//	
//	printf("------------- 读写 浮点数实验 --------------- \r\n");
//	// 浮点数 读写测试
//	union float_union{
//		float float_num;			// 浮点数占4个字节
//		double double_num;			// 双精度浮点数占8个字节
//		uint8_t buf[8];				// 定义 8个字节 的空间
//	};
//	union float_union write_float_data;	// 用来写
//	union float_union read_float_data;	// 用来读
//	
//	// 先测试第一个 浮点数
//	write_float_data.float_num = 3.1415f;
//	read_float_data.float_num = 0;
//	W25QXX_Write(write_float_data.buf, 20, 4);
//	W25QXX_Read(read_float_data.buf, 20, 4);
//	if(read_float_data.float_num == 0)
//	{
//		printf("Read float fail \r\n");
//	} else {
//		printf("Read float data is %f \r\n", read_float_data.float_num);
//	}
//	// 再测试第二个 双精度浮点数
//	write_float_data.double_num = 3.1415;
//	read_float_data.double_num = 0;
//	
//	W25QXX_Write(write_float_data.buf, 20, 8);
//	W25QXX_Read(read_float_data.buf, 20, 8);
//	if(read_float_data.float_num == 0)
//	{
//		printf("Read double fail \r\n");
//	} else {
//		printf("Read double data is %.15f \r\n", read_float_data.double_num);
//	}
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    //loop();
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
