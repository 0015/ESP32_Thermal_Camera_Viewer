/*****************************************************************************
 * @file     AutoGain.c
 * @version  V1.00
 * @brief    This file holds all the automatic and manual gain changing related functions
 *
 * @copyright (C) 2018 Meridian Innovation 
 ******************************************************************************/

#include "AutoGain.h"


const uint8_t MaxFrame = 100;
const uint8_t Temp_Arr_Size = 10;
uint8_t Gain_Switch_Completed = 0;
uint8_t PrevMode = 0;
uint8_t PrevIntergrator = 0;
uint8_t FrameCount = 0;
uint8_t AutoGainReady = 0;
uint8_t Init = 1;
uint8_t High_Res;
uint8_t B9_State;
int Firmware_Averaging = 0;
int Pix_Cnt = FRAMEWIDTH_BUF*(FRAMEHEIGHT_BUF-2);
int Pix_Max = 0;
uint16_t Temp_Arr[10];


int compare (const void * num1, const void * num2) {
   if(*(uint16_t*)num1 > *(uint16_t*)num2)
    return 1;
   else
    return -1;
}

/******************************************************************************
 * @brief       Get_Max
 * @param       Arr -> Array to be sorted
								len -> Lenght of array
 * @return      Avereged Max value in frame
 * @details     Calculated max value in frame ignoring dead pixels
 *****************************************************************************/
int Get_Max(uint16_t arr[], int len)
{
	int i,c=0;
	int start = 0;
	int stop = len-5;		// Remove top 5 pixels
	int sum = 0;
		
	qsort(arr, len, sizeof(uint16_t), compare);
	
	for(i = start; i < stop; i++)
	{
		sum+=arr[i];
		c++;
	}
	return sum/c;
}


/******************************************************************************
 * @brief       AutoGain
 * @param       Mode
 * @return      None
 * @details     Set PGA/LNA/INT Gain Automatically Or Manually, Auto scaling of max temperature
								display
 *****************************************************************************/

