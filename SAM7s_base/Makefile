# Hey Emacs, this is a -*- makefile -*-
#
# WinARM template makefile
# by Martin Thomas, Kaiserslautern, Germany
# <eversmith@heizung-thomas.de>
#
# based on the WinAVR makefile written by Eric B. Weddington, J�rg Wunsch, et al.
# Released to the Public Domain
# Please read the make user manual!
#
#
# On command line:
#
# make all = Make software.
#
# make clean = Clean out built project files.
#
# make program = Download the hex file to the device
#
# (TODO: make filename.s = Just compile filename.c into the assembler code only)
#
# To rebuild project do "make clean" then "make all".
#
# Changelog:
# - 17. Feb. 2005  - added thumb-interwork support (mth)
# - 28. Apr. 2005  - added C++ support (mth)
# - 29. Arp. 2005  - changed handling for lst-Filename (mth)
# -  1. Nov. 2005  - exception-vector placement options (mth)
# - 15. Nov. 2005  - added library-search-path (EXTRA_LIB...) (mth)
# -  2. Dec. 2005  - fixed ihex and binary file extensions (mth)
#

include version.mk
TRACKS_MEMORY_LENGTH = 10240
CHANNELS_MEMORY_LENGTH = 3584
CONFIG_MEMORY_LENGTH = 4096

RCP_RELEASE_DIR ?= .
ifdef BUILD_NUMBER
	BUILD_NUMBER_SUFFIX = _$(BUILD_NUMBER)
else
	BUILD_NUMBER_SUFFIX =
endif

RELEASE_NAME = RaceCapturePro-$(MAJOR).$(MINOR).$(BUGFIX)$(BUILD_NUMBER_SUFFIX)
RELEASE_NAME_ZIP = $(RELEASE_NAME).zip
RELEASE_NAME_BIN = $(RELEASE_NAME).bin
RELEASE_NAME_ELF = $(RELEASE_NAME).elf
RCP_INSTALL_DIR = RaceCapturePro_Firmware

# MCU name and submodel
MCU      = arm7tdmi
SUBMDL   = AT91SAM7S256
#SUBMDL   = AT91SAM7S64
THUMB    = -mthumb
THUMB_IW = -mthumb-interwork


## Create ROM-Image (final)
RUN_MODE=ROM_RUN

## Exception-Vector placement only supported for "ROM_RUN"
## (placement settings ignored when using "RAM_RUN")
## - Exception vectors in ROM:
#VECTOR_LOCATION=VECTORS_IN_ROM
## - Exception vectors in RAM:
VECTOR_LOCATION=VECTORS_IN_RAM


## Output format. (can be ihex or binary)
#FORMAT = ihex
FORMAT = binary


# Target file name (without extension).
TARGET = main


# Common directories
INCLUDE_DIR = include
SRC_DIR = src

