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

#include "loggerApi_test.h"
#include "FreeRTOS.h"
#include "constants.h"
#include "api.h"
#include "loggerApi.h"
#include "mock_serial.h"
#include "imu.h"
#include "cpu.h"
#include "loggerConfig.h"
#include "jsmn.h"
#include "mod_string.h"
#include "modp_atonum.h"
#include "memory_mock.h"
#include "printk.h"
#include <string>
#include <fstream>
#include <streambuf>
#include "predictive_timer_2.h"
#include "luaScript.h"
#include "rcp_cpp_unit.hh"
#include "cellModem.h"
#include "sim900.h"
#include "bluetooth.h"
#include "logger.h"
#include "lap_stats.h"
#include "launch_control.h"
#include "task.h"
#include "task_testing.h"

#define JSON_TOKENS 10000
#define FILE_PREFIX string("json_api_files/")

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( LoggerApiTest );


char * LoggerApiTest::processApiGeneric(string filename){
	Serial *serial = getMockSerial();
	string json = readFile(filename);
	mock_resetTxBuffer();
	process_api(getMockSerial(),(char *)json.c_str(), json.size());
	char *txBuffer = mock_getTxBuffer();
	return txBuffer;
}


void LoggerApiTest::stringToJson(string buffer, Object &json){
	std::stringstream stream;
	for (size_t i = 0; i < buffer.size(); i++){
		stream.put(buffer[i]);
	}
	try{
			Reader::Read(json, stream);
	}
	catch (json::Exception &e){
			throw ("Could not parse json string");
	}
}

void LoggerApiTest::assertGenericResponse(char *buffer, const char * messageName, int responseCode){
	static jsmn_parser parser;
	static jsmntok_t json_tok[JSON_TOKENS];
	jsmn_init(&parser);
	int r = jsmn_parse(&parser, buffer, json_tok, JSON_TOKENS);
	if (r == JSMN_SUCCESS){
		if (json_tok->type == JSMN_OBJECT && json_tok->size == 2){
			jsmntok_t * tok_msgName = json_tok + 1;
			jsmn_trimData(tok_msgName);
			if (strcmp(tok_msgName->data, messageName) != 0){
				CPPUNIT_FAIL("assertGenericResponse: message name does not match");
			}
			jsmntok_t * tok_resultInfo = json_tok + 2;
			if (tok_resultInfo->type== JSMN_OBJECT && tok_resultInfo->size == 2){
				jsmntok_t *tok_rc = tok_resultInfo + 1;
				jsmntok_t *tok_rcVal = tok_resultInfo + 2;
				jsmn_trimData(tok_rc);
				jsmn_trimData(tok_rcVal);
				if (strcmp("rc", tok_rc->data) != 0){
					CPPUNIT_FAIL("assertGenericResponse: rc element name does not match");
				}

                                const int actual_response_code = modp_atoi(tok_rcVal->data);
                                if (responseCode != actual_response_code) {
                                        char buff[256];
                                        sprintf(buff, "Msg \"%s\" failed.  Expected %d, got %d",
                                                messageName, responseCode, actual_response_code);
                                        CPPUNIT_FAIL(buff);

                                }
			}
			else{
				CPPUNIT_FAIL("assertGenericResponse: did not see expected result object");
			}
		}
		else{
			CPPUNIT_FAIL("assertGenericResponse: expected object at root level");
		}
	}
	else{
		CPPUNIT_FAIL("assertGenericResponse: failed to parse");
	}
}

int LoggerApiTest::findAndReplace(string & source, const string find, const string replace)
{
    int num=0;
    string::size_type fLen = find.size();
    string::size_type rLen = replace.size();
    for (string::size_type pos=0; (pos=source.find(find, pos))!=string::npos; pos+=rLen)
    {
        num++;
        source.replace(pos, fLen, replace);
    }
    return num;
}

string LoggerApiTest::readFile(string filename){
	ifstream t(filename.c_str());
	if (!t.is_open()){
		t.open(string(FILE_PREFIX + filename).c_str());

	}
	if (!t.is_open()){
		throw ("Can not find file " + filename);
	}
	string str;

	t.seekg(0, ios::end);
	int length = t.tellg();
	str.reserve(length);
	t.seekg(0, ios::beg);

	str.assign((istreambuf_iterator<char>(t)),
				istreambuf_iterator<char>());

	findAndReplace(str,"\r","");
	findAndReplace(str,string("\n"),string(""));
	str.append("\r\n");
	return str;
}

void LoggerApiTest::setUp()
{
	LoggerConfig *config = getWorkingLoggerConfig();
	initApi();
	initialize_logger_config();
	setupMockSerial();
	imu_init(config);
	resetPredictiveTimer();
        lapStats_init();
}


void LoggerApiTest::tearDown()
{
}

void LoggerApiTest::testUnescapeTextField(){
	{
		char test[] =  "test1";
		unescapeTextField(test);
		CPPUNIT_ASSERT_EQUAL(string("test1"), string(test));
	}

	{
		char test[] =  "test\\n1";
		unescapeTextField(test);
		CPPUNIT_ASSERT_EQUAL(string("test\n1"), string(test));
	}
	{
		char test[] =  "test\\r1";
		unescapeTextField(test);
		CPPUNIT_ASSERT_EQUAL(string("test\r1"), string(test));
	}
	{
		char test[] =  "test\\\\1";
		unescapeTextField(test);
		CPPUNIT_ASSERT_EQUAL(string("test\\1"), string(test));
	}
	{
		char test[] =  "test\\\"1";
		unescapeTextField(test);
		CPPUNIT_ASSERT_EQUAL(string("test\"1"), string(test));
	}

}

void LoggerApiTest::populateChannelConfig(ChannelConfig *cfg, const int i, const int splRt) {
   sprintf(cfg->label, "testName_%d", i);
   sprintf(cfg->units, "unit_%d", i);
   cfg->min = 0.1 + i;
   cfg->max = 100.1 + i;
   cfg->sampleRate = encodeSampleRate(splRt);
   cfg->precision = (unsigned char) i + 1;
}

void LoggerApiTest::checkChannelConfig(Object &json, const int i, string &iString, const int splRt) {
   string expNmStr = string("testName_") + iString;
   CPPUNIT_ASSERT_EQUAL(expNmStr, string((String)json["nm"]));

   string expUtStr = string("unit_") + iString;
   CPPUNIT_ASSERT_EQUAL(expUtStr, string((String)json["ut"]));

   CPPUNIT_ASSERT_EQUAL(0.1f + i, (float)(Number)json["min"]);
   CPPUNIT_ASSERT_EQUAL(100.1f + i, (float)(Number)json["max"]);

   CPPUNIT_ASSERT_EQUAL(i + 1, (int)(Number)json["prec"]);
   CPPUNIT_ASSERT_EQUAL(splRt, (int)(Number)json["sr"]);
}

void LoggerApiTest::testGetMultipleAnalogCfg(){
   LoggerConfig *c = getWorkingLoggerConfig();

   for (int i = 0; i < CONFIG_ADC_CHANNELS; i++){
      ADCConfig *analogCfg = &c->ADCConfigs[i];

      populateChannelConfig(&(analogCfg->cfg), i, 100);

      analogCfg->linearScaling = 3.21F + i;
      analogCfg->linearOffset = 9.0F + i;
      analogCfg->scalingMode = i;
      analogCfg->calibration = 1.0F + i;

      for (int x = 0; x < ANALOG_SCALING_BINS; x++){
         analogCfg->scalingMap.rawValues[x] = i * x;
      }

      for (int x = 0; x < ANALOG_SCALING_BINS; x++){
         analogCfg->scalingMap.scaledValues[x] = float(i * x) + 0.1F;
      }
   }

   char * response = processApiGeneric("getAnalogCfg3.json");

   Object json;
   stringToJson(response, json);

   for (int i = 0; i < CONFIG_ADC_CHANNELS; i++){
      std::ostringstream stringStream;
      stringStream << i;
      string iString = stringStream.str();

      Object &analogJson = json["analogCfg"][iString];

      checkChannelConfig(analogJson, i, iString, 100);

      CPPUNIT_ASSERT_EQUAL(3.21F + i, (float)(Number)analogJson["scaling"]);
      CPPUNIT_ASSERT_EQUAL(9.0F + i, (float)(Number)analogJson["offset"]);
      CPPUNIT_ASSERT_EQUAL(i, (int)(Number)analogJson["scalMod"]);
      CPPUNIT_ASSERT_EQUAL(1.0F + i, (float)(Number)analogJson["cal"] );

      Object scalMap = (Object)analogJson["map"];
      Array raw = (Array)scalMap["raw"];
      Array scal = (Array)scalMap["scal"];

      for (int x = 0; x < ANALOG_SCALING_BINS; x++){
         CPPUNIT_ASSERT_EQUAL(i * x, (int)(Number)raw[x]);
      }
      for (int x = 0; x < ANALOG_SCALING_BINS; x++){
         CPPUNIT_ASSERT_EQUAL((float)(i * x) + 0.1F, (float)(Number)scal[x]);
      }
   }
}

