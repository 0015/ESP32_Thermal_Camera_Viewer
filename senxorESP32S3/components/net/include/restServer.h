/*****************************************************************************
 * @file     restServer.h
 * @version  2.16
 * @brief    Header file for restServer.c
 * @date	 26 Apr 2023
 ******************************************************************************/
#ifndef COMPONENTS_NET_INCLUDE_RESTSERVER_H_
#define COMPONENTS_NET_INCLUDE_RESTSERVER_H_
#include <string.h>
#include <fcntl.h>
#include <esp_http_server.h>
#include <esp_chip_info.h>
#include <esp_random.h>
#include <esp_log.h>
#include <esp_vfs.h>
#include <esp_system.h>
#include <cJSON.h>
#include "DrvWLAN.h"
#include "DrvNVS.h"
#include "msg.h"


//Structure definition for JSON strings
//Area configuration
typedef struct areaJsonStruct
{
    uint8_t 	mIsAlrmEn;			//Whether the alarm should trigger as threshold reaches
    uint16_t 	mThershold;			//Threshold
    uint16_t 	mAvgTmp;			//Average temperature in this area (mK). Not in use during configuration phase.
    uint8_t 	mX;					//Location - x coordinates
    uint8_t 	mY;					//Location - y coordinates
    uint8_t 	mWidth;				//Dimension - Width
    uint8_t 	mHeight;			//Dimension - Height
}areaJsonCfgStruct;


typedef struct areaJsonCfgArrStruct
{
	areaJsonCfgStruct mAreaJsonCfgArrStructObj[4];
}areaJsonCfgArrStruct;


//JSON string node definition

//JSON string for alert
#define JSON_ALRM_ROOT						"alarm"
//JSON string for area configuration
#define JSON_AREA_ROOT_NVS_KEY				"config_area"
#define JSON_AREA_ROOT						"area_config"
#define JSON_SUB_AREA_FORMAT				"area_%d"
#define JSON_AREA_DIMENSION					"dimension"
#define JSON_AREA_THERSHOLD					"threshold"
#define JSON_AREA_AVG_TEMP					"avg_temp"
#define JSON_AREA_ALRM_EN					"is_alarm_en"



//JSON string for status report
#define JSON_STATUS_ROOT					"status"
#define JSON_STATUS_AREA					"area"
#define JSON_SENXOR							"senxor"
#define JSON_SENXOR_MODE					"capture_mode"
#define JSON_SENXOR_MODULE					"module"
#define JSON_SENXOR_TYPE					"type"
#define JSON_TOKEN							"token"

#define JSON_SYS_INFO_ROOT					"sys_info"
#define JSON_SYS_INFO_FW_VER				"fw_version"
#define JSON_SYS_INFO_SXRLIB_VER			"senxorlib_version"
#define JSON_SYS_INFO_IDF_VER				"idf_version"
#define JSON_SYS_INFO_CPU_CORE				"cores"
#define JSON_SYS_INFO_CPU_SPD				"speed"
#define JSON_SYS_INFO_CPU_MODEL				"model"
#define BLUFI_JSON_DEV_OPMODE               "op_mode"

//JSON string for wlan configuration
#define JSON_WLAN_ROOT_NVS_KEY				"config_wlan"
#define JSON_WLAN_ROOT						"wifi_config"
#define JSON_WLAN_STAT_PWD					"pwd"
#define JSON_WLAN_MODE						"wlan_mode"
#define JSON_WLAN_MDNS_HOSTNAME				"hostname"


//HTTP Server defines
#define CHECK_FILE_EXTENSION(filename, ext) (strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0)
#define FILE_PATH_MAX 						(ESP_VFS_PATH_MAX + 128)
#ifdef CONFIG_SPIRAM
#define SCRATCH_BUFSIZE 					(20480)
#else
#define SCRATCH_BUFSIZE 					(10240)
#endif


void restServer_Init();

void restServer_Deinit();

void printAreaCfgDetails(const areaJsonCfgStruct* areaCfgObj);

httpd_handle_t getRestServerHandler();

#endif /* COMPONENTS_NET_INCLUDE_RESTSERVER_H_ */
