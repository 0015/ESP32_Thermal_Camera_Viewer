/*****************************************************************************
 * @file     DrvSPI.c
 * @version  2.00
 * @brief    Contains functions that initialising, configuring and controlling SPI
 * @date	 5 May 2022
 ******************************************************************************/
#include "DrvSPIHost.h"
#include "esp_err.h"

//public:
extern volatile uint32_t SenXorError;
extern EXT_RAM_BSS_ATTR uint16_t CalibData_BufferData[CALIBDATA_FLASH_SIZE];
uint8_t dataBuff[4] = {0};								//Data buffer holds data from SPI
uint8_t Read_Flash_Timeout = 0;
DMA_ATTR uint8_t dummy[4] = {0x80,0x00};		//Dummy data for SPI reading phrase
DMA_ATTR uint8_t dataBuff_dma[4] = {0};
uint8_t SPI_Tx_Done = 0;

//private:
static spi_device_handle_t mHandler;					//SPI handler
static lldesc_t *dmaDescTx;								//Address of DMA TX descriptor
static lldesc_t *dmaDescRx;								//Address of DMA RX descriptor
static int rx_dma_ch;									//RX Channel allocated for a SPI transaction
static int tx_dma_ch;									//TX Channel allocated for a SPI transaction
static uint32_t selectSPISpd(const uint8_t sel);		//Select SPI clock speed by selection

/******************************************************************************
 * @brief       Drv_SPI_Init
 * @param       None
 * @return      None
 * @details     Initialise MCU SPI
 *****************************************************************************/
void Drv_SPI_Init(void)
{
	esp_err_t ret = ESP_OK;
	spi_bus_config_t spi_config = {0};
	spi_config.miso_io_num = PIN_SPI_MISO;										//Configuring MISO PIN
	spi_config.mosi_io_num = PIN_SPI_MOSI;										//Configuring MOSI PIN
	spi_config.sclk_io_num = PIN_SPI_CLK;										//Configuring SPI Clock
	spi_config.quadwp_io_num = -1;
	spi_config.quadhd_io_num = -1;
	ret = spi_bus_initialize(SPI2_HOST, &spi_config, SPI_DMA_CH_AUTO);				//Initialise SPI bus. Using a DMA channel selected by driver


	if(ret == ESP_OK)
	{
		ESP_LOGI(STAG,SPI_BUS_INIT);
		ESP_LOGI(STAG,SPI_BUS_INFO,PIN_SPI_MISO,PIN_SPI_MOSI,PIN_SPI_CLK);
	}
	else 
	{
		ESP_LOGE(STAG,SPI_BUS_INIT_FAILED);
		ESP_LOGE(STAG,"%s",esp_err_to_name(ret));
	}// End if-else

	dmaDescTx = spi_bus_get_attr(SPI2_HOST)->dmadesc_tx;						//Obtain the address of DMA TX Descriptor
	dmaDescRx = spi_bus_get_attr(SPI2_HOST)->dmadesc_rx;						//Obtain the address of DMA RX Descriptor
	tx_dma_ch = spi_bus_get_attr(SPI2_HOST)->tx_dma_chan;						//Obtain the id of TX channel
	rx_dma_ch = spi_bus_get_attr(SPI2_HOST)->rx_dma_chan;						//Obtain the id of RX channel

	ESP_LOGI(STAG,SPI_DMA_INIT,tx_dma_ch,rx_dma_ch);
	Drv_SPI_DMA_PrepDesc(dummy, dataBuff_dma, DEFAULT_SPI_LENGTH);

}
/******************************************************************************
 * @brief       Drv_SPI_SENXOR_Init
 * @param       clk_speed - SPI clock speed
 * 				FlashEnable - Determine whether the SPI is interfacing with SenXor's FLASH
 * @return      None
 * @details     Attach SenXor to SPI
 *****************************************************************************/
