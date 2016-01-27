#include stm32_base/Makefile

# Introduce code to allow me to be verbose later
Q := @

PHONY :=

PHONY += clean
clean:
	$(Q)find . -type f \
	-name "*.d"   -o \
	-name "*.lst" -o \
	-name "*.o"      \
	| xargs rm -f

PHONY += TAGS
TAGS:
	$(Q)find . -type f -regex '.*\.\(c\|cpp\|h\|hh\)$$' | etags -

.PHONY: $(PHONY)
