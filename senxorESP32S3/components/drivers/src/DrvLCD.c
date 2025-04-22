/*****************************************************************************
 * @file     DrvLCD.c
 * @version  1.2
 * @brief    Initialising LCD and provide functions for displaying SenXor data
 * @date	 17 Jun 2022
 *
 ******************************************************************************/
#include "DrvLCD.h"
#include "MeridianLogo.h"
#include "More_perfect_VGA_dark.h"
#include "icons.h"

#ifdef CONFIG_MI_LCD_EN
//private:
static esp_lcd_panel_handle_t panel_handle;		//LCD panel handler
static esp_lcd_panel_io_handle_t io_handle;

static uint16_t* outputBuffer[14];				//Display buffers go here

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
static EXT_RAM_BSS_ATTR uint16_t bg[BG_SEG*BG_SEG];
#else
static EXT_RAM_BSS_ATTR uint16_t bg[BG_SEG*BG_SEG];
#endif

//protected:
static void ST7789Init(void);
static void GCLCDInit(void);
static void SPILCDInit(void);

#ifdef CONFIG_MI_LCD_BLK_PWM
static void LCDBLKCtrlInit(void);
#endif

static void LCDBuffInit(void);

/******************************************************************************
 * @brief       LCDInit
 * @param       None
 * @return      None
 * @details     Initialise SPI LCD
 *****************************************************************************/
void LCDInit(void)
{
#ifdef CONFIG_MI_LCD_EN
	//K-config capable functions
	//Initialise SPI here
	ESP_LOGI(LCDTAG, LCD_INIT_INFO);
	spi_bus_config_t spi_config = {0};
	spi_config.miso_io_num = LCD_MISO;									//Configuring MISO PIN
	spi_config.mosi_io_num = LCD_MOSI;									//Configuring MOSI PIN
	spi_config.sclk_io_num = LCD_CLK;									//Configuring SPI Clock
	spi_config.quadwp_io_num = -1;										//Disabling quad SPI pin
	spi_config.quadhd_io_num = -1;										//Disabling quad SPI pin
	spi_config.max_transfer_sz =  LCD_PIX*sizeof(uint16_t);				//Define maximum transfer size
	spi_bus_initialize(SPI3_HOST, &spi_config, SPI_DMA_CH_AUTO);		//Initialise SPI bus. Using a DMA channel selected by driver

	esp_lcd_panel_io_spi_config_t io_config = {0};						//LCD IO configuration structure
	io_config.dc_gpio_num = LCD_DC;							//Define LCD DC Pin
	io_config.cs_gpio_num = LCD_CS;							//Define LCD CD Pin
	io_config.pclk_hz = LCD_PIXEL_CLK;									//Define LCD SPI Clock speed
	io_config.lcd_cmd_bits = LCD_CMD_BITS;								//Define the length of LCD command data (In bit)
	io_config.lcd_param_bits = LCD_PARAM_BITS;							//Define the length of parameter data (In bit)
	io_config.spi_mode = 0;												//Define SPI mode
	io_config.trans_queue_depth = 10;									//Define SPI queue length
	ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI3_HOST, &io_config, &io_handle));	//Initialise LCD IO

#ifdef CONFIG_MI_LCD_BLK_PWM
	LCDBLKCtrlInit();
#endif

	LCDBuffInit();

	switch(CONFIG_MI_LCD_TYPE_VAL)
	{
		case 0:
			ST7789Init();
			break;
		case 1:
			GCLCDInit();
			break;
		default:
			break;
	}
#endif
}
/******************************************************************************
 * @brief       ST7789Init
 * @param       None
 * @return      None
 * @details     Initialise SPI for LCD (ST7789)
 *****************************************************************************/
