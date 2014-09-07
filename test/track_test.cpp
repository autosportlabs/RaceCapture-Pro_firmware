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

#include "tracks.h"
#include "track_test.h"

#define VALID_CIRCUIT_TRACK {                   \
    TRACK_TYPE_CIRCUIT,                         \
      {                                         \
        {                                       \
          {47.806934,-122.341150},              \
            {47.806875,-122.335818},            \
              {47.79974,-122.335704},           \
                {47.799719,-122.346416},        \
                  {47.806886,-122.346494},      \
                    }                           \
      }                                         \
}

#define INVALID_CIRCUIT_TRACK {                 \
    TRACK_TYPE_CIRCUIT,                         \
      {                                         \
        {                                       \
          {0.0, 0.0},                           \
            }                                   \
      }                                         \
  }

// Start, Finish, Sectors[]
#define VALID_STAGE_TRACK {                     \
    TRACK_TYPE_STAGE,                           \
      {                                         \
        {                                       \
          {48.806934,-120.341150},              \
            {48.806886,-120.346494},            \
              {48.806875,-120.335818},          \
                {48.79974,-120.335704},         \
                  {48.799719,-120.346416},      \
                    }                           \
      }                                         \
  }

#define INVALID_STAGE_TRACK {                   \
    TRACK_TYPE_STAGE,                           \
    {                                           \
		{                                         \
        {0.0, 0.0},                             \
          }                                     \
    }                                           \
  }

CPPUNIT_TEST_SUITE_REGISTRATION( TrackTest );

void TrackTest::setUp() {}

void TrackTest::tearDown() {}

void TrackTest::testGetFinish() {
   const Track vCircuit = VALID_CIRCUIT_TRACK;
   const Track ivCircuit = INVALID_CIRCUIT_TRACK;
   const Track vStage = VALID_STAGE_TRACK;
   const Track ivStage = INVALID_STAGE_TRACK;

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