void LoggerApiTest::testGetAnalogConfigFile(string filename, int index){
	std::ostringstream stringStream;
	stringStream << index;
        string iStr = stringStream.str();

	LoggerConfig *c = getWorkingLoggerConfig();
	ADCConfig *analogCfg = &c->ADCConfigs[index];

        populateChannelConfig(&(analogCfg->cfg), index, 50);

	analogCfg->linearScaling = 1.234F;
	analogCfg->linearOffset = 9.9F;
	analogCfg->scalingMode = 2;
	analogCfg->filterAlpha = 0.6F;
	analogCfg->calibration = 1.01F;

	int i = 0;
	for (int x = 0; x < ANALOG_SCALING_BINS; i+=10,x++){
		analogCfg->scalingMap.rawValues[x] = i;
	}
	float iv = 0;
	for (int x = 0; x < ANALOG_SCALING_BINS; iv+=1.1,x++){
		analogCfg->scalingMap.scaledValues[x] = iv;
	}
	char * response = processApiGeneric(filename);

	Object json;
	stringToJson(response, json);

	Object &analogJson = json["analogCfg"][iStr];

        checkChannelConfig(analogJson, index, iStr, 50);

	CPPUNIT_ASSERT_EQUAL(1.234F, (float)(Number)analogJson["scaling"]);
	CPPUNIT_ASSERT_EQUAL(9.9F, (float)(Number)analogJson["offset"]);
	CPPUNIT_ASSERT_EQUAL(0.6F, (float)(Number)analogJson["alpha"]);
	CPPUNIT_ASSERT_EQUAL(2, (int)(Number)analogJson["scalMod"]);
	CPPUNIT_ASSERT_EQUAL(1.01F, (float)(Number)analogJson["cal"]);

	Object scalMap = (Object)analogJson["map"];
	Array raw = (Array)scalMap["raw"];
	Array scal = (Array)scalMap["scal"];

	i = 0;
	for (int x = 0; x < ANALOG_SCALING_BINS; i+=10, x++){
		CPPUNIT_ASSERT_EQUAL(i, (int)(Number)raw[x]);
	}
	iv = 0;
	for (int x = 0; x < ANALOG_SCALING_BINS; iv+=1.1, x++){
		CPPUNIT_ASSERT_EQUAL(iv, (float)(Number)scal[x]);
	}
}

void LoggerApiTest::testGetAnalogCfg(){
	testGetAnalogConfigFile("getAnalogCfg1.json", 1 );
	testGetAnalogConfigFile("getAnalogCfg2.json", 0 );

}

void LoggerApiTest::testSetAnalogConfigFile(string filename){

	Serial *serial = getMockSerial();
	string json = readFile(filename);
	mock_resetTxBuffer();
	process_api(getMockSerial(),(char *)json.c_str(), json.size());

	LoggerConfig *c = getWorkingLoggerConfig();

	ADCConfig *adcCfg = &c->ADCConfigs[0];

	ChannelConfig *cfg = &adcCfg->cfg;
	CPPUNIT_ASSERT_EQUAL(string("I <3 Racing"), string(cfg->label));
	CPPUNIT_ASSERT_EQUAL(string("Wheels"), string(cfg->units));
	CPPUNIT_ASSERT_EQUAL(-1.0f, cfg->min);
	CPPUNIT_ASSERT_EQUAL(1.0f, cfg->max);
	CPPUNIT_ASSERT_EQUAL(50, decodeSampleRate(cfg->sampleRate));
	CPPUNIT_ASSERT_EQUAL(1, (int)cfg->precision);

	CPPUNIT_ASSERT_EQUAL(2, (int)adcCfg->scalingMode);
	CPPUNIT_ASSERT_EQUAL(1.234F, adcCfg->linearScaling);
	CPPUNIT_ASSERT_EQUAL(9.9F, adcCfg->linearOffset);
	CPPUNIT_ASSERT_EQUAL(0.6F, adcCfg->filterAlpha);
	CPPUNIT_ASSERT_EQUAL(1.01F, adcCfg->calibration);

	CPPUNIT_ASSERT_EQUAL(0.0F, adcCfg->scalingMap.rawValues[0]);
	CPPUNIT_ASSERT_EQUAL(1.25F, adcCfg->scalingMap.rawValues[1]);
	CPPUNIT_ASSERT_EQUAL(2.5F, adcCfg->scalingMap.rawValues[2]);
	CPPUNIT_ASSERT_EQUAL(3.75F, adcCfg->scalingMap.rawValues[3]);
	CPPUNIT_ASSERT_EQUAL(5.0F, adcCfg->scalingMap.rawValues[4]);

	CPPUNIT_ASSERT_EQUAL(1.1F, adcCfg->scalingMap.scaledValues[0]);
	CPPUNIT_ASSERT_EQUAL(1.2F, adcCfg->scalingMap.scaledValues[1]);
	CPPUNIT_ASSERT_EQUAL(1.3F, adcCfg->scalingMap.scaledValues[2]);
	CPPUNIT_ASSERT_EQUAL(1.4F, adcCfg->scalingMap.scaledValues[3]);
	CPPUNIT_ASSERT_EQUAL(1.5F, adcCfg->scalingMap.scaledValues[4]);

	char *txBuffer = mock_getTxBuffer();
	assertGenericResponse(txBuffer, "setAnalogCfg", API_SUCCESS);

}

void LoggerApiTest::testSetAnalogCfg()
{
	testSetAnalogConfigFile("setAnalogCfg1.json");
	testSetAnalogConfigFile("setAnalogCfg2.json");
	testSetAnalogConfigFile("setAnalogCfg3.json");
}

void LoggerApiTest::testGetImuConfigFile(string filename, int index){
	LoggerConfig *c = getWorkingLoggerConfig();
	ImuConfig *imuCfg = &c->ImuConfigs[index];

        populateChannelConfig(&imuCfg->cfg, index, 100);

	imuCfg->mode = 1;
	imuCfg->physicalChannel = 3;
	imuCfg->zeroValue = 1234;
	imuCfg->filterAlpha = 0.7F;

	char * response = processApiGeneric(filename);

	Object json;
	stringToJson(response, json);

	std::ostringstream stringStream;
	stringStream << index;
        string iStr = stringStream.str();

	Object &imuJson = json["imuCfg"][iStr];

        checkChannelConfig(imuJson, index, iStr, 100);

	CPPUNIT_ASSERT_EQUAL(1, (int)(Number)imuJson["mode"]);
	CPPUNIT_ASSERT_EQUAL(3, (int)(Number)imuJson["chan"]);
	CPPUNIT_ASSERT_EQUAL(1234, (int)(Number)imuJson["zeroVal"]);
	CPPUNIT_ASSERT_EQUAL(0.7F, (float)(Number)imuJson["alpha"]);
}

void LoggerApiTest::testGetImuCfg(){
	testGetImuConfigFile("getImuCfg1.json", API_SUCCESS);
}

