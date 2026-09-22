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
uint8_t	 sd_block_addressing = 0;
uint8_t	 sd_initialized 	= 0;
uint32_t sd_next_block 		= 0;

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

	for(uint8_t i = 0; i < 10; i++){	// Send 1111 1111 ten times or until a valid reponse is received.
		uint8_t r = sd_xfer(0xFF);
		if((r&0x80)==0){return r;}		// When idle, SD always has 1 at bit 7 as MISO is driven high and 0 in a valid response
	}									// return once a response is valid
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

/**
  * @brief SD card intialization
  * @retval sd_status_t value
  */

sd_status_t sd_init(void){

	uint8_t  r1;
	uint8_t  resp[4];
	uint32_t t0;

	/* Wake card up by sending 74+ clocks with CS and MOSI High */
	HAL_Delay(10);
	cs_high();
	for (int i = 0; i < 10; i++) { sd_xfer(0xFF); }// 10*8 = 80 clocks

	/* Start communicating */
	cs_low();

	/* Enter SPI mode. Command->CMDO = 	40 00 00 00 00 95 */
	r1 = 0xFF;
	for(uint8_t i = 0; i<10 && r1!= 0x01; i++){		// Try 10 times. First attempt often fails
		r1 = sd_send_read_r1(0, 0x00000000, 0x95);		// Expect r1 = 0x01(idle)
	}
	if (r1 != 0x01) { cs_high(); return SD_ERROR_CMDO;}	// Raise an error if r1 is still not 0x01

	/* Check version and if the card support the specified voltage range. Command: CMD8	48 00 00 01 AA 87 */
	r1 = sd_send_read_r1(8, 0x000001AA, 0x87);		// Expect r1 = 0x01(idle)
	// 0x1 -> host has operating range of 2.7-3.6V
	// AA is a chosen pattern for the card to echo back
	if (r1 != 0x01) { cs_high(); return SD_ERROR_CMD8; }
	for (uint8_t i = 0; i < 4; i++){ resp[i]=sd_xfer(0xFF); }

	if (resp[2] != 0x01 || resp[3] != 0xAA) { cs_high(); return SD_ERROR_CMD8; }

	/* Finish intialization and leave idle state. Command->ACMD41 = 69 40 00 00 00 01*/
	// Poll until intialization is complete. 0x01 busy, then 0x00 ready. Can take up  to 1000 ms.
	// Aplication specific commands must be preceeded by the prefix CMD55 = 77 00 00 00 00 01. R1 = 0x01 while still idle
	t0 = HAL_GetTick();
	do{
		sd_send_read_r1(55, 0, 0x01);	// CMD55
		r1 = sd_send_read_r1(41, 0x40000000, 0x01);	// ACMD41
		if (HAL_GetTick() - t0 > 1000) { cs_high(); return SD_ERROR_ACMD41; }
	 } while (r1 != 0x00);

	/* Check if intialization is complete. Command: CMD58	7A 00 00 00 00 01 */
	r1 = sd_send_read_r1(58, 0x00000000, 0x01);
	if (r1 != 0x00) { cs_high(); return SD_ERROR_CMD58; }
	for (int i = 0; i < 4; i++){ resp[i] = sd_xfer(0xFF); }// OCR bytes follow R1 directly
	// OCR[31]: intitialization complete
	if ((resp[0] & 0x80) == 0) { cs_high(); return SD_ERROR_CMD58;}

	// OCR[30]: 1 = SDHC/SDXC block addressing
	sd_block_addressing = (resp[0] & 0x40) ? 1 : 0;

	cs_high();
	sd_xfer(0xFF);

	 // Now, it is safe to speed up.
	  __HAL_SPI_DISABLE(&hspi1);
	  hspi1.Instance->CR1 = (hspi1.Instance->CR1 & ~SPI_CR1_BR_Msk)
	                         | SPI_BAUDRATEPRESCALER_8;     /* 84/8 = 10.5 MHz */
	  __HAL_SPI_ENABLE(&hspi1);

	  sd_next_block= SD_START_BLOCK;
	  sd_initialized = 1;

	  return SD_OK;
}



