/**
  ******************************************************************************
  * @file           : shell.c
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
#include "shell.h"

#include "stm32f4xx_ll_usart.h"

#include <string.h>		// Byte and string manipulation
#include <stdio.h>		// Declares vsnprintf, used in shell_printf
#include <stdarg.h>		// Standard arguments, machinery for variadic functions.
#include <stdlib.h>		// Standard library.
/* Variables ---------------------------------------------------------*/
uint8_t usart_rx_buffer[USART_RX_BUFFER_SIZE];
char usart_tx_buffer[USART_TX_BUFFER_SIZE];
/* Code ---------------------------------------------------------*/

/**
  * @brief  Formats a message and hand it to the TX DMA.
  */

void shell_printf(const char *str, ...){

    va_list args;   // A type for handling the arguments represented by ..., it iterates over the additional arguments
    int len;

    //while (huart2.gState != HAL_UART_STATE_READY){
        // Wait until UART is ready for another operation after the latest HAL_UART_Transmit_DMA
        // Acceptable because it is only called in the main loop
    //}

    // Intialize args --> args-> first agrument after str (the last named argument of the function)
    va_start(args, str);        // Gives vsnprint access to the arguments supplied through ...

    // Formulate the arguments into a string buffer with a max.
    len = vsnprintf(usart_tx_buffer, USART_TX_BUFFER_SIZE, str, args);        // Difference between snprintf is that it accepts vardiac (accepts additional arguments)
                                                               // Len recieves the bytes written or would have been written if the buffer is not long enough in the buffer. (minus 1(\0))
    va_end(args);      // Finished using va_start
    if (len > 0){
        if(len> (int) USART_TX_BUFFER_SIZE){
            len = (int) USART_TX_BUFFER_SIZE;
        }
        // Starts the transmission and returns before all the bytes have physically been transmitted.
        HAL_UART_Transmit(&huart2,(uint8_t *)usart_tx_buffer, (uint16_t) len,100);
    }

}