static void ST7789Init(void)
{
#ifdef CONFIG_MI_ST7789
	esp_lcd_panel_dev_config_t panel_config = {0};						//Panel configuration structure
	panel_config.reset_gpio_num = LCD_RST;								//Define LCD reset pin
	panel_config.color_space = ESP_LCD_COLOR_SPACE_RGB;					//Define the colour space used by the LCD
	panel_config.bits_per_pixel = CONFIG_MI_COLOUR_BIT_VAL;				//Define LCD's colour depth (in bits)


	esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle);	//Attach the LCD to MCU
	ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));					//Reset LCD first before initialise
	ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));					//Initialise LCD
	ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));		//Turn on LCD
	ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, false));		//Swap xy to rotate the screen
	ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));	//Colour is inverted for displaying the colour correctly


	ESP_LOGI(LCDTAG, LCD_INIT_DONE);
	ESP_LOGI(LCDTAG, LCD_DETAILS, LCD_H_RES, LCD_V_RES, LCD_COLOUR_BIT, LCD_PIXEL_CLK);
	LCDBuffInit();														//Initialise the LCD buffer
	fillScreen(BLACK);													//Give a black background to display
	drawIcon(96,76,ICON_INITIALISING);
	drawText(1, 76+48,"Initialising...");
	drawText(1,240-12,"2023 Meridian Innovation");
#endif
}

/******************************************************************************
 * @brief       GCLCDInit
 * @param       None
 * @return      None
 * @details     Initialise SPI for LCD GC9A01
 *****************************************************************************/

static void GCLCDInit(void)
{
#ifdef CONFIG_MI_GC9A01
	ESP_LOGI(LCDTAG, LCD_INIT_INFO);

	esp_lcd_panel_dev_config_t panel_config = {0};						//Panel configuration structure
	panel_config.reset_gpio_num = LCD_RST;								//Define LCD reset pin
	panel_config.color_space = ESP_LCD_COLOR_SPACE_BGR;					//Define the colour space used by the LCD
	panel_config.bits_per_pixel = CONFIG_MI_COLOUR_BIT_VAL;

	esp_lcd_new_panel_gc9a01(io_handle, &panel_config, &panel_handle);	//Attach the LCD to MCU

	ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));					//Reset LCD first before initialise
	ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));					//Initialise LCD
	ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));		//Turn on LCD
	ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, true, false));	//Rotation
	ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));	//Colour is inverted for displaying the colour correctly

	ESP_LOGI(LCDTAG, LCD_INIT_DONE);
	ESP_LOGI(LCDTAG, LCD_DETAILS, LCD_H_RES, LCD_V_RES, LCD_COLOUR_BIT, LCD_PIXEL_CLK);

	LCDBuffInit();														//Initialise the LCD buffer
	displayLogo();
	fillScreen(BLACK);													//Give a black background to display
	drawText(20,120,"Initialising...");
	drawText(1,240-12,"2023 Meridian Innovation");
#endif
}

/******************************************************************************
 * @brief       SPILCDInit
 * @param		None
 * @return      None
 * @details     Initialise SPI LCD (Customised function)
 *****************************************************************************/
#ifdef CONFIG_MI_OTHER
static void SPILCDInit(void)
{

}
#endif
/******************************************************************************
 * @brief       LCDBLKCtrlInit
 * @param		None
 * @return      None
 * @details     Initialise PWM for controlling LCD brightness.
 *****************************************************************************/
#ifdef CONFIG_MI_LCD_BLK_PWM
static void LCDBLKCtrlInit(void)
{

	if(LCD_BLK == -1)
	{
		return;
	}
	/*
	 *  PWM Timer configuration
	 */
	ledc_timer_config_t ledc_timer = {0};						//PWM timer structure.
	ledc_timer.speed_mode       = LEDC_LOW_SPEED_MODE;			//Configure PWM mode.
	ledc_timer.timer_num        = LEDC_TIMER_0;					//Configure clock source.
	ledc_timer.duty_resolution  = LEDC_TIMER_8_BIT;				//Configure timer resolution
	ledc_timer.freq_hz          = CONFIG_MI_LCD_BLK_PWM_FREQ;	//Configure PWM frequency
	ledc_timer.clk_cfg          = LEDC_AUTO_CLK;				//Configure PWM timer clock source
	ledc_timer_config(&ledc_timer);

	/*
	 * Channel configuration
	 */
	ledc_channel_config_t ledc_channel = {0};
	ledc_channel.speed_mode     = LEDC_LOW_SPEED_MODE;
	ledc_channel.channel        = LEDC_CHANNEL_0;
	ledc_channel.timer_sel      = LEDC_TIMER_0;
	ledc_channel.intr_type      = LEDC_INTR_DISABLE;
	ledc_channel.gpio_num       = LCD_BLK;
	ledc_channel.duty           = 0;
	ledc_channel.hpoint         = 0;
	ledc_channel_config(&ledc_channel);

	setLCDBrightness(CONFIG_MI_LCD_BRIGHTNESS);
}
#endif
/******************************************************************************
 * @brief       LCDBuffInit
 * @param		None
 * @return      None
 * @details     Initialise buffer.
 *****************************************************************************/
