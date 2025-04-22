message("================================================")
message("Component to be configured: " ${COMPONENT_NAME})
message("================================================")
set(src "src/restServer.c" )
message("Components to be registered: ")
message(${src})
idf_component_register(	SRCS ${src}
                    	INCLUDE_DIRS "include"
                    	REQUIRES drivers esp_https_server esp_http_client vfs json spiffs )
                    	
message("================================================")
message("=================== Done =======================")
message("================================================")  
