/**************************************************************************//**
 * @file     Customer_Interface.c
 * @version  V1.00
 * @brief    Customer Interface of the frame
 *
 * @copyright (C) 2018 Meridian Innovation
 ******************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "MCU_Dependent.h"
#include "SenXorLib.h"
#include "SenXor_FLASH.h"
#include "Customer_Interface.h"
#ifdef WITH_TOF_VL53L1
#include "tof_vl53l1_user.h"
#endif
#include "defines.h"
#include "version.h"
#include "imageProcessingLib.h"

extern MCU_REG MCU_REGISTER;
extern MERGE MergeBuffer;
CUST_REG App_Register;
#define V6M_AIRCR_VECTKEY_DATA      0x05FA0000UL
#define V6M_AIRCR_SYSRESETREQ       0x00000004UL
ImageProcessing Image_Processing;
#ifdef MEDIAN_STARK
uint16_t* STARKBuff;
uint16_t* result;
// uint16_t STARKBuff[FILTER_BUFFER];
// uint16_t result[FILTER_BUFFER];
#endif
uint16_t minTemp;
uint16_t maxTemp;
uint8_t kernelSize=1;
uint8_t MkernelSize=3;

union FP16_union {
  float           fp;            /* Floating-point value */
  unsigned long   ul;           /* Unsigned long value */
};

/******************************************************************************
 * @brief       Application_Version
 * @param       None
 * @return      None
 * @details     Read version.
 *****************************************************************************/
int ApplicationReadVersion (int Address)
{
	switch (Address) 
	{
		case 0xB2:
		return Application_Version_Major<<4 | (Application_Version_Minor &0x0F);
		case 0xB3:
		return Application_Version_Build;
		default:
		return 0xF3;
	}
}
/******************************************************************************
 * @brief       Customer_Interface_Read_Registers
 * @param       None
 * @return      None
 * @details     Read Customer register.
 *****************************************************************************/
int Customer_Interface_Read_Registers (int Address)
{
	int tmpbyte = 0xFF;

	switch (Address) 
	{
		case 0x00:
		return App_Register.REG_00.Setting;
		case 0x01:
		return App_Register.REG_01.Setting;
		case 0x20:
		return Image_Processing.MCU_REG_20.Setting;
		case 0x21:
		return Image_Processing.MCU_REG_21.Setting;
		case 0x22:
		return Image_Processing.MCU_REG_22.Setting;
		case 0x23:
		return Image_Processing.MCU_REG_23.Setting;
		case 0x25:
		return Image_Processing.MCU_REG_25.Setting;
		case 0x30:
		return Image_Processing.MCU_REG_30.Setting;
		case 0x31:
		return Image_Processing.MCU_REG_31.Setting;
		default:
		return 0xFF;
	}
}

/******************************************************************************
 * @brief       Customer_Interface_Write_Registers
 * @param       None
 * @return      None
 * @details     Write Customer register.
 *****************************************************************************/
