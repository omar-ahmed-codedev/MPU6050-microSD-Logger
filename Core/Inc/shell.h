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
#define UART_RX_BUFFER_SIZE	64
#define UART_TX_BUFFER_SIZE	256
#define MSG_LEN_MAX				64

/* Variables ---------------------------------------------------------*/
extern UART_HandleTypeDef huart2;

extern uint8_t usart_rx_buffer[UART_RX_BUFFER_SIZE];
extern char usart_tx_buffer[UART_TX_BUFFER_SIZE];
extern uint8_t uart_rx_ready;

/* Functions ---------------------------------------------------------*/
void shell_printf(const char *msg, ...);		// ... is ellipsis. variadic function --> accepts any number of additional arguments
void UART_Recieve_Start(void);
void shell_poll(void);
void shell_execute(char *msg);
int parse_int(char *str, uint32_t *val);
#endif
