#ifndef DRV_DRVGPIO_H_
#define DRV_DRVGPIO_H_
#include <esp_log.h>				//ESP Logger
#include <FreeRTOS/FreeRTOS.h>		//Using FreeRTOS task library
#include <FreeRTOS/task.h>			//Using FreeRTOS task library
#include <driver/ledc.h>			//Using ESP PWM library
#include <driver/gpio.h>			//Using HAL GPIO library
#include <driver/rtc_io.h>			///Using HAL GPIO RTC library
#include <hal/gpio_ll.h>			//Using ESP low level GPIO library

#include "DrvLED.h"					//LED driver for controlling LED
#include "DrvNVS.h"					//NVS driver
#include "SenXorLib.h"				//SenXorLib
#include "Senxor_Capturedata.h"
#include "msg.h"

/*****************************************************************************
 * @brief   GPIO pins definition
 * 			Define GPIO pins here. The numbers represent which GPIO channel
 * 			is utilised.
 * 			Not use in Couager: CAPTURE, SSDATAN, SSREGN
 * 			For example:
 * 			PIN_CAPTURE 8 means GPIO 8 is connected to the pin named "CAPUTRE"
*****************************************************************************/
#define PIN_BOOT		0

/*
 *	ESP32S3-EYE (OCTAL PSRAM)
 *	ESP32S3-DevKitC1 (QUAD PSRAM)
 */
#if (CONFIG_ESPMODEL_S3EYE) || (CONFIG_ESPMODELS3_DevKitC1)
#define PIN_DATA_AV  		14								//Input pin - Indicate data is ready in SenXor
#define PIN_SSDATAN  		45								//Output pin - SPI Chip select to read data from SenXor
#define PIN_SSFLASHN 		15								//Output pin - SPI Chip select to access FLASH on SenXor
#define PIN_SYSRST 	 		46								//Output pin  - SenXor reset. (Active low)
/*
 * 	ESP32S3 MINI
 */
#elif CONFIG_ESPMODEL_S3MINI
#define PIN_DATA_AV  		4								//Input pin - Indicate data is ready in SenXor
#define PIN_SSDATAN  		16								//Output pin - SPI Chip select to read data from SenXor
#define PIN_SSFLASHN 		5								//Output pin - SPI Chip select to access FLASH on SenXor
#define PIN_SYSRST 	 		17								//Output pin  - SenXor reset. (Active low)

/*
 * 	ESP32S3 MINI REV.C / Fire detector
 */
#elif (CONFIG_ESPMODEL_S3MINI_C) || (CONFIG_ESPMODEL_MIFD)
#define PIN_DATA_AV  		4									//Input pin - Indicate data is ready in SenXor
#define PIN_SSDATAN  		16									//Output pin - SPI Chip select to read data from SenXor
#define PIN_SSFLASHN 		5									//Output pin - SPI Chip select to access FLASH on SenXor
#define PIN_SYSRST 	 		17									//Output pin  - SenXor reset. (Active low)
#define PIN_SXR_PWR_DWN		3									//Power down pins

	#ifdef CONFIG_MI_BAT_CHARGE_EN
	#define PIN_CHARGING		CONFIG_MI_BAT_PIN_CHARGING			//Charging pins
	#define PIN_PG				CONFIG_MI_BAT_PIN_PG				//Power down pins
	#endif
/*
 * 	Customised pins
 */
#elif (CONFIG_ESPMODEL_S3_OTHER)
#define PIN_DATA_AV  		CONFIG_MI_SENXOR_PIN_DATA_AV		//Input pin - Indicate data is ready in SenXor
#define PIN_SSDATAN  		CONFIG_MI_SENXOR_SPI_CS				//Output pin - SPI Chip select to read data from SenXor
#define PIN_SSFLASHN 		CONFIG_MI_SENXOR_SPI_CS_FLASH		//Output pin - SPI Chip select to access FLASH on SenXor
#define PIN_SYSRST 	 		CONFIG_MI_SENXOR_PIN_RESET			//Output pin  - SenXor reset. (Active low)
#endif

/*****************************************************************************
 * @brief   GPIO pins masking
 *			Masking GPIO pins to 64 bits for initialising GPIO
*****************************************************************************/
#if (CONFIG_ESPMODEL_S3EYE) || (CONFIG_ESPMODELS3_DevKitC1)
#define GPIO_PIN_SEL  		BIT64(PIN_SSDATAN) | BIT64(PIN_SSFLASHN) | BIT64(PIN_SYSRST)
#elif (CONFIG_ESPMODEL_S3MINI)
#define GPIO_PIN_SEL  		BIT64(PIN_SSDATAN) | BIT64(PIN_SSFLASHN) | BIT64(PIN_SYSRST)
#elif (CONFIG_ESPMODEL_S3MINI_C)
#define GPIO_PIN_SEL  		BIT64(PIN_SSDATAN) | BIT64(PIN_SSFLASHN) | BIT64(PIN_SYSRST) | BIT64(PIN_SXR_PWR_DWN)
#elif (CONFIG_ESPMODEL_S3_OTHER)
#define GPIO_PIN_SEL		BIT64(PIN_SSDATAN) | BIT64(PIN_SSFLASHN) | BIT64(PIN_SYSRST)
#endif

//DATA_AV pins are universal and must be initialised
#define GPIO_PIN_SEL_INPUT  BIT64(PIN_DATA_AV)


/******************************************************************************
 * @brief       Function prototyping
 *****************************************************************************/
void Drv_Gpio_Init(void);										//Initialise GPIO

void Drv_Gpio_Init_BatRpt(void);

void Drv_Gpio_Init_DATA_IRQ(void* pvParameters);				//Initialise DataAV interrupt

void Drv_Gpio_CAPTURE_PIN_Set(const uint8_t OnOf);

void Drv_Gpio_Disable_Capture_interupt(void);					//Holder. Not in use

void Drv_Gpio_Enable_Capture_interupt(void);					//Holder. Not in use

void Drv_Gpio_Enable_MCU_PIN(const int OnOff);					//Holder. Not in use

void Drv_Gpio_HOST_DATA_READY_Set(const uint8_t OnOff);			//SSDATAN toggle

void Drv_Gpio_LED_PIN_Set(uint8_t OnOff);

void Drv_Gpio_SSDATAN_PIN_Set (const uint8_t OnOff);

void Drv_Gpio_SSFLASH_PIN_Set(const uint8_t OnOff);

void Drv_Gpio_SSREGN_PIN_Set(const uint8_t OnOff);

void Drv_Gpio_SXR_PWR_DWN_PIN_Set(const uint8_t OnOff);

void Drv_Gpio_RESET_N_PIN_Set(const uint8_t OnOff);

void Drv_Gpio_WRPROT_PIN_Set(const uint8_t OnOff);


#endif /* DRV_DRVGPIO_H_ */