void LoggerApiTest::testSetImuConfigFile(string filename){
	Serial *serial = getMockSerial();
	string json = readFile(filename);
	mock_resetTxBuffer();
	process_api(getMockSerial(),(char *)json.c_str(), json.size());

	LoggerConfig *c = getWorkingLoggerConfig();

	ImuConfig *imuCfg = &c->ImuConfigs[0];

        ChannelConfig *cfg = &imuCfg->cfg;
        CPPUNIT_ASSERT_EQUAL(string("I <3 Inerta"), string(cfg->label));
        CPPUNIT_ASSERT_EQUAL(string("Forces"), string(cfg->units));
        CPPUNIT_ASSERT_EQUAL(-1.0f, cfg->min);
        CPPUNIT_ASSERT_EQUAL(1.0f, cfg->max);
	CPPUNIT_ASSERT_EQUAL(50, decodeSampleRate(cfg->sampleRate));
	CPPUNIT_ASSERT_EQUAL(1, (int)cfg->precision);

	CPPUNIT_ASSERT_EQUAL(1, (int)imuCfg->mode);
	CPPUNIT_ASSERT_EQUAL(2, (int)imuCfg->physicalChannel);
	CPPUNIT_ASSERT_EQUAL(1234, (int)imuCfg->zeroValue);
	CPPUNIT_ASSERT_EQUAL(0.7F, imuCfg->filterAlpha);

	char *txBuffer = mock_getTxBuffer();
	assertGenericResponse(txBuffer, "setImuCfg", API_SUCCESS);
}

void LoggerApiTest::testSetImuCfg(){
	testSetImuConfigFile("setImuCfg1.json");
}

void LoggerApiTest::testSetConnectivityCfgFile(string filename){
	LoggerConfig *c = getWorkingLoggerConfig();
	ConnectivityConfig *connCfg = &c->ConnectivityConfigs;

	processApiGeneric(filename);
	char *txBuffer = mock_getTxBuffer();
	assertGenericResponse(txBuffer, "setConnCfg", API_SUCCESS);

	CPPUNIT_ASSERT_EQUAL(1, (int)connCfg->cellularConfig.cellEnabled);
	CPPUNIT_ASSERT_EQUAL(string("foo.xyz"), string(connCfg->cellularConfig.apnHost));
	CPPUNIT_ASSERT_EQUAL(string("blarg"), string(connCfg->cellularConfig.apnUser));
	CPPUNIT_ASSERT_EQUAL(string("blorg"), string(connCfg->cellularConfig.apnPass));

	CPPUNIT_ASSERT_EQUAL(1, (int)connCfg->bluetoothConfig.btEnabled);
	CPPUNIT_ASSERT_EQUAL(string("myRacecar"), string(connCfg->bluetoothConfig.new_name));
	CPPUNIT_ASSERT_EQUAL(string("3311"), string(connCfg->bluetoothConfig.new_pin));

	CPPUNIT_ASSERT_EQUAL(1, (int)connCfg->telemetryConfig.backgroundStreaming);
	CPPUNIT_ASSERT_EQUAL(string("xyz123"), string(connCfg->telemetryConfig.telemetryDeviceId));
	CPPUNIT_ASSERT_EQUAL(string("a.b.c"), string(connCfg->telemetryConfig.telemetryServerHost));
}

void LoggerApiTest::testSetConnectivityCfg(){
	testSetConnectivityCfgFile("setConnCfg1.json");
}

void LoggerApiTest::testGetConnectivityCfg(){
	LoggerConfig *c = getWorkingLoggerConfig();
	ConnectivityConfig *connCfg = &c->ConnectivityConfigs;

	strcpy(connCfg->bluetoothConfig.new_name, "btDevName");
	strcpy(connCfg->bluetoothConfig.new_pin, "6543");

	strcpy(connCfg->cellularConfig.apnHost, "apnHost");
	strcpy(connCfg->cellularConfig.apnUser, "apnUser");
	strcpy(connCfg->cellularConfig.apnPass, "apnPass");

	char *response = processApiGeneric("getConnCfg1.json");
	Object json;
	stringToJson(response, json);

	Object &connJson = json["connCfg"];

	CPPUNIT_ASSERT_EQUAL((int)(connCfg->bluetoothConfig.btEnabled), (int)(Number)connJson["btCfg"]["btEn"]);
	CPPUNIT_ASSERT_EQUAL(string(connCfg->bluetoothConfig.new_name), string((String)connJson["btCfg"]["name"]));
	CPPUNIT_ASSERT_EQUAL(string(connCfg->bluetoothConfig.new_pin), string((String)connJson["btCfg"]["pass"]));

	CPPUNIT_ASSERT_EQUAL((int)(connCfg->cellularConfig.cellEnabled), (int)(Number)connJson["cellCfg"]["cellEn"]);
	CPPUNIT_ASSERT_EQUAL(string(connCfg->cellularConfig.apnHost), string((String)connJson["cellCfg"]["apnHost"]));
	CPPUNIT_ASSERT_EQUAL(string(connCfg->cellularConfig.apnUser), string((String)connJson["cellCfg"]["apnUser"]));
	CPPUNIT_ASSERT_EQUAL(string(connCfg->cellularConfig.apnPass), string((String)connJson["cellCfg"]["apnPass"]));

	CPPUNIT_ASSERT_EQUAL((int)connCfg->telemetryConfig.backgroundStreaming, (int)(Number)connJson["telCfg"]["bgStream"]);
	CPPUNIT_ASSERT_EQUAL(string(connCfg->telemetryConfig.telemetryDeviceId), string((String)connJson["telCfg"]["deviceId"]));
	CPPUNIT_ASSERT_EQUAL(string(connCfg->telemetryConfig.telemetryServerHost), string((String)connJson["telCfg"]["host"]));
}

void LoggerApiTest::testGetPwmConfigFile(string filename, int index){
	LoggerConfig *c = getWorkingLoggerConfig();
	PWMConfig *pwmCfg = &c->PWMConfigs[index];

        populateChannelConfig(&pwmCfg->cfg, index, 100);

	pwmCfg->outputMode = 3;
	pwmCfg->loggingMode = 1;
	pwmCfg->startupDutyCycle = 55;
	pwmCfg->startupPeriod = 321;

	char * response = processApiGeneric(filename);

	Object json;
	stringToJson(response, json);

	std::ostringstream stringStream;
	stringStream << index;
        string iStr = stringStream.str();

	Object &analogJson = json["pwmCfg"][iStr];

        checkChannelConfig(analogJson, index, iStr, 100);

	CPPUNIT_ASSERT_EQUAL(3, (int)(Number)analogJson["outMode"]);
	CPPUNIT_ASSERT_EQUAL(1, (int)(Number)analogJson["logMode"]);
	CPPUNIT_ASSERT_EQUAL(55, (int)(Number)analogJson["stDutyCyc"]);
	CPPUNIT_ASSERT_EQUAL(321, (int)(Number)analogJson["stPeriod"]);
}

void LoggerApiTest::testGetPwmCfg(){
	testGetPwmConfigFile("getPwmCfg1.json", 0);
}


void LoggerApiTest::testSetPwmConfigFile(string filename){
	Serial *serial = getMockSerial();
	string json = readFile(filename);
	mock_resetTxBuffer();
	process_api(getMockSerial(),(char *)json.c_str(), json.size());

	LoggerConfig *c = getWorkingLoggerConfig();
	PWMConfig *pwmCfg = &c->PWMConfigs[0];

        ChannelConfig *cfg = &pwmCfg->cfg;
        CPPUNIT_ASSERT_EQUAL(string("I <3 Pulses"), string(cfg->label));
        CPPUNIT_ASSERT_EQUAL(string("Hertz"), string(cfg->units));
        CPPUNIT_ASSERT_EQUAL(-1.0f, cfg->min);
        CPPUNIT_ASSERT_EQUAL(1.0f, cfg->max);
	CPPUNIT_ASSERT_EQUAL(100, decodeSampleRate(cfg->sampleRate));
	CPPUNIT_ASSERT_EQUAL(1, (int)cfg->precision);

	CPPUNIT_ASSERT_EQUAL(1, (int)pwmCfg->outputMode);
	CPPUNIT_ASSERT_EQUAL(1, (int)pwmCfg->loggingMode);
	CPPUNIT_ASSERT_EQUAL(50, (int)pwmCfg->startupDutyCycle);
	CPPUNIT_ASSERT_EQUAL(110, (int)pwmCfg->startupPeriod);

	char *txBuffer = mock_getTxBuffer();
	assertGenericResponse(txBuffer, "setPwmCfg", API_SUCCESS);

}

