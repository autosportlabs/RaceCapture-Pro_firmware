
#include "gps_test.h"
#include "gps.h"
#include "mod_string.h"

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( GpsTest );


void
GpsTest::setUp()
{
}


void
GpsTest::tearDown()
{
}


void
GpsTest::testChecksum()
{
	const char *goodGpsData = "$GPGLL,5300.97914,N,00259.98174,E,125926,A*28";
	const char *badGpsData = "$GPGLL,5300.97914,N,00259.98174,E,125926,A*29"; //bad checksum

	CPPUNIT_ASSERT(checksumValid(goodGpsData, strlen(goodGpsData)) == 1);
	CPPUNIT_ASSERT(checksumValid(badGpsData, strlen(badGpsData)) == 0);
}
