# Leave this commented until we have a proper
# top-level makefile working.
#include stm32_base/Makefile

include version.mk

Q := @
PHONY :=

# Default target is all.
all: package

#
# Testing bits.
#
TEST_DIR := test
PHONY += test-run
test-run: test-build
	$(MAKE) -C $(TEST_DIR) test-run

PHONY += test-build
test-build:
	$(MAKE) -C $(TEST_DIR) all

PHONY += test-clean
test-clean:
	$(MAKE) -C $(TEST_DIR) clean

PHONY += test-pristine
test-pristine: test-clean
	$(MAKE) test-run

PHONY += test
test: test-run


#
# Lua Bits
#
LUA_DIR := lib_lua
PHONY += lua-build
lua-build:
	$(MAKE) -C $(LUA_DIR) PLAT=stm32 generic

PHONY += lua-clean
lua-clean:
	$(MAKE) -C $(LUA_DIR) PLAT=stm32 clean

PHONY += lua-pristine
lua-pristine: lua-clean
	$(MAKE) lua-build


#
# MK2
#
MK2_DIR := stm32_base
PHONY += mk2-build
mk2-build: lua-build
	$(MAKE) -C $(MK2_DIR) all

PHONY += mk2-clean
mk2-clean:
	$(MAKE) -C $(MK2_DIR) clean

PHONY += mk2-pristine
mk2-pristine: lua-pristine mk2-clean
	$(MAKE) mk2-build

PHONY += mk2-package
mk2-package: mk2-pristine
	./bin/package_release.sh MK2 $(VERSION_STR) $(MK2_DIR)


#
# RCT
#
RCT_DIR := RCT
PHONY += rct-build
rct-build: lua-build
	$(MAKE) -C $(RCT_DIR) all

PHONY += rct-clean
rct-clean:
	$(MAKE) -C $(RCT_DIR) clean

PHONY += rct-pristine
rct-pristine: lua-pristine rct-clean
	$(MAKE) rct-build

PHONY += rct-package
rct-package: rct-pristine
# NO-OP just yet.
#	./bin/package_release.sh RCT $(VERSION_STR) $(RCT_DIR)


#
# Common targets.
#
PHONY += clean
clean:
	$(Q)find . -type f \
	-name "*.d"   -o \
	-name "*.lst" -o \
	-name "*.o"      \
	| xargs rm -f

package: test-pristine
	$(MAKE) mk2-package
	$(MAKE) rct-package

PHONY += TAGS
TAGS:
	$(Q)find . -type f -regex '.*\.\(c\|cpp\|h\|hh\)$$' | etags -

.PHONY: $(PHONY)
