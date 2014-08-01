#include "loggerConfig_test.h"
#include "loggerConfig.h"
#include "mod_string.h"

#include <string>

using std::string;

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( LoggerConfigTest );

#define FIELD_LENGTH 200

void LoggerConfigTest::setUp()
{
	initialize_logger_config();
}


void LoggerConfigTest::tearDown()
{
}
