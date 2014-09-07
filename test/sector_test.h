#ifndef PREDICTIVE_TIME_TEST_H_
#define PREDICTIVE_TIME_TEST_H_

#include <cppunit/extensions/HelperMacros.h>
#include <string>
using std::string;
using std::vector;

class SectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( SectorTest );
  CPPUNIT_TEST( testSectorTimes );
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  string readFile(string filename);
  vector<string> split(string &s, char delim);
  vector<string> &split(string &s, char delim, vector<string> &elems);

  float sumSectorTimes(vector<float> & sectorTimes);
  void outputSectorTimes(vector<float> & sectorTimes, int lap);
  void testSectorTimes();
};


#endif /* PREDICTIVE_TIME_TEST_H_ */