SAM7s_BASE_DIR = SAM7s_base
GPS_MTK_DIR = $(SAM7s_BASE_DIR)/gps_mtk
MCP2515_DIR = $(SAM7s_BASE_DIR)/MCP2515
KXR94_2353_DIR = $(SAM7s_BASE_DIR)/KXR94_2353
SIM900_AT91_DIR = $(SAM7s_BASE_DIR)/sim900_at91
ADC_AT91_DIR = $(SAM7s_BASE_DIR)/ADC_at91
PWM_AT91_DIR = $(SAM7s_BASE_DIR)/PWM_at91
SPI_AT91_DIR = $(SAM7s_BASE_DIR)/SPI_at91
TIMER_AT91_DIR= $(SAM7s_BASE_DIR)/timer_at91
LED_AT91_DIR= $(SAM7s_BASE_DIR)/LED_at91
GPIO_AT91_DIR= $(SAM7s_BASE_DIR)/GPIO_at91
WATCHDOG_AT91_DIR= $(SAM7s_BASE_DIR)/watchdog_at91
MEMORY_AT91_DIR= $(SAM7s_BASE_DIR)/memory_at91
CPU_AT91_DIR= $(SAM7s_BASE_DIR)/cpu_at91
USART_AT91_DIR= $(SAM7s_BASE_DIR)/usart_at91
MEM_MANG_DIR = $(SAM7s_BASE_DIR)/mem_mang
HW_DIR = $(SAM7s_BASE_DIR)/hardware
RTOS_DIR = $(SAM7s_BASE_DIR)/FreeRTOS
RTOS_SRC_DIR = $(RTOS_DIR)/Source
RTOS_PORT_DIR = $(RTOS_SRC_DIR)/portable
RTOS_GCC_DIR = $(RTOS_PORT_DIR)/GCC/ARM7_AT91SAM7S
RTOS_MEMMANG_DIR = $(RTOS_PORT_DIR)/MemMang
USB_AT91_DIR = $(SAM7s_BASE_DIR)/usb_at91
USB_COMM_SRC_DIR = $(SRC_DIR)/usb_comm
SERIAL_SRC_DIR = $(SRC_DIR)/serial
AUTO_CONFIG_SRC_DIR = $(SRC_DIR)/auto_config
LOGGER_SRC_DIR = $(SRC_DIR)/logger
CHANNELS_SRC_DIR = $(SRC_DIR)/channels
TRACKS_SRC_DIR = $(SRC_DIR)/tracks
SPI_DIR = $(SRC_DIR)/spi
CPU_SRC_DIR = $(SRC_DIR)/cpu
MEMORY_SRC_DIR = $(SRC_DIR)/memory
LUA_SRC_DIR = $(SRC_DIR)/lua
CMD_SRC_DIR = $(SRC_DIR)/command
FILTER_DIR = $(SRC_DIR)/filter
CAN_DIR = $(SRC_DIR)/CAN
OBD2_DIR =$(SRC_DIR)/OBD2
ADC_DIR = $(SRC_DIR)/ADC
USART_DIR = $(SRC_DIR)/usart
TIMER_DIR = $(SRC_DIR)/timer
PWM_DIR = $(SRC_DIR)/PWM
LED_DIR = $(SRC_DIR)/LED
GPIO_DIR = $(SRC_DIR)/GPIO
WATCHDOG_DIR = $(SRC_DIR)/watchdog
IMU_SRC_DIR = $(SRC_DIR)/imu
LOGGING_DIR = $(SRC_DIR)/logging
TASKS_DIR = tasks
JSMN_SRC_DIR = $(SRC_DIR)/jsmn
API_SRC_DIR = $(SRC_DIR)/api
MESSAGING_SRC_DIR = $(SRC_DIR)/messaging
DEVICES_SRC_DIR = $(SRC_DIR)/devices
PRED_TIMER_DIR = $(SRC_DIR)/predictive_timer
VIRTUAL_CHANNEL_SRC_DIR = $(SRC_DIR)/virtual_channel
UTIL_DIR = $(SRC_DIR)/util
JSON_DIR = ./json
GPS_SRC_DIR = $(SRC_DIR)/gps

#App specific dirs
FAT_SD_SRC_DIR = $(SAM7s_BASE_DIR)/fat_sd_at91
SDCARD_SRC_DIR = $(SRC_DIR)/sdcard

