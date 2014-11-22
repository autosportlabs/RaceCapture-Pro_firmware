#include "loggerData_test.h"
#include "loggerSampleData.test.h"
#include "loggerApi.h"
#include "mod_string.h"
#include "mock_serial.h"
#include "imu_mock.h"
#include "imu.h"
#include <stdlib.h>
// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( LoggerDataTest );



void LoggerDataTest::setUp()
{
	setupMockSerial();
	initialize_logger_config();
}


void LoggerDataTest::tearDown()
{
}


void LoggerDataTest::testMappedValue()
{
	ScalingMap m;
	m.rawValues[0] = 100;
	m.rawValues[1] = 200;
	m.rawValues[2] = 300;
	m.rawValues[3] = 400;
	m.rawValues[4] = 500;

	m.scaledValues[0] = 1.0f;
	m.scaledValues[1] = 2.0f;
	m.scaledValues[2] = 3.0f;
	m.scaledValues[3] = 4.0f;
	m.scaledValues[4] = 5.0f;

	for (int i = 0; i < 1024; i++){
		float scaled = get_mapped_value(i, &m);
		float expected = (float)i / 100.0f;
		if (i <= 100) expected = 1.0f;
		else if (i >= 500) expected = 5.0f;
		CPPUNIT_ASSERT(( abs((scaled - expected)) < 0.00001));
	}
}
