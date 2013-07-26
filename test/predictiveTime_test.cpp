#include "predictiveTime_test.h"
#include "predictive_timer.h"


// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( PredictiveTimeTest );


void PredictiveTimeTest::setUp()
{
	init_timer();
}


void PredictiveTimeTest::tearDown()
{
}


void PredictiveTimeTest::testAddSample()
{
	for (size_t i = 0; i < 20; i++){
		add_sample(100, 1);
	}
	LapBuffer * currentBuffer = get_current_lap_buffer();

	CPPUNIT_ASSERT_EQUAL((size_t)10, currentBuffer->sampleCount);
	CPPUNIT_ASSERT_EQUAL((size_t)2, currentBuffer->sampleInterval);

	size_t totalDistance = 0;
	for (size_t i = 0; i < 10; i++){
		totalDistance += currentBuffer->samples[i].distance;
	}
	CPPUNIT_ASSERT_EQUAL((size_t)20, totalDistance);
}