# List C source files here. (C dependencies are automatically generated.)
# use file-extension c for "c-only"-files
SRC	= \
$(TARGET).c \
$(JSMN_SRC_DIR)/jsmn.c \
$(CMD_SRC_DIR)/baseCommands.c \
$(API_SRC_DIR)/api.c \
$(LOGGER_SRC_DIR)/loggerApi.c \
$(HW_DIR)/lib_AT91SAM7S256.c \
$(RTOS_SRC_DIR)/tasks.c \
$(RTOS_SRC_DIR)/queue.c \
$(RTOS_SRC_DIR)/list.c \
$(RTOS_GCC_DIR)/port.c \
$(UTIL_DIR)/modp_numtoa.c \
$(UTIL_DIR)/modp_atonum.c \
$(UTIL_DIR)/taskUtil.c \
$(USB_AT91_DIR)/USB-CDC_device_at91.c \
$(USB_COMM_SRC_DIR)/usb_comm.c \
$(USART_DIR)/usart.c \
$(CMD_SRC_DIR)/command.c \
$(SERIAL_SRC_DIR)/serial.c \
$(FAT_SD_SRC_DIR)/fattime.c \
$(FAT_SD_SRC_DIR)/ff.c \
$(FAT_SD_SRC_DIR)/sd_spi_at91.c \
$(FAT_SD_SRC_DIR)/rtc.c \
$(SDCARD_SRC_DIR)/sdcard.c \
$(IMU_SRC_DIR)/imu.c \
$(LOGGER_SRC_DIR)/sampleRecord.c \
$(LOGGER_SRC_DIR)/fileWriter.c \
$(LOGGER_SRC_DIR)/loggerHardware.c \
$(LOGGER_SRC_DIR)/loggerData.c \
$(LOGGER_SRC_DIR)/loggerSampleData.c \
$(LOGGER_SRC_DIR)/loggerTaskEx.c \
$(LOGGER_SRC_DIR)/connectivityTask.c \
$(GPS_SRC_DIR)/gps.c \
$(GPS_SRC_DIR)/geoCircle.c \
$(GPS_SRC_DIR)/gpsTask.c \
$(GPS_SRC_DIR)/dateTime.c \
$(LOGGER_SRC_DIR)/loggerConfig.c \
$(CHANNELS_SRC_DIR)/channelMeta.c \
$(TRACKS_SRC_DIR)/tracks.c \
$(GPS_SRC_DIR)/geopoint.c \
$(LOGGER_SRC_DIR)/luaLoggerBinding.c \
$(LOGGER_SRC_DIR)/loggerCommands.c \
$(MEMORY_SRC_DIR)/memory.c \
$(CPU_SRC_DIR)/cpu.c \
$(SPI_AT91_DIR)/spi.c \
$(MCP2515_DIR)/CAN_device_MCP2515.c \
$(GPS_MTK_DIR)/gps_device_mtk3339.c \
$(KXR94_2353_DIR)/imu_device_KXR94_2353.c \
$(ADC_AT91_DIR)/ADC_device_at91.c \
$(SIM900_AT91_DIR)/sim900_device_at91.c \
$(TIMER_AT91_DIR)/timer_device_at91.c \
$(PWM_AT91_DIR)/PWM_device_at91.c \
$(LED_AT91_DIR)/LED_device_at91.c \
$(GPIO_AT91_DIR)/GPIO_device_at91.c \
$(USART_AT91_DIR)/usart_device_at91.c \
$(WATCHDOG_AT91_DIR)/watchdog_device_at91.c \
$(MEMORY_AT91_DIR)/memory_device_at91.c \
$(CPU_AT91_DIR)/cpu_device_at91.c \
$(GPIO_DIR)/gpioTasks.c \
$(LUA_SRC_DIR)/luaTask.c \
$(LUA_SRC_DIR)/luaScript.c \
$(LUA_SRC_DIR)/luaBaseBinding.c \
$(LUA_SRC_DIR)/luaCommands.c \
$(RTOS_PORT_DIR)/MemMang/heap_4.c \
$(LOGGING_DIR)/printk.c \
$(FILTER_DIR)/filter.c \
$(CAN_DIR)/CAN.c \
$(OBD2_DIR)/OBD2.c \
$(OBD2_DIR)/OBD2_task.c \
$(ADC_DIR)/ADC.c \
$(TIMER_DIR)/timer.c \
$(PWM_DIR)/PWM.c \
$(LED_DIR)/LED.c \
$(GPIO_DIR)/GPIO.c \
$(WATCHDOG_DIR)/watchdog.c \
$(LOGGING_DIR)/ring_buffer.c \
$(MESSAGING_SRC_DIR)/messaging.c \
$(PRED_TIMER_DIR)/predictive_timer_2.c \
$(UTIL_DIR)/linear_interpolate.c \
$(DEVICES_SRC_DIR)/cellModem.c \
$(DEVICES_SRC_DIR)/null_device.c \
$(DEVICES_SRC_DIR)/bluetooth.c \
$(DEVICES_SRC_DIR)/sim900.c \
$(VIRTUAL_CHANNEL_SRC_DIR)/virtual_channel.c \
$(AUTO_CONFIG_SRC_DIR)/auto_track.c \
$(SRC_DIR)/launch_control.c \


