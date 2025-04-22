/*****************************************************************************
 * @file     DrvWLAN.c
 * @version  1.8
 * @brief    WiFi driver
 * @date	 8 Jul 2022
 ******************************************************************************/
#include "DrvWLAN.h"
#include "DrvNVS.h"
#include "MCU_Dependent.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

//public:
extern void ledCtrlPwmSet(uint8_t r, uint8_t g, uint8_t b, uint16_t fadeDuration);
//private:
static bool mIsIpObtain = 0;

static char ip_obtain[40];
static esp_event_handler_instance_t instance_any_id;
static esp_event_handler_instance_t instance_got_ip;
EXT_RAM_BSS_ATTR static EventGroupHandle_t s_wifi_event_group;
static uint8_t s_retry_num = 0;
static uint8_t sta_ssid[32];

static void Drv_WLAN_EventGrpInit(void);
static void Drv_WLAN_EventGrpDeinit(void);

/*
 * ***********************************************************************
 * @brief       ip_event_handler
 * @param       None
 * @return      None
 * @details     Event handler for IP events
 **************************************************************************/
static void ip_event_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{
	switch(event_id)
	{
		case IP_EVENT_STA_GOT_IP:
	        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
	        ESP_LOGI(WLANTAG, WLAN_IP_GET IPSTR, IP2STR(&event->ip_info.ip));
	        sprintf(ip_obtain, IPSTR, IP2STR(&event->ip_info.ip));
	        s_retry_num = 0;
	        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
	        mIsIpObtain = true;
	    break;
		case IP_EVENT_STA_LOST_IP:
			ip_obtain[0] = 0;
			mIsIpObtain = false;
		break;
		default:
		break;
	}//End switch
}

/*
 * ***********************************************************************
 * @brief       wifi_event_handler
 * @param       None
 * @return      None
 * @details     Event handler for WiFi driver
 **************************************************************************/
static void wifi_event_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{
	wifi_event_sta_connected_t *event;
	switch(event_id)
	{
		case WIFI_EVENT_WIFI_READY:
			ESP_LOGI(WLANTAG, WLAN_INFO_STA_INIT_DONE);
		break;
		case WIFI_EVENT_STA_START:
			esp_wifi_connect();
		break;
		case WIFI_EVENT_STA_CONNECTED:
			ESP_LOGI(WLANTAG, WLAN_INFO_STA_CONNECTED);
			event = (wifi_event_sta_connected_t*) event_data;
			memcpy(sta_ssid, event->ssid, event->ssid_len);

		break;
		case WIFI_EVENT_STA_DISCONNECTED:
	        if (s_retry_num < CONFIG_ESP_MAXIMUM_RETRY)
	        {
	            esp_wifi_connect();
	            s_retry_num++;
	            ESP_LOGI(WLANTAG, WLAN_ERR_AP_CONNECT_RTY, s_retry_num,CONFIG_ESP_MAXIMUM_RETRY);
	        }
	        else
	        {
	            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);

	#if CONFIG_MI_LED_PWM_EN
				ledCtrlPwmSet(255, 210, 0, 0);
	#endif
	#ifdef CONFIG_MI_LCD_EN
	        	drawIcon(96,76,ICON_WIFI_FAIL);
	        	drawText(1, 76+48,"Fail to connect to AP.");
	#endif
	        }//End if

		break;

		case WIFI_EVENT_SCAN_DONE:
			ESP_LOGI(WLANTAG, "WiFi scanned");
		break;

		case WIFI_EVENT_AP_START:
			ESP_LOGI(WLANTAG, WLAN_AP_INIT);
		break;

	    case WIFI_EVENT_AP_STACONNECTED:
	        wifi_event_ap_staconnected_t* apConnectEvent = (wifi_event_ap_staconnected_t*) event_data;
	        ESP_LOGI(WLANTAG, "%02x:%02x:%02x:%02x:%02x:%02x", MAC2STR(apConnectEvent->mac) );
	        ESP_LOGI(WLANTAG, WLAN_AP_JOIN, apConnectEvent->aid);
	    break;

	    case WIFI_EVENT_AP_STADISCONNECTED:
	        wifi_event_ap_stadisconnected_t* apDisconnectEvent = (wifi_event_ap_stadisconnected_t*) event_data;
	        ESP_LOGI(WLANTAG, "%02x:%02x:%02x:%02x:%02x:%02x", MAC2STR(apDisconnectEvent->mac) );
	        ESP_LOGI(WLANTAG, WLAN_AP_LEFT, apDisconnectEvent->aid);
	        //Acces_Write_Reg(0xB1,0);
	    break;
	}//End switch
}

