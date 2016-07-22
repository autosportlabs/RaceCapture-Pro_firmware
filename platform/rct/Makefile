ifeq ($(V),)
.SILENT:
endif

################################################################################
# Toolchain environment
################################################################################
export PATH := /opt/arm-2012.03/bin:$(PATH)
CROSS_COMPILE := arm-none-eabi-
AS := $(CROSS_COMPILE)as
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)gcc
OBJDUMP := $(CROSS_COMPILE)objdump
OBJCOPY := $(CROSS_COMPILE)objcopy

ASL_FW_POSTPROCESS := asl_f4_fw_postprocess

#specify the use of the external crystal
BOARD_DEFINES = -DPLL_SOURCE_HSE -DUSE_I2C1

# Needed for FreeRTOS GDB debugging
FREERTOS_LD_FLAGS := -Wl,--undefined=uxTopUsedPriority

################################################################################
# Project definition
################################################################################
OUTDIR := .build

RC_TOP_LEVEL_DIR := ../..
RC_PLATFORM_DIR := $(RC_TOP_LEVEL_DIR)/platform
RC_LIBRARY_DIR := $(RC_TOP_LEVEL_DIR)/lib
RC_INCLUDE_DIR := $(RC_TOP_LEVEL_DIR)/include
RC_SRC_DIR := $(RC_TOP_LEVEL_DIR)/src

# CMSIS
src-y := \
    Libraries/CMSIS/Device/ST/STM32F30x/Source/Templates/TrueSTUDIO/startup_stm32f30x.s \
    Libraries/CMSIS/Device/ST/STM32F30x/Source/Templates/system_stm32f30x.c
inc-y := \
    Libraries/CMSIS/Device/ST/STM32F30x/Include \
    Libraries/CMSIS/Include