void Drv_SPI_SENXOR_Init(uint32_t clk_speed, const uint8_t FlashEnable)
{
	/*
	 * If there is an existing device, we remove it first to prevent resource conflict
	 */
	if(mHandler)
	{
		// ESP_LOGW(STAG,SPI_WARN_DEV_EXISTS);
		spi_bus_remove_device(mHandler);
	}

	//Clock selection
	if(clk_speed < SPI_CLK_SPEED_5M || clk_speed > SELECT_SPICLK_20M)
	{
		uint32_t sel_clk = selectSPISpd(clk_speed);

		// ESP_LOGW(STAG,SPI_WARN_CLKSPD,clk_speed,sel_clk);
		clk_speed = sel_clk;
	}

	spi_device_interface_config_t spi_inter_config = {0};
	spi_inter_config.clock_speed_hz = clk_speed;								//SPI Speed
	spi_inter_config.duty_cycle_pos = 128;										//SPI Clock duty cycle
	spi_inter_config.cs_ena_posttrans = 3;										//Number of cycle that SPI CS hold after a transaction
	spi_inter_config.queue_size = 1;											//Queue size
	spi_inter_config.spics_io_num = -1;											//Chip select

	spi_bus_add_device(SPI2_HOST, &spi_inter_config, &mHandler);					//Applies configuration and attach device to SPI

	GPSPI2.dma_int_ena.trans_done = 1;
	GPSPI2.clock.clk_equ_sysclk = 0;											//SPI clock should not equal to system clock.
	GPSPI2.user.doutdin = 1;													//Enable full-duplex mode

}

/******************************************************************************
 * @brief       Drv_SPI_Senxor_ConstructData
 * @param       reg 	-> 7 bit Registers to access
				write 	-> 1 = Write , 0 = Read
				data 	-> 8 Bit data to write
 * @return      None
 * @details     Construct 16 bit data SPI Transfer for registers access of the SenXor
 *****************************************************************************/
uint32_t Drv_SPI_Senxor_ConstructData(const uint8_t reg, const uint8_t write, const uint8_t data)
{
	return ((reg << 9)) | ((write & 0x01) << 8) | (data);
}

/******************************************************************************
 * @brief       Drv_SPI_Senxor_Read_Reg
 * @param       reg  - The address of register to be read
 * @return      Data read from register
 * @details     Read from SenXor Register
 *****************************************************************************/
int Drv_SPI_Senxor_Read_Reg(const uint8_t reg)
{
	const uint32_t write = Drv_SPI_Senxor_ConstructData(reg, 0x00, 0x00);	//Produce a pattern specialised for reading/writing registers
	uint8_t buff[4] = {(write&0xff00) >> 8  , (write&0x00ff)};				//buffer

	uint8_t recvReg[4] = {0};
	Drv_SPI_DMA_Disable();
	spi_ll_clear_int_stat(&GPSPI2);											//Clear SPI transfer done interrupt bit
	spi_ll_set_mosi_bitlen(&GPSPI2, DEFAULT_SPI_LENGTH);					//Configure SPI data length

	Drv_Gpio_SSREGN_PIN_Set(0);

	spi_ll_write_buffer(&GPSPI2,buff,DEFAULT_SPI_LENGTH);					//Write SPI buffer

	spi_ll_enable_mosi(&GPSPI2, 1);											//Enable MOSI
	spi_ll_enable_miso(&GPSPI2, 1);											//Enable MISO

	spi_ll_apply_config(&GPSPI2);
	spi_ll_user_start(&GPSPI2);										//Initiate an SPI transaction

	while(!spi_ll_usr_is_done(&GPSPI2));

	Drv_Gpio_SSREGN_PIN_Set(1);

	spi_ll_read_buffer(&GPSPI2,recvReg,DEFAULT_SPI_LENGTH);					//Read buffer from SPI

	return (int) recvReg[1];					//Return the data received
}

/******************************************************************************
 * @brief       Drv_SPI_Senxor_Read_8Bit
 * @param       None
 * @return      8 bits data from SPI buffer
 * @details     Read SenXor Flash
 *****************************************************************************/
uint8_t Drv_SPI_Senxor_Read_8Bit(void)
{
	const uint8_t dummyFLASH[4] = {0xAA,0x00,0x00,0x00};
	Drv_SPI_DMA_Disable();
	spi_ll_clear_int_stat(&GPSPI2);										//Clear SPI transfer done interrupt bit
	spi_ll_set_mosi_bitlen(&GPSPI2, FLASH_SPI_LENGTH);					//Configure data length (in bits)

	spi_ll_write_buffer(&GPSPI2, dummyFLASH, FLASH_SPI_LENGTH);
	spi_ll_enable_mosi(&GPSPI2, 1);										//Enable MOSI
	spi_ll_enable_miso(&GPSPI2, 1);										//Enable MISO

	spi_ll_apply_config(&GPSPI2);
	spi_ll_user_start(&GPSPI2);										//Initiate an SPI transaction

	while(!spi_ll_usr_is_done(&GPSPI2));								//Wait while transaction done
	spi_ll_read_buffer(&GPSPI2,dataBuff,FLASH_SPI_LENGTH);				//Read buffer from SPI

	return dataBuff[0];
}

