/*
 * Race Capture Pro Firmware
 *
 * Copyright (C) 2016 Autosport Labs
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

#include "LapStatsTest.hh"
#include "track_test.h"

#include "lap_stats.h"
#include "dateTime.h"
#include "gps.h"

/* Inclue the code to test here */
extern "C" {
#include "lap_stats.c"
}

CPPUNIT_TEST_SUITE_REGISTRATION( LapStatsTest );

GpsSnapshot gps_ss;

void LapStatsTest::setUp()
{
        lapStats_init();

        /* Give us valid values. */
        gps_ss.sample.quality = GPS_QUALITY_2D;
        gps_ss.sample.point.latitude = 1.1;
        gps_ss.sample.point.longitude = 2.1;
        gps_ss.sample.time = 1234567;
        gps_ss.deltaFirstFix = 303;

        gps_ss.previousPoint.latitude = 1.0;
        gps_ss.previousPoint.longitude = 2.0;
}

void LapStatsTest::tearDown() {}

void LapStatsTest::test_init()
{
        /* Internal state checks.  Only do these if no other options */
        CPPUNIT_ASSERT_EQUAL((float) 0, g_geo_circle_radius);
        CPPUNIT_ASSERT_EQUAL(0, g_configured);

        CPPUNIT_ASSERT_EQUAL((tiny_millis_t) 0, lapstats_elapsed_time());
        CPPUNIT_ASSERT_EQUAL((tiny_millis_t) 0, getLastLapTime());
        CPPUNIT_ASSERT_EQUAL((tiny_millis_t) 0, getLastSectorTime());

        CPPUNIT_ASSERT_EQUAL(0, getLapCount());
        CPPUNIT_ASSERT_EQUAL(0, lapstats_current_lap());

        CPPUNIT_ASSERT_EQUAL(-1, getSector());
        CPPUNIT_ASSERT_EQUAL(-1, getLastSector());

        CPPUNIT_ASSERT_EQUAL((float) 0, getLapDistance());

        CPPUNIT_ASSERT_EQUAL(0, getAtStartFinish());
        CPPUNIT_ASSERT_EQUAL(0, getAtSector());
        CPPUNIT_ASSERT_EQUAL(false, lapstats_lap_in_progress());

        CPPUNIT_ASSERT_EQUAL(0, lapstats_get_selected_track_id());
        CPPUNIT_ASSERT_EQUAL(TRACK_STATUS_WAITING_TO_CONFIG,
                             lapstats_get_track_status());

        lapstats_location_updated(&gps_ss);
}

void LapStatsTest::null_track_test()
{
        lapstats_setup_internals(NULL, 10, false);

        CPPUNIT_ASSERT_EQUAL(0, g_configured);
        CPPUNIT_ASSERT_EQUAL(TRACK_STATUS_WAITING_TO_CONFIG,
                             lapstats_get_track_status());

        lapstats_location_updated(&gps_ss);
}

void LapStatsTest::invalid_track_test()
{
        const Track track = TEST_TRACK_INVALID_CIRCUIT_TRACK;
        lapstats_setup_internals(&track, 10, false);

        CPPUNIT_ASSERT_EQUAL(1, g_configured);
        CPPUNIT_ASSERT_EQUAL(TRACK_STATUS_FIXED_CONFIG,
                             lapstats_get_track_status());

        lapstats_location_updated(&gps_ss);
}

void LapStatsTest::automatic_track_test()
{
        const Track track = TEST_TRACK_VALID_CIRCUIT_TRACK;
        lapstats_setup_internals(&track, 10, true);

        CPPUNIT_ASSERT_EQUAL(1, g_configured);
        CPPUNIT_ASSERT_EQUAL(TRACK_STATUS_AUTO_DETECTED,
                             lapstats_get_track_status());
}

void LapStatsTest::manual_track_test()
{
        const Track track = TEST_TRACK_VALID_CIRCUIT_TRACK;
        lapstats_setup_internals(&track, 10, false);

        CPPUNIT_ASSERT_EQUAL(1, g_configured);
        CPPUNIT_ASSERT_EQUAL(TRACK_STATUS_FIXED_CONFIG,
                             lapstats_get_track_status());
}

void LapStatsTest::sectors_disabled_test()
{
        const Track track = TEST_TRACK_VALID_CIRCUIT_TRACK_NO_SECTORS;
        lapstats_setup_internals(&track, 10, false);

        CPPUNIT_ASSERT_EQUAL(false, (bool) g_sector_enabled);
}

void LapStatsTest::sectors_enabled_test()
{
        const Track track = TEST_TRACK_VALID_CIRCUIT_TRACK;
        lapstats_setup_internals(&track, 10, false);

        CPPUNIT_ASSERT_EQUAL(true, (bool) g_sector_enabled);
}