void AutoGain(void)
{
	uint8_t CaptureState = 0;
	uint8_t Mode = 0;
	int c = 0, i = 0;
	
	Firmware_Averaging = Acces_Read_Reg(0xB4);
	Mode = Acces_Read_Reg(0xB9) & 0x0F;
	High_Res = Acces_Read_Reg(0xB9) & 0x80;
	B9_State = Acces_Read_Reg(0xB9);
	
	if (Firmware_Averaging == 0)
		Firmware_Averaging = 1;
	
	FrameCount =  FrameCount * Firmware_Averaging;
	
	if (FrameCount > MaxFrame) // Delay to let frame settle
	{
		AutoGainReady = 1;
		FrameCount = MaxFrame;
	}
	
	if ((Mode != PrevMode || Mode == PRESET_AUTO) )// && AutoGainReady) // Mode Changed
	{
		PrevMode = Mode;		
				
		if (Mode == PRESET_AUTO && AutoGainReady) // Auto
		{
			GetTransmitFrameBuffer();
			
			for (i = FRAMEWIDTH_BUF*2 + 12; i < Pix_Cnt; i+=2)
			{
				if (c > Temp_Arr_Size-1)
					c = 0;
				if (TransmitFrame->TXBuf[i] > Temp_Arr[c])
					Temp_Arr[c] = TransmitFrame->TXBuf[i];
				c++;
			}
			
			Pix_Max = Get_Max(Temp_Arr, Temp_Arr_Size); // sort and get avg of max
			if (High_Res == 0x80)
			{
				Pix_Max = Pix_Max / 100 - 273;
			}
			else
			{
				Pix_Max = Pix_Max / 10 - 273;
			}
			

			if (Pix_Max < 100 && !(Gain_Switch_Completed == 1)) //under 100c
			{
				CaptureState = Acces_Read_Reg(0xB1); // Get capture state, Continues/Single/Paused
				Gain_Switch_Completed = 0; // reset flag
				
				B9_State &= ~(7 << 4); 
				
				Acces_Write_Reg(0xB1, 0x60); // stop capture
				Acces_Write_Reg(0x08, 0x14); // Set gain 1x 
				Acces_Write_Reg(0x0A, 0x03);
				Acces_Write_Reg(0xB9, B9_State);
				Acces_Write_Reg(0xB1, CaptureState); // Resume previus capture state
				
				Gain_Switch_Completed = 1;
			}
			else if ( ((Pix_Max > 120) && (Pix_Max < 180)) && !(Gain_Switch_Completed == 2)) // 120-180c Range
			{
				CaptureState = Acces_Read_Reg(0xB1); // Get capture state, Continues/Single/Paused
				Gain_Switch_Completed = 0; // reset flag
				
				B9_State &= ~(7 << 4);
				B9_State |= 1 << 4;
				
				Acces_Write_Reg(0xB1, 0x60); // stop capture
				Acces_Write_Reg(0x08, 0x14); // Set gain 0.5x 
				Acces_Write_Reg(0x0A, 0x01);
				Acces_Write_Reg(0xB9, B9_State);
				Acces_Write_Reg(0xB1, CaptureState); // Resume previus capture state

				Gain_Switch_Completed = 2;
			}
			else if ( ((Pix_Max > 200) && (Pix_Max < 400)) && !(Gain_Switch_Completed == 3)) // 200-400c Range
			{
				CaptureState = Acces_Read_Reg(0xB1); // Get capture state, Continues/Single/Paused
				Gain_Switch_Completed = 0; // reset flag
				
				B9_State &= ~(7 << 4); 
				B9_State |= 2 << 4;
				
				Acces_Write_Reg(0xB1, 0x60); // stop capture
				Acces_Write_Reg(0x08, 0x04); // Set Gain 0.25x
				Acces_Write_Reg(0x0A, 0x01);
				Acces_Write_Reg(0xB9, B9_State);
				Acces_Write_Reg(0xB1, CaptureState); // Resume previus capture state
				
				Gain_Switch_Completed = 3;
			}
			
			FrameCount = 0;
			AutoGainReady = 0;
			
			for (i = 0; i < Temp_Arr_Size; i++)
			{
					Temp_Arr[i] = 0;
			}
			
		}
		else if (Mode > PRESET_AUTO)
		{
			CaptureState = Acces_Read_Reg(0xB1); // Get capture state, Continues/Single/Paused

			Acces_Write_Reg(0xB1, 0x60); // stop capture
			
			if (Mode == PRESET_1) 	{Acces_Write_Reg(0x08, 0x04); Acces_Write_Reg(0x0A, 0x01);}	// Set gain 0.25x
			else if (Mode == PRESET_2) 	{Acces_Write_Reg(0x08, 0x14); Acces_Write_Reg(0x0A, 0x01);}	// Set gain 0.5x
			else if (Mode == PRESET_3) 	{Acces_Write_Reg(0x08, 0x14); Acces_Write_Reg(0x0A, 0x03);}	// Set gain 1x
			else												{Acces_Write_Reg(0x08, 0x14); Acces_Write_Reg(0x0A, 0x03);}	// Set gain 1x
					
			Acces_Write_Reg(0xC5, 0x42); // Init Blind
			Acces_Write_Reg(0xB1, CaptureState); // Resume previus capture state
		}
		
		if (Init != 0)
		{
			CaptureState = Acces_Read_Reg(0xB1); // Get capture state, Continues/Single/Paused
			Acces_Write_Reg(0xB1, 0x60); // stop capture
			Acces_Write_Reg(0xC5, 0x42); // Init Blind
			Acces_Write_Reg(0xB1, CaptureState); // Resume previus capture state
			Init = 0;
		}
		
		if (Mode == PRESET_OFF) //Off
		{
				CaptureState = Acces_Read_Reg(0xB1); // Get capture state, Continues/Single/Paused
				
				Acces_Write_Reg(0xB1, 0x60); // stop capture
				Acces_Write_Reg(0x08, 0x14); // Set Gain default
				Acces_Write_Reg(0x0A, 0x03); // Set Gain default
				Acces_Write_Reg(0xC5, 0x42); // Init Blind
				Acces_Write_Reg(0xB1, CaptureState); // Resume previus capture state
				
				Init = 1;
				Gain_Switch_Completed = 0;
		}
	}
}
