/*
 * loggerApi_test.cpp
 *
 *  Created on: Jun 10, 2013
 *      Author: brent
 */
#include "loggerApi_test.h"

#include "api.h"
#include "loggerApi.h"
#include "mock_serial.h"

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( LoggerApiTest );


void LoggerApiTest::setUp()
{
	setupMockSerial();
}


void LoggerApiTest::tearDown()
{
}

#define SET_ANALOG_CFG \
		"{" \
		    "\"setAnalogCfg\": {" \
		        "\"0\": {" \
		            "\"nm\": \"analog1\"," \
		            "\"ut\": \"Hz\"," \
		            "\"sr\": 30" \
		        "}" \
		    "}" \
		"}\r\n"

void LoggerApiTest::testSetCellCfg()
{
	char buffer[20000];

	Serial *serial = getMockSerial();

	mock_setBuffer(SET_ANALOG_CFG);

	process_api(serial, buffer, 20000);
}

