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
