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

#ifndef _LAPSTATSTEST_H_
#define _LAPSTATSTEST_H_

#include <cppunit/extensions/HelperMacros.h>

class LapStatsTest : public CppUnit::TestFixture
{
        CPPUNIT_TEST_SUITE( LapStatsTest );
        CPPUNIT_TEST( reset_test );
        CPPUNIT_TEST( reset_track_test );
        CPPUNIT_TEST( config_changed_test );
        CPPUNIT_TEST( null_track_test );
        CPPUNIT_TEST( invalid_track_test );
        CPPUNIT_TEST( automatic_track_test );
        CPPUNIT_TEST( manual_track_test );
        CPPUNIT_TEST( sectors_disabled_test );
        CPPUNIT_TEST( sectors_enabled_test );
        CPPUNIT_TEST( circuit_lap_start_event_test );
        CPPUNIT_TEST( stage_lap_start_event_test );
        CPPUNIT_TEST( circuit_lap_finish_event_test );
        CPPUNIT_TEST( stage_lap_finish_event_test );
        CPPUNIT_TEST( sector_boundary_event_test );
        CPPUNIT_TEST( update_distance_test );
        CPPUNIT_TEST( update_distance_low_speed_test );
        CPPUNIT_TEST( update_sector_geo_circle_test );
        CPPUNIT_TEST( update_elapsed_time_test );
        CPPUNIT_TEST( at_sf_reset_test );
        CPPUNIT_TEST( at_sector_reset_test );
        CPPUNIT_TEST_SUITE_END();

public:
        void setUp();
        void reset_test();
        void reset_track_test();
        void config_changed_test();
        void null_track_test();
        void invalid_track_test();
        void automatic_track_test();
        void manual_track_test();
        void sectors_disabled_test();
        void sectors_enabled_test();
        void circuit_lap_start_event_test();
        void stage_lap_start_event_test();
        void circuit_lap_finish_event_test();
        void stage_lap_finish_event_test();
        void sector_boundary_event_test();
        void update_distance_test();
        void update_distance_low_speed_test();
        void update_sector_geo_circle_test();
        void update_elapsed_time_test();
        void at_sf_reset_test();
        void at_sector_reset_test();
};


#endif /* _LAPSTATSTEST_H_ */
