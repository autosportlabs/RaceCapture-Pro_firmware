#ifndef ATONUMTEST_H
#define ATONUMTEST_H

#include <cppunit/extensions/HelperMacros.h>

class AtoNumTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( AtoNumTest );
  CPPUNIT_TEST( testHexByteConversion );
  CPPUNIT_TEST( testDoubleConversion );
  CPPUNIT_TEST( testFloatConversion );
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testHexByteConversion(void);
  void testDoubleConversion(void);
  void testFloatConversion(void);
};

#endif  // ATONUMTEST_H
