#ifndef COMPONENTS_UTIL_INCLUDE_UTIL_H_
#define COMPONENTS_UTIL_INCLUDE_UTIL_H_

#include <stdint.h>
#include <cJSON.h>
#include "esp_log.h"			//ESP logger
#include "esp_attr.h"
#include "DrvWLAN.h"
#include "MCU_Dependent.h"

#define colour565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))

typedef enum iconSet{
	ICON_AP_MODE,
	ICON_BATTERY,
	ICON_BATTERY_FULL,
	ICON_CHARGING,
	ICON_CLIENT_CONNECTED,
	ICON_ERROR,
	ICON_INITIALISING,
	ICON_PENDING,
	ICON_WIFI_FAIL
}iconSet;

typedef struct
{
    const uint16_t *data;
    uint16_t width;
    uint16_t height;
    uint8_t dataSize;
}tImage;

typedef struct{
    long int code;
    const tImage *image;
}tChar;

typedef struct{
    int length;
    const tChar *chars;
}tFont;


void printSenXorCaliData(const uint16_t *caliData);

void printSenXorData(const uint16_t *senData);

void printSenXorLog(const uint16_t *senData);

void displaySenxorInfo(void);

void getSysInfoJson(cJSON* jsonObj);

uint32_t getFrameAvg(uint16_t x, uint16_t y, uint16_t h, uint16_t w, const uint16_t* frame);

uint16_t getCRC(const uint8_t* pData, uint16_t pDataSize);
#endif /* COMPONENTS_UTIL_INCLUDE_MSG_H_ */
