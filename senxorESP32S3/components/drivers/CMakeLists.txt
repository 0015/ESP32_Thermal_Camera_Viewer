message("================================================")
message("Component to be configured: " ${COMPONENT_NAME})
message("================================================")

idf_component_register(
					SRCS "src/Drv_BT.c"
					SRCS "src/Drv_BTSec.c"
					SRCS "src/Drv_Boot.c"
					SRCS "src/Drv_CRC.c"
					SRCS "src/DrvGPIO.c"
					SRCS "src/DrvLCD.c"
					SRCS "src/DrvLED.c"
					SRCS "src/DrvNVS.c"
					SRCS "src/DrvTimer.c"
					SRCS "src/DrvSPIHost.c"
					SRCS "src/DrvWLAN.c"
					SRCS "src/DrvUSB.c"
					SRCS "src/MCU_Dependent.c"
					SRCS "src/SenXor_FLASH.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_tinyusb driver bt esp_lcd esp_wifi espressif__led_strip esp_lcd_gc9a01 SenXorLib mbedtls nvs_flash net util ulp tinyusb usb
                    PRIV_INCLUDE_DIRS .
                    PRIV_REQUIRES esp_adc esp_lcd net mbedtls
                    )

message("================================================")
message("=================== Done =======================")
message("================================================")    