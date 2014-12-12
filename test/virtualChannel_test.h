/*
 * loggerData_test.h
 *
 *  Created on: May 1, 2013
 *      Author: brent
 */

#ifndef LOGGERDATA_TEST_H_
#define LOGGERDATA_TEST_H_

#include <cppunit/extensions/HelperMacros.h>

class VirtualChannelTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( VirtualChannelTest);
  CPPUNIT_TEST( testAddChannel );
  CPPUNIT_TEST( testAddDuplicateChannel );
  CPPUNIT_TEST( testAddChannelOverflow );
  CPPUNIT_TEST( testSetChannelValue );
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testAddChannel(void);
  void testAddDuplicateChannel(void);
  void testAddChannelOverflow(void);
  void testSetChannelValue(void);

};

#endif /* LOGGERDATA_TEST_H_ */
