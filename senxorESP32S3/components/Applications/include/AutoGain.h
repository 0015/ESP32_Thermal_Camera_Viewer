#ifndef AutoGain_H_
#define AutoGain_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defines.h"
#include "SenXorLib.h"


enum PRESETS {
	PRESET_OFF=0,
  PRESET_AUTO,
  PRESET_1,
  PRESET_2,
	PRESET_3,
	PRESET_4,
	PRESET_5,
	PRESET_6,
	PRESET_7,
	PRESET_8,
	PRESET_9,
	PRESET_10,
	PRESET_11,
	PRESET_12,
	PRESET_13,
	PRESET_14,
};


void AutoGain(void);


#endif
