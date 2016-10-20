/*
 * Race Capture Firmware
 *
 * Copyright (C) 2016 Autosport Labs
 *
 * This file is part of the Race Capture firmware suite
 *
 * This is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should
 * have received a copy of the GNU General Public License along with
 * this code. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LOGGERAPI_TEST_H_
#define LOGGERAPI_TEST_H_

#include "channel_config.h"
#include "json/elements.h"
#include "json/reader.h"
#include "json/writer.h"
#include "loggerConfig.h"
#include <cppunit/extensions/HelperMacros.h>
#include <stdbool.h>

using namespace json;
using std::string;
using std::ifstream;
using std::ios;
using std::istreambuf_iterator;

class LoggerApiTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( LoggerApiTest );
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
    CPPUNIT_TEST( testAddTrackDb );
    CPPUNIT_TEST( testGetTrackDb );
    CPPUNIT_TEST( testSampleData1 );
    CPPUNIT_TEST( testSampleData2 );
    CPPUNIT_TEST( testHeartBeat );
    CPPUNIT_TEST( testGetMeta );
    CPPUNIT_TEST( testLogStartStop );
    CPPUNIT_TEST( testCalibrateImu);
    CPPUNIT_TEST( testFlashConfig);
    CPPUNIT_TEST( testSetLogLevel);
    CPPUNIT_TEST( testSetObd2Cfg);
    CPPUNIT_TEST( testSetObd2ConfigFile_fromIndex);
    CPPUNIT_TEST( testSetObd2ConfigFile_invalid);
    CPPUNIT_TEST( testGetObd2Cfg);
    CPPUNIT_TEST( testGetScript);
    CPPUNIT_TEST( testSetScript);
    CPPUNIT_TEST( testRunScript);
    CPPUNIT_TEST( testGetVersion);
    CPPUNIT_TEST( testGetStatus);
    CPPUNIT_TEST( testGetCapabilities);
    CPPUNIT_TEST( testSetWifiCfg );
    CPPUNIT_TEST( testSetWifiCfgApBadChannel );
    CPPUNIT_TEST( testSetWifiCfgApBadEncryption );
    CPPUNIT_TEST( testGetWifiCfgDefault );
    CPPUNIT_TEST( testSetGetWifiCfg );
    CPPUNIT_TEST( setActiveTrack );
    CPPUNIT_TEST( setActiveTrackSectors );
    CPPUNIT_TEST( setActiveTrackInvalid );
    CPPUNIT_TEST( setActiveTrackRadiusMeters );
    CPPUNIT_TEST( setActiveTrackRadiusDegrees );
    CPPUNIT_TEST( testGetAutoLoggerCfgDefault );
    CPPUNIT_TEST( testSetAutoLoggerCfg );

    CPPUNIT_TEST_SUITE_END();

public:

    int findAndReplace(string & source, const string find, const string replace);
    string readFile(string filename);
    void setUp();

    void setActiveTrack();
    void setActiveTrackSectors();
    void setActiveTrackInvalid();
    void setActiveTrackRadiusMeters();
    void setActiveTrackRadiusDegrees();

    void stringToJson(const char* buffer, Object &json);
    char * processApiGeneric(string filename);

    void assertGenericResponse(char *buffer, const char *messageName, int responseCode);
    void testSampleData1();
    void testSampleData2();
    void testHeartBeat();
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
    void testAddTrackDb();
    void testGetTrackDb();
    void testCalibrateImu();
    void testFlashConfig();
    void testSetLogLevel();
    void testGetCanCfg();
    void testSetCanCfg();
    void testSetObd2Cfg();
    void testSetObd2ConfigFile_fromIndex();
    void testSetObd2ConfigFile_invalid();
    void testGetObd2Cfg();
    void testSetScript();
    void testGetScript();
    void testRunScript();
    void testGetVersion();
    void testGetStatus();
    void testGetCapabilities();
    void testSetWifiCfg();
    void testSetWifiCfgApBadChannel();
    void testSetWifiCfgApBadEncryption();
    void testGetWifiCfgDefault();
    void testSetGetWifiCfg();
    void testGetAutoLoggerCfgDefault();
    void testSetAutoLoggerCfg();

private:
    void testSetScriptFile(string filename);
    void testGetScriptFile(string filename);
    void testRunScriptFile(string filename);
    void testLogStartStopFile(string filename);
    string getSampleResponse(string requestJson);
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
    void testSetGpsConfigFile(string filename,
			      unsigned char channelsEnabled,
			      unsigned short sampleRate,
			      const bool metric);
    void testAddTrackDbFile(string filename);
    void testGetTrackDbFile(string filename, string addedFilename);
    void testSetLapConfigFile(string filename);
    void testGetLapConfigFile(string filename);
    void testCalibrateImuFile(string filename);
    void testFlashConfigFile(string filename);
    void testSetLogLevelFile(string filename, int expectedResponse);
    void testGetCanCfgFile(string filename);
    void testSetCanCfgFile(string filename);
    void testGetObd2ConfigFile(string filename);
    void testSetObd2ConfigFile(string filename);
    void populateChannelConfig(ChannelConfig *cfg, const int id, const int splRt);
    void checkChannelConfig(Object &json, const int i, string &iString, const int splRt);
    void testChannelConfig(ChannelConfig *chCfg, string expNm, string expUt, unsigned short sr);
};



#endif /* LOGGERAPI_TEST_H_ */
