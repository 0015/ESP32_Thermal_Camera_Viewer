/*****************************************************************************
 * @file     DrvLED.c
 * @version  1.00
 * @brief    LED control function
 * @date	 29 May 2023
 ******************************************************************************/
#include "DrvLED.h"


#ifdef CONFIG_MI_LED_EN


//private:
static led_strip_handle_t led_strip;

static void Drv_LED_Gpio_Init();
static void Drv_LED_PWM_Init();
static void Drv_LED_Strip_Init();
static void Drv_LED_Custom_Init();
/******************************************************************************
 * @brief       Drv_LED_Init
 * @param       _type - Type of LED
 * @return      None
 * @details     Initialise LED
 *****************************************************************************/
void Drv_LED_Init(Drv_LED_Type _type)
{
	ESP_LOGI(DRV_LEDTAG,DRV_LED_INIT);
	switch(_type)
	{
		case LED_GPIO:
		#ifdef CONFIG_MI_LED_PWM_EN
			Drv_LED_PWM_Init();
		#else
			Drv_LED_Gpio_Init();
		#endif
		break;
		case LED_STRIP:
			Drv_LED_Strip_Init();
		break;
		case LED_GPIO_STRIP:
			Drv_LED_Gpio_Init();
		#ifdef CONFIG_MI_LED_PWM_EN
			Drv_LED_PWM_Init();
		#endif
			Drv_LED_Strip_Init();
		break;
		case LED_OTHER:
			Drv_LED_Custom_Init();
		break;
		default:
			ESP_LOGE(DRV_LEDTAG,DRV_LED_ERR_INIT_ARG);
		break;

	}//End switch
}

/******************************************************************************
 * @brief       Drv_LED_Gpio_Single_En
 * @param       pin - GPIO number to LED
 * 				isEnabled - LED_ON / LED_OFF
 * @return      None
 * @details     Turn LED on / off
 *****************************************************************************/
void Drv_LED_Gpio_Single_En(Drv_LED_Status isEnabled)
{
#if CONFIG_ESPMODEL_S3EYE == 1 || CONFIG_ESPMODELS3_DevKitC1 == 1 || CONFIG_ESPMODEL_S3_OTHER == 1
	gpio_ll_set_level(&GPIO, LED_PIN, isEnabled);
#else
	gpio_ll_set_level(&GPIO, LED_PIN_R, isEnabled);
	gpio_ll_set_level(&GPIO, LED_PIN_G, isEnabled);
	gpio_ll_set_level(&GPIO, LED_PIN_B, isEnabled);
#endif
}

/******************************************************************************
 * @brief       Drv_LED_Gpio_En
 * @param       isEnabled - 1 to turn on 0 otherwise
 * @return      None
 * @details     Turn a particular LED on / off
 *****************************************************************************/
void Drv_LED_Gpio_En(uint32_t pin, Drv_LED_Status isEnabled)
{
	gpio_ll_set_level(&GPIO, pin, isEnabled);
}

/******************************************************************************
 * @brief       Drv_LED_PWM_setDuty
 * @param       channel - Channel connects to LED
 * 				duty - Duty cycle (in %)
 * @return      None
 * @details     Set brightness on a particular LED
 *****************************************************************************/
void Drv_LED_PWM_setDuty(ledc_channel_t channel, uint8_t duty)
{
#ifdef CONFIG_MI_LED_PWM_EN
	const float duty_val = (float)(pow(2,LEDC_TIMER_13_BIT)) * (float)(duty/100.0);
	ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, channel, duty_val));
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, channel));
#endif
}

/******************************************************************************
 * @brief       Drv_LED_PWM_setFade
 * @param       channel - Channel connects to LED
 * 				duty - Duty cycle (in %)
 * 				duration - Fade duration (in ms)
 * @return      None
 * @details     Enable fade on a particular LED
 *****************************************************************************/
void Drv_LED_PWM_setFade(ledc_channel_t channel, uint8_t duty, int duration)
{
#ifdef CONFIG_MI_LED_PWM_EN
	const float duty_val = (float)(pow(2,LEDC_TIMER_13_BIT) * (duty/100.0));
	ledc_set_fade_time_and_start(LEDC_LOW_SPEED_MODE,channel,duty_val,duration,LEDC_FADE_NO_WAIT);
#endif
}

/******************************************************************************
 * @brief       Drv_LED_Strip_Clr
 * @param       None
 * @return      None
 * @details     Turn LED Strip off
 *****************************************************************************/
void Drv_LED_Strip_Clr(void)
{
	if(led_strip == 0)
	{
		ESP_LOGE(DRV_LEDTAG,DRV_LED_ERR_RMT_HANDLE_NULL);
		return;
	}
#if MI_LED_TYPE_VAL != 0
	led_strip_clear(led_strip);
#else
	ESP_LOGE(DRV_LEDTAG,DRV_LED_ERR_RMT_NOT_EN);
#endif
}

/******************************************************************************
 * @brief       Drv_LED_Strip_setPixel
 * @param       idx - Index of LED. Count from 0.
 * 				rgb[] - Array contains the RGB value.
 * 				The array should be filled with
 * @return      None
 * @details     Set LED strip pixel
 *****************************************************************************/
