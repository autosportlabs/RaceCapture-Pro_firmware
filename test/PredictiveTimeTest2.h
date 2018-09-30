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

#ifndef PREDICTIVETIMETEST2_H_
#define PREDICTIVETIMETEST2_H_

#include <cppunit/extensions/HelperMacros.h>
#include <string>
#include <stdlib.h>

#include "geopoint.h"
#include "predictive_timer/predictive_timer_2.h"

using std::string;
using std::vector;

//// HACK.  Exposing the testing methods here
//float distPctBtwnTwoPoints(GeoPoint *s, GeoPoint *e, GeoPoint *m);

class PredictiveTimeTest2 : public CppUnit::TestFixture
{
        CPPUNIT_TEST_SUITE( PredictiveTimeTest2 );
        //	CPPUNIT_TEST( testPredictedTimeGpsFeed );
        CPPUNIT_TEST( testProjectedDistance );
        CPPUNIT_TEST_SUITE_END();

public:
        PredictiveTimeTest2();
        void setUp();
        void tearDown();
        void testPredictedTimeGpsFeed();
        void testProjectedDistance();

private:
        string readFile(string filename);
        vector<string> split(string &s, char delim);
        vector<string> & split(string &s, char delim, vector<string> &elems);
};

#endif /* PREDICTIVETIMETEST2_H_ */
