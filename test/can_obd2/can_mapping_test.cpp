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
#include <string.h>
#include <cppunit/extensions/HelperMacros.h>
#include "byteswap.h"
#include <stdlib.h>

/*
 * #define CAN_MAPPING_TEST_DEBUG
 */


CPPUNIT_TEST_SUITE_REGISTRATION( CANMappingTest );


void CANMappingTest::formula_test(void)
{
       CANMapping mapping;
        {
                mapping.multiplier = 0.0f;
                mapping.adder = 0.0f;
                mapping.divider = 0.0f;
                float transformed = canmapping_apply_formula(1.0, &mapping);
                CPPUNIT_ASSERT_EQUAL(0.0f, transformed);
        }
        {
                mapping.multiplier = 1.0f;
                mapping.adder = 0.0f;
                mapping.divider = 0.0f;
                float transformed = canmapping_apply_formula(1.0, &mapping);
                CPPUNIT_ASSERT_EQUAL(1.0f, transformed);
        }
        {
                mapping.multiplier = 1.0f;
                mapping.adder = 1.0f;
                mapping.divider = 0.0f;
                float transformed = canmapping_apply_formula(1.0, &mapping);
                CPPUNIT_ASSERT_EQUAL(2.0f, transformed);
        }
        {
                mapping.multiplier = 1.0f;
                mapping.adder = 0.0f;
                mapping.divider = 2.0f;
                float transformed = canmapping_apply_formula(1.0, &mapping);
                CPPUNIT_ASSERT_EQUAL(0.5f, transformed);
        }
        {
                mapping.multiplier = 1.0f;
                mapping.adder = 1.0f;
                mapping.divider = 2.0f;
                float transformed = canmapping_apply_formula(1.0, &mapping);
                CPPUNIT_ASSERT_EQUAL(1.5f, transformed);
        }
}

void CANMappingTest::extract_test_bit_mode(void)
{
        srand(time(NULL));

        for (size_t bitpattern = 0; bitpattern <= 1; bitpattern++){
                for (size_t endian = 0; endian <= 1; endian++){
                        for (uint8_t length = 0; length <= 32; length++ ) {

                                uint64_t test_value;
                                if (!bitpattern) {
                                        /* just test a bit pattern of all 1's */
                                        test_value = ((uint64_t)1 << length) - 1;
                                }
                                else {
                                        /* test with bit pattern ... */
                                        test_value = 0xAA55AA55AA55AA55 & (((uint64_t)1 << length) - 1);
                                }

                                uint64_t encoded_value = test_value;
                                if (endian == 0) {
                                        /* perform byte oriented endian flip that accounts for variable bit length */
                                        if (length > 8 && length <=16) {
                                                encoded_value = ((encoded_value & 0xFF) << (length - 8)) + (encoded_value >> 8);
                                        }
                                        if (length > 16 && length <=24) {
                                                encoded_value = (swap_uint16(encoded_value) << (length - 16)) + (encoded_value >> 16);
                                        }
                                        if (length > 24 && length <=32) {
                                                encoded_value = (swap_uint24(encoded_value) << (length - 24)) + (encoded_value >> 24);
                                        }
                                }

                                /* shift it all the way to the left */
                                uint64_t shifted_test_value = encoded_value << (64-length);

                                for (uint8_t offset = 0; offset < (CAN_MSG_SIZE * 8) - length + 1; offset++) {
                                        CAN_msg msg;
                                        CANMapping mapping;
                                        memset(&mapping, 0, sizeof(mapping));
                                        memset(&msg, 0, sizeof(CAN_msg));
                                        mapping.offset = offset;
                                        mapping.length = length;
                                        mapping.type = CANMappingType_unsigned;
                                        mapping.bit_mode = true;
                                        mapping.big_endian = (bool)endian;

                                        /* now shift the value by the offset */
                                        uint64_t offset_test_value = shifted_test_value >> offset;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
                                        /* account for platform's endian-ness */
                                        offset_test_value = swap_uint64(offset_test_value);
#endif
                                        msg.data64 = offset_test_value;

                                        float value = canmapping_extract_value(msg.data64, &mapping);

                                        /* prepare the comparison value */
//#ifdef CAN_MAPPING_TEST_DEBUG
                                        printf("bitmode test: endian=%u / test_value=%lu / offset=%d / length=%d / return = %f\r\n" ,
                                               mapping.big_endian, test_value, offset, length, value);
                                        printf("CAN data: ");
                                        for (size_t di = 0; di < CAN_MSG_SIZE; di++){
                                                uint8_t d = msg.data[di];
                                                printf("%02x ", d);
                                        }
                                        printf("\r\n");

                                        for (size_t di = 0; di < CAN_MSG_SIZE; di++){
                                                uint8_t d = msg.data[di];
                                                static char b[9];
                                                b[0] = '\0';
                                                for (int z = 128; z > 0; z >>= 1)
                                                {
                                                    strcat(b, ((d & z) == z) ? "1" : "0");
                                                }
                                                printf("%s ", b);
                                        }
                                        printf("\r\n");
//#endif

                                        CPPUNIT_ASSERT_EQUAL((float)test_value, value);
                                }
                        }
                }
        }
}

