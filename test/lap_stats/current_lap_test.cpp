/**
 * Race Capture Pro Firmware
 *
 * Copyright (C) 2014 Autosport Labs
 *
 * This file is part of the Race Capture Pro fimrware suite
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

#include "current_lap_test.h"
#include "gps.h"
#include "lap_stats.testing.h"

#include <string.h>

static GpsSnapshot gpsSnap;
static Track track;

CPPUNIT_TEST_SUITE_REGISTRATION( Current_Lap_Test );

void Current_Lap_Test::setUp() {
        lapStats_init();
        set_active_track(&track);
        memset(&gpsSnap, 0, sizeof(GpsSnapshot));
}

void Current_Lap_Test::test_lap_increment() {
        CPPUNIT_ASSERT_EQUAL(0, lapstats_get_lap());

        lap_started_normal_event(&gpsSnap);

        CPPUNIT_ASSERT_EQUAL(1, lapstats_get_lap());
}

void Current_Lap_Test::test_no_increment() {
        CPPUNIT_ASSERT_EQUAL(0, lapstats_get_lap());

        lap_finished_event(&gpsSnap);

        CPPUNIT_ASSERT_EQUAL(0, lapstats_get_lap());
}

void Current_Lap_Test::test_reset() {
        lap_started_normal_event(&gpsSnap);
        CPPUNIT_ASSERT_EQUAL(1, lapstats_get_lap());

        reset_lap();
        CPPUNIT_ASSERT_EQUAL(0, lapstats_get_lap());
}
