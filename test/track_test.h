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

#ifndef _TRACK_TEST_H_
#define _TRACK_TEST_H_

#include <cppunit/extensions/HelperMacros.h>

#define TEST_TRACK_VALID_CIRCUIT_TRACK {                        \
                1234,                                           \
                TRACK_TYPE_CIRCUIT,                             \
                {                                               \
                        {                                       \
                                {47.806934,-122.341150},        \
                                {47.806875,-122.335818},        \
                                {47.79974,-122.335704},         \
                                {47.799719,-122.346416},        \
                                {47.806886,-122.346494},        \
                        }                                       \
                }                                               \
        }

#define TEST_TRACK_VALID_CIRCUIT_TRACK_NO_SECTORS {             \
                1234,                                           \
                TRACK_TYPE_CIRCUIT,                             \
                {                                               \
                        {                                       \
                                {47.806934,-122.341150},        \
                        }                                       \
                }                                               \
        }

// Start, Finish, Sectors[]
#define TEST_TRACK_VALID_STAGE_TRACK {                          \
                4567,                                           \
                TRACK_TYPE_STAGE,                               \
                {                                               \
                        {                                       \
                                {48.806934,-120.341150},        \
                                {48.806886,-120.346494},        \
                                {48.806875,-120.335818},        \
                                {48.79974,-120.335704},         \
                                {48.799719,-120.346416},        \
                        }                                       \
                }                                               \
        }

#define TEST_TRACK_VALID_STAGE_TRACK_NO_SECTORS {               \
                4567,                                           \
                TRACK_TYPE_STAGE,                               \
                {                                               \
                        {                                       \
                                {48.806934,-120.341150},        \
                                {48.806886,-120.346494},        \
                        }                                       \
                }                                               \
        }

#define TEST_TRACK_INVALID_CIRCUIT_TRACK {      \
                1111,                           \
                TRACK_TYPE_CIRCUIT,             \
                {                               \
                        {                       \
                                {0.0, 0.0},     \
                        }                       \
                }                               \
        }


#define TEST_TRACK_INVALID_STAGE_TRACK {        \
                2222,                           \
                TRACK_TYPE_STAGE,               \
                {                               \
                        {                       \
                                {0.0, 0.0},     \
                        }                       \
                }                               \
        }


class TrackTest : public CppUnit::TestFixture
{
        CPPUNIT_TEST_SUITE( TrackTest );
        CPPUNIT_TEST( testGetFinish );
        CPPUNIT_TEST( testGetStart );
        CPPUNIT_TEST( testGetSector );
        CPPUNIT_TEST( testGeoPointsEqual );
        CPPUNIT_TEST( testGeoPointsValid );
        CPPUNIT_TEST_SUITE_END();

public:
        void setUp();
        void tearDown();
        void testGetFinish();
        void testGetStart();
        void testGetSector();
        void testGeoPointsEqual();
        void testGeoPointsValid();

};

#endif /* _TRACK_TEST_H_ */
