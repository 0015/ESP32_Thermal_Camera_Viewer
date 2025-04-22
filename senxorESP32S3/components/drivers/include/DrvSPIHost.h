#ifndef DRV_DRVSPI_H_
#define DRV_DRVSPI_H_

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "driver/spi_master.h"
#include "esp_private/spi_common_internal.h"
#include "esp_log.h"
#include "hal/spi_hal.h"
#include "hal/gdma_ll.h"

#include "defines.h"
#include "DrvGPIO.h"
#include "SenXorLib.h"
#include "SenXor_FLASH.h"
#include "msg.h"

/******************************************************************************
 * @brief       SPI pins assignment
 *****************************************************************************/

#if CONFIG_ESPMODEL_S3EYE == 1 || CONFIG_ESPMODELS3_DevKitC1 == 1
#define PIN_SPI_CLK  	39							//GPIO port for SPI Clock
#define PIN_SPI_MOSI	38							//GPIO port for MOSI
#define PIN_SPI_MISO	41							//GPIO port for MISO
#else
#define PIN_SPI_CLK  	6							//GPIO port for SPI Clock
#define PIN_SPI_MISO	7							//GPIO port for MISO
#define PIN_SPI_MOSI	15							//GPIO port for MOSI

#endif
/******************************************************************************
 * @brief       SPI clock speed selection
 *****************************************************************************/
#define DEFAULT_SPI_CLK_SPD		            14000000
#define SPI_CLK_SPEED			            DEFAULT_SPI_CLK_SPD	//20MHz
#define SPI_CLK_SPEED_5M		            5000000				//5MHz
#define SPI_CLK_SPEED_6M		            6000000				//6MHz
#define SPI_CLK_SPEED_10M		            10000000			//10MHz
#define SPI_CLK_SPEED_14M		            14000000			//14MHz
#define SPI_CLK_SPEED_20M		            20000000			//20MHz

#define SELECT_SPICLK_14M		            1					// 14MHz FOR netd function target clock speed in HZ
#define SELECT_SPICLK_5M		            0					// 5MHz FOR netd function target clock speed in HZ
#define SELECT_SPICLK_10M		            2					// 10MHz FOR netd function target clock speed in HZ
#define SELECT_SPICLK_6M		            3					// 6MHz FOR netd function target clock speed in HZ
#define SELECT_SPICLK_20M		            4					// 6MHz FOR netd function target clock speed in HZ (fastest, 10 defaults to 12)
/******************************************************************************
 * @brief       Flash size & SPI DATA LENGTH
 *****************************************************************************/
#define CALIBDATA_FLASH_START_ADDRESS		0x50000								/* CALIBDATA_FLASH_BASE address				*/
#define CALIBDATA_FLASH_END_ADDRESS   	  	0x67180								/* Data Flash test start address            */
#define CALIBDATA_FLASH_SIZE	            (CALIBDATA_FLASH_END_ADDRESS-CALIBDATA_FLASH_START_ADDRESS)/2
#define DEFAULT_SPI_LENGTH 		            16
#define FLASH_SPI_LENGTH 		            8
/******************************************************************************
 * @brief       Functions prototyping
 *****************************************************************************/

void DataFrameReceiveError(void);

void Drv_SPI_Init(void);

void Drv_SPI_SENXOR_Init(uint32_t clk_speed,const uint8_t FlashEnable);

uint32_t Drv_SPI_Senxor_ConstructData(const uint8_t reg, const uint8_t write, const uint8_t data);

int Drv_SPI_Senxor_Write_Reg(const uint8_t reg, const uint8_t data);

int Drv_SPI_Senxor_Read_Reg(const uint8_t reg);

uint8_t Drv_SPI_Senxor_Read_8Bit(void);

void Drv_SPI_Senxor_Write_8Bit(const uint8_t data);

void Drv_SPI_Host_PDMA_Disable(void);

uint16_t IRAM_ATTR Drv_SPI_Read(void);

void IRAM_ATTR Drv_SPI_Write(void);

uint16_t IRAM_ATTR Drv_SPI_Transmit(void);

uint16_t IRAM_ATTR Drv_SPI_DMA_Transmit(void);

void Drv_SPI_DMA_Disable(void);

void Drv_SPI_DMA_Enable(void);

void Drv_SPI_DMA_PrepDesc(uint16_t *txBuff, uint16_t *rxBuff,const int dataLen);

void Read_CalibrationData(void);


#endif /* DRV_DRVSPI_H_ */
