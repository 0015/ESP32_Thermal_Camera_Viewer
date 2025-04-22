#ifndef __IMAGEPROCESSINGLIB_H__
#define __IMAGEPROCESSINGLIB_H__
#include <stdint.h>
#include "defines.h"

void MEDIAN_ImagePRocessing(uint16_t* ProcessDataFrame_TXBuf,uint16_t l_FrameSize) ;
void MEDIAN_Initialize (uint8_t MedianFilterEnable  ,uint8_t MkernelSize ,uint16_t* Filter_result);
void MedianFilter(uint16_t* current, int l_FrameSize, int kernel_size, uint8_t SenXorModel);

void STARK_Initialize(uint8_t Stark_Control,uint8_t FilterControl, uint8_t STARK_Setting ,uint8_t STARK_grad, uint8_t STARK_scale, uint8_t kernelSize,uint16_t* Filter_result, uint16_t* STARKbuff);
void STARK_ImagePRocessing(uint16_t* buffer, int l_FrameSize,  uint16_t *Frame_min, uint16_t *Frame_max, uint16_t Tgain, uint16_t module_type);

void KXMS_stabilizer(uint16_t* arr, int l_FrameSize,  uint16_t *Frame_min, uint16_t* Frame_max);
void KXMS_Initialize(uint8_t KXMS_used, uint8_t Roll_used);
#endif// __IMAGEPROCESSINGLIB_H__
