
#include "numtoa_test.h"
#include "modp_numtoa.h"
#include <mod_string.h>
// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( NumtoaTest );


void
NumtoaTest::setUp()
{
}


void
NumtoaTest::tearDown()
{
}


void
NumtoaTest::testDoubleConversion()
{
	char test[100];
	char compare[100];
	int p = 3;

	for (double f = 0; f < 1; f=f+.000001){
		sprintf(compare,"%0.3f",f);
		modp_dtoa(f, test, p);
		int result = strcmp(compare,test);
		//printf("compare %s %s %d\n", test, compare, result);

		CPPUNIT_ASSERT(result == 0);
	}
	for (double f = 0; f < 10000; f=f+.1){
		sprintf(compare,"%0.3f",f);
		modp_dtoa(f, test, p);
		int result = strcmp(compare,test);
		//printf("compare %s %s %d\n", test, compare, result);

		CPPUNIT_ASSERT(result == 0);
	}

}
