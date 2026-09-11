/**
  ******************************************************************************
  * @file           : mpu.h
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
#ifndef MPU6050_H
#define MPU6050_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_i2c.h"
#include "shell.h"

/* Defines ------------------------------------------------------------*/
#define I2C_RX_FRAME_SIZE	14
#define I2C_RX_BUFFER_SIZE	128
#define MPU6050_DATA_START_REG 0x3B
#define MPU6050_ADDR  0x68
#define I2C_LOG_BUFFER_SIZE 512

/* Variables ---------------------------------------------------------*/
extern I2C_HandleTypeDef hi2c1;

extern volatile uint8_t i2c_sample_flag;
extern volatile uint8_t i2c_frame_ready ;

extern uint8_t i2c_rx[I2C_RX_FRAME_SIZE];
extern uint8_t i2c_rx_buffer[I2C_RX_BUFFER_SIZE];

extern int i2c_sample_count;
extern uint16_t indx;
extern volatile uint32_t i2c_overrun_count;

/* Functions ------------------------------------------------------------------*/
void mpu6050_sample(void);
#endif
