ifeq ($(PLAT), sam7s)
	include SAM7s_base/Makefile
endif

ifeq ($(PLAT), stm32)
	include stm32_base/Makefile
endif

TAGS:
	find . -type f -regex '.*\.\(c\|cpp\|h\|hh\)$$' | etags -

.PHONY: TAGS
