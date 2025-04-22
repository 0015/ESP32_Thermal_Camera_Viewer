#ifndef __SENXOR_CAPTUREDATA_H__
#define __SENXOR_CAPTUREDATA_H__

#include <stdint.h>
#include <esp_attr.h>			//Using ESP attributes
#include "DrvSPIHost.h"


// Error Code during frame capture or any other access to senxor
#define ERROR_SPIBUSY_INCORRECTLY 		0x01
#define ERROR_INCORRECT_VSYNC			0x02
#define ERROR_DATA_AV_TIMEOUT			0x04
#define ERROR_SPI_ERROR_BIT				0x08
#define ERROR_TRANSFER_UNDERFLOW_BIT	0x10
#define ERROR_TRANSFER_OVERFLOW_BIT		0x20
#define ERROR_FIFO_UNDERFLOW_BIT		0x40
#define ERROR_FIFO_OVERFLOW_BIT			0x80
#define ERROR_INCORRECT_HSYNC			0x100  // no use
#define ERROR_FRAME_SPI_ERROR_BIT		0x200  // no use
#define ERROR_INCORRECT_DATA_IMEOUT		0x4000

#define ERROR_EXT_FLASH_WRITE_ERROR		0x800
#define ERROR_EXT_FLASH_READ_ERROR		0x400

#define ERROR_EEPROM_SERIAL_ALREADY_WRITTEN 0x1000
#define ERROR_EEPROM_VERIFY_ERROR		0x2000
#define ERROR_EEPROM_WRITE_ERROR		32


void IRAM_ATTR Data_AV_FIFO_Int_Handler(void* arg);
#endif //__SENXOR_CAPTUREDATA_H__
