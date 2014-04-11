/*
 * PredictiveTimeTest2.cpp
 *
 *  Created on: Feb 3, 2014
 *      Author: stieg
 */

#include "PredictiveTimeTest2.h"

#include <stdlib.h>

#include "geopoint.h"
#include "gps.h"
#include "modp_atonum.h"
#include "predictive_timer_2.h"
#include "loggerConfig.h"

#define FILE_PREFIX string("test/")

using std::ifstream;
using std::ios;
using std::istreambuf_iterator;
using std::vector;

CPPUNIT_TEST_SUITE_REGISTRATION( PredictiveTimeTest2 );

PredictiveTimeTest2::PredictiveTimeTest2() {}

void PredictiveTimeTest2::setUp() {
	initGPS();
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

void PredictiveTimeTest2::testPredictedTimeGpsFeed() {
	string log = readFile("predictive_time_test_lap.log");

	std::istringstream iss(log);

	Track *trackCfg  = 	&(getWorkingLoggerConfig()->TrackConfigs.track);
	trackCfg->startFinish.latitude = 47.806934;
	trackCfg->startFinish.longitude = -122.341150;
	trackCfg->radius = 0.0004;
	setGPSQuality (GPS_QUALITY_DIFFERENTIAL);

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

		if (values[0][0] != '#' && latitudeRaw.size() > 0
				&& longitudeRaw.size() > 0 && speedRaw.size() > 0
				&& timeRaw.size() > 0) {
			//printf("%s", line.c_str());
			float lat = modp_atof(latitudeRaw.c_str());
			float lon = modp_atof(longitudeRaw.c_str());
			float speed = modp_atof(speedRaw.c_str());
			float utcTime = modp_atof(timeRaw.c_str());

			setGPSSpeed(speed);
			setUTCTime(utcTime);
			updatePosition(lat, lon);
			double secondsSinceMidnight = calculateSecondsSinceMidnight(
					timeRaw.c_str());
			updateSecondsSinceMidnight(secondsSinceMidnight);
			onLocationUpdated();

			GeoPoint gp;
			populateGeoPoint(&gp);
			float predTime = getPredictedTime(gp, utcTime);

			printf("Lap:%d- pred: %f - last lap: %f\n", getLapCount(), predTime, getLastLapTime());
		}
	}
}
