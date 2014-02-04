#ifndef SAMPLERECORD_TEST_H_
#define SAMPLERECORD_TEST_H_

#include <cppunit/extensions/HelperMacros.h>


class SampleRecordTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( SampleRecordTest );
  CPPUNIT_TEST( testInitSampleRecord );
  CPPUNIT_TEST( testPopulateSampleRecord );
  CPPUNIT_TEST_SUITE_END();

public:

  void setUp();
  void tearDown();
  void testInitSampleRecord();
  void testPopulateSampleRecord();

private:


};



#endif /* LOGGERAPI_TEST_H_ */
