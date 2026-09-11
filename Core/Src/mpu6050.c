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

uint8_t i2c_rx[I2C_RX_FRAME_SIZE];
uint8_t i2c_rx_buffer[I2C_RX_BUFFER_SIZE];

int i2c_sample_count = 0;
uint16_t indx = 0;
volatile uint32_t i2c_overrun_count = 0;


/**
* @brief Sample mpu6050
* @retval None
*/
void mpu6050_sample(void){

	HAL_I2C_Mem_Read_DMA(&hi2c1, MPU6050_ADDR << 1, MPU6050_DATA_START_REG, 		// MEM: Memory/register operation
						I2C_MEMADD_SIZE_8BIT, i2c_rx, I2C_RX_FRAME_SIZE);

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
