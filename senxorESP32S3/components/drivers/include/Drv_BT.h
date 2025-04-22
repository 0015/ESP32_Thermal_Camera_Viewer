/*****************************************************************************
 * @file     Drv_BT.c
 * @version  1.0
 * @brief    Bluetooth driver
 * @date	 3 Jul 2023
 ******************************************************************************/

#ifndef COMPONENTS_DRIVERS_INCLUDE_DRV_BT_H_
#define COMPONENTS_DRIVERS_INCLUDE_DRV_BT_H_
#include <stdio.h>
#include <esp_err.h>
#include <esp_blufi.h>
#include <esp_blufi_api.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_bt_device.h>
#include <esp_heap_caps.h>
#include <esp_log.h>
#include <esp_system.h>

#include "Drv_BTSec.h"
#include "DrvWLAN.h"
#include "MCU_Dependent.h"
#include "msg.h"
#include "util.h"

#define BLUFI_DEG_LOG           0

#define BLUFI_BUFF_LEN			200
#define BLUFI_CTM_JSON_ROOT		"blufiCustomData"

//JSON
#define BLUFI_JSON_ROOT 		"blufi"
#define BLUFI_JSON_DEVNAME_KEY	"dev_name"
#define BLUFI_JSON_DEV_OPMODE   "op_mode"


#define BLUFI_FLAG_JSON_ROOT	"flags"
#define BLUFI_WIPE_JSON_KEY		"wipe"
#define BLUFI_RST_JSON_KEY      "restart"

//Bluetooth device's name
#define BLUFI_FILTER_ID			"BFI_"
#define BLUFI_DEV_NAME_NVS_KEY	"BFI_NAME"
#define BLUFI_OPMODE_NVS_KEY    "BFI_OP_MODE"


void Drv_BT_Init(void);

void Drv_BT_DeInit(void);

void Drv_BT_ReInit(void);

void Drv_BluFi_SendCtmDataJson(cJSON* mJson);

void Drv_BluFi_GetCtmDataJson(char* jsonStr, char* jsonOut);

#endif /* COMPONENTS_DRIVERS_INCLUDE_DRV_BT_H_ */
