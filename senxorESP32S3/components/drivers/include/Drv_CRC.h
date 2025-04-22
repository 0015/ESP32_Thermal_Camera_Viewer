#ifndef COMPONENTS_DRIVERS_INCLUDE_DRV_CRC_H_
#define COMPONENTS_DRIVERS_INCLUDE_DRV_CRC_H_

#include <stdint.h>

/******************************************************************************
 * @brief       Function prototyping
 *****************************************************************************/
void Drv_Crc_Open(void);
void Drv_Crc_crc32_open(void);
uint16_t Drv_Crc_WriteCRC(uint16_t data);
uint32_t Drv_Crc_GetCRCcheckSum(void);
extern uint32_t CalData_CRC;


#endif /* COMPONENTS_DRIVERS_INCLUDE_DRV_CRC_H_ */
