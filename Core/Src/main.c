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
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"


/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "mpu6050.h"
#include "shell.h"
#include "peripheral_config.h"
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

// SPI
volatile uint8_t spi_log_flag = 0;


// USART
//static uint8_t usart_tx_buffer[];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */



/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
* @brief TIM2 interrupt callback. Signal mpu sample every 1000 ms
* @retval None
*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){

	if (htim->Instance == TIM2){
		if (hi2c1.State == HAL_I2C_STATE_READY){
			i2c_sample_flag = 1;
		}
		else{
			i2c_overrun_count++;
		}
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);

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
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();

  /* USER CODE BEGIN 2 */
  mpu6050_init();
  HAL_TIM_Base_Start_IT(&htim2);	 // Enable timer interrupt after the mpu6050 has been
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */

	  // Sample MPU6050 every 1000ms
	  if (i2c_sample_flag){
		 mpu6050_sample();
		 i2c_sample_flag = 0;
	  }

	  // Save received frame bytes in buffer
	  if (i2c_frame_ready){
		 for (uint8_t i = 0; i < I2C_RX_FRAME_SIZE; i++){
			 i2c_rx_buffer[indx++] = i2c_rx[i];
			 if (indx == I2C_RX_BUFFER_SIZE){	// Check if frame still fits 512 bytes
				spi_log_flag = 1;		// Flag SD writing
				 break;
			 }
		 }
		 i2c_frame_ready = 0;
	  }

	  // Print overrun error if the timer ticked while the i2c was receiving
	  if (i2c_overrun_count > 0){
	      shell_printf("I2C sample overrun: %u!\r\n", i2c_overrun_count);
	  }



  }
  /* USER CODE END 3 */
}



