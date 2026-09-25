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
#include "sd.h"
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
I2C_HandleTypeDef hi2c1;
DMA_HandleTypeDef hdma_i2c1_rx;
DMA_HandleTypeDef hdma_i2c1_tx;

SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_rx;
DMA_HandleTypeDef hdma_spi1_tx;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart2_tx;

/* USER CODE BEGIN PV */
uint8_t next = 0;
uint8_t sd_write_failed = 0;
// USART
//static uint8_t usart_tx_buffer[];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
* @brief TIM2 interrupt callback. Signal mpu sample every 1000 ms
* @retval None
*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){

	if (htim->Instance == TIM2){
		//if (hi2c1.State == HAL_I2C_STATE_READY){
			i2c_sample_flag = 1;
		//}
		//else{
			//i2c_overrun_count++;
		//}
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
  UART_Recieve_Start();

  sd_status_t result;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  /* Read uart rx when a frame is received */
	  if (uart_rx_ready){
		  uart_rx_ready = 0;
	  	  shell_poll();
	  }



	  /* Sample MPU6050 every 1000ms */
	  if (i2c_sample_flag && !sd_buffer_log_flag && !i2c_frame_ready){
		 mpu6050_sample();
		 convert_display_live_mpu6050();
		 i2c_sample_flag =0;
	  }

	  /* Save received frame bytes in buffer */
	  if (i2c_frame_ready && !sd_buffer_log_flag){
		  if(indx <= MPU_LOG_DATA_SIZE-I2C_RX_FRAME_SIZE){
			 for (uint8_t i = 0; i < I2C_RX_FRAME_SIZE; i++){
				  mpu_log_buffer[indx++] = i2c_rx_buffer[i];
			 }
			 i2c_frame_ready = 0;
		  }
		  if (indx == MPU_LOG_DATA_SIZE){	// Check if frame still fits 512 bytes
			 for (uint16_t i = MPU_LOG_DATA_SIZE; i < LOG_BUFFER_SIZE; i++){
				 mpu_log_buffer[i] = 0xFF;
			  }
			  sd_buffer_log_flag = 1;		// Flag SD writing

		  }
	  }

	  /* Fill write buffer when log buffer is full */
	  if (sd_buffer_log_flag && !sd_write_flag){
		  for (uint16_t i = 0; i<LOG_BUFFER_SIZE; i++){
		  sd_write_buffer[i]=mpu_log_buffer[i];
		  }
		  indx = 0;
		  sd_buffer_log_flag = 0;
		  sd_write_flag = 1;

	  }

	  /* Write buffer in SD */
	  if (sd_write_flag && !sd_write_failed){
		  if (!sd_write_in_progress){
			  result = sd_write_block(sd_write_buffer);
		  }
		  else{
			  result = sd_write_poll();
		  }

		  if(result == SD_OK){
			  sd_write_flag = 0;
			  shell_printf("Data block written to card.\r\n");
		  }
		  else if (result != SD_BUSY){
			  sd_write_failed = 1;
			  shell_printf("SD write failed: %d\r\n", result);
		  }
	  }



	  /* Print overrun error if the timer ticked while the i2c was receiving */
	  if (i2c_overrun_count > 0){
	      shell_printf("I2C sample overrun: %u!\r\n", i2c_overrun_count);
	  }



  }
  /* USER CODE END 3 */
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
