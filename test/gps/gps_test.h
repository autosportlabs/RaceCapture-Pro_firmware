#ifndef NUMTOATEST_H
#define NUMTOATEST_H

#include <cppunit/extensions/HelperMacros.h>

class GpsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( GpsTest );
  CPPUNIT_TEST( testChecksum );
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testChecksum();
};

#endif  // NUMTOATEST_H
