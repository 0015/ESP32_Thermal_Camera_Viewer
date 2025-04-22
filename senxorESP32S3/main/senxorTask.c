/*****************************************************************************
 * @file     senxorTask.c
 * @version  2.01
 * @brief    FreeRTOS task for interfacing with SenXor
 * @date	 11 Jul 2022
 ******************************************************************************/
#include <esp_log.h>				//ESP logger
#include "Customer_Interface.h"
#include "DrvLED.h"
#include "MCU_Dependent.h"
#include "esp_err.h"
#include "esp_mac.h"
#include "esp_system.h"
#include "msg.h"					//Messages
#include "SenXorLib.h"				//SenXor library
#include "SenXor_Capturedata.h"		//Interrupt handler
#include "senxorTask.h"
#include "tcpServerTask.h"
#include "util.h"
#include "ledCtrlTask.h"			//LED control task

//public:
EXT_RAM_BSS_ATTR uint16_t CalibData_BufferData[CALIBDATA_FLASH_SIZE];			//Array to hold the calibration data
EXT_RAM_BSS_ATTR QueueHandle_t senxorFrameQueue = NULL;

TaskHandle_t senxorTaskHandle = NULL;
//private:
EXT_RAM_BSS_ATTR static senxorFrame mSenxorFrameObj;
static void senxorTask_Init(void);


/*
 * ***********************************************************************
 * @brief       senxorInit
 * @param       None
 * @return      None
 * @details     Initialise SenXor
 **************************************************************************/
uint8_t senxorInit(void)
{

	Initialize_McuRegister();																//Initialise SenXor software registers

	Power_On_Senxor(1);																//Power up SenXor and determine its model

	if(Initialize_SenXor(1))													//Initialise SenXor peripherals.
	{
#ifdef CONFIG_MI_LCD_EN
		drawIcon(96,76,ICON_ERROR);
		drawText(1,76+48,"Failed to initialise \nSenXor. Program halted.");
#endif
		return 1; 																			//Exit program immediately if failed.
	}//End if

	Read_CalibrationData();																	//Load calibration data from flash
	ESP_LOGI(SXRTAG,SXR_PROCESS_CALI);
	Process_CalibrationData(1,(uint16_t*)CalibData_BufferData);		//Process calibration data
	ESP_LOGI(SXRTAG,SXR_FITLER_INIT);
	Initialize_Filter();																	//Initialise filters
	Read_AGC_LUT();																			//Read auto gain

	/*
	 * If TCP server is used, TCP server should be started BEFORE SenXor starting capture.
	 */
	Acces_Write_Reg(0xB1,0);													//Stop capturing

	ESP_LOGI(SXRTAG,SXR_INIT_DONE);

	return 0;
}

/*
 * ***********************************************************************
 * @brief       senxorTask
 * @param       pvParameters - Task arguments
 * @return      None
 * @details     SenXor task
 **************************************************************************/
void senxorTask(void * pvParameters)
{
	//uint8_t tOpMode = MCU_getOpMode();
	ESP_LOGI(SXRTAG,SXR_TASK_INFO,xPortGetCoreID());
	ESP_LOGI(SXRTAG,MAIN_FREE_RAM " / " MAIN_TOTAL_RAM,heap_caps_get_free_size(MALLOC_CAP_INTERNAL), heap_caps_get_total_size(MALLOC_CAP_INTERNAL));				//Display the total amount of DRAM
	ESP_LOGI(SXRTAG,MAIN_FREE_SPIRAM " / " MAIN_TOTAL_SPIRAM,heap_caps_get_free_size(MALLOC_CAP_SPIRAM), heap_caps_get_total_size(MALLOC_CAP_SPIRAM));				//Display the total amount of PSRAM

	senxorTask_Init();																																					//Initialise queue
	for(;;)
	{
		if ( (Acces_Read_Reg(0xB1) & B1_SINGLE_CONT) || (Acces_Read_Reg(0xB1) & B1_START_CAPTURE) )
		{
			DataFrameReceiveSenxor();													//Receive frame from SenXor
			const uint16_t* senxorData = DataFrameGetPointer();							//Get processed frame

			if (senxorData != 0)
			{
#ifdef CONFIG_MI_SENXOR_DBG
				printSenXorLog(senxorData);
#endif
				memcpy(mSenxorFrameObj.mFrame,senxorData,sizeof(mSenxorFrameObj.mFrame));	//Get a copy of thermal frame
				senxorFrame* pSenxorFrameObj = &mSenxorFrameObj;							//Create a pointer to the copy of thermal frame  for sending to queue
				if(tcpServerGetIsClientConnected())
				{
					xQueueSend(senxorFrameQueue, (void *)&pSenxorFrameObj, 0);										//Send to queue and do not wait for queue
				}

			}//End if
			DataFrameProcess();															//Thermal frame post-processing
		}//End if

		vTaskDelay(1);																	//A tiny delay to feed the watch dog
	}//End for
}//End senxorTask


/*
 * ***********************************************************************
 * @brief       DataFrameReceiveError
 * @param       None
 * @return      None
 * @details     Frame receive error handler
 **************************************************************************/
void DataFrameReceiveError(void)
{
	if(SenXorError)
    {
	  ESP_LOGE(SXRTAG,SXR_ERR,SenXorError);
	  ESP_LOGW(SXRTAG,SXR_WARN_RECR);
      SenXorError = 0;                                                    //Clear error flag
      Acces_Write_Reg(0xB1,0);                                            //Stop capturing
      Acces_Write_Reg(0xB0,3);                                            //Reinitialise SenXor
    }//End if
}


/*
 * ***********************************************************************
 * @brief       senxorTask_Init
 * @param       None
 * @return      None
 * @details     Initialise SenXorTask
 **************************************************************************/
static void senxorTask_Init(void)
{
	ESP_LOGI(MTAG,MAIN_INIT_QUEUE);

	senxorFrameQueue = xQueueCreate(THERMAL_FRAME_BUFFER_NO, sizeof(uint16_t*));
	if( senxorFrameQueue == 0 )
	{
		vTaskDelete(NULL);
	}

}
