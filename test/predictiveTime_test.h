/*
 * predictive_time_test.h
 *
 *  Created on: Jul 24, 2013
 *      Author: brent
 */

#ifndef PREDICTIVE_TIME_TEST_H_
#define PREDICTIVE_TIME_TEST_H_

#include <cppunit/extensions/HelperMacros.h>
#include <string>
using std::string;
using std::vector;

class PredictiveTimeTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( PredictiveTimeTest );
  CPPUNIT_TEST( testAddSamples );
  CPPUNIT_TEST( testPredictLapTime );
  CPPUNIT_TEST( testPredictedLapTimeFullLap );
  CPPUNIT_TEST( testPredictedTimeGpsFeed );

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  string readFile(string filename);
  vector<string> split(string &s, char delim);
  vector<string> &split(string &s, char delim, vector<string> &elems);

  void testPredictLapTime();
  void testPredictedLapTimeFullLap();
  void testPredictedTimeGpsFeed();
  void testAddSamples();
};


#endif /* PREDICTIVE_TIME_TEST_H_ */
