#ifndef COMPONENTS_DRIVERS_INCLUDE_MCU_DEPENDENT_H_
#define COMPONENTS_DRIVERS_INCLUDE_MCU_DEPENDENT_H_
#include <esp_log.h>

#include "Drv_BT.h"
#include "Drv_CRC.h"
#include "DrvGPIO.h"
#include "DrvLCD.h"
#include "DrvLED.h"
#include "DrvNVS.h"
#include "DrvSPIHost.h"
#include "DrvTimer.h"
#include "DrvWLAN.h"
#include "DrvUSB.h"
#include "SenXorLib.h"

typedef enum OpMode
{
    WLAN_MODE,
    USB_MODE
} OpMode;

void ESP32_Peri_Init(void);

uint8_t MCU_getOpMode(void);
#endif /* COMPONENTS_DRIVERS_INCLUDE_MCU_DEPENDENT_H_ */
