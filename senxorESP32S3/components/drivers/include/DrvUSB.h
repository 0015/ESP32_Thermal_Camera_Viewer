#ifndef DRV_DRVUSB_H_
#define DRV_DRVUSB_H_

#include <stdint.h>
#include "esp_log.h"

#include "tinyusb.h"
#include "tusb_cdc_acm.h"
#include "common/tusb_types.h"

void Drv_USB_Init(void);

void Drv_USB_CDC_Init(const tinyusb_config_cdcacm_t *pCfg);

#endif
