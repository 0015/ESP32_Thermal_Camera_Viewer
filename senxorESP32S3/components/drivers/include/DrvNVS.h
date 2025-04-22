/*****************************************************************************
 * @file     DrvNVS.h
 * @version  1.2
 * @brief    Contains function for accessing NVS
 * @date	 22 Feb 2023
 ******************************************************************************/
#ifndef COMPONENTS_DRIVERS_INCLUDE_DRVNVS_H_
#define COMPONENTS_DRIVERS_INCLUDE_DRVNVS_H_
#include <esp_log.h>
#include <nvs_flash.h>
#include <stdio.h>
#include "msg.h"


void NVS_Init(void);

void NVS_WriteInt(const char* key, const int32_t value);

void NVS_WriteStr(const char* key, const char* value);

void NVS_ReadInt(const char* key, int32_t value);

size_t NVS_ReadSize(const char* key);

void NVS_ReadStr(const char* key, char* buffer);

void NVS_PartDismount();

void NVS_PartErase();

void NVS_PartMount(const char* part_name);

nvs_handle_t getHandler();


#endif /* COMPONENTS_DRIVERS_INCLUDE_DRVNVS_H_ */
