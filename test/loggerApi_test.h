/*
 * loggerApi_test.h
 *
 *  Created on: Jun 10, 2013
 *      Author: brent
 */

#ifndef LOGGERAPI_TEST_H_
#define LOGGERAPI_TEST_H_

#include <cppunit/extensions/HelperMacros.h>

class LoggerApiTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( LoggerApiTest );
  CPPUNIT_TEST( testSetCellCfg );
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testSetCellCfg();
};



#endif /* LOGGERAPI_TEST_H_ */
