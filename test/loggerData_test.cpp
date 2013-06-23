#include "loggerData_test.h"
#include "loggerData.h"
#include "loggerApi.h"
#include "mod_string.h"
#include "mock_serial.h"

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


void LoggerDataTest::testAnalogChannels()
{
	//todo test reading analog channels and scaling functions etc.
}

void LoggerDataTest::testGpsChannels()
{
}
