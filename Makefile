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
RELEASE_DIR ?= .
RELEASE_NAME = RaceCapturePro-$(MAJOR).$(MINOR).$(BUGFIX).bin

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
SAM7s_BASE_DIR = SAM7s_base
HW_DIR = $(SAM7s_BASE_DIR)/hardware
RTOS_DIR = $(SAM7s_BASE_DIR)/FreeRTOS
RTOS_SRC_DIR = $(RTOS_DIR)/Source
RTOS_PORT_DIR = $(RTOS_SRC_DIR)/portable
RTOS_GCC_DIR = $(RTOS_PORT_DIR)/GCC/ARM7_AT91SAM7S
RTOS_MEMMANG_DIR = $(RTOS_PORT_DIR)/MemMang
USB_SRC_DIR = $(SAM7s_BASE_DIR)/usb
SERIAL_SRC_DIR = $(SAM7s_BASE_DIR)/serial
UTIL_DIR = $(SAM7s_BASE_DIR)/util
MEMORY_SRC_DIR = $(SAM7s_BASE_DIR)/memory
LUA_SRC_DIR = $(SAM7s_BASE_DIR)/lua
CMD_SRC_DIR = $(SAM7s_BASE_DIR)/command

#App specific dirs
FAT_SD_SRC_DIR = fat_sd
LOGGER_SRC_DIR = logger
SDCARD_SRC_DIR = sdcard
ACCELEROMETER_SRC_DIR = accelerometer
TWEETER_DIR = tweeter

# List C source files here. (C dependencies are automatically generated.)
# use file-extension c for "c-only"-files
SRC	= \
$(TARGET).c \
baseCommands.c \
$(HW_DIR)/lib_AT91SAM7S256.c \
$(RTOS_SRC_DIR)/tasks.c \
$(RTOS_SRC_DIR)/queue.c \
$(RTOS_SRC_DIR)/list.c \
$(RTOS_GCC_DIR)/port.c \
$(UTIL_DIR)/modp_numtoa.c \
$(UTIL_DIR)/modp_atonum.c \
$(UTIL_DIR)/taskUtil.c \
$(USB_SRC_DIR)/source/usb_comm.c \
$(USB_SRC_DIR)/source/USB-CDC.c \
$(SERIAL_SRC_DIR)/usart.c \
$(FAT_SD_SRC_DIR)/fattime.c \
$(FAT_SD_SRC_DIR)/ff.c \
$(FAT_SD_SRC_DIR)/sd_spi_at91.c \
$(FAT_SD_SRC_DIR)/rtc.c \
$(FAT_SD_SRC_DIR)/option/ccsbcs.c \
$(SDCARD_SRC_DIR)/sdcard.c \
$(ACCELEROMETER_SRC_DIR)/accelerometer.c \
$(LOGGER_SRC_DIR)/sampleRecord.c \
$(LOGGER_SRC_DIR)/fileWriter.c \
$(LOGGER_SRC_DIR)/loggerHardware.c \
$(LOGGER_SRC_DIR)/loggerData.c \
$(LOGGER_SRC_DIR)/gpioTasks.c \
$(LOGGER_SRC_DIR)/loggerTask.c \
$(LOGGER_SRC_DIR)/loggerTaskEx.c \
$(LOGGER_SRC_DIR)/telemetryTask.c \
$(LOGGER_SRC_DIR)/p2pTelemetry.c \
$(LOGGER_SRC_DIR)/cellTelemetry.c \
$(LOGGER_SRC_DIR)/btTelemetry.c \
$(LOGGER_SRC_DIR)/gps.c \
$(LOGGER_SRC_DIR)/cellModem.c \
$(LOGGER_SRC_DIR)/loggerConfig.c \
$(LOGGER_SRC_DIR)/geometry.c \
$(LOGGER_SRC_DIR)/luaLoggerBinding.c \
$(LOGGER_SRC_DIR)/loggerCommands.c \
$(MEMORY_SRC_DIR)/memory.c \
$(LUA_SRC_DIR)/luaTask.c \
$(LUA_SRC_DIR)/luaScript.c \
$(LUA_SRC_DIR)/luaBaseBinding.c \
$(LUA_SRC_DIR)/luaCommands.c \
$(RTOS_PORT_DIR)/MemMang/heap_2_combine.c \
$(TWEETER_DIR)/raceTask.c 

# List C source files here which must be compiled in ARM-Mode.
# use file-extension c for "c-only"-files
SRCARM = $(RTOS_GCC_DIR)/portISR.c \
interrupt_utils.c \
$(USB_SRC_DIR)/source/USBIsr.c \
$(LOGGER_SRC_DIR)/gpioIsr.c \
$(LOGGER_SRC_DIR)/serialIsr.c \
$(LOGGER_SRC_DIR)/timerIsr.c

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
ASRCARM = startup_SAM7S.S
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
EXTRAINCDIRS = SAM7s_lua/src

