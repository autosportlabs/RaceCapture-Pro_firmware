/**
 * Race Capture Pro Firmware
 *
 * Copyright (C) 2014 Autosport Labs
 *
 * This file is part of the Race Capture Pro fimrware suite
 *
 * This is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should have received a copy of the GNU
 * General Public License along with this code. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Stieg
 */


#include "byteswap_test.h"
#include "byteswap.h"

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( ByteswapTest );

void ByteswapTest::setUp() {}

void ByteswapTest::tearDown() {}

void ByteswapTest::test_int16_swap(void)
{
        CPPUNIT_ASSERT_EQUAL((int16_t)0x2211, swap_int16(0x1122));
}

void ByteswapTest::test_uint16_swap(void)
{
        CPPUNIT_ASSERT_EQUAL((uint16_t)0x2211, swap_uint16(0x1122));
}

void ByteswapTest::test_uint24_swap(void)
{
        CPPUNIT_ASSERT_EQUAL((uint32_t)0x332211, swap_uint24(0x112233));
}

void ByteswapTest::test_int32_swap(void)
{
        CPPUNIT_ASSERT_EQUAL((int32_t)0x44332211, swap_int32(0x11223344));
}

void ByteswapTest::test_uint32_swap(void)
{
        CPPUNIT_ASSERT_EQUAL((uint32_t)0x44332211, swap_uint32(0x11223344));
}
