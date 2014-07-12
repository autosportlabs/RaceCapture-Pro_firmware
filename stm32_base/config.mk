# The name of our project (and the associated artifacts created)
TARGET = main

# board specific config file
include board/open407v_d/config.mk

#Base directory of our application (assumes FreeRTOS_Base is '.')
APP_BASE = .

#Version of FreeRTOS we'll be using
FREERTOS = FreeRTOSV7.6.0

# Uncomment the following to enable STM32 Peripheral libraries
STM32F4XX_LIBS = 1

#Uncomment the following line to enable stm32 USB libraries
#STM32_USB_LIBS = 1

#Uncomment any of the following three to build in support for USB DEV/OTG/Host
#STM32_USB_DEV = 1
#STM32_USB_HOST = 1
#STM32_USB_OTG = 1

#Uncomment the following line to enable ITM support (Trace Usart)
ITM = 1

INCLUDE_DIR = $(APP_PATH)/include

# The source files of our application
APP_SRC = 	$(APP_PATH)/main2.c \
			$(APP_PATH)/src/LED/LED.c \
			$(APP_PATH)/stm32_base/LED_stm32/LED_device_stm32.c \
			$(APP_PATH)/stm32_base/memory_stm32/memory_device_stm32.c \
			$(APP_PATH)/src/logger/loggerHardware.c

#Macro that expands our source files into their fully qualified paths
#and adds a macro explaining how to convert them to binary
APP_OBJS = $(addprefix $(APP_BASE)/, $(APP_SRC:.c=.o))

# Adds this directory to the global application includes
APP_INCLUDES += -I. \
				-I$(APP_PATH) \
				-Iutil \
				-I$(INCLUDE_DIR)/jsmn \
				-I$(INCLUDE_DIR)/api \
				-I$(INCLUDE_DIR)/logger \
				-I$(INCLUDE_DIR)/channels \
				-I$(INCLUDE_DIR)/tracks \
				-I$(INCLUDE_DIR)/logging \
				-I$(INCLUDE_DIR)/filter \
				-I$(INCLUDE_DIR)/CAN \
				-I$(INCLUDE_DIR)/OBD2 \
				-I$(INCLUDE_DIR)/ADC \
				-I$(INCLUDE_DIR)/timer \
				-I$(INCLUDE_DIR)/PWM \
				-I$(INCLUDE_DIR)/LED \
				-I$(INCLUDE_DIR)/GPIO \
				-I$(INCLUDE_DIR)/watchdog \
				-I$(INCLUDE_DIR)/memory \
				-I$(INCLUDE_DIR)/cpu \
				-I$(INCLUDE_DIR)/spi \
				-I$(INCLUDE_DIR)/serial \
				-I$(INCLUDE_DIR)/magic \
				-I$(INCLUDE_DIR)/lua \
				-I$(INCLUDE_DIR)/imu \
				-I$(INCLUDE_DIR)/messaging \
				-I$(INCLUDE_DIR)/predictive_timer \
				-I$(INCLUDE_DIR)/util \
				-I$(INCLUDE_DIR)/devices \
				-I$(INCLUDE_DIR)/gps \
				-I$(INCLUDE_DIR)/sdcard \
				-I$(INCLUDE_DIR)/command \
				-I$(INCLUDE_DIR)/virtual_channel \
				-I$(INCLUDE_DIR)/auto_config \
				-I$(JSON_DIR)

#Uncomment the following to enable newlib support
APP_INCLUDES += -Iutil
NEWLIB_SRC += newlib.c
NEWLIB_OBJS += $(addprefix util/, $(NEWLIB_SRC:.c=.o))
APP_OBJS += $(NEWLIB_OBJS)

#Uncomment the following to use the ITM (trace macrocell) for printf
APP_DEFINES += -DUSE_ITM

# CPU is generally defined by the Board's config.mk file
ifeq ($(CPU),)
$(error CPU is not defined, please define it in your CPU specific config.mk file)
endif

#Optional command to flash the board using an ST-Link
APP_FLASH = sudo st-flash write $(TARGET).bin 0x08000000

