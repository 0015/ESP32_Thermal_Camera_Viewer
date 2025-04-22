/*****************************************************************************
 * @file     Drv_BT.c
 * @version  1.0
 * @brief    Bluetooth driver
 * @date	 3 Jul 2023
 ******************************************************************************/
#include "Drv_BT.h"
#include "MCU_Dependent.h"
#include "cJSON.h"
#include "esp_wifi.h"
#include "sdkconfig.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

//public:

//private:
EXT_RAM_BSS_ATTR static char mBlufiBuff[BLUFI_BUFF_LEN];
EXT_RAM_BSS_ATTR static wifi_config_t mWifiCfg;
EXT_RAM_BSS_ATTR static wlanCfg_t mWlanCfgObj;
EXT_RAM_BSS_ATTR static esp_blufi_extra_info_t mBlufExtraInfo;							//Blufi info object

static void Drv_Blufi_event_handler(esp_blufi_cb_event_t event, esp_blufi_cb_param_t *param);
static void Drv_Blufi_Init(void);
static void Drv_BluFi_DeInit(void);
static void Drv_BluFi_Advertise(char* devName);
static void Drv_BluFi_fetchDevJSON(char* mJson);
static void Drv_BluFi_fetchWipeJSON(char* mJson);
static void Drv_BluFi_loadPref();

//Callback structure
static esp_blufi_callbacks_t blufiCallbackObj =
{
	.event_cb = Drv_Blufi_event_handler,						//Blufi callback
	.negotiate_data_handler = blufi_dh_negotiate_data_handler,	//
	.encrypt_func = Drv_blufi_aes_encrypt,
	.decrypt_func = Drv_blufi_aes_decrypt,
	.checksum_func = Drv_getChecksum
};
/*
 * ***********************************************************************
 * @brief       Drv_Blufi_event_handler
 * @param       event - Blufi event
 * 				param - Parameter passed by Blufi callback function
 * @return      None
 * @details     Event handler for Blufi
 **************************************************************************/