void LapStatsTest::circuit_lap_start_event_test()
{
        const Track track = TEST_TRACK_VALID_CIRCUIT_TRACK;
        lapstats_setup_internals(&track, 10, false);

        const tiny_millis_t time = 45;
        const GeoPoint pt = gps_ss.sample.point;
        const float distance = 7;
        lap_started_event(time, &pt, distance);

        CPPUNIT_ASSERT_EQUAL(time, g_lapStartTimestamp);

        CPPUNIT_ASSERT_EQUAL((tiny_millis_t) 0, lapstats_elapsed_time());
        CPPUNIT_ASSERT_EQUAL((tiny_millis_t) 0, getLastLapTime());
        CPPUNIT_ASSERT_EQUAL((tiny_millis_t) 0, getLastSectorTime());

        CPPUNIT_ASSERT_EQUAL(0, getLapCount());
        CPPUNIT_ASSERT_EQUAL(1, lapstats_current_lap());

        CPPUNIT_ASSERT_EQUAL(0, getSector());
        CPPUNIT_ASSERT_EQUAL(-1, getLastSector());

        CPPUNIT_ASSERT_EQUAL(distance, getLapDistance());

        CPPUNIT_ASSERT_EQUAL(true, (bool) getAtStartFinish());
        CPPUNIT_ASSERT_EQUAL(true, (bool) lapstats_lap_in_progress());

        CPPUNIT_ASSERT_EQUAL(false,
                             isGeoTriggerTripped(&g_finish_geo_trigger));
}

void LapStatsTest::stage_lap_start_event_test()
{
        const Track track = TEST_TRACK_VALID_STAGE_TRACK;
        lapstats_setup_internals(&track, 10, false);

        const tiny_millis_t time = 42;
        const GeoPoint pt = gps_ss.sample.point;
        const float distance = 5;
        lap_started_event(time, &pt, distance);

        CPPUNIT_ASSERT_EQUAL(time, g_lapStartTimestamp);

        CPPUNIT_ASSERT_EQUAL((tiny_millis_t) 0, lapstats_elapsed_time());
        CPPUNIT_ASSERT_EQUAL((tiny_millis_t) 0, getLastLapTime());
        CPPUNIT_ASSERT_EQUAL((tiny_millis_t) 0, getLastSectorTime());

        CPPUNIT_ASSERT_EQUAL(0, getLapCount());
        CPPUNIT_ASSERT_EQUAL(1, lapstats_current_lap());

        CPPUNIT_ASSERT_EQUAL(0, getSector());
        CPPUNIT_ASSERT_EQUAL(-1, getLastSector());

        CPPUNIT_ASSERT_EQUAL(distance, getLapDistance());

        CPPUNIT_ASSERT_EQUAL(true, (bool) getAtStartFinish());
        CPPUNIT_ASSERT_EQUAL(true, (bool) lapstats_lap_in_progress());

        CPPUNIT_ASSERT_EQUAL(false,
                             isGeoTriggerTripped(&g_finish_geo_trigger));
}

void LapStatsTest::circuit_lap_finish_event_test()
{
        const Track track = TEST_TRACK_VALID_CIRCUIT_TRACK;
        lapstats_setup_internals(&track, 10, false);

        const tiny_millis_t stime = 3;
        const GeoPoint pt = gps_ss.sample.point;
        const float sdistance = 4;
        lap_started_event(stime, &pt, sdistance);

        lap_finished_event(&gps_ss);

        CPPUNIT_ASSERT_EQUAL(-1, g_lapStartTimestamp);

        CPPUNIT_ASSERT_EQUAL(gps_ss.deltaFirstFix - stime,
                             getLastLapTime());

        CPPUNIT_ASSERT_EQUAL(1, getLapCount());
        CPPUNIT_ASSERT_EQUAL(1, lapstats_current_lap());

        CPPUNIT_ASSERT_EQUAL(sdistance, getLapDistance());

        CPPUNIT_ASSERT_EQUAL(true, (bool) getAtStartFinish());
        CPPUNIT_ASSERT_EQUAL(false, (bool) lapstats_lap_in_progress());

        /* Stays tripped in CIRCUIT tracks */
        CPPUNIT_ASSERT_EQUAL(true,
                             isGeoTriggerTripped(&g_start_geo_trigger));
}

void LapStatsTest::stage_lap_finish_event_test()
{
        const Track track = TEST_TRACK_VALID_STAGE_TRACK;
        lapstats_setup_internals(&track, 10, false);

        const tiny_millis_t stime = 6;
        const GeoPoint pt = gps_ss.sample.point;
        const float sdistance = 8;
        lap_started_event(stime, &pt, sdistance);

        lap_finished_event(&gps_ss);

        CPPUNIT_ASSERT_EQUAL(-1, g_lapStartTimestamp);

        CPPUNIT_ASSERT_EQUAL(gps_ss.deltaFirstFix - stime,
                             getLastLapTime());

        CPPUNIT_ASSERT_EQUAL(1, getLapCount());
        CPPUNIT_ASSERT_EQUAL(1, lapstats_current_lap());

        CPPUNIT_ASSERT_EQUAL(sdistance, getLapDistance());

        CPPUNIT_ASSERT_EQUAL(true, (bool) getAtStartFinish());
        CPPUNIT_ASSERT_EQUAL(false, (bool) lapstats_lap_in_progress());

        /* Resets on STAGE tracks */
        CPPUNIT_ASSERT_EQUAL(false,
                             isGeoTriggerTripped(&g_start_geo_trigger));
}

