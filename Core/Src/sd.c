/**
  ******************************************************************************
  * @file           : sd.c
  * @brief          :
  ******************************************************************************
  * @details
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "sd.h"
#include "main.h"
#include "peripheral_config.h"

/* Defines ------------------------------------------------------------------*/
#define	SD_START_BLOCK	100000	//51 MB in

/* Variables ---------------------------------------------------------*/

/* Code ---------------------------------------------------------*/

void cs_low(void){ HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET); }
void cs_high(void){ HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET); }


/**
  * @brief Transfer a byte to the SD card
  * @retval none
  */
uint8_t sd_xfer(uint8_t out){

	uint8_t in = 0xFF;
	HAL_SPI_TransmitReceive(&hspi1, &out, &in, 1, 100);		//Receive in. 1 = size of frames
	return in;

}

/**
  * @brief Format and send a command to the SD card
  * @retval none
  */
void sd_send_command(uint8_t cmd, uint32_t arg, uint8_t crc){

	sd_xfer(0xFF);
	sd_xfer(0x40 | cmd);
	sd_xfer((uint8_t)(arg>>24)); //last byte of arg
    sd_xfer((uint8_t)(arg >> 16));
    sd_xfer((uint8_t)(arg >> 8));
    sd_xfer((uint8_t)(arg));	// first byte of arg
    sd_xfer(crc);

}
/**
  * @brief Read SD response
  * @retval response
  */
uint8_t sd_read_r1(void){

	for(uint8_t i = 0; i < 10; i++){	// Send 1111 1111 10 times or until a valid reponse is received.
		uint8_t r = sd_xfer(0xFF);
		if((r&0x80)==0){return r;}		// When idle, SD always has 1 at bit 7 as MISO is driven high and 0 in a valid response
	}
    return 0xFF;                      // nothing came back
}

/**
  * @brief Send a command and read response
  * @retval none
  */

uint8_t sd_send_read_r1(uint8_t cmd, uint32_t arg, uint8_t crc){

	sd_send_command(cmd, arg, crc);
	return sd_read_r1();
}

