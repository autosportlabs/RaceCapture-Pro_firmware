/*
 * loggerData_test.h
 *
 *  Created on: May 1, 2013
 *      Author: brent
 */

#ifndef LOGGERDATA_TEST_H_
#define LOGGERDATA_TEST_H_

#include <cppunit/extensions/HelperMacros.h>

class LoggerConfigTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( LoggerConfigTest );
  CPPUNIT_TEST( testSetTextField );
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testSetTextField();
};

#endif /* LOGGERDATA_TEST_H_ */
