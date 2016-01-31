# Leave this commented until we have a proper
# top-level makefile working.
#include stm32_base/Makefile

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