/******************************************************************************
 * @brief       Drv_SPI_Senxor_Write_Reg
 * @param       reg  -> registers to Write
				data -> Data to write to register
 * @return      None
 * @details     Write to SenXor Register
 *****************************************************************************/
int Drv_SPI_Senxor_Write_Reg(const uint8_t reg, const uint8_t data)
{
	const uint32_t write = Drv_SPI_Senxor_ConstructData(reg, 0x01, data);		//Construct the data for register writing
	uint8_t buff[4] = {(write&0xff00) >> 8  , (write&0x00ff)};					//Load the data into buffer
	uint8_t recvReg[4] = {0};															//Initialise receive buffer
	Drv_SPI_DMA_Disable();
	spi_ll_clear_int_stat(&GPSPI2);													//Clear SPI transfer done interrupt bit
	spi_ll_set_mosi_bitlen(&GPSPI2, DEFAULT_SPI_LENGTH);					//Configure the data length

	spi_ll_write_buffer(&GPSPI2,buff,DEFAULT_SPI_LENGTH);					//Write SPI buffer

	spi_ll_enable_mosi(&GPSPI2, 1);											//Enable MOSI
	spi_ll_enable_miso(&GPSPI2, 1);											//Enable MISO

	Drv_Gpio_SSREGN_PIN_Set(0);
	spi_ll_apply_config(&GPSPI2);
	spi_ll_user_start(&GPSPI2);										//Initiate an SPI transaction

	while(!spi_ll_usr_is_done(&GPSPI2));
	Drv_Gpio_SSREGN_PIN_Set(1);

	spi_ll_read_buffer(&GPSPI2,recvReg,DEFAULT_SPI_LENGTH);

	return (int)recvReg[0] << 8  | (int)recvReg[1];						//Return the data received. Meaningless for a register write operation.

}
/******************************************************************************
 * @brief       Read_CalibrationData
 * @param       None
 * @return      None
 * @details     Read Calibration Data
 *****************************************************************************/
void Read_CalibrationData(void)
{

	uint8_t flashreadtemp = 0;

	//Power_On_Senxor and  Initialize_SenXor should not be inside the Read calibration data let put outside
	CalData_Available = false;

	Drv_SPI_SENXOR_Init(SELECT_SPICLK_6M,1);

	Read_Flash_Timeout = 0;
	Read_SenXorExternal_Flash(0x0000,1,&flashreadtemp);

	if(Read_Flash_Timeout == 0)
	{
		Read_SenXorExternal_Flash(0x0000,CALIBDATA_FLASH_SIZE*2,(uint8_t*)CalibData_BufferData);
	}

}

/******************************************************************************
 * @brief       Drv_SPI_Host_PDMA_Disable
 * @param
 * @return      None
 * @details     stop pdma transmit and Disable interrupt
 *****************************************************************************/
void Drv_SPI_Host_PDMA_Disable(void)
{
	//Not in use
}
/******************************************************************************
 * @brief       Drv_SPI_Read
 * @param		None
 * @return      16bit integer
 * @details     Read via SPI direct mode. This function is ISR safe.
 *****************************************************************************/
uint16_t Drv_SPI_Read(void)
{
	Drv_Gpio_SSDATAN_PIN_Set(1);
	spi_ll_read_buffer(&GPSPI2,dataBuff,DEFAULT_SPI_LENGTH);
	return dataBuff[0] << 8 | dataBuff[1];

}

/******************************************************************************
 * @brief       Drv_SPI_Write (MOSI)
 * @param		None
 * @return      None
 * @details     Write via SPI direct mode. This function is ISR safe.
 *****************************************************************************/
