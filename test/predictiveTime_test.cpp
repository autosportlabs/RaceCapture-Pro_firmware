#include "predictiveTime_test.h"
#include "predictive_timer.h"
#include <stdlib.h>
#include <fstream>
#include <streambuf>
using std::ifstream;
using std::ios;
using std::istreambuf_iterator;

#define CPPUNIT_ASSERT_CLOSE_ENOUGH(ACTUAL, EXPECTED) CPPUNIT_ASSERT((abs((ACTUAL - EXPECTED)) < 0.00001))

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( PredictiveTimeTest );

#define FILE_PREFIX string("test/")

void PredictiveTimeTest::setUp()
{
	init_predictive_timer();
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

	string line;
	while (std::getline(iss, line))
	{
		vector<string> values = split(line, ',');
		printf("%s\n", values[0].c_str());
	}
}

void PredictiveTimeTest::testPredictedLapTimeFullLap(){
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
