# FreeRTOS configuration file
#
# Jeff Ciesielski <jeff.ciesielski@gmail.com>

ifneq ($(FREERTOS),)

FREERTOS_SOURCE ?= libs/$(FREERTOS)/FreeRTOS/Source
rtos_srcs = croutine.c list.c queue.c tasks.c timers.c

ifeq ($(CPU_ARCH),CM0)
FREERTOS_PORT = ARM_CM0
endif
ifeq ($(CPU_ARCH),ARMCM3)
FREERTOS_PORT = ARM_CM3
endif
ifeq ($(CPU_ARCH),ARMCM4)
FREERTOS_PORT = ARM_CM4F
endif

ifeq ($(FREERTOS_PORT),)
$(error CPU_ARCH not defined. This is required to build the FreeRTOS Port)
endif

RTOS_PORT_SRC = $(FREERTOS_SOURCE)/portable/GCC/$(FREERTOS_PORT)/port.c

#If no heap implementation has been defined, choose heap 1
# Note that this implementation does not allow free
ifeq ($(FREERTOS_HEAP),)
$(warning No heap selected, defaulting to heap 1 (Memory cannot be freed!!))
FREERTOS_HEAP = heap_1
endif

SRCS += utils/sbrk.c

FREERTOS_SRCS += $(FREERTOS_SOURCE)/portable/MemMang/$(FREERTOS_HEAP).c $(RTOS_PORT_SRC)

#All of the standard sources
FREERTOS_SRCS += $(addprefix $(FREERTOS_SOURCE)/,$(rtos_srcs))
FREERTOS_OBJS = $(FREERTOS_SRCS:.c=.o)

#FreeRTOS Includes
LIB_INCLUDES += -I$(FREERTOS_SOURCE)/include \
	-I$(FREERTOS_SOURCE)/portable/GCC/$(FREERTOS_PORT)

BASE_LIBS += freertos

endif