void LoggerApiTest::testSetPwmCfg(){
	testSetPwmConfigFile("setPwmCfg1.json");
}

void LoggerApiTest::testGetGpioConfigFile(string filename, int index){
	LoggerConfig *c = getWorkingLoggerConfig();
	GPIOConfig *gpioCfg = &c->GPIOConfigs[index];

        populateChannelConfig(&gpioCfg->cfg, index, 100);
	gpioCfg->mode = 1;

	char * response = processApiGeneric(filename);

	Object json;
	stringToJson(response, json);

	std::ostringstream stringStream;
	stringStream << index;
        string iStr = stringStream.str();

	Object &analogJson = json["gpioCfg"][iStr];

        checkChannelConfig(analogJson, index, iStr, 100);
	CPPUNIT_ASSERT_EQUAL(1, (int)(Number)analogJson["mode"]);
}

void LoggerApiTest::testGetGpioCfg(){
	testGetGpioConfigFile("getGpioCfg1.json", 0);
}

void LoggerApiTest::testSetGpioConfigFile(string filename){
	Serial *serial = getMockSerial();
	string json = readFile(filename);
	mock_resetTxBuffer();
	process_api(getMockSerial(),(char *)json.c_str(), json.size());

	LoggerConfig *c = getWorkingLoggerConfig();
	GPIOConfig *gpioCfg = &c->GPIOConfigs[0];

        ChannelConfig *cfg = &gpioCfg->cfg;
        CPPUNIT_ASSERT_EQUAL(string("I <3 GenIO"), string(cfg->label));
        CPPUNIT_ASSERT_EQUAL(string("Sexynss"), string(cfg->units));
        CPPUNIT_ASSERT_EQUAL(-1.0f, cfg->min);
        CPPUNIT_ASSERT_EQUAL(1.0f, cfg->max);
	CPPUNIT_ASSERT_EQUAL(100, decodeSampleRate(cfg->sampleRate));
	CPPUNIT_ASSERT_EQUAL(1, (int)cfg->precision);

	CPPUNIT_ASSERT_EQUAL(1, (int)gpioCfg->mode);

	char *txBuffer = mock_getTxBuffer();
	assertGenericResponse(txBuffer, "setGpioCfg", API_SUCCESS);

}

void LoggerApiTest::testSetGpioCfg(){
	testSetGpioConfigFile("setGpioCfg1.json");
}

void LoggerApiTest::testGetTimerConfigFile(string filename, int index){
	LoggerConfig *c = getWorkingLoggerConfig();
	TimerConfig *timerCfg = &c->TimerConfigs[index];

        populateChannelConfig(&timerCfg->cfg, index, 100);

	timerCfg->slowTimerEnabled = 1;
	timerCfg->mode = 2;
	timerCfg->filterAlpha = 0.5F;
	timerCfg->pulsePerRevolution = 3;
	timerCfg->timerSpeed = 2;

	char * response = processApiGeneric(filename);

	Object json;
	stringToJson(response, json);

	std::ostringstream stringStream;
	stringStream << index;
        string iStr = stringStream.str();

	Object &timerJson = json["timerCfg"][iStr];

        checkChannelConfig(timerJson, index, iStr, 100);
	CPPUNIT_ASSERT_EQUAL(1, (int)(Number)timerJson["st"]);
	CPPUNIT_ASSERT_EQUAL(2, (int)(Number)timerJson["mode"]);
	CPPUNIT_ASSERT_EQUAL(0.5F, (float)(Number)timerJson["alpha"]);
	CPPUNIT_ASSERT_EQUAL(3, (int)(Number)timerJson["ppr"]);
	CPPUNIT_ASSERT_EQUAL(2, (int)(Number)timerJson["speed"]);
}

void LoggerApiTest::testGetTimerCfg(){
	testGetTimerConfigFile("getTimerCfg1.json", API_SUCCESS);
}

void LoggerApiTest::testSetTimerConfigFile(string filename){
	Serial *serial = getMockSerial();
	string json = readFile(filename);
	mock_resetTxBuffer();
	process_api(getMockSerial(),(char *)json.c_str(), json.size());

	LoggerConfig *c = getWorkingLoggerConfig();
	TimerConfig *timerCfg = &c->TimerConfigs[0];

        ChannelConfig *cfg = &timerCfg->cfg;
        CPPUNIT_ASSERT_EQUAL(string("I <3 Timing"), string(cfg->label));
        CPPUNIT_ASSERT_EQUAL(string("seconds"), string(cfg->units));
        CPPUNIT_ASSERT_EQUAL(-1.0f, cfg->min);
        CPPUNIT_ASSERT_EQUAL(1.0f, cfg->max);
	CPPUNIT_ASSERT_EQUAL(10, decodeSampleRate(cfg->sampleRate));
	CPPUNIT_ASSERT_EQUAL(1, (int)cfg->precision);

	CPPUNIT_ASSERT_EQUAL(1, (int)timerCfg->slowTimerEnabled);
	CPPUNIT_ASSERT_EQUAL(1, (int)timerCfg->mode);
	CPPUNIT_ASSERT_EQUAL(0.5F, timerCfg->filterAlpha);
	CPPUNIT_ASSERT_EQUAL(4, (int)timerCfg->pulsePerRevolution);
	CPPUNIT_ASSERT_EQUAL(2, (int)timerCfg->timerSpeed);

	char *txBuffer = mock_getTxBuffer();
	assertGenericResponse(txBuffer, "setTimerCfg", API_SUCCESS);
}

void LoggerApiTest::testSetTimerCfg(){
	testSetTimerConfigFile("setTimerCfg1.json");
}

string LoggerApiTest::getSampleResponse(string requestJson) {
	mock_resetTxBuffer();
	process_api(getMockSerial(),(char *)requestJson.c_str(), requestJson.size());

	LoggerConfig *c = getWorkingLoggerConfig();
	char *txBuffer = mock_getTxBuffer();
	string txString(txBuffer);
        return txString;
}

void LoggerApiTest::testGetMeta(){
	string requestJson = readFile("getMeta.json");
	string expectedResponseJson = readFile("getMeta_response.json");

	CPPUNIT_ASSERT_EQUAL(expectedResponseJson,
                        getSampleResponse(requestJson));
}

void LoggerApiTest::testSampleData1() {
	string requestJson1 = readFile("sampleData1.json");
	string expectedResponseJson1 = readFile("sampleData_response1.json");

	CPPUNIT_ASSERT_EQUAL(expectedResponseJson1,
                        getSampleResponse(requestJson1));
}

void LoggerApiTest::testSampleData2() {
	string requestJson2 = readFile("sampleData2.json");
	string expectedResponseJson2 = readFile("sampleData_response2.json");

	CPPUNIT_ASSERT_EQUAL(expectedResponseJson2,
                        getSampleResponse(requestJson2));
}

void LoggerApiTest::testHeartBeat(){
	set_ticks(3);
    string requestJson = readFile("heartBeat_request.json");
    string expectedResponseJson = readFile("heartBeat_response.json");

    getUptimeAsInt();
    CPPUNIT_ASSERT_EQUAL(expectedResponseJson,
                        getSampleResponse(requestJson));
}

void LoggerApiTest::testLogStartStopFile(string filename){

	string json = readFile(filename);
	mock_resetTxBuffer();
	process_api(getMockSerial(), (char *)json.c_str(), json.size());
	char *txBuffer = mock_getTxBuffer();
	assertGenericResponse(txBuffer,"log", API_SUCCESS);
}