/*
 * ***********************************************************************
 * @brief       Drv_WLAN_Init
 * @param       None
 * @return      None
 * @details     Initialise WiFi
 **************************************************************************/
void Drv_WLAN_Init(void)
{
	if(MCU_getOpMode() == USB_MODE)
	{
		return;
	}
	ESP_ERROR_CHECK(esp_netif_init());											//Initialise ESP-NETIF library allowing the use of DHCP

	wlanCfg_t* wlanCfgObj = malloc(sizeof(wlanCfg_t));					//Allocate WiFi configuration object
	memset(wlanCfgObj, 0, sizeof(wlanCfg_t));
	Drv_WLAN_getCfgNvs(wlanCfgObj);												// Read configuration from NVS

	Drv_WLAN_EventGrpInit();													//Create default event loop

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();						//Load default WLAN configuration (can be changed via menuconfig)
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));								//Initialise WiFi sub-system

	// Turn on some functions based on operation mode to save resources


#if CONFIG_MI_WLAN_AP_FORCE
	goto DRV_WLAN_MODE_AP;
#else
	// Turn on some functions based on operation mode to save resources
	switch (MCU_getOpMode()) 
	{
		case WLAN_MODE:
			goto DRV_WLAN_MODE_APSTAT;
		break;
		case USB_MODE:
			goto DRV_WLAN_MODE_AP;
		break;
	}// End switch
	// goto DRV_WLAN_MODE_APSTAT;
#endif


DRV_WLAN_MODE_AP:
			
	esp_netif_create_default_wifi_ap();												// Initialise event handler for AP mode

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));							//	Set WiFi as AP mode
	Drv_WLAN_InitAP(wlanCfgObj->AP_SSID, wlanCfgObj->AP_PWD);			//	Initialise WiFi in AP mode

	goto DRV_WLAN_START;


DRV_WLAN_MODE_APSTAT:

	esp_netif_create_default_wifi_sta();											// Initialise event handler for AP mode
	esp_netif_create_default_wifi_ap();												// Initialise event handler for Station mode

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));						//	Set WiFi as AP+STATION mode
	Drv_WLAN_InitAP(wlanCfgObj->AP_SSID, wlanCfgObj->AP_PWD);			//	Initialise WiFi in AP mode
	Drv_WLAN_InitStat(wlanCfgObj->STA_SSID, wlanCfgObj->STA_PWD);		//	Initialise WiFi in Station mode

	goto DRV_WLAN_START;

DRV_WLAN_START:
	Drv_WLAN_Start();																// Start WiFi
	free(wlanCfgObj);																// Clean up resources
}

/*
 * ***********************************************************************
 * @brief       Drv_WLAN_InitAP
 * @param       ssid - AP's SSID
 * 				pwd - AP's password
 * @return      None
 * @details     Initialise WiFi (AP Mode)
 **************************************************************************/
