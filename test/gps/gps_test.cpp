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

#include "gps_test.h"
#include "gps.h"
#include "mod_string.h"

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( GpsTest );


void GpsTest::setUp() {
	initGPS();
}


void GpsTest::tearDown() {}


void GpsTest::testChecksum() {
	const char *goodGpsData = "$GPGLL,5300.97914,N,00259.98174,E,125926,A*28";
	const char *goodGpsData2 = "$GPGLL,5300.97914,N,00259.98174,E,125926,A*28                  ";
	const char *goodGpsData3 = "$GPGSA,M,3,12,17,04,25,29,10,,,,,,,2.45,1.89,1.56*03";
	const char *badGpsData = "$GPGLL,5300.97914,N,00259.98174,E,125926,A*29"; //bad checksum

	CPPUNIT_ASSERT(checksumValid(goodGpsData, strlen(goodGpsData)) == 1);
	CPPUNIT_ASSERT(checksumValid(goodGpsData2, strlen(goodGpsData2)) == 1);
	CPPUNIT_ASSERT(checksumValid(goodGpsData3, strlen(goodGpsData3)) == 1);
	CPPUNIT_ASSERT(checksumValid(badGpsData, strlen(badGpsData)) == 0);
}

void GpsTest::testGpsDistance() {
    setGpsDistanceKms(1.0);

    CPPUNIT_ASSERT_EQUAL((float) 1.0, getGpsDistanceKms());
    CPPUNIT_ASSERT_EQUAL((float) (1.0f * KMS_TO_MILES_CONSTANT), getGpsDistanceMiles());
}
