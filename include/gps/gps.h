/**
 * Race Capture Firmware
 *
 * Copyright (C) 2014 Autosport Labs
 *
 * This file is part of the Race Capture firmware suite
 *
 * This is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should have received a copy of the GNU
 * General Public License along with this code. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Stieg
 */

#ifndef GPS_H_
#define GPS_H_

#include "LED.h"
#include "geopoint.h"
#include "dateTime.h"
#include "serial.h"

enum GpsSignalQuality {
    GPS_QUALITY_NO_FIX = 0,
    GPS_QUALITY_2D = 1,
    GPS_QUALITY_3D = 2,
    GPS_QUALITY_3D_DGNSS = 3
};

typedef struct _GpsSample {
    enum GpsSignalQuality quality;
    GeoPoint point;
    millis_t time;
    float speed;
    float altitude;
    uint8_t satellites;
    uint8_t fixMode;
    float DOP;
} GpsSample;

typedef struct _GpsSnapshot {
    GpsSample sample;
    tiny_millis_t deltaFirstFix;
    GeoPoint previousPoint;
} GpsSnapshot;

typedef enum {
    GPS_STATUS_NOT_INIT = 0,
    GPS_STATUS_PROVISIONED,
    GPS_STATUS_ERROR
} gps_status_t;

gps_status_t GPS_getStatus();

gps_status_t GPS_init(uint8_t targetSampleRate, Serial *serial);

float getSecondsSinceMidnight();

void updateSecondsSinceMidnight(float secondsSinceMidnight);

float getTimeDiff(float from, float to);

float getTimeSince(float t1);

float GPS_getLatitude();

float GPS_getLongitude();

float getAltitude();

bool isGpsDataCold();

bool isGpsSignalUsable(enum GpsSignalQuality q);

int GPS_getQuality();

float GPS_getDOP();

int GPS_getSatellitesUsedForPosition();

float getGPSSpeed();

/**
 * Returns Date time information as provided by the GPS system.
 */
millis_t getLastFix();

/**
 * @return Milliseconds since Unix Epoch.  0 indicates not available.
 */
millis_t getMillisSinceEpoch();

long long getMillisSinceEpochAsLongLong();

/**
 * This exists for backwards compatibility and will be deprecated in the future.  Use
 * #getLastFixDateTime() or #getMillisSinceEpoch() instead if possible.
 * @return the seconds since the first GPS fix.
 */
float getSecondsSinceFirstFix();

/**
 * @return The current known location.
 */
GeoPoint getGeoPoint();

/**
 * @return The previous known location.
 */
GeoPoint getPreviousGeoPoint();

/**
 * @return the current GPS Sample
 */
GpsSample getGpsSample();

/**
 * @return the current GPS Snapshot
 */
GpsSnapshot getGpsSnapshot();

/**
 * @return Milliseconds since our first fix.
 */
tiny_millis_t getMillisSinceFirstFix();

/**
 * @return The uptime when our most recent GPS sample was taken.  0 indicates no sample.
 */
tiny_millis_t getUptimeAtSample();

float getGpsSpeedInMph();

int GPS_processUpdate(Serial *serial);

int checksumValid(const char *gpsData, size_t len);

#endif /*GPS_H_*/
