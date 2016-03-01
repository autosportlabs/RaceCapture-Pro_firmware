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

#include "versionInfo.h"

#include <stddef.h>

bool versionChanged(const volatile VersionInfo *versionInfo)
{
        bool major_version_changed = versionInfo->major != MAJOR_REV;
        bool minor_version_changed = versionInfo->minor != MINOR_REV;
        bool changed = (major_version_changed || minor_version_changed);
        return changed;
}

/**
 * @return The output of `git describe --dirty` on the HEAD of the git
 * tree when this release was built.  This human readable string provides
 * info on where the code has come from along with # of committs and whether
 * or not the tree was dirty at the time of build.
 */
const char* version_git_description()
{
        /* The git hash value is defined by the build */
        return RC_BUILD_GIT_DESCRIPTION;
}

/**
 * @return An enum that represents the intention of the build.  This allows
 * us to more easily differentiate the intention of the build.  Of course
 * we could get this info from the SHA-1 git commit hash as well, but this
 * is more direct.
 */
enum release_type version_get_release_type()
{
        return RC_BUILD_RELEASE_TYPE;
}

/**
 * @return The API key that represents our release type in the API.
 */
const char* version_release_type_api_key(const enum release_type rt)
{
        switch(rt) {
        case RELEASE_TYPE_BETA:
                return "beta";
        case RELEASE_TYPE_DEVEL:
                return "development";
        case RELEASE_TYPE_OFFICIAL:
                return "official";
        default:
                return "unknown";
        }
}
