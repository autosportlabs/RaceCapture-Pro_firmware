#include "loggerData_test.h"
#include "loggerData.h"
#include "loggerApi.h"
#include "mod_string.h"
#include "mock_serial.h"
#include "accelerometer_mock.h"
#include "accelerometer.h"
#include <stdlib.h>
// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( LoggerDataTest );



void LoggerDataTest::setUp()
{
	setupMockSerial();
	updateActiveLoggerConfig();
}


void LoggerDataTest::tearDown()
{
}


void LoggerDataTest::testAccelChannels()
{
//	mock_setAccelValue(0,2048);
//	mock_setAccelValue(1,2048);
//	mock_setAccelValue(1,2866);

	float x = ACCEL_RAW_TO_GFORCE(2866,2048);

}

void LoggerDataTest::testAnalogChannels()
{
	//todo test reading analog channels and scaling functions etc.
}

void LoggerDataTest::testGpsChannels()
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
		float scaled = GetMappedValue(i, &m);
		float expected = (float)i / 100.0f;
		if (i <= 100) expected = 1.0f;
		else if (i >= 500) expected = 5.0f;
		//CPPUNIT_ASSERT(( abs((scaled - expected)) < 0.00001));
	}
}
