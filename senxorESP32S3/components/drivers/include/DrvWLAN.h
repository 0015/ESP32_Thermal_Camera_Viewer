/*****************************************************************************
 * @file     DrvWLAN.h
 * @version  1.2
 * @brief    Contains function for controlling WiFi
 * @date	 5 July 2022
 ******************************************************************************/

#ifndef COMPONENTS_DRIVERS_INCLUDE_DRVWLAN_H_
#define COMPONENTS_DRIVERS_INCLUDE_DRVWLAN_H_
#include <stddef.h>
#include <string.h>
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "DrvNVS.h"
#include "msg.h"

//JSON
#define JSON_WLAN_STATUS_ROOT	"wlan_status"
#define JSON_WLAN_STAT_ROOT		"station"
#define JSON_WLAN_STAT_SSID		"station_ssid"
#define JSON_WLAN_AP_ROOT		"ap"
#define JSON_WLAN_AP_SSID		"ap_ssid"
#define JSON_WLAN_AP_PWD		"ap_pwd"
#define JSON_WLAN_AP_MAC		"ap_mac"
#define JSON_WLAN_AP_CH			"ap_channel"
#define JSON_WLAN_AP_MAXCLIENT	"ap_max_connection"
#define JSON_WLAN_AP_AUTHMODE	"ap_auth_mode"
#define JSON_WLAN_NET_ROOT		"network"
#define JSON_WLAN_NET_IP		"ip_addr"

// FreeRTOS event group to signal when we are connected to wifi
#define WIFI_CONNECTED_BIT	BIT0
#define WIFI_FAIL_BIT      	BIT1

// Messages
#define WLANTAG 				"[WiFi]"
#define WLAN_AP_INIT			"WiFi AP mode initialised."
#define WLAN_AP_JOIN			"A client is joined. AID = %d"
#define WLAN_AP_LEFT			"A client is left. AID = %d "
#define WLAN_AUTH_OPEN			"Unsecured"
#define WLAN_AUTH_NA			"Unknown security mode"
#define WLAN_AUTH_WEP			"WEP"
#define WLAN_AUTH_WPA			"WPA PSK"
#define WLAN_AUTH_WPA_WPA2		"WPA or WPA2 PSK"
#define WLAN_AUTH_WPA2			"WPA2 PSK"
#define WLAN_AUTH_WPA2_EN		"WPA2 Enterprise"
#define WLAN_AUTH_WPA3			"WPA3 PSK"
#define WLAN_AUTH_WPA3_WPA2		"WPA3 or WPA2 PSK"
#define WLAN_CLIENT_CONNECT		"%s joined. AID: %d"
#define WLAN_CLIENT_DISCONNECT	"%s left. AID: %d"
#define WLAN_DEINIT_OK			"WiFi is shutdown."
#define WLAN_ERR_AP_CONNECT		"Cannot connect to the AP. Program halted."
#define WLAN_ERR_AP_CONNECT_RTY "Cannot connect to the AP. Retrying... Attempt: %d / %d"
#define WLAN_ERR_AP_NO_PWD		"No password is required to connect to this access point. This means the traffic is unencrypted and it is vulnerable to cyber attack. Consider to use WPA2 or higher security with strong password."
#define WLAN_ERR_AP_NOT_ENPT	"UNENCRYPTED WIFI"
#define WLAN_ERR_AP_CONNECT_RTN	"Cannot connected to %s. Returning to AP mode..."
#define WLAN_ERR_AP_PWD_PARAM	"Parameter provided invalid password"
#define WLAN_ERR_AP_PWDLEN		"The length of password in AP mode must be greater than 8."
#define WLAN_ERR_EVT_NA			"Unknown WiFi event."
#define WLAN_ERR_SSID_NULL		"Incorrect SSID."
#define WLAN_ERR_NOT_CONN		"WiFi is disconnected from station."
#define WLAN_ERR_NOT_INIT		"WiFi is not initialised."
#define WLAN_ERR_NA				"Unexpected event."
#define WLAN_ERR_MODE			"Invalid WiFi mode. "
#define WLAN_INIT_INFO			"WiFi initialising..."
#define WLAN_INIT_DONE			"WiFi initialised."
#define WLAN_INFO				"Starting WiFi task..."
#define WLAN_INFO_AP			"Access point (AP) Mode"
#define WLAN_INFO_AP_AUTH		"Security: "
#define WLAN_INFO_AP_CH			"Channel: %d "
#define WLAN_INFO_AP_INIT		"Initialsing WiFi in AP mode..."
#define WLAN_INFO_AP_MAXCON		"Maximum client allowed: %d"
#define WLAN_INFO_AP_SSID		"AP SSID: %s "
#define WLAN_INFO_AP_STH		"Strength: %d dBm"
#define WLAN_INFO_STA_INIT		"Initialsing WiFi in station mode..."
#define WLAN_INFO_STA_INIT_DONE	"WiFi initialised in station mode."
#define WLAN_INFO_STA_CONNECTED	"Connected to a WiFi access point."
#define WLAN_INFO_STA_USEPARAM	"Using SSID other than sdkconfig. SSID: %s"
#define WLAN_INFO_USE_NVS		"Using configuration from NVS."
#define WLAN_IP_GET				"IP Obtained: "
#define WLAN_RST				"Restarting WiFi..."
#define WLAN_SCAN_START			"Start scanning for available AP..."
#define WLAN_SCAN_TOTALAP		"Total APs found: %u"
#define WLAN_STAT_CONNECT		"The device is connected to: %s ."
#define WLAN_STOP				"Disabling WiFi..."
#define WLAN_WARN_AP_SDKCFGPWD	"Using the pre-configurated password in sdkconfig."
#define WLAN_WARN_STA_SDKCFGSSID "Using the pre-configurated SSID in sdkconfig."
#define WLAN_WARN_EVTGRP_EXIST	"An event group already initialised. Using the existing one."
#define WLAN_WARN_DEINIT		"WiFi is de-initialised. A complete initialisation is required to restart WiFi."


typedef struct wlanCfg{
	char AP_SSID[40];													//SSID buffer
	char AP_PWD[70];													//Password buffer
	char STA_SSID[40];													//SSID buffer
	char STA_PWD[70];													//Password buffer
	size_t mAPSSIDLen;
	size_t mAPPwdLen;
	size_t mSSIDLen;
	size_t mPwdLen;
}wlanCfg_t;


void Drv_WLAN_Init();

void Drv_WLAN_InitAP(const char* ssid, const char* pwd);

void Drv_WLAN_InitStat(const char* mSsid, const char* mPwd);

void Drv_WLAN_Stop(void);

void Drv_WLAN_Start(void);

void Drv_WLAN_Deinit(void);

void Drv_WLAN_Stat_Disconnect(void);

void Drv_WLAN_Stat_Reconnect(void);

void Drv_WLAN_Restart(const uint8_t WIFI_MODE);

void Drv_WLAN_getCfgNvs(wlanCfg_t* wlanCfgObj);

void Drv_WLAN_getRptToJson(cJSON* jsonObj);

void Drv_WLAN_getRptJsonStr(char* result, int buffLen);

int Drv_WLAN_getMode();

const char* Drv_WLAN_getIP(void);

bool Drv_WLAN_getIsIpObtained();

void Drv_WLAN_getAuthMode(int authmode);


uint8_t* Drv_WLAN_getStaSSID();
#endif /* COMPONENTS_DRIVERS_INCLUDE_DRVWLAN_H_ */
