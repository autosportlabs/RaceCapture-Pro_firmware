#ifndef NUMTOATEST_H
#define NUMTOATEST_H

#include <cppunit/extensions/HelperMacros.h>

class NumtoaTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( NumtoaTest );
  CPPUNIT_TEST( testConversion );
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testConversion();
};

#endif  // MONEYTEST_H