void Drv_WLAN_InitAP(const char* ssid, const char* pwd)
{
	ESP_LOGI(WLANTAG,WLAN_INFO_AP_INIT);

	/*
	 * Configure WiFi access point here
	 * SSID and password SHOULD NOT BE hardcode here. It should point to the value specified in Kconfig.projbuild
	 * to allow user to change the value via sdkconfig
	 */

	wifi_config_t wifi_config_ap = {
		   .ap = {
				.ssid = CONFIG_ESP_WIFI_AP_SSID,
				.ssid_len = strlen(CONFIG_ESP_WIFI_AP_SSID),
				.password = CONFIG_ESP_WIFI_AP_PASSWORD,
				.max_connection = CONFIG_ESP_MAX_STA_CONN,
				.authmode = WIFI_AUTH_WPA2_PSK,
				.pmf_cfg = {
								.required = true,
				            },
				},
	};

#ifdef CONFIG_MI_WLAN_USE_NVS
	/*
	 * Check if SSID is NULL or not. If not, use passed SSID instead of sdkconfig
	 */
	if(strlen(ssid) != 0 && ssid != NULL && strcmp(ssid," ")!=0)
	{
		ESP_LOGI(WLANTAG,WLAN_INFO_USE_NVS);
		ESP_LOGI(WLANTAG,WLAN_INFO_AP_SSID,ssid);
		memcpy(wifi_config_ap.ap.ssid,ssid,32);
		wifi_config_ap.ap.ssid_len = strlen(ssid);
	}
	else
	{
		ESP_LOGE(WLANTAG,WLAN_ERR_SSID_NULL);
	}
	//End if-else

	/*
	 * Switch to open network when no password is inserted in AP mode
	 * A warning will be displayed on console to advise user switching to encrypted WiFi
	 */
	//If provided password is empty
	if (strlen(pwd) == 0 || pwd == NULL)
	{
		//Check if sdkconfig had password
		//If true MCU AP will be initialised in open network
		if(strlen(CONFIG_ESP_WIFI_AP_PASSWORD) == 0)
		{
			ESP_LOGE(WLANTAG,WLAN_ERR_AP_NO_PWD);
			wifi_config_ap.ap.authmode = WIFI_AUTH_OPEN;
		}
		else
		{
			ESP_LOGE(WLANTAG,WLAN_ERR_AP_PWD_PARAM);
			ESP_LOGW(WLANTAG,WLAN_WARN_AP_SDKCFGPWD);
		}//End if-else
	}
	else
	{
		if(strlen(pwd) > 8)
		{
			memcpy(wifi_config_ap.ap.password,pwd,64);
			wifi_config_ap.ap.authmode = WIFI_AUTH_WPA2_PSK;
		}
		else
		{
			ESP_LOGE(WLANTAG,WLAN_ERR_AP_PWDLEN);
			ESP_LOGW(WLANTAG,WLAN_WARN_AP_SDKCFGPWD);
		}//End if-else
	}//End if-else
#endif

	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config_ap));				//Configure ESP32 to be worked in AP Mode

	//Display AP info
	wifi_config_t wifi_config_ap_info;												//WiFi AP info object
	esp_wifi_get_config(WIFI_IF_AP, &wifi_config_ap_info);							//Obtain AP info
	ESP_LOGI(WLANTAG, WLAN_INIT_DONE);
	ESP_LOGI(WLANTAG, WLAN_INFO_AP);
	ESP_LOGI(WLANTAG, WLAN_INFO_AP_SSID, wifi_config_ap_info.ap.ssid);
	ESP_LOGI(WLANTAG, WLAN_INFO_AP_CH, wifi_config_ap_info.ap.channel);
	Drv_WLAN_getAuthMode(wifi_config_ap_info.ap.authmode);
	ESP_LOGI(WLANTAG, WLAN_INFO_AP_MAXCON, wifi_config_ap_info.ap.max_connection);

}
/*
 * ***********************************************************************
 * @brief       Drv_WLAN_InitStat
 * @param       mSsid - AP's ssid
 * 				mPwd - AP's password
 * @return      None
 * @details     Initialise WiFi (Station Mode)
 **************************************************************************/