static void Drv_Blufi_event_handler(esp_blufi_cb_event_t event, esp_blufi_cb_param_t *param)
{
	switch (event)
	{
		//On Blufi initialised
		case ESP_BLUFI_EVENT_INIT_FINISH:
#if BLUFI_DEG_LOG		
			ESP_LOGI(BTTAG,BLUFI_INFO_INIT_DONE);
#endif
#if CONFIG_MI_BFI_DEV_NAME_SCFG
			Drv_BluFi_Advertise(CONFIG_MI_BFI_DEV_NAME);
#else
			Drv_BluFi_loadPref();
			Drv_BluFi_Advertise(mBlufiBuff);
#endif
		break;

		//On Blufi deinitialised
#if BLUFI_DEG_LOG
		case ESP_BLUFI_EVENT_DEINIT_FINISH:
			ESP_LOGI(BTTAG,BLUFI_INFO_DEINIT_DONE);	//Show Blufi is deinitialised
		break;
#endif
		//On Blufi connected
		case ESP_BLUFI_EVENT_BLE_CONNECT:
#if BLUFI_DEG_LOG
			ESP_LOGI(BTTAG,BLUFI_INFO_EVENT_CONNECT);
#endif
			esp_blufi_adv_stop();
			Drv_blufi_sec_init();
		break;
		//On Blufi disconnected
		case ESP_BLUFI_EVENT_BLE_DISCONNECT:
#if BLUFI_DEG_LOG
			ESP_LOGI(BTTAG,BLUFI_INFO_EVENT_DISCONNECT);
#endif
#if CONFIG_MI_BFI_DEV_NAME_SCFG
			Drv_BluFi_Advertise(CONFIG_MI_BFI_DEV_NAME);
#else
			Drv_BluFi_loadPref();
			Drv_BluFi_Advertise(mBlufiBuff);
#endif
			Drv_blufi_sec_deinit();

		break;


		//On Blufi issue connect command
		case ESP_BLUFI_EVENT_REQ_CONNECT_TO_AP:
#if BLUFI_DEG_LOG		
			ESP_LOGI(BTTAG,BLUFI_INFO_EVENT_REQ_TO_AP);
#endif			
			Drv_WLAN_Stat_Reconnect();
		break;

		//On Blufi issue disconnect command
		case ESP_BLUFI_EVENT_REQ_DISCONNECT_FROM_AP:
#if BLUFI_DEG_LOG			
			ESP_LOGI(BTTAG,BLUFI_INFO_EVENT_REQ_DIS_AP);
#endif				
			Drv_WLAN_Stat_Disconnect();
		break;

		//On Blufi error
		case ESP_BLUFI_EVENT_REPORT_ERROR:
#if BLUFI_DEG_LOG			
			ESP_LOGI(BTTAG,BLUFI_ERR_EVENT, param->report_error.state);
#endif			
			esp_blufi_send_error_info(param->report_error.state);
		break;

		//On Blufi issue get WiFi status
		case ESP_BLUFI_EVENT_GET_WIFI_STATUS:
#if BLUFI_DEG_LOG			
			ESP_LOGI(BTTAG,BLUFI_INFO_EVENT_GET_APSTA_STATUS);
#endif
			memset(&mBlufExtraInfo, 0, sizeof(esp_blufi_extra_info_t));
			Drv_WLAN_getCfgNvs(&mWlanCfgObj);

			// Retrieve Station SSID
			mBlufExtraInfo.sta_ssid = (uint8_t*)mWlanCfgObj.STA_SSID;
			mBlufExtraInfo.sta_ssid_len = mWlanCfgObj.mSSIDLen;
			mBlufExtraInfo.sta_passwd = (uint8_t*) mWlanCfgObj.STA_PWD;
			mBlufExtraInfo.sta_passwd_len = mWlanCfgObj.mPwdLen;

			// Retrieve AP SSID
			mBlufExtraInfo.softap_ssid = (uint8_t*) mWlanCfgObj.AP_SSID;
			mBlufExtraInfo.softap_ssid_len = mWlanCfgObj.mAPSSIDLen;

			mBlufExtraInfo.softap_passwd = (uint8_t*) mWlanCfgObj.AP_PWD;
			mBlufExtraInfo.softap_passwd_len = mWlanCfgObj.mAPPwdLen;

			//Don't care the onnection status
			esp_blufi_send_wifi_conn_report(WIFI_MODE_APSTA,ESP_BLUFI_STA_CONN_SUCCESS,1,&mBlufExtraInfo);
		
			cJSON *root = cJSON_CreateObject();
		    Drv_WLAN_getRptToJson(root);									//Add network information to JSON
			getSysInfoJson(root);											//Add system information
		    Drv_BluFi_SendCtmDataJson(root);									//Wrap constructed JSON and send
			cJSON_Delete(root);												//Free JSON object after use

		break;

		//On Bluetooth client is disconnected
		case ESP_BLUFI_EVENT_RECV_SLAVE_DISCONNECT_BLE:
#if BLUFI_DEG_LOG
			ESP_LOGI(BTTAG,BLUFI_INFO_EVENT_GATT_CLOSE);
#endif
			esp_blufi_disconnect();
		break;

		/*
		 *	WiFi Station configuration
		 */
		case ESP_BLUFI_EVENT_RECV_STA_SSID:
#if BLUFI_DEG_LOG
			ESP_LOGI(BTTAG,BLUFI_INFO_EVENT_GET_STA_SSID);
			ESP_LOGI(BTTAG,"%s",(char *)param->sta_ssid.ssid);
#endif
			memset(mWifiCfg.sta.ssid, 0, sizeof(mWifiCfg.sta.ssid));
			if(param->sta_ssid.ssid_len > 0)
			{
				strncpy((char *)mWifiCfg.sta.ssid, (char *)param->sta_ssid.ssid, param->sta_ssid.ssid_len);
				// mWifiCfg.sta.ssid[param->sta_ssid.ssid_len] = '\0';
			}
			else
			{
				ESP_LOGW(BTTAG,"SSID field in parameter is empty");
				strncpy((char *)mWifiCfg.ap.ssid,CONFIG_ESP_WIFI_STA_SSID, 32);
				// mWifiCfg.sta.ssid[31] = '\0';
			}

			esp_wifi_set_config(WIFI_IF_STA, &mWifiCfg);

			NVS_WriteStr("ssid",(char *)mWifiCfg.sta.ssid);															//Store SSID in NVS
		break;

		case ESP_BLUFI_EVENT_RECV_STA_PASSWD:
#if BLUFI_DEG_LOG
			ESP_LOGI(BTTAG,BLUFI_INFO_EVENT_GET_STA_PWD);
#endif
			memset(mWifiCfg.sta.password, 0, sizeof(mWifiCfg.sta.password));
			if(param->sta_passwd.passwd_len > 0)
			{
				strncpy((char *)mWifiCfg.sta.password, (char *)param->sta_passwd.passwd, param->sta_passwd.passwd_len);
			}
			else 
			{
				ESP_LOGW(BTTAG,"Password field in parameter is empty");
				strncpy((char *)mWifiCfg.sta.password, CONFIG_ESP_WIFI_STA_PASSWORD, sizeof(mWifiCfg.sta.password));
			}
			esp_wifi_set_config(WIFI_IF_STA, &mWifiCfg);
			
			NVS_WriteStr("pwd", (char *)mWifiCfg.sta.password);
		break;

		/*
		 *	WiFi Hotspot configuration
		 *	Always enable regardless of the operation modes
		 */
		case ESP_BLUFI_EVENT_RECV_SOFTAP_SSID:
#if BLUFI_DEG_LOG
			ESP_LOGI(BTTAG,BLUFI_INFO_EVENT_GET_AP_SSID);
#endif
			memset(mWifiCfg.ap.ssid, 0, sizeof(mWifiCfg.ap.ssid));
			if(param->softap_ssid.ssid_len > 0)
			{
				ESP_LOGI(BTTAG,"ESP_BLUFI_EVENT_RECV_SOFTAP_SSID = %s",(char *)param->softap_ssid.ssid);
				strncpy((char *)mWifiCfg.ap.ssid, (char *)param->softap_ssid.ssid, param->softap_ssid.ssid_len);
				mWifiCfg.ap.ssid_len = param->softap_ssid.ssid_len;
			}
			else 
			{
				ESP_LOGW(BTTAG,"AP SSID field in parameter is empty. Using default SSID from sdkconfig.");
				strncpy((char *)mWifiCfg.ap.ssid,CONFIG_ESP_WIFI_AP_SSID, 32);
				mWifiCfg.ap.ssid_len = strlen(CONFIG_ESP_WIFI_AP_SSID);
			}
			
			esp_wifi_set_config(WIFI_IF_AP, &mWifiCfg);
			NVS_WriteStr("ssid_ap",(char *)mWifiCfg.ap.ssid);
		break;
		case ESP_BLUFI_EVENT_RECV_SOFTAP_PASSWD:
#if BLUFI_DEG_LOG
			ESP_LOGI(BTTAG,BLUFI_INFO_EVENT_GET_AP_PWD);
#endif
			memset(mWifiCfg.ap.password, 0, sizeof(mWifiCfg.ap.password));
			if(param->softap_passwd.passwd_len > 0)
			{
				strncpy((char *)mWifiCfg.ap.password, (char *)param->softap_passwd.passwd, param->softap_passwd.passwd_len);
				mWifiCfg.ap.ssid_len = param->softap_passwd.passwd_len;
			}
			else
			{
				ESP_LOGW(BTTAG,"AP Password field in parameter is empty, using default password");
				strncpy((char *)mWifiCfg.ap.password,CONFIG_ESP_WIFI_AP_PASSWORD, 64);	
				mWifiCfg.ap.ssid_len = strlen(CONFIG_ESP_WIFI_AP_PASSWORD);
			}// End if-else
			mWifiCfg.ap.authmode = WIFI_AUTH_WPA2_PSK;
			mWifiCfg.ap.max_connection = CONFIG_ESP_MAX_STA_CONN;
			mWifiCfg.ap.channel =CONFIG_ESP_WIFI_AP_CHANNEL;

			esp_wifi_set_config(WIFI_IF_AP, &mWifiCfg);
			NVS_WriteStr("pwd_ap",(char *) mWifiCfg.ap.password);
		break;

		//on custom data received
		case ESP_BLUFI_EVENT_RECV_CUSTOM_DATA:
#if BLUFI_DEG_LOG
			ESP_LOGI(BTTAG,BLUFI_INFO_EVENT_GET_CUSTDATA);
			ESP_LOGI(BTTAG,"Rec Custom Data %" PRIu32 "\n", param->custom_data.data_len);
			esp_log_buffer_char("Custom Data", param->custom_data.data, param->custom_data.data_len);
#endif
			mBlufiBuff[0] = 0;
			Drv_BluFi_GetCtmDataJson((char*)param->custom_data.data,mBlufiBuff);
			Drv_BluFi_fetchDevJSON(mBlufiBuff);
			Drv_BluFi_fetchWipeJSON(mBlufiBuff);
		break;

		default:
		break;
	}//End switch
}

