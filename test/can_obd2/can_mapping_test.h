/*
 * can_mapping_test.h
 *
 *  Created on: Feb 20, 2017
 *      Author: brent
 */

#ifndef TEST_CAN_OBD2_CAN_MAPPING_TEST_H_
#define TEST_CAN_OBD2_CAN_MAPPING_TEST_H_

#include <cppunit/extensions/HelperMacros.h>

class CANMappingTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( CANMappingTest );
    CPPUNIT_TEST( mapping_test );
    CPPUNIT_TEST_SUITE_END();

public:
    void mapping_test();
};

#endif /* TEST_CAN_OBD2_CAN_MAPPING_TEST_H_ */
