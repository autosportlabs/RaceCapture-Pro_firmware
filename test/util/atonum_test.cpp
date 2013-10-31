
#include "atonum_test.h"
#include "modp_atonum.h"

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( AtoNumTest );


void
AtoNumTest::setUp()
{
}


void
AtoNumTest::tearDown()
{
}


void
AtoNumTest::testHexByteConversion()
{
	CPPUNIT_ASSERT(modp_xtoc("00") == 0x00);
	CPPUNIT_ASSERT(modp_xtoc("FF") == 0xFF);
	CPPUNIT_ASSERT(modp_xtoc("AA") == 0xAA);
	CPPUNIT_ASSERT(modp_xtoc("99") == 0x99);
}
