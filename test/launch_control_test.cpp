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

#include "gps.h"
#include "launch_control_test.h"
#include "launch_control.h"
#include "tracks.h"

#include <cppunit/extensions/HelperMacros.h>

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( LaunchControlTest );

void LaunchControlTest::setUp() {
   lc_reset();
}

void LaunchControlTest::tearDown() {}

void LaunchControlTest::testHasLaunchedReset() {
   CPPUNIT_ASSERT_EQUAL(false, lc_hasLaunched());
   CPPUNIT_ASSERT_EQUAL(false, lc_is_armed());
   CPPUNIT_ASSERT_EQUAL(-1, lc_getLaunchTime());

   lc_reset();

   CPPUNIT_ASSERT_EQUAL(false, lc_hasLaunched());
   CPPUNIT_ASSERT_EQUAL(false, lc_is_armed());
   CPPUNIT_ASSERT_EQUAL(-1, lc_getLaunchTime());
}

void LaunchControlTest::testCircuitLaunch() {
   /*
    * A circuit launch is the same as traditional launch.  This just ensures
    * we haven't regresed.
    */
   Circuit c = {{1.0, 1.0}};
   Track t = { TRACK_TYPE_CIRCUIT };
   t.circuit = c;

   const GeoPoint pts[] = {
      {0.9, 0.9},
      {1.0, 1.0},
      {1.1, 1.1},
      {0.0, 0.0},
   };

   lc_setup(&t, 1.0);
   CPPUNIT_ASSERT_EQUAL(false, lc_hasLaunched());
   CPPUNIT_ASSERT_EQUAL(false, lc_is_armed());
   CPPUNIT_ASSERT_EQUAL(-1, lc_getLaunchTime());

   GpsSnapshot snap;
   const GeoPoint *pt = pts;
   for (tiny_millis_t i = 0; isValidPoint(pt); ++pt, ++i) {
      snap.sample.point = *pt;
      // Use the address of the point as the sample time.
      snap.deltaFirstFix = i * 100;
      snap.sample.speed = 40; // Speed well above the threshold.

      lc_supplyGpsSnapshot(&snap);

      const bool armed = pt == &pts[1];
      CPPUNIT_ASSERT_EQUAL(armed, lc_is_armed());
   }

   CPPUNIT_ASSERT_EQUAL(true, lc_hasLaunched());
   CPPUNIT_ASSERT_EQUAL(false, lc_is_armed());
   CPPUNIT_ASSERT_EQUAL(100, lc_getLaunchTime()); // Should be point 1.
}

void LaunchControlTest::testStageSimpleLaunch() {
   /*
    * A Stage launch has a time where the driver is in the zone below a certain
    * speed.  We need to detect when they start driving.
    */
   Stage s = {{1.0, 1.0}};
   Track t = { TRACK_TYPE_CIRCUIT };
   t.stage = s;

   const GeoPoint pts[] = {
      {0.9, 0.9},
      {1.0, 1.0}, // <-- This is the arming point
      {1.0, 1.0},
      {1.0, 1.0},
      {1.0, 1.0},
      {1.0, 1.0},
      {1.0, 1.0}, // <-- This is the launch point
      {1.1, 1.1}, // <-- Launch registered here.
      {0.0, 0.0},
   };

   lc_setup(&t, 1.0);
   CPPUNIT_ASSERT_EQUAL(false, lc_hasLaunched());
   CPPUNIT_ASSERT_EQUAL(false, lc_is_armed());
   CPPUNIT_ASSERT_EQUAL(-1, lc_getLaunchTime());

   GpsSnapshot snap;
   const GeoPoint *pt = pts;
   for (unsigned indx = 0; isValidPoint(pt); ++pt, ++indx) {
      // Generate a fake time
      snap.deltaFirstFix = (tiny_millis_t) indx * 100;

      snap.sample.point = *pt;
      snap.sample.speed = pt < (pts + 7) ? 0: 10;
      lc_supplyGpsSnapshot(&snap);

      bool launched = pt >= (pts + 7);
      char msg[64];
      sprintf(msg, "At Index point %d\n", indx);

      CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, launched, lc_hasLaunched());

      const bool armed = &pts[1] <= pt && pt <= &pts[6];
      CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, armed, lc_is_armed());
   }

   CPPUNIT_ASSERT_EQUAL(true, lc_hasLaunched());
   CPPUNIT_ASSERT_EQUAL(false, lc_is_armed());
   CPPUNIT_ASSERT_EQUAL(600, lc_getLaunchTime());
}


void LaunchControlTest::testStageTrickyLaunch() {
   /*
    * A Stage launch has a time where the driver is in the zone below a certain
    * speed.  We need to detect when they start driving.
    *
    * This test tests that the speed logic works.
    */
   Stage s = {{1.0, 1.0}};
   Track t = { TRACK_TYPE_CIRCUIT };
   t.stage = s;

   const GeoPoint pts[] = {
      {0.9, 0.9},
      {1.0, 1.0},
      {1.0, 1.0},
      {1.0, 1.0},
      {1.0, 1.0}, // <-- This is the launch point time
      {1.0, 1.0},
      {1.0, 1.0},
      {1.1, 1.1}, // <-- Launch registered here.
      {1.11, 1.11},
      {1.11, 1.11},
      {1.11, 1.11},
      {0.0, 0.0},
   };

   lc_setup(&t, 1.0);
   CPPUNIT_ASSERT_EQUAL(false, lc_hasLaunched());
   CPPUNIT_ASSERT_EQUAL(-1, lc_getLaunchTime());

   GpsSnapshot snap;
   const GeoPoint *pt = pts;
   for (tiny_millis_t i = 0; isValidPoint(pt); ++pt, i += 100) {
      snap.sample.point = *pt;
      // Fake time
      snap.deltaFirstFix = i;

      // Set speed to 0 at 5th reading.  This triggers launch time set.
      snap.sample.speed = pt == (pts + 4)? 0 : 5;
      lc_supplyGpsSnapshot(&snap);

      bool launched = pt >= (pts + 7);
      CPPUNIT_ASSERT_EQUAL(launched, lc_hasLaunched());
   }

   CPPUNIT_ASSERT_EQUAL(true, lc_hasLaunched());
   CPPUNIT_ASSERT_EQUAL(400, lc_getLaunchTime());
}