void LoggerApiTest::testLogStartStop(){
	testLogStartStopFile("log1.json");
	testLogStartStopFile("log2.json");
}

void LoggerApiTest::testCalibrateImuFile(string filename){

	string json = readFile(filename);
	mock_resetTxBuffer();
	process_api(getMockSerial(), (char *)json.c_str(), json.size());
	char *txBuffer = mock_getTxBuffer();
	assertGenericResponse(txBuffer,"calImu",1);
}

void LoggerApiTest::testCalibrateImu(){
	testCalibrateImuFile("calibrateImu.json");
}

void LoggerApiTest::testFlashConfigFile(string filename){
	memory_mock_set_is_flashed(1);
	string json = readFile(filename);
	mock_resetTxBuffer();
	process_api(getMockSerial(), (char *)json.c_str(), json.size());
	char *txBuffer = mock_getTxBuffer();
	int isFlashed = memory_mock_get_is_flashed();
	CPPUNIT_ASSERT_EQUAL(isFlashed, 1);
	assertGenericResponse(txBuffer,"flashCfg",1);
}

void LoggerApiTest::testFlashConfig(){
	testFlashConfigFile("flashCfg.json");
}

void LoggerApiTest::testChannelConfig(ChannelConfig *cfg, string expNm, string expUt, unsigned short sr) {
    CPPUNIT_ASSERT_EQUAL(expNm, string(cfg->label));
    CPPUNIT_ASSERT_EQUAL(expUt, string(cfg->units));
    CPPUNIT_ASSERT_EQUAL((int) sr, decodeSampleRate(cfg->sampleRate));
}

void LoggerApiTest::testSetGpsConfigFile(string filename, unsigned char channelsEnabled, unsigned short sampleRate){
	processApiGeneric(filename);
	char *txBuffer = mock_getTxBuffer();

	LoggerConfig *c = getWorkingLoggerConfig();
	GPSConfig *gpsCfg = &c->GPSConfigs;

        if (channelsEnabled == 0)
           sampleRate = 0;

        testChannelConfig(&gpsCfg->latitude, string("Latitude"), string("Degrees"), sampleRate);
        testChannelConfig(&gpsCfg->longitude, string("Longitude"), string("Degrees"), sampleRate);
        testChannelConfig(&gpsCfg->speed, string("Speed"), string("MPH"), sampleRate);
        testChannelConfig(&gpsCfg->distance, string("Distance"), string("Miles"), sampleRate);
        testChannelConfig(&gpsCfg->altitude, string("Altitude"), string("Feet"), sampleRate);
        testChannelConfig(&gpsCfg->satellites, string("GPSSats"), string(""), sampleRate);
        testChannelConfig(&gpsCfg->quality, string("GPSQual"), string(""), sampleRate);
        testChannelConfig(&gpsCfg->DOP, string("GPSDOP"), string(""), sampleRate);

	assertGenericResponse(txBuffer, "setGpsCfg", API_SUCCESS);
}

void LoggerApiTest::testSetGpsCfg(){
	testSetGpsConfigFile("setGpsCfg1.json", 1, 100);
	testSetGpsConfigFile("setGpsCfg2.json", 0, 50);
}

void LoggerApiTest::testGetGpsConfigFile(string filename){
   LoggerConfig *c = getWorkingLoggerConfig();
   GPSConfig *gpsCfg = &c->GPSConfigs;

   populateChannelConfig(&gpsCfg->latitude, 0, 100);
   populateChannelConfig(&gpsCfg->longitude, 0, 100);
   populateChannelConfig(&gpsCfg->speed, 0, 100);
   populateChannelConfig(&gpsCfg->distance, 0, 100);
   populateChannelConfig(&gpsCfg->altitude, 0, 100);
   populateChannelConfig(&gpsCfg->satellites, 0, 100);
   populateChannelConfig(&gpsCfg->quality, 0, 100);
   populateChannelConfig(&gpsCfg->DOP, 0, 100);

   char * response = processApiGeneric(filename);

   Object json;
   stringToJson(response, json);

   Object &gpsCfgJson = json["gpsCfg"];

   CPPUNIT_ASSERT_EQUAL((int)100, (int)(Number)gpsCfgJson["sr"]);

   CPPUNIT_ASSERT_EQUAL(1, (int)(Number)gpsCfgJson["pos"]);
   CPPUNIT_ASSERT_EQUAL(1, (int)(Number)gpsCfgJson["speed"]);
   CPPUNIT_ASSERT_EQUAL(1, (int)(Number)gpsCfgJson["dist"]);
   CPPUNIT_ASSERT_EQUAL(1, (int)(Number)gpsCfgJson["alt"]);
   CPPUNIT_ASSERT_EQUAL(1, (int)(Number)gpsCfgJson["sats"]);
   CPPUNIT_ASSERT_EQUAL(1, (int)(Number)gpsCfgJson["qual"]);
   CPPUNIT_ASSERT_EQUAL(1, (int)(Number)gpsCfgJson["dop"]);
}

void LoggerApiTest::testGetGpsCfg(){
	testGetGpsConfigFile("getGpsCfg1.json");
}

void LoggerApiTest::testSetLapCfg(){
	testSetLapConfigFile("setLapCfg1.json");
}

void LoggerApiTest::testSetLapConfigFile(string filename){
	processApiGeneric(filename);
	char *txBuffer = mock_getTxBuffer();

	LoggerConfig *c = getWorkingLoggerConfig();
	LapConfig *cfg = &c->LapConfigs;

	assertGenericResponse(txBuffer, "setLapCfg", API_SUCCESS);

        testChannelConfig(&cfg->lapCountCfg, string("LapCount"), string(""), 50);
        testChannelConfig(&cfg->lapTimeCfg, string("LapTime"), string("Min"), 50);
        testChannelConfig(&cfg->sectorCfg, string("Sector"), string(""), 50);
        testChannelConfig(&cfg->sectorTimeCfg, string("SectorTime"), string("Min"), 50);
        testChannelConfig(&cfg->predTimeCfg, string("PredTime"), string("Min"), 50);
}

void LoggerApiTest::testGetLapConfigFile(string filename){

	LoggerConfig *c = getWorkingLoggerConfig();
	LapConfig *cfg = &c->LapConfigs;

        populateChannelConfig(&cfg->lapCountCfg, 1, 50);
        populateChannelConfig(&cfg->lapTimeCfg, 2, 50);
        populateChannelConfig(&cfg->sectorCfg, 3, 50);
        populateChannelConfig(&cfg->sectorTimeCfg, 4, 50);
        populateChannelConfig(&cfg->predTimeCfg, 5, 50);

	char * response = processApiGeneric(filename);

	Object json;
	stringToJson(response, json);

        Object &lapCount = json["lapCfg"]["lapCount"];
        Object &lapTime = json["lapCfg"]["lapTime"];
        Object &lapSector = json["lapCfg"]["sector"];
        Object &lapSectorTime = json["lapCfg"]["sectorTime"];
        Object &lapPredTime = json["lapCfg"]["predTime"];

        string str1 = string("1");
        string str2 = string("2");
        string str3 = string("3");
        string str4 = string("4");
        string str5 = string("5");

        checkChannelConfig(lapCount, 1, str1, 50);
        checkChannelConfig(lapTime, 2, str2, 50);
        checkChannelConfig(lapSector, 3, str3, 50);
        checkChannelConfig(lapSectorTime, 4, str4, 50);
        checkChannelConfig(lapPredTime, 5, str5, 50);
}

void LoggerApiTest::testGetLapCfg(){
	testGetLapConfigFile("getLapCfg1.json");
}