void CANMappingTest::extract_test(void)
{
        /*
         * This test sweeps through every offset and data lengths of 1-4 bytes,
         * testing the ability to extract values from anywhere in the message.
         */
        for (size_t endian = 0; endian <= 1; endian++){
                uint32_t test_value = 0;
                for (uint8_t length = 1; length <= 4; length++ ) {
                        /* make a test pattern like 0x01020304 */
                        test_value = (test_value << 8) + length;
                        uint32_t can_value = test_value;
                        if (!endian) {
                                can_value = decode_little_endian_bitmode(can_value, length * 8);
                        }

                        for (uint8_t offset = 0; offset < CAN_MSG_SIZE - length + 1; offset++) {
                                CAN_msg msg;
                                CANMapping mapping;
                                memset(&mapping, 0, sizeof(mapping));
                                memset(&msg, 0, sizeof(CAN_msg));
                                mapping.offset = offset;
                                mapping.length = length;
                                mapping.type = CANMappingType_unsigned;
                                mapping.big_endian = (bool)endian;


                                /* populate byte values starting at offset */
                                for (size_t l = 0; l < length; l++) {
                                        msg.data[offset + length - l - 1] = (can_value >> l * 8) & 0xff;
                                }
                                float value = canmapping_extract_value(msg.data64, &mapping);

#ifdef CAN_MAPPING_TEST_DEBUG
                                printf("endian=%d / test value=%d / offset=%d / length=%d / return=%f\r\n" ,
                                       endian, test_value, offset, length, value);
                                printf("CAN data: ");
                                for (size_t di = 0; di < CAN_MSG_SIZE; di++){
                                        printf("%2x ", msg.data[di]);
                                }
                                printf("\r\n");
#endif


                               CPPUNIT_ASSERT_EQUAL((float)test_value, value);
                        }
                }
        }
}

