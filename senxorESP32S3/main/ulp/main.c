/*****************************************************************************
 * @file     main.c
 * @version  1.00
 * @brief    ULP RISC-V program entry point
 * @date	 23 Feb 2023
 ******************************************************************************/
#include <stdio.h>
#include <stdbool.h>
#include "ulp_riscv.h"
#include "ulp_riscv_adc_ulp_core.h"
#include "ulp_riscv_gpio.h"
#include "ulp_riscv_utils.h"

#ifdef CONFIG_MI_BAT_CHARGE_EN
#define PIN_ULP_PG				CONFIG_MI_BAT_PIN_PG				//Power down pins

volatile bool chargingPg = 0;
volatile int32_t batLv = 0;
#endif
int main (void)
{

#ifdef CONFIG_MI_BAT_CHARGE_EN
    while(1)
    {
    	chargingPg = (bool)ulp_riscv_gpio_get_level(PIN_ULP_PG);

    	batLv = ulp_riscv_adc_read_channel(ADC_UNIT_1, ADC_CHANNEL_9);
    }//End while
#endif
}
