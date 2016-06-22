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

ASL_CFLAGS :=

ASL_C_GDB ?=
export ASL_C_GDB
ASL_CFLAGS += $(ASL_C_GDB)

ASL_C_OPT ?= -Os
export ASL_C_OPT
ASL_CFLAGS += $(ASL_C_OPT)

ASL_CSTD := -std=gnu99
ASL_CFLAGS += $(ASL_CSTD)


ASL_WARNING_FLAGS := \
-Wall \
-Werror \
-Wempty-body \
-Wclobbered \
-Wignored-qualifiers \
-Wmissing-parameter-type \
-Wold-style-declaration \
-Wunused-but-set-parameter \

ASL_CFLAGS += $(ASL_WARNING_FLAGS)


ASL_WATCHDOG := false
ifeq ($(RELEASE_TYPE),RELEASE_TYPE_OFFICIAL)
ASL_WATCHDOG := true
endif

ASL_CFLAGS += -DASL_WATCHDOG=$(ASL_WATCHDOG)


ASL_CFLAGS_MISC := \
-fno-common \
-fno-strict-aliasing \

ASL_CFLAGS += $(ASL_CFLAGS_MISC)

#
# Only specify ASL_DEBUG on devel builds. Reason being that when
# we release RC builds to the field, we want them to be as close
# to the official build as possible without being labeled as official.
# That means no checks that would occur when DEBUG is active.
#
ifeq ($(RELEASE_TYPE),RELEASE_TYPE_DEVEL)
ASL_CFLAGS += -DASL_DEBUG
endif


export VERSION_CFLAGS := \
-DAPI_REV=$(API) \
-DMAJOR_REV=$(MAJOR) \
-DMINOR_REV=$(MINOR) \
-DBUGFIX_REV=$(BUGFIX) \
-DRC_BUILD_GIT_DESCRIPTION=$(GIT_DESCRIPTION) \
-DRC_BUILD_RELEASE_TYPE=$(RELEASE_TYPE) \

ASL_CFLAGS += $(VERSION_CFLAGS)


export ASL_CFLAGS
