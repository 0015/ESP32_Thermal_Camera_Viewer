message("================================================")
message("Component to be configured: " ${COMPONENT_NAME})
message("================================================")

idf_component_register(
					SRCS "src/util.c"
					SRCS "src/cmdParser.c"
                    INCLUDE_DIRS "." "include" 
                    REQUIRES Applications drivers json net)

message("================================================")
message("=================== Done =======================")
message("================================================") 