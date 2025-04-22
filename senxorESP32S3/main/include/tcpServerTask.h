/*****************************************************************************
 * @file     tcpServerTask.h
 * @version  1.4
 * @brief    Header file for tcpServerTask.c
 * @date	 17 Jul 2022
 ******************************************************************************/
#ifndef MAIN_INCLUDE_TCPSERVERTASK_H_
#define MAIN_INCLUDE_TCPSERVERTASK_H_

#include "DrvNVS.h"
#include "DrvGPIO.h"
#include "DrvWLAN.h"
#include "restServer.h"
#include "senxorTask.h"
#include "cmdParser.h"
#include "msg.h"
#include "util.h"

//4+4+(160*124)*2+3+4
//Data size+
#define PACKET_SIZE              4+4+4+(160+80+80*63)*2+4           // Packet size 160*3 + 160*121 ＝ 19840 message length =  39688 (0x9B08) 4+4+(160*124)*2+3+4
#define PACKET_RX_SIZE           128

//Server configuration
#define PORT                     CONFIG_MI_TCP_PORT							//TCP/IP Port to listen

#ifdef CONFIG_MI_SER_MODE_TCP
#define KEEPALIVE_EN			 CONFIG_MI_TCP_KEEPALIVE_EN					//Enable / disable keep alive
#define KEEPALIVE_IDLE           CONFIG_MI_TCP_KEEPALIVE_IDLE				//TCP Keep alive idle
#define KEEPALIVE_INTERVAL       CONFIG_MI_TCP_KEEPALIVE_INTERVAL			//TCP Keep Alive interval
#define KEEPALIVE_COUNT          CONFIG_MI_TCP_KEEPALIVE_COUNT				//TCP Keep Alive count
#endif
//Task configuration
#define TCP_TASK_STACK_SIZE      4096

//Messages
//D-9 - TCP Server
#define TCPTAG 					"[SERVER]"
#define TCP_ACCPET				"IP Address accepted: %s."
#define TCP_BIND				"Socket bind: %d."
#define TCP_CONNECT 			"TCP Connection established."
#define TCP_DATA_INFO			"Attempt to send data with length %d ..."
#define TCP_DATA_INIT			"Sending / Receiving Data..."
#define TCP_DATA_REC			"Packet received:\n%s"
#define TCP_DATA_REC_WAIT		"Waiting for packet..."
#define TCP_DATA_EMT			"Data buffer is empty"
#define TCP_INIT_INFO			"TCP Server initialising... Running on Core %d."

#define TCP_ERR_ACCEPT			"Error occurred while accepting connection.\nError code:%d (%s)"
#define TCP_ERR_BLIND			"Error occurred while blinding address.\nError code:%d (%s)"
#define TCP_ERR_BUFF			"Unable to allocate buffer for transmitting data frame. Closing server..."
#define TCP_ERR_CREATE			"Error occurred while creating server.\nError code:%d (%s)"
#define TCP_ERR_LISTEN			"Error occurred while listening to port %d.\nError code:%d (%s)"
#define TCP_ERR_SOCK			"Cannot connect to socket %d."
#define TCP_ERR_TASK_FAIL_INIT	"TCP task failed to initialised. The program will exit now."
#define TCP_ERR_TRANS			"Error occurred during receive/transmit phase: Socket: %d | Error: %d (%s)"

#define TCP_SER_INFO			"Server address: %s.Port to be listened: %s."
#define TCP_SER_SOCK_UDP		"Using UDP as protocol."
#define TCP_SER_SOCK			"Creating server with port %d "
#define TCP_SER_RESTART			"Reloading TCP server..."
#define TCP_SER_RESTART_FULL	"Restarting TCP server..."
#define TCP_SER_SHUTDOWN		"Shutting down TCP server..."

#define TCP_SOCK_CREATE			"Socket created."
#define TCP_SOCK_CLE			"Closing socket..."
#define TCP_SOCK_INFO			"Socket: %d."
#define TCP_LIS_INFO			"Listening on socket %d\nWaiting for connection..."

#define TCP_MSG_TEST			"Test message from TCP Server."
#define TCP_WARN_SOCK_EXIST		"Closing the socket now for next use."

void tcpServerStart(void);

void tcpServerRecvTask(void* pvParameters);

void tcpServerRestart(const bool isFullRestart);

void tcpServerShutdown(void);

void tcpServerTask(void * pvParameters);

int tcpServerSend(const uint8_t* data, const size_t t);

bool tcpServerGetIsClientConnected(void);

void tcpServer_InitThermalBuff(void);

#endif /* MAIN_INCLUDE_TCPSERVERTASK_H_ */
