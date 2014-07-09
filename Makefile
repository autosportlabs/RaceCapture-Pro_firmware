ifeq ($(PLAT), SAM7s)
	include SAM7s_base/Makefile
endif

ifeq ($(PLAT), stm32)
	include stm32_base/Makefile
endif
 