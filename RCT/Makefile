.SILENT:

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

################################################################################
# Project definition
################################################################################
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

# USB
src-y += $(wildcard Libraries/STM32_USB-FS-Device_Driver/src/*.c)
inc-y += Libraries/STM32_USB-FS-Device_Driver/inc

# Utilities
src-y += $(wildcard Utilities/STM32F3_Discovery/*.c)
inc-y += Utilities/STM32F3_Discovery

# Project
src-y += $(wildcard Project/*.c)
inc-y += Project

# FreeRTOS
src-y += $(wildcard FreeRTOS/Source/*.c) \
         FreeRTOS/Source/portable/GCC/ARM_CM3/port.c \
         FreeRTOS/Source/portable/MemMang/heap_1.c
inc-y += FreeRTOS/Source/include \
         FreeRTOS/Source/portable/GCC/ARM_CM3

################################################################################
# Compilation flags
################################################################################
obj-y := $(addprefix $(OUTDIR),$(patsubst %.s,%.o,$(patsubst %.c,%.o,$(src-y))))

CFLAGS := -DUSE_STDPERIPH_DRIVER -DSTM32F30X -DUSE_STM32303C_EVAL -MD
CFLAGS += -mcpu=cortex-m3 -mthumb
CFLAGS += $(addprefix -I,$(inc-y))
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
