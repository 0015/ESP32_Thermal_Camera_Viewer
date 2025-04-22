/*****************************************************************************
 * @file     senxorTask.h
 * @version  2.01
 * @brief    Header file for senxorTask.c
 * @date	 21 Jul 2022
 ******************************************************************************/
#ifndef MAIN_INCLUDE_SENXORTASK_H_
#define MAIN_INCLUDE_SENXORTASK_H_
#include "esp_log.h"					//ESP logger
#include <stdlib.h>
#include <stdbool.h>

#include "MCU_Dependent.h"
#include "msg.h"						//Messages
#include "SenXorLib.h"					//Using SenXor library
#include "restServer.h"

#define SENXOR_TASK_STACK_SIZE	4096	//Task stack size
#define THERMAL_FRAME_BUFFER_NO 3		//Queue size


typedef struct senxorFrame{
	uint16_t mFrame[80*63];
}senxorFrame;


uint8_t senxorInit(void);

void senxorTask(void * pvParameters);

#endif /* MAIN_INCLUDE_SENXORTASK_H_ */
