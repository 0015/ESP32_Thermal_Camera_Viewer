/*****************************************************************************
 * @file     main.c
 * @version  2.04
 * @brief    Program entry point
 * @date	 24 May 2022
 ******************************************************************************/
#include <esp_log.h>				//ESP logger
#include <sdkconfig.h>				//ESP IDF SDK configuration

//SenXorLib:
#include "Customer_Interface.h"
#include "MCU_Dependent.h"			//MCU initialisation code
#include "esp_err.h"
#include "msg.h"					//Messages
#include "SenXorLib.h"				//Using SenXor library

//Tasks:
#include "ledCtrlTask.h"			//LED control task
#include "senxorTask.h"				//senxorTask
#include "tcpServerTask.h"			//tcpServerTask
#include "usbSerialTask.h"			//usbSerialTask

//public:
MCU_REG MCU_REGISTER;

//private:
static void ESP32_Net_Init(void);
static uint8_t initCheck(void);

//Stack buffers for tasks
#if CONFIG_MI_LED_EN
EXT_RAM_BSS_ATTR static StackType_t ledTaskStack[LED_TASK_STACK_SIZE];
static StaticTask_t ledTaskBuffer;
extern TaskHandle_t ledCtrlTaskHandle;
#endif

#if CONFIG_FREERTOS_USE_TRACE_FACILITY
EXT_RAM_BSS_ATTR char ptrTaskList[2500];
#endif

EXT_RAM_BSS_ATTR static StackType_t senxorTaskStack[SENXOR_TASK_STACK_SIZE];
static StaticTask_t senxorTaskBuffer;
extern TaskHandle_t senxorTaskHandle;

EXT_RAM_BSS_ATTR static StackType_t tcpServerTaskStack[TCP_TASK_STACK_SIZE];
static StaticTask_t tcpServerTaskBuffer;
extern TaskHandle_t tcpServerTaskHandle;

EXT_RAM_BSS_ATTR static StackType_t usbSerialTaskStack[USB_TASK_STACK_SIZE];
static StaticTask_t usbSerialTaskBuffer;
extern TaskHandle_t usbSerialTaskHandle;
/******************************************************************************
 * @brief       app_main
 * @param       none
 * @return      None
 * @details     Program entry point
 *****************************************************************************/
void app_main(void)
{
#if CONFIG_FREERTOS_USE_TRACE_FACILITY
	vTaskList(ptrTaskList);
	ESP_LOGI(MTAG,"%s\r\n",ptrTaskList);
#endif
	//Self check before initialisation
	switch(initCheck())
	{
		case 1:
			ESP_LOGE(MTAG,MAIN_ERR_CHK_FAIL);
			vTaskDelete(NULL);
		break;
		case 2:
			ESP_LOGW(MTAG,MAIN_WARN_CHK);
		break;
		default:
			ESP_LOGI(MTAG,MAIN_CHK_PASS);
		break;
	}//End switch


	ESP32_Peri_Init();																									//Initialise MCU peripherals

	if(senxorInit() != 0)
	{
		ESP_LOGE(SXRTAG,SXR_ERR_INIT);
	#if CONFIG_MI_LED_EN
		ledCtrlSingleSet(RED_LED,LED_ON,3000);
	#endif
		vTaskDelete(NULL);
	}//End if

#if CONFIG_MI_LED_EN
	ledCtrlTaskHandle = xTaskCreateStatic(ledCtrlTask, "ledCtrlTask", LED_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY, ledTaskStack, &ledTaskBuffer);
	if(!ledCtrlTaskHandle)
	{
		ESP_LOGW(LEDTAG,LED_ERR_TASK_FAIL_INIT);
	}//End if
#endif
	//Summoning SenXor task
	senxorTaskHandle = xTaskCreateStaticPinnedToCore(senxorTask, "senxorTask", SENXOR_TASK_STACK_SIZE, NULL, 7, senxorTaskStack, &senxorTaskBuffer, 1);
	if(!senxorTaskHandle)
	{
		ESP_LOGE(SXRTAG,SXR_ERR_TASK_FAIL_INIT);
		vTaskDelete(NULL);
	}//End if

	// //WARNING: Net components should be enabled only after SenXor is initialised.
	ESP32_Net_Init();

	tcpServerTaskHandle = xTaskCreateStaticPinnedToCore(tcpServerTask, "tcpServerTask", TCP_TASK_STACK_SIZE, NULL, 7, tcpServerTaskStack, &tcpServerTaskBuffer, 0);																					//Free main task from queue
}
/******************************************************************************
 * @brief       ESP32_Net_Init
 * @param       None
 * @return      None
 * @details     Initialise network related components
 *****************************************************************************/
