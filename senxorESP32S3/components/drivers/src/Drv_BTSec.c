/*****************************************************************************
 * @file     Drv_BTSec.c
 * @version  1.0
 * @brief    Contains functions for encryption / decryption
 * @date	 4 Jul 2023
 ******************************************************************************/
#include "Drv_BTSec.h"

//public:
extern void btc_blufi_report_error(esp_blufi_error_state_t state);

//private:
static blufi_security_t *blufi_sec;
static int Drv_fillRand( void *rng_state, unsigned char *output, size_t len );


/*
 * ***********************************************************************
 * @brief       Drv_blufi_sec_init
 * @param       None
 * @return      None
 * @details    	Initialise Blufi security
 **************************************************************************/
esp_err_t Drv_blufi_sec_init(void)
{
	blufi_sec = heap_caps_malloc(sizeof(blufi_security_t), MALLOC_CAP_SPIRAM);
//	blufi_sec = (blufi_security_t *)malloc(sizeof(blufi_security_t));
    if (blufi_sec == NULL)
    {
    	free(blufi_sec);
        return ESP_FAIL;
    }

    memset(blufi_sec, 0x0, sizeof(struct blufi_security));

    mbedtls_dhm_init(&blufi_sec->dhm);
    mbedtls_aes_init(&blufi_sec->aes);

    memset(blufi_sec->iv, 0x0, 16);
    return 0;
}

/*
 * ***********************************************************************
 * @brief       Drv_blufi_sec_deinit
 * @param       None
 * @return      None
 * @details    	De-Initialise Blufi security
 **************************************************************************/
void Drv_blufi_sec_deinit(void)
{
    if (blufi_sec == NULL)
    {
        return;
    }//End if

    if (blufi_sec->dh_param)
    {
        free(blufi_sec->dh_param);
        blufi_sec->dh_param = NULL;
    }//End if
    mbedtls_dhm_free(&blufi_sec->dhm);
    mbedtls_aes_free(&blufi_sec->aes);

    memset(blufi_sec, 0x0, sizeof(struct blufi_security));

    free(blufi_sec);
    blufi_sec =  NULL;
}

/*
 * ***********************************************************************
 * @brief       blufi_dh_negotiate_data_handler
 * @param       None
 * @return      None
 * @details    	Blufi data handler
 **************************************************************************/
void blufi_dh_negotiate_data_handler(uint8_t *data, int len, uint8_t **output_data, int *output_len, bool *need_free)
{
    int ret;
    uint8_t type = data[0];

    if (blufi_sec == NULL)
    {
    	ESP_LOGI("[BT]","BLUFI Security is not initialised");
        btc_blufi_report_error(ESP_BLUFI_INIT_SECURITY_ERROR);
        return;
    }//End if

    switch (type)
    {
		case SEC_TYPE_DH_PARAM_LEN:
			blufi_sec->dh_param_len = ((data[1]<<8)|data[2]);
			if (blufi_sec->dh_param)
			{
				free(blufi_sec->dh_param);
				blufi_sec->dh_param = NULL;
			}//End if

			blufi_sec->dh_param = (uint8_t *)malloc(blufi_sec->dh_param_len);
			if (blufi_sec->dh_param == NULL)
			{
				btc_blufi_report_error(ESP_BLUFI_DH_MALLOC_ERROR);
				return;
			}//End if
		break;

		case SEC_TYPE_DH_PARAM_DATA:
			if (blufi_sec->dh_param == NULL)
			{
				btc_blufi_report_error(ESP_BLUFI_DH_PARAM_ERROR);
				return;
			}//End if

			uint8_t *param = blufi_sec->dh_param;
			memcpy(blufi_sec->dh_param, &data[1], blufi_sec->dh_param_len);

			ret = mbedtls_dhm_read_params(&blufi_sec->dhm, &param, &param[blufi_sec->dh_param_len]);
			if (ret)
			{
				btc_blufi_report_error(ESP_BLUFI_READ_PARAM_ERROR);
				return;
			}//End if
			free(blufi_sec->dh_param);
			blufi_sec->dh_param = NULL;

			const int dhm_len = mbedtls_dhm_get_len(&blufi_sec->dhm);
			ret = mbedtls_dhm_make_public(&blufi_sec->dhm, dhm_len, blufi_sec->self_public_key, dhm_len, Drv_fillRand, NULL);
			if (ret)
			{
				btc_blufi_report_error(ESP_BLUFI_MAKE_PUBLIC_ERROR);
				return;
			}//End if

			ret = mbedtls_dhm_calc_secret( &blufi_sec->dhm,blufi_sec->share_key,SHARE_KEY_BIT_LEN,&blufi_sec->share_len,Drv_fillRand, NULL);
			if (ret)
			{
				btc_blufi_report_error(ESP_BLUFI_DH_PARAM_ERROR);
				return;
			}//End if

			ret = mbedtls_md5(blufi_sec->share_key, blufi_sec->share_len, blufi_sec->psk);
			if (ret)
			{
				btc_blufi_report_error(ESP_BLUFI_CALC_MD5_ERROR);
				return;
			}//End if

			mbedtls_aes_setkey_enc(&blufi_sec->aes, blufi_sec->psk, 128);

			/* alloc output data */
			*output_data = &blufi_sec->self_public_key[0];
			*output_len = dhm_len;
			*need_free = false;
		break;
		case SEC_TYPE_DH_P:
		break;
		case SEC_TYPE_DH_G:
		break;
		case SEC_TYPE_DH_PUBLIC:
		break;
    }//End switch
}

