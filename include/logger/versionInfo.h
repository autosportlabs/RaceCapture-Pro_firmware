/*
 * Race Capture Firmware
 *
 * Copyright (C) 2016 Autosport Labs
 *
 * This file is part of the Race Capture firmware suite
 *
 * This is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should
 * have received a copy of the GNU General Public License along with
 * this code. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef VERSIONINFO_H_
#define VERSIONINFO_H_

#include "cpp_guard.h"

#include <stdint.h>
#include <stdbool.h>

CPP_GUARD_BEGIN

enum release_type {
        RELEASE_TYPE_BETA,
        RELEASE_TYPE_DEVEL,
        RELEASE_TYPE_OFFICIAL,
};

typedef struct _VersionInfo {
        uint32_t major;
        uint32_t minor;
        uint32_t bugfix;
} VersionInfo;

bool version_check_changed(const VersionInfo *versionInfo,
                           const char* log_pfx);
const VersionInfo* get_current_version_info();
const char* version_full();
enum release_type version_get_release_type();
const char* version_release_type_api_key(const enum release_type rt);

CPP_GUARD_END

#endif /*VERSIONINFO_H_*/
