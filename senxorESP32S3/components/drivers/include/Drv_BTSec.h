/*****************************************************************************
 * @file     Drv_BTSec.h
 * @version  1.0
 * @brief    Contains functions for encryption / decryption
 * @date	 4 Jul 2023
 ******************************************************************************/
#ifndef COMPONENTS_DRIVERS_INCLUDE_DRV_BTSEC_H_
#define COMPONENTS_DRIVERS_INCLUDE_DRV_BTSEC_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_bt.h>
#include <esp_blufi_api.h>
#include <esp_crc.h>
#include <esp_random.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <mbedtls/aes.h>
#include <mbedtls/dhm.h>
#include <mbedtls/md5.h>
#include <nvs_flash.h>
#include "Drv_BT.h"

#define SEC_TYPE_DH_PARAM_LEN   0x00
#define SEC_TYPE_DH_PARAM_DATA  0x01
#define SEC_TYPE_DH_P           0x02
#define SEC_TYPE_DH_G           0x03
#define SEC_TYPE_DH_PUBLIC      0x04
#define DH_SELF_PUB_KEY_LEN     128
#define DH_SELF_PUB_KEY_BIT_LEN (DH_SELF_PUB_KEY_LEN * 8)
#define SHARE_KEY_LEN           128
#define SHARE_KEY_BIT_LEN       (SHARE_KEY_LEN * 8)
#define PSK_LEN                 16

typedef struct blufi_security
{
    uint8_t  self_public_key[DH_SELF_PUB_KEY_LEN];
    uint8_t  share_key[SHARE_KEY_LEN];
    size_t   share_len;
    uint8_t  psk[PSK_LEN];
    uint8_t  *dh_param;
    int      dh_param_len;
    uint8_t  iv[16];
    mbedtls_dhm_context dhm;
    mbedtls_aes_context aes;
}blufi_security_t;

esp_err_t Drv_blufi_sec_init(void);

void Drv_blufi_sec_deinit(void);

void blufi_dh_negotiate_data_handler(uint8_t *data, int len, uint8_t **output_data, int *output_len, bool *need_free);

int Drv_blufi_aes_encrypt(uint8_t iv8, uint8_t *crypt_data, int crypt_len);

int Drv_blufi_aes_decrypt(uint8_t iv8, uint8_t *crypt_data, int crypt_len);

uint16_t Drv_getChecksum(uint8_t iv8, uint8_t *data, int len);

#endif /* COMPONENTS_DRIVERS_INCLUDE_DRV_BTSEC_H_ */
