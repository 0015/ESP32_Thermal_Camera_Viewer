#ifndef MAIN_INCLUDE_USBSERIALTASK_H_
#define MAIN_INCLUDE_USBSERIALTASK_H_

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "DrvUSB.h"
#include "senxorTask.h"
#include "cmdParser.h"

#define USB_TX_SIZE                             4+4+4+(160+80+80*63)*2+4                                               // Panther requires 39696 bytes 
#define USB_TASK_STACK_SIZE                     4096
#define USB_TX_PACKET_SIZE                      CONFIG_TINYUSB_CDC_TX_BUFSIZE / 2

// Debug message
#define USBTaskTAG 							    "[USB_TASK]"

#define USBTASK_ERR_FLUSH_BUFF                  "Error while flusing buffer: %s "
#define USBTASK_ERR_STOP_CAP                    "Maximum retry attempt reached, stopping capture..."
#define USBTASK_ERR_RX                          "Read error upon receive from USB."
#define USBTASK_ERR_TASK_FAIL_INIT				"USBTask task failed to initialised. USB function will not be avaliable."
#define USBTASK_ERR_TASK_QUEUE_INIT_FAIL		"Cannot allocate queue for MI48 task. USB function will not be avaliable."
#define USBTASK_ERR_RX_LEN_TOO_SHORT            "Invalid EVK command."
#define USBTASK_INFO_REMAIN_BYTES               "Sending remaining %d bytes"
#define USBTASK_INFO_INIT						"USB Task initialising... Running on Core %d."
#define USBTASK_INFO_TASK_RESUME                "USB Task resumed."



void usbSerialTask(void * pvParameters);

void usbSerialTask_InitThermalBuff(void);

#endif
