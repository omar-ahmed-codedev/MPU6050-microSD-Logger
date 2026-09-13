/**
 ******************************************************************************
 * @file    peripheral_config.h
 * @author  Omar Ahmed
 * @brief   Peripheral initialization function declarations
 ******************************************************************************
 */

#ifndef PERIPHERAL_CONFIG_H
#define PERIPHERAL_CONFIG_H

#include "stm32f4xx_hal.h"
#include "main.h"

/* Private variables ---------------------------------------------------------*/
extern I2C_HandleTypeDef hi2c1;
extern DMA_HandleTypeDef hdma_i2c1_rx;
extern DMA_HandleTypeDef hdma_i2c1_tx;

extern SPI_HandleTypeDef hspi1;
extern DMA_HandleTypeDef hdma_spi1_rx;
extern DMA_HandleTypeDef hdma_spi1_tx;

extern TIM_HandleTypeDef htim2;

extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart2_tx;

/* Function prototypes */
void SystemClock_Config(void);
void MX_I2C1_Init(void);
void MX_SPI1_Init(void);
void MX_TIM2_Init(void);
void MX_USART2_UART_Init(void);
void MX_DMA_Init(void);
void MX_GPIO_Init(void);

#endif
