/*****************************************************************************
 * @file     SenXor_FLASH.c
 * @version  V1.00
 * @brief    This file holds all function to read from external flash
 * @copyright (C) 2018 Meridian Innovation
 ******************************************************************************/
#include "SenXor_FLASH.h"
#include "SenXorLib.h"

//#include "WB_ref_code_def.h"

#define FlashSize 0xffff
#define mode4byte 0
#define MsgError 55
#define MsgSuccess 1


int mode = 0;
int qpi_flag = 0;

void Write_Enable(void) ;

/******************************************************************************
 * @brief       Read_SenXorExternal_Flash
 * @param       addr: address
                     ReadDataBuffer: store read data buffer
                     Count: number of byte
 * @return      None
 * @details     Read External Flash
 *****************************************************************************/
uint32_t Read_SenXorExternal_Flash(uint32_t addr,uint32_t count, uint8_t* ReadDataBuffer)
{ //0x03 Read data
	unsigned int i;
	if(CheckBusy()==1)
	{
		return MsgError;		//Flash Busy
	}

	if(addr > FlashSize)
	{
		return MsgError;		//out of range
	}

	if(count<1)
	{
		return MsgError;
	}

	Drv_Gpio_SSFLASH_PIN_Set(0);								//Chip selected

	Drv_SPI_Senxor_Write_8Bit(SPI_RD_CMD);

	//for large IC. IF IC is single die and the mode in 4bytemode
	//multi die IC always read in 3 byte address.

	Drv_SPI_Senxor_Write_8Bit((addr>>16)&0xFF);

	Drv_SPI_Senxor_Write_8Bit((addr>>8)&0xFF);

	Drv_SPI_Senxor_Write_8Bit((addr)&0xFF);

	for(i=0; i<count; i++)
	{
		*(ReadDataBuffer+i) = Drv_SPI_Senxor_Read_8Bit();
	}
	Drv_Gpio_SSFLASH_PIN_Set(1);                      //Chip unselected
	return 1;
}


/******************************************************************************
 * @brief       Write_SenXorExternal_Flash
 * @param       addr: address
                     updatebuffer: store write data buffer
                     Count: number of byte
 * @return      None
 * @details     write External Flash
 *****************************************************************************/
unsigned int Write_SenXorExternal_Flash(uint32_t addr,uint32_t count, uint8_t* updatebuffer)
{
	int i;
	if (CheckBusy()==1) return MsgError;		//Flash Busy
	if (addr> FlashSize) return MsgError;		//out of range

	Write_Enable();
	Drv_Gpio_SSFLASH_PIN_Set(0);						//Chip selected
	Drv_SPI_Senxor_Write_8Bit(SPI_PP_CMD);

	//for large IC. IF IC in 4bytemode
	if(mode4byte)
	{
		Drv_SPI_Senxor_Write_8Bit((addr>>24)&0xFF);
	}

	Drv_SPI_Senxor_Write_8Bit((addr>>16)&0xFF);
	Drv_SPI_Senxor_Write_8Bit((addr>>8)&0xFF);
	Drv_SPI_Senxor_Write_8Bit((addr)&0xFF);

	for( i =0 ;i<count;i++)
	{
		Drv_SPI_Senxor_Write_8Bit(*(updatebuffer+i));
	}

	Drv_Gpio_SSFLASH_PIN_Set(1);                      	//Chip unselected
	while (CheckBusy() == 1);

	return MsgSuccess;
}


/******************************************************************************
 * @brief       Read_RDSR1_StatusRegister
 * @param       None
 * @return      None
 * @details     Read Status Register 1
 *****************************************************************************/
unsigned char Read_RDSR1_StatusRegister()
{
	//0x05 Read Status Register 1
	unsigned char StatusRegister1data;

	Drv_Gpio_SSFLASH_PIN_Set(0);								//Chip selected

	Drv_SPI_Senxor_Write_8Bit(SPI_RDSR1_CMD);

	StatusRegister1data = Drv_SPI_Senxor_Read_8Bit();

	Drv_Gpio_SSFLASH_PIN_Set(1);                      //Chip unselected
	return StatusRegister1data;
}

/******************************************************************************
 * @brief       Read_RDSR2_StatusRegister
 * @param       None
 * @return      None
 * @details     Read Status Register 2
 *****************************************************************************/
unsigned char Read_RDSR2_StatusRegister() {
	//0x35 Read Status Register 2
	unsigned char StatusRegister2data;

	Drv_Gpio_SSFLASH_PIN_Set(0);								//Chip selected

	Drv_SPI_Senxor_Write_8Bit(SPI_RDSR2_CMD);
	StatusRegister2data = Drv_SPI_Senxor_Read_8Bit();

	Drv_Gpio_SSFLASH_PIN_Set(1);                      //Chip unselected
	return StatusRegister2data;
}

/******************************************************************************
 * @brief       HalfBlockErase
 * @param       addr
 * @return      None
 * @details     32KB Block Erase
 *****************************************************************************/
unsigned int HalfBlockErase(unsigned int addr)
{//0x52 32kb block erase
	if (CheckBusy()==1) return MsgError;		//Flash Busy
	if (addr> FlashSize) return MsgError;		//out of range

	Write_Enable();
	Drv_Gpio_SSFLASH_PIN_Set(0);								//Chip selected
	Drv_SPI_Senxor_Write_8Bit(SPI_BE32_CMD);
	//for large IC. IF IC in 4bytemode
	if(mode4byte)
	{
	Drv_SPI_Senxor_Write_8Bit((addr>>24)&0xFF);
	}
	Drv_SPI_Senxor_Write_8Bit((addr>>16)&0xFF);
	Drv_SPI_Senxor_Write_8Bit((addr>>8)&0xFF);
	Drv_SPI_Senxor_Write_8Bit((addr)&0xFF);
	Drv_Gpio_SSFLASH_PIN_Set(1);                      //Chip unselected
	while (CheckBusy() == 1);
	return MsgSuccess;
}

/******************************************************************************
 * @brief       CheckBusy
 * @param       addr
 * @return      None
 * @details     Check ReadyBusy Bit
 *****************************************************************************/
unsigned char CheckBusy()
{
	unsigned char temp;

	temp = Read_RDSR1_StatusRegister();
	if((temp& 0x01)==0x01)
	{
		return 1;	//busy
	}
	else
	{
		return 0;	//ready
	}
}

/******************************************************************************
 * @brief       Write_Enable
 * @param       addr
 * @return      None
 * @details     Write Enable
 *****************************************************************************/
void Write_Enable(void)
{														//0x06 Write Enable
	Drv_Gpio_SSFLASH_PIN_Set(0);						//Chip selected

	Drv_SPI_Senxor_Write_8Bit(SPI_WREN_CMD);

	Drv_Gpio_SSFLASH_PIN_Set(1);                      //Chip unselected

}
