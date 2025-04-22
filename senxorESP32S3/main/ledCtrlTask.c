/*****************************************************************************
 * @file     ledCtrlTask.c
 * @version  1.1
 * @brief    Task controlling LEDs
 * @date	 27 Feb 2023
 * @Author	 Sarashina Ruka
 ******************************************************************************/
#include "ledCtrlTask.h"
#if CONFIG_MI_LED_EN
//public:
QueueHandle_t ledTaskQueue = NULL;
TaskHandle_t ledCtrlTaskHandle = NULL;				//LED Task

//private:
#if (CONFIG_MI_LED_TYPE_VAL) == 0 && (CONFIG_MI_LED_PWM_EN) == 0
EXT_RAM_BSS_ATTR static ledGrpStruct ledGrpStructObj;				//Single LED control object
#endif

static uint8_t R = 0;
static uint8_t G = 0;
static uint8_t B = 0;
static uint16_t mFadeDuration = 0;

/*
 * ***********************************************************************
 * @brief       ledCtrlTask
 * @param       pvParameters - Command arguments for a task
 * @return      None
 * @details     LED Control task
 **************************************************************************/
void ledCtrlTask(void * pvParameters)
{

	ESP_LOGI(LEDTAG,LED_TASK_INFO,xPortGetCoreID());
	ESP_LOGI(LEDTAG,MAIN_FREE_RAM " / " MAIN_TOTAL_RAM,heap_caps_get_free_size(MALLOC_CAP_INTERNAL), heap_caps_get_total_size(MALLOC_CAP_INTERNAL));								//Display the total amount of DRAM
	ESP_LOGI(LEDTAG,MAIN_FREE_SPIRAM " / " MAIN_TOTAL_SPIRAM,heap_caps_get_free_size(MALLOC_CAP_SPIRAM), heap_caps_get_total_size(MALLOC_CAP_SPIRAM));								//Display the total amount of PSRAM

	ledGrpStruct* pLedGrpStructObj;
	ledSingleStruct ledSingleStructObj;

	ledCtrlTask_Init();																//Initialise LED task

	while(1)
	{
		#if (CONFIG_MI_LED_TYPE_VAL) == 0 && (CONFIG_MI_LED_PWM_EN) == 0
			ledCtrlSingle(&ledGrpStructObj);

			if(ledGrpStructObj.ledSingleStructObj[0].flashInterMs > 2)
			{
				vTaskDelay(pdMS_TO_TICKS(ledGrpStructObj.ledSingleStructObj[0].flashInterMs/2));
				ledCtrlSingleClr(&ledGrpStructObj);
				vTaskDelay(pdMS_TO_TICKS(ledGrpStructObj.ledSingleStructObj[0].flashInterMs/2));
			}//End if
		#endif
	#if CONFIG_MI_LED_PWM_EN
		#if (CONFIG_ESPMODEL_S3MINI) || (CONFIG_ESPMODEL_S3MINI_C) || (CONFIG_ESPMODEL_MIFD)
			if(mFadeDuration > 100)
			{

				Drv_LED_PWM_setFade(LEDC_CHANNEL_0, R, mFadeDuration);
				Drv_LED_PWM_setFade(LEDC_CHANNEL_1, G, mFadeDuration);
				Drv_LED_PWM_setFade(LEDC_CHANNEL_2, B, mFadeDuration);
				vTaskDelay(pdMS_TO_TICKS(mFadeDuration*3));
				Drv_LED_PWM_setFade(LEDC_CHANNEL_0, 0, mFadeDuration);
				Drv_LED_PWM_setFade(LEDC_CHANNEL_1, 0, mFadeDuration);
				Drv_LED_PWM_setFade(LEDC_CHANNEL_2, 0, mFadeDuration);
				vTaskDelay(pdMS_TO_TICKS(mFadeDuration*3));
			}
			else
			{
				Drv_LED_PWM_setDuty(LEDC_CHANNEL_0, R);
				Drv_LED_PWM_setDuty(LEDC_CHANNEL_1, G);
				Drv_LED_PWM_setDuty(LEDC_CHANNEL_2, B);
			}//End if-else
		#endif
	#endif
		vTaskDelay(1);
	}//End for

}//End ledCtrlTask

