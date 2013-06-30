#include "loggerData_test.h"
#include "loggerData.h"
#include "loggerApi.h"
#include "mod_string.h"
#include "mock_serial.h"
#include "accelerometer_mock.h"
#include "accelerometer.h"

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
