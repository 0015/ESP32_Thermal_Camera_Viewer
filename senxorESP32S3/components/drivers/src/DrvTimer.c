/*****************************************************************************
 * @file     DrvTimer.c
 * @version  1.10
 * @brief    Initialising and configuring timer & PWM for generating sysclk
 * @date	 5 May 2022
 ******************************************************************************/
#include <DrvTimer.h>
/******************************************************************************
 * @brief       DrvPWM_init
 * @param       None
 * @return      None
 * @details     Initialise PWM
 *****************************************************************************/
void Drv_PWM_init(void)
{
#ifdef CONFIG_MI_ONBOARD_CLK
	/*
	 *  PWM Timer configuration
	 */
	ledc_timer_config_t ledc_timer = {0};					//PWM timer structure.
	ledc_timer.speed_mode       = LEDC_LOW_SPEED_MODE;		//Configure PWM mode.
	ledc_timer.timer_num        = LEDC_TIMER_1;				//Configure clock source.
	ledc_timer.duty_resolution  = LEDC_TIMER_3_BIT;			//Configure timer resolution
	ledc_timer.freq_hz          = PWM_FREQUENCY;			//Configure PWM frequency
	ledc_timer.clk_cfg          = LEDC_USE_XTAL_CLK;		//Configure PWM timer clock source
	ledc_timer_config(&ledc_timer);

	/*
	 * Channel configuration
	 */
	ledc_channel_config_t ledc_channel = {0};
	ledc_channel.speed_mode     = LEDC_LOW_SPEED_MODE;
	ledc_channel.channel        = LEDC_CHANNEL_0;
	ledc_channel.timer_sel      = LEDC_TIMER_1;
	ledc_channel.intr_type      = LEDC_INTR_DISABLE;
	ledc_channel.gpio_num       = PWM_OUTPUT_IO;
	ledc_channel.duty           = 4;
	ledc_channel.hpoint         = 0;
	ledc_channel_config(&ledc_channel);

    ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, ledc_channel.duty);		//Setting duty cycle
    ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);						//Apply duty cycle

    ESP_LOGI(PWMSYSCLK_TAG,PWMSYSCLK_INFO,ledc_channel.timer_sel,ledc_channel.channel,ledc_channel.gpio_num,PWM_FREQUENCY);
	if (PWM_FREQUENCY != 3*1000*1000)
	{
		ESP_LOGW(PWMSYSCLK_TAG,PWMSYSCLK_WARN_PWM_LIB,PWM_FREQUENCY);
	}
	else
	{
		ESP_LOGI(PWMSYSCLK_TAG,PWMSYSCLK_FREQ_OK);
	}
	ESP_LOGI(PWMSYSCLK_TAG,PWMSYSCLK_INTI_OK);
#else
	ESP_LOGE(PWMSYSCLK_TAG,PWMSYSCLK_ERR_NOTEN);
#endif
}
/******************************************************************************
 * @brief       DrvTimer_init
 * @param       None
 * @return      None
 * @details     Initialise Timer. On ESP32, general purpose timers are grouped with
 * 				watchdog timers as TimerGroup. Each TimerGroup contains 1
 * 				general purpose timer and 1 watchdog timer.
 * 				When the timer is overflow with the alarm and interrupt are configured,
 * 				an interrupt will be generated to indicate that
 * 				For general case, general purpose timer should be utilised
 * 				Use DrvTimer.h to adjust the parameters
 *****************************************************************************/
void Drv_Timer_init(void)
{
	timer_ll_enable_alarm(&TIMERG0,TIMER_NO, 0);									//Disable alarm before initialisation
	timer_ll_enable_counter(&TIMERG0,TIMER_NO, 0);								//Disable counter before initialisation
	timer_ll_set_clock_source(&TIMERG0,TIMER_NO,GPTIMER_CLK_SRC_APB);		//Set clock source
	timer_ll_set_clock_prescale(&TIMERG0,TIMER_NO,TIMER_PRESCARE);			//Set prescale
	timer_ll_set_count_direction(&TIMERG0,TIMER_NO,GPTIMER_COUNT_UP);		//Set counting direction
	timer_ll_set_alarm_value(&TIMERG0,TIMER_NO,1000000);				//Set Alarm value

	timer_ll_enable_alarm(&TIMERG0,TIMER_NO, 1);									//Enable alarm
	timer_ll_enable_intr(&TIMERG0, TIMER_LL_EVENT_ALARM(TIMER_NO), 1);				//Enable timer interrupt


}

