/*****************************************************************************
 * @file     restServer.c
 * @version  1.1
 * @brief    REST Sever
 * @date	 26 Apr 2023
 * @Author	 Sarashina Ruka
 ******************************************************************************/
#include "restServer.h"

typedef struct rest_server_context {
    char scratch[SCRATCH_BUFSIZE];
} rest_server_context_t;

//public:

EXT_RAM_BSS_ATTR areaJsonCfgArrStruct monitorAreaCfgObj;
#if CONFIG_ESPMODEL_S3MINI_C
//extern QueueHandle_t areaCfgQueue;
#endif
extern void senxorTask_getRptToJson(cJSON* jsonObj);
//private:
static httpd_handle_t server = NULL;

static esp_err_t restServer_Start();
static esp_err_t system_info_get_handler(httpd_req_t *req);

/*
 * ***********************************************************************
 * @brief       restServer_Init
 * @param       None
 * @return      None
 * @details     Initialise REST server
 **************************************************************************/
void restServer_Init()
{
	const esp_err_t err = restServer_Start();
	switch(err)
	{
		case ESP_OK:
			ESP_LOGI(RSTTAG,RSTSER_INIT_OK);
		break;
		default:
			ESP_LOGE(RSTTAG,RSTSER_ERR,esp_err_to_name(err));
		break;
	}//End switch
}

/*
 * ***********************************************************************
 * @brief       restServer_Deinit
 * @param       None
 * @return      None
 * @details     Stop REST server
 **************************************************************************/
void restServer_Deinit()
{
	if (getHandler() == 0)
	{
		ESP_LOGE(RSTTAG,RSTSER_ERR_HANDLE_NULL);
		return;
	}

	const esp_err_t err = httpd_stop(getRestServerHandler());
	switch(err)
	{
		case ESP_OK:
			ESP_LOGI(RSTTAG,RSTSER_DEINIT_OK);
		break;
		default:
			ESP_LOGE(RSTTAG, RSTSER_ERR_DEINIT,esp_err_to_name(err));
		break;
	}//End switch

}

/*
 * ***********************************************************************
 * @brief       getHandler
 * @param       None
 * @return      httpd_handle_t - Return server handler
 * @details     Get server handler
 **************************************************************************/
httpd_handle_t getRestServerHandler()
{
	return server;
}

/*
 * ***********************************************************************
 * @brief       restServer_Start
 * @param       base_path - Path to server root
 * @return      esp_err_t - Return error code if failed, ESP_OK otherwise
 * @details     Start REST server
 **************************************************************************/
static esp_err_t restServer_Start()
{

#ifdef CONFIG_SPIRAM
    rest_server_context_t *rest_context = heap_caps_malloc(sizeof(rest_server_context_t),MALLOC_CAP_SPIRAM);
#else
    rest_server_context_t *rest_context = calloc(1, sizeof(rest_server_context_t));
#endif
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.stack_size = 2048;
    config.max_open_sockets = 2;
    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_LOGI(RSTTAG, RSTSER_INFO);
    ESP_LOGI(RSTTAG,MAIN_FREE_RAM " / " MAIN_TOTAL_RAM,heap_caps_get_free_size(MALLOC_CAP_INTERNAL), heap_caps_get_total_size(MALLOC_CAP_INTERNAL));								//Display the total amount of DRAM
    ESP_LOGI(RSTTAG,MAIN_FREE_SPIRAM " / " MAIN_TOTAL_SPIRAM,heap_caps_get_free_size(MALLOC_CAP_SPIRAM), heap_caps_get_total_size(MALLOC_CAP_SPIRAM));								//Display the total amount of PSRAM

	esp_err_t err = httpd_start(&server, &config);

    switch(err)
    {
    	case ESP_OK:
    		break;
		default:
			return err;
    }//End switch

    /*
     * Initialise URI Handler here
     */

    /* URI handler for fetching system info */
    httpd_uri_t system_info_get_uri = {
        .uri = "/info",
        .method = HTTP_GET,
        .handler = system_info_get_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &system_info_get_uri);

    switch(err)
    {
    	case ESP_OK:
    		ESP_LOGI(RSTTAG, RSTSER_INIT_OK);
    		return err;
		default:
			free(rest_context);
			return err;
    }//End switch

}

/*
 * ***********************************************************************
 * @brief       system_info_get_handler
 * @param       req - HTTP Request object
 * @return      None
 * @details     Handler for sending WiFi configuration to client via HTTP GET
 **************************************************************************/
static esp_err_t system_info_get_handler(httpd_req_t *req)
{
	httpd_resp_set_type(req, "application/json");				//HTTP content type is JSON
    cJSON *root = cJSON_CreateObject();							//Create a JSON object
    cJSON *sys_info = cJSON_AddObjectToObject(root, "sys_info");

    /*
     * Add data to JSON
     */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    cJSON_AddStringToObject(sys_info, "version", IDF_VER);
    cJSON_AddNumberToObject(sys_info, "cores", chip_info.cores);
    cJSON_AddNumberToObject(sys_info, "model", chip_info.model);
    cJSON_AddNumberToObject(sys_info, "speed", CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ);

    const char *sendStr = cJSON_Print(root);									//Format JSON to string
    httpd_resp_sendstr(req, sendStr);											//Send JSON object

    free((void *)sendStr);
    cJSON_Delete(root);
    return ESP_OK;
}//End system_info_get_handler


