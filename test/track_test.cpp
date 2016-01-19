/*
 * Race Capture Firmware
 *
 * Copyright (C) 2016 Autosport Labs
 *
 * This file is part of the Race Capture firmware suite
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

#include "tracks.h"
#include "track_test.h"

CPPUNIT_TEST_SUITE_REGISTRATION( TrackTest );

void TrackTest::setUp() {}

void TrackTest::tearDown() {}

void TrackTest::testGetFinish() {
   const Track vCircuit = TEST_TRACK_VALID_CIRCUIT_TRACK;
   const Track ivCircuit = TEST_TRACK_INVALID_CIRCUIT_TRACK;
   const Track vStage = TEST_TRACK_VALID_STAGE_TRACK;
   const Track ivStage = TEST_TRACK_INVALID_STAGE_TRACK;

   CPPUNIT_ASSERT(isFinishPointValid(&vCircuit));
   CPPUNIT_ASSERT(isFinishPointValid(&vStage));
   CPPUNIT_ASSERT(!isFinishPointValid(&ivCircuit));
   CPPUNIT_ASSERT(!isFinishPointValid(&ivStage));

   const GeoPoint cfl = getFinishPoint(&vCircuit);
   const GeoPoint expCfl = vCircuit.circuit.startFinish;
   CPPUNIT_ASSERT_EQUAL(cfl.latitude, expCfl.latitude);
   CPPUNIT_ASSERT_EQUAL(cfl.longitude, expCfl.longitude);

   const GeoPoint sfl = getFinishPoint(&vStage);
   const GeoPoint expSfl = vStage.stage.finish;
   CPPUNIT_ASSERT_EQUAL(sfl.latitude, expSfl.latitude);
   CPPUNIT_ASSERT_EQUAL(sfl.longitude, expSfl.longitude);
}

void TrackTest::testGetStart() {
   const Track vCircuit = TEST_TRACK_VALID_CIRCUIT_TRACK;
   const Track ivCircuit = TEST_TRACK_INVALID_CIRCUIT_TRACK;
   const Track vStage = TEST_TRACK_VALID_STAGE_TRACK;
   const Track ivStage = TEST_TRACK_INVALID_STAGE_TRACK;

   CPPUNIT_ASSERT(isStartPointValid(&vCircuit));
   CPPUNIT_ASSERT(isStartPointValid(&vStage));
   CPPUNIT_ASSERT(!isStartPointValid(&ivCircuit));
   CPPUNIT_ASSERT(!isStartPointValid(&ivStage));

   const GeoPoint c = getStartPoint(&vCircuit);
   const GeoPoint exp = vCircuit.circuit.startFinish;
   CPPUNIT_ASSERT_EQUAL(c.latitude, exp.latitude);
   CPPUNIT_ASSERT_EQUAL(c.longitude, exp.longitude);

   const GeoPoint sfl = getStartPoint(&vStage);
   const GeoPoint expSfl = vStage.stage.start;
   CPPUNIT_ASSERT_EQUAL(sfl.latitude, expSfl.latitude);
   CPPUNIT_ASSERT_EQUAL(sfl.longitude, expSfl.longitude);
}

void TrackTest::testGetSector() {
   const Track vcTrack = TEST_TRACK_VALID_CIRCUIT_TRACK;
   const Track vsTrack = TEST_TRACK_VALID_STAGE_TRACK;

   // The Circuit Test track has 4 sectors while the Stage has 3.
   const int cSecMax = 4;
   const int sSecMax = 3;

   for(int i = 0; i < 5; ++i) {
     const GeoPoint cp = getSectorGeoPointAtIndex(&vcTrack , i);
     const GeoPoint ecp = i < cSecMax ? vcTrack.circuit.sectors[i] : vcTrack.circuit.startFinish;

     CPPUNIT_ASSERT_EQUAL(ecp.latitude, cp.latitude);
     CPPUNIT_ASSERT_EQUAL(ecp.longitude, cp.longitude);

     const GeoPoint sp = getSectorGeoPointAtIndex(&vsTrack , i);
     const GeoPoint esp = i < sSecMax ? vsTrack.stage.sectors[i] : vsTrack.stage.finish;

     CPPUNIT_ASSERT_EQUAL(esp.latitude, sp.latitude);
     CPPUNIT_ASSERT_EQUAL(esp.longitude, sp.longitude);
   }
}

void TrackTest::testGeoPointsEqual() {
  GeoPoint a = { .latitude = 1.0, .longitude = 2.0 };
  GeoPoint b = { .latitude = 3.0, .longitude = 4.0 };

  CPPUNIT_ASSERT(areGeoPointsEqual(a, a));
  CPPUNIT_ASSERT(areGeoPointsEqual(b, b));
  CPPUNIT_ASSERT(!areGeoPointsEqual(a, b));
}

void TrackTest::testGeoPointsValid() {
  GeoPoint v1 = { .latitude = 0.0, .longitude = 2.0 };
  GeoPoint v2 = { .latitude = 1.0, .longitude = 0.0 };
  GeoPoint i = { .latitude = 0.0, .longitude = 0.0 };

  CPPUNIT_ASSERT(isValidPoint(&v1));
  CPPUNIT_ASSERT(isValidPoint(&v2));
  CPPUNIT_ASSERT(!isValidPoint(&i));
}
