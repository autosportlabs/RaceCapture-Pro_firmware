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
  CPPUNIT_TEST( testSetAnalogCfg );
  CPPUNIT_TEST( testSetAccelCfg );
  CPPUNIT_TEST( testSetPwmCfg );
  CPPUNIT_TEST( testSetGpioCfg );
  CPPUNIT_TEST_SUITE_END();

public:
  int findAndReplace(string & source, const string find, const string replace);
  string readFile(string filename);
  void setUp();
  void tearDown();

  void testSetCellCfg();
  void testSetAnalogCfg();
  void testSetAccelCfg();
  void testSetPwmCfg();
  void testSetGpioCfg();

private:
  void testAnalogConfigFile(string filename);
  void testAccelConfigFile(string filename);
  void testSetCellConfigFile(string filename);
  void testSetPwmConfigFile(string filename);
  void testSetGpioConfigFile(string filename);


};



#endif /* LOGGERAPI_TEST_H_ */
