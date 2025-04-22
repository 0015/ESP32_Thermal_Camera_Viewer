/*****************************************************************************
 * @file     MCU_Dependent.c
 * @version  1.13
 * @brief    Initialising drivers
 * @date	 5 May 2022
 ******************************************************************************/
#include "MCU_Dependent.h"

// private:
static uint8_t mOpMode = USB_MODE;
/******************************************************************************
 * @brief       ESP32_Peri_Init
 * @param       None
 * @return      None
 * @details     Initialise ESP32 peripherals
 *****************************************************************************/
void ESP32_Peri_Init(void)
{
	char tOpMode[2] = {0,0};
	NVS_Init();										//Initialise NVS
	NVS_PartMount("meridian");			//Mount Partition

	// Read operation mode from NVS
	NVS_ReadStr(BLUFI_OPMODE_NVS_KEY,tOpMode);
	if(tOpMode[0] == '0' || tOpMode[0] == 0 )
	{
		mOpMode = WLAN_MODE;
	}
	else
	{
		mOpMode = USB_MODE;
		Drv_USB_Init();
	}
	ESP_LOGI(MCUTAG,MCU_INIT_INFO);

	Drv_Gpio_Init();								//Initialise GPIO

#if CONFIG_MI_ONBOARD_CLK
	Drv_PWM_init();									//Initialise clock to SenXor
#endif


	xTaskCreatePinnedToCore(Drv_Gpio_Init_DATA_IRQ, "Drv_Gpio_Init_DATA_IRQ", 3048, NULL, 20, NULL, 1);

#ifdef CONFIG_MI_LCD_EN
	LCDInit();										//Initialise LCD
#endif

#if CONFIG_MI_LED_EN
	Drv_LED_Init(CONFIG_MI_LED_TYPE_VAL);			//Initialise LED
#endif

#if CONFIG_MI_NVS_CLR
	NVS_PartErase();
#endif

	Drv_SPI_Init();															//Initialise MCU SPI Bus
	Drv_SPI_SENXOR_Init(DEFAULT_SPI_CLK_SPD,0);		//Initialise SPI Interface

	ESP_LOGI(MCUTAG,MCU_INIT_DONE);
}


/******************************************************************************
 * @brief       MCU_getOpMode
 * @param       None
 * @return      mOpMode - Either EVK_WLAN_MODE or EVK_USB_MODE
 * @details     Get the operation mode of the MCU
 *****************************************************************************/
uint8_t MCU_getOpMode(void)
{
	return mOpMode;
}