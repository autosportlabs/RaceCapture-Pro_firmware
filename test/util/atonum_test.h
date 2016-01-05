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

#ifndef ATONUMTEST_H
#define ATONUMTEST_H

#include <cppunit/extensions/HelperMacros.h>

class AtoNumTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( AtoNumTest );
    CPPUNIT_TEST( testHexByteConversion );
    CPPUNIT_TEST( testDoubleConversion );
    CPPUNIT_TEST( testFloatConversion );
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testHexByteConversion(void);
    void testDoubleConversion(void);
    void testFloatConversion(void);
};

#endif  // ATONUMTEST_H
