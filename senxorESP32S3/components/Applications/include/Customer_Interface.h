#ifndef __CUSTOMER_INTERFACE_H__
#define __CUSTOMER_INTERFACE_H__

#include <stdint.h>

int Read_Customer_Registers (int Address);
void Write_Customer_Registers (int Address,int Data);
void FilterFrame(uint16_t* current, int l_FrameSize);
int Customer_Interface_Read_Registers (int Address);
void Customer_Interface_Write_Registers (int Address,int Data);
int FlashAccessEnable_Read_Registers (int Address);
void FlashAccessEnable_Write_Registers (int Address,int Data);
int ApplicationReadVersion (int Address);

enum SenXor_Capture {
   DATA_INITIAL,
   DATA_PROCESS,
   DATA_TRANSMIT,
   DATA_RECEIVE_COMPLETE
};

typedef struct
{
	union Buffer
	{
		uint16_t Data;
		 struct
		{
			uint8_t LSB;
			uint8_t MSB;
		}Set;
	}Buffer;
} MERGE;
typedef struct
{
	union Frame_Mode
	{
		uint8_t Setting;
		 struct
		{
			uint8_t Start_Capture :1;
			uint8_t Single_Continious:1;
			uint8_t Frame_capture_mode:3;
			uint8_t Header_type:2;
			uint8_t Compensated:1;
		}Set;
	}MCU_REG_B1;
	union Power
	{
		uint8_t Setting;
		 struct
		{
			uint8_t Time:6;
			uint8_t MultiOneSec:1;
			uint8_t LowPower:1;
		}Set;
	}MCU_REG_B5;

	union LowClockSpeed
	{
		uint8_t Setting;
		 struct
		{
			uint8_t LowClkSpeed:1;
			uint8_t Parallel_Process:1;
			uint8_t PowerOff_Or_PowerDown:1;
			uint8_t nouse:5;
		}Set;
	}MCU_REG_B7;
	union MCU_Reg_0xBA_SenXorType
	{
		uint8_t Setting;
		 struct
		{
			uint8_t SenXorType;
		}Set;
	}MCU_REG_BA;
	union MCU_Reg_0xBB_ModuleType
	{
		uint8_t Setting;
		 struct
		{
			uint8_t ModuleType;
		}Set;
	}MCU_REG_BB;
	union AutoGain
	{
		uint8_t Setting;
		 struct
		{
			uint8_t GainPreset:4;
			uint8_t Auto_Gain_State:3;
			uint8_t HighRes:1;
		}Set;
	}MCU_REG_B9;
	union VDDCalibration
	{
		uint8_t Setting;
		 struct
		{
			uint8_t nouse1:1;
			uint8_t InitBlind_calibration:1;
			uint8_t Blind_calibrationStatus:1;
			uint8_t EnableVDDPixel:1;
			uint8_t SelfCalib:1;
			uint8_t CalSampleSize:3;

		}Set;
	}MCU_REG_C5;

	union TemperatureOffset_correction
	{
		uint8_t TemperatureOffset;
		 struct
		{
			uint8_t TemperatureOffset_correction;
		}Set;
	}MCU_REG_CB;
	union NoiseFilter
	{
		uint8_t Setting;
		 struct
		{
			uint8_t NoiseFilterEnable:1;
			uint8_t Initial_Noise_Filter:1;
			uint8_t nouse1:1;
			uint8_t nouse:2;
			uint8_t nouse2:2;
			uint8_t NoiseFilterErrorCheck:1;
		}Set;
	}
	MCU_REG_D0;
	union NoiseFilterValue
	{
		uint16_t Setting;
		 struct
		{
			uint8_t LSB;
			uint8_t MSB;
		}Set;
	}MCU_REG_D1;
	union nouse1
	{
		uint8_t Setting;
		 struct
		{
			uint8_t nouse;
		}Set;
	}MCU_REG_D3;
	union Row_mode
	{
		uint8_t Setting;
		 struct
		{
			uint8_t Enable_NETD:1;
			uint8_t InsertFrame:1;
			uint8_t nouse:6;
			uint8_t NETD_Reduction_factor:4;
		}Set;
	}
	MCU_REG_D4;
	union NETDReductionfactor
	{
		uint8_t Setting;
		 struct
		{
			uint8_t NETD_Reduction_factor;
		}Set;
	}
	MCU_REG_D5;
	union Single_PixelY
	{
		uint8_t Setting;
		 struct
		{
			uint8_t Single_Pixel_X;
		}Set;
	}
	MCU_REG_D6;
	union Single_PixelX
	{
		uint8_t Setting;
		 struct
		{
			uint8_t Single_Pixel_Y;
		}Set;
	}
	MCU_REG_D7;
	union FlashAccessEnable
	{
		uint8_t Setting;
		 struct
		{
			uint8_t FlashAccessEnable:1;
			uint8_t nouse:7;
		}Set;
	}
	MCU_REG_D8;
	union Application_Version
	{
		uint16_t Setting;
		 struct
		{
			uint16_t FlashAccessEnable:1;
			uint16_t nouse:7;
		}Set;
	}
	MCU_REG_D9_DA;
	union External_Flash
	{
		uint32_t Address;
		 struct
		{
			uint8_t F2_ADDR7_0;
			uint8_t F1_ADDR15_8;
			uint8_t F0_ADDR23_16;
		}Set;
	}
	MCU_REG_F2F1F0;
	union Look_Table
	{
		uint8_t Setting;
		 struct
		{
			uint8_t Look_Table:1;
			uint8_t nouse:3;
			uint8_t LUTversion:4;
		}Set;
	}
	MCU_REG_BC;

	union MinMaxControl
	{
		uint8_t Setting;
		 struct
		{
			uint8_t RollingAverageMinMax:6;
			uint8_t ClipUsingMin:1;
			uint8_t ClipUsingMax:1;

		}Set;
	}
	MCU_REG_D9;
} MCU_REG;

