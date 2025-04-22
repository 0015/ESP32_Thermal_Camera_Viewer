/******************************************************************************
 * @file     SenXor_PowerMode.c
 * @version  V1.00
 * @brief    This file holds function that slow down the speed to power consumption
			 
 *
 * @copyright (C) 2018 Meridian Innovation 
 ******************************************************************************/
 
#include "SenXorLib.h"
#include "SenXor_PowerDownMode.h"
#include "DrvTimer.h"
#include "Customer_Interface.h"
#include "defines.h"
#include "DrvGPIO.h"

uint8_t PowerMode=CHANGE_CLK_COMPLETE;
uint8_t PowerModeCurrentStatus=0;
extern uint8_t SleepModeStatus;

void TimePowerDown(void);
void PowerDownChangeClock(void);
void CPU_SENXORPowerDown(void);
void TimePowerDown(void);
void PowerOffModule(void);


/******************************************************************************
 * @brief       PowerDownCheck
 * @param       None				
 * @return      None
 * @details     check the status of the clock selected
 *****************************************************************************/
void PowerDownCheck(void)
{
#ifdef POWERDOWN_FEATURE
	PowerDownChangeClock();	
	TimePowerDown();
	if (PCB_Type == PCB_TYPE_MI48TL)
	{
		CPU_SENXORPowerDown();
	}
#endif
}

#ifdef POWERDOWN_FEATURE
/******************************************************************************
 * @brief       PowerDownChangeClock
 * @param       None				
 * @return      None
 * @details     Change clock 
 *****************************************************************************/
void PowerDownChangeClock(void)
{
	switch(PowerModeStatus)
	{
		case CHANGE_CLOCK_192MHZ:
			Halt_Capture();	
			Drv_PowerDown_SpeedUpMCU();
			PowerMode = CHANGE_CLOCK_192MHZ;				
			PowerModeStatus = CHANGE_CLK_COMPLETE;
		break;
		
		case CHANGE_CLOCK_80MHZ:				
			Halt_Capture();
			Drv_PowerDown_SpeedDownMCU();	

			PowerMode = CHANGE_CLOCK_80MHZ;				
			PowerModeStatus = CHANGE_CLK_COMPLETE;
		break;
		
		default:
		case CHANGE_CLK_COMPLETE:
		break;	
	}
}	

/******************************************************************************
 * @brief       PowerDownCheck
 * @param       None				
 * @return      None
 * @details     check the status of the clock selected
 *****************************************************************************/

void CPU_SENXORPowerDown(void)
{

	switch(PowerModeDownStatus)
	{
		case CPU_SENXOR_POWERDOWN:	
			Halt_Capture();							
			PowerOffModule();
			Drv_PowerDown_McuSleep();					
			Initialize_SenXor(1);

			Drv_Timer_TimerDelay(50000); 										//to avoid a line every 4  vertical


			PowerModeDownStatus = CPU_SENXOR_RUNNING;
		break;
			
		default:
		break;

	}
} 





/******************************************************************************
 * @brief       TimePowerDown
 * @param       None				
 * @return      None
 * @details     Time Power Down mode
 *****************************************************************************/
void TimePowerDown(void)
{
//	uint32_t Time;
	uint8_t Data_B1=0;

	switch(SleepModeStatus)
	{
		case CPU_WAKEUP_RUNNING:
			if(MCU_REGISTER.MCU_REG_B1.Set.Single_Continious == 1)
			{
				if((MCU_REGISTER.MCU_REG_B5.Set.Time != 0) && (MCU_REGISTER.MCU_REG_B5.Set.LowPower ==0))
				{
					if (PCB_Type == PCB_TYPE_MI48TL)
					{
						PowerModeDownStatus = CPU_SENXOR_RUNNING;					//start trigge to start capture
						Data_B1 = Acces_Read_Reg(0xB1);
						Data_B1 &= ~B1_SINGLE_CONT;
						Data_B1 |= B1_START_CAPTURE;		
						Acces_Write_Reg(0xB1, Data_B1);
						MCU_REGISTER.MCU_REG_B1.Set.Single_Continious=1;
						SleepModeStatus =	CPU_CAPTURE;
						Drv_Timer_Start_TimeOut_Timer_mSec(0);						//start 1410ms
					}
				}
				else if(MCU_REGISTER.MCU_REG_B7.Set.Parallel_Process == 0)
				{
					SleepModeStatus = CPU_PARALLEL;
					Data_B1 = Acces_Read_Reg(0xB1);
					Data_B1 &= ~B1_SINGLE_CONT;
					Data_B1 |= B1_START_CAPTURE;		
					Acces_Write_Reg(0xB1, Data_B1);	
					MCU_REGISTER.MCU_REG_B1.Set.Single_Continious=1;					
					Drv_Timer_Start_TimeOut_Timer_mSec(0);						//start 1410ms
				}

			}
			break;
		case CPU_PARALLEL:
			if ((Acces_Read_Reg(0xB1)&B1_SINGLE_CONT)|| (Acces_Read_Reg(0xB1) & B1_START_CAPTURE) )				
			{
				;
			}
			else
			{
				SleepModeStatus =	CPU_WAKEUP_RUNNING;
			}			
			break;

		case CPU_CAPTURE:
			if(MCU_REGISTER.MCU_REG_B5.Set.Time != 0)									//check finish capture
			{
				if((Acces_Read_Reg(0xB1)&B1_SINGLE_CONT)==B1_SINGLE_CONT)
				{
					SleepModeStatus =	CPU_WAKEUP_RUNNING;
				}
				else if ( (Acces_Read_Reg(0xB1)&B1_SINGLE_CONT)|| (Acces_Read_Reg(0xB1) & B1_START_CAPTURE) )
				{
					;
				}
				else
				{
					SleepModeStatus =	CPU_SLEEP_POWERDOWN;
				}
			}	
			else
			{
				SleepModeStatus =	CPU_WAKEUP_RUNNING;
			}
		break;	
		case CPU_SLEEP_POWERDOWN:										//start sleep
            Drv_PowerDown_SleepWakeupByTimer();

			SleepModeStatus = CPU_WAKEUP_RUNNING;
	
			break;
	
		case CPU_SLEEP_POWERDOWN_USB_STOP:										//start wake up
			SleepModeStatus =	CPU_WAKEUP_RUNNING;

		default:
		break;

	}
}

#endif

