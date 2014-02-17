/*
 * PredictiveTimeTest2.h
 *
 *  Created on: Feb 3, 2014
 *      Author: stieg
 */

#ifndef PREDICTIVETIMETEST2_H_
#define PREDICTIVETIMETEST2_H_

#include <cppunit/extensions/HelperMacros.h>
#include <string>
#include <stdlib.h>

using std::string;
using std::vector;

class PredictiveTimeTest2 : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE( PredictiveTimeTest2 );
	CPPUNIT_TEST( testPredictedTimeGpsFeed );
	CPPUNIT_TEST_SUITE_END();

public:
	PredictiveTimeTest2();
	void setUp();
	void tearDown();
	void testPredictedTimeGpsFeed();

private:
	string readFile(string filename);
	vector<string> split(string &s, char delim);
	vector<string> & split(string &s, char delim, vector<string> &elems);
};

#endif /* PREDICTIVETIMETEST2_H_ */
