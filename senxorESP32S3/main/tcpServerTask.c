/*****************************************************************************
 * @file     tcpServerTask.c
 * @version  1.5
 * @brief    TCP server main task. Provide functions for handling socket flow.
 * @date	 17 Jul 2022
 * @author	 Meridian Innovations
 ******************************************************************************/
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <sys/param.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_netif.h>

#include <lwip/err.h>
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/netdb.h>

#include "DrvSPIHost.h"
#include "SenXorLib.h"
#include "ledCtrlTask.h"
#include "tcpServerTask.h"

//public:
extern QueueHandle_t senxorFrameQueue;
TaskHandle_t tcpServerTaskHandle = NULL;						//TCP server handler

//private:
//Buffers
EXT_RAM_BSS_ATTR static cmdPhaser 	mCmdPhaserObj;														//Command phaser object
EXT_RAM_BSS_ATTR static uint8_t 	mRxBuff[128];					//Buffer storing the data from client
EXT_RAM_BSS_ATTR static uint8_t 	mAckBuff[20];					//Buffer for returning data to host
EXT_RAM_BSS_ATTR static uint8_t 	mTxBuff[PACKET_SIZE];			//Buffer holding the thermal data

static uint16_t mAckSize = 0;
static uint16_t mMemcpySize = 0;
static uint16_t mMemcpyOffset = 12;
static uint16_t mTxPacketSize = 0;
static uint16_t mTxSize = PACKET_SIZE;

//Sockets file descriptors
static int connect_sock = 0;									//Client Socket file descriptor. TCP Only
static int server_sock = 0;										//Server Socket file descriptor

//Configuration variables
#ifdef CONFIG_MI_SER_MODE_TCP
//TCP
static int keepAlive = KEEPALIVE_EN;							//TCP keep alive value
static int keepIdle = KEEPALIVE_IDLE;							//TCP keep idle value
static int keepInterval = KEEPALIVE_INTERVAL;					//TCP keep alive interval
static int keepCount = KEEPALIVE_COUNT;							//TCP keep alive count
#endif
//IP Addresses
struct sockaddr_storage dest_addr;								//Destination address
#ifdef CONFIG_MI_SER_MODE_UDP
struct sockaddr_storage source_addr; 							// Large enough for both IPv4 or IPv6
#endif
static TaskHandle_t tcpServerRecvTaskHandle = NULL;				//TCP receive task handler
static SemaphoreHandle_t tcpServerSemaphore = NULL;	
//flags
static bool isClientConnected = false;							//Indicates if a client is connected to the server
static bool isServerUp = false;									//Indicates if the server is running
static bool isFirstRun = true;									//Indicates if it is the first time the server started up

static int tcpServerGet(void);

/*
 * ***********************************************************************
 * @brief       tcpServerTask
 * @param       None
 * @return      None
 * @details     Task for handling TCP request
 **************************************************************************/
void tcpServerTask(void * pvParameters)
{
	ESP_LOGI(TCPTAG, "Creating tcpServerTask...");
	//senxorFrame *mSenxorFrameObj;

	ESP_LOGI(TCPTAG,TCP_INIT_INFO,xPortGetCoreID());
	cmdParser_Init(&mCmdPhaserObj);			
	tcpServer_InitThermalBuff();
	tcpServerStart();																											//Start tcp server
	tcpServerRestart(0);																										//Start listen to the port
	//ESP_LOGI(TCPTAG, "Creating tcpServerRecvTask...");
	//BaseType_t res = xTaskCreate(tcpServerRecvTask, "tcpRecvTask", 8192, NULL, 4, &tcpServerRecvTaskHandle);
	//ESP_LOGI(TCPTAG, "tcpRecvTask result: %s", res == pdPASS ? "Success" : "Fail");
	tcpServerSemaphore = xSemaphoreCreateMutex();

	uint16_t* senxorDataPtr = NULL;
	
	for(;;)
	{
		if(xQueueReceive(senxorFrameQueue, &senxorDataPtr, portMAX_DELAY) && isClientConnected)
		{
			//memcpy(mTxBuff+mMemcpyOffset, mSenxorFrameObj->mFrame, mMemcpySize*sizeof(uint16_t));		//Copy received object to buffer
			//sprintf((char *)&mTxBuff[mTxPacketSize - 4], "%04X", getCRC(mTxBuff+4,mTxPacketSize-4));	
			xSemaphoreTake(tcpServerSemaphore, portMAX_DELAY);
			//tcpServerSend(mTxBuff,mTxPacketSize); 

			printf("Sending thermal frame: %d bytes\n", 80 * 64 * sizeof(uint16_t));  // Will print 10240
			tcpServerSend((uint8_t*)senxorDataPtr, 80 * 64 * sizeof(uint16_t));  // Send 10240 bytes

			//printf("mTxPacketSize: %d\n", mTxPacketSize);
			xSemaphoreGive(tcpServerSemaphore);
			isFirstRun = false;
		}// End if
		vTaskDelay(1);
	}//End for

}//End tcpServerTask

