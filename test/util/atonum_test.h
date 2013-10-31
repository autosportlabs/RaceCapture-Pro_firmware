#ifndef NUMTOATEST_H
#define NUMTOATEST_H

#include <cppunit/extensions/HelperMacros.h>

class AtoNumTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( AtoNumTest );
  CPPUNIT_TEST( testHexByteConversion );
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testHexByteConversion();
};

#endif  // NUMTOATEST_H
