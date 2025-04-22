message("================================================")
message("Component to be configured: " ${COMPONENT_NAME})
message("================================================")

message("Configuring main component...")

# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES Applications drivers net SenXorLib util soc nvs_flash ulp)
set(COMPONENT_PRIV_REQUIRES  esp_lcd )
set(COMPONENT_ADD_INCLUDEDIRS "." "include" )
set(COMPONENT_SRCS "main.c" "ledCtrlTask.c" "senxorTask.c" "tcpServerTask.c" "usbSerialTask.c") 
message("Components registered:")
message(${COMPONENT_REQUIRES})

register_component()
# message("================================================")
# message("Registering ULP components...")
# message("================================================")

# # 1. The ULP app name must be unique (if multiple components use ULP).
# set(ulp_app_name ulp_${COMPONENT_NAME})

# # 2. Specify all C and Assembly source files.
# #    Files should be placed into a separate directory (in this case, ulp/),
# #    which should not be added to COMPONENT_SRCS.
# set(ulp_riscv_sources "ulp/main.c")


# # 3. List all the component source files which include automatically
# #    generated ULP export file, ${ulp_app_name}.h:
# set(ulp_exp_dep_srcs "main.c")

# message("ULP App name: " ${ulp_app_name})
# message("ULP RISCV source: " ${ulp_riscv_sources})
# message("Source files which include automatically generated ULP export file: " ${ulp_exp_dep_srcs})
# message("ulp_embed_binary = " ${ulp_app_name} "${ulp_riscv_sources}" "${ulp_exp_dep_srcs}")


# ulp_embed_binary(${ulp_app_name} "${ulp_riscv_sources}" "${ulp_exp_dep_srcs}")
message("================================================")
message("=================== Done =======================")
message("================================================")
