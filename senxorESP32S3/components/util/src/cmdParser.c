/*****************************************************************************
 * @file     cmdParser.c
 * @version  1.00
 * @brief    Parse command from input
 * @date	 26 Apr 2024
 * @author	 Meridian Innovations
 ******************************************************************************/
#include "cmdParser.h"
#include "SenXorLib.h"

//public:
extern int ApplicationReadVersion (int Address);
/******************************************************************************
 * @brief       getHexValue
 * @param       c - Data to be converted
 * @return      Character of the data
 * @details     Convert hexadecimal value to decimal
 *****************************************************************************/
int getHexValue(const char c)
{
	if((c >= '0') && (c <='9'))
	{
	    return c - '0';
	}
	if((c >= 'A') && (c <= 'F'))
	{
	    return c -'A' + 10;
	}
	return LENGTH_PARSE_ERROR;
}

/******************************************************************************
 * @brief       toHex
 * @param       str - String contains the data
 * @return		Hexadecimal representation of the string
 * @details     Interpret hexadecimal number from string
 *****************************************************************************/
int toHex(const char* pStr)
{
  int result = 0;
  const int length = strlen(pStr);								//Obtains the length
  //For each elements inside the buffer
  //Convert it to hexadecimal value
  for(int i = 0; i < length; i++)
  {
    int val = getHexValue(pStr[i]);
    if(val == LENGTH_PARSE_ERROR)
	{
      return LENGTH_PARSE_ERROR;
    }
    result += (int)(val * pow(16, length-i-1));
  }
  return result;
}// toHex

 /******************************************************************************
 * @brief       cmdParser_Init
 * @param       pCmdPhaser - cmdParser object
 * @return      none
 * @details     Initialise cmdParser object. 
 * 				This will erase all the data stored in the object
 *****************************************************************************/
void cmdParser_Init(cmdPhaser* pCmdPhaser)
{
	// Do not execute if null object is passed.
	if(!pCmdPhaser)
	{
		return;
	}// End if
	pCmdPhaser->mCmdParserState = START_CHAR;
	memset(pCmdPhaser->mCmd,0,CP_CMD_FIELD_LEN);
	memset(pCmdPhaser->mCmdLen,0,CP_DLEN_FIELD_LEN);
	memset(pCmdPhaser->mData,0,CP_DATA_FIELD_LEN);
	memset(pCmdPhaser->mCRC,0,CP_CRC_FIELD_LEN);
}// cmdParser_Init

/******************************************************************************
 * @brief       cmdParser
 * @param       pCmdPhaser - cmdParser object
 * 				pInput - Input string
				pInputSize - Size of the input string
 * @return      none
 * @details     Pharse the command and data in a given string
 *****************************************************************************/
