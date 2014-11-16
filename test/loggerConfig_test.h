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
  CPPUNIT_TEST( testLoggerInitVersionInfo );
  CPPUNIT_TEST( testLoggerInitPwmClock );
  CPPUNIT_TEST( testLoggerInitTimeConfig );
  CPPUNIT_TEST( testLoggerInitAdcConfig );
  CPPUNIT_TEST( testLoggerInitPwmConfig );
  CPPUNIT_TEST( testLoggerInitGpioConfig );
  CPPUNIT_TEST( testLoggerInitTimerConfig );
  CPPUNIT_TEST( testLoggerInitImuConfig );
  CPPUNIT_TEST( testLoggerInitObd2Config );
  CPPUNIT_TEST( testLoggerInitGpsConfig );
  CPPUNIT_TEST( testLoggerInitLapConfig );
  CPPUNIT_TEST( testLoggerInitConnectivityConfig );
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();
  void testLoggerInitVersionInfo();
  void testLoggerInitPwmClock();
  void testLoggerInitTimeConfig();
  void testLoggerInitAdcConfig();
  void testLoggerInitPwmConfig();
  void testLoggerInitGpioConfig();
  void testLoggerInitTimerConfig();
  void testLoggerInitImuConfig();
  void testLoggerInitObd2Config();
  void testLoggerInitGpsConfig();
  void testLoggerInitLapConfig();
  void testLoggerInitConnectivityConfig();
};

#endif /* LOGGERDATA_TEST_H_ */
