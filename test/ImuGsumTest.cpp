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
 
#include "ImuGsumTest.hh"

/* Include the code to test here */
extern "C" {
#include "imu_gsum.c"
}

CPPUNIT_TEST_SUITE_REGISTRATION( ImuGsumTest );

void ImuGsumTest::get_imu_gsum_max_new_value_test()
{
        g_gsum = 1.05;
        set_distance(0);
        g_gsum_maxes[0] = 0.99;
        CPPUNIT_ASSERT_EQUAL(1.05, get_imu_gsum_max());
        CPPUNIT_ASSERT_EQUAL(1.05, g_gsum_maxes[0]);
}

void ImuGsumTest::get_imu_gsum_max_old_value_test()
{
        g_gsum = 1.05;
        set_distance(0);
        g_gsum_maxes[0] = 1.46;
        CPPUNIT_ASSERT_EQUAL(1.46, get_imu_gsum_max());
        CPPUNIT_ASSERT_EQUAL(1.46, g_gsum_maxes[0]);
}

void ImuGsumTest::get_imu_gsum_pct_test()
{
        g_gsum = 1;
        set_distance(0);
        g_gsum_maxes[0] = 2;
        CPPUNIT_ASSERT_EQUAL(50, get_imu_gsum_pct());
}