void Customer_Interface_Write_Registers (int Address,int Data)
{


	switch (Address)
	{
		case 0x0:
			App_Register.REG_00.Setting=Data;
			if (App_Register.REG_00.Set.Mcu_Reset == 1)
			{

			}
			break;
		case 0x01:
			App_Register.REG_01.Setting=Data;

		   if(App_Register.REG_01.Set.Stop_Host_Transfer ==1)
		   {
			   Drv_SPI_Host_PDMA_Disable();
			   App_Register.REG_01.Set.Stop_Host_Transfer=0;
		   }

			break;
#ifdef MEDIAN_STARK
		case 0x20:
			Image_Processing.MCU_REG_20.Setting=Data;
			if(Image_Processing.MCU_REG_20.GetSet.Kernel_size == 1)
			{
				kernelSize = 2;
			}
			else
			{
				kernelSize = 1;
			}
			STARK_Initialize(Image_Processing.MCU_REG_20.Setting,Image_Processing.MCU_REG_30.Setting,Image_Processing.MCU_REG_21.Setting,Image_Processing.MCU_REG_22.Setting,Image_Processing.MCU_REG_23.Setting, kernelSize,result, STARKBuff);
			break;
		case 0x21:
			Image_Processing.MCU_REG_21.Setting=Data;
			STARK_Initialize(Image_Processing.MCU_REG_20.Setting,Image_Processing.MCU_REG_30.Setting,Image_Processing.MCU_REG_21.Setting,Image_Processing.MCU_REG_22.Setting,Image_Processing.MCU_REG_23.Setting, kernelSize,result, STARKBuff);

			break;
		case 0x22:
			Image_Processing.MCU_REG_22.Setting=Data;
			STARK_Initialize(Image_Processing.MCU_REG_20.Setting,Image_Processing.MCU_REG_30.Setting,Image_Processing.MCU_REG_21.Setting,Image_Processing.MCU_REG_22.Setting, Image_Processing.MCU_REG_23.Setting, kernelSize,result, STARKBuff);
			break;
		case 0x25:
			Image_Processing.MCU_REG_25.Setting=Data;
			KXMS_Initialize(Image_Processing.MCU_REG_25.GetSet.KXMS, Image_Processing.MCU_REG_25.GetSet.Roll);
			break;
		case 0x23:
			Image_Processing.MCU_REG_23.Setting=Data;
			STARK_Initialize(Image_Processing.MCU_REG_20.Setting,Image_Processing.MCU_REG_30.Setting,Image_Processing.MCU_REG_21.Setting,Image_Processing.MCU_REG_22.Setting,Image_Processing.MCU_REG_23.Setting, kernelSize,result, STARKBuff);
			break;
		case 0x30:
			Image_Processing.MCU_REG_30.Setting=Data;
			if(Image_Processing.MCU_REG_30.GetSet.MedianFilterSize == 1)
			{
				MkernelSize = 5;
			}
			else
			{
				MkernelSize = 3;
			}
			if(Image_Processing.MCU_REG_20.GetSet.Kernel_size == 1)
			{
				kernelSize = 2;
			}
			else
			{
				kernelSize = 1;
			}
			MEDIAN_Initialize(Image_Processing.MCU_REG_30.GetSet.MedianFilterEnable,MkernelSize,result);
			break;
#endif
		case 0x31:
			Image_Processing.MCU_REG_31.Setting=Data;
			break;

		default:
			break;

	}

}

/******************************************************************************
 * @brief       Initialize_Filter
 * @param       None
 * @return
 * @details     This Function initial image register during power on
 *****************************************************************************/
void Initialize_Filter(void)
{
#ifdef MEDIAN_STARK

	STARKBuff = heap_caps_malloc(FILTER_BUFFER*sizeof(uint16_t), MALLOC_CAP_SPIRAM);
	if(STARKBuff == 0 )
	{
		free(STARKBuff);
		return;
	}

	result = heap_caps_malloc(FILTER_BUFFER*sizeof(uint16_t), MALLOC_CAP_SPIRAM);
	if(result == 0 )
	{
		free(result);
		return;
	}

	 // STARK
	Acces_Write_Reg(0xB4, 3); 							// avg value set to 3
	Acces_Write_Reg(0xD0, 3); 							// ENABLE kalman filter
	Acces_Write_Reg(0xD1, 10); 							// ENABLE kalman filter
	Acces_Write_Reg(0xD2, 0); 							// ENABLE kalman filter

	Image_Processing.MCU_REG_20.GetSet.STARK_Enable =1; // enable stark
	Image_Processing.MCU_REG_20.GetSet.STARK_auto=1;
	Image_Processing.MCU_REG_23.GetSet.STARK_scale=80;
	Image_Processing.MCU_REG_25.Setting=0;
	Image_Processing.MCU_REG_25.GetSet.KXMS=1;

	Image_Processing.MCU_REG_30.GetSet.MedianFilterEnable=0;
	Image_Processing.MCU_REG_30.GetSet.MedianFilterSize=0; // starck which
	Image_Processing.MCU_REG_21.GetSet.STARK_cuttoff=10; // stark cuttoff default is 1
	Image_Processing.MCU_REG_22.GetSet.STARK_grad=10; // stark grad default is 1
	if(Image_Processing.MCU_REG_30.GetSet.MedianFilterSize == 1)
	{
		MkernelSize = 5;
	}
	else
	{
		MkernelSize = 3;
	}
	if(Image_Processing.MCU_REG_20.GetSet.Kernel_size == 1)
	{
		kernelSize = 2;
	}
	else
	{
		kernelSize = 1;
	}
	STARK_Initialize(Image_Processing.MCU_REG_20.Setting,Image_Processing.MCU_REG_30.Setting,Image_Processing.MCU_REG_21.Setting,Image_Processing.MCU_REG_22.Setting, Image_Processing.MCU_REG_23.Setting, kernelSize,result, STARKBuff);

	MEDIAN_Initialize(Image_Processing.MCU_REG_30.GetSet.MedianFilterEnable,MkernelSize,result);

	KXMS_Initialize(Image_Processing.MCU_REG_25.GetSet.KXMS, Image_Processing.MCU_REG_25.GetSet.Roll);
#endif

}




