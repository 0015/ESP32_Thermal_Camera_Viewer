///**************************************************************************//**
// * @file     Senxor_Capturedata.c
// * @version  V1.00
// * @brief    capture IR Frame
// *
// * @copyright (C) 2018 Meridian Innovation
// ******************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "DrvLED.h"
#include "DrvSPIHost.h"
#include "MCU_Dependent.h"
#include "SenXorLib.h"
#include "Senxor_Flash.h"
#include "Senxor_Capturedata.h"
#include "defines.h"
#include "portmacro.h"


extern void IRAM_ATTR GetReceiveFrameBuffer();
extern void CaptureProcessFrame(uint16_t tmp);

/******************************************************************************
 * @brief       Data_AV_FIFO_Int_Handler
 * @param       none
 * @return      None
 * @details     GPIO Interrupt handler. This function is pre-loaded into instruction
 *  			RAM to speed up the operation.
 *****************************************************************************/
#define BURST_MODE 8							//maximum of allowed write to MCU SPI (MCU FIFO Size)
void IRAM_ATTR Data_AV_FIFO_Int_Handler(void* arg)
{

	uint8_t Burstmode = BURST_MODE;

	GetReceiveFrameBuffer();
//	Drv_SPI_DMA_Enable();

	if (DATA_AV_Threshold < BURST_MODE)
	{
		Burstmode = 1;	// Check on illegal burst mode.
	}

	
	for (uint16_t byte_cnt = 0; byte_cnt<(DATA_AV_Threshold/Burstmode); byte_cnt++)
	{
		for(uint16_t Bcnt = 0; Bcnt<Burstmode; Bcnt++)
		{
			ReceiveFrame->TXBuf[PixelCnt++] = Drv_SPI_Transmit(); //Drv_SPI_DMA_Transmit(); ;
		}

	}//End for
#if CONFIG_MI_LED_EN
	Drv_LED_Gpio_En(LED_PIN_G , LED_OFF);
#endif
	if (PixelCnt == FrameSize) // Check if end of frame
	{
#if CONFIG_MI_LED_EN
		Drv_LED_Gpio_En(LED_PIN_G, LED_ON);
#endif	
		Drv_SPI_DMA_Disable();
		CaptureProcessFrame(ReceiveFrame->TXBuf[PixelCnt-1]);
	}//End if

}//Data AV handler (FIFO) END
