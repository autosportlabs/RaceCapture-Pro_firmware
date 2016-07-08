/*
 * Race Capture Pro Firmware
 *
 * Copyright (C) 2015 Autosport Labs
 *
 * This file is part of the Race Capture Pro fimrware suite
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


#include "luaTask.h"
#include "mem_mang.h"
#include "memory.h"
#include <string.h>
#include "printk.h"
#include "tracks.h"

#ifndef RCP_TESTING
#include "memory.h"
static const volatile Tracks g_tracks __attribute__((section(".tracks\n\t#")));
#else
static Tracks g_tracks = {};
#endif

void initialize_tracks()
{
        const VersionInfo vi = g_tracks.versionInfo;
        if (version_check_changed(&vi, "Tracks DB"))
                flash_default_tracks();
}

int flash_default_tracks(void)
{
        Tracks* def_tracks = calloc(1, sizeof(Tracks));
        const VersionInfo* cv = get_current_version_info();
        memcpy(&def_tracks->versionInfo, cv, sizeof(VersionInfo));

        pr_info("flashing default tracks...");
        const int status = flash_tracks(def_tracks, sizeof(Tracks));

        free(def_tracks);
        return status;
}

int flash_tracks(const Tracks *source, size_t rawSize)
{
    int result = memory_flash_region((void *)&g_tracks, (void *)source, rawSize);
    if (result == 0) pr_info("win\r\n");
    else pr_info("fail\r\n");
    return result;
}

const Tracks * get_tracks()
{
    return (Tracks *)&g_tracks;
}


enum track_add_result add_track(const Track *track, const size_t index,
                                const enum track_add_mode mode)
{
        if (index >= MAX_TRACK_COUNT) {
                pr_error("tracks: Invalid track index\r\n");
                return TRACK_ADD_RESULT_FAIL;
        }

        switch (mode) {
        case TRACK_ADD_MODE_IN_PROGRESS:
        case TRACK_ADD_MODE_COMPLETE:
                /* Valid cases.  Carry on */
                break;
        default:
                pr_error_int_msg("tracks: Unknown track_add_mode: ", mode);
                return TRACK_ADD_RESULT_FAIL;
        }

        static Tracks *g_tracksBuffer;
        if (NULL == g_tracksBuffer) {

#if LUA_SUPPORT
                lua_task_stop();
#endif /* LUA_SUPPORT */

                pr_debug("tracks: Allocating new tracks buffer\r\n");
                g_tracksBuffer = (Tracks *) portMalloc(sizeof(Tracks));
                memcpy(g_tracksBuffer, (void*) &g_tracks, sizeof(Tracks));
        }

        if (NULL == g_tracksBuffer) {
                pr_error("tracks: Failed to allocate memory for track buffer.\r\n");
                return TRACK_ADD_RESULT_FAIL;
        }

        Track *trackToAdd = g_tracksBuffer->tracks + index;
        memcpy(trackToAdd, track, sizeof(Track));
        g_tracksBuffer->count = index + 1;

        /* If we made it here and are still in progress, then we are done */
        if (TRACK_ADD_MODE_IN_PROGRESS == mode)
                return TRACK_ADD_RESULT_OK;

        /* If here, time to flash and tidy up */
        pr_info("tracks: Completed updating tracks. Flashing... ");
        const int rc = flash_tracks(g_tracksBuffer, sizeof(Tracks));
        portFree(g_tracksBuffer);
        g_tracksBuffer = NULL;

        if (0 != rc) {
                pr_info_int_msg("failed with code ", rc);
                return TRACK_ADD_RESULT_FAIL;
        }

        pr_info("win!\r\n");

#if LUA_SUPPORT
        lua_task_start();
#endif /* LUA_SUPPORT */

        return TRACK_ADD_RESULT_OK;
}

static int isStage(const Track *t)
{
    return t->track_type == TRACK_TYPE_STAGE;
}

GeoPoint getFinishPoint(const Track *t)
{
    return isStage(t) ? t->stage.finish : t->circuit.startFinish;
}

int isFinishPointValid(const Track *t)
{
    if (NULL == t)
        return 0;

    const GeoPoint p = getFinishPoint(t);
    return isValidPoint(&p);
}

GeoPoint getStartPoint(const Track *t)
{
    return isStage(t) ? t->stage.start : t->circuit.startFinish;
}

int isStartPointValid(const Track *t)
{
    if (NULL == t)
        return 0;

    const GeoPoint p = getStartPoint(t);
    return isValidPoint(&p);
}

GeoPoint getSectorGeoPointAtIndex(const Track *t, int index)
{
    if (index < 0) index = 0;
    const int max = isStage(t) ? STAGE_SECTOR_COUNT : CIRCUIT_SECTOR_COUNT;
    const GeoPoint *sectors = isStage(t) ? t->stage.sectors : t->circuit.sectors;

    if (index < max && isValidPoint(sectors + index))
        return sectors[index];

    // If here, return the finish since that is logically the next sector point.
    return getFinishPoint(t);
}

int areGeoPointsEqual(const GeoPoint a, const GeoPoint b)
{
    return a.latitude == b.latitude && a.longitude == b.longitude;
}
