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

#ifndef _GSUMDISTANCETEST_H_
#define _GSUMDISTANCETEST_H_

#include <cppunit/extensions/HelperMacros.h>

class GsumDistanceTest : public CppUnit::TestFixture
{
        CPPUNIT_TEST_SUITE( GsumDistanceTest );
        CPPUNIT_TEST( get_segment_by_distance_test );
        CPPUNIT_TEST( get_segment_by_distance_border_distance_test );
        CPPUNIT_TEST_SUITE_END();

public:
        void get_segment_by_distance_test();
        void get_segment_by_distance_border_distance_test();
};

#endif /* _GSUMDISTANCETEST_H_ */