/*
 * ***********************************************************************
 * @brief       tcpServerRecvTask
 * @param       None
 * @return      None
 * @details     Wait for client connect
 **************************************************************************/
void tcpServerRecvTask(void* pvParameters)
{
	ESP_LOGI(TCPTAG, ">>>> tcpServerRecvTask started");

	for(;;)
	{

		ESP_LOGI(TCPTAG, "tcpServerRecvTask running...");
		if(tcpServerGet() < 0)
		{
			tcpServerRestart(0);			//If there is an error during command receiving phase. Restart the server.
		}
		else
		{
			isClientConnected = true;		//If no error, it is assumed that the client is connected and the data can be sent via TCP/UDP.
			Acces_Write_Reg(0xB1, 0x03);
			ESP_LOGI(TCPTAG, "Client connected, stream started");
		}
		vTaskDelay(pdMS_TO_TICKS(200));		//Give way every 100ms
	}// 
}//End tcpServerRecvTask

/*
 * ***********************************************************************
 * @brief       tcpServerStart
 * @param       None
 * @return      None
 * @details     Start TCP Server
 **************************************************************************/
void tcpServerStart(void)
{

	ESP_LOGI(TCPTAG,TCP_INIT_INFO,xPortGetCoreID());
	ESP_LOGI(TCPTAG,MAIN_FREE_RAM " / " MAIN_TOTAL_RAM,heap_caps_get_free_size(MALLOC_CAP_INTERNAL), heap_caps_get_total_size(MALLOC_CAP_INTERNAL));								//Display the total amount of DRAM
	ESP_LOGI(TCPTAG,MAIN_FREE_SPIRAM " / " MAIN_TOTAL_SPIRAM,heap_caps_get_free_size(MALLOC_CAP_SPIRAM), heap_caps_get_total_size(MALLOC_CAP_SPIRAM));								//Display the total amount of PSRAM


#ifdef CONFIG_MI_LCD_EN
	drawText(1, 76+48+16*2,"Name: \n");
	drawText(1, 76+48+16*3,getMndsHostname());
#endif

	//Setup IPv4 address
	struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
	dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
	dest_addr_ip4->sin_family = AF_INET;
	dest_addr_ip4->sin_port = htons(PORT);

//TCP
#if CONFIG_MI_SER_MODE_TCP
	/*
	 * Phrase 1 - Create a socket
	 */
	server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);							//Create a socket. SOCK_STREAM = USE TCP
	if (server_sock < 0)
	{
		//If the socket cannot be created, TCP server will shutdown immediately
		ESP_LOGE(TCPTAG, TCP_ERR_CREATE,errno,strerror(errno));
		tcpServerShutdown();														//Shutdown server and delete TCP Server task
	}

	/*
	 *  Phrase 2 - Socket binding
	 */
	if (bind(server_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) != 0)
	{
		//If the socket cannot be binded, server will shutdown immediately
		ESP_LOGE(TCPTAG, TCP_ERR_BLIND, errno,strerror(errno));
		tcpServerShutdown();														//Shutdown server and delete TCP Server task
	}

	ESP_LOGI(TCPTAG, TCP_BIND, PORT);
	ESP_LOGI(TCPTAG, "%s\n",Drv_WLAN_getIP());
	ESP_LOGI(TCPTAG, TCP_SOCK_CREATE);
#endif

//UDP
#if CONFIG_MI_SER_MODE_UDP
	ESP_LOGI(TCPTAG,TCP_SER_SOCK_UDP);
#endif

	isServerUp = true;

}//End tcpServerStart

/*
 * ***********************************************************************
 * @brief       tcpServerRestart
 * @param       isFullRestart -
 * 				0: Listen and wait for connection again without a full restart
 * 				1: Perform a full restart of the server. This will reset all the
 * 				flags and close
 * @return      None
 * @details     Restart TCP server
 * 				This function wait until a client is connected.
 **************************************************************************/