# List C source files here which must be compiled in ARM-Mode.
# use file-extension c for "c-only"-files
SRCARM = $(RTOS_GCC_DIR)/portISR.c \
$(USB_AT91_DIR)/USBIsr.c \
$(GPIO_AT91_DIR)/gpioIsr.c \
$(USART_AT91_DIR)/serialIsr.c \
$(TIMER_AT91_DIR)/timerIsr.c \
$(LOGGER_SRC_DIR)/onTickHandler.c

# List C++ source files here.
# use file-extension cpp for C++-files (use extension .cpp)
CPPSRC =

# List C++ source files here which must be compiled in ARM-Mode.
# use file-extension cpp for C++-files (use extension .cpp)
#CPPSRCARM = $(TARGET).cpp
CPPSRCARM =

# List Assembler source files here.
# Make them always end in a capital .S.  Files ending in a lowercase .s
# will not be considered source files but generated files (assembler
# output from the compiler), and will be deleted upon "make clean"!
# Even though the DOS/Win* filesystem matches both .s and .S the same,
# it will preserve the spelling of the filenames, and gcc itself does
# care about how the name is spelled on its command-line.
ASRC =

# List Assembler source files here which must be assembled in ARM-Mode..
ASRCARM = $(SAM7s_BASE_DIR)/startup_SAM7S.S
#ASRCARM = crt.S

# Optimization level, can be [0, 1, 2, 3, s].
# 0 = turn off optimization. s = optimize for size.
# (Note: 3 is not always the best optimization level. See avr-libc FAQ.)
OPT = s

# Debugging format.
# Native formats for AVR-GCC's -g are stabs [default], or dwarf-2.
# AVR (extended) COFF requires stabs, plus an avr-objcopy run.
#DEBUG = stabs
#DEBUG = dwarf-2
DEBUG = -g
#DEBUG =

# List any extra directories to look for include files here.
#     Each directory must be seperated by a space.
EXTRAINCDIRS = lib_lua/src

# List any extra directories to look for library files here.
#     Each directory must be seperated by a space.
EXTRA_LIBDIRS = lib_lua/src

# Compiler flag to set the C Standard level.
# c89   - "ANSI" C
# gnu89 - c89 plus GCC extensions
# c99   - ISO C99 standard (not yet fully implemented)
# gnu99 - c99 plus GCC extensions
CSTANDARD = -std=gnu99

# Place -D or -U options for C here
CDEFS  = -D$(RUN_MODE) -DMAJOR_REV=$(MAJOR) -DMINOR_REV=$(MINOR) -DBUGFIX_REV=$(BUGFIX) -DTRACKS_MEMORY_LENGTH=$(TRACKS_MEMORY_LENGTH) -DCHANNELS_MEMORY_LENGTH=$(CHANNELS_MEMORY_LENGTH) -DCONFIG_MEMORY_LENGTH=$(CONFIG_MEMORY_LENGTH)




# Compile for SAM7 using GCC
CDEFS += -DSAM7_GCC

# If using THUMB mode, then add this for the assembly process
CDEFS += -DTHUMB_INTERWORK

# Place -I options here
CINCS = \
-I. \
-I$(SAM7s_BASE_DIR) \
-I$(RTOS_MEMMANG_DIR) \
-I$(UTIL_DIR) \
-I$(SAM7S_UTIL_DIR) \
-I$(LUA_SRC_DIR) \
-I$(MEM_MANG_DIR) \
-I$(FAT_SD_SRC_DIR) \
-I$(SPI_DIR) \
-I$(UART_SRC_DIR) \
-I$(IMU_SRC_DIR) \
-I$(LOGGER_SRC_DIR) \
-I$(USB_AT91_DIR) \
-I$(MEMORY_AT91_DIR) \
-I$(HW_DIR)/include \
-I$(RTOS_SRC_DIR)/include \
-I$(RTOS_GCC_DIR) \
-I$(CMD_SRC_DIR) \
-I$(INCLUDE_DIR) \
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
-I$(INCLUDE_DIR)/serial \
-I$(INCLUDE_DIR)/timer \
-I$(INCLUDE_DIR)/PWM \
-I$(INCLUDE_DIR)/LED \
-I$(INCLUDE_DIR)/GPIO \
-I$(GPIO_AT91_DIR) \
-I$(SPI_AT91_DIR) \
-I$(INCLUDE_DIR)/watchdog \
-I$(INCLUDE_DIR)/memory \
-I$(INCLUDE_DIR)/cpu \
-I$(INCLUDE_DIR)/spi \
-I$(INCLUDE_DIR)/lua \
-I$(INCLUDE_DIR)/imu \
-I$(INCLUDE_DIR)/usart \
-I$(INCLUDE_DIR)/usb_comm \
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