/*
 * ***********************************************************************
 * @brief       Drv_BT_Init
 * @param       None
 * @return      None
 * @details     Initialise Bluetooth
 **************************************************************************/
void Drv_BT_Init()
{
	ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

	//Init BT
	esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

	esp_err_t ret = esp_bt_controller_init(&bt_cfg);

	if (ret)
	{
		ESP_LOGE(BTTAG,BT_ERR_CTRL_INIT_FAIL, esp_err_to_name(ret));
	}

	//Using bluetooth low energy
	ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
	if (ret)
	{
		ESP_LOGE(BTTAG,BT_ERR_CTRL_INIT_EN, esp_err_to_name(ret));
		return;
	}
	ESP_LOGI(BTTAG,BT_INFO_CTRL_INIT_DONE);
	Drv_Blufi_Init();

}

/*
 * ***********************************************************************
 * @brief       Drv_BT_DeInit
 * @param       None
 * @return      None
 * @details     De-Initialise Bluetooth
 **************************************************************************/
void Drv_BT_DeInit()
{

	Drv_BluFi_DeInit();

	ESP_ERROR_CHECK(esp_bt_controller_disable());
	int ret = esp_bt_controller_deinit();
	if (ret)
	{
		ESP_LOGE(BTTAG,BLUFI_ERR_DEINIT_FAIL, esp_err_to_name(ret));
		return;
	}
	ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
}