# StdPeriph
src-y += $(wildcard Libraries/STM32F30x_StdPeriph_Driver/src/*.c)
inc-y += Libraries/STM32F30x_StdPeriph_Driver/inc

# USB
src-y += $(wildcard Libraries/STM32_USB-FS-Device_Driver/src/*.c)
inc-y += Libraries/STM32_USB-FS-Device_Driver/inc

# Memory Management
inc-y += mem_mang
inc-y += FreeRTOS/Source/portable/MemMang

# Logging
src-y += $(wildcard $(RC_SRC_DIR)/logging/*.c)
inc-y += $(RC_INCLUDE_DIR)/logging

# Utilities
#src-y += $(wildcard Utilities/STM32F3_Discovery/*.c)
#inc-y += Utilities/STM32F3_Discovery

src-y += $(wildcard $(RC_SRC_DIR)/util/*.c)
inc-y += $(RC_INCLUDE_DIR)/util

# serial API
src-y += $(wildcard $(RC_SRC_DIR)/serial/*.c)
inc-y += $(RC_INCLUDE_DIR)/serial

# system
src-y += $(wildcard $(RC_SRC_DIR)/system/*.c)
inc-y += $(RC_INCLUDE_DIR)/system

src-y += $(wildcard $(RC_SRC_DIR)/usart/*.c)
inc-y += $(RC_INCLUDE_DIR)/usart


#################HARDWARE ABSTRACTION LAYER#####################

# ADC
src-y += $(wildcard $(RC_SRC_DIR)/ADC/*.c)
src-y += $(wildcard hal/ADC_stm32/*.c)
inc-y += $(RC_INCLUDE_DIR)/ADC

# CAN
src-y += $(wildcard $(RC_SRC_DIR)/CAN/*.c)
src-y += $(wildcard hal/CAN_stm32/*.c)
inc-y += hal/CAN_stm32
inc-y += $(RC_INCLUDE_DIR)/CAN

# GPS
src-y += $(wildcard $(RC_SRC_DIR)/gps/*.c)
inc-y += $(RC_INCLUDE_DIR)/gps

# CPU
src-y += $(wildcard $(RC_SRC_DIR)/cpu/*.c)
src-y += $(wildcard hal/cpu_stm32/*.c)
inc-y += $(RC_INCLUDE_DIR)/cpu

# GPS skytraq
src-y += $(wildcard hal/gps_skytraq/*.c)
inc-y += $(RC_INCLUDE_DIR)/gps

#filter
src-y += $(wildcard $(RC_SRC_DIR)/filter/*.c)
inc-y += $(RC_INCLUDE_DIR)/filter

#predictive timer
src-y += $(wildcard $(RC_SRC_DIR)/predictive_timer/*.c)
inc-y += $(RC_INCLUDE_DIR)/predictive_timer

#lap stats
src-y += $(wildcard $(RC_SRC_DIR)/lap_stats/*.c)
inc-y += $(RC_INCLUDE_DIR)/lap_stats

#auto lap config
src-y += $(wildcard $(RC_SRC_DIR)/auto_config/*.c)
inc-y += $(RC_INCLUDE_DIR)/auto_config

#tracks
src-y += $(wildcard $(RC_SRC_DIR)/tracks/*.c)
inc-y += $(RC_INCLUDE_DIR)/tracks

# I2C
src-y += $(wildcard hal/i2c_stm32/*.c)
inc-y += hal/i2c_stm32

# wifi
src-y += $(wildcard hal/wifi_esp8266/*.c)

# IMU
src-y += $(wildcard $(RC_SRC_DIR)/imu/*.c)
src-y += $(wildcard hal/imu_stm32/*.c)
inc-y += hal/imu_stm32
inc-y += $(RC_INCLUDE_DIR)/imu

#launch control
src-y += $(wildcard $(RC_SRC_DIR)/launch_control.c)
inc-y += $(RC_INCLUDE_DIR)

# LED
src-y += $(wildcard hal/LED_stm32/*.c)
src-y += $(wildcard $(RC_SRC_DIR)/LED/*.c)
inc-y += $(RC_INCLUDE_DIR)/LED

# memory
src-y += $(wildcard hal/memory_stm32/*.c)
src-y += $(wildcard $(RC_SRC_DIR)/memory/*.c)
inc-y += $(RC_INCLUDE_DIR)/memory

# Units
src-y += $(wildcard $(RC_SRC_DIR)/units/*.c)
inc-y += $(RC_INCLUDE_DIR)/units

# USART
src-y += $(wildcard $(RC_SRC_DIR)/usart/*.c)
src-y += $(wildcard hal/usart_stm32/*.c)
inc-y += $(RC_INCLUDE_DIR)/usart

# Serial Device
src-y += $(wildcard hal/serial/*.c)

# Watchdog
src-y += $(wildcard $(RC_SRC_DIR)/watchdog/*.c)
src-y += $(wildcard hal/watchdog_stm32/*.c)
inc-y += $(RC_INCLUDE_DIR)/watchdog

# OBD
src-y += $(wildcard $(RC_SRC_DIR)/OBD2/*.c)
inc-y += $(RC_INCLUDE_DIR)/OBD2

#devices
src-y += $(wildcard $(RC_SRC_DIR)/devices/*.c)
inc-y += $(RC_INCLUDE_DIR)/devices

#drivers
src-y += $(wildcard $(RC_SRC_DIR)/drivers/*.c)
inc-y += $(RC_INCLUDE_DIR)/drivers

#devices
src-y += $(wildcard $(RC_SRC_DIR)/api/*.c)
inc-y += $(RC_INCLUDE_DIR)/api

#command
src-y += $(wildcard $(RC_SRC_DIR)/command/*.c)
inc-y += $(RC_INCLUDE_DIR)/command

#command
src-y += $(wildcard $(RC_SRC_DIR)/jsmn/*.c)
inc-y += $(RC_INCLUDE_DIR)/jsmn

#GSM
src-y += $(wildcard $(RC_SRC_DIR)/gsm/*.c)
inc-y += $(RC_INCLUDE_DIR)/gsm

#logger
src-y += $(wildcard $(RC_SRC_DIR)/logger/*.c)
# STRIP fileWriter.c because we don't want to compile it.
src-y := $(filter-out $(RC_SRC_DIR)/logger/fileWriter.c,$(src-y))
inc-y += $(RC_INCLUDE_DIR)/logger

#messaging
src-y += $(wildcard $(RC_SRC_DIR)/messaging/*.c)
inc-y += $(RC_INCLUDE_DIR)/messaging

#modem
src-y += $(wildcard $(RC_SRC_DIR)/modem/*.c)
inc-y += $(RC_INCLUDE_DIR)/modem

# Tasks
src-y += $(wildcard $(RC_SRC_DIR)/tasks/*.c)
inc-y += $(RC_INCLUDE_DIR)/tasks

# CDC-ACM
src-y += $(wildcard hal/usb_stm32/*.c)
src-y += $(wildcard $(RC_SRC_DIR)/usb_comm/*.c)
inc-y += hal/usb_stm32

# Combined interrupts
src-y += $(wildcard hal/common_irq_stm32/*.c)

# FreeRTOS
src-y += $(wildcard FreeRTOS/Source/*.c) \
         FreeRTOS/Source/portable/GCC/ARM_CM3/port.c \
         FreeRTOS/Source/portable/MemMang/heap_4.c \

inc-y += FreeRTOS/Source/include \
         FreeRTOS/Source/portable/GCC/ARM_CM3

#newlib
src-y += $(wildcard util/*.c)
inc-y += util


inc-y += .

# base
inc-y += $(RC_TOP_LEVEL_DIR)

# Project
src-y += $(wildcard $(RC_TOP_LEVEL_DIR)/*.c)
inc-y += Project

################################################################################
# Includes for Headers we need for compilation, but don't use their methods
################################################################################
inc-y += $(RC_INCLUDE_DIR)/GPIO
inc-y += $(RC_INCLUDE_DIR)/PWM
inc-y += $(RC_INCLUDE_DIR)/lua
inc-y += $(RC_INCLUDE_DIR)/sdcard
inc-y += $(RC_INCLUDE_DIR)/timer
inc-y += $(RC_INCLUDE_DIR)/usb_comm
inc-y += $(RC_INCLUDE_DIR)/virtual_channel
inc-y += $(RC_LIBRARY_DIR)/lua/src
inc-y += $(RC_PLATFORM_DIR)/mk2/hal/fat_sd_stm32/fatfs

################################################################################
# Compilation flags
################################################################################
obj-y := $(addprefix $(OUTDIR)/,$(patsubst %.s,%.o,$(patsubst %.c,%.o,$(src-y))))

#
# Our chip is actually a cortex-m4 with FPU.  We have not included the correct
# FreeRTOS libs as well as compiler args to make it work properly with this arch
# so for now this stays as an M3.
# -mcpu=cortex-m4 \
# -mthumb \
# -mfpu=fpv4-sp-d16 \
# -mfloat-abi=softfp \
#
CPU_FLAGS := \
-mcpu=cortex-m3 \
-mthumb \
-mfloat-abi=soft \

CFLAGS := -DUSE_STDPERIPH_DRIVER -DSTM32F30X -MD -c
CFLAGS += $(CPU_FLAGS)
CFLAGS += $(ASL_CFLAGS)
CFLAGS += $(BOARD_DEFINES)
CFLAGS += $(addprefix -I,$(inc-y))

LDFLAGS := $(CPU_FLAGS)
LDFLAGS += -TProject/STM32_FLASH.ld
LDFLAGS += -Wl,-cref,-u,Reset_Handler,-Map=main.map
LDFLAGS += -Wl,--gc-sections,-static
LDFLAGS += $(FREERTOS_LD_FLAGS)

LDLIBS :=
LDLIBS += -lc
LDLIBS += -lm

ASFLAGS := $(CPU_FLAGS) --warn

# disable auto-removing of intermediate files
.PRECIOUS: $(obj-y)

$(OUTDIR)/%.o: %.c
	@echo "[CC] $(@F)"
	mkdir -p $(@D)
	$(CCACHE) $(CC) $(CFLAGS) -c -o $@ $<

$(OUTDIR)/%.o: %.s
	@echo "[AS] $(@F)"
	mkdir -p $(@D)
	$(AS) $(ASFLAGS) -c -o $@ $<

%.ihex: %.bin
	@echo "[ASL_FW_POSTPROCESS] $(@F)"
	$(PYTHON) $(ASL_FW_POSTPROCESS) -f $< -e main.elf -o 0x08004000 -b $< -i $@

%.elf: $(sort $(obj-y))
	@echo "[LD] $(@F)"
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.bin: %.elf
	@echo "[OBJCOPY] $(@F)"
	$(OBJCOPY) -O binary $< $@

%.hex: %.elf
	@echo "[OBJCOPY] $(@F)"
	$(OBJCOPY) -O ihex $< $@

%.dis: %.elf
	@echo "[OBJDUMP] $(@F)"
	$(OBJDUMP) -D $< > $@

all: main.elf main.bin main.hex main.dis main.ihex

clean:
	rm -rf $(OUTDIR)

dbg:
	@echo "src: $(src-y)"
	@echo "obj: $(obj-y)"
	@echo "inc: $(inc-y)"

flash: main.elf
	openocd -f ./openocd_flash.cfg

gdb: main.elf
	gdb -x ./gdb_setup.gdb main.elf

.PHONY: all clean
