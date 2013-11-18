/*
 * loggerApi_test.cpp
 *
 *  Created on: Jun 10, 2013
 *      Author: brent
 */
#include "loggerApi_test.h"

#include "api.h"
#include "loggerApi.h"
#include "mock_serial.h"
#include "loggerHardware_mock.h"
#include "loggerConfig.h"
#include "jsmn.h"
#include "mod_string.h"
#include "modp_atonum.h"
#include <string>
#include <fstream>
#include <streambuf>
#include "predictive_timer.h"

#define JSON_TOKENS 10000
#define FILE_PREFIX string("test/")

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
				CPPUNIT_ASSERT_EQUAL(responseCode, modp_atoi(tok_rcVal->data));
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
	initApi();
	updateActiveLoggerConfig();
	setupMockSerial();
	init_predictive_timer();
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

void LoggerApiTest::testGetMultipleAnalogCfg(){

	LoggerConfig *c = getWorkingLoggerConfig();
	for (int i = 0; i < CONFIG_ADC_CHANNELS; i++){

		ADCConfig *analogCfg = &c->ADCConfigs[i];

		std::ostringstream theLabel;
		theLabel << "lab_" << i;
		strcpy(analogCfg->cfg.label, theLabel.str().c_str());
		theLabel.str("");
		theLabel << "ut_" << i;
		strcpy(analogCfg->cfg.units, theLabel.str().c_str());
		analogCfg->cfg.sampleRate = 1 + i;
		analogCfg->linearScaling = 3.21 + i;
		analogCfg->loggingPrecision = i;
		analogCfg->scalingMode = i;

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
		Object &analogJson = json["getAnalogCfg"][stringStream.str().c_str()];

		std::ostringstream theLabel;
		theLabel << "lab_" << i;
		CPPUNIT_ASSERT_EQUAL(theLabel.str(), string((String)analogJson["nm"]));
		theLabel.str("");
		theLabel << "ut_" << i;
		CPPUNIT_ASSERT_EQUAL(theLabel.str(), string((String)analogJson["ut"]));
		CPPUNIT_ASSERT_EQUAL(1 + i, (int)(Number)analogJson["sr"]);
		CPPUNIT_ASSERT_EQUAL(3.21F + i, (float)(Number)analogJson["linScal"]);
		CPPUNIT_ASSERT_EQUAL(i, (int)(Number)analogJson["prec"]);
		CPPUNIT_ASSERT_EQUAL(i, (int)(Number)analogJson["scalMod"]);

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

	LoggerConfig *c = getWorkingLoggerConfig();
	ADCConfig *analogCfg = &c->ADCConfigs[index];

	strcpy(analogCfg->cfg.label, "aLabel");
	strcpy(analogCfg->cfg.units, "aUnits");
	analogCfg->cfg.sampleRate = 100;
	analogCfg->linearScaling = 3.21;
	analogCfg->loggingPrecision = 5;
	analogCfg->scalingMode = 2;


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

	std::ostringstream stringStream;
	stringStream << index;
	Object &analogJson = json["getAnalogCfg"][stringStream.str()];


	CPPUNIT_ASSERT_EQUAL(string("aLabel"), string((String)analogJson["nm"]));
	CPPUNIT_ASSERT_EQUAL(string("aUnits"), string((String)analogJson["ut"]));
	CPPUNIT_ASSERT_EQUAL(100, (int)(Number)analogJson["sr"]);
	CPPUNIT_ASSERT_EQUAL(3.21F, (float)(Number)analogJson["linScal"]);
	CPPUNIT_ASSERT_EQUAL(5, (int)(Number)analogJson["prec"]);
	CPPUNIT_ASSERT_EQUAL(2, (int)(Number)analogJson["scalMod"]);

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

	CPPUNIT_ASSERT_EQUAL(string("analog1"),string(adcCfg->cfg.label));
	CPPUNIT_ASSERT_EQUAL(50, decodeSampleRate(adcCfg->cfg.sampleRate));
	CPPUNIT_ASSERT_EQUAL(string("Hz"),string(adcCfg->cfg.units));

	CPPUNIT_ASSERT_EQUAL(2, (int)adcCfg->scalingMode);
	CPPUNIT_ASSERT_EQUAL(3, adcCfg->loggingPrecision);
	CPPUNIT_ASSERT_EQUAL(1.234F, adcCfg->linearScaling);

	CPPUNIT_ASSERT_EQUAL(1, (int)adcCfg->scalingMap.rawValues[0]);
	CPPUNIT_ASSERT_EQUAL(2, (int)adcCfg->scalingMap.rawValues[1]);
	CPPUNIT_ASSERT_EQUAL(3, (int)adcCfg->scalingMap.rawValues[2]);
	CPPUNIT_ASSERT_EQUAL(4, (int)adcCfg->scalingMap.rawValues[3]);
	CPPUNIT_ASSERT_EQUAL(5, (int)adcCfg->scalingMap.rawValues[4]);

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

void LoggerApiTest::testGetAccelConfigFile(string filename, int index){
	LoggerConfig *c = getWorkingLoggerConfig();
	AccelConfig *accelCfg = &c->AccelConfigs[index];

	strcpy(accelCfg->cfg.label, "pLabel");
	strcpy(accelCfg->cfg.units, "pUnits");
	accelCfg->cfg.sampleRate = 100;
	accelCfg->mode = 1;
	accelCfg->accelChannel = 3;
	accelCfg->zeroValue = 1234;

	char * response = processApiGeneric(filename);

	Object json;
	stringToJson(response, json);

	std::ostringstream stringStream;
	stringStream << index;
	Object &analogJson = json["getAccelCfg"][stringStream.str()];

	CPPUNIT_ASSERT_EQUAL(string("pLabel"), string((String)analogJson["nm"]));
	CPPUNIT_ASSERT_EQUAL(string("pUnits"), string((String)analogJson["ut"]));
	CPPUNIT_ASSERT_EQUAL(100, (int)(Number)analogJson["sr"]);

	CPPUNIT_ASSERT_EQUAL(1, (int)(Number)analogJson["mode"]);
	CPPUNIT_ASSERT_EQUAL(3, (int)(Number)analogJson["chan"]);
	CPPUNIT_ASSERT_EQUAL(1234, (int)(Number)analogJson["zeroVal"]);
}

void LoggerApiTest::testGetAccelCfg(){
	testGetAccelConfigFile("getAccelCfg1.json", API_SUCCESS);
}

void LoggerApiTest::testSetAccelConfigFile(string filename){
	Serial *serial = getMockSerial();
	string json = readFile(filename);
	mock_resetTxBuffer();
	process_api(getMockSerial(),(char *)json.c_str(), json.size());

	LoggerConfig *c = getWorkingLoggerConfig();

	AccelConfig *accelCfg = &c->AccelConfigs[0];

	CPPUNIT_ASSERT_EQUAL(string("accel1"),string(accelCfg->cfg.label));
	CPPUNIT_ASSERT_EQUAL(50, decodeSampleRate(accelCfg->cfg.sampleRate));
	CPPUNIT_ASSERT_EQUAL(string("GG"),string(accelCfg->cfg.units));
	CPPUNIT_ASSERT_EQUAL(1, (int)accelCfg->mode);
	CPPUNIT_ASSERT_EQUAL(2, (int)accelCfg->accelChannel);
	CPPUNIT_ASSERT_EQUAL(1234, (int)accelCfg->zeroValue);

	char *txBuffer = mock_getTxBuffer();
	assertGenericResponse(txBuffer, "setAccelCfg", API_SUCCESS);
}

void LoggerApiTest::testSetAccelCfg(){
	testSetAccelConfigFile("setAccelCfg1.json");
}

void LoggerApiTest::testSetCellConfigFile(string filename){
	Serial *serial = getMockSerial();
	string json = readFile(filename);
	mock_resetTxBuffer();
	process_api(getMockSerial(),(char *)json.c_str(), json.size());

	LoggerConfig *c = getWorkingLoggerConfig();

	CellularConfig *cellCfg = &c->ConnectivityConfigs.cellularConfig;

	CPPUNIT_ASSERT_EQUAL(string("foo.xyz"), string(cellCfg->apnHost));
	CPPUNIT_ASSERT_EQUAL(string("blarg"), string(cellCfg->apnUser));
	CPPUNIT_ASSERT_EQUAL(string("blorg"), string(cellCfg->apnPass));

	char *txBuffer = mock_getTxBuffer();
	assertGenericResponse(txBuffer, "setCellCfg", API_SUCCESS);
}

void LoggerApiTest::testSetCellCfg()
{
	testSetCellConfigFile("setCellCfg1.json");
}

void LoggerApiTest::testGetCellCfg(){
	LoggerConfig *c = getWorkingLoggerConfig();
	CellularConfig *cellCfg = &c->ConnectivityConfigs.cellularConfig;
	strcpy(cellCfg->apnHost, "my.host");
	strcpy(cellCfg->apnUser, "user1");
	strcpy(cellCfg->apnPass, "pass1");

	char *response = processApiGeneric("getCellCfg1.json");
	Object json;
	stringToJson(response, json);

	Object &cellJson = json["getCellCfg"];

	string host = (String)cellJson["apnHost"];
	string user = (String)cellJson["apnUser"];
	string pass = (String)cellJson["apnPass"];

	CPPUNIT_ASSERT_EQUAL(string("my.host"), host);
	CPPUNIT_ASSERT_EQUAL(string("user1"), user);
	CPPUNIT_ASSERT_EQUAL(string("pass1"), pass);

}

void LoggerApiTest::testSetBtConfigFile(string filename){
	Serial *serial = getMockSerial();
	string json = readFile(filename);
	mock_resetTxBuffer();
	process_api(getMockSerial(),(char *)json.c_str(), json.size());

	LoggerConfig *c = getWorkingLoggerConfig();

	BluetoothConfig *btCfg = &c->ConnectivityConfigs.bluetoothConfig;

	CPPUNIT_ASSERT_EQUAL(string("myRacecar"), string(btCfg->deviceName));
	CPPUNIT_ASSERT_EQUAL(string("3311"), string(btCfg->passcode));
}

void LoggerApiTest::testSetBtCfg()
{
	testSetBtConfigFile("setBtCfg1.json");
}

void LoggerApiTest::testGetBtCfg(){
	LoggerConfig *c = getWorkingLoggerConfig();
	BluetoothConfig *btCfg = &c->ConnectivityConfigs.bluetoothConfig;
	strcpy(btCfg->deviceName,"myRacecar");
	strcpy(btCfg->passcode,"3311");

	char *response = processApiGeneric("getBtCfg1.json");
	Object json;
	stringToJson(response, json);

	Object &btJson = json["getBtCfg"];

	string name = (String)btJson["btName"];
	string pass = (String)btJson["btPass"];

	CPPUNIT_ASSERT_EQUAL(string("myRacecar"), name);
	CPPUNIT_ASSERT_EQUAL(string("3311"), pass);
}


void LoggerApiTest::testSetConnectivityCfgFile(string filename){
	LoggerConfig *c = getWorkingLoggerConfig();
	ConnectivityConfig *connCfg = &c->ConnectivityConfigs;
	connCfg->sdLoggingMode = 0;
	connCfg->connectivityMode = 0;
	connCfg->backgroundStreaming = 0;

	processApiGeneric(filename);

	CPPUNIT_ASSERT_EQUAL(1, (int)connCfg->sdLoggingMode);
	CPPUNIT_ASSERT_EQUAL(1, (int)connCfg->connectivityMode);
	CPPUNIT_ASSERT_EQUAL(1, (int)connCfg->backgroundStreaming);

	char *txBuffer = mock_getTxBuffer();
	assertGenericResponse(txBuffer, "setConnCfg", API_SUCCESS);
}

void LoggerApiTest::testSetConnectivityCfg(){
	testSetConnectivityCfgFile("setConnCfg1.json");
}

void LoggerApiTest::testGetConnectivityCfg(){
	LoggerConfig *c = getWorkingLoggerConfig();
	ConnectivityConfig *btCfg = &c->ConnectivityConfigs;
	btCfg->sdLoggingMode = 0;
	btCfg->connectivityMode = 2;

	char *response = processApiGeneric("getConnCfg1.json");
	Object json;
	stringToJson(response, json);

	Object &btJson = json["getConnCfg"];

	int sdMode = (Number)btJson["sdMode"];
	int connMode = (Number)btJson["connMode"];

	CPPUNIT_ASSERT_EQUAL(0, sdMode);
	CPPUNIT_ASSERT_EQUAL(2, connMode);
}

void LoggerApiTest::testGetPwmConfigFile(string filename, int index){
	LoggerConfig *c = getWorkingLoggerConfig();
	PWMConfig *pwmCfg = &c->PWMConfigs[index];

	strcpy(pwmCfg->cfg.label, "pLabel");
	strcpy(pwmCfg->cfg.units, "pUnits");
	pwmCfg->cfg.sampleRate = 100;
	pwmCfg->loggingPrecision = 2;
	pwmCfg->outputMode = 3;
	pwmCfg->loggingMode = 1;
	pwmCfg->startupDutyCycle = 55;
	pwmCfg->startupPeriod = 321;
	pwmCfg->voltageScaling = 1.23;

	char * response = processApiGeneric(filename);

	Object json;
	stringToJson(response, json);

	std::ostringstream stringStream;
	stringStream << index;
	Object &analogJson = json["getPwmCfg"][stringStream.str()];

	CPPUNIT_ASSERT_EQUAL(string("pLabel"), string((String)analogJson["nm"]));
	CPPUNIT_ASSERT_EQUAL(string("pUnits"), string((String)analogJson["ut"]));
	CPPUNIT_ASSERT_EQUAL(100, (int)(Number)analogJson["sr"]);
	CPPUNIT_ASSERT_EQUAL(2, (int)(Number)analogJson["logPrec"]);
	CPPUNIT_ASSERT_EQUAL(3, (int)(Number)analogJson["outMode"]);
	CPPUNIT_ASSERT_EQUAL(1, (int)(Number)analogJson["logMode"]);
	CPPUNIT_ASSERT_EQUAL(55, (int)(Number)analogJson["stDutyCyc"]);
	CPPUNIT_ASSERT_EQUAL(321, (int)(Number)analogJson["stPeriod"]);
	CPPUNIT_ASSERT_EQUAL(1.23F, (float)(Number)analogJson["vScal"]);
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
	CPPUNIT_ASSERT_EQUAL(string("pwm1"),string(pwmCfg->cfg.label));
	CPPUNIT_ASSERT_EQUAL(100, decodeSampleRate(pwmCfg->cfg.sampleRate));
	CPPUNIT_ASSERT_EQUAL(string("ut1"),string(pwmCfg->cfg.units));
	CPPUNIT_ASSERT_EQUAL(2, (int)pwmCfg->loggingPrecision);
	CPPUNIT_ASSERT_EQUAL(1, (int)pwmCfg->outputMode);
	CPPUNIT_ASSERT_EQUAL(1, (int)pwmCfg->loggingMode);
	CPPUNIT_ASSERT_EQUAL(50, (int)pwmCfg->startupDutyCycle);
	CPPUNIT_ASSERT_EQUAL(110, (int)pwmCfg->startupPeriod);
	CPPUNIT_ASSERT_EQUAL(2.5F, (float)pwmCfg->voltageScaling);

	char *txBuffer = mock_getTxBuffer();
	assertGenericResponse(txBuffer, "setPwmCfg", API_SUCCESS);

}

void LoggerApiTest::testSetPwmCfg(){
	testSetPwmConfigFile("setPwmCfg1.json");
}

void LoggerApiTest::testGetGpioConfigFile(string filename, int index){
	LoggerConfig *c = getWorkingLoggerConfig();
	GPIOConfig *gpioCfg = &c->GPIOConfigs[index];

	strcpy(gpioCfg->cfg.label, "gLabel");
	strcpy(gpioCfg->cfg.units, "gUnits");
	gpioCfg->cfg.sampleRate = 100;
	gpioCfg->mode = 1;

	char * response = processApiGeneric(filename);

	Object json;
	stringToJson(response, json);

	std::ostringstream stringStream;
	stringStream << index;
	Object &analogJson = json["getGpioCfg"][stringStream.str()];

	CPPUNIT_ASSERT_EQUAL(string("gLabel"), string((String)analogJson["nm"]));
	CPPUNIT_ASSERT_EQUAL(string("gUnits"), string((String)analogJson["ut"]));
	CPPUNIT_ASSERT_EQUAL(100, (int)(Number)analogJson["sr"]);
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

	CPPUNIT_ASSERT_EQUAL(string("gpio1"),string(gpioCfg->cfg.label));
	CPPUNIT_ASSERT_EQUAL(100, decodeSampleRate(gpioCfg->cfg.sampleRate));
	CPPUNIT_ASSERT_EQUAL(string("ut1"),string(gpioCfg->cfg.units));
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

	strcpy(timerCfg->cfg.label, "gLabel");
	strcpy(timerCfg->cfg.units, "gUnits");
	timerCfg->cfg.sampleRate = 100;
	timerCfg->loggingPrecision = 3;
	timerCfg->slowTimerEnabled = 1;
	timerCfg->mode = 2;
	timerCfg->pulsePerRevolution = 3;
	timerCfg->timerDivider = 30000;

	char * response = processApiGeneric(filename);

	Object json;
	stringToJson(response, json);

	std::ostringstream stringStream;
	stringStream << index;
	Object &analogJson = json["getTimerCfg"][stringStream.str()];

	CPPUNIT_ASSERT_EQUAL(string("gLabel"), string((String)analogJson["nm"]));
	CPPUNIT_ASSERT_EQUAL(string("gUnits"), string((String)analogJson["ut"]));
	CPPUNIT_ASSERT_EQUAL(100, (int)(Number)analogJson["sr"]);
	CPPUNIT_ASSERT_EQUAL(3, (int)(Number)analogJson["prec"]);
	CPPUNIT_ASSERT_EQUAL(1, (int)(Number)analogJson["sTimer"]);
	CPPUNIT_ASSERT_EQUAL(2, (int)(Number)analogJson["mode"]);
	CPPUNIT_ASSERT_EQUAL(3, (int)(Number)analogJson["ppRev"]);
	CPPUNIT_ASSERT_EQUAL(30000, (int)(Number)analogJson["timDiv"]);
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

	CPPUNIT_ASSERT_EQUAL(string("timer1"),string(timerCfg->cfg.label));
	CPPUNIT_ASSERT_EQUAL(10, decodeSampleRate(timerCfg->cfg.sampleRate));
	CPPUNIT_ASSERT_EQUAL(string("ut1"),string(timerCfg->cfg.units));
	CPPUNIT_ASSERT_EQUAL(3, (int)timerCfg->loggingPrecision);
	CPPUNIT_ASSERT_EQUAL(1, (int)timerCfg->slowTimerEnabled);
	CPPUNIT_ASSERT_EQUAL(1, (int)timerCfg->mode);
	CPPUNIT_ASSERT_EQUAL(4, (int)timerCfg->pulsePerRevolution);
	CPPUNIT_ASSERT_EQUAL(2, (int)timerCfg->timerDivider);

	char *txBuffer = mock_getTxBuffer();
	assertGenericResponse(txBuffer, "setTimerCfg", API_SUCCESS);
}

void LoggerApiTest::testSetTimerCfg(){
	testSetTimerConfigFile("setTimerCfg1.json");
}

void LoggerApiTest::testGetMeta(){
	testSampleDataFile("getMeta.json", "getMeta_response.json");
}

void LoggerApiTest::testSampleData(){
	testSampleDataFile("sampleData1.json", "sampleData_response1.json");
	testSampleDataFile("sampleData2.json", "sampleData_response2.json");
}

void LoggerApiTest::testSampleDataFile(string requestFilename, string responseFilename){

	string requestJson = readFile(requestFilename);
	string responseJson = readFile(responseFilename);

	mock_resetTxBuffer();
	process_api(getMockSerial(),(char *)requestJson.c_str(), requestJson.size());

	LoggerConfig *c = getWorkingLoggerConfig();
	char *txBuffer = mock_getTxBuffer();
	string txString(txBuffer);
	CPPUNIT_ASSERT_EQUAL(responseJson, txString );
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

void LoggerApiTest::testCalibrateAccelFile(string filename){

	string json = readFile(filename);
	mock_resetTxBuffer();
	process_api(getMockSerial(), (char *)json.c_str(), json.size());
	char *txBuffer = mock_getTxBuffer();
	assertGenericResponse(txBuffer,"calAccel",1);
}

void LoggerApiTest::testCalibrateAccel(){
	testCalibrateAccelFile("calibrateAccel.json");
}

void LoggerApiTest::testFlashConfigFile(string filename){
	mock_setIsFlashed(1);
	string json = readFile(filename);
	mock_resetTxBuffer();
	process_api(getMockSerial(), (char *)json.c_str(), json.size());
	char *txBuffer = mock_getTxBuffer();
	int isFlashed = mock_getIsFlashed();
	CPPUNIT_ASSERT_EQUAL(isFlashed, 1);
	assertGenericResponse(txBuffer,"flashCfg",1);
}

void LoggerApiTest::testFlashConfig(){
	testFlashConfigFile("flashCfg.json");
}

void LoggerApiTest::testSetGpsConfigFile(string filename){
	processApiGeneric(filename);
	char *txBuffer = mock_getTxBuffer();

	LoggerConfig *c = getWorkingLoggerConfig();
	GPSConfig *gpsCfg = &c->GPSConfigs;

	assertGenericResponse(txBuffer, "setGpsCfg", API_SUCCESS);

	CPPUNIT_ASSERT_EQUAL(string("latNm"), string(gpsCfg->latitudeCfg.label));
	CPPUNIT_ASSERT_EQUAL(string("latUt"), string(gpsCfg->latitudeCfg.units));
	CPPUNIT_ASSERT_EQUAL(10, decodeSampleRate(gpsCfg->latitudeCfg.sampleRate));

	CPPUNIT_ASSERT_EQUAL(string("longNm"), string(gpsCfg->longitudeCfg.label));
	CPPUNIT_ASSERT_EQUAL(string("longUt"), string(gpsCfg->longitudeCfg.units));
	CPPUNIT_ASSERT_EQUAL(20, decodeSampleRate(gpsCfg->longitudeCfg.sampleRate));

	CPPUNIT_ASSERT_EQUAL(string("speedNm"), string(gpsCfg->speedCfg.label));
	CPPUNIT_ASSERT_EQUAL(string("speedUt"), string(gpsCfg->speedCfg.units));
	CPPUNIT_ASSERT_EQUAL(30, decodeSampleRate(gpsCfg->speedCfg.sampleRate));

	CPPUNIT_ASSERT_EQUAL(string("timeNm"), string(gpsCfg->timeCfg.label));
	CPPUNIT_ASSERT_EQUAL(string("timeUt"), string(gpsCfg->timeCfg.units));
	CPPUNIT_ASSERT_EQUAL(50, decodeSampleRate(gpsCfg->timeCfg.sampleRate));

	CPPUNIT_ASSERT_EQUAL(string("satsNm"), string(gpsCfg->satellitesCfg.label));
	CPPUNIT_ASSERT_EQUAL(string("satsUt"), string(gpsCfg->satellitesCfg.units));
	CPPUNIT_ASSERT_EQUAL(100, decodeSampleRate(gpsCfg->satellitesCfg.sampleRate));
}

void LoggerApiTest::testSetGpsCfg(){
	testSetGpsConfigFile("setGpsCfg1.json");
}

void LoggerApiTest::testGetGpsConfigFile(string filename){
	LoggerConfig *c = getWorkingLoggerConfig();
	GPSConfig *gpsCfg = &c->GPSConfigs;

	ChannelConfig *latCfg = &gpsCfg->latitudeCfg;
	ChannelConfig *longCfg = &gpsCfg->longitudeCfg;
	ChannelConfig *speedCfg = &gpsCfg->speedCfg;
	ChannelConfig *timeCfg = &gpsCfg->timeCfg;
	ChannelConfig *satsCfg = &gpsCfg->satellitesCfg;

	strcpy(latCfg->label, "latNm");
	strcpy(latCfg->units, "latUt");
	latCfg->sampleRate = 10;

	strcpy(longCfg->label, "longNm");
	strcpy(longCfg->units, "longUt");
	longCfg->sampleRate = 20;

	strcpy(speedCfg->label, "speedNm");
	strcpy(speedCfg->units, "speedUt");
	speedCfg->sampleRate = 30;

	strcpy(timeCfg->label, "timeNm");
	strcpy(timeCfg->units, "timeUt");
	timeCfg->sampleRate = 50;

	strcpy(satsCfg->label, "satsNm");
	strcpy(satsCfg->units, "satsUt");
	satsCfg->sampleRate = 100;

	char * response = processApiGeneric(filename);

	Object json;
	stringToJson(response, json);

	Object &latJson = json["getGpsCfg"]["lat"];
	Object &longJson = json["getGpsCfg"]["long"];
	Object &speedJson = json["getGpsCfg"]["speed"];
	Object &timeJson = json["getGpsCfg"]["time"];
	Object &satsJson = json["getGpsCfg"]["sats"];

	CPPUNIT_ASSERT_EQUAL(string("latNm"), string((String)latJson["nm"]));
	CPPUNIT_ASSERT_EQUAL(string("latUt"), string((String)latJson["ut"]));
	CPPUNIT_ASSERT_EQUAL(10, (int)(Number)latJson["sr"]);

	CPPUNIT_ASSERT_EQUAL(string("longNm"), string((String)longJson["nm"]));
	CPPUNIT_ASSERT_EQUAL(string("longUt"), string((String)longJson["ut"]));
	CPPUNIT_ASSERT_EQUAL(20, (int)(Number)longJson["sr"]);

	CPPUNIT_ASSERT_EQUAL(string("speedNm"), string((String)speedJson["nm"]));
	CPPUNIT_ASSERT_EQUAL(string("speedUt"), string((String)speedJson["ut"]));
	CPPUNIT_ASSERT_EQUAL(30, (int)(Number)speedJson["sr"]);

	CPPUNIT_ASSERT_EQUAL(string("timeNm"), string((String)timeJson["nm"]));
	CPPUNIT_ASSERT_EQUAL(string("timeUt"), string((String)timeJson["ut"]));
	CPPUNIT_ASSERT_EQUAL(50, (int)(Number)timeJson["sr"]);

	CPPUNIT_ASSERT_EQUAL(string("satsNm"), string((String)satsJson["nm"]));
	CPPUNIT_ASSERT_EQUAL(string("satsUt"), string((String)satsJson["ut"]));
	CPPUNIT_ASSERT_EQUAL(100, (int)(Number)satsJson["sr"]);
}

void LoggerApiTest::testGetGpsCfg(){
	testGetGpsConfigFile("getGpsCfg1.json");
}

void LoggerApiTest::testSetTrackConfigFile(string filename){
	processApiGeneric(filename);
	char *txBuffer = mock_getTxBuffer();

	LoggerConfig *c = getWorkingLoggerConfig();
	TrackConfig *cfg = &c->TrackConfigs;

	assertGenericResponse(txBuffer, "setTrackCfg", API_SUCCESS);

	CPPUNIT_ASSERT_EQUAL(38.161518F, cfg->startFinishConfig.latitude);
	CPPUNIT_ASSERT_EQUAL(-122.454711F, cfg->startFinishConfig.longitude);
	CPPUNIT_ASSERT_EQUAL(0.0006F, cfg->startFinishConfig.targetRadius);

	CPPUNIT_ASSERT_EQUAL(38.166389F, cfg->splitConfig.latitude);
	CPPUNIT_ASSERT_EQUAL(-122.462442F, cfg->splitConfig.longitude);
	CPPUNIT_ASSERT_EQUAL(0.0006F, cfg->splitConfig.targetRadius);
}

void LoggerApiTest::testSetTrackCfg(){
	testSetTrackConfigFile("setTrackCfg1.json");
}

void LoggerApiTest::testGetTrackConfigFile(string filename){
	LoggerConfig *c = getWorkingLoggerConfig();
	TrackConfig *cfg = &c->TrackConfigs;

	cfg->startFinishConfig.latitude = 1.234;
	cfg->startFinishConfig.longitude = 5.678;
	cfg->startFinishConfig.targetRadius = 0.001;

	cfg->splitConfig.latitude = 11.234;
	cfg->splitConfig.longitude = 55.678;
	cfg->splitConfig.targetRadius = 0.002;

	char * response = processApiGeneric(filename);

	Object json;
	stringToJson(response, json);

	Object &startFinish = json["getTrackCfg"]["startFinish"];
	Object &split = json["getTrackCfg"]["split"];

	CPPUNIT_ASSERT_EQUAL(1.234F, (float)(Number)startFinish["lat"]);
	CPPUNIT_ASSERT_EQUAL(5.678F, (float)(Number)startFinish["long"]);
	CPPUNIT_ASSERT_EQUAL(0.001F, (float)(Number)startFinish["rad"]);

	CPPUNIT_ASSERT_EQUAL(11.234F, (float)(Number)split["lat"]);
	CPPUNIT_ASSERT_EQUAL(55.678F, (float)(Number)split["long"]);
	CPPUNIT_ASSERT_EQUAL(0.002F, (float)(Number)split["rad"]);
}

void LoggerApiTest::testGetTrackCfg(){
	testGetTrackConfigFile("getTrackCfg1.json");
}