#CINCS = -I. -I$(HW_DIR)/include -I$(RTOS_SRC_DIR)/include -I$(RTOS_GCC_DIR)
# Place -D or -U options for ASM here
ADEFS =  -D$(RUN_MODE)

ifdef VECTOR_LOCATION
CDEFS += -D$(VECTOR_LOCATION)
ADEFS += -D$(VECTOR_LOCATION)
endif

# Compiler flags.
#  -g*:          generate debugging information
#  -O*:          optimization level
#  -f...:        tuning, see GCC manual and avr-libc documentation
#  -Wall...:     warning level
#  -Wa,...:      tell GCC to pass this to the assembler.
#    -adhlns...: create assembler listing
#
# Flags for C and C++ (arm-elf-gcc/arm-elf-g++)
CFLAGS = $(DEBUG)
CFLAGS += $(CDEFS) $(CINCS)
CFLAGS += -O$(OPT)
CFLAGS += -Wall
CFLAGS += -Wsign-compare
CFLAGS += -Wuninitialized
CFLAGS += -Werror
CFLAGS += -fomit-frame-pointer
CFLAGS += -Wcast-align -Wimplicit
CFLAGS += -Wpointer-arith -Wswitch
CFLAGS += -Wredundant-decls -Wreturn-type -Wshadow -Wunused
CFLAGS += -ffunction-sections -fdata-sections
#CFLAGS += -Wno-strict-aliasing -Wextra
CFLAGS += -Wa,-adhlns=$(subst $(suffix $<),.lst,$<)
CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))
#AT91-lib warnings with:
##CFLAGS += -Wcast-qual


# flags only for C
CONLYFLAGS += -Wnested-externs
CONLYFLAGS += $(CSTANDARD)
#AT91-lib warnings with:
##CONLYFLAGS += -Wmissing-prototypes
##CONLYFLAGS += -Wstrict-prototypes
##CONLYFLAGS += -Wmissing-declarations


# flags only for C++ (arm-elf-g++)
# CPPFLAGS = -fno-rtti -fno-exceptions
CPPFLAGS =

# Assembler flags.
#  -Wa,...:   tell GCC to pass this to the assembler.
#  -ahlms:    create listing
#  -g$(DEBUG):have the assembler create line number information
ASFLAGS = $(ADEFS) -Wa,-adhlns=$(<:.S=.lst), $(DEBUG)


#Additional libraries.

# Extra libraries
#    Each library-name must be seperated by a space.
EXTRA_LIBS = lua m

#Support for newlibc-lpc (file: libnewlibc-lpc.a)
#NEWLIBLPC = -lnewlib-lpc

MATH_LIB = -lm

# CPLUSPLUS_LIB = -lstdc++

# Linker flags.
#  -Wl,...:     tell GCC to pass this to linker.
#    -Map:      create map file
#    --cref:    add cross reference to  map file
LDFLAGS = -nostartfiles -Wl,-Map=$(TARGET).map,--cref,--gc-sections
LDFLAGS += -lc
LDFLAGS += $(NEWLIBLPC) $(MATH_LIB)
LDFLAGS += -lc -lgcc
LDFLAGS += $(CPLUSPLUS_LIB)
LDFLAGS += $(patsubst %,-L%,$(EXTRA_LIBDIRS))
LDFLAGS += $(patsubst %,-l%,$(EXTRA_LIBS))


