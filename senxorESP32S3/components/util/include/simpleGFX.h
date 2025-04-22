/*****************************************************************************
 * @file     simpleGFX.h
 * @version  1.01
 * @brief    Provide functions to process the thermal image
 * @date	 7 Dec 2022
 ******************************************************************************/
#ifndef COMPONENTS_UTIL_INCLUDE_SIMPLEGFX_H_
#define COMPONENTS_UTIL_INCLUDE_SIMPLEGFX_H_

#include <stdint.h>
#include "colourmap.h"
#include "DrvLCD.h"
#include "sdkconfig.h"
#define colour565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))

uint16_t* colourMapping(uint16_t *temperateData , uint16_t minTemperature, uint16_t maxTemperature, const uint8_t selectedColourMapIndex);

void drawCrossByOverlay(uint16_t* srcImg, const uint16_t colour, const uint8_t scale);

void flipImg(uint16_t *unscaledImg);

void resizeBilinearGray(const uint16_t *pixels, uint16_t *output, const uint16_t w, const uint16_t h, const uint16_t w2, const uint16_t h2);

void resizeNearestrGray(const uint16_t *pixels, uint16_t *output, const uint16_t w1,const uint16_t h1,const uint16_t w2,const uint16_t h2);

void send2DispBuff(const uint16_t* scaled, uint16_t* dispBuff[], const uint8_t scale);

#endif /* COMPONENTS_UTIL_INCLUDE_SIMPLEGFX_H_ */