/* below can me remove or edit , just as a example*/
typedef struct
{
	union Registers_0
	{
		uint8_t Setting;
		 struct
		{
			uint8_t Mcu_Reset:1;
			uint8_t nouse:7;
		}Set;
	}REG_00;
	union Registers_1
	{
		uint8_t Setting;
		 struct
		{
			uint8_t DMA_Timeout_Enable:1;
			uint8_t Time_Out_Selection:2;
			uint8_t Stop_Host_Transfer:1;
			uint8_t nouse:4;
		}Set;
	}REG_01;

} CUST_REG;

typedef struct
{
	union Filter
	{
		uint8_t Setting;
		 struct
		{
			uint8_t STARK_Enable:1;	// Enables STARK
			uint8_t STARK_auto:1;
			uint8_t STARK_Type:1; // if 0 no BG remove, else if 1 BG remove
			uint8_t Kernel_size:1;

			uint8_t nouse:4;
		}GetSet;
	}
	MCU_REG_20;
		union STARK_REG1
	{
		uint8_t Setting;
		 struct
		{
			uint8_t STARK_cuttoff;
		}GetSet;
	}MCU_REG_21;
	union  STARK_REG2 // Roling_Average
	{
		uint8_t Setting;
		 struct
		{
			uint8_t STARK_grad;	// STARK: SMOOTHSTEP gradient
		}GetSet;
	}MCU_REG_22;
	union  STARK_REG3 // Roling_Average
	{
		uint8_t Setting;
		 struct
		{
			uint8_t STARK_scale;	// STARK: SMOOTHSTEP gradient
		}GetSet;
	}MCU_REG_23;
	union Stabilizer
	{
		uint8_t Setting;
		struct
		{
			uint8_t KXMS:1;
			uint8_t Roll:1;
			uint8_t nouse:6;
		}GetSet;
	}
	MCU_REG_25;
	union Filter2
	{
		uint8_t Setting;
		 struct
		{
			uint8_t MedianFilterEnable:1;	// Median Filter Kernel Size
			uint8_t MedianFilterSize:1; // Median Filter Kernel Size
			uint8_t nouse:6;
		}GetSet;
	}
	MCU_REG_30;
	union Image_Transfer_Forma
	{
		uint8_t Setting;
		 struct
		{
			uint8_t Temperature_Format;
		}GetSet;
	}
	MCU_REG_31;
} ImageProcessing;



/************************************************/

extern CUST_REG App_Register;
void Initialize_Filter(void);


extern MCU_REG MCU_REGISTER;
void Process_Header(uint16_t* ProcessDataFrame_TXBuf);
#define B1_START_CAPTURE 	0x01
#define B1_SINGLE_CONT 		0x02
#endif //__CUSTOMER_INTERFACE_H__
