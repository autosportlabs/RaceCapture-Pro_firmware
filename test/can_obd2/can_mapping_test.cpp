/*
 * Race Capture Firmware
 *
 * Copyright (C) 2016 Autosport Labs
 *
 * This file is part of the Race Capture firmware suite
 *
 * This is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should
 * have received a copy of the GNU General Public License along with
 * this code. If not, see <http://www.gnu.org/licenses/>.
 */

#include "can_mapping.h"
#include "can_mapping_test.h"

#include <cppunit/extensions/HelperMacros.h>
/* Inclue the code to test here */
//extern "C" {
////#include "CAN/can_mapping.c"
//}
CPPUNIT_TEST_SUITE_REGISTRATION( CANMappingTest );


void CANMappingTest::mapping_test()
{
    for (uint8_t length = 1; length <= 1; length++ ) {
        for (size_t offset = 0; offset < 7; offset++) {
                CAN_msg msg;
                CANMapping mapping;
                mapping.offset = offset;
                mapping.length = length;
                //memset(&msg, 0, sizeof(CAN_msg));
                msg.data[offset] = 0x55;
                float value = foo();
               // float value = map_value(&msg, &mapping);
                printf("value %f", value);
        }
    }

//    CPPUNIT_ASSERT_EQUAL(false, should_start_logging(&gps_sample, t1));
}


