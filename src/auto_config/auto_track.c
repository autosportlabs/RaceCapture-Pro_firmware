/*
 * Race Capture Firmware
 *
 * Copyright (C) 2015 Autosport Labs
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

#include "auto_track.h"
#include "geopoint.h"
#include "loggerConfig.h"
#include "printk.h"
#include "tracks.h"

static const Track* findClosestTrack(const Tracks *tracks, const GeoPoint *location)
{
    float dist = MAX_DIST_FROM_SF;
    const Track *best = NULL;

    for (unsigned i = 0; i < tracks->count; ++i) {
        const Track *track = &(tracks->tracks[i]);

        // XXX: inaccurate but fast.  Good enough for now.
        GeoPoint startPoint = getStartPoint(track);
        float track_distance = distPythag(&startPoint, location);

        if (track_distance >= dist)
            continue;

        // If here then we have a new best.  Set it accordingly
        dist = track_distance;
        best = track;
    }

    return best;
}

const Track* auto_configure_track(const Track *defaultCfg, const GeoPoint *gp)
{
    const Tracks *tracks = get_tracks();
    if (!tracks || tracks->count <= 0) {
        // Well shit!
        return defaultCfg;
    }

    const Track *foundTrack = findClosestTrack(tracks, gp);
    if (!foundTrack) {
        foundTrack = defaultCfg;
    } else {
        pr_info_int_msg("tracks: found ", foundTrack->trackId);
    }
    return foundTrack;
}
