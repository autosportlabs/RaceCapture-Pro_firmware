/**
 * Race Capture Pro Firmware
 *
 * Copyright (C) 2015 Autosport Labs
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
 */

#include "geo_point_test.hh"
#include "rcp_cpp_unit.hh"

#include "geopoint.h"

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( GeoPointTest );

void GeoPointTest::setUp() {}

void GeoPointTest::tearDown() {}

void GeoPointTest::test_gps_bearing() {
        /*
         * All points relative to Madworks.  Madworks is the center.
         */
        const GeoPoint gp_madworks = { 43.058228, -89.477696 };
        const GeoPoint gp_north = { 43.058965, -89.477654 };
        const GeoPoint gp_east = { 43.058181, -89.474564 };
        const GeoPoint gp_south = { 43.056597, -89.477589 };
        const GeoPoint gp_west = { 43.058181, -89.480464 };

        const float tolerance = 3; /* 3 degrees of tolerance */

        const float north_heading = gps_bearing(&gp_madworks, &gp_north);
        RCPUNIT_DEBUG("North heading is %f\n", north_heading);
        RCPUNIT_ASSERT_CLOSE((float) 0,   tolerance, north_heading);

        const float south_heading = gps_bearing(&gp_madworks, &gp_south);
        RCPUNIT_DEBUG("South heading is %f\n", south_heading);
        RCPUNIT_ASSERT_CLOSE((float) 180,   tolerance, south_heading);

        const float east_heading = gps_bearing(&gp_madworks, &gp_east);
        RCPUNIT_DEBUG("East heading is %f\n", east_heading);
        RCPUNIT_ASSERT_CLOSE((float) 90,   tolerance, east_heading);

        const float west_heading = gps_bearing(&gp_madworks, &gp_west);
        RCPUNIT_DEBUG("West heading is %f\n", west_heading);
        RCPUNIT_ASSERT_CLOSE((float) 270,   tolerance, west_heading);
}
