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

ASL_DEBUG := -ggdb
ASL_CFLAGS += $(ASL_DEBUG)


ASL_COPT := s
ASL_CFLAGS += -O$(ASL_COPT)


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


ifneq ($(RELEASE_TYPE),RELEASE_TYPE_OFFICIAL)
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