# List any extra directories to look for library files here.
#     Each directory must be seperated by a space.
EXTRA_LIBDIRS = SAM7s_lua/src

# Compiler flag to set the C Standard level.
# c89   - "ANSI" C
# gnu89 - c89 plus GCC extensions
# c99   - ISO C99 standard (not yet fully implemented)
# gnu99 - c99 plus GCC extensions
CSTANDARD = -std=gnu99

# Place -D or -U options for C here
CDEFS  = -D$(RUN_MODE) -DMAJOR_REV=\"$(MAJOR)\" -DMINOR_REV=\"$(MINOR)\" -DBUGFIX_REV=\"$(BUGFIX)\"

# Compile for SAM7 using GCC
CDEFS += -DSAM7_GCC

# If using THUMB mode, then add this for the assembly process
CDEFS += -DTHUMB_INTERWORK

# Place -I options here
CINCS = -I. -I$(RTOS_MEMMANG_DIR) -I$(UTIL_DIR) -I$(LUA_SRC_DIR) -I$(MEMORY_SRC_DIR) -I$(FAT_SD_SRC_DIR) -I$(SDCARD_SRC_DIR) -I$(SERIAL_SRC_DIR) -I$(ACCELEROMETER_SRC_DIR) -I$(LOGGER_SRC_DIR) -I$(USB_SRC_DIR)/include -I$(HW_DIR)/include -I$(RTOS_SRC_DIR)/include -I$(RTOS_GCC_DIR) -I$(TWEETER_DIR) -I$(CMD_SRC_DIR)
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
LDFLAGS +=-T$(SUBMDL)-ROM.ld




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
LST += $(CPPSRC:.cpp=.lst) $(CPPSRCARM:.cpp=.lst)

# Compiler flags to generate dependency files.
### GENDEPFLAGS = -Wp,-M,-MP,-MT,$(*F).o,-MF,.dep/$(@F).d
GENDEPFLAGS = -MD -MP -MF .dep/$(@F).d

# Combine all necessary flags and optional flags.
# Add target processor to flags.
ALL_CFLAGS = -mcpu=$(MCU) $(THUMB_IW) -I. $(CFLAGS) $(GENDEPFLAGS)
ALL_ASFLAGS = -mcpu=$(MCU) $(THUMB_IW) -I. -x assembler-with-cpp $(ASFLAGS)


# Default target.
all: begin gccversion sizebefore build sizeafter finished end

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
	$(OBJCOPY) -O $(FORMAT) $< $@
	
# Create final output file (.bin) from ELF output file.
%.bin: %.elf
	@echo
	@echo $(MSG_FLASH) $@
	$(OBJCOPY) -O $(FORMAT) $< $@


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
	$(NM) -n $< > $@


# Link: create ELF output file from object files.
.SECONDARY : $(TARGET).elf
.PRECIOUS : $(AOBJARM) $(AOBJ) $(COBJARM) $(COBJ) $(CPPOBJ) $(CPPOBJARM)
%.elf:  $(AOBJARM) $(AOBJ) $(COBJARM) $(COBJ) $(CPPOBJ) $(CPPOBJARM)
	@echo
	@echo $(MSG_LINKING) $@
	$(CC) $(THUMB) $(ALL_CFLAGS) $(AOBJARM) $(AOBJ) $(COBJARM) $(COBJ) $(CPPOBJ) $(CPPOBJARM) --output $@ $(LDFLAGS)
#	$(CPP) $(THUMB) $(ALL_CFLAGS) $(AOBJARM) $(AOBJ) $(COBJARM) $(COBJ) $(CPPOBJ) $(CPPOBJARM) --output $@ $(LDFLAGS)

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
	$(CPP) -c $(THUMB) $(ALL_CFLAGS) $(CPPFLAGS) $< -o $@ 

# Compile: create object files from C++ source files. ARM-only
$(CPPOBJARM) : %.o : %.cpp
	@echo
	@echo $(MSG_COMPILINGCPP_ARM) $<
	$(CPP) -c $(ALL_CFLAGS) $(CPPFLAGS) $< -o $@ 


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
clean: begin clean_list finished end


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


# Include the dependency files.
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)


# Listing of phony targets.
.PHONY : all begin finish end sizebefore sizeafter gccversion \
build elf hex bin lss sym clean clean_list program


release : clean all
	mv main.bin $(RELEASE_DIR)/$(RELEASE_NAME)
	cp RELEASE_NOTES.TXT $(RELEASE_DIR)
	

# **********************************************************************************************
#                            FLASH PROGRAMMING      (using OpenOCD 0.5.0 and Olimex ARM-USB-OCD)
# **********************************************************************************************
# inspect for later->	@cmd /c 'echo mww 0xfffffd08 0xa5000401 >> $(OPENOCD_SCRIPT)'

# program the AT91SAM7S256 internal flash memory
program: $(PROGRAM_TARGET)
	openocd -f flash.cfg
	