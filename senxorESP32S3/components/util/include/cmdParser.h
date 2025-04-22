#ifndef COMPONENTS_UTIL_INCLUDE_CMDPARSER_H_
#define COMPONENTS_UTIL_INCLUDE_CMDPARSER_H_

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "DrvSPIHost.h"

#include "msg.h"
#include "util.h"


/* Command definition*/
#define CMD_WREG "WREG"
#define CMD_RREG "RREG"
#define CMD_WSID "WSID"
#define CMD_WPWD "WPWD"
#define CMD_WMDE "WMDE"
#define CMD_RRSE "RRSE"

/* Data format definition*/
#define EVK_CMD_START_CHAR 		'#'
#define NUM_BYTES_LEN_FIELD 	4
#define NUM_BYTES_CMD_FIELD 	4
#define NUM_BYTES_CRC_FIELD 	4
#define NUM_BYTES_DATA_FIELD	20
#define NUM_BYTES_LEN_BUFF		5
#define LENGTH_PARSE_ERROR 		-1
#define WAIT_TIMEOUT 			50

#define CP_START_CHAR			'#'
#define CP_CMD_FIELD_LEN		5
#define CP_DLEN_FIELD_LEN		5
#define CP_CRC_FIELD_LEN		5
#define CP_DATA_FIELD_LEN		512

// Messages
#define CPTAG					"[CMD_PHASER]"
#define CP_ERR_CMD_LEN			"Incorrect command length."
#define CP_ERR_CMD_INVALID		"Invalid command"
#define CP_ERR_CRC_FAIL			"Checksum mismatched. Failed to verify the data."
#define CP_ERR_CRC_FAIL_INFO	"Checksum calucated: %lu . Checksum in data: %lu"
#define CP_ERR_INPUT_NULL		"Cannot phase command with a empty input."
#define CP_ERR_OBJ_NULL			"Cannot phase command with a NULL phaser object."
#define CP_ERR_STATE			"Invalid state, resetting phaser..."
#define CP_INFO_INIT			"Initialising phaser object..."
#define CP_INFO_START			"Start phrasing command from strings:"
#define CP_INFO_STAGE_1			"STAGE 1 - Searching for start character # ..."
#define CP_INFO_STAGE_2			"STAGE 2 - Determinating command length..."
#define CP_INFO_STAGE_3			"STAGE 3 - Extracting command and data..."
#define CP_INFO_STAGE_4			"STAGE 4 - Determinating CRC and verifying data..."
#define CP_INFO_CMD_LEN			"Command length: %lu"
#define CP_INFO_CRC_OK			"Data verified."
#define CP_WARN_INPUT_LEN_OVER	"Input size exceeds the limit. Capping size to %d ."
#define CP_WARN_INPUT_CRC_NULL	"No checksum is provided. Ignoring data integrity check."

typedef enum cmdList
{	
	WREG,
	RREG,
	WRSE,
	RRAL,
	WEEP,
	REEP,	
	WFLA,
	RFLA,
	GFRA,
	GFAV,
	GFRC,
	WCAL,
	SFMO,
	SCMO,
	SFRA,
	DACA,
	TEST,
	FTES,
	I2TS,
	RRSE	
}cmdList;

typedef enum cmdParserState
{
	START_CHAR,
	LEN,
	DATA,
	CRC,
	RST
}cmdParserState;


typedef struct cmdPhaserClass
{
	cmdParserState mCmdParserState;
	uint8_t mCmdLen[CP_DLEN_FIELD_LEN];
	uint8_t mCmd[CP_CMD_FIELD_LEN];
	uint8_t mData[CP_DATA_FIELD_LEN];
	uint8_t mCRC[CP_CRC_FIELD_LEN];
}cmdPhaser;


void cmdParser_Init(cmdPhaser* pCmdPhaser);

uint8_t cmdParser_CommitCmd(const cmdPhaser* pCmdPhaser, uint8_t* pAckBuff);

int cmdParser_PharseCmd(cmdPhaser* pCmdPhaser, const uint8_t* pInput, size_t pInputSize);

void cmdParser_PrintResult(const cmdPhaser* pCmdPhaser);

int toHex(const char* str);

int getHexValue(const char c);

#endif /* COMPONENTS_UTIL_INCLUDE_CMDPARSER_H_ */
