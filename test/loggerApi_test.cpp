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
	assertGenericResponse(txBuffer, "setAnalogCfg", 1);

}

void LoggerApiTest::testSetAnalogCfg()
{
	testSetAnalogConfigFile("setAnalogCfg1.json");
	testSetAnalogConfigFile("setAnalogCfg2.json");
	testSetAnalogConfigFile("setAnalogCfg3.json");
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
	assertGenericResponse(txBuffer, "setAccelCfg", 1);
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
	assertGenericResponse(txBuffer, "setCellCfg", 1);
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
	ConnectivityConfig *btCfg = &c->ConnectivityConfigs;
	btCfg->sdLoggingMode = 0;
	btCfg->connectivityMode = 0;

	processApiGeneric(filename);

	CPPUNIT_ASSERT_EQUAL(1, (int)btCfg->sdLoggingMode);
	CPPUNIT_ASSERT_EQUAL(1, (int)btCfg->connectivityMode);

	char *txBuffer = mock_getTxBuffer();
	assertGenericResponse(txBuffer, "setConnCfg", 1);
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
	PWMConfig *accelCfg = &c->PWMConfigs[index];

	strcpy(accelCfg->cfg.label, "pLabel");
	strcpy(accelCfg->cfg.units, "pUnits");
	accelCfg->cfg.sampleRate = 100;
	accelCfg->loggingPrecision = 2;
	accelCfg->outputMode = 3;
	accelCfg->loggingMode = 1;
	accelCfg->startupDutyCycle = 55;
	accelCfg->startupPeriod = 321;
	accelCfg->voltageScaling = 1.23;

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
	assertGenericResponse(txBuffer, "setPwmCfg", 1);

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
	assertGenericResponse(txBuffer, "setGpioCfg", 1);

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
	testGetTimerConfigFile("getTimerCfg1.json", 1);
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
	assertGenericResponse(txBuffer, "setTimerCfg", 1);
}

void LoggerApiTest::testSetTimerCfg(){
	testSetTimerConfigFile("setTimerCfg1.json");
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
}

void LoggerApiTest::testLogStartStopFile(string filename){

	string json = readFile(filename);
	mock_resetTxBuffer();
	process_api(getMockSerial(), (char *)json.c_str(), json.size());
	char *txBuffer = mock_getTxBuffer();
	assertGenericResponse(txBuffer,"log", 1);
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

