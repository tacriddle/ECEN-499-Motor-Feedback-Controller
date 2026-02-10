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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MLX_ADDR (0x0C << 1)
#define CMD_SB    0x10  // Start Burst (for continuous mode)
#define CMD_RM    0x4E  // Read Measurement (X, Y, Z, T)
#define CMD_RT    0xF0  // Reset
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
uint8_t buffer[10];

int16_t status;
int16_t x_mag;
int16_t y_mag;
int16_t z_mag;

volatile uint8_t data_ready_flag = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
void MLX90393_PowerOn_Sequence();
void MLX90393_Init();
void MLX90393_VerifyConfig();
void MLX90393_Verify_Reg2();
void MLX90393_ReadData(int16_t *status, int16_t *x, int16_t *y, int16_t *z);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);
    return ch;
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
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
//  MLX90393_Init();
  MLX90393_PowerOn_Sequence();
  printf("Verify Config...\r\n");
  MLX90393_VerifyConfig();
  printf("Verify Reg2...\r\n");
  MLX90393_Verify_Reg2();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  if (data_ready_flag) {
		  data_ready_flag = 0;
		  MLX90393_ReadData(&status, &x_mag, &y_mag, &z_mag);
		  printf("Status: 0x%X | X: %d | Y: %d | Z: %d \r\n", status, x_mag, y_mag, z_mag);
	  }


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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : MLX_INT_Pin */
  GPIO_InitStruct.Pin = MLX_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(MLX_INT_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void MLX90393_PowerOn_Sequence() {
    uint8_t cmd;
    uint8_t config[4];

    // 1. EXIT Burst Mode (Crucial: Register writes only work in IDLE)
    cmd = 0x80;
    HAL_I2C_Master_Transmit(&hi2c1, MLX_ADDR, &cmd, 1, 100);
    HAL_Delay(50);

    // 2. Full Reset
    cmd = 0xF0;
    HAL_I2C_Master_Transmit(&hi2c1, MLX_ADDR, &cmd, 1, 100);
    HAL_Delay(100);

    // 3. Write Register 2 (Enable INT_SER_EN)
    // We take the default 0x1D5E and set Bit 15 -> 0x9D5E
    // (Note: Some modules use 0x8000 + default bits. Let's try 0x9D5E)
    config[0] = 0x60; // WR Command
    config[1] = 0x9D; // High Byte (Bit 15 is INT_SER_EN)
    config[2] = 0x5E; // Low Byte
    config[3] = 0x08; // Reg 2 << 2

    HAL_I2C_Master_Transmit(&hi2c1, MLX_ADDR, config, 4, 100);
    HAL_Delay(50);

    // 4. Start Burst (X, Y, Z)
    cmd = 0x1E;
    HAL_I2C_Master_Transmit(&hi2c1, MLX_ADDR, &cmd, 1, 100);
    HAL_Delay(20);
}

void MLX90393_Init() {
    uint8_t cmd[4];

    // 1. Reset the sensor
    cmd[0] = 0xF0;
    HAL_I2C_Master_Transmit(&hi2c1, MLX_ADDR, cmd, 1, 100);
    HAL_Delay(20);

    // 2. Enable INT pin (Write Register 2)
    // Command 0x60 (WR), Data 0x8000 (INT_SER_EN), Reg 0x02 << 2 = 0x08
    uint8_t config_int[] = {0x60, 0x80, 0x00, 0x08};
    HAL_I2C_Master_Transmit(&hi2c1, MLX_ADDR, config_int, 4, 100);
    HAL_Delay(10);

    // 3. Start Burst Mode for X, Y, Z (Command 0x1E)
    cmd[0] = 0x1E;
    HAL_I2C_Master_Transmit(&hi2c1, MLX_ADDR, cmd, 1, 100);
}

void MLX90393_VerifyConfig() {
    uint8_t read_reg_cmd = 0x50 | (0x02 << 2); // RR command for Reg 2
    uint8_t rx[3]; // Status + 2 bytes data
    HAL_I2C_Master_Transmit(&hi2c1, MLX_ADDR, &read_reg_cmd, 1, 100);
    HAL_I2C_Master_Receive(&hi2c1, MLX_ADDR, rx, 3, 100);
    printf("Reg 2 Value: 0x%02X%02X\r\n", rx[1], rx[2]);
}

void MLX90393_Verify_Reg2() {
    uint8_t rr_cmd = 0x58; // RR (Read Register) for Register 2
    uint8_t rx[3];
    HAL_I2C_Master_Transmit(&hi2c1, MLX_ADDR, &rr_cmd, 1, 100);
    HAL_I2C_Master_Receive(&hi2c1, MLX_ADDR, rx, 3, 100);
    // rx[0] is status, rx[1] is High Byte, rx[2] is Low Byte
    printf("Verify Reg 2: Status 0x%02X | Data: 0x%02X%02X\r\n", rx[0], rx[1], rx[2]);
}

void MLX90393_ReadData(int16_t *status, int16_t *x, int16_t *y, int16_t *z) {
    // 0x4E is the RM (Read Measurement) command for X, Y, Z
    uint8_t read_cmd = 0x4E;
    uint8_t data[7];

    HAL_I2C_Master_Transmit(&hi2c1, MLX_ADDR, &read_cmd, 1, 100);

    if (HAL_I2C_Master_Receive(&hi2c1, MLX_ADDR, data, 7, 100) == HAL_OK) {
        *status = data[0];
        *x = (int16_t)((data[1] << 8) | data[2]);
        *y = (int16_t)((data[3] << 8) | data[4]);
        *z = (int16_t)((data[5] << 8) | data[6]);
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == MLX_INT_Pin) {
        data_ready_flag = 1;
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