/******************************************************************************
 * @brief       Update min max header
 * @param       uint16_t* buffer, framesize
 * @return      NONE
 * @details     this Function will be called from custumer interface to update min max header
 *****************************************************************************/
void COnvert_Image_Transfer_Format(uint16_t* buffer, int l_FrameSize)
{
	uint16_t i = 0;
	uint16_t minT = 0xFFFF;
	uint16_t maxT = 0;
	uint16_t temp ;
  	float temp_res_divide=10 ;

	union FP16_union temp_data_f16 ;
	float temp_data ;

	float Celsius_Scale = 273.15;
	uint16_t Tgain = Acces_Read_Reg(0xB9);

	if((Tgain&0x80) == 0x80)
	{
		temp_res_divide=100 ;
	}
	switch(Image_Processing.MCU_REG_31.Setting)
	{
		case 1:
			for (i=0; i<l_FrameSize; i++)
			{
				temp_data = buffer[i]/temp_res_divide;
				temp_data = (temp_data - Celsius_Scale)*temp_res_divide;
				buffer[i]= (int)temp_data;
			}
			temp_data = minTemp/temp_res_divide;  					//min  pixel
			temp_data = (temp_data - Celsius_Scale)*temp_res_divide;
			minTemp= (int)temp_data;

			temp_data = maxTemp/temp_res_divide;  					//max  pixel
			temp_data = (temp_data - Celsius_Scale)*temp_res_divide;
			maxTemp= (int)temp_data;



		break;
		case 2:
			for (i=0; i<l_FrameSize; i++)
			{
				temp_data = buffer[i]/temp_res_divide;
				temp_data = (float)(((temp_data - Celsius_Scale) * 9/5) + 32)*temp_res_divide;
				buffer[i]= (int)temp_data;
			}
			temp_data = minTemp/temp_res_divide;  					//min  pixel
			temp_data = (float)(((temp_data - Celsius_Scale) * 9/5) + 32)*temp_res_divide;
			minTemp= (int)temp_data;

			temp_data = maxTemp/temp_res_divide;					//max  pixel
			temp_data = (float)(((temp_data - Celsius_Scale) * 9/5) + 32)*temp_res_divide;
			maxTemp= (int)temp_data;
		break;
		case 4:
			for (i=0; i<l_FrameSize; i++)
			{

				temp_data_f16.fp = buffer[i]/temp_res_divide;

				buffer[i]= (uint16_t)temp_data_f16.ul;
			}
			temp_data_f16.fp = minTemp/temp_res_divide;  					//min  pixel
			minTemp= (uint16_t)temp_data_f16.ul;

			temp_data_f16.fp = maxTemp/temp_res_divide;					//max  pixel
			maxTemp= (uint16_t)temp_data_f16.ul;

		break;
		case 5:
			for (i=0; i<l_FrameSize; i++)
			{
				temp_data = buffer[i]/temp_res_divide;
				temp_data_f16.fp = (temp_data - Celsius_Scale);
				buffer[i]= (uint16_t)temp_data_f16.ul;
			}

			temp_data = minTemp/temp_res_divide;  					//min  pixel
			temp_data_f16.fp = (temp_data - Celsius_Scale);
			minTemp= (uint16_t)temp_data_f16.ul;

			temp_data = maxTemp/temp_res_divide;					//max  pixel
			temp_data_f16.fp = (temp_data - Celsius_Scale);
			maxTemp= (uint16_t)temp_data_f16.ul;

		break;
		case 6:
			for (i=0; i<l_FrameSize; i++)
			{

				temp_data = buffer[i]/10.0;
				temp_data_f16.fp = (float)(((temp_data - Celsius_Scale) * 9/5) + 32);
				buffer[i]= (uint16_t)temp_data_f16.ul;
			}
			temp_data = minTemp/10.0;  					//min  pixel
			temp_data_f16.fp = (float)(((temp_data - Celsius_Scale) * 9/5) + 32);
			minTemp= (uint16_t)temp_data_f16.ul;

			temp_data = maxTemp/10.0;						//max  pixel
			temp_data_f16.fp = (float)(((temp_data - Celsius_Scale) * 9/5) + 32);
			maxTemp= (uint16_t)temp_data_f16.ul;
		break;



	}

}