void Drv_WLAN_InitStat(const char* mSsid, const char* mPwd)
{

	ESP_LOGI(WLANTAG,WLAN_INFO_STA_INIT);

	//WiFi configuration object
	wifi_config_t wifi_config_stat = {
			.sta = {
					.ssid = CONFIG_ESP_WIFI_STA_SSID,
					.password = CONFIG_ESP_WIFI_STA_PASSWORD
			},
	};

#ifdef CONFIG_MI_WLAN_USE_NVS
	/*
	 * Check if SSID is NULL or not. If not, use passed SSID instead of sdkconfig
	 */
	if(strlen(mSsid) != 0 && mSsid != NULL && strcmp(mSsid," ")!=0)
	{
		ESP_LOGI(WLANTAG,WLAN_INFO_USE_NVS);
		ESP_LOGI(WLANTAG,WLAN_INFO_AP_SSID,mSsid);
		memcpy(wifi_config_stat.sta.ssid,mSsid,32);
	}
	else
	{
		ESP_LOGE(WLANTAG,WLAN_ERR_SSID_NULL);
		ESP_LOGW(WLANTAG,WLAN_WARN_STA_SDKCFGSSID);
	}
	//End if-else

	/*
	 * Switch to open network when no password is inserted in AP mode
	 * A warning will be displayed on console to advise user switching to encrypted WiFi
	 */
	//If provided password is empty
	if (strlen(mPwd) == 0 || mPwd == NULL || strcmp(mPwd," ") == 0)
	{
		//Check if sdkconfig had password
		//If true MCU AP will be initialised in open network
		if(strlen(CONFIG_ESP_WIFI_AP_PASSWORD) == 0)
		{
			ESP_LOGE(WLANTAG,WLAN_ERR_AP_NO_PWD);
		}
	}
	else
	{
		memcpy(wifi_config_stat.sta.password,mPwd,64);
	}//End if-else
#endif

	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config_stat));	//Configure ESP32 to be worked in Station Mode
	ESP_ERROR_CHECK(esp_wifi_set_ps(CONFIG_MI_WLAN_PWR_SAVE_VAL));			//Enable WiFi power saving
}

/*
 * ***********************************************************************
 * @brief       Drv_WLAN_Deinit
 * @param       None
 * @return      None
 * @details     Deinitialise WiFi.
 * 				This will free all resources allocated to WiFi.
 * 				A complete initialisation must be called to
 * 				turn WiFi back on.
 **************************************************************************/
void Drv_WLAN_Deinit(void)
{
	ESP_LOGI(WLANTAG,WLAN_STOP);

	esp_err_t err = esp_wifi_stop();				//Stop WiFi and free all related control block
	if(err != ESP_OK)
	{
		ESP_LOGE(WLANTAG,"%s",esp_err_to_name(err));
	}

	err = esp_wifi_deinit();						//De-initialise WiFi hardware
	ESP_LOGI(WLANTAG,"%s",esp_err_to_name(err));
	if(err != ESP_OK)
	{
		ESP_LOGE(WLANTAG,"%s",esp_err_to_name(err));
	}

	ESP_LOGI(WLANTAG,WLAN_DEINIT_OK);
	ESP_LOGW(WLANTAG,WLAN_WARN_DEINIT);
	Drv_WLAN_EventGrpDeinit();						//De-initialise event group also
}
/*
 * ***********************************************************************
 * @brief       Drv_WLAN_Stop
 * @param       None
 * @return      None
 * @details     Stop WiFi control block but keep WiFi driver loaded
 **************************************************************************/
void Drv_WLAN_Stop(void)
{
	ESP_LOGI(WLANTAG,WLAN_STOP);
	esp_err_t err = esp_wifi_stop();
	if(err != ESP_OK)
	{
		ESP_LOGE(WLANTAG,"%s",esp_err_to_name(err));
	}
}
/*
 * ***********************************************************************
 * @brief       Drv_WLAN_Start
 * @param       None
 * @return      None
 * @details     Start WiFi
 **************************************************************************/