static void ESP32_Net_Init(void)
{

#if (CONFIG_SOC_BT_SUPPORTED)
#if (CONFIG_BT_ENABLED) && (CONFIG_BT_BLUEDROID_ENABLED) && (CONFIG_MI_BFI_EN)
	Drv_BT_Init();									//Initialise Bluetooth for bluefi
#else
	ESP_LOGE(MTAG,BT_ERR_NOT_ENABLED);
#endif
#else
	ESP_LOGE(MTAG,BT_ERR_NOT_SUPPORTED);
#endif
	Drv_WLAN_Init();								//Initialise WiFi
	if(MCU_getOpMode() == WLAN_MODE)
	{
		
		restServer_Init();								//Initialise REST server also
	}
}//End ESP32_Net_Init


/******************************************************************************
 * @brief       initCheck
 * @param       none
 * @return      0 if passed, 1 if failed.
 * @details     Check system capability before starting the program
 *****************************************************************************/
static uint8_t initCheck(void)
{
	uint8_t CHK_FLAG = 0;

	ESP_LOGI(MTAG,MAIN_SYS_INFO, CONFIG_IDF_TARGET, CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ, CONFIG_SOC_CPU_CORES_NUM);	//Display system information

	ESP_LOGI(MTAG,MAIN_FREE_RAM " / " MAIN_TOTAL_RAM,heap_caps_get_free_size(MALLOC_CAP_INTERNAL), heap_caps_get_total_size(MALLOC_CAP_INTERNAL));								//Display the total amount of DRAM
	ESP_LOGI(MTAG,MAIN_FREE_SPIRAM " / " MAIN_TOTAL_SPIRAM,heap_caps_get_free_size(MALLOC_CAP_SPIRAM), heap_caps_get_total_size(MALLOC_CAP_SPIRAM));								//Display the total amount of PSRAM

	ESP_LOGI(MTAG,MSG_APPNAME);																					//Display app name
	ESP_LOGI(MTAG,MSG_INTRO);																					//Display introduction message
	ESP_LOGI(MTAG,MAIN_TASK_INIT,xPortGetCoreID());																//Display the core that run this task

	ESP_LOGI(MTAG,MAIN_IDF_VER, esp_get_idf_version());															//Display ESP-IDF version
	ESP_LOGI(MTAG,MAIN_CHK_INFO);
/*
 * Check if PSRAM is enabled.
 */
#ifndef CONFIG_SPIRAM
	ESP_LOGE(MTAG,MAIN_ERR_SPIRAM_NOT_EN);
	CHK_FLAG = 1;
#else
	ESP_LOGI(MTAG,MAIN_TOTAL_SPIRAM, heap_caps_get_total_size(MALLOC_CAP_SPIRAM));								//Display the total amount of PSRAM
	ESP_LOGI(MTAG,MAIN_FREE_SPIRAM,heap_caps_get_free_size(MALLOC_CAP_SPIRAM));									//Display the total amount of free PSRAM
#endif


#ifndef CONFIG_SPIRAM_ALLOW_BSS_SEG_EXTERNAL_MEMORY		//Check if variable is allowed to allocate in PSRAM
	ESP_LOGE(MTAG,MAIN_ERR_SPIRAM_BSS_DIS);
	CHK_FLAG = 1;
#endif

/*
 * Check if WiFi is supported in this SOC
 */
#ifndef CONFIG_SOC_WIFI_SUPPORTED
	ESP_LOGE(MTAG,MAIN_ERR_WIFI_NOT_SUP);
	CHK_FLAG = 1;
#endif
/*
 * Check if WiFi and LwIP subsystem are allowed to use PSRAM
 */
#ifndef CONFIG_SPIRAM_TRY_ALLOCATE_WIFI_LWIP
	ESP_LOGE(MTAG,MAIN_ERR_WIFI_SPIRAM_NOT_ALLOW);
	CHK_FLAG = 1;
#endif


#ifndef CONFIG_SOC_RMT_SUPPORTED
	ESP_LOGW(MTAG,LED_ERR_RMT_NOT_SUPPORTED);
	CHK_FLAG = 2;
#endif

#ifndef CONFIG_SOC_GDMA_SUPPORTED
	ESP_LOGW(MTAG,MAIN_WARN_GDMA_NOT_SUPPORTED);
	CHK_FLAG = 2;
#endif

	return CHK_FLAG;
}//End initCheck

