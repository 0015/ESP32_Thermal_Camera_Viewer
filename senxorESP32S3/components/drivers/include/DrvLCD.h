#ifndef COMPONENTS_DRIVERS_INCLUDE_DRVLCD_H_
#define COMPONENTS_DRIVERS_INCLUDE_DRVLCD_H_

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "sdkconfig.h"
#include "esp_lcd_gc9a01.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_log.h"
#include "esp_system.h"

#include "driver/spi_master.h"
#include "driver/ledc.h"
#include "DrvGPIO.h"
#include "msg.h"
#include "util.h"

/******************************************************************************
 * @brief       LCD specification
 *****************************************************************************/
//A - COLOUR AND RESOLUTION
#define LCD_H_RES 240				//Number of horizontal pixels
#define LCD_V_RES 240				//Number of vertical pixels
#define LCD_COLOUR_BIT	16			//Number of bits per pixel
#define LCD_PIX LCD_H_RES*LCD_V_RES //LCD Resolution


//B - DATA
#define LCD_CMD_BITS	8			//Command bit length
#define LCD_PARAM_BITS  8			//Parameter bit length
#define BG_SEG 80					//Background segments
#define LOG_SEG_H 34				//Height for a segment containing some part of logo
#define LOG_SEG_W 80				//Width for a segment containing some part of logo
#define LCD_BUFF_PIX_SENXOR	4960	//Number of pixel reserves for SenXor
#define LCD_SENXOR_FRAME_H 62		//Height of a SenXor's frame
#define LCD_SENXOR_FRAME_W 80		//Width of a SenXor's frame

//C - Hardware specification
#define LCD_PIXEL_CLK	(80 * 1000 * 1000)

#ifndef CONFIG_MI_LCD_CUSTOM_PINS
#ifdef CONFIG_MI_ST7789
#define LCD_BLK		-1
#define LCD_CLK		21
#define LCD_CS		44
#define LCD_DC		43
#define LCD_RST		-1
#define LCD_MOSI	47
#define LCD_MISO	-1
#endif

#ifdef CONFIG_MI_GC9A01
#define LCD_BLK		4
#define LCD_CLK		21
#define LCD_CS		40
#define LCD_DC		13
#define LCD_RST		12
#define LCD_MOSI	47
#define LCD_MISO	-1
#endif

#else
#define LCD_BLK		CONFIG_MI_LCD_BLK
#define LCD_CLK		CONFIG_MI_LCD_CLK
#define LCD_CS		CONFIG_MI_LCD_CS
#define LCD_DC		CONFIG_MI_LCD_DC
#define LCD_RST		CONFIG_MI_LCD_RST
#define LCD_MOSI	CONFIG_MI_LCD_MOSI
#define LCD_MISO	CONFIG_MI_LCD_MISO
#endif

//D - SPI PIN
/*
 * GC9A01		ST7789
 * CLK 21		21
 * MOSI 47		47
 * MISO -1		-1
 * CS 40		44
 * BLK 4		-1
 * DC 13		43
 * RST 12		-1
 */
//Colour definition, in BRG
#define DARK_GREEN colour565(0x2F,0x00,0xE5)
#define GREEN colour565(0x00, 0x00, 0xFF)

#define RED colour565(0x00, 0xFF, 0x00)
#define BLUE colour565(0xFF, 0x00, 0x00)
#define STD_BLUE colour565(0xFF, 0x00, 0x59)
#define WHITE colour565(0xFF, 0xFF, 0xFF)
#define BLACK colour565(0x00, 0x00, 0x00)
#define YELLOW colour565(0x00, 0xFF, 0xE0)

/******************************************************************************
 * @brief       Shape specification
 *****************************************************************************/
#define CROSS_THINKNESS 5
#define CROSS_LENGTH	50
/******************************************************************************
 * @brief       Function definition
 *****************************************************************************/
//public:

void LCDInit(void);

void setLCDBrightness(const float brightness);

void displayLogo(void);

void drawIcon(const uint8_t draw_x, const uint8_t draw_y, const uint8_t icon);

void drawPixel(const uint8_t draw_x, const uint8_t draw_y, const uint16_t colour);

void drawText(const uint8_t draw_x, const uint8_t draw_y, const char* text);

void fillScreen(const uint16_t colour);

#endif /* COMPONENTS_DRIVERS_INCLUDE_DRVLCD_H_ */
