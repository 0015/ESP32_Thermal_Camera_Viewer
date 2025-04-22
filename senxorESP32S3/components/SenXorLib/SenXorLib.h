#ifndef __SENXORLIB_H__
#define __SENXORLIB_H__
#include "stdint.h"
#include "defines.h"

#ifdef  Application_SPI_USB_Panther
#define FRAMEWIDTH_BUF 160
#define FRAMEHEIGHT_BUF 124
#define FILTER_BUFFER 120*FRAMEWIDTH_BUF
#else
#define FRAMEWIDTH_BUF 80
#define FRAMEHEIGHT_BUF 64
#define FILTER_BUFFER 62*FRAMEWIDTH_BUF
#endif

#define B1_START_CAPTURE 	0x01
#define B1_SINGLE_CONT 		0x02

typedef struct
{
	uint16_t TXBuf[(FRAMEHEIGHT_BUF)*FRAMEWIDTH_BUF+80];		// 10 is for CRC space
	uint16_t FrameSize;
	uint16_t StartOfFrame;
} FrameBuf_struct;

typedef struct
{
	uint8_t TXHeader[4+4+4];			// padd headear with 3 space to make 4 byte alignment possible.
	uint16_t TXBuf[(FRAMEHEIGHT_BUF)*FRAMEWIDTH_BUF+80+160];		// 10 is for CRC space
	uint16_t FrameSize;
	uint16_t StartOfFrame;
	uint16_t PixelMax;
} Transmit_FrameBuf_struct;

extern uint8_t Capture_DMA_Enable;
extern uint32_t CalData_Available;
extern FrameBuf_struct * ReceiveFrame;
extern uint32_t CapFirstByte;
extern volatile uint32_t PixelCnt;
extern uint32_t DATA_AV_Threshold;
extern uint8_t SenXorModel;
extern uint32_t Capture_Process_Status;
extern int DataProcessComplete;
extern volatile uint32_t SenXorError;
extern uint8_t SPI_Tx_Done;
extern uint8_t PowerModeStatus;
extern Transmit_FrameBuf_struct * TransmitFrame;
extern uint8_t	DetectModule;
extern uint32_t FrameSize;
extern uint32_t FrameWidth;
extern uint8_t SleepModeStatus;
extern uint32_t Read_Exertenal_Flash_address;
extern uint8_t PowerModeDownStatus;

void Process_CalibrationData(uint8_t PowerOn_init,uint16_t *address);

void DataFrameReceiveSenxor (void);

void DataFrameProcess (void);

void Halt_Capture(void);

void Acces_Write_Reg(int Address, uint8_t Data);

void Write_MCU_Registers_Interrupt (int Address,int Data);

void RegsiterSetClearDATA_READY(uint8_t SetClear);

void Clear_Capture_Flags (void);

void ReadBuf_SenXorInternal_Flash(uint16_t address , uint16_t len, uint8_t *buf );

void CaptureProcessFrame_nonFIFO(uint16_t tmp) ;

void CaptureProcessFrame(uint16_t tmp);

void Initialize_McuRegister (void);

void PowerOffModule(void);

void Power_On_Senxor(int On_off);

void GetReceiveFrameBuffer (void);

void GetTransmitFrameBuffer (void);

void SelfCalibration_Data(uint8_t PowerOn_init,uint8_t ModuleType);

uint8_t Acces_Read_Reg(int Address);

uint8_t Initialize_SenXor (uint8_t register_init);

uint8_t NetdChangeRow(uint16_t pixel_cnt);

uint8_t RegisterBank_Select(void);

uint16_t* DataFrameGetPointer (void);

void Read_AGC_LUT(void);

#endif// __LIBRARY_H__