void LoggerApiTest::testSetTrackCfgCircuit(){

	int sectors = 10; //to match file
	processApiGeneric("setTrackCfg1.json");
	char *txBuffer = mock_getTxBuffer();

	LoggerConfig *c = getWorkingLoggerConfig();
	TrackConfig *cfg = &c->TrackConfigs;

	assertGenericResponse(txBuffer, "setTrackCfg", API_SUCCESS);

	CPPUNIT_ASSERT_EQUAL(0, (int)cfg->track.track_type);
        CPPUNIT_ASSERT_EQUAL(6674, cfg->track.trackId);
	CPPUNIT_ASSERT_CLOSE_ENOUGH(0.0001F, cfg->radius);
	CPPUNIT_ASSERT_EQUAL(0, (int)cfg->auto_detect);
	CPPUNIT_ASSERT_CLOSE_ENOUGH(1.0F, cfg->track.circuit.startFinish.latitude);
	CPPUNIT_ASSERT_CLOSE_ENOUGH(2.0F, cfg->track.circuit.startFinish.longitude);

	float startingValue = 1.1;

	for (int i = 0; i < sectors; i++){
		CPPUNIT_ASSERT_CLOSE_ENOUGH(startingValue, cfg->track.circuit.sectors[i].latitude);
		startingValue++;
		CPPUNIT_ASSERT_CLOSE_ENOUGH(startingValue, cfg->track.circuit.sectors[i].longitude);
		startingValue++;
	}
}

void LoggerApiTest::testGetTrackCfgCircuit(){
	LoggerConfig *c = getWorkingLoggerConfig();
	TrackConfig *cfg = &c->TrackConfigs;

	cfg->track.circuit.startFinish.latitude  = 1.0;
	cfg->track.circuit.startFinish.longitude = 2.0;
	float startingValue = 1.1;
	for (size_t i = 0; i < CIRCUIT_SECTOR_COUNT; i++){
		GeoPoint *point = (cfg->track.circuit.sectors + i);
		point->latitude = startingValue;
		startingValue++;
		point->longitude = startingValue;
		startingValue++;
	}
	cfg->radius = 0.009;
	cfg->auto_detect = 0;
	cfg->track.track_type = TRACK_TYPE_CIRCUIT;
	cfg->track.trackId = 1345;

	char * response = processApiGeneric("getTrackCfg1.json");

	Object json;
	stringToJson(response, json);

	CPPUNIT_ASSERT_EQUAL(0.009F, (float)(Number)json["trackCfg"]["rad"]);
	CPPUNIT_ASSERT_EQUAL(0, (int)(Number)json["trackCfg"]["autoDetect"]);
	CPPUNIT_ASSERT_EQUAL(0, (int)(Number)json["trackCfg"]["track"]["type"]);
	CPPUNIT_ASSERT_EQUAL(1345, (int)(Number)json["trackCfg"]["track"]["id"]);

	CPPUNIT_ASSERT_EQUAL(1.0F, (float)(Number)json["trackCfg"]["track"]["sf"][0]);
	CPPUNIT_ASSERT_EQUAL(2.0F, (float)(Number)json["trackCfg"]["track"]["sf"][1]);

	Array &sectors = json["trackCfg"]["track"]["sec"];

	startingValue = 1.1;
	for (size_t i = 0; i < CIRCUIT_SECTOR_COUNT; i++){
		CPPUNIT_ASSERT_CLOSE_ENOUGH(startingValue, (float)(Number) sectors[i][0]);
		startingValue++;
		CPPUNIT_ASSERT_CLOSE_ENOUGH(startingValue, (float)(Number) sectors[i][1]);
		startingValue++;
	}
}

void LoggerApiTest::testAddTrackDb(){
	testAddTrackDbFile("addTrackDb_circuit.json");
	testAddTrackDbFile("addTrackDb_stage.json");
}

void LoggerApiTest::testAddTrackDbFile(string filename){
	processApiGeneric(filename);
	char *txBuffer = mock_getTxBuffer();
	const Tracks *tracks = get_tracks();

	Object jsonCompare;
	string compare = readFile(filename);
	stringToJson(compare, jsonCompare);

	int index = (int)(Number)jsonCompare["addTrackDb"]["index"];
	const Track *track = tracks->tracks + index;

	int trackType = (int)(Number)jsonCompare["addTrackDb"]["track"]["type"];
	CPPUNIT_ASSERT_EQUAL(trackType, (int)track->track_type);

	if (trackType == TRACK_TYPE_CIRCUIT){
          CPPUNIT_ASSERT_EQUAL((float)(Number)jsonCompare["addTrackDb"]["track"]["sf"][0],
                               (float)track->circuit.startFinish.latitude);

          CPPUNIT_ASSERT_EQUAL((float)(Number)jsonCompare["addTrackDb"]["track"]["sf"][1],
                               (float)track->circuit.startFinish.longitude);
	} else {
          CPPUNIT_ASSERT_EQUAL((float)(Number)jsonCompare["addTrackDb"]["track"]["st"][0],
                               (float)track->stage.start.latitude);

          CPPUNIT_ASSERT_EQUAL((float)(Number)jsonCompare["addTrackDb"]["track"]["st"][1],
                               (float)track->stage.start.longitude);

          CPPUNIT_ASSERT_EQUAL((float)(Number)jsonCompare["addTrackDb"]["track"]["fin"][0],
                               (float)track->stage.finish.latitude);

          CPPUNIT_ASSERT_EQUAL((float)(Number)jsonCompare["addTrackDb"]["track"]["fin"][1],
                               (float)track->stage.finish.longitude);
	}

	Array secNode = (Array)jsonCompare["addTrackDb"]["track"]["sec"];
	for (int i = 0; i < secNode.Size(); i++){
		if (trackType == TRACK_TYPE_CIRCUIT){
			CPPUNIT_ASSERT_EQUAL((float)(Number)jsonCompare["addTrackDb"]["track"]["sec"][i][0], (float)track->circuit.sectors[i].latitude);
			CPPUNIT_ASSERT_EQUAL((float)(Number)jsonCompare["addTrackDb"]["track"]["sec"][i][1], (float)track->circuit.sectors[i].longitude);
		}
		else{
			CPPUNIT_ASSERT_EQUAL((float)(Number)jsonCompare["addTrackDb"]["track"]["sec"][i][0], (float)track->stage.sectors[i].latitude);
			CPPUNIT_ASSERT_EQUAL((float)(Number)jsonCompare["addTrackDb"]["track"]["sec"][i][1], (float)track->stage.sectors[i].longitude);
		}
	}
}

void LoggerApiTest::testGetTrackDb(){
	testGetTrackDbFile("getTrackDb1.json", "addTrackDb_circuit.json");
}

void LoggerApiTest::testGetTrackDbFile(string filename, string addedFilename){

	//add a track first
	processApiGeneric(addedFilename);

	//now get all the tracks
	char * response = processApiGeneric(filename);

	Object jsonResponse;
	stringToJson(response, jsonResponse);

	Object jsonCompare;
	string compare= readFile(addedFilename);
	stringToJson(compare, jsonCompare);

	CPPUNIT_ASSERT_EQUAL((int)(Number)jsonResponse["trackDb"]["size"], 1);
	CPPUNIT_ASSERT_EQUAL((int)(Number)jsonResponse["trackDb"]["max"], MAX_TRACK_COUNT);
	CPPUNIT_ASSERT_EQUAL((int)(Number)jsonResponse["trackDb"]["tracks"][0]["type"], (int)(Number)jsonCompare["addTrackDb"]["track"]["type"]);

	CPPUNIT_ASSERT_EQUAL((float)(Number)jsonResponse["trackDb"]["tracks"][0]["sf"][0], (float)(Number)jsonCompare["addTrackDb"]["track"]["sf"][0]);
	CPPUNIT_ASSERT_EQUAL((float)(Number)jsonResponse["trackDb"]["tracks"][0]["sf"][1], (float)(Number)jsonCompare["addTrackDb"]["track"]["sf"][1]);

	Array secNode = (Array)jsonResponse["trackDb"]["tracks"][0]["sec"];

	for (int i = 0; i < secNode.Size(); i++){
		CPPUNIT_ASSERT_EQUAL((float)(Number)jsonResponse["trackDb"]["tracks"][0]["sec"][i][0], (float)(Number)jsonCompare["addTrackDb"]["track"]["sec"][i][0]);
		CPPUNIT_ASSERT_EQUAL((float)(Number)jsonResponse["trackDb"]["tracks"][0]["sec"][i][1], (float)(Number)jsonCompare["addTrackDb"]["track"]["sec"][i][1]);
	}
}

