#STM32 USB Library makefile
#
#Jeff Ciesielski <jeff.ciesielski@gmail.com>

ifneq ($(STM32_USB_LIBS),)

ST_USB_BASE = libs/STM32_USB-Host-Device_Lib_V2.1.0/Libraries

ifneq ($(STM32_USB_DEV),)
#Device Library
ST_USB_DEV_BASE = $(ST_USB_BASE)/STM32_USB_Device_Library

#Core libraries
ST_USB_SRCS += $(shell find $(ST_USB_DEV_BASE)/Core/src -name '*.c')
LIB_INCLUDES += -I$(ST_USB_DEV_BASE)/Core/inc/
LIB_INCLUDES += -I$(ST_USB_OTG_BASE)/inc

ST_USB_OTG_BASE = $(ST_USB_BASE)/STM32_USB_OTG_Driver

#Class libraries
ST_USB_DEV_CLASS_BASE = $(ST_USB_DEV_BASE)/Class

  ifneq ($(STM32_USB_DEV_AUDIO),)
    #Audio
    ST_USB_SRCS += $(shell find $(ST_USB_DEV_CLASS_BASE)/audio/src -name '*[^template].c')
    LIB_INCLUDES += -I$(ST_USB_DEV_CLASS_BASE)/audio/inc/
  endif
  ifneq ($(STM32_USB_DEV_CDC),)
    #CDC
    ST_CDC_SRCS += usbd_cdc_core.c
    ST_USB_SRCS += $(addprefix $(ST_USB_DEV_CLASS_BASE)/cdc/src/, $(ST_CDC_SRCS))
    LIB_INCLUDES += -I$(ST_USB_DEV_CLASS_BASE)/cdc/inc/
  endif
  ifneq ($(STM32_USB_DEV_DFU),)
    #DFU
    ST_USB_SRCS += $(shell find $(ST_USB_DEV_CLASS_BASE)/dfu/src -name '*[^template].c')
    LIB_INCLUDES += -I$(ST_USB_DEV_CLASS_BASE)/dfu/inc/
  endif
  ifneq ($(STM32_USB_DEV_HID),)
    #HID
    ST_USB_SRCS += $(shell find $(ST_USB_DEV_CLASS_BASE)/hid/src -name '*[^template].c')
    LIB_INCLUDES += -I$(ST_USB_DEV_CLASS_BASE)/hid/inc/
  endif
  ifneq ($(STM32_USB_DEV_MSC),)
    #MSC
    ST_USB_SRCS += $(shell find $(ST_USB_DEV_CLASS_BASE)/msc/src -name '*[^template].c')
    LIB_INCLUDES += -I$(ST_USB_DEV_CLASS_BASE)/msc/inc/
  endif

endif #USB_DEV

ifneq ($(STM32_USB_HOST),)
#Host Library
ST_USB_HOST_BASE = $(ST_USB_BASE)/STM32_USB_HOST_Library

#Core libraries
ST_USB_SRCS += $(shell find $(ST_USB_HOST_BASE)/Core/src -name '*.c')
LIB_INCLUDES += -I$(ST_USB_HOST_BASE)/Core/inc/

#Class Libraries
ST_USB_HOST_CLASS_BASE = $(ST_USB_HOST_BASE)/Class

#HID
ST_USB_SRCS += $(shell find $(ST_USB_HOST_CLASS_BASE)/HID/src -name '*.c')
LIB_INCLUDES += -I$(ST_USB_HOST_CLASS_BASE)/HID/inc/

#MSC
ST_USB_SRCS += $(shell find $(ST_USB_HOST_CLASS_BASE)/MSC/src -name '*.c')
LIB_INCLUDES += -I$(ST_USB_HOST_CLASS_BASE)/MSC/inc/

endif #USB_HOST

OTG_SRCS += usb_core.c

#Host and device have different usb otg source files
ifeq ($(STM32_USB_DEV),1)
 OTG_SRCS += usb_dcd.c usb_dcd_int.c
endif

ifeq ($(STM32_USB_HOST),1)
 OTG_SRCS += usb_hcd.c usb_hcd_int.c
endif

ST_USB_SRCS += $(addprefix $(ST_USB_OTG_BASE)/src/, $(OTG_SRCS))
LIB_INCLUDES += -I$(ST_USB_OTG_BASE)/inc

STM32_USB_OBJS = $(sort $(ST_USB_SRCS:.c=.o))
BASE_LIBS += stm32_usb

endif #STM32_USB_LIBS
