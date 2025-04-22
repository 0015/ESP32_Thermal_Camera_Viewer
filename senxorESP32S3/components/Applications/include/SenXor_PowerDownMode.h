#ifndef __SENXOR_POWERDOWNMODE_H__
#define __SENXOR_POWERDOWNMODE_H__

#include "defines.h"
#include <stdint.h>


enum SenXor_PowerModeChangeClk {
   CHANGE_CLK_COMPLETE,	
   CHANGE_CLOCK_192MHZ,
   CHANGE_CLOCK_80MHZ,
   
};
 
enum SenXor_CpuSenxorPowerMode {
   CPU_SENXOR_RUNNING,
   CPU_SENXOR_POWERDOWN,
   CPU_SENXOR_POWERDOWN_USB,	 
   CPU_SENXOR_WAITWAKEUP,
   CPU_SENXOR_WAKEUP,
};

enum SenXor_CpuSenxorSleepMode {
   CPU_WAKEUP_RUNNING,
   CPU_CAPTURE,	
   CPU_SLEEP_POWERDOWN,
   CPU_SLEEP_POWERDOWN_SPI,	
   CPU_SLEEP_POWERDOWN_USB_WAIT,
   CPU_SLEEP_POWERDOWN_USB_WAKE, 
   CPU_SLEEP_POWERDOWN_USB_STOP,
   CPU_PARALLEL,
   CPU_WAKEUP,
};
void PowerDownCheck(void);
extern uint8_t PowerMode;

#endif //__SENXOR_POWERDOWNMODE_H__