# Set Linker-Script Depending On Selected Memory and Controller
LDFLAGS +=-T$(SAM7s_BASE_DIR)/$(SUBMDL)-ROM.ld




# ---------------------------------------------------------------------------
# Flash-Programming support using lpc21isp by Martin Maurer
# only for Philips LPC and Analog ADu ARMs
#
# Settings and variables:
#LPC21ISP = lpc21isp
LPC21ISP = lpc21isp_beta
LPC21ISP_PORT = com1
LPC21ISP_BAUD = 115200
LPC21ISP_XTAL = 14746
LPC21ISP_FLASHFILE = $(TARGET).hex
# verbose output:
## LPC21ISP_DEBUG = -debug
# enter bootloader via RS232 DTR/RTS (only if hardware supports this
# feature - see Philips AppNote):
LPC21ISP_CONTROL = -control


# ---------------------------------------------------------------------------

# Define directories, if needed.
## DIRARM = c:/WinARM/
## DIRARMBIN = $(DIRAVR)/bin/
## DIRAVRUTILS = $(DIRAVR)/utils/bin/

# Define programs and commands.
SHELL = sh

#gnuarm settings
CC = arm-elf-gcc
CPP = arm-elf-g++
OBJCOPY = arm-elf-objcopy
OBJDUMP = arm-elf-objdump
SIZE = arm-elf-size
NM = arm-elf-nm

#CodeSourcery settings
#CC = arm-none-eabi-gcc
#CPP = arm-none-eabi-g++
#OBJCOPY = arm-none-eabi-objcopy
#OBJDUMP = arm-none-eabi-objdump
#SIZE = arm-none-eabi-size
#NM = arm-none-eabi-nm


REMOVE = rm -f
COPY = cp


# Define Messages
# English
MSG_ERRORS_NONE = Errors: none
MSG_BEGIN = "-------- begin (mode: $(RUN_MODE)) --------"
MSG_END = --------  end  --------
MSG_SIZE_BEFORE = Size before:
MSG_SIZE_AFTER = Size after:
MSG_FLASH = Creating load file for Flash:
MSG_EXTENDED_LISTING = Creating Extended Listing:
MSG_SYMBOL_TABLE = Creating Symbol Table:
MSG_LINKING = Linking:
MSG_COMPILING = Compiling C:
MSG_COMPILING_ARM = "Compiling C (ARM-only):"
MSG_COMPILINGCPP = Compiling C++:
MSG_COMPILINGCPP_ARM = "Compiling C++ (ARM-only):"
MSG_ASSEMBLING = Assembling:
MSG_ASSEMBLING_ARM = "Assembling (ARM-only):"
MSG_CLEANING = Cleaning project:
MSG_FORMATERROR = Can not handle output-format
MSG_LPC21_RESETREMINDER = You may have to bring the target in bootloader-mode now.


# Define all object files.
COBJ      = $(SRC:.c=.o)
AOBJ      = $(ASRC:.S=.o)
COBJARM   = $(SRCARM:.c=.o)
AOBJARM   = $(ASRCARM:.S=.o)
CPPOBJ    = $(CPPSRC:.cpp=.o)
CPPOBJARM = $(CPPSRCARM:.cpp=.o)

# Define all listing files.
LST = $(ASRC:.S=.lst) $(ASRCARM:.S=.lst) $(SRC:.c=.lst) $(SRCARM:.c=.lst)
LST += $(CPPSRC:.cpp=.lst) $(CPPSRCARM:.cpp=.lst) $(COBJ:.o=.lst) $(SRC:.c=.lst)

# Compiler flags to generate dependency files.
### GENDEPFLAGS = -Wp,-M,-MP,-MT,$(*F).o,-MF,.dep/$(@F).d
GENDEPFLAGS = -MD -MP -MF .dep/$(@F).d

# Combine all necessary flags and optional flags.
# Add target processor to flags.
ALL_CFLAGS = -mcpu=$(MCU) $(THUMB_IW) -I. $(CFLAGS) $(GENDEPFLAGS)
ALL_ASFLAGS = -mcpu=$(MCU) $(THUMB_IW) -I. -x assembler-with-cpp $(ASFLAGS)


