/*
 * PredictiveTimeTest2.cpp
 *
 *  Created on: Feb 3, 2014
 *      Author: stieg
 */

#include "PredictiveTimeTest2.h"
#include "dateTime.h"
#include "geopoint.h"
#include "gps.h"
#include "gps.testing.h"
#include "lap_stats.h"
#include "loggerConfig.h"
#include "mock_serial.h"
#include "predictive_timer_2.h"
#include "rcp_cpp_unit.hh"
#include <stdlib.h>
#include <string.h>

#define FILE_PREFIX string("test/")


using std::ifstream;
using std::ios;
using std::istreambuf_iterator;
using std::vector;

CPPUNIT_TEST_SUITE_REGISTRATION( PredictiveTimeTest2 );

PredictiveTimeTest2::PredictiveTimeTest2() {}

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

void PredictiveTimeTest2::setUp() {
        setupMockSerial();
	GPS_init(10, getMockSerial());
	resetPredictiveTimer();
}

void PredictiveTimeTest2::tearDown() {}

vector<string> & PredictiveTimeTest2::split(string &s, char delim, vector<string> &elems) {
    std::stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

vector<string> PredictiveTimeTest2::split(string &s, char delim) {
    vector<string> elems;
    split(s, delim, elems);
    return elems;
}

// And I am totally lazy and just copying because I want this code done.
string PredictiveTimeTest2::readFile(string filename) {
	ifstream t(filename.c_str());
	if (!t.is_open()) {
		t.open(string(FILE_PREFIX + filename).c_str());

	}
	if (!t.is_open()) {
		throw("Can not find file " + filename);
	}
	string str;

	t.seekg(0, ios::end);
	int length = t.tellg();
	str.reserve(length);
	t.seekg(0, ios::beg);

	str.assign((istreambuf_iterator<char>(t)), istreambuf_iterator<char>());
	return str;
}

void PredictiveTimeTest2::testProjectedDistance() {
  GeoPoint s = { .latitude = 2.0, .longitude = 0.0 }; // start
  GeoPoint m = { .latitude = 2.0, .longitude = 1.0 }; // middle
  GeoPoint e = { .latitude = 2.0, .longitude = 2.0 }; // end

  const float expected = 0.5;
  const float actual = distPctBtwnTwoPoints(&s, &e, &m);
  CPPUNIT_ASSERT_CLOSE_ENOUGH(expected, actual);
}

void PredictiveTimeTest2::testPredictedTimeGpsFeed() {
	string log = readFile("predictive_time_test_lap.log");

	std::istringstream iss(log);


	TrackConfig *trackCfg  = &(getWorkingLoggerConfig()->TrackConfigs);
	trackCfg->track.circuit.startFinish.latitude = 47.806934;
	trackCfg->track.circuit.startFinish.longitude = -122.341150;
	trackCfg->radius = 0.0004;

	int lineNo = 0;
	string line;

	while (std::getline(iss, line)) {
          lineNo++;
          vector <string> values = split(line, ',');

          string latitudeRaw = values[5];
          string longitudeRaw = values[6];
          string speedRaw = values[7];
          string timeRaw = values[8];
          timeRaw = "0" + timeRaw;

          if (values[0][0] == '#' || latitudeRaw.size() <= 0
              || longitudeRaw.size() <= 0 || speedRaw.size() <= 0
              || timeRaw.size() <= 0) continue;

          //printf("%s", line.c_str());
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

          const unsigned long millis = getMillisSinceEpoch();
          //printf("---\n");
          //printf("DateTime - YY MM DD HH MM SS.mmm : %02d %02d %02d %02d %02d %02d.%03d\n", dt.year,
          //        dt.month, dt.day, dt.hour, dt.minute, dt.second, dt.millisecond);
          //printf("lat = %f : lon = %f : speed = %f : UTC_Str = \"%s\" : time = %f : millis = %lu\n",
          //      lat, lon, speed, utcTimeStr, utcTime, millis);

          const GeoPoint gp = getGeoPoint();
          const millis_t epochMillis = getMillisSinceEpoch();
          const millis_t predTime = getPredictedTime(&gp, epochMillis);
          //printf("Lap #%d - Predicted Time: %ull\n", getLapCount(), predTime);

	}
}