static void LCDBuffInit(void)
{
	const size_t buffSize = LCD_BUFF_PIX_SENXOR*sizeof(uint16_t);

	for(uint8_t i = 0 ; i < 14 ; ++i)
	{
		outputBuffer[i] = heap_caps_malloc(buffSize, MALLOC_CAP_SPIRAM);
		if(outputBuffer[i] == 0)
		{
			ESP_LOGE(LCDTAG, LCD_ERR_ALLO_BUFF);
			return;
		}
	}

	for(uint8_t j = 12 ; j < 14 ; ++j)
	{
		uint16_t* pixel = outputBuffer[j];
		for(uint16_t k = 0 ; k <LCD_BUFF_PIX_SENXOR ; ++k)
		{
			pixel[k] = BLACK;
		}
	}
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
	colourMappedScaled = heap_caps_malloc(LCD_SENXOR_FRAME_H*LCD_SENXOR_FRAME_W*16*sizeof(uint16_t), MALLOC_CAP_SPIRAM);
#endif
	ESP_LOGI(LCDTAG, LCD_BUFF_INIT, buffSize/2);
}
/******************************************************************************
 * @brief       setLCDBrightness
 * @param		brightness - Brightness. This value will affect the PWM duty
 * 				that drive the LCD. The range is 0 - 100, where 0 = PWM duty 0%.
 * 				100 = PWM duty 100%
 * @return      None
 * @details     Set LCD brightness by adjusting the PWM duty cycle.
 *****************************************************************************/
void setLCDBrightness(const float brightness)
{
	float brightPercent = (brightness/100);

	if (brightness > 100)
	{
		brightPercent = 1;
	}
	else if(brightness < 0)
	{
		brightPercent = 0;
	}

	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, (pow(2,LEDC_TIMER_8_BIT)-1)*brightPercent);		//Setting duty cycle
	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

/******************************************************************************
 * @brief       displayLogo
 * @param		None
 * @return      None
 * @details     Display Meridian logo on LCD
 *****************************************************************************/
