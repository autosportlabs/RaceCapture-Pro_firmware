# This file is part of the Racecapture/Track project
#
# Copyright (C) 2013 Autosport Labs
#
# Author(s):
# 	Andrey Smirnov <andrew.smirnov@gmail.com>
# 	Jeff Ciesielski <jeffciesielski@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# This file can be used to autoconfigure your board for ease of
# building, otherwise it passes the BOARD environment variable to the
# rest of the makefile

APP_PATH = ..

#load the board specific configuration
include config.mk

ifeq ($(CPU_TYPE),)
$(error CPU_TYPE is not defined, please ensure it is defined in your cpu config.mk)
endif

PREFIX	?= arm-none-eabi

CC      := $(PREFIX)-gcc
AS      := $(PREFIX)-as
CXX     := $(PREFIX)-g++
AR      := $(PREFIX)-ar
NM      := $(PREFIX)-nm
OBJCOPY := $(PREFIX)-objcopy
OBJDUMP := $(PREFIX)-objdump
SIZE    := $(PREFIX)-size

OPENOCD := openocd
DDD     := ddd
GDB     := $(PREFIX)-gdb

#application libraries
include libs/stm32f4_periph.mk
include libs/freertos.mk
include libs/stm32_usb.mk

CFLAGS ?= -Os -g -Wall -fno-common -c -mthumb -fno-strict-aliasing \
		-Wuninitialized -Wsign-compare -mcpu=$(CPU_TYPE) -MD \
		-std=gnu99

LUA_INCLUDE=-I$(APP_PATH)/lib_lua/src
LUA_LIBS=lua
LUA_LIB_DIR=$(APP_PATH)/lib_lua/src

INCLUDES += $(CPU_INCLUDES) $(BOARD_INCLUDES) $(LIB_INCLUDES) $(APP_INCLUDES) $(LUA_INCLUDE)
CFLAGS += $(INCLUDES) $(CPU_DEFINES) $(BOARD_DEFINES) $(APP_DEFINES) $(CPU_FLAGS)
ASFLAGS += -mcpu=$(CPU_TYPE) $(FPU) -g -Wa,--warn

LIBS = -lnosys
LIBS += $(addprefix -l,$(BASE_LIBS))
LIBS += $(addprefix -l,$(LUA_LIBS))

LDFLAGS ?= --specs=nano.specs -lc -lgcc $(LIBS) -mcpu=$(CPU_TYPE) -g -ggdb \
	-L. -Lcpu/common -L$(LUA_LIB_DIR) -L$(CPU_BASE) -T$(CPU_LINK_MEM) -Tlink_sections.ld \
	-nostartfiles -Wl,--gc-sections -mthumb -mcpu=$(CPU_TYPE) \
	-msoft-float

# Be silent per default, but 'make V=1' will show all compiler calls.
ifneq ($(V),1)

Q := @
# Do not print "Entering directory ...".
MAKEFLAGS += --no-print-directory
endif

# common objects
OBJS += $(CPU_OBJS) $(BOARD_OBJS) $(APP_OBJS)

ifeq ($(TARGET),)
$(error TARGET is not defined, please define it in your applications config.mk)
endif

ASL_FW_POSTPROCESS=asl_f4_fw_postprocess
ifeq ($(PYTHON),python2.7)
ASL_FW_POSTPROCESS=/usr/local/bin/asl_f4_fw_postprocess
else
PYTHON=
endif

LIBS_ALL = $(addprefix lib,$(BASE_LIBS:=.a))

all: $(LIBS_ALL) $(TARGET).ihex

libfreertos.a: $(FREERTOS_OBJS)
	$(Q)$(AR) $(ARFLAGS) $@ $^

libstm32f4_periph.a: $(STM32F4_PERIPH_OBJS)
	$(Q)$(AR) $(ARFLAGS) $@ $^

libstm32_usb.a: $(STM32_USB_OBJS)
	$(Q)$(AR) $(ARFLAGS) $@ $^

$(TARGET).ihex: $(TARGET).bin
	@printf "  Creating upgrade images\n"
	$(PYTHON) $(ASL_FW_POSTPROCESS) -f $(TARGET).bin -o 0x08020000 -b $(TARGET).bin -i $(TARGET).ihex

$(TARGET).bin: $(TARGET).elf
	@printf "  OBJCOPY $(subst $(shell pwd)/,,$(@))\n"
	$(Q)$(PREFIX)-objcopy -Obinary $< $@

$(TARGET).elf: $(OBJS)
	@printf "  LD      $(subst $(shell pwd)/,,$(@))\n"
	$(Q)$(CC) $(OBJS) $(LDFLAGS) -o $@

.c.o:
	@printf "  CC      $(subst $(shell pwd)/,,$(@))\n"
	$(Q)$(CC) $(CFLAGS) -D_RCP_BASE_FILE_="\"$(notdir $<): \"" $< -o $@

.s.o:
	@printf "  AS      $(subst $(shell pwd)/,,$(@))\n"
	$(Q)$(CC) $(ASFLAGS) -c $< -o $@

clean:
	$(Q)rm -f *.o *.a *.d ../*.o ../*.d \
	$(OBJS) \
	$(LIBS_ALL)\
	$(STM32F4_PERIPH_OBJS) \
	$(FREERTOS_OBJS) \
	$(STM32_USB_OBJS)\
	$(shell find . -name "*.d") \
	$(TARGET).bin \
	$(TARGET).elf

flash: $(TARGET).bin
	$(APP_FLASH)

dumpasm: $(TARGET).elf
	$(PREFIX)-objdump -D $< > $(TARGET).asm

debug: $(TARGET).bin
	$(OPENOCD) -f $(APP_PATH)/openocd_stlinkv2.cfg

ddd: $(TARGET).elf
	$(DDD) --eval-command="target remote localhost:3333" --eval-command="set env LD_LIBRARY_PATH ../lib_lua/src" --debugger $(GDB) $(TARGET).elf

gdb: $(TARGET).elf
	$(GDB) -x gdb_stlinkv2.gdb

.PHONY: clean flash debug ddd dumpasm