/******************************************************************************
 * @brief       Update min max header
 * @param       uint16_t* buffer, framesize
 * @return      NONE
 * @details     this Function will be called from custumer interface to update min max header
 *****************************************************************************/
void COnvert_Image_Transfer_Format_header(uint16_t* ProcessDataFrame_TXBuf)

{
	uint16_t i = 0;

  	float temp_res_divide=10 ;
	union FP16_union temp_data_f16 ;
	float temp_data ;
	float Celsius_Scale = 273.15;
	uint16_t Tgain = Acces_Read_Reg(0xB9);

	if((Tgain&0x80) == 0x80)
	{
		temp_res_divide=100 ;
	}
	switch(Image_Processing.MCU_REG_31.Setting)
	{
		case 1:
			temp_data=	ProcessDataFrame_TXBuf[2]/100.0;				//Intern chip temperature
			temp_data = (temp_data - Celsius_Scale)*100.0;
			ProcessDataFrame_TXBuf[2]= (int)temp_data;

//			temp_data=	ProcessDataFrame_TXBuf[9]/temp_res_divide;	 //single pixel in header
	//		temp_data = (temp_data - Celsius_Scale)*temp_res_divide;
//			ProcessDataFrame_TXBuf[9]= (int)temp_data;


		break;
		case 2:
			temp_data=	ProcessDataFrame_TXBuf[2]/100.0;
			temp_data = (float)(((temp_data - Celsius_Scale) * 9/5) + 32)*100.0;
			ProcessDataFrame_TXBuf[2]= (int)temp_data;
//			temp_data=	ProcessDataFrame_TXBuf[9]/temp_res_divide;
//			temp_data = (float)(((temp_data - Celsius_Scale) * 9/5) + 32)*temp_res_divide;
//			ProcessDataFrame_TXBuf[9]= (uint16_t)temp_data;

		break;
		case 4:
			temp_data=	ProcessDataFrame_TXBuf[2]/100.0;				//Intern chip temperature
			temp_data_f16.fp = temp_data;
			ProcessDataFrame_TXBuf[2]= (uint16_t)temp_data_f16.ul;

//			temp_data=	ProcessDataFrame_TXBuf[9]/temp_res_divide;  //single pixel in header
//			temp_data_f16.fp = temp_data;
//			ProcessDataFrame_TXBuf[9]= (uint16_t)temp_data_f16.ul;

		break;
		case 5:
			temp_data=	ProcessDataFrame_TXBuf[2]/100.0;				//Intern chip temperature
			temp_data_f16.fp = (temp_data - Celsius_Scale);
			ProcessDataFrame_TXBuf[2]= (uint16_t)temp_data_f16.ul;

//			temp_data=	ProcessDataFrame_TXBuf[9]/temp_res_divide;	//single pixel in header
//			temp_data_f16.fp = (temp_data - Celsius_Scale);
//			ProcessDataFrame_TXBuf[9]= (uint16_t)temp_data_f16.ul;

		break;
		case 6:
			temp_data=	ProcessDataFrame_TXBuf[2]/100.0;				//Intern chip temperature
			temp_data_f16.fp = (float)(((temp_data - Celsius_Scale) * 9/5) + 32);
			ProcessDataFrame_TXBuf[2]= (uint16_t)temp_data_f16.ul;

//			temp_data=	ProcessDataFrame_TXBuf[9]/temp_res_divide;   //single pixel in header
//			temp_data_f16.fp = (float)(((temp_data - Celsius_Scale) * 9/5) + 32);
//			ProcessDataFrame_TXBuf[9]= (uint16_t)temp_data_f16.ul;
		break;



	}


}
/******************************************************************************
 * @brief       Update min max header
 * @param       uint16_t* buffer, framesize
 * @return      NONE
 * @details     this Function will be called from custumer interface to update min max header
 *****************************************************************************/
