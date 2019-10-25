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

#include "timing_scoring_test.h"
#include "timing_scoring_drv.h"
#include <cppunit/extensions/HelperMacros.h>
#include "loggerConfig.h"
// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( TimingScoringTest );

void TimingScoringTest::setUp() {
        reset_logger_config();
}

void TimingScoringTest::tearDown() {}

void TimingScoringTest::test_getter_setters()
{
        TimingScoringState *ts = timing_scoring_get_state();
        ts->driver_id = 1234;
        CPPUNIT_ASSERT_EQUAL((int)1234, timing_scoring_get_driver_id());

        ts->position_in_class = 43;
        CPPUNIT_ASSERT_EQUAL((int)43, timing_scoring_get_position_in_class());

        ts->car_number_ahead = 55;
        CPPUNIT_ASSERT_EQUAL((int)55, timing_scoring_get_car_number_ahead());

        ts->gap_to_ahead = 1.234;
        CPPUNIT_ASSERT_EQUAL((float)1.234, timing_scoring_get_gap_to_ahead());

        ts->car_number_behind = 66;
        CPPUNIT_ASSERT_EQUAL((int)66, timing_scoring_get_car_number_behind());

        ts->gap_to_behind = 2.35;
        CPPUNIT_ASSERT_EQUAL((float)2.35, timing_scoring_get_gap_to_behind());

        ts->tns_laptime = 4.23;
        CPPUNIT_ASSERT_EQUAL((float)4.23, timing_scoring_get_tns_laptime());

        ts->full_course_flag_status = 3;
        CPPUNIT_ASSERT_EQUAL((int)3, timing_scoring_get_full_course_status());

        ts->black_flag = true;
        CPPUNIT_ASSERT_EQUAL((bool)true, timing_scoring_get_black_flag());
}

