#include "predictiveTime_test.h"
#include "predictive_timer.h"


// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( PredictiveTimeTest );


void PredictiveTimeTest::setUp()
{
	init_predictive_timer();
}


void PredictiveTimeTest::tearDown()
{
}

void PredictiveTimeTest::testPredictedLapTimeFullLap(){
	//load up a lap
	for (size_t i = 0; i < 20; i++){
		add_predictive_sample(90, 1, 1);
	}
	end_lap();
	for (size_t i = 0; i < 20; i++){
		float predTime = get_predicted_time(100);
		printf("%d: pred: %f\n", i, predTime);
		add_predictive_sample(100, 1, .9);
	}
	float predTime = get_predicted_time(100);
	printf("end pred: %f\n", predTime);
}

void PredictiveTimeTest::testPredictLapTime()
{
	for (size_t i = 0; i < 20; i++){
		add_predictive_sample(90, 1, 1);
	}
	end_lap();
	for (size_t i = 0; i < 10; i++){
		add_predictive_sample(100, 1, .9);
	}
	float predTime = get_predicted_time(100);
	printf("predictedTime: %f\n", predTime);
	}

void PredictiveTimeTest::testAddSamples()
{
	for (size_t i = 0; i < 20; i++){
		add_predictive_sample(100, 1, 2);
	}
	LapBuffer * currentBuffer = get_current_lap_buffer();

	CPPUNIT_ASSERT_EQUAL((size_t)10, currentBuffer->sampleCount);
	CPPUNIT_ASSERT_EQUAL((size_t)2, currentBuffer->sampleInterval);

	//total distance
	CPPUNIT_ASSERT_EQUAL((float)20, currentBuffer->samples[9].distance);
	CPPUNIT_ASSERT_EQUAL((float)40, currentBuffer->samples[9].time);
}
