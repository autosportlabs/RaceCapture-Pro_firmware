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

#include "gps.h"
#include "gps_device.h"
#include "mod_string.h"
#include "modp_atonum.h"

#define GPS_LOCK_FLASH_COUNT 5
#define GPS_NOFIX_FLASH_COUNT 50

static GpsSnapshot g_gpsSnapshot;
gps_status_t gps_status = GPS_STATUS_NOT_INIT;
static int g_flashCount;
static millis_t g_timeFirstFix;
static tiny_millis_t g_uptimeAtSample;

bool isGpsSignalUsable(enum GpsSignalQuality q)
{
    return q != GPS_QUALITY_NO_FIX;
}

gps_status_t GPS_init(uint8_t targetSampleRate, Serial *serial)
{
    memset(&g_gpsSnapshot, 0, sizeof(GpsSnapshot));
    g_timeFirstFix = 0;
    g_flashCount = 0;
    g_uptimeAtSample = 0;
    gps_status = GPS_device_init(targetSampleRate, serial);
    return gps_status;
}

gps_status_t GPS_getStatus()
{
    return gps_status;
}

static void flashGpsStatusLed(enum GpsSignalQuality gpsQuality)
{
    if (g_flashCount == 0) {
        LED_disable(1);
    }
    g_flashCount++;

    int targetFlashCount = isGpsSignalUsable(gpsQuality) ?
                           GPS_LOCK_FLASH_COUNT : GPS_NOFIX_FLASH_COUNT;

    if (g_flashCount >= targetFlashCount) {
        LED_enable(1);
        g_flashCount = 0;
    }
}

/**
 * @return true if we haven't parsed any data yet, false otherwise.
 */
bool isGpsDataCold()
{
    return g_timeFirstFix == 0;
}

static tiny_millis_t getDeltaSinceSample()
{
    return getUptime() - g_uptimeAtSample;
}

/**
 * Use this method add hoc when you don't have access to GpsSnapshot
 */
millis_t getMillisSinceEpoch()
{
    // If we have no GPS data, return 0 to indicate that.
    if (isGpsDataCold()) return 0;

    //interpolate milliseconds from system clock
    return g_gpsSnapshot.sample.time + getDeltaSinceSample();
}

/**
 * Use this method add hoc when you don't have access to GpsSnapshot
 */
tiny_millis_t getMillisSinceFirstFix()
{
    // If we have no GPS data, return 0 to indicate that.
    if (isGpsDataCold()) return 0;

    return (tiny_millis_t) (getMillisSinceEpoch() - g_timeFirstFix);
}

long long getMillisSinceEpochAsLongLong()
{
    return (long long) getMillisSinceEpoch();
}

tiny_millis_t getUptimeAtSample()
{
    return g_uptimeAtSample;
}

float GPS_getLatitude()
{
    return g_gpsSnapshot.sample.point.latitude;
}

float GPS_getLongitude()
{
    return g_gpsSnapshot.sample.point.longitude;
}

float getAltitude()
{
    return g_gpsSnapshot.sample.altitude;
}

int GPS_getQuality()
{
    return (int)g_gpsSnapshot.sample.quality;
}

float GPS_getDOP()
{
    return g_gpsSnapshot.sample.DOP;
}

int GPS_getSatellitesUsedForPosition()
{
    return g_gpsSnapshot.sample.satellites;
}

float getGPSSpeed()
{
    return g_gpsSnapshot.sample.speed;
}

float getGpsSpeedInMph()
{
    return getGPSSpeed() * 0.621371192; //convert to MPH
}

millis_t getLastFix()
{
    return g_gpsSnapshot.sample.time;
}

GeoPoint getGeoPoint()
{
    return g_gpsSnapshot.sample.point;
}

GeoPoint getPreviousGeoPoint()
{
    return g_gpsSnapshot.previousPoint;
}

GpsSample getGpsSample()
{
    return g_gpsSnapshot.sample;
}

GpsSnapshot getGpsSnapshot()
{
    return g_gpsSnapshot;
}

static void updateFullDateTime(GpsSample *gpsSample)
{
    g_uptimeAtSample = getUptime();

    if (g_timeFirstFix == 0) g_timeFirstFix = gpsSample->time;
}

void GPS_sample_update(GpsSample *newSample)
{
    if (!isGpsSignalUsable(newSample->quality)) return;

    const GeoPoint prevPoint = g_gpsSnapshot.sample.point;

    // Deep copy stuff.
    g_gpsSnapshot.sample = *newSample;
    updateFullDateTime(newSample);
    g_gpsSnapshot.deltaFirstFix = newSample->time - g_timeFirstFix;
    g_gpsSnapshot.previousPoint = prevPoint;
}

int GPS_processUpdate(Serial *serial)
{
    GpsSample s;
    const gps_msg_result_t result = GPS_device_get_update(&s, serial);

    flashGpsStatusLed(s.quality);

    if (result == GPS_MSG_SUCCESS) {
        GPS_sample_update(&s);
    }

    return result;
}

int checksumValid(const char *gpsData, size_t len)
{
    int valid = 0;
    unsigned char checksum = 0;
    size_t i = 0;

    for (; i < len - 1; i++) {
        char c = *(gpsData + i);
        if (c == '*' || c == '\0') break;
        else if (c == '$') continue;
        else checksum ^= c;
    }

    if (len > i + 2) {
        unsigned char dataChecksum = modp_xtoc(gpsData + i + 1);
        if (checksum == dataChecksum)
            valid = 1;
    }

    return valid;
}
