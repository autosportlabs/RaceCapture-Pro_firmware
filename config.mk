# The name of our project (and the associated artifacts created)
TARGET = f4_helloworld

# board specific config file
include board/open407v_d/config.mk

#Base directory of our application (assumes FreeRTOS_Base is '.')
APP_BASE = app/f4_helloworld

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

# The source files of our application
APP_SRC = main.c leds.c blinky.c

#Macro that expands our source files into their fully qualified paths
#and adds a macro explaining how to convert them to binary
APP_OBJS = $(addprefix $(APP_BASE)/, $(APP_SRC:.c=.o))

# Adds this directory to the global application includes
APP_INCLUDES += -Iapp/f4_helloworld

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

