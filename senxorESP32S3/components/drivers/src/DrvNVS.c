/*****************************************************************************
 * @file     DrvNVS.c
 * @version  1.2
 * @brief    Contains function for accessing NVS
 * @date	 22 Feb 2023
 ******************************************************************************/
#include <string.h>
#include "DrvNVS.h"
#include "DrvLED.h"
//private:
static nvs_handle_t nvs_handler;
/*
 * ***********************************************************************
 * @brief       NVS_Init
 * @param       None
 * @return      None
 * @details     Initialise NVS storage
 **************************************************************************/
void NVS_Init(void)
{

	esp_err_t ret = nvs_flash_init();			//Initialise NVS Flash
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		/*
		 * Erase pages when NVS is full or NVS has new version
		 */
		ESP_LOGE(NVSTAG,NVS_ERR_FULL);
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}//End if

	ESP_LOGI(NVSTAG,NVS_INIT_DONE " (%s)",esp_err_to_name(ret));
}

/*
 * ***********************************************************************
 * @brief       NVS_WriteInt
 * @param       key - Key to locate the value
 * 				value - Integer to be stored
 * @return      None
 * @details     Write string to NVS
 **************************************************************************/
void NVS_WriteInt(const char* key, const int32_t value)
{
	if(nvs_handler == 0)
	{
		ESP_LOGE(NVSTAG,NVS_ERR_HANDLE_NULL);
		return;
	}//End if
	esp_err_t err = nvs_set_i32(nvs_handler, key, value);

	if(err != ESP_OK)
	{
		ESP_LOGE(NVSTAG,NVS_ERR_WRITE,esp_err_to_name(err));
		return;
	}//End if

	err = nvs_commit(nvs_handler);

	if(err != ESP_OK)
	{
		ESP_LOGE(NVSTAG,NVS_ERR_WRITE,esp_err_to_name(err));
		return;
	}//End if
}

/*
 * ***********************************************************************
 * @brief       NVS_WriteStr
 * @param       key - Key to locate the value
 * 				Value - Data written to NVS
 * @return      None
 * @details     Write string to NVS
 **************************************************************************/
void NVS_WriteStr(const char* key, const char* value)
{
	if(nvs_handler == 0)
	{
		ESP_LOGE(NVSTAG,NVS_ERR_HANDLE_NULL);
		return;
	}//End if
	esp_err_t err = nvs_set_str(nvs_handler, key, value);

	if(err != ESP_OK)
	{
		ESP_LOGE(NVSTAG,NVS_ERR_WRITE,esp_err_to_name(err));
		return;
	}//End if

	err = nvs_commit(nvs_handler);

	if(err != ESP_OK)
	{
		ESP_LOGE(NVSTAG,NVS_ERR_WRITE,esp_err_to_name(err));
		return;
	}//End if
}
/*
 * ***********************************************************************
 * @brief       NVS_ReadInt
 * @param       key - Key to locate the value
 * 				value - Integer variable to hold the value
 * @return      Character pointer containing the value. NULL if error occurred.
 * @details     Read data by key
 **************************************************************************/
void NVS_ReadInt(const char* key, int32_t value)
{

	if(nvs_handler == 0)
	{
		return;
	}//End if


	esp_err_t err = nvs_get_i32(nvs_handler, key, &value);


	switch(err)
	{
		case ESP_OK:
			ESP_LOGI(NVSTAG,"%s", esp_err_to_name(err));
			break;
		case ESP_ERR_NVS_NOT_FOUND:
			ESP_LOGE(NVSTAG,NVS_ERR_PART_NOT_EXIST);
			break;
		default:
			ESP_LOGE(NVSTAG,NVS_ERR_RD, esp_err_to_name(err));
			break;
	}//End switch
}

/*
 * ***********************************************************************
 * @brief       NVS_ReadSize
 * @param       key - Key to locate the value
 * @return      Size of the data associated with the key
 * @details     Get the size of the data by a key string.
 **************************************************************************/
size_t NVS_ReadSize(const char* key)
{
	if(nvs_handler == 0)
	{
		return 0;
	}//End if

	size_t rdSize = 0;
	esp_err_t err = nvs_get_str(nvs_handler, key, NULL, &rdSize);
	return rdSize;
}

/*
 * ***********************************************************************
 * @brief       NVS_ReadStr
 * @param       key - Key to locate the value
 * @return      Character pointer containing the value. NULL if error occurred.
 * @details     Read data by key
 **************************************************************************/
void NVS_ReadStr(const char* key, char* buffer)
{

	if(nvs_handler == 0)
	{
		return;
	}//End if

	size_t rdSize = 0;

	esp_err_t err = nvs_get_str(nvs_handler, key, NULL, &rdSize);

	char* tempBuff = malloc(rdSize);

	if(tempBuff == 0 || buffer == 0)
	{
		return;
	}//End if

	err = nvs_get_str(nvs_handler, key, tempBuff, &rdSize);


	switch(err)
	{
		case ESP_OK:
			strncpy(buffer,tempBuff,rdSize);
			break;
		case ESP_ERR_NVS_NOT_FOUND:
			ESP_LOGE(NVSTAG,NVS_ERR_PART_NOT_EXIST);
			break;
		default:
			ESP_LOGE(NVSTAG,NVS_ERR_RD, esp_err_to_name(err));
			break;
	}//End switch
	free(tempBuff);
}

/*
 * ***********************************************************************
 * @brief       NVS_PartErase
 * @param       None
 * @return      None
 * @details     Erase a partition
 **************************************************************************/
void NVS_PartErase()
{
	if(nvs_handler == 0)
	{

		return;
	}//End if
	nvs_erase_all(nvs_handler);

}

/*
 * ***********************************************************************
 * @brief       NVS_PartDismount
 * @param       None
 * @return      None
 * @details     Dismount a partition. This will close the handler
 **************************************************************************/
void NVS_PartDismount()
{
	if(nvs_handler == 0)
	{
		return;
	}//End if
	nvs_close(nvs_handler);
}

/*
 * ***********************************************************************
 * @brief       NVS_PartDismount
 * @param       None
 * @return      None
 * @details     Mount a partition
 **************************************************************************/
void NVS_PartMount(const char* part_name)
{

	esp_err_t err = nvs_open(part_name, NVS_READWRITE, &nvs_handler);
    if (err != ESP_OK)
    {
    	ESP_LOGE(NVSTAG,NVS_ERR_HANDLE, esp_err_to_name(err));
        nvs_close(nvs_handler);
    }//End if
    ESP_LOGI(NVSTAG,NVS_PART_INFO,part_name);
}

/*
 * ***********************************************************************
 * @brief       getHandler
 * @param       None
 * @return      None
 * @details     Get NVS handler
 **************************************************************************/
nvs_handle_t getHandler()
{
	return nvs_handler;
}