/*
 * ***********************************************************************
 * @brief       ledCtrlTask_Init
 * @param       None
 * @return      None
 * @details     LED Control task initialisation
 **************************************************************************/
void ledCtrlTask_Init(void)
{
	ESP_LOGI(LEDTAG,LED_INIT_INFO);
/*
 * 	Initialise object and queues
 */

#if CONFIG_MI_LED_PWM_EN
	ledCtrlPwmSet(128, 128, 128, 100);
#else
	#if (CONFIG_MI_LED_TYPE_VAL) == 0 && (CONFIG_MI_LED_PWM_EN) == 0

		for(uint8_t i = 0 ; i < CONFIG_MI_LED_LED_MAX ; ++i)
		{
			ledGrpStructObj.ledSingleStructObj[i].flashInterMs = DEFAULT_FLASH_INTER_MS;
			#if CONFIG_ESPMODEL_S3MINI == 1 || CONFIG_ESPMODEL_S3MINI_C == 1 || CONFIG_ESPMODEL_MIFD == 1
			switch(i)
			{
				case 0:
					ledGrpStructObj.ledSingleStructObj[i].ledPin = LED_PIN_R;
				break;
				case 1:
					ledGrpStructObj.ledSingleStructObj[i].ledPin = LED_PIN_G;
				break;
				case 2:
					ledGrpStructObj.ledSingleStructObj[i].ledPin = LED_PIN_B;
				break;
			}//End switch
			#else
				ledGrpStructObj.ledSingleStructObj[i].ledPin = LED_PIN;
			#endif
			ledGrpStructObj.ledSingleStructObj[i].status = LED_OFF;
		}//End for

		ledCtrlSingle(&ledGrpStructObj);

	#endif

#endif

}//End ledCtrlTask_Init

/*
 * ***********************************************************************
 * @brief       ledCtrlSingle
 * @param       ledGrpStruct - Object contains a list of LED objects
 * @return      None
 * @details     Toggle a list of LED
 **************************************************************************/
void ledCtrlSingle(ledGrpStruct* ledGrpStruct)
{
#if CONFIG_MI_GPIO_LED_MAX == 1
	Drv_LED_Gpio_En(ledGrpStruct->ledSingleStructObj[0].ledPin, ledGrpStruct->ledSingleStructObj[0].status);
#else
	for(uint8_t i = 0 ; i < CONFIG_MI_LED_LED_MAX ; ++i)
	{
		Drv_LED_Gpio_En(ledGrpStruct->ledSingleStructObj[i].ledPin, ledGrpStruct->ledSingleStructObj[i].status);
	}//End for
#endif
}//End ledCtrlSingle

/*
 * ***********************************************************************
 * @brief       ledCtrlSingleClr
 * @param       None
 * @return      None
 * @details     Clear LED
 **************************************************************************/
void ledCtrlSingleClr(ledGrpStruct* ledGrpStruct)
{
#if CONFIG_MI_LED_TYPE_VAL == 0
	for(uint8_t i = 0 ; i < CONFIG_MI_LED_LED_MAX ; ++i)
	{
		Drv_LED_Gpio_En(ledGrpStruct->ledSingleStructObj[i].ledPin, LED_OFF);
	}//End for
#endif
}//End ledCtrlSingleClr

/******************************************************************************
 * @brief       ledCtrlSingleSet
 * @param       colour - Colour
 * 				status - LED_ON / LED_OFF
 * 				flashInterMs - Flash interval (in ms)
 * @return      None
 * @details     Set single LED with flash interval
 *****************************************************************************/
