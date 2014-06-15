/*
 * loggerApi_test.h
 *
 *  Created on: Jun 10, 2013
 *      Author: brent
 */

#ifndef LOGGERAPI_TEST_H_
#define LOGGERAPI_TEST_H_

#include <cppunit/extensions/HelperMacros.h>
#include "json/reader.h"
#include "json/writer.h"
#include "json/elements.h"

using namespace json;
using std::string;
using std::ifstream;
using std::ios;
using std::istreambuf_iterator;

class LoggerApiTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( LoggerApiTest );
  CPPUNIT_TEST( testUnescapeTextField );
  CPPUNIT_TEST( testSetConnectivityCfg );
  CPPUNIT_TEST( testGetConnectivityCfg );
  CPPUNIT_TEST( testGetAnalogCfg );
  CPPUNIT_TEST( testGetMultipleAnalogCfg );
  CPPUNIT_TEST( testSetAnalogCfg );
  CPPUNIT_TEST( testGetImuCfg );
  CPPUNIT_TEST( testSetImuCfg );
  CPPUNIT_TEST( testGetPwmCfg );
  CPPUNIT_TEST( testSetPwmCfg );
  CPPUNIT_TEST( testGetGpioCfg );
  CPPUNIT_TEST( testSetGpioCfg );
  CPPUNIT_TEST( testGetTimerCfg );
  CPPUNIT_TEST( testSetTimerCfg );
  CPPUNIT_TEST( testSetGpsCfg );
  CPPUNIT_TEST( testGetGpsCfg );
  CPPUNIT_TEST( testSetLapCfg );
  CPPUNIT_TEST( testGetLapCfg );
  CPPUNIT_TEST( testSetTrackCfgCircuit );
  CPPUNIT_TEST( testGetTrackCfgCircuit );
  CPPUNIT_TEST( testGetChannels );
  CPPUNIT_TEST( testAddChannel );
  CPPUNIT_TEST( testAddTrackDb );
  CPPUNIT_TEST( testSampleData );
  CPPUNIT_TEST( testGetMeta );
  CPPUNIT_TEST( testLogStartStop );
  CPPUNIT_TEST( testCalibrateImu);
  CPPUNIT_TEST( testFlashConfig);
  CPPUNIT_TEST( testSetLogLevel);
  CPPUNIT_TEST( testSetObd2Cfg);
  CPPUNIT_TEST( testGetObd2Cfg);
  CPPUNIT_TEST( testGetScript);
  CPPUNIT_TEST( testSetScript);
  CPPUNIT_TEST( testRunScript);
  CPPUNIT_TEST( testGetVersion);
  CPPUNIT_TEST_SUITE_END();

public:

  int findAndReplace(string & source, const string find, const string replace);
  string readFile(string filename);
  void setUp();
  void tearDown();

  void stringToJson(string buffer, Object &json);
  char * processApiGeneric(string filename);

  void assertGenericResponse(char *buffer, const char *messageName, int responseCode);
  void testUnescapeTextField();
  void testSampleData();
  void testGetMeta();
  void testLogStartStop();
  void testSetConnectivityCfg();
  void testGetConnectivityCfg();
  void testGetAnalogCfg();
  void testGetMultipleAnalogCfg();
  void testSetAnalogCfg();
  void testGetImuCfg();
  void testSetImuCfg();
  void testGetPwmCfg();
  void testSetPwmCfg();
  void testGetGpioCfg();
  void testSetGpioCfg();
  void testGetTimerCfg();
  void testSetTimerCfg();
  void testGetGpsCfg();
  void testSetGpsCfg();
  void testSetLapCfg();
  void testGetLapCfg();
  void testSetTrackCfgCircuit();
  void testGetTrackCfgCircuit();
  void testGetChannels();
  void testAddChannel();
  void testAddTrackDb();
  void testCalibrateImu();
  void testFlashConfig();
  void testSetLogLevel();
  void testGetCanCfg();
  void testSetCanCfg();
  void testSetObd2Cfg();
  void testGetObd2Cfg();
  void testSetScript();
  void testGetScript();
  void testRunScript();
  void testGetVersion();

private:
  void testSetScriptFile(string filename);
  void testGetScriptFile(string filename);
  void testRunScriptFile(string filename);
  void testLogStartStopFile(string filename);
  void testSampleDataFile(string requestFilename, string responseFilename);
  void testGetAnalogConfigFile(string filename, int index);
  void testSetAnalogConfigFile(string filename);
  void testSetImuConfigFile(string filename);
  void testGetImuConfigFile(string filename, int index);
  void testSetConnectivityCfgFile(string filename);
  void testGetPwmConfigFile(string filename, int index);
  void testSetPwmConfigFile(string filename);
  void testGetGpioConfigFile(string filename, int index);
  void testSetGpioConfigFile(string filename);
  void testGetTimerConfigFile(string filename, int index);
  void testSetTimerConfigFile(string filename);
  void testGetGpsConfigFile(string filename);
  void testSetGpsConfigFile(string filename, unsigned char channelsEnabled, unsigned short sampleRate);
  void testAddChannelFile(string filename);
  void testGetChannelsFile(string filename);
  void testAddTrackDbFile(string filename);
  void testSetLapConfigFile(string filename);
  void testGetLapConfigFile(string filename);
  void testCalibrateImuFile(string filename);
  void testFlashConfigFile(string filename);
  void testSetLogLevelFile(string filename, int expectedResponse);
  void testGetCanCfgFile(string filename);
  void testSetCanCfgFile(string filename);
  void testGetObd2ConfigFile(string filename);
  void testSetObd2ConfigFile(string filename);

};



#endif /* LOGGERAPI_TEST_H_ */