/*
 * ***********************************************************************
 * @brief       Drv_BT_ReInit
 * @param       None
 * @return      None
 * @details     Reload Bluetooth device completely
 **************************************************************************/
void Drv_BT_ReInit()
{
	Drv_BT_DeInit();
	Drv_BT_Init();
}
/*
 * ***********************************************************************
 * @brief       Drv_Blufi_Init
 * @param       None
 * @return      None
 * @details     Initialise Blufi
 **************************************************************************/
static void Drv_Blufi_Init()
{
	ESP_LOGI(BTTAG,BLUFI_INFO_INIT);
	esp_bluedroid_config_t espBluedroidCfgObj = BT_BLUEDROID_INIT_CONFIG_DEFAULT();
	esp_err_t ret = esp_bluedroid_init_with_cfg(&espBluedroidCfgObj);										//Initialise Bluedroid
	if (ret)
	{
		ESP_LOGE(BTTAG,BLUFI_ERR_INIT_FAIL, esp_err_to_name(ret));
		return;
	}//End if

	ret = esp_bluedroid_enable();										//Enable Bluedroid
	if (ret)
	{
		ESP_LOGE(BTTAG,BLUFI_ERR_INIT_EN_FAIL, esp_err_to_name(ret));
		return;
	}//End if

	//Register callbacks
	ret = esp_blufi_register_callbacks(&blufiCallbackObj);
	if(ret)
	{
		ESP_LOGE(BTTAG,BLUFI_ERR_CALLBACK_REG_FAIL, ret);
		return;
	}//End if


	ret = esp_ble_gap_register_callback(esp_blufi_gap_event_handler);
	if(ret)
	{
		ESP_LOGE(BTTAG,BT_ERR_CALLBACK_GAP_FAIL, ret);
		return;
	}//End if

	ret =  esp_blufi_profile_init();
	if(ret)
	{
		ESP_LOGE(BTTAG,BT_ERR_PROFILE_GAP_FAIL, ret);
		return;
	}//End if
	ESP_LOGI(BTTAG,BLUFI_INFO_INIT_DONE);
}

/*
 * ***********************************************************************
 * @brief       Drv_BluFi_DeInit
 * @param       None
 * @return      None
 * @details     Deinitialise Blufi
 **************************************************************************/
static void Drv_BluFi_DeInit()
{

	ESP_LOGE(BTTAG,BLUFI_INFO_DEINIT);

	int ret = esp_blufi_profile_deinit();
	if(ret != ESP_OK)
	{
		return;
	}

	ret = esp_bluedroid_disable();
	if (ret)
	{
		ESP_LOGE(BTTAG,BLUFI_ERR_DISABLE_FAIL, esp_err_to_name(ret));
		return;
	}

	ret = esp_bluedroid_deinit();
	if (ret)
	{
		ESP_LOGE(BTTAG, BLUFI_ERR_DEINIT_FAIL, esp_err_to_name(ret));

		return;
	}
	ESP_LOGE(BTTAG,BLUFI_INFO_DEINIT_DONE);
}

/*
 * ***********************************************************************
 * @brief       Drv_BluFi_Advertise
 * @param       devName - BluFi Name
 * @return      None
 * @details     Start Bluetooth LE advertising with custom hostname
 **************************************************************************/