void Drv_LED_Strip_setPixel(uint32_t idx, uint32_t R, uint32_t G, uint32_t B)
{
	if(led_strip == 0)
	{
		ESP_LOGE(DRV_LEDTAG,DRV_LED_ERR_RMT_HANDLE_NULL);
		return;
	}
#if CONFIG_MI_LED_TYPE_VAL != 0
	led_strip_set_pixel(led_strip, idx, R, G, B);
	led_strip_refresh(led_strip);									//Refresh LED strip to apply changes
#else
	ESP_LOGE(DRV_LEDTAG,DRV_LED_ERR_RMT_NOT_EN);
#endif
}
/******************************************************************************
 * @brief       Drv_LED_Single_Init
 * @param       None
 * @return      None
 * @details     Initialise LED
 *****************************************************************************/
static void Drv_LED_Gpio_Init()
{
	ESP_LOGI(DRV_LEDTAG,DRV_LED_MODE_GPIO);
	gpio_config_t gpio_conf = {0};								//GPIO configuration structure
	gpio_conf.intr_type = GPIO_INTR_DISABLE;					//Disable interrupt
	gpio_conf.mode = GPIO_MODE_OUTPUT;							//Configure pin mode to input
	gpio_conf.pin_bit_mask = LED_PIN_SEL;						//Configure bit mask
	gpio_conf.pull_down_en = 0;									//Configure GPIO pull down
	gpio_conf.pull_up_en = 0;									//Configure GPIO pull up
	gpio_config(&gpio_conf);
}

/******************************************************************************
 * @brief       Drv_LED_PWM_Init
 * @param       None
 * @return      None
 * @details     Initialise PWM LED
 *****************************************************************************/
static void Drv_LED_PWM_Init()
{
#if CONFIG_MI_LED_PWM_EN
	ESP_LOGI(DRV_LEDTAG,DRV_LED_MODE_PWM);
	ledc_timer_config_t ledc_timer = {
		.speed_mode       = LEDC_LOW_SPEED_MODE,
		.timer_num        = LEDC_TIMER_0,
		.duty_resolution  = LEDC_TIMER_13_BIT,
		.freq_hz          = CONFIG_MI_LED_PWM_FREQ,  // Set output frequency at 5 kHz
		.clk_cfg          = LEDC_AUTO_CLK
	};
	ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

	const float duty_val = (float)(pow(2,LEDC_TIMER_13_BIT)) * (float)(CONFIG_MI_LED_PWM_INTI_DCYE/100);

	ledc_channel_config_t ledc_channel = {
			.speed_mode     	 = LEDC_LOW_SPEED_MODE,
			.timer_sel      	 = LEDC_TIMER_0,
			.intr_type      	 = LEDC_INTR_DISABLE,
			.duty           	 = duty_val, 									// Set duty to 50%
	};

	#if CONFIG_ESPMODEL_S3_OTHER == 1 || CONFIG_ESPMODEL_S3EYE == 1 || CONFIG_ESPMODELS3_DevKitC1 == 1
		/*
		 * 	Implement customised PWM initialisation
		 */
		ledc_channel.channel = LEDC_CHANNEL_0;
		ledc_channel.gpio_num = LED_PIN;
		ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

		//Add other configuration here if required

	#else
		//LED pins cannot be modified for ESP32S3Mini.
		ledc_channel.channel = LEDC_CHANNEL_0;
		ledc_channel.gpio_num = LED_PIN_R;
		ledc_channel.flags.output_invert = 1;
		ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

		ledc_channel.channel = LEDC_CHANNEL_1;
		ledc_channel.gpio_num = LED_PIN_G;
		ledc_channel.flags.output_invert = 1;
		ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

		ledc_channel.channel = LEDC_CHANNEL_2;
		ledc_channel.gpio_num = LED_PIN_B;
		ledc_channel.flags.output_invert = 1;
		ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
	#endif
		ledc_fade_func_install(0);
#endif
}

/******************************************************************************
 * @brief       Drv_LED_Strip_Init
 * @param       None
 * @return      None
 * @details     Initialise LED strip
 *****************************************************************************/
static void Drv_LED_Strip_Init()
{
#ifndef CONFIG_SOC_RMT_SUPPORTED
	ESP_LOGE(DRV_LEDTAG,DRV_LED_ERR_RMT_NOT_SUPPORTED);
	return;
#endif

#if MI_LED_TYPE_VAL != 0
	ESP_LOGI(LEDTAG,LED_MODE_RMT);

	led_strip_config_t strip_config = {
		.strip_gpio_num = CONFIG_MI_LED_STRIP_PIN,
		.max_leds = CONFIG_MAX_LED, 					// at least one LED on board
		.led_model = LED_MODEL_WS2812,
		.led_pixel_format = CONFIG_MI_LED_FORMAT_VAL
	};

	//RMT configuration
	led_strip_rmt_config_t rmt_config = {
		.resolution_hz = 10 * 1000 * 1000 , 				//10MHz
		.flags.with_dma = 0
	};

	ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));	//Attach LED strip to RMT device
	Drv_LED_Strip_Clr();
#endif
}

/******************************************************************************
 * @brief       Drv_LED_Custom_Init
 * @param       None
 * @return      None
 * @details     Initialise LED (Customised implementation)
 *****************************************************************************/
static void Drv_LED_Custom_Init()
{
	//Custom initialisation routine
}

/******************************************************************************
 * @brief       Drv_LED_Init
 * @param       _type - Type of LED
 * @return      None
 * @details     If user attempt to initialise LED without enable.
 * 				This initialisation function will show error message
 * 				directly
 *****************************************************************************/
#else
void Drv_LED_Init(Drv_LED_Type _type)
{
	ESP_LOGE(DRV_LEDTAG,DRV_LED_ERR_RMT_NOT_EN);
}
#endif

//End DrvLED.c
