/*
 * loggerApi_test.h
 *
 *  Created on: Jun 10, 2013
 *      Author: brent
 */

#ifndef LOGGERAPI_TEST_H_
#define LOGGERAPI_TEST_H_

#include <cppunit/extensions/HelperMacros.h>

using std::string;
using std::ifstream;
using std::ios;
using std::istreambuf_iterator;

class LoggerApiTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( LoggerApiTest );
  CPPUNIT_TEST( testSetCellCfg );
  CPPUNIT_TEST( testSetBtCfg );
  CPPUNIT_TEST( testSetAnalogCfg );
  CPPUNIT_TEST( testSetAccelCfg );
  CPPUNIT_TEST( testSetPwmCfg );
  CPPUNIT_TEST( testSetGpioCfg );
  CPPUNIT_TEST( testSetTimerCfg );
  CPPUNIT_TEST( testSampleData );
  CPPUNIT_TEST( testLogStartStop );
  CPPUNIT_TEST( testCalibrateAccel);
  CPPUNIT_TEST_SUITE_END();

public:

  int findAndReplace(string & source, const string find, const string replace);
  string readFile(string filename);
  void setUp();
  void tearDown();

  void assertGenericResponse(char *buffer, const char *messageName, int responseCode);
  void testSampleData();
  void testLogStartStop();
  void testSetCellCfg();
  void testSetBtCfg();
  void testSetAnalogCfg();
  void testSetAccelCfg();
  void testSetPwmCfg();
  void testSetGpioCfg();
  void testSetTimerCfg();
  void testCalibrateAccel();

private:
  void testLogStartStopFile(string filename);
  void testSampleDataFile(string requestFilename, string responseFilename);
  void testAnalogConfigFile(string filename);
  void testAccelConfigFile(string filename);
  void testSetCellConfigFile(string filename);
  void testSetBtConfigFile(string filename);
  void testSetPwmConfigFile(string filename);
  void testSetGpioConfigFile(string filename);
  void testSetTimerConfigFile(string filename);
  void testCalibrateAccelFile(string filename);
};



#endif /* LOGGERAPI_TEST_H_ */
