/*****************************************************************************
 * @file     DrvLED.h
 * @version  1.00
 * @brief    LED control function
 * @date	 29 May 2023
 ******************************************************************************/
#ifndef COMPONENTS_DRIVERS_INCLUDE_DRVLED_H_
#define COMPONENTS_DRIVERS_INCLUDE_DRVLED_H_
#include <math.h>
#include <esp_log.h>
#include <driver/ledc.h>
#include <driver/gpio.h>		//Using HAL GPIO library
#include <hal/gpio_ll.h>		//Using ESP low level GPIO library
#include "led_strip.h"
#include "msg.h"

#if (CONFIG_ESPMODEL_S3MINI) || (CONFIG_ESPMODEL_S3MINI_C) || (CONFIG_ESPMODEL_MIFD)
#define CONFIG_MI_LED_LED_MAX 3
#elif (CONFIG_ESPMODELS3_DevKitC1) || (CONFIG_ESPMODEL_S3EYE) || (CONFIG_ESPMODEL_S3MINI_C)
#define CONFIG_MI_LED_LED_MAX 1
#endif

#define DRV_LEDTAG							"[LED]"
#define DRV_LED_ERR_INIT_ARG				"Invalid LED type. LED not initialised."
#define DRV_LED_ERR_NOT_EN					"LED features not enabled in sdkconfig."
#define DRV_LED_ERR_RMT_NOT_EN				"RMT is not enabled in sdkconfig."
#define DRV_LED_ERR_RMT_NOT_SUPPORTED		"RMT is not supported on this device. Connected devices (such as LED Strip) that use RMT will be unavailable on this platform."
#define DRV_LED_ERR_RMT_HANDLE_NULL			"LED Strip handler is not initialised. Try again by reinitialising the LED strip."
#define DRV_LED_INIT						"Initialising LED driver"
#define DRV_LED_MODE_GPIO					"Using GPIO-controlled LED"
#define DRV_LED_MODE_PWM					"Using PWM-controlled LED"
#define DRV_LED_MODE_RMT					"Using RMT based LED Strip"



typedef enum{
#if (CONFIG_ESPMODEL_S3MINI ) || (CONFIG_ESPMODEL_S3MINI_C) || (CONFIG_ESPMODEL_MIFD)
	LED_ON,
	LED_OFF
#else
	LED_OFF,
	LED_ON
#endif
}Drv_LED_Status;


typedef enum{
	LED_GPIO,
	LED_STRIP,
	LED_GPIO_STRIP,
	LED_OTHER
}Drv_LED_Type;


typedef struct ledSingleStruct{
	Drv_LED_Status status;
	uint32_t ledPin;
	uint32_t flashInterMs;
}ledSingleStruct;

typedef struct ledGrpStruct{
	ledSingleStruct ledSingleStructObj[CONFIG_MI_LED_LED_MAX];
}ledGrpStruct;
#if CONFIG_MI_LED_EN
#if CONFIG_ESPMODEL_S3_OTHER == 1

		//If other board is used. Define customised LED pins here.
		#define LED_PIN 	 		0				//LED pin 1
		#define LED_PIN_1			1				//LED pin 2
		#define LED_PIN_B 	 		LED_PIN				//LED pin blue
#else
	#if CONFIG_ESPMODEL_S3EYE == 1 || CONFIG_ESPMODELS3_DevKitC1 == 1
		#define LED_PIN 	 		3				//LED pin
		#define LED_PIN_SEL 		BIT64(LED_PIN)
		#define LED_PIN_B 	 		LED_PIN				//LED pin blue
	#else
		#define LED_PIN_R 			1				//LED pin red
		#define LED_PIN_G 	 		38				//LED pin green
		#define LED_PIN_B 	 		2				//LED pin blue
		#define LED_PIN_SEL 		BIT64(LED_PIN_R) | BIT64(LED_PIN_G) | BIT64(LED_PIN_B)
		#define LED_PIN_SEL_PWM 	LED_PIN_R | LED_PIN_B | LED_PIN_G
	#endif
#endif

#if CONFIG_MI_LED_PWM_EN
#define LEDC_DUTY               	CONFIG_MI_LED_PWM_INTI_DCYE			// Set duty to 50%. ((2 ** 13) - 1) * 50% = 4095
#define LEDC_FREQUENCY          	CONFIG_MI_LED_PWM_FREQ				// Frequency in Hertz. Set frequency at 5 kHz
#endif

void Drv_LED_Init(Drv_LED_Type _type);

void Drv_LED_Gpio_Single_En(Drv_LED_Status isEnabled);

void Drv_LED_Gpio_En(uint32_t pin, Drv_LED_Status isEnabled);

void Drv_LED_PWM_setDuty(ledc_channel_t channel, uint8_t duty);

void Drv_LED_PWM_setFade(ledc_channel_t channel, uint8_t duty, int duration);

void Drv_LED_Strip_setPixel(uint32_t idx, uint32_t R, uint32_t G, uint32_t B);

void Drv_LED_Strip_Clr(void);
#endif

#endif /* COMPONENTS_DRIVERS_INCLUDE_DRVLED_H_ */
