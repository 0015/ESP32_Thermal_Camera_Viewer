message("========================================")
message("libSenXor (Image processing part) ")
message("2023 Meridian Innovation. All rights reserved.")
message("========================================")

message("Registering ESP-IDF components....")
idf_component_register(
                    	INCLUDE_DIRS "." "include"
                   		REQUIRES drivers
                    	REQUIRES Applications)
                     
set(LIBS imageProcessingLib)

add_library(${LIBS} STATIC IMPORTED)
set_property(TARGET ${LIBS} PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib${LIBS}.a)
target_link_libraries(${COMPONENT_LIB} INTERFACE ${LIBS})
set_property(TARGET ${LIBS} APPEND PROPERTY INTERFACE_LINK_LIBRARIES ${COMPONENT_LIB})
message("================================================")
message("=================== Done =======================")
message("================================================")
