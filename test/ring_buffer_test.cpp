/*
 * Race Capture Pro Firmware
 *
 * Copyright (C) 2015 Autosport Labs
 *
 * This file is part of the Race Capture Pro fimrware suite
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

#include "ring_buffer.h"
#include "ring_buffer_test.hh"

#include <string.h>
#include <stdio.h>

CPPUNIT_TEST_SUITE_REGISTRATION( RingBufferTest );

static const size_t buff_size = 8;
static char buff[buff_size];
static struct ring_buff rb;

void RingBufferTest::setUp()
{
        init_ring_buffer(&rb, buff, buff_size);

        /* To better test wrapping, move head and tail forward */
        rb.head++;
        rb.tail++;
}

void RingBufferTest::tearDown() {}

void RingBufferTest::putGetTest()
{
        char data_in[] = "FOO";
        const size_t size = sizeof(data_in);
        char data_out[size];

        /* Check initial sizes */
        CPPUNIT_ASSERT_EQUAL(buff_size - 1, get_space(&rb));
        CPPUNIT_ASSERT_EQUAL((size_t) 0, get_used(&rb));

        /* Put in the data */
        CPPUNIT_ASSERT_EQUAL(size, put_data(&rb, data_in, size));

        /* Check the sizes now */
        CPPUNIT_ASSERT_EQUAL(buff_size - 1 - size, get_space(&rb));
        CPPUNIT_ASSERT_EQUAL(size, get_used(&rb));

        /* Get the data */
        CPPUNIT_ASSERT_EQUAL(size, get_data(&rb, data_out, size));

        /* Check final sizes */
        CPPUNIT_ASSERT_EQUAL(buff_size - 1, get_space(&rb));
        CPPUNIT_ASSERT_EQUAL((size_t) 0, get_used(&rb));

        /* Check data in == data out */
        CPPUNIT_ASSERT(0 == strcmp(data_in, data_out));
}

void RingBufferTest::putStringTest()
{
        const char str[] = "FooBarBazBizFizzBah";
        const char *ptr = str;

        /* First addition should result in failure */
        ptr = put_string(&rb, ptr);
        CPPUNIT_ASSERT_EQUAL(str + 7, ptr);

        /* Now dump 7 bytes and try to add more */
        CPPUNIT_ASSERT_EQUAL((size_t) 7, dump_data(&rb, 7));

        /* Second addition should result in failure as well. */
        ptr = put_string(&rb, ptr);
        CPPUNIT_ASSERT_EQUAL(str + 14, ptr);

        /* Now dump 7 bytes again and try to add more */
        CPPUNIT_ASSERT_EQUAL((size_t) 7, dump_data(&rb, 7));

        /* The final addition should result in success. */
        ptr = put_string(&rb, ptr);
        CPPUNIT_ASSERT_EQUAL((const char *) NULL, ptr);
}

void RingBufferTest::putFailTest()
{
        /* Fill up the buffer */
        for(size_t i = 0; i < buff_size - 1; ++i)
                CPPUNIT_ASSERT_EQUAL((size_t) 1, put_data(&rb, "F", 1));

        /* Check that we have no space */
        CPPUNIT_ASSERT_EQUAL(false, have_space(&rb, 1));

        /* Now it should fail to put */
        CPPUNIT_ASSERT_EQUAL((size_t) 0, put_data(&rb, "F", 1));
}

void RingBufferTest::getFailTest()
{
        char buff[1];

        /* Clear our ring bufer (should be already) */
        CPPUNIT_ASSERT_EQUAL((size_t) 0, clear_data(&rb));

        /* Check that we don't get anything */
        CPPUNIT_ASSERT_EQUAL((size_t) 0, get_data(&rb, buff, 1));
}

void RingBufferTest::dumpTest()
{
        const size_t toDump = 3;

        /* Sanity Check */
        CPPUNIT_ASSERT(toDump < buff_size - 1);

        /* Fill up the buffer */
        for(size_t i = 0; i < buff_size - 1; ++i)
                CPPUNIT_ASSERT_EQUAL((size_t) 1, put_data(&rb, "F", 1));

        /* Check that we have no space */
        CPPUNIT_ASSERT_EQUAL((size_t) 0, get_space(&rb));

        /* Dump toDump bytes */
        CPPUNIT_ASSERT_EQUAL(toDump, dump_data(&rb, toDump));

        /* Check that we have toDump space now */
        CPPUNIT_ASSERT_EQUAL(toDump, get_space(&rb));
}

void RingBufferTest::clearTest()
{
        /* Fill up the buffer */
        for(size_t i = 0; i < buff_size - 1; ++i)
                CPPUNIT_ASSERT_EQUAL((size_t) 1, put_data(&rb, "F", 1));

        /* Check that we have no space */
        CPPUNIT_ASSERT_EQUAL((size_t) 0, get_space(&rb));

        /* Clear out all the data.  Ensure we clear buff_size -1 bytes */
        CPPUNIT_ASSERT_EQUAL(buff_size - 1, clear_data(&rb));

        /* Check that we have no space */
        CPPUNIT_ASSERT_EQUAL(buff_size - 1, get_space(&rb));
}

void RingBufferTest::createDestroyTest()
{
        const size_t size = 11;

        /* Check that creation works as expected */
        CPPUNIT_ASSERT_EQUAL(size - 1, create_ring_buffer(&rb, size));

        /* Check that we have expected space */
        CPPUNIT_ASSERT_EQUAL(size - 1, get_space(&rb));

        /* Now destroy it */
        free_ring_buffer(&rb);

        /* Assert that we have no space internally */
        CPPUNIT_ASSERT_EQUAL((size_t) 0, rb.size);
}
