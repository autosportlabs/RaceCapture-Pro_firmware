
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


void AtoNumTest::testHexByteConversion(void)
{
	CPPUNIT_ASSERT(modp_xtoc("00") == 0x00);
	CPPUNIT_ASSERT(modp_xtoc("FF") == 0xFF);
	CPPUNIT_ASSERT(modp_xtoc("AA") == 0xAA);
	CPPUNIT_ASSERT(modp_xtoc("99") == 0x99);
}

void AtoNumTest::testFloatConversion(void){
	{
		const char *test = "1.123456";
		float result = modp_atof(test);
		float expected = 1.123456;
		CPPUNIT_ASSERT_EQUAL(expected, result);
	}

	{
		const char *test = "1.12345678912345612345672322";
		float result = modp_atof(test);
		float expected = 1.123456789;
		CPPUNIT_ASSERT_EQUAL(expected, result);
	}
}

void AtoNumTest::testDoubleConversion(void){
	{
		const char *test = "1.123456";
		double result = modp_atod(test);
		double expected = 1.123456;
		CPPUNIT_ASSERT_EQUAL(expected, result);
	}
	{
		const char *test = "1.12345678912345612345672322";
		double result = modp_atod(test);
		double expected = 1.123456789;
		CPPUNIT_ASSERT_EQUAL(expected, result);
	}
}