# Default target.
all:
	$(MAKE) begin
	$(MAKE) gccversion
	$(MAKE) sizebefore
	$(MAKE) build
	$(MAKE) sizeafter
	$(MAKE) finished
	$(MAKE) end

ifeq ($(FORMAT),ihex)
build: elf hex lss sym
hex: $(TARGET).hex
else
ifeq ($(FORMAT),binary)
build: elf bin lss sym
bin: $(TARGET).bin
else
$(error "$(MSG_FORMATERROR) $(FORMAT)")
endif
endif

elf: $(TARGET).elf
lss: $(TARGET).lss
sym: $(TARGET).sym

# Eye candy.
begin:
	@echo
	@echo $(MSG_BEGIN)
	@echo
	@echo $(PATH)

finished:
	@echo $(MSG_ERRORS_NONE)

end:
	@echo $(MSG_END)
	@echo


# Display size of file.
HEXSIZE = $(SIZE) --target=$(FORMAT) $(TARGET).hex
ELFSIZE = $(SIZE) -A $(TARGET).elf
sizebefore:
	@if [ -f $(TARGET).elf ]; then echo; echo $(MSG_SIZE_BEFORE); $(ELFSIZE); echo; fi

sizeafter:
	@if [ -f $(TARGET).elf ]; then echo; echo $(MSG_SIZE_AFTER); $(ELFSIZE); echo; fi


# Display compiler version information.
gccversion :
	@$(CC) --version


# Create final output files (.hex, .eep) from ELF output file.
# TODO: handle an .eeprom-section but should be redundant
%.hex: %.elf
	@echo
	@echo $(MSG_FLASH) $@
	@$(OBJCOPY) -O $(FORMAT) $< $@

# Create final output file (.bin) from ELF output file.
%.bin: %.elf
	@echo
	@echo $(MSG_FLASH) $@
	@$(OBJCOPY) -O $(FORMAT) $< $@


# Create extended listing file from ELF output file.
# testing: option -C
%.lss: %.elf
	@echo
#	@echo $(MSG_EXTENDED_LISTING) $@
#	$(OBJDUMP) -h -S -C $< > $@


# Create a symbol table from ELF output file.
%.sym: %.elf
	@echo
	@echo $(MSG_SYMBOL_TABLE) $@
	@$(NM) -n $< > $@


# Link: create ELF output file from object files.
.SECONDARY : $(TARGET).elf
.PRECIOUS : $(AOBJARM) $(AOBJ) $(COBJARM) $(COBJ) $(CPPOBJ) $(CPPOBJARM)
%.elf:  $(AOBJARM) $(AOBJ) $(COBJARM) $(COBJ) $(CPPOBJ) $(CPPOBJARM)
	@echo
	@echo $(MSG_LINKING) $@
	$(CC) $(THUMB) $(ALL_CFLAGS) $(AOBJARM) $(AOBJ) $(COBJARM) $(COBJ) $(CPPOBJ) $(CPPOBJARM) --output $@ $(LDFLAGS)

# Compile: create object files from C source files. ARM/Thumb
$(COBJ) : %.o : %.c
	@echo
	@echo $(MSG_COMPILING) $<
	$(CC) -c $(THUMB) $(ALL_CFLAGS) $(CONLYFLAGS) $< -o $@

# Compile: create object files from C source files. ARM-only
$(COBJARM) : %.o : %.c
	@echo
	@echo $(MSG_COMPILING_ARM) $<
	$(CC) -c $(ALL_CFLAGS) $(CONLYFLAGS) $< -o $@

# Compile: create object files from C++ source files. ARM/Thumb
$(CPPOBJ) : %.o : %.cpp
	@echo
	@echo $(MSG_COMPILINGCPP) $<
	@$(CPP) -c $(THUMB) $(ALL_CFLAGS) $(CPPFLAGS) $< -o $@

# Compile: create object files from C++ source files. ARM-only
$(CPPOBJARM) : %.o : %.cpp
	@echo
	@echo $(MSG_COMPILINGCPP_ARM) $<
	@$(CPP) -c $(ALL_CFLAGS) $(CPPFLAGS) $< -o $@


