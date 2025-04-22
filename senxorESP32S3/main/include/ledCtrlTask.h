/*****************************************************************************
 * @file     ledCtrlTask.h
 * @version  1.0
 * @brief    Task controlling LEDs
 * @date	 27 Feb 2023
 * @Author	 Sarashina Ruka
 ******************************************************************************/
#ifndef MAIN_INCLUDE_LEDCTRLTASK_H_
#define MAIN_INCLUDE_LEDCTRLTASK_H_
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "DrvLED.h"

#define LED_TASK_STACK_SIZE		2096
#define DEFAULT_FLASH_INTER_MS	5000

//Debug messages
#define LEDTAG							"[LED]"
#define LED_ERR_INIT_ARG				"Invalid LED type. LED not initialised."
#define LED_ERR_NOT_EN					"LED features not enabled in sdkconfig."
#define LED_ERR_RMT_NOT_EN				"RMT is not enabled in sdkconfig."
#define LED_ERR_RMT_NOT_SUPPORTED		"RMT is not supported on this device. Connected devices (such as LED Strip) that use RMT will be unavailable on this platform."
#define LED_ERR_RMT_HANDLE_NULL			"LED Strip handler is not initialised. Try again by reinitialising the LED strip."
#define LED_ERR_TASK_FAIL_INIT			"LED task failed to initialised."
#define LED_INIT_INFO					"Initialising LED"
#define LED_MODE_GPIO					"Using GPIO-controlled LED"
#define LED_MODE_PWM					"Using PWM-controlled LED"
#define LED_MODE_RMT					"Using RMT based LED Strip"
#define LED_TASK_INFO					"SenXor task started, running on core %d ."
#define LED_TASK_EXIT					"SenXor task exiting..."

typedef enum ledColour
{
	OFF,
	RED_LED,
	GREEN_LED,
	BLUE_LED,
	YELLOW_LED,
	AQUA_LED,
	PINK_LED,
	ALL
}ledColour;


//public:
void ledCtrlTask_Init();

void ledCtrlTask(void * pvParameters);

void ledCtrlSingle(ledGrpStruct* ledGrpStruct);

void ledCtrlSingleClr(ledGrpStruct* ledGrpStruct);

void ledCtrlSingleSet(ledColour colour, Drv_LED_Status status, uint32_t flashInterMs);

void ledCtrlPwmSet(uint8_t r, uint8_t g, uint8_t b, uint16_t fadeDuration);
#endif /* MAIN_INCLUDE_LEDCTRLTASK_H_ */
