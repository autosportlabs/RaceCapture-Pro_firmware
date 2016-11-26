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

#ifndef PREDICTIVE_TIME_TEST_H_
#define PREDICTIVE_TIME_TEST_H_

#include <cppunit/extensions/HelperMacros.h>
#include <string>

using std::string;
using std::vector;

class SectorTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( SectorTest );
    CPPUNIT_TEST( testSectorTimes );
    CPPUNIT_TEST( testStageSectorTimes );
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    string readFile(string filename);
    vector<string> split(string &s, char delim);
    vector<string> &split(string &s, char delim, vector<string> &elems);

    float sumSectorTimes(vector<float> & sectorTimes);
    void outputSectorTimes(vector<float> & sectorTimes, int lap);
    void testSectorTimes();
    void testStageSectorTimes();
    static bool is_debug();
};


#endif /* PREDICTIVE_TIME_TEST_H_ */