# Compile: create assembler files from C source files. ARM/Thumb
## does not work - TODO - hints welcome
##$(COBJ) : %.s : %.c
##	$(CC) $(THUMB) -S $(ALL_CFLAGS) $< -o $@


# Assemble: create object files from assembler source files. ARM/Thumb
$(AOBJ) : %.o : %.S
	@echo
	@echo $(MSG_ASSEMBLING) $<
	$(CC) -c $(THUMB) $(ALL_ASFLAGS) $< -o $@


# Assemble: create object files from assembler source files. ARM-only
$(AOBJARM) : %.o : %.S
	@echo
	@echo $(MSG_ASSEMBLING_ARM) $<
	$(CC) -c $(ALL_ASFLAGS) $< -o $@


# Target: clean project.
clean:
	$(MAKE) begin
	$(MAKE) clean_list
	$(MAKE) finished
	$(MAKE) end

clean_list :
	@echo
	@echo $(MSG_CLEANING)
	$(REMOVE) $(TARGET).hex
	$(REMOVE) *.bin
	$(REMOVE) $(TARGET).obj
	$(REMOVE) $(TARGET).elf
	$(REMOVE) $(TARGET).map
	$(REMOVE) $(TARGET).obj
	$(REMOVE) $(TARGET).a90
	$(REMOVE) $(TARGET).sym
	$(REMOVE) $(TARGET).lnk
	$(REMOVE) $(TARGET).lss
	$(REMOVE) $(COBJ)
	$(REMOVE) $(CPPOBJ)
	$(REMOVE) $(AOBJ)
	$(REMOVE) $(COBJARM)
	$(REMOVE) $(CPPOBJARM)
	$(REMOVE) $(AOBJARM)
	$(REMOVE) $(LST)
	$(REMOVE) $(SRC:.c=.s)
	$(REMOVE) $(SRC:.c=.d)
	$(REMOVE) $(SRCARM:.c=.s)
	$(REMOVE) $(SRCARM:.c=.d)
	$(REMOVE) $(CPPSRC:.cpp=.s)
	$(REMOVE) $(CPPSRC:.cpp=.d)
	$(REMOVE) $(CPPSRCARM:.cpp=.s)
	$(REMOVE) $(CPPSRCARM:.cpp=.d)
	$(REMOVE) .dep/*

clean_all: clean
	$(REMOVE) cscope.* TAGS

# Include the dependency files.
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

release :
	$(MAKE) clean
	$(MAKE) all
	rm -rf $(RELEASE_NAME_ZIP)
	cp main.elf $(RELEASE_NAME_ELF)
	cp $(RELEASE_NAME_ELF) $(RCP_RELEASE_DIR)
	rm -rf $(RCP_INSTALL_DIR)
	mkdir $(RCP_INSTALL_DIR)
	cp $(SAM7s_BASE_DIR)/installer/* $(RCP_INSTALL_DIR)
	cp $(RELEASE_NAME_ELF) $(RCP_INSTALL_DIR)
	cp CHANGELOG $(RCP_INSTALL_DIR)
	cp $(SAM7s_BASE_DIR)/HOW_TO_FLASH.txt $(RCP_INSTALL_DIR)
	zip -r $(RELEASE_NAME_ZIP) $(RCP_INSTALL_DIR)
	cp $(RELEASE_NAME_ZIP) $(RCP_RELEASE_DIR)

# **********************************************************************************************
#                            FLASH PROGRAMMING      (using OpenOCD 0.5.0 and Olimex ARM-USB-OCD)
# **********************************************************************************************
# inspect for later->	@cmd /c 'echo mww 0xfffffd08 0xa5000401 >> $(OPENOCD_SCRIPT)'

# program the AT91SAM7S256 internal flash memory
program: $(PROGRAM_TARGET)
	openocd -f flash.cfg

TAGS:
	@find . -type f -name '*\.[ch]' | etags -

clean_build:
	$(MAKE) clean
	$(MAKE) all

# Listing of phony targets.
.PHONY : all begin finish end sizebefore sizeafter gccversion \
build elf hex bin lss sym clean clean_list program TAGS clean_build
