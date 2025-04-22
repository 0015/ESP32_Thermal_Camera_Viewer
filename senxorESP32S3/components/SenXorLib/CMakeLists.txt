message("========================================")
message("libSenXor for C ")
message("2023 Meridian Innovation. All rights reserved.")
message("========================================")

#
# Set this flag to true if 4MHz clock is supplied to SenXor
# NOTICE: 
# If clock other than 4MHz / 3MHz (default) is supplied to SenXor,
# a SenXorLib optimised with that particular clock must be 
# supplied with the program.
#
if(CONFIG_MI_ONBOARD_CLK)

set(isUse4MLib TRUE)
else()
set(isUse4MLib FALSE)
endif()

message("Registering ESP-IDF components....")
idf_component_register(
                    	INCLUDE_DIRS "." "./include"
                   		REQUIRES drivers
                    	REQUIRES Applications)
                     
if(NOT isUse4MLib)
message("================================================")                 
message( "Using libSenXor working with 3MHz clock.")
message("================================================")
set(LIBS senxorLib)
else()
message("================================================")
message("============= USING 4MHz clock =============")
message("================================================")
message("Using libSenXor built for SenXor running with")
message("system clock 4MHz.")
message("ENSURE THAT A SUITABLE CLOCK IS SUPPLY TO SenXor")
message("================================================")
set(LIBS senxorLib4M)
endif()


add_library(${LIBS} STATIC IMPORTED)
set_property(TARGET ${LIBS} PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib${LIBS}.a)
target_link_libraries(${COMPONENT_LIB} INTERFACE ${LIBS})
set_property(TARGET ${LIBS} APPEND PROPERTY INTERFACE_LINK_LIBRARIES ${COMPONENT_LIB})
message("================================================")
message("=================== Done =======================")
message("================================================")
