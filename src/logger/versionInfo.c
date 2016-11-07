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

#include "printk.h"
#include "versionInfo.h"
#include <stddef.h>

static void print_version(const VersionInfo *vi)
{
        pr_info_int(vi->major);
        pr_info_char('.');
        pr_info_int(vi->minor);
        pr_info_char('.');
        pr_info_int(vi->bugfix);
}

const VersionInfo* get_current_version_info() {
        static const VersionInfo vi = {
                .major = MAJOR_REV,
                .minor = MINOR_REV,
                .bugfix = BUGFIX_REV,
        };

        return &vi;
}

bool version_check_changed(const VersionInfo *pv,
                           const char* log_pfx)
{
        const VersionInfo* cv = get_current_version_info();
        const bool changed = cv->major != pv->major ||
                cv->minor != pv->minor;

        if (changed) {
                if (log_pfx) {
                        pr_info(log_pfx);
                        pr_info(" version changed: ");
                } else {
                        pr_info("Version changed: ");
                }

                print_version(pv);
                pr_info(" -> ");
                print_version(cv);
                pr_info("\r\n");
        }

        return changed;
}

/**
 * @return The full version string.  This is effectively the output of
 * `git describe --dirty` on the HEAD of the git tree when this release
 * was built.  This human readable string provides info on where the code
 * has come from along with # of committs and whether or not the tree was
 * dirty at the time of build.
 */
const char* version_full()
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
