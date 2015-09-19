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

#ifndef _LAUNCH_CONTROL_TEST_H_
#define _LAUNCH_CONTROL_TEST_H_

#include <cppunit/extensions/HelperMacros.h>

class LaunchControlTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( LaunchControlTest );
    CPPUNIT_TEST( testHasLaunchedReset );
    CPPUNIT_TEST( testCircuitLaunch );
    CPPUNIT_TEST( testStageSimpleLaunch );
    CPPUNIT_TEST( testStageTrickyLaunch );
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();
    void testHasLaunchedReset();
    void testCircuitLaunch();
    void testStageSimpleLaunch();
    void testStageTrickyLaunch();
};


#endif /* _LAUNCH_CONTROL_TEST_H_ */
