ifeq ($(V),)
.SILENT:
endif

################################################################################
# Toolchain environment
################################################################################
export PATH := /opt/arm-2012.03/bin:$(PATH)
CROSS_COMPILE := arm-none-eabi-
AS := $(CROSS_COMPILE)gcc -x assembler-with-cpp
CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
LD := $(CROSS_COMPILE)g++
OBJDUMP := $(CROSS_COMPILE)objdump
OBJCOPY := $(CROSS_COMPILE)objcopy

MAKEFLAGS += -j16

#specify the use of the external crystal
BOARD_DEFINES = -DPLL_SOURCE_HSE

################################################################################
# Project definition
################################################################################
include ../version.mk

OUTDIR := .build/

# CMSIS
src-y := \
    Libraries/CMSIS/Device/ST/STM32F30x/Source/Templates/TrueSTUDIO/startup_stm32f30x.s
#    Libraries/CMSIS/Device/ST/STM32F30x/Source/Templates/system_stm32f30x.c
inc-y := \
    Libraries/CMSIS/Device/ST/STM32F30x/Include \
    Libraries/CMSIS/Include

# StdPeriph
src-y += $(wildcard Libraries/STM32F30x_StdPeriph_Driver/src/*.c)
inc-y += Libraries/STM32F30x_StdPeriph_Driver/inc

# Memory Management
inc-y += mem_mang
inc-y += FreeRTOS/Source/portable/MemMang

# Logging
src-y += $(wildcard ../src/logging/*.c)
inc-y += ../include/logging

# Utilities
#src-y += $(wildcard Utilities/STM32F3_Discovery/*.c)
#inc-y += Utilities/STM32F3_Discovery

src-y += $(wildcard ../src/util/*.c)
inc-y += ../include/util

# serial API
src-y += $(wildcard ../src/serial/*.c)
inc-y += ../include/serial
src-y += $(wildcard ../src/usart/*.c)
inc-y += ../include/usart


#################HARDWARE ABSTRACTION LAYER#####################

# ADC
src-y += $(wildcard ../src/ADC/*.c)
src-y += $(wildcard hal/ADC_stm32/*.c)
inc-y += ../include/ADC

# CAN
src-y += $(wildcard ../src/CAN/*.c)
src-y += $(wildcard hal/CAN_stm32/*.c)
inc-y += ../include/CAN

# GPS
src-y += $(wildcard ../src/gps/*.c)
inc-y += ../include/gps

# CPU
src-y += $(wildcard ../src/cpu/*.c)
src-y += $(wildcard hal/cpu_stm32/*.c)
inc-y += ../include/cpu

# GPS skytraq
src-y += $(wildcard hal/gps_skytraq/*.c)
inc-y += ../include/gps

#filter
src-y += $(wildcard ../src/filter/*.c)
inc-y += ../include/filter

#predictive timer
src-y += $(wildcard ../src/predictive_timer/*.c)
inc-y += ../include/predictive_timer

#lap stats
src-y += $(wildcard ../src/lap_stats/*.c)
inc-y += ../include/lap_stats

#auto lap config
src-y += $(wildcard ../src/auto_config/*.c)
inc-y += ../include/auto_config

#tracks
src-y += $(wildcard ../src/tracks/*.c)
inc-y += ../include/tracks

# I2C
src-y += $(wildcard hal/i2c_stm32/*.c)
inc-y += hal/i2c_stm32

# IMU
src-y += $(wildcard ../src/imu/*.c)
src-y += $(wildcard hal/imu_stm32/*.c)
inc-y += hal/imu_stm32
inc-y += ../include/imu

#launch control
src-y += $(wildcard ../src/launch_control.c)
inc-y += ../include

# LED
src-y += $(wildcard hal/LED_stm32/*.c)
src-y += $(wildcard ../src/LED/*.c)
inc-y += ../include/LED

# memory
src-y += $(wildcard hal/memory_stm32/*.c)
src-y += $(wildcard ../src/memory/*.c)
inc-y += ../include/memory

# USART
src-y += $(wildcard ../src/usart/*.c)
src-y += $(wildcard hal/usart_stm32/*.c)
inc-y += ../include/usart

# Watchdog
src-y += $(wildcard ../src/watchdog/*.c)
src-y += $(wildcard hal/watchdog_stm32/*.c)
inc-y += ../include/watchdog

# OBD
src-y += $(wildcard ../src/OBD2/*.c)
inc-y += ../include/OBD2

#devices
src-y += ../src/devices/bluetooth.c \
		 ../src/devices/null_device.c
inc-y += ../include/devices

#devices
src-y += $(wildcard ../src/api/*.c)
inc-y += ../include/api

#command
src-y += $(wildcard ../src/command/*.c)
inc-y += ../include/command

#command
src-y += $(wildcard ../src/jsmn/*.c)
inc-y += ../include/jsmn

#logger
src-y += $(wildcard ../src/logger/*.c)
# STRIP fileWriter.c because we don't want to compile it.
src-y := $(filter-out ../src/logger/fileWriter.c,$(src-y))
inc-y += ../include/logger

#messaging
src-y += $(wildcard ../src/messaging/*.c)
inc-y += ../include/messaging

# FreeRTOS
src-y += $(wildcard FreeRTOS/Source/*.c) \
         FreeRTOS/Source/portable/GCC/ARM_CM3/port.c \
         FreeRTOS/Source/portable/MemMang/heap_4.c \

inc-y += FreeRTOS/Source/include \
         FreeRTOS/Source/portable/GCC/ARM_CM3

src-y += Project/system_stm32f30x.c

#newlib
src-y += $(wildcard util/*.c)
inc-y += util


inc-y += .

# base
inc-y += ..

# Project
src-y += $(wildcard ../*.c)
inc-y += Project

################################################################################
# Includes for Headers we need for compilation, but don't use their methods
################################################################################
inc-y += ../include/GPIO
inc-y += ../include/lua
inc-y += ../include/sdcard
inc-y += ../include/usb_comm
inc-y += ../lib_lua/src
inc-y += ../stm32_base/hal/fat_sd_stm32/fatfs

################################################################################
# Compilation flags
################################################################################
obj-y := $(addprefix $(OUTDIR),$(patsubst %.s,%.o,$(patsubst %.c,%.o,$(src-y))))

CFLAGS := -DUSE_STDPERIPH_DRIVER -DSTM32F30X -DUSE_STM32303C_EVAL -MD -std=c99
CFLAGS += -mcpu=cortex-m3 -mthumb
CFLAGS += -DAPI_REV=$(API) -DMAJOR_REV=$(MAJOR) -DMINOR_REV=$(MINOR) -DBUGFIX_REV=$(BUGFIX)
CFLAGS += $(BOARD_DEFINES)
CFLAGS += -fno-builtin-sprintf
CFLAGS += $(addprefix -I,$(inc-y))
CFLAGS += -O0 -g
LDFLAGS := -mcpu=cortex-m3 -mthumb -TProject/STM32_FLASH.ld -Wl,-cref,-u,Reset_Handler,-Map=$(OUTDIR)main.map,--gc-sections
AFLAGS := $(CFLAGS)

# disable auto-removing of intermediate files
.PRECIOUS: $(obj-y)

$(OUTDIR)%.o: %.c
	@echo "[CC] $(@F)"
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OUTDIR)%.o: %.s
	@echo "[AS] $(@F)"
	mkdir -p $(@D)
	$(AS) $(AFLAGS) -c -o $@ $<

$(OUTDIR)%.elf: $(obj-y)
	@echo "[LD] $(@F)"
	$(LD) $(LDFLAGS) -o $@ $^

$(OUTDIR)%.bin: $(OUTDIR)%.elf
	@echo "[OBJCOPY] $(@F)"
	$(OBJCOPY) -O binary $< $@

$(OUTDIR)%.hex: $(OUTDIR)%.elf
	@echo "[OBJCOPY] $(@F)"
	$(OBJCOPY) -O ihex $< $@

$(OUTDIR)%.dis: $(OUTDIR)%.elf
	@echo "[OBJDUMP] $(@F)"
	$(OBJDUMP) -D $< > $@

.PHONY: all
all: $(OUTDIR)main.elf $(OUTDIR)main.bin $(OUTDIR)main.hex $(OUTDIR)main.dis

.PHONY: clean
clean:
	rm -rf $(OUTDIR)

flash: $(OUTDIR)main.hex
	openocd -f board/stm32f3discovery.cfg -f ./flash.cfg

dbg:
	@echo "src: $(src-y)"
	@echo "obj: $(obj-y)"
	@echo "inc: $(inc-y)"

gdb: $(OUTDIR)main.elf
	openocd -f board/stm32f3discovery.cfg -c "init" -c "reset init" -c halt
