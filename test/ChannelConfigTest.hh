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

#ifndef _CHANNELCONFIGTEST_H_
#define _CHANNELCONFIGTEST_H_

#include <cppunit/extensions/HelperMacros.h>

class ChannelConfigTest : public CppUnit::TestFixture
{
        CPPUNIT_TEST_SUITE( ChannelConfigTest );
        CPPUNIT_TEST( test_defaults );
        CPPUNIT_TEST( test_validate_label );
        CPPUNIT_TEST( test_validate_units );
        CPPUNIT_TEST( test_validate );
        CPPUNIT_TEST_SUITE_END();

public:
        void setUp();
        void test_defaults();
        void test_validate_label();
        void test_validate_units();
        void test_validate();
};

#endif /* _CHANNELCONFIGTEST_H_ */