/******************************************************************************
 * @brief       Drv_Timer_Start
 * @param       None
 * @return      None
 * @details     Start timer
 *****************************************************************************/
void Drv_Timer_Start(void)
{
	timer_ll_enable_alarm(&TIMERG0,TIMER_NO, 1);							//Enable alarm
	timer_ll_enable_counter(&TIMERG0,TIMER_NO, 1);							//Enable counter
	timer_ll_enable_intr(&TIMERG0, TIMER_LL_EVENT_ALARM(TIMER_NO), 1);		//Enable timer interrupt
}

/******************************************************************************
 * @brief       Drv_Timer_Pause
 * @param       None
 * @return      None
 * @details     Pause timer
 *****************************************************************************/
void Drv_Timer_Pause(void)
{
	timer_ll_enable_alarm(&TIMERG0,TIMER_NO, 0);							//Enable alarm
	timer_ll_enable_counter(&TIMERG0,TIMER_NO, 0);							//Disable counter
}

/******************************************************************************
 * @brief       Drv_Timer_Restart
 * @param       None
 * @return      None
 * @details     Restart timer
 *****************************************************************************/
void Drv_Timer_Restart(void)
{
	Drv_Timer_Pause();
	Drv_Timer_Start();
}

/******************************************************************************
 * @brief       Drv_Timer_Reset
 * @param       None
 * @return      None
 * @details     Reset timer
 *****************************************************************************/
void Drv_Timer_Reset(void)
{
	Drv_Timer_Pause();
	timer_ll_set_reload_value(&TIMERG0,TIMER_NO,0);
	timer_ll_trigger_soft_reload(&TIMERG0,TIMER_NO);
}

/******************************************************************************
 * @brief       Drv_Timer_TimerDelay
 * @param       time_ms - Delay (in ms)
 * @return      None
 * @details     Generate a delay using timer
 *****************************************************************************/
void Drv_Timer_TimerDelay(const int time_ms)
{
	//If parameter is 0 that means no delay
	if(time_ms == 0)
	{
		return;
	}

	int prescaledSpd = esp_clk_apb_freq();

	if(TIMER_PRESCARE != 0)
	{
		prescaledSpd = prescaledSpd /TIMER_PRESCARE;
	}

	const uint32_t alarm_val = (float)(time_ms*prescaledSpd/1000);
	Drv_Timer_Reset();
	timer_ll_set_alarm_value(&TIMERG0,TIMER_NO,alarm_val);					//Set Alarm value
	Drv_Timer_Start();
	while(!TIMERG0.int_raw_timers.t0_int_raw);
	Drv_Timer_Reset();
	timer_ll_clear_intr_status(&TIMERG0, TIMER_LL_EVENT_ALARM(TIMER_NO));
}

/******************************************************************************
 * @brief       Drv_Timer_Start_TimeOut_Timer_mSec
 * @param       TimeOut_umsec > required timeout in milliseconds
 * @return      None
 * @details     Setup Timeout timer, to check if timeout expired , 1410mS on TimeOut_umsec = 0
 *****************************************************************************/
void Drv_Timer_Start_TimeOut_Timer_mSec(const int TimeOut_umsec)
{
	//Not in use

}

/******************************************************************************
* @brief       Drv_Timer_TimeOut_Occured
* @param       None
* @return      return 1 when timeout has expired. otherwise will return 0
* @details     check if if timeout has expired
*****************************************************************************/
int Drv_Timer_TimeOut_Occured(void)
{
	return 0;
}
