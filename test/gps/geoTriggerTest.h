/**
 * Race Capture Firmware
 *
 * Copyright (C) 2014 Autosport Labs
 *
 * This file is part of the Race Capture firmware suite
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

#ifndef _GEOTRIGGERTEST_H_
#define _GEOTRIGGERTEST_H_

#include <cppunit/extensions/HelperMacros.h>

class GeoTriggerTest : public CppUnit::TestFixture
{
        CPPUNIT_TEST_SUITE( GeoTriggerTest );
        CPPUNIT_TEST( testShouldTrigger );
        CPPUNIT_TEST( testNoTrigger );
        CPPUNIT_TEST( testReset );
        CPPUNIT_TEST_SUITE_END();

public:
        void setUp();
        void tearDown();
        void testShouldTrigger();
        void testNoTrigger();
        void testReset();
};


#endif /* _GEOTRIGGERTEST_H_ */