void LoggerApiTest::testSetLogLevelFile(string filename, int expectedResponse){
	processApiGeneric(filename);

	//set_log_level(DEBUG);
	//char *txBuffer = mock_getTxBuffer();
	CPPUNIT_ASSERT_EQUAL( 7, (int)get_log_level());
	//assertGenericResponse(txBuffer, "setLogfileLevel", expectedResponse);
}

void LoggerApiTest::testSetLogLevel(){
	testSetLogLevelFile("setLogLevel1.json", API_SUCCESS);
}

void LoggerApiTest::testGetCanCfg(){
	testGetCanCfgFile("getCanCfg1.json");
}

void LoggerApiTest::testGetCanCfgFile(string filename){
	LoggerConfig *c = getWorkingLoggerConfig();
	CANConfig *canConfig= &c->CanConfig;

	canConfig->enabled = 1;
	canConfig->baud[0] = 1000000;
	canConfig->baud[1] = 125000;
	char * response = processApiGeneric(filename);

	Object json;
	stringToJson(response, json);
	CPPUNIT_ASSERT_EQUAL(1, (int)(Number)json["canCfg"]["en"]);
	CPPUNIT_ASSERT_EQUAL(1000000, (int)(Number)json["canCfg"]["baud"][0]);
	CPPUNIT_ASSERT_EQUAL(125000, (int)(Number)json["canCfg"]["baud"][1]);
}

void LoggerApiTest::testSetCanCfg(){
	testSetCanCfgFile("setCanCfg1.json");
}

void LoggerApiTest::testSetCanCfgFile(string filename){
	processApiGeneric(filename);
	char *txBuffer = mock_getTxBuffer();

	LoggerConfig *c = getWorkingLoggerConfig();
	CANConfig *canCfg = &c->CanConfig;

	CPPUNIT_ASSERT_EQUAL(1, (int)canCfg->enabled );
	CPPUNIT_ASSERT_EQUAL(125000, (int)canCfg->baud[0]);
	CPPUNIT_ASSERT_EQUAL(1000000, (int)canCfg->baud[1]);
}

void LoggerApiTest::testSetObd2Cfg(){
	testSetObd2ConfigFile("setObd2Cfg1.json");
}

void LoggerApiTest::testGetObd2Cfg(){
	testGetObd2ConfigFile("getObd2Cfg1.json");
}

void LoggerApiTest::testGetObd2ConfigFile(string filename){
	LoggerConfig *c = getWorkingLoggerConfig();
	OBD2Config *obd2Config = &c->OBD2Configs;

	obd2Config->enabled = 1;
	obd2Config->enabledPids = 2;

        ChannelConfig *cfg1 = &obd2Config->pids[0].cfg;
        populateChannelConfig(cfg1, 1, 1);
	obd2Config->pids[0].pid = 0x05;

        ChannelConfig *cfg2 = &obd2Config->pids[1].cfg;
        populateChannelConfig(cfg2, 2, 50);
	obd2Config->pids[1].pid = 0x06;

	char * response = processApiGeneric(filename);

	Object json;
	stringToJson(response, json);

	Object pid1 = (Object)json["obd2Cfg"]["pids"][0];
	Object pid2 = (Object)json["obd2Cfg"]["pids"][1];

        string str1 = string("1");
        string str2 = string("2");

        checkChannelConfig(pid1, 1, str1, 1);
	CPPUNIT_ASSERT_EQUAL(0x05, (int)(Number)pid1["pid"]);

        checkChannelConfig(pid2, 2, str2, 50);
	CPPUNIT_ASSERT_EQUAL(0x06, (int)(Number)pid2["pid"]);

	CPPUNIT_ASSERT_EQUAL(1, (int)(Number)json["obd2Cfg"]["en"]);
}

void LoggerApiTest::testSetObd2ConfigFile(string filename){
	processApiGeneric(filename);
	char *txBuffer = mock_getTxBuffer();

	LoggerConfig *c = getWorkingLoggerConfig();
	OBD2Config *obd2Config = &c->OBD2Configs;

	CPPUNIT_ASSERT_EQUAL(1, (int)obd2Config->enabled);
	CPPUNIT_ASSERT_EQUAL(2, (int)obd2Config->enabledPids);

	PidConfig *pidCfg1 = &obd2Config->pids[0];
	PidConfig *pidCfg2 = &obd2Config->pids[1];

        ChannelConfig *cfg = &pidCfg1->cfg;
        CPPUNIT_ASSERT_EQUAL(string("I <3 OBD2"), string(cfg->label));
        CPPUNIT_ASSERT_EQUAL(string("?????"), string(cfg->units));
        CPPUNIT_ASSERT_EQUAL(-1.0f, cfg->min);
        CPPUNIT_ASSERT_EQUAL(1.0f, cfg->max);
	CPPUNIT_ASSERT_EQUAL(10, decodeSampleRate(cfg->sampleRate));
	CPPUNIT_ASSERT_EQUAL(1, (int)cfg->precision);
	CPPUNIT_ASSERT_EQUAL(5, (int)pidCfg1->pid);

        cfg = &pidCfg2->cfg;
        CPPUNIT_ASSERT_EQUAL(string("I <3 OBD2"), string(cfg->label));
        CPPUNIT_ASSERT_EQUAL(string("?????"), string(cfg->units));
        CPPUNIT_ASSERT_EQUAL(-1.0f, cfg->min);
        CPPUNIT_ASSERT_EQUAL(1.0f, cfg->max);
	CPPUNIT_ASSERT_EQUAL(10, decodeSampleRate(cfg->sampleRate));
	CPPUNIT_ASSERT_EQUAL(1, (int)cfg->precision);
	CPPUNIT_ASSERT_EQUAL(6, (int)pidCfg2->pid);
}

void LoggerApiTest::testSetObd2ConfigFile_fromIndex(){
	processApiGeneric("setObd2Cfg_fromIndex.json");
	char *txBuffer = mock_getTxBuffer();

	LoggerConfig *c = getWorkingLoggerConfig();
	OBD2Config *obd2Config = &c->OBD2Configs;

	CPPUNIT_ASSERT_EQUAL(1, (int)obd2Config->enabled);

	//when setting PIDs from an index, index + length of PID array becomes the total number of enabled PIDs
	CPPUNIT_ASSERT_EQUAL(4, (int)obd2Config->enabledPids);

	PidConfig *pidCfg1 = &obd2Config->pids[2];
	PidConfig *pidCfg2 = &obd2Config->pids[3];

        ChannelConfig *cfg = &pidCfg1->cfg;
        CPPUNIT_ASSERT_EQUAL(string("I <3 OBD2"), string(cfg->label));
        CPPUNIT_ASSERT_EQUAL(string("?????"), string(cfg->units));
        CPPUNIT_ASSERT_EQUAL(-1.0f, cfg->min);
        CPPUNIT_ASSERT_EQUAL(1.0f, cfg->max);
	CPPUNIT_ASSERT_EQUAL(10, decodeSampleRate(cfg->sampleRate));
	CPPUNIT_ASSERT_EQUAL(1, (int)cfg->precision);
	CPPUNIT_ASSERT_EQUAL(5, (int)pidCfg1->pid);

        cfg = &pidCfg2->cfg;
        CPPUNIT_ASSERT_EQUAL(string("I <3 OBD2"), string(cfg->label));
        CPPUNIT_ASSERT_EQUAL(string("?????"), string(cfg->units));
        CPPUNIT_ASSERT_EQUAL(-1.0f, cfg->min);
        CPPUNIT_ASSERT_EQUAL(1.0f, cfg->max);
	CPPUNIT_ASSERT_EQUAL(10, decodeSampleRate(cfg->sampleRate));
	CPPUNIT_ASSERT_EQUAL(1, (int)cfg->precision);
	CPPUNIT_ASSERT_EQUAL(6, (int)pidCfg2->pid);
}