void tcpServerRestart(const bool isFullRestart)
{
	char addr_str[128];
	isClientConnected = false;														//Clear client connected flag
	isServerUp = false;																//Clear server on flag

	/* If a full restart is required.
	 * The server is brought down first, which means all the sockets
	 * previously create and binded will be release.
	 * Afterwards, it will create and bind the socket again.
	*/
	if(isFullRestart)
	{
		ESP_LOGI(TCPTAG,TCP_SER_RESTART_FULL);
		tcpServerShutdown();
		tcpServerStart();
		return;
	}

#if CONFIG_MI_SER_MODE_TCP
	int status = listen(server_sock, 1);
	/*
	 * Phrase 3 - Enable server to listen a socket
	 */
	if (status != 0)
	{
		//If the socket cannot be listened, TCP server will shutdown immediately
		ESP_LOGE(TCPTAG, TCP_ERR_LISTEN, server_sock,errno,strerror(errno));
		tcpServerShutdown();														//Shutdown server													//Delete TCP Server task
	}

	ESP_LOGI(TCPTAG, TCP_LIS_INFO,PORT);
#if CONFIG_MI_LED_EN
	ledCtrlSingleSet(YELLOW_LED,LED_ON,500);
#endif
	struct sockaddr_storage source_addr; 											//Large enough for both IPv4 or IPv6
	socklen_t addr_len = sizeof(source_addr);										//Length of address

	/*
	 * Phrase 4 - Wait until a client connects
	 * Wait for connection. THIS IS A BLOCKING OPERATION.
	 * Once a client connects, it will unblock once a client is connected.
	 */
	if(connect_sock !=0 )
	{
		ESP_LOGW(TCPTAG,TCP_WARN_SOCK_EXIST);
		ESP_LOGI(TCPTAG,TCP_SOCK_CLE TCP_SOCK_INFO, connect_sock);
		close(connect_sock);
	}//End if
	connect_sock = accept(server_sock, (struct sockaddr *)&source_addr, &addr_len);

	if (connect_sock >= 0) {
		// Mark client connected
		isClientConnected = true;
		
		// Start thermal streaming
		Acces_Write_Reg(0xB1, 0x03);  // <-- this triggers the sensor to start pushing frames
		ESP_LOGI(TCPTAG, "Client connected, stream started automatically.");
	}else if (connect_sock < 0)
	{
		//If there is an error while accepting a connection, TCP server will close the socket

		ESP_LOGI(TCPTAG, "Client Disconnected");
		ESP_LOGE(TCPTAG, TCP_ERR_ACCEPT, errno,strerror(errno));
		close(connect_sock);

		isClientConnected = false;
	}

	// Set tcp keepalive option
	setsockopt(connect_sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));		//Configuring TCP keep alive value
	setsockopt(connect_sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));		//Configuring TCP keep alive idle value
	setsockopt(connect_sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));	//Configuring TCP keep alive interval
	setsockopt(connect_sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));		//Configuring TCP keep alive count

	// Convert IP address to string
	if (source_addr.ss_family == PF_INET)
	{
		inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
	}//End if

#endif

	/*
	 * UDP mode
	 */
#if CONFIG_MI_SER_MODE_UDP

	if(server_sock < 1)
	{
		close(server_sock);
	}//End if

	server_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);							//Create a socket. SOCK_DGRAM = USE UDP
	if (server_sock < 0)
	{
		//If the socket cannot be created, TCP server will shutdown immediately
		ESP_LOGE(TCPTAG, TCP_ERR_CREATE,errno,strerror(errno));
		tcpServerShutdown();														//Shutdown server and delete TCP Server task
	}//End if

    int status = bind(server_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (status < 0)
    {
    	ESP_LOGE(TCPTAG, TCP_ERR_ACCEPT, errno,strerror(errno));
    	close(server_sock);
    }
    else
    {
    	ESP_LOGI(TCPTAG, "Socket bound, port %d", PORT);
    }//End if-else

#endif
	ESP_LOGI(TCPTAG, TCP_ACCPET, addr_str);
	isServerUp = true;
#if CONFIG_MI_LED_EN
	ledCtrlSingleSet(GREEN_LED,LED_ON,0);
#endif
}//End tcpServerRestart

/*
 * ***********************************************************************
 * @brief       tcpServerShutdown
 * @param       None
 * @return      None
 * @details     Shutdown server and release the socket
 **************************************************************************/
void tcpServerShutdown(void)
{
	ESP_LOGI(TCPTAG, TCP_SER_SHUTDOWN);
	isClientConnected = false;
	shutdown(server_sock, 0);				//Shutdown all the connections
	close(connect_sock);					//Release socket that connects to server
	close(server_sock);						//Release socket that are listening by server
	isServerUp = false;
	isFirstRun = true;
	vTaskDelete(tcpServerRecvTaskHandle);	//Remove task finally
}//End tcpServerShutdown


/*
 * ***********************************************************************
 * @brief       tcpServerSend
 * @param       data - 8 bits data
				t - Size of the data to be sent (in bytes)
 * @return      No. of bytes sent. -1 if error is occurred
 * @details     Send data via TCP/IP
 **************************************************************************/