void LapStatsTest::sector_boundary_event_test()
{
        const Track track = TEST_TRACK_VALID_STAGE_TRACK;
        lapstats_setup_internals(&track, 10, false);

        const tiny_millis_t stime = 17;
        const GeoPoint pt = gps_ss.sample.point;
        const float sdistance = 0;
        lap_started_event(stime, &pt, sdistance);

        sector_boundary_event(&gps_ss);

        CPPUNIT_ASSERT_EQUAL(gps_ss.deltaFirstFix - stime,
                             getLastSectorTime());
        CPPUNIT_ASSERT_EQUAL(1, getSector());
        CPPUNIT_ASSERT_EQUAL(0, getLastSector());
        CPPUNIT_ASSERT_EQUAL(true, (bool) getAtSector());
}

void LapStatsTest::update_distance_test()
{
        const float expected = distPythag(&gps_ss.previousPoint,
                                          &gps_ss.sample.point) / 1000;

        CPPUNIT_ASSERT_EQUAL((float) 0, getLapDistance());

        update_distance(&gps_ss);

        CPPUNIT_ASSERT_EQUAL(expected, getLapDistance());
}

void LapStatsTest::update_distance_no_prev_point_test()
{
        CPPUNIT_ASSERT_EQUAL((float) 0, getLapDistance());

        gps_ss.previousPoint = (GeoPoint) { 0 };
        update_distance(&gps_ss);

        CPPUNIT_ASSERT_EQUAL((float) 0, getLapDistance());
}

void LapStatsTest::update_sector_geo_circle_test()
{
        const Track track = TEST_TRACK_VALID_STAGE_TRACK;
        lapstats_setup_internals(&track, 1, false);

        const GeoPoint point0 =
                getSectorGeoPointAtIndex(&track, 0);
        CPPUNIT_ASSERT_EQUAL(point0.latitude, g_geo_circles.sector.point.latitude);
        CPPUNIT_ASSERT_EQUAL(point0.longitude, g_geo_circles.sector.point.longitude);

        update_sector_geo_circle(1);

        const GeoPoint point1 =
                getSectorGeoPointAtIndex(&track, 1);
        CPPUNIT_ASSERT_EQUAL(point1.latitude, g_geo_circles.sector.point.latitude);
        CPPUNIT_ASSERT_EQUAL(point1.longitude, g_geo_circles.sector.point.longitude);
}

void LapStatsTest::update_elapsed_time_test()
{
        const Track track = TEST_TRACK_VALID_STAGE_TRACK;
        lapstats_setup_internals(&track, 1, false);

        const tiny_millis_t stime = 5;
        const GeoPoint pt = gps_ss.sample.point;
        const float sdistance = 94;
        lap_started_event(stime, &pt, sdistance);

        CPPUNIT_ASSERT_EQUAL((tiny_millis_t) 0, lapstats_elapsed_time());

        update_elapsed_time(&gps_ss);

        CPPUNIT_ASSERT_EQUAL(gps_ss.deltaFirstFix - stime,
                             lapstats_elapsed_time());
}

void LapStatsTest::at_sf_reset_test()
{
        const Track track = TEST_TRACK_VALID_STAGE_TRACK;
        lapstats_setup_internals(&track, 1, false);

        CPPUNIT_ASSERT_EQUAL(false, (bool) getAtStartFinish());

        const tiny_millis_t stime = 5;
        const GeoPoint pt = gps_ss.sample.point;
        const float sdistance = 94;
        lap_started_event(stime, &pt, sdistance);

        CPPUNIT_ASSERT_EQUAL(true, (bool) getAtStartFinish());

        lapstats_location_updated(&gps_ss);

        CPPUNIT_ASSERT_EQUAL(false, (bool) getAtStartFinish());
}

void LapStatsTest::at_sector_reset_test()
{
        const Track track = TEST_TRACK_VALID_STAGE_TRACK;
        lapstats_setup_internals(&track, 1, false);

        CPPUNIT_ASSERT_EQUAL(false, (bool) getAtSector());

        const tiny_millis_t stime = 5;
        const GeoPoint pt = gps_ss.sample.point;
        const float sdistance = 94;
        lap_started_event(stime, &pt, sdistance);
        sector_boundary_event(&gps_ss);

        CPPUNIT_ASSERT_EQUAL(true, (bool) getAtSector());

        lapstats_location_updated(&gps_ss);

        CPPUNIT_ASSERT_EQUAL(false, (bool) getAtSector());
}
