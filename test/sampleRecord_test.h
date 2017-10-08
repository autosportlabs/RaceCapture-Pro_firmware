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

#ifndef SAMPLERECORD_TEST_H_
#define SAMPLERECORD_TEST_H_

#include <cppunit/extensions/HelperMacros.h>

class SampleRecordTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( SampleRecordTest );
    CPPUNIT_TEST( testInitSampleRecord );
    CPPUNIT_TEST( testPopulateSampleRecord );
    CPPUNIT_TEST( testIsValidLoggerMessage );
    CPPUNIT_TEST( testLoggerMessageAlwaysHasTime );
    CPPUNIT_TEST( test_get_sample_value_by_name );
    CPPUNIT_TEST_SUITE_END();

public:

    void setUp();
    void tearDown();
    void testInitSampleRecord();
    void testPopulateSampleRecord();
    void testIsValidLoggerMessage();
    void testLoggerMessageAlwaysHasTime();
    void test_get_sample_value_by_name();

private:


};



#endif /* LOGGERAPI_TEST_H_ */
