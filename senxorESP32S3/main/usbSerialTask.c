/*****************************************************************************
 * @file     usbSerialTask.c
 * @version  1.00
 * @brief    USB CDC Task
 * @date	 23 Apr 2024
 * @author	 Meridian Innovations
 ******************************************************************************/
#include "usbSerialTask.h"
#include "SenXorLib.h"
#include "class/cdc/cdc_device.h"
#include "projdefs.h"

//public:
extern QueueHandle_t senxorFrameQueue;
TaskHandle_t usbSerialTaskHandle = NULL;						//TCP server handler

//private:
EXT_RAM_BSS_ATTR static uint8_t 	mCDCRxbuf[CONFIG_TINYUSB_CDC_RX_BUFSIZE + 1];
EXT_RAM_BSS_ATTR static uint8_t 	mAckBuff[CONFIG_TINYUSB_CDC_RX_BUFSIZE];			//Buffer for returning data to host
EXT_RAM_BSS_ATTR static uint8_t 	mFrameTxBuff[USB_TX_SIZE];							//Buffer holding the thermal data
EXT_RAM_BSS_ATTR static cmdPhaser	cmdPhaserObj;														//Command phaser object

static uint16_t mAckSize = 0;
static uint16_t mMemcpySize = 0;
static uint16_t mMemcpyOffset = 12;
static uint8_t mTxErr = 0;
static uint16_t mTxSize = USB_TX_SIZE;
static uint16_t mTxSizeRemain = USB_TX_SIZE % CONFIG_TINYUSB_CDC_TX_BUFSIZE;
static uint16_t mTxPacketSize = 0;

static void usbSerialTask_Init(void);

/*
 * ***********************************************************************
 * @brief       tinyusb_cdc_rx_callback
 * @param       itf - CDC port
 * 				event - CDC event
 * @return      None
 * @details     RX callback
 **************************************************************************/
static void tinyusb_cdc_rx_callback(int itf, cdcacm_event_t *event)
{
    /* initialization */
    size_t rx_size = 0;


#if CONFIG_MI_EVK_USB_RX_DBG
    esp_err_t ret = tinyusb_cdcacm_read(itf, mCDCRxbuf, CONFIG_TINYUSB_CDC_RX_BUFSIZE, &rx_size);
   
	if (ret == ESP_OK) 
    {
        ESP_LOGI(USBTaskTAG, "Data from channel %d:", itf);
        ESP_LOG_BUFFER_HEXDUMP(USBTaskTAG, mCDCRxbuf, rx_size, ESP_LOG_INFO);
		ESP_LOGI(USBTaskTAG, "Size =  %d", rx_size);
    } 
    else 
    {
        ESP_LOGE(USBTaskTAG, "Read error");
    }
#else 
	tinyusb_cdcacm_read(itf, mCDCRxbuf, CONFIG_TINYUSB_CDC_RX_BUFSIZE, &rx_size);		// Read regardless of the result
#endif

	/*
	*	The minimum length of the command is 12 bytes
	*	4 bytes - Delimiter
	*	4 bytes - Command
	*	variable bytes - Data
	*	4 bytes - CRC
	*	Therefore any data received from USB less than 12 bytes is rejected
	*	alongside missing the delimiter character 'X' for data longer than 12 bytes
	*/
	if(rx_size > 12)
	{
		cmdParser_PharseCmd(&cmdPhaserObj, mCDCRxbuf, rx_size);
		mAckSize = cmdParser_CommitCmd(&cmdPhaserObj, mAckBuff);
		if(mAckSize != 0)
		{
			tinyusb_cdcacm_write_queue(itf, mAckBuff, mAckSize);
#if CONFIG_MI_EVK_USB_RX_DBG
			ESP_LOGI(USBTaskTAG, "mAckBuff %s:", mAckBuff);
#endif
			tinyusb_cdcacm_write_flush(itf, 0);			// No blocking here
		}
		cmdParser_Init(&cmdPhaserObj);
	}
	else 
	{
		ESP_LOGW(USBTaskTAG, USBTASK_ERR_RX_LEN_TOO_SHORT);
	}
	

}// tinyusb_cdc_rx_callback

/*
 * ***********************************************************************
 * @brief       usbSerialTask
 * @param       pvParameters - Task arguments
 * @return      None
 * @details     USB Serial Task
 **************************************************************************/