void Update_min_max_header(uint16_t* buffer, int l_FrameSize)
{
	uint16_t i = 0;
	uint16_t minT = 0xFFFF;
	uint16_t maxT = 0;
	uint16_t temp;

	if(Image_Processing.MCU_REG_30.GetSet.MedianFilterEnable ==1)
	{
		for (i=0; i<l_FrameSize; i++)
		{
			temp = buffer[i];
			if(minT > temp)
			{
				minT = temp;
			}
			if(maxT < temp)
			{
				maxT = temp;
			}
		}
		minTemp = minT;
		maxTemp = maxT;
	}

}

/******************************************************************************
 * @brief       Customer_imageprocessing
 * @param       uint16_t* buffer, framesize
 * @return      NONE
 * @details     this Function will be called from libray to allow image processing outside the library.
 *****************************************************************************/
void Customer_imageprocessing(uint16_t* buffer, int l_FrameSize, uint16_t Frame_min, uint16_t Frame_max)
{
#ifdef MEDIAN_STARK

	uint16_t Tgain = Acces_Read_Reg(0xB9);
	uint16_t module_type =  Acces_Read_Reg(0xBB);
	uint16_t F_FrameSize= l_FrameSize-FRAMEWIDTH_BUF;
	minTemp = Frame_min;
	maxTemp = Frame_max;

	STARK_ImagePRocessing(buffer,F_FrameSize, &minTemp, &maxTemp, Tgain, module_type);
	MEDIAN_ImagePRocessing(buffer,F_FrameSize);
	KXMS_stabilizer(buffer, F_FrameSize, &minTemp, &maxTemp);
	COnvert_Image_Transfer_Format(buffer, F_FrameSize);

	// Find min max if KXMS_stabilizer is not used
	if ((Image_Processing.MCU_REG_25.GetSet.Roll==0)&&(Image_Processing.MCU_REG_25.GetSet.KXMS==0)&&(Image_Processing.MCU_REG_20.GetSet.STARK_Enable==0))
	{
		Update_min_max_header(buffer, F_FrameSize);
	}

#endif
}
/******************************************************************************
 * @brief       FlashAccessEnable_Read_Registers
 * @param       uint16_t* Address
 * @return      NONE
 * @details     this Function will  Flash Access Enable Read Registers
 *****************************************************************************/
//int FlashAccessEnable_Read_Registers (int Address)
//{
//	uint8_t tmp = 0;
//	uint8_t *flashBase = (uint8_t*)FLASH_LAST_SECTOR_ADDR;
//
//	tmp = flashBase[Address];
//
//	return tmp;
//}

/******************************************************************************
 * @brief       FlashAccessEnable_Write_Registers
 * @param       uint16_t* Address
 * @return      NONE
 * @details     this Function will   Access write MCU Flash
 *****************************************************************************/
