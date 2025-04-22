/*****************************************************************************
 * @file     DrvGPIO.c
 * @version  1.10
 * @brief    Initialising and configure GPIO
 * @date	 5 May 2022
 *
 ******************************************************************************/
#include "DrvGPIO.h"

/*
 * ***********************************************************************
 * @brief       BootButton_Handler
 * @param       None
 * @return      None
 * @details     ISR routine for Boot button
 *****************************************************************************/

static void IRAM_ATTR BootButton_Handler(void* arg)
{
	NVS_PartErase();
}

/*
 * ***********************************************************************
 * @brief       DrvGPIO_init
 * @param       None
 * @return      None
 * @details     Initialise GPIO
 *****************************************************************************/
void Drv_Gpio_Init(void)
{
	ESP_LOGI(GPIOTAG, GPIO_INIT_INFO);

	gpio_config_t gpio_conf = {0};									//GPIO configuration structure
	gpio_conf.intr_type = GPIO_INTR_DISABLE;						//Disable interrupt
	gpio_conf.mode = GPIO_MODE_OUTPUT;								//Configure pin mode to input
	gpio_conf.pin_bit_mask = GPIO_PIN_SEL;							//Configure bit mask
	gpio_conf.pull_down_en = 0;										//Configure GPIO pull down
	gpio_conf.pull_up_en = 0;										//Configure GPIO pull up
	gpio_config(&gpio_conf);										//Initialise GPIO configuration


	Drv_Gpio_RESET_N_PIN_Set(1);									//RESET pin set initially
	Drv_Gpio_SSDATAN_PIN_Set(1);									//SSDATAN set initially
	Drv_Gpio_SSFLASH_PIN_Set(1);									//SSFLASH set initially
	Drv_Gpio_SXR_PWR_DWN_PIN_Set(1);								//Enable SenXor LDO

	ESP_LOGI(GPIOTAG, GPIO_INIT);
}

/******************************************************************************
 * @brief       Drv_Gpio_Init_DATA_IRQ
 * @param       None
 * @return      None
 * @details     Initialise GPIO for DATA_AV input and enable its interrupt
 *****************************************************************************/
void Drv_Gpio_Init_DATA_IRQ(void * pvParameters)
{

	gpio_config_t gpio_conf = {0};										//GPIO configuration structure
	gpio_conf.mode = GPIO_MODE_INPUT;									//Configure pin mode to input
	gpio_conf.intr_type = GPIO_INTR_POSEDGE;							//Set interrupt type to rising edge
	gpio_conf.pin_bit_mask = GPIO_PIN_SEL_INPUT;						//Configure bit mask
	gpio_conf.pull_up_en = GPIO_PULLUP_DISABLE;
	gpio_config(&gpio_conf);											//Initialise GPIO configuration
	gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3);

	gpio_isr_handler_add(PIN_DATA_AV, Data_AV_FIFO_Int_Handler, (void*)PIN_DATA_AV);

	vTaskDelete(NULL);
}

/*************************************************************************
 * @brief       Drv_Gpio_CAPTURE_PIN_Set
 * @param       OnOff: 1 or 0
 * @return      None
 * @details     Set CAPTURE PIN output level. Has no effect when interfacing with Cougar
 *****************************************************************************/
void Drv_Gpio_CAPTURE_PIN_Set(const uint8_t OnOff)
{
#if CONFIG_MI_SENXOR_MODEL == 0
	gpio_ll_set_level(&GPIO, PIN_CAPTURE, OnOff);
#endif
}

/******************************************************************************
 * @brief       Drv_Gpio_Disable_Capture_interupt
 * @param       None
 * @return      None
 * @details     Disable the Capture interupt (DATA_AV pin)
 *****************************************************************************/
void Drv_Gpio_Disable_Capture_interupt(void)
{
	//Not in use
}
/******************************************************************************
 * @brief       Drv_Gpio_Enable_Capture_interupt
 * @param       None
 * @return      None
 * @details     Enable the Capture interupt (DATA_AV pin)
 *****************************************************************************/
void Drv_Gpio_Enable_Capture_interupt(void)
{

	//Not in use
}
/******************************************************************************
 * @brief       Drv_Gpio_Enable_MCU_PIN
 * @param       OnOff -> 1 = pins active , 0 pin tristate or low
				OnOff 	-> Data to write
 * @return      None
 * @details     Set all pins to either input or output low
				mainly used for SenXor testing.
 *****************************************************************************/
void Drv_Gpio_Enable_MCU_PIN(const int OnOff)
{
	//Holder. Not in use.
}
/******************************************************************************
 * @brief       Drv_Gpio_HOST_DATA_READY_Set
 * @param       OnOff: 1 or 0
 * @return      None
 * @details     Set HOST_DATA_READY PIN output level
 *****************************************************************************/
void Drv_Gpio_HOST_DATA_READY_Set(const uint8_t OnOff)
{
	//Holder. Not in use.
}
/******************************************************************************
 * @brief       Drv_Gpio_HOST_DATA_READY_Set
 * @param       OnOff: 1 or 0
 * @return      None
 * @details     Set SSDATAN PIN output level
 *****************************************************************************/
void Drv_Gpio_SSDATAN_PIN_Set(const uint8_t OnOff)
{
	gpio_ll_set_level(&GPIO, PIN_SSDATAN, OnOff);
}

/******************************************************************************
 * @brief       DrvGPIO_PIN_SSFLASHN_Set
 * @param       OnOff: 1 or 0
 * @return      None
 * @details     Set SSFLASHN PIN output level
 *****************************************************************************/
void Drv_Gpio_SSFLASH_PIN_Set(const uint8_t OnOff)
{
	gpio_ll_set_level(&GPIO, PIN_SSFLASHN, OnOff);
}
/******************************************************************************
 * @brief       Drv_Gpio_SSREGN_PIN_Set
 * @param       OnOff: 1 or 0
 * @return      None
 * @details     Set SSREGN PIN output level.
 *****************************************************************************/
void Drv_Gpio_SSREGN_PIN_Set(const uint8_t OnOff)
{
#if CONFIG_MI_SENXOR_MODEL == 0
	gpio_ll_set_level(&GPIO, PIN_SSREGN, OnOff);
#else
	gpio_ll_set_level(&GPIO, PIN_SSDATAN, OnOff);
#endif
}
/******************************************************************************
 * @brief       Drv_Gpio_SXR_PWR_DWN_PIN_Set
 * @param       1 - Enable output. 0 otherwise
 * @return      None
 * @details     Set SenXor LTO Enable
 *****************************************************************************/
void Drv_Gpio_SXR_PWR_DWN_PIN_Set(const uint8_t OnOff)
{
#if CONFIG_ESPMODEL_S3MINI_C || CONFIG_ESPMODEL_MIFD
	gpio_ll_set_level(&GPIO, PIN_SXR_PWR_DWN, OnOff);
#endif
}
/******************************************************************************
 * @brief       Drv_Gpio_RESET_N_PIN_Set
 * @param       OnOff: 1 or 0
 * @return      None
 * @details     Set SYSRST PIN output level
 *****************************************************************************/
void Drv_Gpio_RESET_N_PIN_Set(const uint8_t OnOff)
{
	gpio_ll_set_level(&GPIO, PIN_SYSRST, OnOff);
}
/******************************************************************************
 * @brief       Drv_Gpio_WRPROT_PIN_Set
 * @param       OnOff: 1 or 0
 * @return      None
 * @details     Set WRPROT PIN output level
 *****************************************************************************/
void Drv_Gpio_WRPROT_PIN_Set(const uint8_t OnOff)
{
	//Not in use
}