void CANMappingTest::extract_type_test(void)
{
    CAN_msg msg;
    CANMapping mapping;
    memset(&mapping, 0, sizeof(mapping));
    memset(&msg, 0, sizeof(CAN_msg));
    mapping.offset = 0;
    mapping.length = 1;
    mapping.bit_mode = false;
    mapping.big_endian = false;

    /* 8 bit signed */
    msg.data[0] = 255;
    mapping.type = CANMappingType_signed;
    float value = canmapping_extract_value(msg.data64, &mapping);
    CPPUNIT_ASSERT_EQUAL((float)-1, value);

    /* 8 bit unsigned */
    mapping.type = CANMappingType_unsigned;
    value = canmapping_extract_value(msg.data64, &mapping);
    CPPUNIT_ASSERT_EQUAL((float)255, value);

    /* 16 bit signed */
    mapping.type = CANMappingType_signed;
    mapping.length = 2;
    msg.data[0] = 255;
    msg.data[1] = 255;
    value = canmapping_extract_value(msg.data64, &mapping);
    CPPUNIT_ASSERT_EQUAL((float)-1, value);

    /* 16 bit unsigned */
    mapping.type = CANMappingType_unsigned;
    value = canmapping_extract_value(msg.data64, &mapping);
    CPPUNIT_ASSERT_EQUAL((float)65535, value);

    /* 32 bit signed */
    mapping.type = CANMappingType_signed;
    mapping.length = 4;
    msg.data[0] = 255;
    msg.data[1] = 255;
    msg.data[2] = 255;
    msg.data[3] = 255;
    value = canmapping_extract_value(msg.data64, &mapping);
    CPPUNIT_ASSERT_EQUAL((float)-1, value);

    /* 32 bit unsigned */
    mapping.type = CANMappingType_unsigned;
    value = canmapping_extract_value(msg.data64, &mapping);
    CPPUNIT_ASSERT_EQUAL((float)4294967295, value);

    /* IEEE754 floating point*/
    mapping.type = CANMappingType_IEEE754;
    mapping.length = 4;
    /* binary representation of 100.0 */
    msg.data[0] = 0x00;
    msg.data[1] = 0x00;
    msg.data[2] = 0xC8;
    msg.data[3] = 0x42;
    value = canmapping_extract_value(msg.data64, &mapping);
    CPPUNIT_ASSERT_EQUAL((float)100.0, value);

    /* 8 bit sign-magnitude */
    mapping.type = CANMappingType_sign_magnitude;
    mapping.length = 1;
    msg.data[0] = 0x01;
    value = canmapping_extract_value(msg.data64, &mapping);
    CPPUNIT_ASSERT_EQUAL((float)1.0, value);

    msg.data[0] = 0x81;
    value = canmapping_extract_value(msg.data64, &mapping);
    CPPUNIT_ASSERT_EQUAL((float)-1.0, value);

    msg.data[0] = 0x0;
    value = canmapping_extract_value(msg.data64, &mapping);
    CPPUNIT_ASSERT_EQUAL((float)0.0, value);

    msg.data[0] = 0x80;
    value = canmapping_extract_value(msg.data64, &mapping);
    CPPUNIT_ASSERT_EQUAL((float)-0.0, value);

    /* 16 bit sign-magnitude */
    mapping.length = 2;
    msg.data[0] = 0x01;
    msg.data[1] = 0x00;
    value = canmapping_extract_value(msg.data64, &mapping);
    CPPUNIT_ASSERT_EQUAL((float)1.0, value);


    msg.data[0] = 0x01;
    msg.data[1] = 0x80;
    value = canmapping_extract_value(msg.data64, &mapping);
    CPPUNIT_ASSERT_EQUAL((float)-1.0, value);

    msg.data[0] = 0x00;
    msg.data[1] = 0x00;
    value = canmapping_extract_value(msg.data64, &mapping);
    CPPUNIT_ASSERT_EQUAL((float)0.0, value);

    msg.data[0] = 0x00;
    msg.data[1] = 0x80;
    value = canmapping_extract_value(msg.data64, &mapping);
    CPPUNIT_ASSERT_EQUAL((float)-0.0, value);

    /* 32 bit sign-magnitude */
    mapping.length = 4;
    msg.data[0] = 0x01;
    msg.data[1] = 0x00;
    msg.data[2] = 0x00;
    msg.data[3] = 0x00;
    value = canmapping_extract_value(msg.data64, &mapping);
    CPPUNIT_ASSERT_EQUAL((float)1.0, value);

    msg.data[0] = 0x01;
    msg.data[1] = 0x00;
    msg.data[2] = 0x00;
    msg.data[3] = 0x80;
    value = canmapping_extract_value(msg.data64, &mapping);
    CPPUNIT_ASSERT_EQUAL((float)-1.0, value);

    msg.data[0] = 0x00;
    msg.data[1] = 0x00;
    msg.data[2] = 0x00;
    msg.data[3] = 0x80;
    value = canmapping_extract_value(msg.data64, &mapping);
    CPPUNIT_ASSERT_EQUAL((float)0.0, value);

    msg.data[0] = 0x00;
    msg.data[1] = 0x00;
    msg.data[2] = 0x00;
    msg.data[3] = 0x80;
    value = canmapping_extract_value(msg.data64, &mapping);
    CPPUNIT_ASSERT_EQUAL((float)-0.0, value);
}


