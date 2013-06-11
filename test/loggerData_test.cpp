#include "loggerData_test.h"
#include "loggerData.h"
#include "loggerApi.h"
#include "mod_string.h"
#include "mock_serial.h"

extern "C"{
#include "api.h"
}
// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( LoggerDataTest );



void LoggerDataTest::setUp()
{
	setupMockSerial();
}


void LoggerDataTest::tearDown()
{
}


void LoggerDataTest::testConfigAnalogChannels()
{
	Serial *serial = getMockSerial();

	char json[] = "{\"blah\":[]}\r\n";
	mock_setBuffer(json);

	unsigned int len = 3;

	process_api(serial,json, strlen(json));
}
