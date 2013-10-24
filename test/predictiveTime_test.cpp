#include "predictiveTime_test.h"
#include "predictive_timer.h"
#include "gps.h"
#include "loggerConfig.h"
#include <stdlib.h>
#include <fstream>
#include <streambuf>
#include "mod_string.h"
#include "modp_atonum.h"

using std::ifstream;
using std::ios;
using std::istreambuf_iterator;

#define CPPUNIT_ASSERT_CLOSE_ENOUGH(ACTUAL, EXPECTED) CPPUNIT_ASSERT((abs((ACTUAL - EXPECTED)) < 0.00001))

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( PredictiveTimeTest );

#define FILE_PREFIX string("test/")

void PredictiveTimeTest::setUp()
{
	initGPS();
}


void PredictiveTimeTest::tearDown()
{
}

vector<string> & PredictiveTimeTest::split(string &s, char delim, vector<string> &elems) {
    std::stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

vector<string> PredictiveTimeTest::split(string &s, char delim) {
    vector<string> elems;
    split(s, delim, elems);
    return elems;
}

string PredictiveTimeTest::readFile(string filename){
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


void PredictiveTimeTest::testPredictedTimeGpsFeed(){
	string log = readFile("predictive_time_test_lap.log");

	std::istringstream iss(log);

	GPSTargetConfig *startFinishCfg = &(getWorkingLoggerConfig()->TrackConfigs.startFinishConfig);
	startFinishCfg->latitude = 47.806934;
	startFinishCfg->longitude = -122.341150;
	startFinishCfg->targetRadius = 0.0004;
	setGPSQuality(GPS_QUALITY_DIFFERENTIAL);

	int lineNo = 0;
	string line;
	while (std::getline(iss, line))
	{
		lineNo++;

		vector<string> values = split(line, ',');

		string latitudeRaw = values[5];
		string longitudeRaw = values[6];
		string speedRaw = values[7];
		string timeRaw = values[8];

		timeRaw = "0" + timeRaw;

		if (values[0][0] != '#' && latitudeRaw.size() > 0 && longitudeRaw.size() > 0 && speedRaw.size() > 0 && timeRaw.size() > 0){
			//printf("%s", line.c_str());
			float lat = modp_atof(latitudeRaw.c_str());
			float lon = modp_atof(longitudeRaw.c_str());
			float speed = modp_atof(speedRaw.c_str());
			float utcTime = modp_atof(timeRaw.c_str());

			setGPSSpeed(speed);
			setUTCTime(utcTime);
			updatePosition(lat, lon);
			printf("lat/lon %f %f\n", lat, lon);
			double secondsSinceMidnight = calculateSecondsSinceMidnight(timeRaw.c_str());
			updateSecondsSinceMidnight(secondsSinceMidnight);
			onLocationUpdated();
			float predTime = get_predicted_time(speed);
			printf("%d Lap/PredTime: %d %f\n", lineNo, getLapCount(), predTime);
		}


	}
}

void PredictiveTimeTest::testPredictedLapTimeFullLap(){
	return;
	//load up a lap
	for (size_t i = 0; i < 20; i++){
		add_predictive_sample(90, 1, 1);
	}
	end_lap();
	for (size_t i = 0; i < 20; i++){
		float predTime = get_predicted_time(100);
		CPPUNIT_ASSERT_CLOSE_ENOUGH(predTime, 18);
		add_predictive_sample(100, 1, .9);
	}
	float predTime = get_predicted_time(100);
	CPPUNIT_ASSERT_CLOSE_ENOUGH(predTime, 18);
}

void PredictiveTimeTest::testPredictLapTime()
{
	return;
	for (size_t i = 0; i < 20; i++){
		//add speed, distance, time
		add_predictive_sample(90, 1, 1);
	}
	end_lap();
	for (size_t i = 0; i < 10; i++){
		//add speed, distance, time
		add_predictive_sample(100, 1, .9);
	}
	float predTime = get_predicted_time(100);
	CPPUNIT_ASSERT_CLOSE_ENOUGH(predTime, 18); //predicted time should be 18
}

void PredictiveTimeTest::testAddSamples()
{
	return;
	for (size_t i = 0; i < 20; i++){
		add_predictive_sample(100, 1, 2);
	}
	LapBuffer * currentBuffer = get_current_lap_buffer();

	for (size_t i = 0; i < 10; i++){
		LocationSample *s = &currentBuffer->samples[i];
	}

	CPPUNIT_ASSERT_EQUAL((size_t)9, currentBuffer->sampleIndex);
	CPPUNIT_ASSERT_EQUAL((size_t)2, currentBuffer->sampleInterval);

	//total distance
	CPPUNIT_ASSERT_EQUAL((float)20, currentBuffer->samples[9].distance);
	CPPUNIT_ASSERT_EQUAL((float)40, currentBuffer->samples[9].time);
}