void IRAM_ATTR Drv_SPI_Write(void)
{

	GPSPI2.dma_int_clr.trans_done = 1;						 //Clear SPI transfer done interrupt bit
	GPSPI2.ms_dlen.ms_data_bitlen = DEFAULT_SPI_LENGTH-1;	 //Configure data length (in bits)
	spi_ll_write_buffer(&GPSPI2, dummy, DEFAULT_SPI_LENGTH); //Write data to SPI buffer
	GPSPI2.user.usr_mosi = GPSPI2.user.usr_miso = 1;		 //Enable MOSI and MISO

	Drv_Gpio_SSDATAN_PIN_Set(0);							//Chip select
	spi_ll_apply_config(&GPSPI2);
	spi_ll_user_start(&GPSPI2);										//Initiate an SPI transaction
	while(!GPSPI2.dma_int_raw.trans_done);					//Wait while transfer complete

}

/******************************************************************************
 * @brief       Drv_SPI_Write_8Bit
 * @param		None
 * @return      None
 * @details     Write 8bit data via SPI.
 *****************************************************************************/
void Drv_SPI_Senxor_Write_8Bit(const uint8_t data)
{
	uint8_t send[4] = {data};
	Drv_SPI_DMA_Disable();
	spi_ll_clear_int_stat(&GPSPI2);							//Clear SPI transfer done interrupt bit
	spi_ll_set_mosi_bitlen(&GPSPI2, FLASH_SPI_LENGTH);		//Configure data length (in bits)

	spi_ll_write_buffer(&GPSPI2, send, FLASH_SPI_LENGTH);	//Configure data length (in bits)
	spi_ll_enable_mosi(&GPSPI2, 1);							//Enable MOSI
	spi_ll_enable_miso(&GPSPI2, 1);							//Enable MISO

	spi_ll_apply_config(&GPSPI2);
	spi_ll_user_start(&GPSPI2);										//Initiate an SPI transaction

	while(!spi_ll_usr_is_done(&GPSPI2));					//Wait while transfer complete

}
/******************************************************************************
 * @brief       Drv_SPI_Transmit
 * @param		None
 * @return      None
 * @details     SPI Operation via Direct mode. This function is ISR safe and designed to
 *				get SenXor's frame after data_AV interrupt
 *****************************************************************************/
uint16_t IRAM_ATTR Drv_SPI_Transmit(void)
{
	// GPSPI2.dma_int_clr.trans_done = 1;						 //Clear SPI transfer done interrupt bit
	// GPSPI2.ms_dlen.ms_data_bitlen = DEFAULT_SPI_LENGTH-1;	 //Configure data length (in bits)

	spi_ll_master_set_cs_setup(&GPSPI2, 0);
    spi_ll_master_set_cs_hold(&GPSPI2, 2);

	spi_ll_clear_int_stat(&GPSPI2);
	spi_ll_set_mosi_bitlen(&GPSPI2, DEFAULT_SPI_LENGTH);
	spi_ll_set_miso_bitlen(&GPSPI2, DEFAULT_SPI_LENGTH);

	spi_ll_write_buffer(&GPSPI2, dummy, DEFAULT_SPI_LENGTH); //Write data to SPI buffer
	
	spi_ll_enable_miso(&GPSPI2, 1);
	spi_ll_enable_miso(&GPSPI2, 1);		 									//Enable MOSI and MISO

	Drv_Gpio_SSDATAN_PIN_Set(0);													 //Chip select
	spi_ll_apply_config(&GPSPI2);
	spi_ll_user_start(&GPSPI2);														//Initiate an SPI transaction
	while(!GPSPI2.dma_int_raw.trans_done);											 	//Wait while transfer complete
	Drv_Gpio_SSDATAN_PIN_Set(1);

	spi_ll_read_buffer(&GPSPI2,dataBuff,DEFAULT_SPI_LENGTH); //Read buffer after transfer complete
	return dataBuff[0] << 8 | dataBuff[1];
}
/******************************************************************************
 * @brief       selectSPISpd
 * @param		sel - Clock selection
 * @return      None
 * @details     Get the clock speed by selection
 *****************************************************************************/
static uint32_t selectSPISpd(const uint8_t sel)
{
	uint32_t _clk = 20*1000*1000;
	switch(sel)
	{
		case SELECT_SPICLK_5M:
			_clk = 5*1000*1000;
			break;
		case SELECT_SPICLK_6M:
			_clk = 6*1000*1000;
			break;
		case SELECT_SPICLK_10M:
			_clk = 10*1000*1000;
			break;
		case SELECT_SPICLK_14M:
			_clk = 14*1000*1000;
			break;
		case SELECT_SPICLK_20M:
			_clk = 20*1000*1000;
			break;
	}
	return _clk;

}
/******************************************************************************
 * @brief       Drv_SPI_DMA_Transmit
 * @param		None
 * @return      None
 * @details     SPI Operation via DMA. This function is ISR safe and designed to
 *				get SenXor's frame after data_AV interrupt
 *****************************************************************************/
