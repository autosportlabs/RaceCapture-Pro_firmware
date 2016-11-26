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

#include "gps.testing.h"
#include "lap_stats.h"
#include "loggerConfig.h"
#include "mock_serial.h"
#include "rcp_cpp_unit.hh"
#include "sector_test.h"
#include <cppunit/extensions/HelperMacros.h>
#include <fstream>
#include <stdlib.h>
#include <streambuf>
#include <string.h>

using std::ifstream;
using std::ios;
using std::istreambuf_iterator;


// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( SectorTest );

#define FILE_PREFIX string("test/")

/**
 * Like atoi, but is non-destructive to the string passed in and provides an offset and length
 * functionality.  Max Len is 3.
 * @param str The start of the String to parse.
 * @param offset How far in to start reading the string.
 * @param len The number of characters to read.
 */
static int atoiOffsetLenSafe(const char *str, size_t offset, size_t len) {
   char buff[4] = { 0 };

   // Bounds check.  Don't want any bleeding hearts in here...
   if (len > (sizeof(buff) - 1))
         len = sizeof(buff) - 1;

   memcpy(buff, str + offset, len);
   return atoi(buff);
}

void SectorTest::setUp()
{
        setupMockSerial();
	GPS_init(10, getMockSerial());
	lapstats_config_changed();
}

bool SectorTest::is_debug()
{
	const bool debug = getenv("TRACE") != NULL;
	return debug;
}
void SectorTest::tearDown()
{
}

vector<string> & SectorTest::split(string &s, char delim, vector<string> &elems) {
    std::stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

vector<string> SectorTest::split(string &s, char delim) {
    vector<string> elems;
    split(s, delim, elems);
    return elems;
}

string SectorTest::readFile(string filename){
	ifstream t(filename.c_str());
	if (!t.is_open()){
		t.open(string(FILE_PREFIX + filename).c_str());

	}
	if (!t.is_open()){
		throw ("Can not find file " + filename);
	}
	string str;

	t.seekg(0, ios::end);
	int length = t.tellg();
	str.reserve(length);
	t.seekg(0, ios::beg);

	str.assign((istreambuf_iterator<char>(t)),
				istreambuf_iterator<char>());
	return str;
}

#define Test_Track { \
    5555,               \
	TRACK_TYPE_CIRCUIT, \
	{ \
		{ \
			{47.806934,-122.341150}, \
			{47.806875,-122.335818}, \
			{47.799740,-122.335704}, \
			{47.799719,-122.346416}, \
			{47.806886,-122.346494}, \
		}\
	} \
	}

void SectorTest::outputSectorTimes(vector<float> & sectorTimes, int lap){
	const bool debug = this->is_debug();
	for (size_t i = 0; i < sectorTimes.size(); i++){
		if (debug) printf("lap %d | sector %zu | %f\r", lap, i + 1, sectorTimes[i]);
	}
}

float SectorTest::sumSectorTimes(vector<float> & sectorTimes){
	float sum = 0;
	for (size_t i = 0; i < sectorTimes.size(); i++){
		sum += sectorTimes[i];
	}
	return sum;
}