int tcpServerSend(const uint8_t* data, const size_t t)
{
#if CONFIG_MI_SER_MODE_TCP
	const int err = write(connect_sock,data,t); 										//Send data via socket. Wait until all bytes are transferred.
#endif

#if CONFIG_MI_SER_MODE_UDP
	const int err = sendto(server_sock, data, t, 0, (struct sockaddr *)&source_addr, sizeof(source_addr));
#endif
	/*
	 * If there is any error during the transmission.
	 * The TCP server will restart alongside the serial processor
	 */
	if(err < 0)
	{
		ESP_LOGE(TCPTAG, TCP_ERR_TRANS, connect_sock,  errno, strerror(errno));
		isClientConnected = false;

		//If client is disconnected
		switch(errno)
		{
			case 104:
			case 128:
				Acces_Write_Reg(0xB1,0x00);
			break;
			default:
				Acces_Write_Reg(0xB1,0x00);
			break;
		}

		tcpServerRestart(0);  // reset accept() loop
	}
	return err;

}//End tcpServerSend

/*
 * ***********************************************************************
 * @brief       tcpServerGetIsClientConnected
 * @param       None
 * @return      isClientConnected - True if a client is connected.
				False otherwise
 * @details		Return isClientConnected
 **************************************************************************/
bool tcpServerGetIsClientConnected(void)
{
	return isClientConnected;
}//End tcpServerGetIsClientConnected

/*
 * ***********************************************************************
 * @brief       tcpServerGet
 * @param       None
 * @return      No. of bytes read. -1 if error is occurred
 * @details     Read bytes from socket
 **************************************************************************/
static int tcpServerGet(void)
{

	ESP_LOGI(TCPTAG, "tcpServerGet()");

	bzero(mRxBuff, 50);
	ESP_LOGI(TCPTAG, TCP_DATA_REC_WAIT);

#if CONFIG_MI_SER_MODE_TCP
	int status = read(connect_sock, mRxBuff, 49);								//Wait and read from socket to buffer
#endif

#if CONFIG_MI_SER_MODE_UDP
    struct iovec iov;
    struct msghdr msg;
    struct cmsghdr *cmsgtmp;


    socklen_t socklen = sizeof(source_addr);
    uint8_t cmsg_buf[CMSG_SPACE(sizeof(struct in_pktinfo))];

    iov.iov_base = mRxBuff;
    iov.iov_len = sizeof(mRxBuff);
    msg.msg_control = cmsg_buf;
    msg.msg_controllen = sizeof(cmsg_buf);
    msg.msg_flags = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_name = (struct sockaddr *)&source_addr;
    msg.msg_namelen = socklen;

    int status = recvmsg(server_sock, &msg, 0);
#endif
	ESP_LOGI(TCPTAG, TCP_DATA_REC,mRxBuff);

	/*
	 * If there is any error during the transmission.
	 * The TCP server will stop the capture and wait for client again
	 */
	if(status < 0)
	{
		ESP_LOGE(TCPTAG, TCP_ERR_TRANS, connect_sock,  errno, strerror(errno));
		switch(errno)
		{
			case 104:
			case 128:
				Drv_SPI_Senxor_Write_Reg(0xB1,0);
			break;
		}//End switch
	}
	else
	{
		cmdParser_PharseCmd(&mCmdPhaserObj, mRxBuff, 49);
		mAckSize = cmdParser_CommitCmd(&mCmdPhaserObj, mAckBuff);
		if(mAckSize != 0)
		{
			xSemaphoreTake(tcpServerSemaphore, portMAX_DELAY);
			tcpServerSend( mAckBuff, mAckSize);
			xSemaphoreGive(tcpServerSemaphore);
		}

		cmdParser_Init(&mCmdPhaserObj);
	}
	return status;
}//End tcpServerGet



 /******************************************************************************
 * @brief       tcpServer_InitThermalBuff
 * @param       pSenxorType - Enum as defined in SenxorType
 * @return      none
 * @details     Initialise thermal buffer for TCP/IP
 *****************************************************************************/
void tcpServer_InitThermalBuff(void)
{
	memset(mTxBuff,0,PACKET_SIZE);
	memset(mAckBuff, 0, sizeof(mAckBuff));
	mMemcpySize = 80 * 64;
	mTxSize = 10256;
	mTxPacketSize = mTxSize; 
	mMemcpyOffset = 12+80*2; 				//(80 words + 4 words)

	// Using 10248 (0x2808) byte packet
	mTxBuff[0]=' ';
	mTxBuff[1]=' ';
	mTxBuff[2]=' ';
	mTxBuff[3]='#';
	mTxBuff[4]='2';
	mTxBuff[5]='8';
	mTxBuff[6]='0';
	mTxBuff[7]='8';
	mTxBuff[8]='G';
	mTxBuff[9]='F';
	mTxBuff[10]='R';
	mTxBuff[11]='A';

	// CRC
	mTxBuff[mTxSize-4]='X';
	mTxBuff[mTxSize-3]='X';
	mTxBuff[mTxSize-2]='X';
	mTxBuff[mTxSize-1]='X';
}// tcpServer_InitThermalBuff
