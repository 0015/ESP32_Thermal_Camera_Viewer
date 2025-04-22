/**************************************************************************//**
 * @file     util.c
 * @version  V1.02
 * @brief    Contains utilities functions
 *
 ******************************************************************************/
#include <stdio.h>

#include "DrvSPIHost.h"
#include "msg.h"
#include "util.h"
#include "restServer.h"

/*
 * ***********************************************************************
 * @brief       printSenXorCaliData
 * @param       None
 * @return      None
 * @details     Display SenXor's calibration data via printf (Without frame header)
 *****************************************************************************/
void printSenXorCaliData(const uint16_t *caliData)
{
	printf("======================\n\r");
	for(int i = 0 ; i<CALIBDATA_FLASH_SIZE;++i)
	{
		if(i%16==0 && i!=0)
		{
			printf("\n");
			if(i == 128)
			{
				break;
			}
		}
		printf("%04X ", caliData[i] );
	}
	printf("======================\n");
	printf("END\n");
	printf("======================\n\r");
}
/*
 * ***********************************************************************
 * @brief       printSenXorData
 * @param       None
 * @return      None
 * @details     Display SenXor data via printf (Without frame header)
 *****************************************************************************/
void printSenXorData(const uint16_t *senData)
{
	printf("======================\n\r");
	for(int i = 80 ; i<5040;++i)
	{
		const uint16_t a = (senData[i] & 0xFF00) >> 8 | (senData[i] & 0x00FF) << 8;

		if(i%16==0 && i!=80)
		{
			printf("\n");
		}
		printf("%04X ", a );
	}
	printf("======================\n");
}
/*
 * ***********************************************************************
 * @brief       printSenXorLog
 * @param       None
 * @return      None
 * @details     Display frame header alongside the value of the first and the last pixel
 *****************************************************************************/
void printSenXorLog(const uint16_t *senData)
{
	if(senData == 0)
	{
		ESP_LOGE(SXRTAG,SXR_NO_DATA);
		return;
	}
	ESP_LOGI(SXRTAG,SXR_PIX_CNT,PixelCnt);
	ESP_LOGI(SXRTAG,SXR_DATA_HEADER, senData[0], senData[1], senData[2] ,senData[5], senData[6]);
	ESP_LOGI(SXRTAG,SXR_DATA_CAP, senData[80], senData[2480], senData[5039]);
}

/*
 * ***********************************************************************
 * @brief       displaySenxorInfo
 * @param       None
 * @return      None
 * @details     Get and display SenXor information
 *****************************************************************************/
void displaySenxorInfo(void)
{
	ESP_LOGI(SXRTAG,SXR_MODEL_RAW,SenXorModel);

	switch(SenXorModel)
	{
		case 0:
			ESP_LOGI(SXRTAG,SXR_MODEL_BOBCAT);
			break;
		case 3:
			ESP_LOGI(SXRTAG,SXR_MODEL_COUGAR);
			break;
	}//End switch

	ESP_LOGI(SXRTAG,SXR_REG_AVG,CONFIG_MI_SENXOR_AVG);


	ESP_LOGI(SXRTAG,SXR_REG_FITLER,CONFIG_MI_SENXOR_FILTER_VAL);

	switch(CONFIG_MI_SENXOR_FILTER_VAL)
	{
		case 0:
			ESP_LOGI(SXRTAG,SXR_FITLER_MODE_0);
			break;
		case 3:
			ESP_LOGI(SXRTAG,SXR_FITLER_MODE_3);
			break;
		case 4:
			ESP_LOGI(SXRTAG,SXR_FITLER_MODE_4);
			break;
		case 7:
			ESP_LOGI(SXRTAG,SXR_FITLER_MODE_7);
			break;
	}//End switch

}

/*
 * ***********************************************************************
 * @brief       getSysInfoJson
 * @param       jsonObj - JSON Object which the information will be added
 * @return     	None
 * @details     Add system information to an existing JSON
 *****************************************************************************/
void getSysInfoJson(cJSON* jsonObj)
{
	if(!jsonObj)
	{
		return;
	}
	cJSON *sys_info = cJSON_AddObjectToObject(jsonObj, JSON_SYS_INFO_ROOT);					//Added JSON object

	/*
	 * Add data to JSON
	 */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    cJSON_AddStringToObject(sys_info, JSON_SYS_INFO_FW_VER, MSG_APP_VER);
    cJSON_AddStringToObject(sys_info, JSON_SYS_INFO_SXRLIB_VER, MSG_SXRLIB_VER);
    cJSON_AddStringToObject(sys_info, JSON_SYS_INFO_IDF_VER, IDF_VER);
    cJSON_AddNumberToObject(sys_info, JSON_SYS_INFO_CPU_CORE, chip_info.cores);
    cJSON_AddNumberToObject(sys_info, JSON_SYS_INFO_CPU_SPD, CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ);
    cJSON_AddNumberToObject(sys_info, JSON_SYS_INFO_CPU_MODEL, chip_info.model);
	cJSON_AddNumberToObject(sys_info, BLUFI_JSON_DEV_OPMODE, MCU_getOpMode());

}

/*
 * ***********************************************************************
 * @brief       getCRC
 * @param       pData - Raw data to be calculated
 * 				pDataSize - Size of the data 
 * @return     	Calculated CRC
 * @details     Calculate CRC
 *****************************************************************************/
uint16_t getCRC(const uint8_t* pData, uint16_t pDataSize)
{
	uint16_t tCRCResult = 0;
	if(!pData)
	{
		return 0;
	}

	for(uint16_t i = 0 ; i < pDataSize ; ++i)
	{
		tCRCResult += pData[i];
	}
	return tCRCResult;
}// getCRC