//void FlashAccessEnable_Write_Registers (int Address,int Data)
//{
//	Acces_Write_McuFlash(Address,Data);
//}

/******************************************************************************
 * @brief       Process_Header
 * @param       ProcessDataFrame_TXBuf -> Frame bufferd
 * @return      None
 * @details     custom data
 *****************************************************************************/
void Process_Header(uint16_t* ProcessDataFrame_TXBuf)
{
#ifdef WITH_TOF_VL53L1
    static VL53L1_RangingMeasurementData_t tofMeasurement;
#endif
//	uint16_t   i=0;

	if(MCU_REGISTER.MCU_REG_B1.Set.Header_type == 0 )				// data can be custom data form 22 to 79
	{
//		i=22;
	}
	else if(MCU_REGISTER.MCU_REG_B1.Set.Header_type == 2)			// data can be custom data form 22 to 79
	{
//		i=22;
	}
	else
	{
		return;
	}


#ifdef WITH_TOF_VL53L1
            // XBob Android TOF
            TOF_VL53L1_GetMeasurement(&tofMeasurement);
            ProcessDataFrame_TXBuf[i++]    = (uint16_t)((tofMeasurement.TimeStamp & 0xFFFF0000) >> 16);
            ProcessDataFrame_TXBuf[i++]    = (uint16_t)(tofMeasurement.TimeStamp & 0x0000FFFF);
            // StreamCount (uint8_t) and RangeQualityLevel (uint8_t)
            ProcessDataFrame_TXBuf[i++]    = (((uint16_t)tofMeasurement.StreamCount) << 8)
                                            + ((uint16_t)tofMeasurement.RangeQualityLevel);
            // SignalRateRtnMegaCps (FixPoint1616_t == uint32_t)
            ProcessDataFrame_TXBuf[i++]    = (uint16_t)((tofMeasurement.SignalRateRtnMegaCps & 0xFFFF0000) >> 16);
            ProcessDataFrame_TXBuf[i++]    = (uint16_t)(tofMeasurement.SignalRateRtnMegaCps & 0x0000FFFF);
            // AmbientRateRtnMegaCps (FixPoint1616_t == uint32_t)
            ProcessDataFrame_TXBuf[i++]    = (uint16_t)((tofMeasurement.AmbientRateRtnMegaCps & 0xFFFF0000) >> 16);
            ProcessDataFrame_TXBuf[i++]    = (uint16_t)(tofMeasurement.AmbientRateRtnMegaCps & 0x0000FFFF);
            // EffectiveSpadRtnCount (uint16_t)
            ProcessDataFrame_TXBuf[i++] = tofMeasurement.EffectiveSpadRtnCount;
            // SigmaMilliMeter (FixPoint1616_t == uint32_t)
            ProcessDataFrame_TXBuf[i++]    = (uint16_t)((tofMeasurement.SigmaMilliMeter & 0xFFFF0000) >> 16);
            ProcessDataFrame_TXBuf[i++]    = (uint16_t)(tofMeasurement.SigmaMilliMeter & 0x0000FFFF);
            // RangeMilliMeter (int16_t)
            ProcessDataFrame_TXBuf[i++]    = (uint16_t)tofMeasurement.RangeMilliMeter;
            // RangeFractionalPart (uint8_t) and RangeStatus (uint8_t)
            ProcessDataFrame_TXBuf[i++]    = (((uint16_t)tofMeasurement.RangeFractionalPart) << 8)
                                            + ((uint16_t)tofMeasurement.RangeStatus);
#endif
		// ProcessDataFrame_TXBuf[0] = minTemp; to  check what was wrong. Seems we need to skip 80 at the end
//	if((Image_Processing.MCU_REG_20.GetSet.STARK_Enable==1)||(Image_Processing.MCU_REG_30.GetSet.MedianFilterEnable ==1))
	{
		ProcessDataFrame_TXBuf[6] = minTemp;
		ProcessDataFrame_TXBuf[5] = maxTemp;
	}

	COnvert_Image_Transfer_Format_header(ProcessDataFrame_TXBuf);
}

