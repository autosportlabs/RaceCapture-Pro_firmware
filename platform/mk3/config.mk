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
STM32_USB_LIBS = 1

#Uncomment any of the following three to build in support for USB DEV/OTG/Host
STM32_USB_DEV = 1
STM32_USB_DEV_CDC = 1
#STM32_USB_OTG = 1
#STM32_USB_HOST = 1

#Uncomment the following to include any USB device classes you might want
STM32_USB_DEV_CDC = 1
#Uncomment the following line to enable ITM support (Trace Usart)
ITM = 1

#HEAP selection
FREERTOS_HEAP=heap_4

RCP_RELEASE_DIR ?= .
RELEASE_NAME = RaceCapturePro-$(MAJOR).$(MINOR).$(BUGFIX)
RELEASE_NAME_ZIP = $(RELEASE_NAME).zip
RELEASE_NAME_BIN = $(RELEASE_NAME).bin
RELEASE_NAME_ELF = $(RELEASE_NAME).elf
RCP_INSTALL_DIR = RaceCapturePro_Firmware

INCLUDE_DIR := $(APP_PATH)/include
PLATFORM_DIR := $(APP_PATH)/platform/mk3
HAL_SRC = $(PLATFORM_DIR)/hal
RCP_SRC = $(APP_PATH)/src