void usbSerialTask(void * pvParameters)
{
	esp_err_t tEspErr = ESP_OK;
    senxorFrame *pSenxorFrameRecObj;
    usbSerialTask_Init();
	
    for(;;)
    {
        if(xQueueReceive(senxorFrameQueue, &pSenxorFrameRecObj, portMAX_DELAY))
        {
			memcpy(mFrameTxBuff+mMemcpyOffset, pSenxorFrameRecObj->mFrame, mMemcpySize*sizeof(uint16_t));		//Copy received object to buffer
			sprintf((char *)&mFrameTxBuff[mTxSize - 4], "%04X", getCRC(mFrameTxBuff+4,mTxSize-4));
			
			tinyusb_cdcacm_write_queue(TINYUSB_CDC_ACM_0,mFrameTxBuff,mTxPacketSize);
			tEspErr = tinyusb_cdcacm_write_flush(TINYUSB_CDC_ACM_0, pdMS_TO_TICKS(5000));			// Give 100ms for transfer
			if(tEspErr != ESP_OK)
			{
				ESP_LOGE(USBTaskTAG, USBTASK_ERR_FLUSH_BUFF, esp_err_to_name(tEspErr));
				++mTxErr;
			}// End if
				
			if(mTxSizeRemain > 0)
			{
				tinyusb_cdcacm_write_queue(TINYUSB_CDC_ACM_0,mFrameTxBuff+mTxPacketSize,mTxSizeRemain);
				tEspErr = tinyusb_cdcacm_write_flush(TINYUSB_CDC_ACM_0, pdMS_TO_TICKS(5000));			// Give 100ms for transfer
				if(tEspErr != ESP_OK)
				{
					ESP_LOGI(USBTaskTAG,USBTASK_INFO_REMAIN_BYTES, mTxSizeRemain);
					ESP_LOGE(USBTaskTAG, USBTASK_ERR_FLUSH_BUFF, esp_err_to_name(tEspErr));
					++mTxErr;
				}// End if
			}  // End if
        }// End if
		if(mTxErr == 5)
		{
			ESP_LOGE(USBTaskTAG,USBTASK_ERR_STOP_CAP);
			Acces_Write_Reg(0xB0,0);
			mTxErr = 0;
		}
        vTaskDelay(1);
    }// End for
}// usbSerialTask

/*
 * ***********************************************************************
 * @brief       usbSerialTask_Init
 * @param       None
 * @return      None
 * @details     Initialise Task
 **************************************************************************/
static void usbSerialTask_Init(void)
{
    tinyusb_config_cdcacm_t acmCfgObj = {0};
    acmCfgObj.usb_dev = TINYUSB_USBDEV_0;
    acmCfgObj.cdc_port = TINYUSB_CDC_ACM_0;
    acmCfgObj.callback_rx = &tinyusb_cdc_rx_callback;

    Drv_USB_CDC_Init(&acmCfgObj);
	cmdParser_Init(&cmdPhaserObj);
	ESP_LOGI(USBTaskTAG, USBTASK_INFO_INIT, xPortGetCoreID());
	ESP_LOGI(USBTaskTAG,MAIN_FREE_RAM " / " MAIN_TOTAL_RAM,heap_caps_get_free_size(MALLOC_CAP_INTERNAL), heap_caps_get_total_size(MALLOC_CAP_INTERNAL));								//Display the total amount of DRAM
	ESP_LOGI(USBTaskTAG,MAIN_FREE_SPIRAM " / " MAIN_TOTAL_SPIRAM,heap_caps_get_free_size(MALLOC_CAP_SPIRAM), heap_caps_get_total_size(MALLOC_CAP_SPIRAM));								//Display the total amount of PSRAM
	usbSerialTask_InitThermalBuff();
}// usbSerialTask_Init

 /******************************************************************************
 * @brief       usbSerialTask_InitThermalBuff
 * @param       pMi48 - MI48 Object
 * @return      none
 * @details     Initialise thermal buffer for USB
 *****************************************************************************/
void usbSerialTask_InitThermalBuff(void)
{
	memset(mAckBuff,0,CONFIG_TINYUSB_CDC_RX_BUFSIZE);
	memset(mFrameTxBuff,0,USB_TX_SIZE);                         // Initialise Frame buffers
	mMemcpySize = 80*64;

	mTxSize = 10256;
	mTxPacketSize = mTxSize; 
	mMemcpyOffset = 12+80*2; 				//(80 words + 4 words)
	
	// Using 10248 (0x2808) byte packet
	mFrameTxBuff[0]=' ';
	mFrameTxBuff[1]=' ';
	mFrameTxBuff[2]=' ';
	mFrameTxBuff[3]='#';
	mFrameTxBuff[4]='2';
	mFrameTxBuff[5]='8';
	mFrameTxBuff[6]='0';
	mFrameTxBuff[7]='8';
	mFrameTxBuff[8]='G';
	mFrameTxBuff[9]='F';
	mFrameTxBuff[10]='R';
	mFrameTxBuff[11]='A';
	// CRC
	mFrameTxBuff[mTxSize-4]='X';
	mFrameTxBuff[mTxSize-3]='X';
	mFrameTxBuff[mTxSize-2]='X';
	mFrameTxBuff[mTxSize-1]='X';

	// Determine if the frame size exceed the limit of CDC buffer size
	if(mTxSize > CONFIG_TINYUSB_CDC_TX_BUFSIZE)
	{
		mTxSizeRemain = mTxSize%CONFIG_TINYUSB_CDC_TX_BUFSIZE;				// Determine the remaining bytes
		mTxPacketSize = CONFIG_TINYUSB_CDC_TX_BUFSIZE;							// Limiting the transfer size
	}
	else
	{
		mTxSizeRemain = 0;
	}// End if-else
				
}// usbSerialTask_InitThermalBuff
