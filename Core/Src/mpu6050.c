/**
  ******************************************************************************
  * @file           : mpu6050.c
  * @brief          :
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

/* Includes ------------------------------------------------------------------*/
#include "mpu6050.h"



/* Variables ---------------------------------------------------------*/
extern I2C_HandleTypeDef hi2c1;

volatile uint8_t i2c_sample_flag = 0;
volatile uint8_t i2c_frame_ready  = 0;

uint8_t i2c_rx_buffer[I2C_RX_FRAME_SIZE];
uint8_t mpu_log_buffer[MPU_LOG_BUFFER_SIZE];

int i2c_sample_count = 0;
uint16_t indx = 0;
volatile uint32_t i2c_overrun_count = 0;
int count;
int count2;
uint8_t start_timer = 0;

/**
* @brief mpu6050 initialization
* @retval None
*/
void mpu6050_init(void){


	// Confirm the chip is there
	uint8_t id =0;			// id should be 0x68
	if(HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR << 1, MPU6050_WHO_AM_I_REG, I2C_MEMADD_SIZE_8BIT, &id, 1, 100) == HAL_OK){
	    shell_printf("MPU found, WHO_AM_I=0x%02X\r\n", id);
	}
	else {
		uint32_t error = HAL_I2C_GetError(&hi2c1);
		shell_printf("WHO_AM_I: error=0x%08lX id=0x%02X\r\n",
			             (unsigned long)error, (unsigned int)id);
	}

	// Wake sensor up ans set clock.
	uint8_t pwr = 0x01; 	// Sleep = 0.  CLKSEL = 0x01 -> set PLL with X-axis gyroscope reference.
	if (HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR << 1, MPU6050_PWR_MGMT_1_REG, I2C_MEMADD_SIZE_8BIT, &pwr, 1, 1000)== HAL_OK) {

		    shell_printf("MPU is awake\r\n");
	}
	else{
		 shell_printf("MPU wake failed\r\n");
	}


	// Sample Rate = Gyroscope Output Rate / (1 + SMPLRT_DIV)
	uint8_t config = 0x03;       // Select output rate as 1 kHz. DLPF_CFG = 3
	uint8_t sample_div = 199;      // 1 kHz / (1 + 199) = 5Hz
	if (HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR << 1, MPU6050_CONFIG_REG, I2C_MEMADD_SIZE_8BIT, &config,  1, 100) == HAL_OK){
		 shell_printf("Gyro output rate 1kHz selected\r\n");
	}
	else{
		shell_printf("Failed to set Gyro output rate\r\n");
	}
	if (HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR << 1, MPU6050_SMPLRT_DIV_REG, I2C_MEMADD_SIZE_8BIT, &sample_div, 1, 100) == HAL_OK){
		 shell_printf("Sample rate set at 5Hz\r\n");
	}
	else{
		shell_printf("Failed to set Sample rate rate\r\n");
	}


	// Config gyro range.
	uint8_t gyro_cfg = (0 << 3);   // FS_SEL=0 -> ±250 °/s

	if (HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR << 1, MPU6050_GYRO_CONFIG_REG, I2C_MEMADD_SIZE_8BIT, &gyro_cfg, 1, 100) == HAL_OK){
			 shell_printf("Gyro range set at ±250 °/s\r\n");
	}
	else{
			shell_printf("Failed to set set gyro range\r\n");
	}

	// Config acceleration range.
	uint8_t accel_cfg = (0 << 3);  // AFS_SEL=0 -> ±2 g
	if (HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR << 1, MPU6050_ACCEL_CONFIG_REG, I2C_MEMADD_SIZE_8BIT, &accel_cfg, 1, 100)== HAL_OK){
		shell_printf("Accelerometer range set at ±2 g\r\n");
	}
	else{
		shell_printf("Failed to set set accelerometer range\r\n");
	}

}


/**
* @brief Sample mpu6050
* @retval None
*/
void mpu6050_sample(void){
	// Sensor by defualt updates its registers with 8kHz (DLPF_CFG = 0)
	HAL_I2C_Mem_Read_DMA(&hi2c1, MPU6050_ADDR << 1, MPU6050_DATA_START_REG,
						I2C_MEMADD_SIZE_8BIT, i2c_rx_buffer, I2C_RX_FRAME_SIZE);

						/* Static so these are easy to inspect in the debugger.
						static volatile HAL_StatusTypeDef dma_status;
						static volatile uint32_t dma_error;

						i2c_frame_ready = 0;

						dma_status = HAL_I2C_Mem_Read_DMA(
						    &hi2c1, MPU6050_ADDR << 1,
						    MPU6050_DATA_START_REG,
						    I2C_MEMADD_SIZE_8BIT,
						    i2c_rx_buffer, I2C_RX_FRAME_SIZE);

						HAL_Delay(100);  // Temporary diagnostic wait, in main context
						dma_error = HAL_I2C_GetError(&hi2c1);

						shell_printf("DMA status=%d, error=0x%08lX, frame_ready=%u\r\n",
						             (int)dma_status,
						             (unsigned long)dma_error,
						             (unsigned int)i2c_frame_ready);
*/

}


/**
* @brief I2C DMA receive interrupt call back
* @retval None
*/
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c){

	if (hi2c->Instance == I2C1){
		i2c_frame_ready = 1;

	}
}


/**
* @brief Handle I2C error
* @retval None
*/
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c){

	if (hi2c->Instance == I2C1){
        i2c_sample_flag = 0;
        i2c_frame_ready = 0;
        shell_printf("I2C error!\r\n");
    }


}