# The source files of our application
APP_SRC = \
$(APP_PATH)/main.c \
$(HAL_SRC)/ADC_stm32/ADC_device_stm32.c \
$(HAL_SRC)/CAN_stm32/CAN_device_stm32.c \
$(HAL_SRC)/GPIO_stm32/GPIO_device_stm32.c \
$(HAL_SRC)/LED_stm32/led_device_stm32.c \
$(HAL_SRC)/cell_device/cell_pwr_btn.c \
$(HAL_SRC)/cpu_stm32/cpu_device_stm32.c \
$(HAL_SRC)/fat_sd_stm32/fatfs/diskio.c \
$(HAL_SRC)/fat_sd_stm32/fatfs/drivers/fatfs_sd_sdio.c \
$(HAL_SRC)/fat_sd_stm32/fatfs/drivers/stm32_fattime.c \
$(HAL_SRC)/fat_sd_stm32/fatfs/ff.c \
$(HAL_SRC)/fat_sd_stm32/fatfs/option/syscall.c \
$(HAL_SRC)/fat_sd_stm32/fatfs/option/unicode.c \
$(HAL_SRC)/fat_sd_stm32/sdcard_device_stm32.c \
$(HAL_SRC)/fat_sd_stm32/tm_stm32f4_fatfs.c \
$(HAL_SRC)/fat_sd_stm32/tm_stm32f4_gpio.c \
$(HAL_SRC)/gps_stm32/gps_device_lld_stm32.c \
$(HAL_SRC)/i2c_stm32/i2c_device_stm32.c \
$(HAL_SRC)/imu_stm32/imu_device_stm32.c \
$(HAL_SRC)/imu_stm32/invensense_9150.c \
$(HAL_SRC)/memory_stm32/memory_device_stm32.c \
$(HAL_SRC)/serial/serial_device.c \
$(HAL_SRC)/timer_stm32/timer_device_stm32.c \
$(HAL_SRC)/usart_stm32/usart_device_stm32.c \
$(HAL_SRC)/usb_stm32/USB-CDC_device_stm32.c \
$(HAL_SRC)/usb_stm32/usb_bsp.c \
$(HAL_SRC)/usb_stm32/usbd_cdc_vcp.c \
$(HAL_SRC)/usb_stm32/usbd_desc.c \
$(HAL_SRC)/usb_stm32/usbd_usr.c \
$(HAL_SRC)/watchdog_stm32/watchdog_device_stm32.c \
$(HAL_SRC)/wifi_esp8266/wifi_esp8266_device.c \
$(RCP_SRC)/ADC/ADC.c \
$(RCP_SRC)/CAN/CAN.c \
$(RCP_SRC)/CAN/CAN_task.c \
$(RCP_SRC)/CAN/CAN_aux_queue.c \
$(RCP_SRC)/CAN/can_mapping.c \
$(RCP_SRC)/CAN/can_channels.c \
$(RCP_SRC)/GPIO/GPIO.c \
$(RCP_SRC)/GPIO/gpioTasks.c \
$(RCP_SRC)/LED/led.c \
$(RCP_SRC)/OBD2/OBD2.c \
$(RCP_SRC)/api/api.c \
$(RCP_SRC)/auto_config/auto_track.c \
$(RCP_SRC)/command/baseCommands.c \
$(RCP_SRC)/command/command.c \
$(RCP_SRC)/cpu/cpu.c \
$(RCP_SRC)/devices/bluetooth.c \
$(RCP_SRC)/devices/cellular.c \
$(RCP_SRC)/devices/cellular_api_status_keys.c \
$(RCP_SRC)/devices/esp8266.c \
$(RCP_SRC)/devices/null_device.c \
$(RCP_SRC)/devices/sara_u280.c \
$(RCP_SRC)/devices/sim900.c \
$(RCP_SRC)/devices/gps_skytraq_s1216_sup500f8.c \
$(RCP_SRC)/drivers/esp8266_drv.c \
$(RCP_SRC)/filter/filter.c \
$(RCP_SRC)/gps/dateTime.c \
$(RCP_SRC)/gps/geoCircle.c \
$(RCP_SRC)/gps/geoTrigger.c \
$(RCP_SRC)/gps/geopoint.c \
$(RCP_SRC)/gps/gps.c \
$(RCP_SRC)/gps/gpsTask.c \
$(RCP_SRC)/gsm/gsm.c \
$(RCP_SRC)/imu/imu.c \
$(RCP_SRC)/jsmn/jsmn.c \
$(RCP_SRC)/lap_stats/lap_stats.c \
$(RCP_SRC)/launch_control.c \
$(RCP_SRC)/logger/auto_control.c \
$(RCP_SRC)/logger/auto_logger.c \
$(RCP_SRC)/logger/camera_control.c \
$(RCP_SRC)/logger/channel_config.c \
$(RCP_SRC)/logger/connectivityTask.c \
$(RCP_SRC)/logger/fileWriter.c \
$(RCP_SRC)/logger/logger.c \
$(RCP_SRC)/logger/loggerApi.c \
$(RCP_SRC)/logger/loggerCommands.c \
$(RCP_SRC)/logger/loggerConfig.c \
$(RCP_SRC)/logger/loggerData.c \
$(RCP_SRC)/logger/loggerHardware.c \
$(RCP_SRC)/logger/loggerSampleData.c \
$(RCP_SRC)/logger/loggerTaskEx.c \
$(RCP_SRC)/logger/sampleRecord.c \
$(RCP_SRC)/logger/versionInfo.c \
$(RCP_SRC)/logging/printk.c \
$(RCP_SRC)/lua/luaBaseBinding.c \
$(RCP_SRC)/lua/luaCommands.c \
$(RCP_SRC)/lua/luaLoggerBinding.c \
$(RCP_SRC)/lua/luaScript.c \
$(RCP_SRC)/lua/luaTask.c \
$(RCP_SRC)/memory/memory.c \
$(RCP_SRC)/messaging/messaging.c \
$(RCP_SRC)/modem/at.c \
$(RCP_SRC)/modem/at_basic.c \
$(RCP_SRC)/predictive_timer/predictive_timer_2.c \
$(RCP_SRC)/sdcard/sdcard.c \
$(RCP_SRC)/serial/rx_buff.c \
$(RCP_SRC)/serial/serial.c \
$(RCP_SRC)/serial/serial_buffer.c \
$(RCP_SRC)/system/flags.c \
$(RCP_SRC)/tasks/wifi.c \
$(RCP_SRC)/timer/timer.c \
$(RCP_SRC)/timer/timer_config.c \
$(RCP_SRC)/tracks/tracks.c \
$(RCP_SRC)/units/units.c \
$(RCP_SRC)/units/units_conversion.c \
$(RCP_SRC)/usart/usart.c \
$(RCP_SRC)/usb_comm/usb_comm.c \
$(RCP_SRC)/util/FreeRTOS-openocd.c \
$(RCP_SRC)/util/byteswap.c \
$(RCP_SRC)/util/convert.c \
$(RCP_SRC)/util/linear_interpolate.c \
$(RCP_SRC)/util/modp_numtoa.c \
$(RCP_SRC)/util/panic.c \
$(RCP_SRC)/util/ring_buffer.c \
$(RCP_SRC)/util/str_util.c \
$(RCP_SRC)/util/taskUtil.c \
$(RCP_SRC)/util/ts_ring_buff.c \
$(RCP_SRC)/virtual_channel/virtual_channel.c \
$(RCP_SRC)/watchdog/watchdog.c \