void Drv_WLAN_Start(void)
{
	esp_err_t err = esp_wifi_start();
	if(err != ESP_OK)
	{
		ESP_LOGE(WLANTAG,"%s",esp_err_to_name(err));
	}
}

/*
 * ***********************************************************************
 * @brief       Drv_WLAN_Stat_Reconnect
 * @param       None
 * @return      None
 * @details     Attempting to reconnect to station.
 * 				Stop if retry limit is reached or the station is connected.
 **************************************************************************/
void Drv_WLAN_Stat_Reconnect(void)
{
	Drv_WLAN_Stat_Disconnect();
	s_retry_num = 0;
    if (s_retry_num < CONFIG_ESP_MAXIMUM_RETRY)
    {
        esp_wifi_connect();
        s_retry_num++;
    }
}

/*
 * ***********************************************************************
 * @brief       Drv_WLAN_Stat_Disconnect
 * @param       None
 * @return      None
 * @details     Disconnect from a station
 **************************************************************************/
void Drv_WLAN_Stat_Disconnect(void)
{
	esp_err_t err = esp_wifi_disconnect();
	if(err != ESP_OK)
	{
		ESP_LOGE(WLANTAG,"%s",esp_err_to_name(err));
	}
}

/*
 * ***********************************************************************
 * @brief       WLANEventGrpInit
 * @param       None
 * @return      None
 * @details     Initialise WiFi Event group
 **************************************************************************/
static void Drv_WLAN_EventGrpInit(void)
{
	s_wifi_event_group = xEventGroupCreate();
	ESP_ERROR_CHECK(esp_event_loop_create_default());		//Create default event loop
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,ESP_EVENT_ANY_ID,&wifi_event_handler,NULL,&instance_any_id));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,IP_EVENT_STA_GOT_IP,&ip_event_handler,NULL,&instance_got_ip));
}

/*
 * ***********************************************************************
 * @brief       WLANEventGrpDeinit
 * @param       None
 * @return      None
 * @details     De-initialise WiFi Event group
 **************************************************************************/
static void Drv_WLAN_EventGrpDeinit(void)
{
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
}

/*
 * ***********************************************************************
 * @brief       Drv_WLAN_getCfgNvs
 * @param       wlanCfgObj - Object hold the ssid and password
 * 				of station and ap mode respectively
 * @return      None
 * @details     Load configuration from NVS. Noted that NVS must be
 * 				initialised prior to this function
 **************************************************************************/
void Drv_WLAN_getCfgNvs(wlanCfg_t* pWlanCfgObj)
{

	if(pWlanCfgObj == 0)
	{
		return;
	}// End if
	
	size_t tLen = NVS_ReadSize("ssid_ap");
	
	if(tLen > 0)
	{
		pWlanCfgObj->mAPSSIDLen = tLen;
	}
	else 
	{
		NVS_WriteStr("ssid_ap",CONFIG_ESP_WIFI_AP_SSID);			//Read stored AP SSID
		pWlanCfgObj->mAPSSIDLen = strlen(CONFIG_ESP_WIFI_AP_SSID);
	}// End if-else

	tLen = NVS_ReadSize("pwd_ap");
	if (tLen > 0) 
	{
		pWlanCfgObj->mAPPwdLen = tLen;
	}
	else 
	{
		NVS_WriteStr("pwd_ap",CONFIG_ESP_WIFI_AP_PASSWORD);			//Read stored AP password
		pWlanCfgObj->mAPPwdLen = strlen(CONFIG_ESP_WIFI_AP_PASSWORD);
	}// End if-else

	NVS_ReadStr("ssid_ap",pWlanCfgObj->AP_SSID);					//Read stored AP SSID
	NVS_ReadStr("pwd_ap",pWlanCfgObj->AP_PWD);						//Read stored AP password

	tLen = NVS_ReadSize("ssid");
	if(tLen > 0)
	{
		pWlanCfgObj->mSSIDLen = tLen;
	}
	else 
	{
		NVS_WriteStr("ssid",CONFIG_ESP_WIFI_STA_SSID);				 //Read stored SSID
		pWlanCfgObj->mAPSSIDLen = strlen(CONFIG_ESP_WIFI_STA_SSID);
	}// End if-else

	tLen = NVS_ReadSize("pwd");
	if(tLen > 0)
	{
		pWlanCfgObj->mPwdLen = tLen;
	}
	else 
	{
		NVS_WriteStr("pwd",CONFIG_ESP_WIFI_STA_PASSWORD);			//Read stored SSID
		pWlanCfgObj->mPwdLen = strlen(CONFIG_ESP_WIFI_STA_PASSWORD);
	}// End if-else

	NVS_ReadStr("ssid",pWlanCfgObj->STA_SSID);						//Read stored SSID
	NVS_ReadStr("pwd",pWlanCfgObj->STA_PWD);						//Read stored password

}