void displayLogo(void)
{

	uint16_t* logoBuff = heap_caps_malloc(MI_LOGO_H*MI_LOGO_W*sizeof(uint16_t), MALLOC_CAP_SPIRAM); 	//Buffer holding the pixel of the logo being converted to colour565
	uint16_t* logoSeg = heap_caps_malloc((LOG_SEG_H*LOG_SEG_W)*sizeof(uint16_t), MALLOC_CAP_SPIRAM);	//Segment buffer with size LOG_SEG_H x LOG_SEG_W

	if(logoBuff == 0 || logoSeg == 0 )
	{
		ESP_LOGE(LCDTAG, LCD_ERR_NO_BUFF);
		return;
	}

	const uint16_t pixel2Seg = LOG_SEG_H*LOG_SEG_W;														//The number of pixels for a segment
	const uint8_t end_x = MI_LOGO_OFFSET_X + LOG_SEG_W;													//X coordinate where the logo should stop drawing the logo
	const uint8_t end_y = MI_LOGO_OFFSET_Y + LOG_SEG_H;													//Y coordinate where the logo should stop drawing the logo

	//Extract the RGB component and covert them to colour565 format
	for(int i = 0, j=0 ; i <MI_LOGO_H*MI_LOGO_W*3 ; ++j, i+=3)
	{
    	uint8_t B = meridian_logo_bgr[i];																//Extract the Blue component
    	uint8_t G = meridian_logo_bgr[i+1];																//Extract the Green component
    	uint8_t R = meridian_logo_bgr[i+2];																//Extract the Red component
    	logoBuff[j] = colour565(R,G,B);																	//Convert the logo to colour565 format
	}

	/*
	 * Start drawing here
	 * Because of the limitation of ESP LCD driver. The logo can only be drawn segment by segment.
	 * Thus the logo is divided to 4 segments first, then drawn segment by segment:
	 *
	 * logoBuff -> Segment 1 -> Display ->... -> logoBuff -> Segment 4 -> Display
	 * For each segment: Height - 34 | Width - 80 | Pixel: 2720 for each segment
	 *
	 *
	 * int i: Segment sequence
	 * int j: Iterator for logo that is converted to colour565 format
	 * int k: Iterator for segment to be displayed on LCD
	 *
	 *
	 */

	for(int i = 0, j = 0; i< 4; ++i)
	{
		//Fill in the segment and display it one by one
		for(int k =0;k <pixel2Seg; ++k)
		{
			/*If iterator reaches the end of the segment row
			 * The iterator of logo will advance by 80
			 */
			if(k%LOG_SEG_W == 0 && k != 0)
			{
				j += 80;
			}
			logoSeg[k] = logoBuff[j];
			++j;
		}

		//Place each segment to proper location on LCD.
		switch (i)
		{
			//Segment 1
			case 0:
				j = LOG_SEG_W;	//Advance the iterator for the next segment
				esp_lcd_panel_draw_bitmap(panel_handle, MI_LOGO_OFFSET_X , MI_LOGO_OFFSET_Y , end_x , end_y , logoSeg);
			break;

			//Segment 2
			case 1:
				esp_lcd_panel_draw_bitmap(panel_handle, MI_LOGO_OFFSET_X+LOG_SEG_W , MI_LOGO_OFFSET_Y , end_x+LOG_SEG_W , end_y , logoSeg);
			break;

			//Segment 3
			case 2:
				j = MI_LOGO_W*LOG_SEG_H - LOG_SEG_W; //Advance the iterator for the next segment
				esp_lcd_panel_draw_bitmap(panel_handle, MI_LOGO_OFFSET_X , MI_LOGO_OFFSET_Y+LOG_SEG_H , end_x , end_y+LOG_SEG_H , logoSeg);
			break;

			//Segment 4
			case 3:
				esp_lcd_panel_draw_bitmap(panel_handle, MI_LOGO_OFFSET_X+LOG_SEG_W , MI_LOGO_OFFSET_Y+LOG_SEG_H , end_x+LOG_SEG_W , end_y+LOG_SEG_H , logoSeg);
				break;
			default:
				break;
		}

	}

	//Buffers are erase from memory afterwards.
	free(logoBuff);
	free(logoSeg);
}
/******************************************************************************
 * @brief       drawPixel
 * @param		draw_x, draw_y - The coordinates of the pixel to be drawn
 * 				colour - Pixel colour in colour 565 format
 * @return      None
 * @details     Fill the screen with black
 *****************************************************************************/
void drawPixel(const uint8_t draw_x, const uint8_t draw_y, const uint16_t colour)
{
	esp_lcd_panel_draw_bitmap(panel_handle , draw_x , draw_y , draw_x+1 , draw_y+1 , &colour);
}
/******************************************************************************
 * @brief       drawText
 * @param		draw_x, draw_y - The coordinates of the text to be drawn
 * @return      None
 * @details     Print text on LCD. Note that the fonts are bitmap, thus it will
 * 				alway have a write background behind
 *****************************************************************************/
