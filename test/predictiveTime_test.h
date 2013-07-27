/*
 * predictive_time_test.h
 *
 *  Created on: Jul 24, 2013
 *      Author: brent
 */

#ifndef PREDICTIVE_TIME_TEST_H_
#define PREDICTIVE_TIME_TEST_H_

#include <cppunit/extensions/HelperMacros.h>

class PredictiveTimeTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( PredictiveTimeTest );
  CPPUNIT_TEST( testAddSamples );
  CPPUNIT_TEST( testPredictLapTime );

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testPredictLapTime();
  void testAddSamples();
};


#endif /* PREDICTIVE_TIME_TEST_H_ */
