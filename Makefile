include stm32_base/Makefile

TAGS:
	@find . -type f -regex '.*\.\(c\|cpp\|h\|hh\)$' | etags -