void ledCtrlSingleSet(ledColour colour, Drv_LED_Status status, uint32_t flashInterMs)
{

#if CONFIG_ESPMODEL_S3MINI == 1 || CONFIG_ESPMODEL_S3MINI_C == 1 || CONFIG_ESPMODEL_MIFD == 1
#if	(CONFIG_MI_LED_PWM_EN) == 0
	/*
	 * Choose which LED is on based on the colour
	 * If no colour is selected, toggle all LED
	 * Only applicable when interfacing with Cougar/Mini Cougar
	 */
	switch (colour)
	{
		case OFF:
			ledGrpStructObj.ledSingleStructObj[0].ledPin = LED_PIN_R;
			ledGrpStructObj.ledSingleStructObj[0].status = LED_OFF;
			ledGrpStructObj.ledSingleStructObj[0].flashInterMs = 0;

			ledGrpStructObj.ledSingleStructObj[1].ledPin = LED_PIN_G;
			ledGrpStructObj.ledSingleStructObj[1].status = LED_OFF;
			ledGrpStructObj.ledSingleStructObj[1].flashInterMs = 0;

			ledGrpStructObj.ledSingleStructObj[2].ledPin = LED_PIN_B;
			ledGrpStructObj.ledSingleStructObj[2].status = LED_OFF;
			ledGrpStructObj.ledSingleStructObj[2].flashInterMs = 0;

		break;
		case RED_LED:
			ledGrpStructObj.ledSingleStructObj[0].ledPin = LED_PIN_R;
			ledGrpStructObj.ledSingleStructObj[0].status = status;
			ledGrpStructObj.ledSingleStructObj[0].flashInterMs = flashInterMs;

			ledGrpStructObj.ledSingleStructObj[1].ledPin = LED_PIN_G;
			ledGrpStructObj.ledSingleStructObj[1].status = LED_OFF;
			ledGrpStructObj.ledSingleStructObj[1].flashInterMs = flashInterMs;

			ledGrpStructObj.ledSingleStructObj[2].ledPin = LED_PIN_B;
			ledGrpStructObj.ledSingleStructObj[2].status = LED_OFF;
			ledGrpStructObj.ledSingleStructObj[2].flashInterMs = flashInterMs;

		break;
		case BLUE_LED:
			ledGrpStructObj.ledSingleStructObj[0].ledPin = LED_PIN_R;
			ledGrpStructObj.ledSingleStructObj[0].status = LED_OFF;
			ledGrpStructObj.ledSingleStructObj[0].flashInterMs = flashInterMs;

			ledGrpStructObj.ledSingleStructObj[1].ledPin = LED_PIN_G;
			ledGrpStructObj.ledSingleStructObj[1].status = LED_OFF;
			ledGrpStructObj.ledSingleStructObj[1].flashInterMs = flashInterMs;

			ledGrpStructObj.ledSingleStructObj[2].ledPin = LED_PIN_B;
			ledGrpStructObj.ledSingleStructObj[2].status = status;
			ledGrpStructObj.ledSingleStructObj[2].flashInterMs = flashInterMs;
		break;
		case GREEN_LED:
			ledGrpStructObj.ledSingleStructObj[0].ledPin = LED_PIN_R;
			ledGrpStructObj.ledSingleStructObj[0].status = LED_OFF;
			ledGrpStructObj.ledSingleStructObj[0].flashInterMs = flashInterMs;

			ledGrpStructObj.ledSingleStructObj[1].ledPin = LED_PIN_G;
			ledGrpStructObj.ledSingleStructObj[1].status = status;
			ledGrpStructObj.ledSingleStructObj[1].flashInterMs = flashInterMs;

			ledGrpStructObj.ledSingleStructObj[2].ledPin = LED_PIN_B;
			ledGrpStructObj.ledSingleStructObj[2].status = LED_OFF;
			ledGrpStructObj.ledSingleStructObj[2].flashInterMs = flashInterMs;
		break;
		case YELLOW_LED:
			ledGrpStructObj.ledSingleStructObj[0].ledPin = LED_PIN_R;
			ledGrpStructObj.ledSingleStructObj[0].status = status;
			ledGrpStructObj.ledSingleStructObj[0].flashInterMs = flashInterMs;

			ledGrpStructObj.ledSingleStructObj[1].ledPin = LED_PIN_G;
			ledGrpStructObj.ledSingleStructObj[1].status = status;
			ledGrpStructObj.ledSingleStructObj[1].flashInterMs = flashInterMs;

			ledGrpStructObj.ledSingleStructObj[2].ledPin = LED_PIN_B;
			ledGrpStructObj.ledSingleStructObj[2].status = LED_OFF;
			ledGrpStructObj.ledSingleStructObj[2].flashInterMs = flashInterMs;
		break;
		case AQUA_LED:
			ledGrpStructObj.ledSingleStructObj[0].ledPin = LED_PIN_R;
			ledGrpStructObj.ledSingleStructObj[0].status = LED_OFF;
			ledGrpStructObj.ledSingleStructObj[0].flashInterMs = flashInterMs;

			ledGrpStructObj.ledSingleStructObj[1].ledPin = LED_PIN_G;
			ledGrpStructObj.ledSingleStructObj[1].status = status;
			ledGrpStructObj.ledSingleStructObj[1].flashInterMs = flashInterMs;

			ledGrpStructObj.ledSingleStructObj[2].ledPin = LED_PIN_B;
			ledGrpStructObj.ledSingleStructObj[2].status = status;
			ledGrpStructObj.ledSingleStructObj[2].flashInterMs = flashInterMs;
		break;
		case PINK_LED:
			ledGrpStructObj.ledSingleStructObj[0].ledPin = LED_PIN_R;
			ledGrpStructObj.ledSingleStructObj[0].status = status;
			ledGrpStructObj.ledSingleStructObj[0].flashInterMs = flashInterMs;

			ledGrpStructObj.ledSingleStructObj[1].ledPin = LED_PIN_G;
			ledGrpStructObj.ledSingleStructObj[1].status = LED_OFF;
			ledGrpStructObj.ledSingleStructObj[1].flashInterMs = flashInterMs;

			ledGrpStructObj.ledSingleStructObj[2].ledPin = LED_PIN_B;
			ledGrpStructObj.ledSingleStructObj[2].status = status;
			ledGrpStructObj.ledSingleStructObj[2].flashInterMs = flashInterMs;
		break;
		default:
			ledGrpStructObj.ledSingleStructObj[0].ledPin = LED_PIN_R;
			ledGrpStructObj.ledSingleStructObj[0].status = status;
			ledGrpStructObj.ledSingleStructObj[0].flashInterMs = flashInterMs;

			ledGrpStructObj.ledSingleStructObj[1].ledPin = LED_PIN_G;
			ledGrpStructObj.ledSingleStructObj[1].status = status;
			ledGrpStructObj.ledSingleStructObj[1].flashInterMs = flashInterMs;

			ledGrpStructObj.ledSingleStructObj[2].ledPin = LED_PIN_B;
			ledGrpStructObj.ledSingleStructObj[2].status = status;
			ledGrpStructObj.ledSingleStructObj[2].flashInterMs = flashInterMs;
		break;
	}//End switch

	ledCtrlSingle(&ledGrpStructObj);
#endif
#endif
}//End ledCtrlSingleSet

/******************************************************************************
 * @brief       ledCtrlPwmSet
 * @param       r - Red component value (0 - 255)
 * 				g - Green component value (0 - 255)
 * 				b - Blue component value (0 - 255)
 * 				fadeDuration - Fade duration (in ms, must be greater than 100)
 * @return      None
 * @details     Set PWM based RGB LED
 *****************************************************************************/
void ledCtrlPwmSet(uint8_t r, uint8_t g, uint8_t b, uint16_t fadeDuration)
{
	R = (r*100)/255;
	G = (g*100)/255;
	B = (b*100)/255;
	mFadeDuration = fadeDuration;
}//End ledCtrlPwmSet
#endif