/*
 * ***********************************************************************
 * @brief       Drv_WLAN_getRptToJson
 * @param       jsonObj - An existing JSON object
 * @return      None
 * @details     Add WLAN report to existing JSON object
 **************************************************************************/
void Drv_WLAN_getRptToJson(cJSON* jsonObj)
{
	uint8_t base_mac_addr[6] = {0};
	char macAddrBuff[20] = {0};
	ESP_ERROR_CHECK(esp_read_mac(base_mac_addr, ESP_MAC_WIFI_SOFTAP));
	snprintf(macAddrBuff,20,"%x:%x:%x:%x:%x:%x",base_mac_addr[0],base_mac_addr[1],base_mac_addr[2],base_mac_addr[3],base_mac_addr[4],base_mac_addr[5]);
	if(jsonObj == 0)
	{
		return;
	}

	//Construct a JSON network report
	cJSON *wlan_status_node = cJSON_AddObjectToObject(jsonObj, JSON_WLAN_STATUS_ROOT);

	cJSON *network_root_node = cJSON_AddObjectToObject(wlan_status_node,JSON_WLAN_NET_ROOT);
	cJSON_AddStringToObject(network_root_node, JSON_WLAN_NET_IP, ip_obtain);
}

/*
 * ***********************************************************************
 * @brief       Drv_WLAN_getRptJsonStr
 * @param       result - Character buffer
 * 				buffLen - Buffer length
 * @return      None
 * @details     Get JSON string
 **************************************************************************/
void Drv_WLAN_getRptJsonStr(char* result, int buffLen)
{

	//Display AP info
	wifi_config_t wifi_config_info;												//WiFi AP info object
	if (esp_wifi_get_config(WIFI_IF_STA, &wifi_config_info) != ESP_OK)
	{
		return;
	}//End if

	//Construct a JSON network report
	cJSON *root = cJSON_CreateObject();
	cJSON *wlan_status_node = cJSON_AddObjectToObject(root, JSON_WLAN_STATUS_ROOT);
	cJSON *stat_root_node = cJSON_AddObjectToObject(wlan_status_node, JSON_WLAN_STAT_ROOT);
	cJSON_AddStringToObject(stat_root_node, JSON_WLAN_STAT_SSID, (char*) wifi_config_info.sta.ssid);

	if (esp_wifi_get_config(WIFI_IF_AP, &wifi_config_info) != ESP_OK)
	{
		cJSON_free(root);
		return;
	}//End if

	cJSON *ap_root_node = cJSON_AddObjectToObject(wlan_status_node,JSON_WLAN_AP_ROOT);
	cJSON_AddStringToObject(ap_root_node, JSON_WLAN_STAT_SSID, (char*) wifi_config_info.ap.ssid);
	cJSON_AddStringToObject(ap_root_node, JSON_WLAN_AP_PWD, (char*) wifi_config_info.ap.password);
	cJSON_AddNumberToObject(ap_root_node, JSON_WLAN_AP_CH, wifi_config_info.ap.channel);

	cJSON *network_root_node = cJSON_AddObjectToObject(wlan_status_node,JSON_WLAN_NET_ROOT);
	cJSON_AddStringToObject(network_root_node, JSON_WLAN_NET_IP, ip_obtain);
    const char* rawJson = cJSON_PrintUnformatted(root);
    memcpy(result,rawJson,strlen(rawJson));
    cJSON_Delete(root);																	//Free up memory
}