uint16_t IRAM_ATTR Drv_SPI_DMA_Transmit(void)
{

	gdma_ll_tx_set_desc_addr(&GDMA, tx_dma_ch, (uint32_t)dmaDescTx);	//Store inlink descriptor's address
	gdma_ll_tx_start(&GDMA, tx_dma_ch);

	gdma_ll_rx_set_desc_addr(&GDMA, rx_dma_ch, (uint32_t)dmaDescRx);	//Store inlink descriptor's address
	gdma_ll_rx_start(&GDMA, rx_dma_ch);									//Start dealing with the inlink descriptors
	GPSPI2.dma_int_clr.trans_done = 1;
	GPSPI2.ms_dlen.ms_data_bitlen = DEFAULT_SPI_LENGTH - 1;		//Configure SPI data length
	GPSPI2.user.usr_mosi = 1;									//Enable MOSI and MISO
	GPSPI2.user.usr_miso = 1;									//Enable MISO

	Drv_Gpio_SSDATAN_PIN_Set(0);
	spi_ll_apply_config(&GPSPI2);
	spi_ll_user_start(&GPSPI2);										//Initiate an SPI transaction

    while(!GPSPI2.dma_int_raw.trans_done);						//Wait while transfer complete
    Drv_Gpio_SSDATAN_PIN_Set(1);								//Chip de-select
    return dataBuff_dma[0] << 8 | dataBuff_dma[1];
}

/******************************************************************************
 * @brief       Drv_SPI_DMA_Enable
 * @param		None
 * @return      None
 * @details     SPI DMA Enable
 *****************************************************************************/
void Drv_SPI_DMA_Enable(void)
{
	GPSPI2.dma_conf.dma_rx_ena = 1;								//Enable SPI DMA RX
	GPSPI2.dma_conf.dma_tx_ena = 1;								//Enable SPI DMA TX

}
/******************************************************************************
 * @brief       Drv_SPI_DMA_Disable
 * @param		None
 * @return      None
 * @details     SPI DMA Disable
 *****************************************************************************/
void Drv_SPI_DMA_Disable(void)
{
	GPSPI2.dma_conf.dma_rx_ena = 0;								//Disable SPI DMA TX
	GPSPI2.dma_conf.dma_tx_ena = 0;								//Disable SPI DMA RX
}
/******************************************************************************
 * @brief       Drv_SPI_DMA_PrepDesc
 * @param		None
 * @return      None
 * @details    Prepare data to DMA descriptor
 *****************************************************************************/
void Drv_SPI_DMA_PrepDesc(uint16_t *txBuff, uint16_t *rxBuff,const int dataLen)
{
	const int dma_dataLen = ((dataLen + 7) / 8);						//DMA descriptor data length

	lldesc_setup_link(dmaDescTx, txBuff, dma_dataLen, false);			//Link TX descriptor with buffer.
	spi_ll_dma_tx_fifo_reset(&GPSPI2);

	spi_ll_outfifo_empty_clr(&GPSPI2);
	gdma_ll_tx_reset_channel(&GDMA, tx_dma_ch);							//Reset DMA TX Channel

	//spi_dma_ll_tx_start
	gdma_ll_tx_set_desc_addr(&GDMA, tx_dma_ch, (uint32_t)dmaDescTx);	//Store inlink descriptor's address
	gdma_ll_tx_start(&GDMA, tx_dma_ch);
	//end

	lldesc_setup_link(dmaDescRx, rxBuff, dma_dataLen, true);		//Link RX descriptor with buffer.
	spi_ll_dma_rx_fifo_reset(&GPSPI2);
	spi_ll_infifo_full_clr(&GPSPI2);
	gdma_ll_rx_reset_channel(&GDMA, rx_dma_ch);						//Reset DMA RX Channel

	gdma_ll_rx_set_desc_addr(&GDMA, rx_dma_ch, (uint32_t)dmaDescRx);	//Store inlink descriptor's address
	gdma_ll_rx_start(&GDMA, rx_dma_ch);									//Start dealing with the inlink descriptors

}


