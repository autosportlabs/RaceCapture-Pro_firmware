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


void LoggerConfigTest::testSetTextField()
{
	{
		char dest[FIELD_LENGTH];
		char test[] =  "simple";
		setTextField(dest,test,FIELD_LENGTH);
		CPPUNIT_ASSERT_EQUAL(string("simple"), string(dest));
	}

	{
		char dest[FIELD_LENGTH];
		char test[] =  "test._1";
		setTextField(dest,test,FIELD_LENGTH);
		CPPUNIT_ASSERT_EQUAL(string("test._1"), string(dest));
	}

	{
		char dest[FIELD_LENGTH];
		char test[] =  "test\n1";
		setTextField(dest,test,FIELD_LENGTH);
		CPPUNIT_ASSERT_EQUAL(string("test_1"), string(dest));
	}

	{
		char dest[FIELD_LENGTH];
		char test[] =  "test!1";
		setTextField(dest,test,FIELD_LENGTH);
		CPPUNIT_ASSERT_EQUAL(string("test_1"), string(dest));
	}
	{
		char dest[FIELD_LENGTH];
		const char test[] =  "_.0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
		setTextField(dest,test,FIELD_LENGTH);
		CPPUNIT_ASSERT_EQUAL(string(test), string(dest));
	}

}
