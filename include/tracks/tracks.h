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

#ifndef TRACKS_H_
#define TRACKS_H_

#include "capabilities.h"
#include "geopoint.h"
#include "stddef.h"
#include "versionInfo.h"
#include <stdint.h>

enum track_add_result {
        TRACK_ADD_RESULT_FAIL = 0,
        TRACK_ADD_RESULT_OK = 1,
};

enum track_add_mode {
        TRACK_ADD_MODE_IN_PROGRESS = 1,
        TRACK_ADD_MODE_COMPLETE = 2,
};

#define MAX_TRACK_COUNT				40
#define SECTOR_COUNT				MAX_SECTORS
#define CIRCUIT_SECTOR_COUNT		SECTOR_COUNT - 1
#define STAGE_SECTOR_COUNT			SECTOR_COUNT - 2

#define DEFAULT_TRACK_TARGET_RADIUS	0.0001

#define DEFAULT_TRACKS \
{ \
    DEFAULT_VERSION_INFO, \
    0, \
    {} \
}

enum TrackType {
    TRACK_TYPE_CIRCUIT = 0,
    TRACK_TYPE_STAGE = 1,
};

typedef struct _Circuit {
    GeoPoint startFinish;
    GeoPoint sectors[CIRCUIT_SECTOR_COUNT];
} Circuit;

typedef struct _Stage {
    GeoPoint start;
    GeoPoint finish;
    GeoPoint sectors[STAGE_SECTOR_COUNT];
} Stage;

typedef struct _Track {
    int32_t trackId;
    enum TrackType track_type;
    union {
        GeoPoint allSectors[SECTOR_COUNT]; // Needed for Loading in data.
        Stage stage;
        Circuit circuit;
    };
} Track;

typedef struct _Tracks {
    VersionInfo versionInfo;
    size_t count;
    Track tracks[MAX_TRACK_COUNT];
} Tracks;

void initialize_tracks();
int flash_tracks(const Tracks *source, size_t rawSize);
enum track_add_result add_track(const Track *track, const size_t index,
                                enum track_add_mode mode);
int flash_default_tracks(void);
const Tracks * get_tracks();

/**
 * Returns the finish point of the track, regardless if its a stage or a circuit.
 * @return The GeoPoint representing the finish line.
 */
GeoPoint getFinishPoint(const Track *t);

/**
 * Tells the caller if the finish line of the given track is valid.
 * @return true if its a real value, false otherwise.
 */
int isFinishPointValid(const Track *t);

/**
 * Returns the start point of the track, regardless if its a stage or a circuit.
 * @return The GeoPoint representing the finish line.
 */
GeoPoint getStartPoint(const Track *t);

/**
 * Tells the caller if the start point of the given track is valid.
 * @return true if its a real value, false otherwise.
 */
int isStartPointValid(const Track *t);

/**
 * @return The GeoPoint of the next sectory boundary at the given index.
 *         This may be the Finish line.
 */
GeoPoint getSectorGeoPointAtIndex(const Track *t, const int index);

int areGeoPointsEqual(const GeoPoint a, const GeoPoint b);

#endif /* TRACKS_H_ */
