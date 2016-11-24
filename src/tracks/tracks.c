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
#include "memory_device.h"
#include "printk.h"
#include "tracks.h"
#include <string.h>

#define LOG_PFX	"[Tracks] "

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

static bool clear_tracks_region()
{
	return memory_device_region_clear(&g_tracks);
}

static bool write_version_info()
{
	const VersionInfo* vi = get_current_version_info();
	const size_t vi_size = sizeof(VersionInfo);
	const volatile void* dst = &g_tracks.versionInfo;
	return vi_size == memory_device_write_words(dst, vi, vi_size);
}

static bool write_track_count(const size_t count)
{
	const size_t size = sizeof(size_t);
	const volatile void* dst = &g_tracks.count;
	return size == memory_device_write_words(dst, &count, size);
}

static bool write_track(const Track *track, const size_t idx)
{
	const size_t size = sizeof(Track);
	const volatile void* dst = g_tracks.tracks + idx;
	return size == memory_device_write_words(dst, track, size);
}

int flash_default_tracks(void)
{
	return clear_tracks_region() &&
		write_version_info() &&
		write_track_count(0) ? 0 : 1;
}

const Tracks* get_tracks()
{
	return (Tracks*) &g_tracks;
}

enum track_add_result add_track(const Track *track, const size_t index,
                                const enum track_add_mode mode)
{
        switch (mode) {
        case TRACK_ADD_MODE_IN_PROGRESS:
        case TRACK_ADD_MODE_COMPLETE:
                /* Valid cases.  Carry on */
                break;
        default:
                pr_error_int_msg(LOG_PFX "Unknown track_add_mode: ", mode);
                return TRACK_ADD_RESULT_FAIL;
        }

	bool status = true;
	static size_t idx = 0;
	if (0 == idx) {
		/* Then start the add process */
		status &= clear_tracks_region();
		status &= write_version_info();
	}

	status &= write_track(track, idx);
	++idx;

        /* If we made it here and are still in progress, then we are done */
        if (TRACK_ADD_MODE_COMPLETE == mode) {
		/* If here, time to flash and tidy up */
		status &= write_track_count(idx);
		pr_info(LOG_PFX "Completed updating tracks.\r\n");
		idx = 0;
	}

        return status ? TRACK_ADD_RESULT_OK : TRACK_ADD_RESULT_FAIL;
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
	const int max = isStage(t) ?
		STAGE_SECTOR_COUNT : CIRCUIT_SECTOR_COUNT;
	const GeoPoint *sectors = isStage(t) ?
		t->stage.sectors : t->circuit.sectors;

	if (index < max && isValidPoint(sectors + index))
		return sectors[index];

	/* If here, return the finish since logically next sector point */
	return getFinishPoint(t);
}

int areGeoPointsEqual(const GeoPoint a, const GeoPoint b)
{
	return a.latitude == b.latitude && a.longitude == b.longitude;
}
