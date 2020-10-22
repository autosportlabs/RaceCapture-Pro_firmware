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
        CPPUNIT_TEST( id_match_test );
        CPPUNIT_TEST( mapping_test );
        CPPUNIT_TEST( formula_test );
        CPPUNIT_TEST( extract_test );
        CPPUNIT_TEST( extract_test_bit_mode );
        CPPUNIT_TEST( extract_type_test );
        CPPUNIT_TEST_SUITE_END();

public:
        void id_match_test(void);
        void mapping_test(void);
        void formula_test(void);
        void extract_test(void);
        void extract_test_bit_mode(void);
        void extract_type_test(void);
};

#endif /* TEST_CAN_OBD2_CAN_MAPPING_TEST_H_ */