/*
 * ***********************************************************************
 * @brief       Drv_blufi_aes_encrypt
 * @param       crypt_data - Pointer to unencrypted data
 * 				crypt_len - Length of encrypted data
 * @return      None
 * @details     Encryption via hardware AES
 **************************************************************************/
int Drv_blufi_aes_encrypt(uint8_t iv8, uint8_t *crypt_data, int crypt_len)
{
    int ret;
    size_t iv_offset = 0;
    uint8_t iv0[16];
    memcpy(iv0, blufi_sec->iv, sizeof(blufi_sec->iv));
    iv0[0] = iv8;   /* set iv8 as the iv0[0] */

    //Using hardware AES to encrypt
    ret = mbedtls_aes_crypt_cfb128(&blufi_sec->aes, MBEDTLS_AES_ENCRYPT, crypt_len, &iv_offset, iv0, crypt_data, crypt_data);
    if (ret)
    {
        return -1;
    }

    return crypt_len;
}

/*
 * ***********************************************************************
 * @brief       Drv_blufi_aes_decrypt
 * @param       crypt_data - Pointer to encrypted data
 * 				crypt_len - Length of encrypted data
 * @return      None
 * @details     Decryption via hardware AES
 **************************************************************************/
int Drv_blufi_aes_decrypt(uint8_t iv8, uint8_t *crypt_data, int crypt_len)
{
    int ret;
    size_t iv_offset = 0;
    uint8_t iv0[16];

    memcpy(iv0, blufi_sec->iv, sizeof(blufi_sec->iv));
    iv0[0] = iv8;   /* set iv8 as the iv0[0] */

    //Using hardware AES to decrypt
    ret = mbedtls_aes_crypt_cfb128(&blufi_sec->aes, MBEDTLS_AES_DECRYPT, crypt_len, &iv_offset, iv0, crypt_data, crypt_data);
    if (ret)
    {
        return -1;
    }

    return crypt_len;
}

/*
 * ***********************************************************************
 * @brief       Drv_fillRand
 * @param       output - Output buffer
 * 				len - Data length
 * @return      0
 * @details     Fill buffer with random generated number
 **************************************************************************/
static int Drv_fillRand( void *rng_state, unsigned char *output, size_t len )
{
    esp_fill_random(output, len);
    return( 0 );
}

/*
 * ***********************************************************************
 * @brief       Drv_blufi_crc_checksum
 * @param       data - Pointer to data
 * 				len - Data length
 * @return      unsigned 16bits check sum
 * @details     Get CRC checksum
 **************************************************************************/
uint16_t Drv_getChecksum(uint8_t iv8, uint8_t *data, int len)
{
	if(data == 0)
	{
		return 0;
	}
    return esp_crc16_be(0, data, len);
}