/*
 * ***********************************************************************
 * @brief       Drv_WLAN_getMode
 * @param       None
 * @return      None
 * @details     Get WiFi Operation mode
 **************************************************************************/
int Drv_WLAN_getMode()
{
	wifi_mode_t wifi_mode;
	switch(esp_wifi_get_mode(&wifi_mode))
	{
		case ESP_OK:
			return wifi_mode;
		default:
			return -1;
	}
}
/*
 * ***********************************************************************
 * @brief       Drv_WLAN_getStaSSID
 * @param       None
 * @return      None
 * @details     Get WiFi station SSID
 **************************************************************************/
uint8_t* Drv_WLAN_getStaSSID()
{
	return sta_ssid;
}
/*
 * ***********************************************************************
 * @brief       Drv_WLAN_getIP
 * @param       None
 * @return      A string containing IP address
 * @details     Return the IP address obtained from access point.
 * 				Only works when the device is connected to a access point with
 * 				a valid IP
 **************************************************************************/
const char* Drv_WLAN_getIP(void)
{
	return ip_obtain;
}

/*
 * ***********************************************************************
 * @brief       Drv_WLAN_getIsIpObtained
 * @param       None
 * @return      None
 * @details     Get flag which set if IP is obtained
 **************************************************************************/
bool Drv_WLAN_getIsIpObtained()
{
	return mIsIpObtain;
}

/*
 * ***********************************************************************
 * @brief       Drv_WLAN_getAuthMode
 * @param       None
 * @return      None
 * @details     Display authentication of the station
 **************************************************************************/
void Drv_WLAN_getAuthMode(int authmode)
{
	ESP_LOGI(WLANTAG,WLAN_INFO_AP_AUTH);
    switch (authmode)
    {
		case WIFI_AUTH_OPEN:
			ESP_LOGW(WLANTAG, WLAN_AUTH_OPEN);
			ESP_LOGE(WLANTAG,WLAN_ERR_AP_NO_PWD);
		break;
		case WIFI_AUTH_WEP:
			ESP_LOGI(WLANTAG, WLAN_AUTH_WEP);
		break;
		case WIFI_AUTH_WPA_PSK:
			ESP_LOGI(WLANTAG, WLAN_AUTH_WPA);
		break;
		case WIFI_AUTH_WPA2_PSK:
			ESP_LOGI(WLANTAG, WLAN_AUTH_WPA2);
		break;
		case WIFI_AUTH_WPA_WPA2_PSK:
			ESP_LOGI(WLANTAG, WLAN_AUTH_WPA_WPA2);
		break;
		case WIFI_AUTH_WPA2_ENTERPRISE:
			ESP_LOGI(WLANTAG, WLAN_AUTH_WPA2_EN);
		break;
		case WIFI_AUTH_WPA3_PSK:
			ESP_LOGI(WLANTAG, WLAN_AUTH_WPA3);
		break;
		case WIFI_AUTH_WPA2_WPA3_PSK:
			ESP_LOGI(WLANTAG, WLAN_AUTH_WPA3_WPA2);
		break;
		default:
			ESP_LOGE(WLANTAG, WLAN_AUTH_NA);
		break;
    }
}