#Macro that expands our source files into their fully qualified paths
#and adds a macro explaining how to convert them to binary
APP_OBJS = $(addprefix $(APP_BASE)/, $(APP_SRC:.c=.o))

# Adds this directory to the global application includes
APP_INCLUDES += \
-I$(APP_PATH) \
-I$(HAL_SRC)/fat_sd_stm32/ \
-I$(HAL_SRC)/fat_sd_stm32/fatfs \
-I$(HAL_SRC)/fat_sd_stm32/fatfs/drivers \
-I$(HAL_SRC)/fat_sd_stm32/fatfs/option \
-I$(HAL_SRC)/i2c_stm32 \
-I$(HAL_SRC)/imu_stm32 \
-I$(HAL_SRC)/usb_stm32 \
-I$(INCLUDE_DIR) \
-I$(INCLUDE_DIR)/ADC \
-I$(INCLUDE_DIR)/CAN \
-I$(INCLUDE_DIR)/GPIO \
-I$(INCLUDE_DIR)/LED \
-I$(INCLUDE_DIR)/OBD2 \
-I$(INCLUDE_DIR)/PWM \
-I$(INCLUDE_DIR)/api \
-I$(INCLUDE_DIR)/auto_config \
-I$(INCLUDE_DIR)/channels \
-I$(INCLUDE_DIR)/command \
-I$(INCLUDE_DIR)/cpu \
-I$(INCLUDE_DIR)/devices \
-I$(INCLUDE_DIR)/drivers \
-I$(INCLUDE_DIR)/filter \
-I$(INCLUDE_DIR)/gps \
-I$(INCLUDE_DIR)/gsm \
-I$(INCLUDE_DIR)/imu \
-I$(INCLUDE_DIR)/jsmn \
-I$(INCLUDE_DIR)/lap_stats \
-I$(INCLUDE_DIR)/logger \
-I$(INCLUDE_DIR)/logging \
-I$(INCLUDE_DIR)/lua \
-I$(INCLUDE_DIR)/magic \
-I$(INCLUDE_DIR)/memory \
-I$(INCLUDE_DIR)/messaging \
-I$(INCLUDE_DIR)/modem \
-I$(INCLUDE_DIR)/predictive_timer \
-I$(INCLUDE_DIR)/sdcard \
-I$(INCLUDE_DIR)/serial \
-I$(INCLUDE_DIR)/spi \
-I$(INCLUDE_DIR)/system \
-I$(INCLUDE_DIR)/tasks \
-I$(INCLUDE_DIR)/timer \
-I$(INCLUDE_DIR)/tracks \
-I$(INCLUDE_DIR)/units \
-I$(INCLUDE_DIR)/usart \
-I$(INCLUDE_DIR)/usb_comm \
-I$(INCLUDE_DIR)/util \
-I$(INCLUDE_DIR)/virtual_channel \
-I$(INCLUDE_DIR)/watchdog \
-I$(PLATFORM_DIR) \
-I$(PLATFORM_DIR)/libs/FreeRTOSV7.6.0/FreeRTOS/Source/portable/MemMang \
-I$(PLATFORM_DIR)/mem_mang \
-I$(PLATFORM_DIR)/util \

#Uncomment the following to enable newlib support
APP_INCLUDES += -Iutil
NEWLIB_SRC += newlib.c
NEWLIB_OBJS += $(addprefix util/, $(NEWLIB_SRC:.c=.o))
APP_OBJS += $(NEWLIB_OBJS)

#Uncomment the following to use the ITM (trace macrocell) for printf
APP_DEFINES += -DUSE_ITM -DSD_SDIO -DUSE_DMA1=1 $(VERSION_CFLAGS)

# CPU is generally defined by the Board's config.mk file
ifeq ($(CPU),)
$(error CPU is not defined, please define it in your CPU specific config.mk file)
endif

#Optional command to flash the board using an ST-Link
APP_FLASH = sudo st-flash write $(TARGET).bin 0x08000000