int cmdParser_PharseCmd(cmdPhaser* pCmdPhaser, const uint8_t* pInput, size_t pInputSize)
{
	uint32_t tCmdPtr = 0;
	uint32_t tInputChksum = 0;
	uint32_t tCalChksum = 0;
	uint32_t tCmdLen = 0;

#if CONFIG_MI_EVK_CP_DBG
	ESP_LOGI(CPTAG,CP_INFO_START);
	ESP_LOGI(CPTAG,"%s",pInput);
#endif
	if(!pInput)
	{
		#if CONFIG_MI_EVK_CP_DBG	
		ESP_LOGE(CPTAG,CP_ERR_INPUT_NULL);
		#endif
		return -1;
	}

	if(!pCmdPhaser)
	{
		#if CONFIG_MI_EVK_CP_DBG	
		ESP_LOGE(CPTAG,CP_ERR_OBJ_NULL);
		#endif
		return -1;
	}

	if(pInputSize > (CP_CMD_FIELD_LEN+CP_DLEN_FIELD_LEN+CP_DATA_FIELD_LEN+CP_CRC_FIELD_LEN))
	{
		
		pInputSize = (CP_CMD_FIELD_LEN+CP_DLEN_FIELD_LEN+CP_DATA_FIELD_LEN+CP_CRC_FIELD_LEN);
		#if CONFIG_MI_EVK_CP_DBG	
		ESP_LOGW(CPTAG,CP_WARN_INPUT_LEN_OVER,pInputSize);
		#endif
	}

	for (size_t i = 0 ; i < pInputSize ; ++i)
	{
		switch (pCmdPhaser->mCmdParserState)
		{
			case START_CHAR:
			#if CONFIG_MI_EVK_CP_DBG
				ESP_LOGI(CPTAG,CP_INFO_STAGE_1);
			#endif
				// Search for start character '#'
				if(pInput[i] == CP_START_CHAR)
				{
					pCmdPhaser->mCmdParserState = LEN;
				}// End if
			break;
			// Command length
			case LEN:
				pCmdPhaser->mCmdLen[tCmdPtr++] = pInput[i];
				tInputChksum += pInput[i];
			#if CONFIG_MI_EVK_CP_DBG
				ESP_LOGI(CPTAG,CP_INFO_STAGE_2);
			#endif
				if(tCmdPtr == CP_CMD_FIELD_LEN - 1)
				{
					tCmdLen = strtoul((char*)pCmdPhaser->mCmdLen,0,16);
					if (tCmdLen > 7) 
					{
						tCmdPtr = 0;
						pCmdPhaser->mCmdParserState = DATA;
					}// End if

					// Rejecting unqualified commands
					if ((tCmdLen < 8)||(tCmdLen==0xffffffff))
					{
						tCmdPtr = 0;
						pCmdPhaser->mCmdParserState = START_CHAR;
					#if CONFIG_MI_EVK_CP_DBG		
						ESP_LOGE(CPTAG,CP_ERR_CMD_LEN);
					#endif
					}// End if
					
				}// End if
				
			break;
				
			// Data + cmd
			case DATA:
				#if CONFIG_MI_EVK_CP_DBG	
				ESP_LOGI(CPTAG,CP_INFO_STAGE_3);
				#endif
				tInputChksum += pInput[i];    

				// Fetch commands first (4 bytes)
				if(tCmdPtr < CP_CMD_FIELD_LEN - 1)
				{
					pCmdPhaser->mCmd[tCmdPtr++] = pInput[i];

				}
				else
				{
					pCmdPhaser->mData[tCmdPtr - (CP_CMD_FIELD_LEN - 1)] = pInput[i];
					tCmdPtr++;
				}// End if-else

				                        
				if (tCmdPtr == tCmdLen - 4 || tCmdPtr == CP_CMD_FIELD_LEN + CP_DATA_FIELD_LEN)
				{
					pCmdPhaser->mCmdParserState = CRC;
					tCmdPtr = 0;
				}//End if
				
				
			break;

			case CRC:
				#if CONFIG_MI_EVK_CP_DBG	
				ESP_LOGI(CPTAG,CP_INFO_STAGE_4);
				#endif
				pCmdPhaser->mCRC[tCmdPtr++] = pInput[i];
				if(tCmdPtr == CP_CRC_FIELD_LEN - 1)
				{ 
					tCalChksum = strtoul((char*)pCmdPhaser->mCRC,0,16);
					tCmdPtr = 0;
					if((pCmdPhaser->mCRC[0] == 'X' &&pCmdPhaser->mCRC[1] == 'X' &&pCmdPhaser->mCRC[2] == 'X' && pCmdPhaser->mCRC[3] == 'X'))
					{
						#if CONFIG_MI_EVK_CP_DBG	
						ESP_LOGW(CPTAG,CP_WARN_INPUT_CRC_NULL);
						#endif
						return 0;
					}

					if(tInputChksum ==  tCalChksum)
					{
						#if CONFIG_MI_EVK_CP_DBG
						ESP_LOGI(CPTAG,CP_INFO_CRC_OK);
						#endif
						return 0;
					}
					else
					{
						#if CONFIG_MI_EVK_CP_DBG	
						ESP_LOGE(CPTAG,CP_ERR_CRC_FAIL);
						ESP_LOGE(CPTAG,CP_ERR_CRC_FAIL_INFO, tCalChksum,tInputChksum);
						#endif
						return -1;
					}// End if-else
				}// End if
				
			break;

			// Reset phaser for any invalid state.
			default:
				#if CONFIG_MI_EVK_CP_DBG	
				ESP_LOGE(SPTAG,CP_ERR_STATE);
				#endif
				pCmdPhaser->mCmdParserState = START_CHAR;
				cmdParser_Init(pCmdPhaser);
			break;
		}//End switch
	}
	return 0;
}// cmdParser_PharseCmd


 /******************************************************************************
 * @brief       cmdParser_CommitCmd
 * @param       pCmdPhaser - cmdParser object
 * 				pAckBuff - Pointer to ACK buffer
 * @return      Length of ACK buffer
 * @details     Execute commands and construct ACK messages
 *****************************************************************************/
