/**
  ******************************************************************************
  * @file           : shell.h
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

#ifndef SHELL_H
#define SHELL_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"


/* Defines ------------------------------------------------------------------*/
#define USART_RX_BUFFER_SIZE	64
#define USART_TX_BUFFER_SIZE	256

/* Variables ---------------------------------------------------------*/
extern UART_HandleTypeDef huart2;

extern uint8_t usart_rx_buffer[USART_RX_BUFFER_SIZE];
extern char usart_tx_buffer[USART_TX_BUFFER_SIZE];

/* Functions ---------------------------------------------------------*/
void shell_printf(const char *msg, ...);		// ... is ellipsis. variadic function --> accepts any number of additional arguments

#endif