void drawText(const uint8_t draw_x, const uint8_t draw_y, const char* text)
{
	uint8_t start_x = draw_x;											//Store initial x position
	uint8_t start_y = draw_y;											//Store initial y position
	const uint16_t len = strlen(text);									//Obtain string length
	const uint16_t h = More_perfect_VGA_dark.chars[0].image->height;	//Get font height
	const uint16_t w = More_perfect_VGA_dark.chars[0].image->width;		//Get font width
	/*
	 * If no string is inserted, the function does not need to do anything
	 */
	if(len == 0 || text == 0)
	{
		return;
	}

	for(int i = 0; i< len;++i)
	{
		/*
		 * Because the array is counted from 0 but the first character is defined as 0x20 (SPACE) in ASCII.
		 * Thus it is essential to subtract 0x02 to locate the character.
		 */
		uint16_t iterator = text[i] - 0x20;

		/*
		 * If encounters new line control character
		 * Immediately jump to the next iteration without drawing
		 */
		if(text[i] == '\n')
		{
			start_x = draw_x;
			start_y += h;
			continue;
		}
		/*
		 * Check where the iterator is out of array boundary. If it is true, it is assumed that the character
		 * does not defined in the font file.
		 * The drawing will not proceed.
		 */
		if(iterator < More_perfect_VGA_dark.length)
		{
			tImage* ci = More_perfect_VGA_dark.chars[iterator].image;				//Get image structure
			uint16_t* character = ci->data;							//Get font data in colour565 format

			esp_lcd_panel_draw_bitmap(panel_handle , start_x , start_y , start_x+w , start_y+h , character);	//Draw character
			/*
			 * If drawn outside the x boundary, move the text to the next line.
			 * Stop writing when outside the LCD boundary.
			 */
			if(start_x + w > LCD_V_RES)
			{
				/*
				 * When there is no line available, stop drawing
				 */
				if(start_y+h > LCD_H_RES)
				{
					return;
				}
				start_x = draw_x;																				//Reset x cursor to original position
				start_y += h;																					//Move y cursor to next line
			}
			else
			{
				start_x += w;																					//Go to next character
			}

		}

	}

}


/******************************************************************************
 * @brief       fillScreen
 * @param		None
 * @return      None
 * @details     Fill the screen with black
 *****************************************************************************/
void fillScreen(const uint16_t colour)
{
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
	//Allocate display buffer in memory
	uint16_t* bg = heap_caps_malloc(BG_SEG*BG_SEG*sizeof(uint16_t), MALLOC_CAP_SPIRAM);
#endif
	//Fill display buffer
	for(uint16_t i = 0 ;i<BG_SEG*BG_SEG; ++i)
	{
		bg[i] = colour;
	}

	//Draw the background segment by segment
	for(uint8_t i = 0;i<(LCD_V_RES/BG_SEG);++i)
	{
		for(uint8_t j = 0;j<(LCD_H_RES/BG_SEG);++j)
		{
			esp_lcd_panel_draw_bitmap(panel_handle , BG_SEG*i , BG_SEG*j , BG_SEG*(i+1) , BG_SEG*(j+1) , bg);
		}
	}
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
	free(bg);	//Free background
#endif
}


/******************************************************************************
 * @brief       drawIcon
 * @param		draw_x, draw_y - Coordinates
 * @return      None
 * @details     Draw icon on screen. The icons are defined in "icons.h"
 *****************************************************************************/
void drawIcon(const uint8_t draw_x, const uint8_t draw_y, const uint8_t icon)
{
	uint16_t height = 0;
	uint16_t width = 0;
	switch(icon)
	{
		case ICON_AP_MODE:
			height = icon_apmode.height;
			width = icon_apmode.width;
			esp_lcd_panel_draw_bitmap(panel_handle , draw_x , draw_y , draw_x+width , draw_y+height , icon_apmode.data);
			break;
		//Cannot connect to SenXor
		case ICON_ERROR:
			height = icon_error.height;
			width = icon_error.width;
			esp_lcd_panel_draw_bitmap(panel_handle , draw_x , draw_y , draw_x+width , draw_y+height , icon_error.data);
			break;
		//Pending connection
		case ICON_PENDING:
			height = icon_pending.height;
			width = icon_pending.width;
			esp_lcd_panel_draw_bitmap(panel_handle , draw_x , draw_y , draw_x+width , draw_y+height , icon_pending.data);
			break;
		//Client connected
		case ICON_CLIENT_CONNECTED:
			height = icon_client_connected.height;
			width = icon_client_connected.width;
			esp_lcd_panel_draw_bitmap(panel_handle , draw_x , draw_y , draw_x+width , draw_y+height , icon_client_connected.data);
			break;

		case ICON_INITIALISING:
			height = icon_initialising.height;
			width = icon_initialising.width;
			esp_lcd_panel_draw_bitmap(panel_handle , draw_x , draw_y , draw_x+width , draw_y+height , icon_initialising.data);
			break;
		case ICON_WIFI_FAIL:
			height = icon_wifi_fail.height;
			width = icon_wifi_fail.width;
			esp_lcd_panel_draw_bitmap(panel_handle , draw_x , draw_y , draw_x+width , draw_y+height , icon_wifi_fail.data);
			break;
		default:
			break;
	}

}
#endif
