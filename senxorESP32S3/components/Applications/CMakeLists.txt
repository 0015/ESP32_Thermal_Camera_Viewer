idf_component_register(
					SRCS "src/AutoGain.c"
					SRCS "src/Customer_Interface.c"
					SRCS "src/Senxor_Capturedata.c"
					SRCS "src/SenXor_PowerDownMode.c"
					SRCS "src/version.c"
                    INCLUDE_DIRS "." "include"
                    REQUIRES SenXorLib imageProcessingLib
                    )

