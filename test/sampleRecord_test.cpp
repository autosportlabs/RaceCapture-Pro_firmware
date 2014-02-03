#include "sampleRecord_test.h"
#include "loggerConfig.h"
#include "sampleRecord.h"
#include "sampleRecord_test.h"

#include <string>

using std::string;

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( SampleRecordTest );

void SampleRecordTest::setUp()
{
	updateActiveLoggerConfig();
}


void SampleRecordTest::tearDown()
{
}


void SampleRecordTest::testInitSampleRecord()
{

}