static void Drv_BluFi_Advertise(char* devName)
{
	uint8_t blufi_ser_uuid128[32] = {
	    /* LSB <--------------------------------------------------------------------------------> MSB */
	    //first uuid, 16bit, [12],[13] is the value
	    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00,
	};

	esp_ble_adv_data_t blufi_adv_data_custom = {
	    .set_scan_rsp = false,
	    .include_name = true,
	    .include_txpower = true,
	    .min_interval = 0x0006, //slave connection min interval, Time = min_interval * 1.25 msec
	    .max_interval = 0x0010, //slave connection max interval, Time = max_interval * 1.25 msec
	    .appearance = 0x00,
	    .manufacturer_len = 0,
	    .p_manufacturer_data =  NULL,
	    .service_data_len = 0,
	    .p_service_data = NULL,
	    .service_uuid_len = 16,
	    .p_service_uuid = blufi_ser_uuid128,
	    .flag = 0x6,
	};

	if(!devName)
	{
		esp_blufi_adv_start();
		return;
	}

	char* tmpBuff = malloc(sizeof(char)*BLUFI_BUFF_LEN);
	memcpy(tmpBuff, BLUFI_FILTER_ID, strlen(BLUFI_FILTER_ID));
	memcpy(tmpBuff+strlen(BLUFI_FILTER_ID), devName, BLUFI_BUFF_LEN - strlen(BLUFI_FILTER_ID));

	esp_ble_gap_set_device_name(tmpBuff);


	esp_ble_gap_config_adv_data(&blufi_adv_data_custom);

//	snprintf(tmpBuff,sizeof(BLUFI_FILTER_ID"%s"),BLUFI_FILTER_ID"%s",devName);

	free(tmpBuff);

}

/*
 * ***********************************************************************
 * @brief       Drv_BluFi_fetchDevJSON
 * @param       mJson - JSON string contains the BluFi's device name
 * @return      None
 * @details     Extract BluFi's device name from JSON and save to NVS
 **************************************************************************/
static void Drv_BluFi_fetchDevJSON(char* mJson)
{
	if(!mJson)
	{
		ESP_LOGE(BTTAG,JSON_ERR_INPUT_NULL);
		return;
	}
	cJSON *root = cJSON_Parse(mJson);
	if(!root)
	{
		cJSON_Delete(root);
		ESP_LOGE(BTTAG,JSON_ERR_NODE_NULL" (root)");
		ESP_LOGI(BTTAG,JSON_INFO_NODE_NULL_HINT);
		return;
	}

	cJSON *blufi_ctm_config = cJSON_GetObjectItem(root, BLUFI_JSON_ROOT);

	if(!blufi_ctm_config)
	{
		ESP_LOGE(BTTAG, JSON_ERR_NODE_NULL" (blufi_ctm_config)");
		ESP_LOGI(BTTAG,JSON_INFO_NODE_NULL_HINT);
		cJSON_Delete(root);
		return;
	}//End if

	char *blufi_dev_name = cJSON_GetObjectItem(blufi_ctm_config,BLUFI_JSON_DEVNAME_KEY)->valuestring;

	if(blufi_dev_name == 0)
	{
		ESP_LOGE(BTTAG, JSON_ERR_NODE_NULL" (blufi_ctm_config)");
		cJSON_Delete(root);
		return;
	}

	NVS_WriteStr(BLUFI_DEV_NAME_NVS_KEY,blufi_dev_name);
	cJSON_Delete(root);
}

/*
 * ***********************************************************************
 * @brief       Drv_BluFi_fetchDevJSON
 * @param       mJson - JSON string contains the BluFi's device name
 * @return      None
 * @details     Extract BluFi's device name from JSON and save to NVS
 **************************************************************************/