uint8_t cmdParser_CommitCmd(const cmdPhaser* pCmdPhaser, uint8_t* pAckBuff)
{
	char tAddr[3];
	char tVal[3];
	int tAddrInt = 0;
	int tValInt = 0;
	uint32_t tCmdLenInt = strtoul((char*)pCmdPhaser->mCmdLen,0,16);
	
	if(!pAckBuff)
	{
		return 0;
	}// End if

	if(!pCmdPhaser)
	{
		return 0;
	}// End if

	if(!strcmp((char*) pCmdPhaser->mCmd,CMD_WREG))
	{
#if CONFIG_MI_EVK_CP_DBG	
		ESP_LOGI(CPTAG,SP_CMD_WREG_INFO);
#endif
		tAddr[0] = pCmdPhaser->mData[0];
		tAddr[1] = pCmdPhaser->mData[1];
		tAddr[2] = 0;

		tVal[0] = pCmdPhaser->mData[2];
		tVal[1] = pCmdPhaser->mData[3];
		tVal[2] = 0;

		tAddrInt = toHex((char*)tAddr);
		tValInt = toHex((char*)tVal);
		Acces_Write_Reg(tAddrInt,tValInt);
		
		pAckBuff[0]=' ';
		pAckBuff[1]=' ';
		pAckBuff[2]=' ';
		pAckBuff[3]='#';
		pAckBuff[4]='0';
		pAckBuff[5]='0';
		pAckBuff[6]='0';
		pAckBuff[7]='8';
		pAckBuff[8]='W';
		pAckBuff[9]='R';
		pAckBuff[10]='E';
		pAckBuff[11]='G';
		sprintf((char *)&pAckBuff[12], "%04X", getCRC(pAckBuff+4,8));	// Add CRC field
		pAckBuff[17]=0;
		return 17;
	}
	else if(!strcmp((char*) pCmdPhaser->mCmd,CMD_RREG))
	{
#if CONFIG_MI_EVK_CP_DBG	
		ESP_LOGI(CPTAG,SP_CMD_RREG_INFO);
#endif
		uint8_t rd = 0;
		tAddr[0] = pCmdPhaser->mData[0];
		tAddr[1] = pCmdPhaser->mData[1];
		tAddr[2] = 0;
		tAddrInt = toHex((char*)tAddr);
		if(tAddrInt == 0xB2 || tAddrInt == 0xB3)
		{
			rd = ApplicationReadVersion(tAddrInt);
		}
		else 
		{
			rd = Acces_Read_Reg(tAddrInt);
		}
		

		pAckBuff[0]=' ';
		pAckBuff[1]=' ';
		pAckBuff[2]=' ';
		pAckBuff[3]='#';
		pAckBuff[4]='0';
		pAckBuff[5]='0';
		pAckBuff[6]='0';
		pAckBuff[7]='A';
		pAckBuff[8]='R';
		pAckBuff[9]='R';
		pAckBuff[10]='E';
		pAckBuff[11]='G';

		sprintf((char *)&pAckBuff[12], "%02X", rd);
		sprintf((char *)&pAckBuff[14], "%04X", getCRC(pAckBuff+4,10));	// Add CRC fieldBuff, 18);
		pAckBuff[18]=0;
		return 19;
	}
	else if (!strcmp((char*) pCmdPhaser->mCmd,CMD_RRSE))
	{
		uint8_t tRd = 0;
		uint16_t tAckLen = (tCmdLenInt - 8 - 2)*2 + 8;  // Len+CMD=8, lastFF=2
		uint16_t tRegCntX2 = (tCmdLenInt - 8 - 2);
		uint16_t j = 12;

		pAckBuff[0]=' ';
		pAckBuff[1]=' ';
		pAckBuff[2]=' ';
		pAckBuff[3]='#';

		sprintf((char *)&pAckBuff[4], "%04X", tAckLen);												// Add Length

		pAckBuff[8]='R';
		pAckBuff[9]='R';
		pAckBuff[10]='S';
		pAckBuff[11]='E';

		for (size_t i = 0; i < tRegCntX2; i+=2,j+=4)
		{
			tAddr[0] = pCmdPhaser->mData[i];
			tAddr[1] = pCmdPhaser->mData[i+1];
			tAddr[2] = 0;
			tAddrInt = toHex((char*)tAddr);
			if(tAddrInt == 0xB2 || tAddrInt == 0xB3)
			{
				tRd = ApplicationReadVersion(tAddrInt);
			}
			else 
			{
				tRd = Acces_Read_Reg(tAddrInt);
			}

			sprintf((char *)&pAckBuff[j], "%02X", tAddrInt);
			sprintf((char *)&pAckBuff[j+2], "%02X", tRd);
		}// End for
		
		sprintf((char *)&pAckBuff[j], "%04X", getCRC(pAckBuff+4,tAckLen));				// Add CRC 

		return tAckLen + 8;
	}
	else
	{
		ESP_LOGE(CPTAG,CP_ERR_CMD_INVALID);
		return 0;
	}//End if-else

}// cmdParser_CommitCmd

 /******************************************************************************
 * @brief       cmdParser_PrintResult
 * @param       pCmdPhaser - cmdParser object
 * @return      none
 * @details     Print the contents inside a cmdParser object
 * *****************************************************************************/
void cmdParser_PrintResult(const cmdPhaser* pCmdPhaser)
{
	if(!pCmdPhaser)
	{
		ESP_LOGE(CPTAG,CP_ERR_OBJ_NULL);
	}// End if

	ESP_LOGI(CPTAG,"Phaser state: %d", pCmdPhaser->mCmdParserState);
	ESP_LOGI(CPTAG,"Phased command length: %s", pCmdPhaser->mCmdLen);
	ESP_LOGI(CPTAG,"Phased command: %s", pCmdPhaser->mCmd);
	ESP_LOGI(CPTAG,"Phased data: %s", pCmdPhaser->mData);
	ESP_LOGI(CPTAG,"Phased CRC: %s", pCmdPhaser->mCRC);
}// cmdParser_PrintResult
