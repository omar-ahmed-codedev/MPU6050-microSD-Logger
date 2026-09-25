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
#include "sd.h"
#include "stm32f4xx_ll_usart.h"

#include <string.h>		// Byte and string manipulation
#include <stdio.h>		// Declares vsnprintf, used in shell_printf
#include <stdarg.h>		// Standard arguments, machinery for variadic functions.
#include <stdlib.h>		// Standard library.

/* Variables ---------------------------------------------------------*/
uint8_t uart_rx_buffer[UART_RX_BUFFER_SIZE];
char uart_tx_buffer[UART_TX_BUFFER_SIZE];
uint8_t uart_rx_ready = 0;

uint8_t rx_head = 0;
uint8_t rx_tail = 0;
char rx_msg[MSG_LEN_MAX];
uint8_t rx_msg_len = 0;


/* Code ---------------------------------------------------------*/
/**
  * @brief  Intialize recieve on the UART through DMA1. Always running.
  */

void UART_Recieve_Start(void){
	rx_tail = 0;
	rx_msg_len = 0;
	HAL_UARTEx_ReceiveToIdle_DMA(&huart2, uart_rx_buffer, UART_RX_BUFFER_SIZE); // Interrupt fires when idel or when data is more than size
}

/**
  * @brief  UART nterrupt call back function
  * @retval none
  */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size){  //size represents number of bytes received when the callback is called

	if(huart->Instance == USART2){
		uart_rx_ready = 1;
		HAL_UARTEx_ReceiveToIdle_DMA(&huart2, uart_rx_buffer, UART_RX_BUFFER_SIZE); // Call interrupt again to continue receiving data since it was disabled after calling the callback
	}
}

/**
  * @brief  Pol what is received in rx_buffer and hand it to shell_execute.
  * @retval none
  */
void shell_poll(void){
	// Buffer size - NDTR, to get where the recieve msg stopped.
	// NDTR is updated continously.
	rx_head = UART_RX_BUFFER_SIZE - (uint16_t)__HAL_DMA_GET_COUNTER(huart2.hdmarx);

	while (rx_tail != rx_head){			// tail = head when a msg is complete

		char c = uart_rx_buffer[rx_tail];
		rx_tail = (rx_tail+1) % UART_RX_BUFFER_SIZE; 	// Increment tail. Wrap at buffer end.

		if (c == '\r' || c == '\n'){		// When a msg is done
			if (rx_msg_len>0){
				rx_msg[rx_msg_len]= '\0';
				shell_execute(rx_msg);
				rx_msg_len = 0;
			}
		}
		else if (rx_msg_len < (MSG_LEN_MAX - 1)){
			rx_msg[rx_msg_len] = c;
			rx_msg_len++;
		}
		else{
			rx_msg_len = 0;	// Clear the rest of the line in overflow msgs
		}
	}
}


/**
  * @brief  Chech content of received msg, act on it, and print a response.
  * @retval none
  */
void shell_execute(char *msg){

	uint32_t block = 0;		// stroul return unsigned long, which is 32 bits on ARM32 M4

	// Check msg
	// help
	if(strncmp(msg, "help",4)==0){
		shell_printf("\r\n"
					"read block <%d-%d>		Print MPU6050 data stored in that block\r\n"
					"help			this text\r\n\r\n",
					SD_START_BLOCK,
					sd_next_block-1);

	}	// Print a specefic block
	else if (strncmp(msg, "get sd block", 12)==0){
		if(!parse_int(msg+3, &block) || block > sd_next_block){
			shell_printf("Error: invalid block address, expects <Range>!\r\n");
		}
		else if(block<=sd_next_block){
			sd_read_block(block);
			shell_printf("Block %d: \r\n", block);
		}

	}
	else{
		shell_printf("Error: unknown command!\r\n\r\n");
	}
}


/**
  * @brief  Parse a decimal value from a string and reject any trailing junk
  * @retval int, 1 on success, 0 on failure.
  */
int parse_int(char *str, uint32_t *val){

	uint32_t v;		// uint32_t is same size as unsigned long on Cortex M4
	char *end;

	while (*str == ' '){
		str++;
	}
	if(*str<'0' || *str>'9'){
		return 0;
	}

	v=strtoul(str, &end, 10);

	while (*end == ' '){
		end++;
	}

	if (*end!='\0'){
		return 0;

	}

	*val = v;
	return 1;
}


/**
 * @brief  Formats a message and hand it to the TX DMA.
 * * @retval None
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

/**
* @brief Clear overflow flag when it occurs
* @retval None
*/
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        __HAL_UART_CLEAR_OREFLAG(huart);
    }

}