static void Drv_BluFi_fetchWipeJSON(char* mJson)
{
	if(!mJson)
	{
		ESP_LOGE(BTTAG,JSON_ERR_INPUT_NULL);
		return;
	}
	cJSON *root = cJSON_Parse(mJson);
	if(!root)
	{
		cJSON_Delete(root);
		// ESP_LOGE(BTTAG,JSON_ERR_NODE_NULL" (root)");
		// ESP_LOGI(BTTAG,JSON_INFO_NODE_NULL_HINT);
		return;
	}

	cJSON *blufi_ctm_config = cJSON_GetObjectItem(root, BLUFI_FLAG_JSON_ROOT);

	if(!blufi_ctm_config)
	{
		// ESP_LOGE(BTTAG, JSON_ERR_NODE_NULL" (blufi_ctm_config)");
		// ESP_LOGI(BTTAG,JSON_INFO_NODE_NULL_HINT);
		cJSON_Delete(root);
		return;
	}//End if

	cJSON *blufi_wipe_obj = cJSON_GetObjectItem(blufi_ctm_config, BLUFI_WIPE_JSON_KEY);
	if(blufi_wipe_obj)
	{
		int blufi_wipe_flag = cJSON_GetObjectItem(blufi_ctm_config,BLUFI_WIPE_JSON_KEY)->valueint;
		// ESP_LOGI(BTTAG,"wipe = %d",blufi_wipe_flag);
		if(blufi_wipe_flag == 1)
		{
			NVS_PartErase();
			esp_restart();
		}
	}

	cJSON *blufi_rst_obj = cJSON_GetObjectItem(blufi_ctm_config, BLUFI_RST_JSON_KEY);
	if(blufi_rst_obj)
	{
		int blufi_wipe_rst = cJSON_GetObjectItem(blufi_ctm_config,BLUFI_RST_JSON_KEY)->valueint;
		if(blufi_wipe_rst == 1)
		{
			esp_restart();
		}
	}//End if

	cJSON_Delete(root);
}
/*
 * ***********************************************************************
 * @brief       Drv_BluFi_loadPref
 * @param       None
 * @return      None
 * @details     Wrap JSON and send via BluFi
 **************************************************************************/
static void Drv_BluFi_loadPref()
{
	const size_t dataSize = NVS_ReadSize(BLUFI_DEV_NAME_NVS_KEY);
	if(dataSize > BLUFI_BUFF_LEN)
	{
		char* tmpBuff = malloc(sizeof(char)*dataSize);
		NVS_ReadStr(BLUFI_DEV_NAME_NVS_KEY,tmpBuff);
		memcpy(mBlufiBuff,tmpBuff,BLUFI_BUFF_LEN);
		free(tmpBuff);
	}
	else if (dataSize == 0)
	{
		strncpy(mBlufiBuff,CONFIG_MI_BFI_DEV_NAME,BLUFI_BUFF_LEN);
	}
	else
	{
		NVS_ReadStr(BLUFI_DEV_NAME_NVS_KEY,mBlufiBuff);
	}//End if-else

}


/*
 * ***********************************************************************
 * @brief       Drv_BluFi_SendCtmDataJson
 * @param       mJSON - JSON to be sent
 * @return      None
 * @details     Wrap JSON and send via BluFi
 **************************************************************************/
void Drv_BluFi_SendCtmDataJson(cJSON* mJson)
{
	if(mJson == 0)
	{
		return;
	}

	cJSON *root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, BLUFI_CTM_JSON_ROOT, mJson);
	

	const char* jsonStr = cJSON_PrintUnformatted(root);
	esp_blufi_send_custom_data((uint8_t*)jsonStr, strlen(jsonStr));
	cJSON_free(root);
}

/*
 * ***********************************************************************
 * @brief       Drv_BluFi_GetCtmDataJson
 * @param       None
 * @return      None
 * @details     Fetch and extract JSON from custom data
 **************************************************************************/
void Drv_BluFi_GetCtmDataJson(char* jsonStr, char* jsonOut)
{
	cJSON *root = cJSON_Parse(jsonStr);
	if(root == 0)
	{
		ESP_LOGE(BTTAG,"Custom data is null");
		cJSON_Delete(root);
		return;
	}

	if(jsonOut == 0)
	{
		return;
	}
	cJSON *ctmDataJson =  cJSON_GetObjectItem(root, BLUFI_CTM_JSON_ROOT);
	if(!ctmDataJson)
	{
		ESP_LOGE(BTTAG,"Invalid ctmDataJson");
	}
	char* tmp = cJSON_PrintUnformatted(ctmDataJson);
	memcpy(jsonOut, tmp, strlen(tmp));
	
	cJSON* blufiDevOpModeObj = cJSON_GetObjectItem(ctmDataJson,BLUFI_JSON_DEV_OPMODE);
	if(blufiDevOpModeObj)
	{
		char* tBlufi_dev_opMode = cJSON_GetObjectItem(ctmDataJson,BLUFI_JSON_DEV_OPMODE)->valuestring;
		NVS_WriteStr(BLUFI_OPMODE_NVS_KEY,tBlufi_dev_opMode);
	}

}