void LoggerApiTest::testSetObd2ConfigFile_invalid(){
	char *response = NULL;

	response = processApiGeneric("setObd2Cfg_too_many_pids_from_index.json");
	assertGenericResponse(response, "setObd2Cfg", API_ERROR_PARAMETER);

	response = processApiGeneric("setObd2Cfg_too_many_pids.json");
	assertGenericResponse(response, "setObd2Cfg", API_ERROR_PARAMETER);
}

void LoggerApiTest::testSetScript(){
	testSetScriptFile("setScript1.json");
}

void LoggerApiTest::testGetScript(){
	testGetScriptFile("getScript1.json");
}

void LoggerApiTest::testGetScriptFile(string filename){
	char * response = processApiGeneric(filename);

	Object json;
	stringToJson(response, json);

	CPPUNIT_ASSERT_EQUAL(string("function onTick() end"), (string)(String)json["scriptCfg"]["data"]);

}

void LoggerApiTest::testSetScriptFile(string filename){
	processApiGeneric(filename);
	char *txBuffer = mock_getTxBuffer();

	LoggerConfig *c = getWorkingLoggerConfig();
	TrackConfig *cfg = &c->TrackConfigs;

	assertGenericResponse(txBuffer, "setScriptCfg", API_SUCCESS);

	const char * script = getScript();

	//todo enable after dealing with const
	CPPUNIT_ASSERT_EQUAL(string("function hello() end"), string(script));
}

void LoggerApiTest::testRunScript(){
	testRunScriptFile("runScript1.json");
}

void LoggerApiTest::testRunScriptFile(string filename){
	processApiGeneric(filename);
	char *txBuffer = mock_getTxBuffer();
	assertGenericResponse(txBuffer, "runScript", API_SUCCESS);
}

void LoggerApiTest::testGetCapabilities(){
	char * response = processApiGeneric("getCapabilities.json");

	Object json;
	stringToJson(response, json);

	CPPUNIT_ASSERT_EQUAL(ANALOG_CHANNELS, (int)(Number)json["capabilities"]["channels"]["analog"]);
	CPPUNIT_ASSERT_EQUAL(IMU_CHANNELS, (int)(Number)json["capabilities"]["channels"]["imu"]);
	CPPUNIT_ASSERT_EQUAL(GPIO_CHANNELS, (int)(Number)json["capabilities"]["channels"]["gpio"]);
	CPPUNIT_ASSERT_EQUAL(TIMER_CHANNELS, (int)(Number)json["capabilities"]["channels"]["timer"]);
	CPPUNIT_ASSERT_EQUAL(PWM_CHANNELS, (int)(Number)json["capabilities"]["channels"]["pwm"]);
	CPPUNIT_ASSERT_EQUAL(CAN_CHANNELS, (int)(Number)json["capabilities"]["channels"]["can"]);

	CPPUNIT_ASSERT_EQUAL(MAX_SENSOR_SAMPLE_RATE, (int)(Number)json["capabilities"]["sampleRates"]["sensor"]);
	CPPUNIT_ASSERT_EQUAL(MAX_GPS_SAMPLE_RATE, (int)(Number)json["capabilities"]["sampleRates"]["gps"]);

	CPPUNIT_ASSERT_EQUAL(MAX_TRACKS, (int)(Number)json["capabilities"]["db"]["tracks"]);
	CPPUNIT_ASSERT_EQUAL(MAX_SECTORS, (int)(Number)json["capabilities"]["db"]["sectors"]);
	CPPUNIT_ASSERT_EQUAL(SCRIPT_MEMORY_LENGTH, (int)(Number)json["capabilities"]["db"]["script"]);
}

void LoggerApiTest::testGetVersion(){
	char * response = processApiGeneric("getVersion1.json");

	Object json;
	stringToJson(response, json);

	CPPUNIT_ASSERT_EQUAL(string(DEVICE_NAME), (string)(String)json["ver"]["name"]);
	CPPUNIT_ASSERT_EQUAL(MAJOR_REV, (int)(Number)json["ver"]["major"]);
	CPPUNIT_ASSERT_EQUAL(MINOR_REV, (int)(Number)json["ver"]["minor"]);
	CPPUNIT_ASSERT_EQUAL(BUGFIX_REV, (int)(Number)json["ver"]["bugfix"]);
	CPPUNIT_ASSERT_EQUAL(string(cpu_get_serialnumber()), (string)(String)json["ver"]["serial"]);
}

void LoggerApiTest::testGetStatus(){
	set_ticks(3);
        lc_reset();
        lapStats_init();

    char * response = processApiGeneric("getStatus1.json");

    Object json;
    stringToJson(response, json);

    CPPUNIT_ASSERT_EQUAL(string(FRIENDLY_DEVICE_NAME), (string)(String)json["status"]["system"]["model"]);
    CPPUNIT_ASSERT_EQUAL(MAJOR_REV, (int)(Number)json["status"]["system"]["ver_major"]);
    CPPUNIT_ASSERT_EQUAL(MINOR_REV, (int)(Number)json["status"]["system"]["ver_minor"]);
    CPPUNIT_ASSERT_EQUAL(BUGFIX_REV, (int)(Number)json["status"]["system"]["ver_bugfix"]);
    CPPUNIT_ASSERT_EQUAL(string(cpu_get_serialnumber()), (string)(String)json["status"]["system"]["serial"]);
    CPPUNIT_ASSERT_EQUAL(15, (int)(Number)json["status"]["system"]["uptime"]);

    CPPUNIT_ASSERT_EQUAL((int)GPS_STATUS_PROVISIONED, (int)(Number)json["status"]["GPS"]["init"]);
    CPPUNIT_ASSERT_EQUAL(0, (int)(Number)json["status"]["GPS"]["qual"]);
    CPPUNIT_ASSERT_EQUAL(0.0f, (float)(Number)json["status"]["GPS"]["lat"]);
    CPPUNIT_ASSERT_EQUAL(0.0f, (float)(Number)json["status"]["GPS"]["lon"]);
    CPPUNIT_ASSERT_EQUAL(0, (int)(Number)json["status"]["GPS"]["sats"]);
    CPPUNIT_ASSERT_EQUAL(0, (int)(Number)json["status"]["GPS"]["DOP"]);

    CPPUNIT_ASSERT_EQUAL((int)CELLMODEM_STATUS_NOT_INIT, (int)(Number)json["status"]["cell"]["init"]);
    CPPUNIT_ASSERT_EQUAL(string(""), (string)(String)json["status"]["cell"]["IMEI"]);
    CPPUNIT_ASSERT_EQUAL(0, (int)(Number)json["status"]["cell"]["sig_str"]);
    CPPUNIT_ASSERT_EQUAL(string(""), (string)(String)json["status"]["cell"]["number"]);

    CPPUNIT_ASSERT_EQUAL((int)BT_STATUS_NOT_INIT, (int)(Number)json["status"]["bt"]["init"]);

    CPPUNIT_ASSERT_EQUAL((int)LOGGING_STATUS_IDLE, (int)(Number)json["status"]["logging"]["status"]);
    CPPUNIT_ASSERT_EQUAL(0, (int)(Number)json["status"]["logging"]["started"]);

    CPPUNIT_ASSERT_EQUAL((int)TRACK_STATUS_WAITING_TO_CONFIG, (int)(Number)json["status"]["track"]["status"]);
    CPPUNIT_ASSERT_EQUAL(0, (int)(Number)json["status"]["track"]["trackId"]);
    CPPUNIT_ASSERT_EQUAL(0, (int)(Number)json["status"]["track"]["armed"]);
    CPPUNIT_ASSERT_EQUAL(0, (int)(Number)json["status"]["track"]["inLap"]);

    CPPUNIT_ASSERT_EQUAL((int)TELEMETRY_STATUS_IDLE, (int)(Number)json["status"]["telemetry"]["status"]);
    CPPUNIT_ASSERT_EQUAL(0, (int)(Number)json["status"]["telemetry"]["started"]);
}