void CANMappingTest::id_match_test(void)
{
        {
                CAN_msg msg;
                CANMapping mapping;
                memset(&mapping, 0, sizeof(mapping));
                memset(&msg, 0, sizeof(CAN_msg));

                msg.addressValue = 0xFF;
                mapping.can_id = 0xFF;
                mapping.can_mask = 0;
                CPPUNIT_ASSERT_EQUAL(true, canmapping_match_id(&msg, &mapping));
                mapping.can_mask = 0xFF;
                CPPUNIT_ASSERT_EQUAL(true, canmapping_match_id(&msg, &mapping));
                mapping.can_mask = 0x01;
                CPPUNIT_ASSERT_EQUAL(false, canmapping_match_id(&msg, &mapping));

                msg.addressValue = 0xFF;
                mapping.can_id = 0xF0;
                mapping.can_mask = 0;
                CPPUNIT_ASSERT_EQUAL(false, canmapping_match_id(&msg, &mapping));

                msg.addressValue = 0xFFFF;
                mapping.can_id = 0xFFFF;
                mapping.can_mask = 0xFFFF;
                CPPUNIT_ASSERT_EQUAL(true, canmapping_match_id(&msg, &mapping));

                mapping.can_mask = 0xFFF0;
                CPPUNIT_ASSERT_EQUAL(false, canmapping_match_id(&msg, &mapping));

                msg.addressValue = 0xFFFF;
                mapping.can_id = 0xFFF0;
                mapping.can_mask = 0xFFFF;
                CPPUNIT_ASSERT_EQUAL(false, canmapping_match_id(&msg, &mapping));
        }
}

#define MAPPING_FORMULA(RAW, MULT, DIV, ADD) RAW * MULT / (DIV == 0 ? 1: DIV) + ADD

void CANMappingTest::mapping_test(void)
{
        /* this is a full end-end functional test for the mapper */
        CAN_msg msg;
        CANMapping mapping;
        memset(&mapping, 0, sizeof(mapping));
        memset(&msg, 0, sizeof(CAN_msg));

        /* set up message */
        msg.data[0] = 1;
        msg.data[1] = 2;
        msg.data[2] = 3;
        msg.data[3] = 4;
        msg.data[4] = 5;
        msg.data[5] = 6;
        msg.data[6] = 7;
        msg.data[7] = 8;
        msg.addressValue = 0x1122;

        /* set up mapping */
        mapping.big_endian = false;

        mapping.bit_mode = false;
        mapping.offset = 0;
        mapping.length = 1;

        float divider = 1.0;
        float multiplier = 10.0;
        float adder = 20.0;

        mapping.multiplier = multiplier;
        mapping.divider = divider;
        mapping.adder = adder;

        mapping.can_id = 0x1122;
        mapping.can_mask = 0;

        bool result;
        float value;

        /* a basic end-end test */
        result = canmapping_map_value(&value, &msg, &mapping);
        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL((float)MAPPING_FORMULA(0x01, multiplier, divider, adder), value);

        /* mask_filtering */
        mapping.can_mask = 0x1121;
        result = canmapping_map_value(&value, &msg, &mapping);
        CPPUNIT_ASSERT_EQUAL(false, result);

        mapping.can_mask = 0x1122;
        result = canmapping_map_value(&value, &msg, &mapping);
        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL((float)MAPPING_FORMULA(0x01, multiplier, divider, adder), value);

        /* multi byte */
        mapping.length = 2;
        result = canmapping_map_value(&value, &msg, &mapping);
        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL((float)MAPPING_FORMULA(0x0201, multiplier, divider, adder), value);

        /* big endian */
        mapping.big_endian = true;
        result = canmapping_map_value(&value, &msg, &mapping);
        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL((float)MAPPING_FORMULA(0x0102, multiplier, divider, adder), value);
}
