/*****************************************************************************
 * @file     DrvUSB.c
 * @version  1.00
 * @brief    USB CDC Driver
 * @date	 23 Apr 2024
 ******************************************************************************/
#include "DrvUSB.h"

/******************************************************************************
 * @brief       Drv_USB_Init
 * @param       None
 * @return      None
 * @details     Initialise USB
 *****************************************************************************/
void Drv_USB_Init(void)
{
    tinyusb_config_t tusb_cfg = {0};
    tusb_cfg.device_descriptor = NULL;
    tusb_cfg.string_descriptor = NULL;
    tusb_cfg.external_phy = false;
    tusb_cfg.configuration_descriptor = NULL;

    tinyusb_driver_install(&tusb_cfg);
}// Drv_USB_Init

/******************************************************************************
 * @brief       Drv_USB_Init
 * @param       pCfg - tiny USB configuration object
 * @return      None
 * @details     Initialise USB CDC
 *****************************************************************************/
void Drv_USB_CDC_Init(const tinyusb_config_cdcacm_t *pCfg)
{
    if(!pCfg)
    {
        return;
    }
    tusb_cdc_acm_init(pCfg);
}// Drv_USB_CDC_Init
