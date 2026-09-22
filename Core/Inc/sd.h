/**
  ******************************************************************************
  * @file           : sd.h
  * @brief          :
  ******************************************************************************
  * @details
  *
  ******************************************************************************
  */

#ifndef SD_H
#define SD_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include <stdint.h>
/* Defines ------------------------------------------------------------------*/
// LOG_BUFFER_SIZE	512
// SD_CS_Pin


/* Variables ---------------------------------------------------------*/
extern uint8_t	sd_initialized;
extern uint32_t sd_next_block;

typedef enum {
	SD_OK = 0,
	SD_ERROR_CMDO,
	SD_ERROR_CMD8,
	SD_ERROR_ACMD41,
	SD_ERROR_CMD58,
	SD_ERROR_WRITE,
	SD_ERROR_TIMEOUT,
	SD_ERROR_NOT_INIT

} sd_status_t;

/* Functions ---------------------------------------------------------*/
uint8_t sd_xfer(uint8_t out);
void sd_send_command (uint8_t cmd, uint32_t arg, uint8_t crc);
void cs_low(void);
void cs_high(void);
sd_status_t sd_init(void);

#endif
