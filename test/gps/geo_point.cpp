/**
 * Race Capture Pro Firmware
 *
 * Copyright Autosport Labs Inc.
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
 * Author: stieg
 */

#include <cppunit/extensions/HelperMacros.h>
#include <geopoint.h>

class GeoPointUnitTest : public CppUnit::TestFixture
{
        CPPUNIT_TEST_SUITE( GeoPointUnitTest );
        CPPUNIT_TEST( testDistPythagDistance );
        CPPUNIT_TEST_SUITE_END();

public:
        void setUp();
        void tearDown();
        void testDistPythagDistance();
};

void GeoPointUnitTest::setUp() {}
void GeoPointUnitTest::tearDown() {}

void GeoPointUnitTest::testDistPythagDistance()
{
        const GeoPoint gp1 = { 43.074859, -89.386336 }; // 100 State
        const GeoPoint gp2 = { 43.075255, -89.385590 }; // ~75 Meters from 100 State
        const float result = distPythag(&gp1, &gp2);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(75.0, result, 0.1);
}

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( GeoPointUnitTest );