void SectorTest::testSectorTimes(){
	const bool debug = this->is_debug();
	if (debug) printf("\rSector Times:\r");
	string log = readFile("predictive_time_test_lap.log");

	std::istringstream iss(log);

	Track *trackCfg = &(getWorkingLoggerConfig()->TrackConfigs.track);

	Track testTrack = Test_Track;
	memcpy(trackCfg, &testTrack, sizeof(Track));

	getWorkingLoggerConfig()->TrackConfigs.auto_detect = 0;

	vector<float> sectorTimes;
	int currentSector = -1;
	int last_lapcount = 0;
	int last_currentlap = 0;
	int lineNo = 0;
	string line;

        if (debug) printf("\r\n");

	while (std::getline(iss, line)) {
           lineNo++;
           vector<string> values = split(line, ',');

           string latitudeRaw = values[5];
           string longitudeRaw = values[6];
           string speedRaw = values[7];
           string timeRaw = values[8];
           timeRaw = "0" + timeRaw;

           if (values[0][0] == '#' || latitudeRaw.size() == 0
               || longitudeRaw.size() == 0 || speedRaw.size() == 0
               || timeRaw.size() == 0) continue;

           if (debug) {
               printf("%d,%d,%d,%f\n",lapstats_current_lap(), getLapCount(), getSector(), getLapDistanceInMiles());
           }

           float lat = atof(latitudeRaw.c_str());
           float lon = atof(longitudeRaw.c_str());
           float speed = atof(speedRaw.c_str());

           const char *utcTimeStr = timeRaw.c_str();
           DateTime dt;
           dt.year = 2014;
           dt.month = 5;
           dt.day = 3;
           dt.hour = (int8_t) atoiOffsetLenSafe(utcTimeStr, 0, 2);
           dt.minute = (int8_t) atoiOffsetLenSafe(utcTimeStr, 2, 2);
           dt.second = (int8_t) atoiOffsetLenSafe(utcTimeStr, 4, 2);
           dt.millisecond = (int16_t) atoiOffsetLenSafe(utcTimeStr, 7, 3);

           GpsSample sample;
           sample.quality = GPS_QUALITY_3D;
           sample.point.latitude = lat;
           sample.point.longitude = lon;
           sample.time = getMillisecondsSinceUnixEpoch(dt);
           sample.speed = speed;
           sample.satellites = 8; //Totally fake.  Shouldn't matter.

           GPS_sample_update(&sample);
           GpsSnapshot snap = getGpsSnapshot();
           lapstats_processUpdate(&snap);

           // Start Work!
           const int sector = getSector();
           const int lap = getLapCount();
           const int currentlap = lapstats_current_lap();


           if (sector != currentSector){
                   if (debug) printf("Sector boundary crossed ( %d -> %d )\r\n",
                                     currentSector, sector);
                   sectorTimes.push_back(getLastSectorTime());
                   currentSector = sector;
           }

           if (currentlap != last_currentlap){
               if (last_currentlap == 0) {
                   //we are calculating distance in advance of the first lap starting
                   CPPUNIT_ASSERT(getLapDistance() > 0);
               }
               last_currentlap = currentlap;
           }

           if (lap != last_lapcount) {
                   const float lastLapTime = getLastLapTime();
                   const float sum = sumSectorTimes(sectorTimes);

                   if (debug) printf("Lap boundary crossed ( %d -> %d ) | "
                                     "Lap time: %f | Sum of sector times: %f"
                                     "\r\n", last_lapcount, lap, lastLapTime, sum);
                   outputSectorTimes(sectorTimes, lap);

                   if (last_lapcount > 0) {
                           CPPUNIT_ASSERT_EQUAL(5, (int) sectorTimes.size());
                           CPPUNIT_ASSERT_CLOSE_ENOUGH(sum, lastLapTime);
                   }

                   sectorTimes.clear();
                   last_lapcount = lap;
           }


           if (debug) printf("%.7f, %.7f, %f | Currently Lap %d, Sector %d | "
                             "Last Lap Time %f, Last Sector Time: (%d) %f\r\n",
                             lat, lon, speed, lap, sector,
                             getLastLapTimeInMinutes(), getLastSector(),
                             getLastSectorTimeInMinutes());

	}

   CPPUNIT_ASSERT_EQUAL(4, last_lapcount);
}

void SectorTest::testStageSectorTimes() {
  const bool debug = this->is_debug();
  const Track track = {
    3333,
    TRACK_TYPE_STAGE,
    {
      {
        {47.806934,-122.341150}, // Start
        {47.806875,-122.335818}, // Finish
        {47.79974,-122.335704},  // Sectors from here down
        {47.799719,-122.346416},
        {47.806886,-122.346494},
      }
    }
  };

  const GeoPoint fakePoint = {1.0, 2.0};

  const GeoPoint points[] = {
    fakePoint,
    {47.806934,-122.341150},
    {47.79974,-122.335704}, // Sector Time = 1
    fakePoint,
    fakePoint,
    {47.799719,-122.346416}, // Sector Time = 3
    fakePoint,
    fakePoint,
    fakePoint,
    fakePoint,
    {47.806886,-122.346494}, // Sector Time = 5
    fakePoint,
    fakePoint,
    fakePoint,
    fakePoint,
    fakePoint,
    fakePoint,
    {47.806875,-122.335818}, // Sector Time = 7
    {0,0},
  };

  // Setup the track.
  LoggerConfig *lc = getWorkingLoggerConfig();
  Track *trackCfg = &(lc->TrackConfigs.track);
  memcpy(trackCfg, &track, sizeof(Track));

  DateTime dt = {
    187, // milis
    0, // sec
    3, // min
    15, // hr
    3, // day
    5, // mon
    14, // yr
  };

  const GeoPoint *gp = points;
  int seconds = 0;
  while(isValidPoint(gp)) {

    dt.second = seconds++;

    // Fake the GPS info.
    GpsSample sample;
    sample.quality = GPS_QUALITY_3D;
    sample.point = *gp;
    sample.time = getMillisecondsSinceUnixEpoch(dt);
    sample.speed = 15.7;
    sample.satellites = 8; //Totally fake.  Shouldn't matter.

    GPS_sample_update(&sample);
    GpsSnapshot snap = getGpsSnapshot();
    lapstats_processUpdate(&snap);

    if (debug) {
    printf("second: %d, atSector = %d, sectorCount = %d, lastSector = %d\n",
           dt.second, getAtSector(), getSector(), getLastSector());
    }

    if (areGeoPointsEqual(*gp, fakePoint)) {
      // Then we should not be at a sector.
      CPPUNIT_ASSERT(!getAtSector());
    } else if (getLastSector() > 0) {
      CPPUNIT_ASSERT(getAtSector());
      const tiny_millis_t expSectorTime = getLastSector() * 2000 + 1000;
      CPPUNIT_ASSERT_EQUAL(expSectorTime, getLastSectorTime());
    }

    ++gp;
  }
}
