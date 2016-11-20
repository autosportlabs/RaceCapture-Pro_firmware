#
# Race Capture Firmware
#
# Copyright (C) 2016 Autosport Labs
#
# This file is part of the Race Capture firmware suite
#
# This is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# See the GNU General Public License for more details. You should
# have received a copy of the GNU General Public License along with
# this code. If not, see <http://www.gnu.org/licenses/>.
#

export MAJOR  := $(shell git describe | cut -d \- -f 1 | cut -d . -f 1)
export MINOR  := $(shell git describe | cut -d \- -f 1 | cut -d . -f 2)
export BUGFIX := $(shell git describe | cut -d \- -f 1 | cut -d . -f 3)
export API    := 1

export VERSION := $(shell git describe --dirty --always)

#
# Gets the sha-1 hash (hex encoded string) of our build)
#
export GIT_DESCRIPTION := "\"$(VERSION)\""

#
# Build types:
#
# * RELEASE_TYPE_DEVEL    - (Default) Build from a dev machine.
# * RELEASE_TYPE_OFFICIAL - Official build from our build server.
#                           Implies a tag is associated with this
#                           release.
# * RELEASE_TYPE_BETA     - A beta release.  One designed for user
#                           testing, but has no official support.
#
RELEASE_TYPE := RELEASE_TYPE_DEVEL
OFFICIAL_TAG := $(if $(shell ./bin/get_official_tag.sh), yes)
ifeq ($(RELEASE),BETA)
    RELEASE_TYPE := RELEASE_TYPE_BETA
endif
ifneq ($(OFFICIAL_TAG),)
    RELEASE_TYPE := RELEASE_TYPE_OFFICIAL
endif
export RELEASE_TYPE

# Now with basic version info, import all our sub makefiles
include make/*.mk

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
LUA_DIR := lib/lua
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
MK2_DIR := platform/mk2
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
	./bin/package_release.sh "RaceCapturePro_MK2" $(VERSION) $(MK2_DIR)

PHONY += mk2-flash
mk2-flash: mk2-build
	cd $(MK2_DIR) && openocd -f openocd_stlinkv2_flash.cfg

PHONY += mk2
mk2: mk2-build


#
# RCT
#
RCT_DIR := platform/rct
PHONY += rct-build
rct-build:
	$(MAKE) -C $(RCT_DIR) all

PHONY += rct-clean
rct-clean:
	$(MAKE) -C $(RCT_DIR) clean

PHONY += rct-flash
rct-flash: rct-build
	cd $(RCT_DIR) && openocd -f openocd_flash.cfg

PHONY += rct-pristine
rct-pristine: rct-clean
	$(MAKE) rct-build

PHONY += rct-package
rct-package: rct-pristine
	./bin/package_release.sh "RaceCapture" $(VERSION) $(RCT_DIR)

PHONY += rct
rct: rct-build


#
# Common targets.
#
PHONY += clean
clean: rct-clean mk2-clean test-clean lua-clean
	$(Q)find . -type f \
	-name "*.a"   -o   \
	-name "*.bin" -o   \
	-name "*.d"   -o   \
	-name "*.dis" -o   \
	-name "*.elf" -o   \
	-name "*.hex" -o   \
	-name "*.ihex" -o   \
	-name "*.lst" -o   \
	-name "*.map" -o   \
	-name "*.o"   -o   \
	-name "*.sym" -o   \
	-name "*.zip"      \
	| xargs rm -f

package: clean
	$(MAKE) test-pristine
	$(MAKE) mk2-package
	$(MAKE) rct-package
	$(MAKE) hashes

PHONY += TAGS
TAGS:
	$(Q)find         \
	include          \
	platform         \
	src              \
	test             \
	-type f          \
	-name "*.c"   -o \
	-name "*.cpp" -o \
	-name "*.h"   -o \
	-name "*.hh"     \
	| etags -

PHONY += hashes
hashes:
	$(Q)echo
	$(Q)echo "SHA256 Hashes for all main.* files: "
	$(Q)sha256sum -b $(sort $(wildcard platform/*/main.*)) </dev/null
	$(Q)echo

.PHONY: $(PHONY)
