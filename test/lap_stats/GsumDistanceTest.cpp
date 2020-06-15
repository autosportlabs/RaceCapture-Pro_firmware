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
 
#include "GsumDistanceTest.hh"
#include "lap_stats.h"

/* Inclue the code to test here */
extern "C" {
#include "gsum_distance.c"
}

CPPUNIT_TEST_SUITE_REGISTRATION( GsumDistanceTest );

void GsumDistanceTest::get_segment_by_distance_test()
{
        set_distance(6.3789);
        int segment = get_segment_by_distance();
        CPPUNIT_ASSERT_EQUAL(637, segment);
}

void GsumDistanceTest::get_segment_by_distance_border_distance_test()
{
        set_distance(12.232);
        int segment = get_segment_by_distance();
        CPPUNIT_ASSERT_EQUAL(1000, segment);
}