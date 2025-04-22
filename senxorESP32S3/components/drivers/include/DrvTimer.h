#ifndef DRV_DRVPWM_H_
#define DRV_DRVPWM_H_

#include <freertos/FreeRTOS.h>
#include <esp_log.h>
#include <esp_private/esp_clk.h>
#include <driver/ledc.h>
#include <hal/timer_ll.h>
#include "driver/gptimer.h"
#include "msg.h"

/******************************************************************************
 * @brief       PWM Output configuration
 * 				For ESP32, because the default PWM frequency (LEDC_FREQUENCY)
 * 				is divided from  APB Clock (80MHz). Please ensure the Clock speed
 * 				is divisible by 80. Otherwise there will be jittering in PWM signal
 *****************************************************************************/
#define PWM_DUTY_RES		LEDC_TIMER_3_BIT					//PWM resolution
#define PWM_FREQUENCY		4 * 1000 * 1000 					//PWM Frequency. (Hz)

#if (CONFIG_ESPMODEL_S3EYE) || (CONFIG_ESPMODELS3_DevKitC1)
#define PWM_OUTPUT_IO		40 									//Configure PWM output pin.
#elif (CONFIG_ESPMODEL_S3MINI_C) || (CONFIG_ESPMODEL_MIFD)
#define PWM_OUTPUT_IO		18									//Configure PWM output pin.
#elif (CONFIGESPMODEL_S3_OTHER)
#define PWM_OUTPUT_IO		CONFIG_MI_SENXOR_PIN_SYSCLK
#endif
/******************************************************************************
 * @brief       Timer configuration
 * 				Configure timer here. The timer will generate an interrupt once
 * 				its value reaches TIMER_SCALE
 *****************************************************************************/
#define TIMER_PRESCARE				80
#define TIMER_NO					0

/******************************************************************************
 * @brief       Functions prototyping
 *****************************************************************************/
void Drv_PWM_init(void);

void Drv_Timer_init(void);

void Drv_Timer_Start(void);

void Drv_Timer_Pause(void);

void Drv_Timer_Restart(void);

void Drv_Timer_Reset(void);

void Drv_Timer_TimerDelay(const int time_ms);

void Drv_Timer_Start_TimeOut_Timer_mSec(const int TimeOut_umsec);

int Drv_Timer_TimeOut_Occured(void);

#endif /* DRV_DRVPWM_H_ */
