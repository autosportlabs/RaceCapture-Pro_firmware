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

#include "macros.h"
#include "ring_buffer.h"
#include "ring_buffer_test.hh"
#include <stdio.h>
#include <string.h>

#define RING_BUFF_CAP	((size_t) 8)

static struct ring_buff *rb;

CPPUNIT_TEST_SUITE_REGISTRATION( RingBufferTest );

void RingBufferTest::setUp()
{
        rb = ring_buffer_create(RING_BUFF_CAP);
}

void RingBufferTest::tearDown()
{
        ring_buffer_destroy(rb);
        rb = NULL;
}

void RingBufferTest::testSanity()
{
        CPPUNIT_ASSERT_EQUAL((size_t) RING_BUFF_CAP,
                             ring_buffer_capacity(rb));
        CPPUNIT_ASSERT_EQUAL((size_t) RING_BUFF_CAP,
                             ring_buffer_bytes_free(rb));
        CPPUNIT_ASSERT_EQUAL((size_t) 0,
                             ring_buffer_bytes_used(rb));
        CPPUNIT_ASSERT_EQUAL((size_t) 0,
                             ring_buffer_peek(rb, NULL, RING_BUFF_CAP));
        CPPUNIT_ASSERT_EQUAL((size_t) 0,
                             ring_buffer_get(rb, NULL, RING_BUFF_CAP));
}

void RingBufferTest::testFreeAndUsed()
{
        /* We know that 0 works from test above */
        size_t used;
        for (used = 0; used <= RING_BUFF_CAP;
             ++used, ring_buffer_put(rb, "A", 1)) {
                const size_t free = RING_BUFF_CAP - used;

                CPPUNIT_ASSERT_EQUAL(free, ring_buffer_bytes_free(rb));
                CPPUNIT_ASSERT_EQUAL(used, ring_buffer_bytes_used(rb));
        }

        CPPUNIT_ASSERT_EQUAL(RING_BUFF_CAP + 1, used);
}

void RingBufferTest::testPutGet()
{
        const char data[] = "FOO";
        char buff[ARRAY_LEN(data) + 1];
        const size_t size = ARRAY_LEN(data);

        /* Put in the data */
        CPPUNIT_ASSERT_EQUAL(size, ring_buffer_put(rb, data, size));

        /* Check the sizes now */
        const size_t free_exp = RING_BUFF_CAP - size;
        CPPUNIT_ASSERT_EQUAL(size, ring_buffer_bytes_used(rb));
        CPPUNIT_ASSERT_EQUAL(free_exp, ring_buffer_bytes_free(rb));

        /* Get the data.  Ensure we only read `size` bytes */
        CPPUNIT_ASSERT_EQUAL(size, ring_buffer_get(rb, buff,
                                                   ARRAY_LEN(buff)));

        /* Check sizes again */
        CPPUNIT_ASSERT_EQUAL((size_t) 0, ring_buffer_bytes_used(rb));
        CPPUNIT_ASSERT_EQUAL((size_t) RING_BUFF_CAP,
                             ring_buffer_bytes_free(rb));

        /* Check data in == data out */
        CPPUNIT_ASSERT(0 == strcmp(data, buff));
}

void RingBufferTest::testPutTooMuch()
{
        const char data[] = "FooBarBazBizFizzBah";
        CPPUNIT_ASSERT_EQUAL((size_t) RING_BUFF_CAP,
                             ring_buffer_put(rb, data, ARRAY_LEN(data)));

        char buff[RING_BUFF_CAP + 1];
        CPPUNIT_ASSERT_EQUAL((size_t) RING_BUFF_CAP,
                             ring_buffer_get(rb, buff, ARRAY_LEN(buff)));

        /* Check data out to ensure its the last bits of the input */
        const size_t offset = ARRAY_LEN(data) - RING_BUFF_CAP;
        CPPUNIT_ASSERT(0 == strcmp(data + offset, buff));
}

void RingBufferTest::testPeek()
{
        const char data[] = "FOO";
        char buff[ARRAY_LEN(data) + 1];
        const size_t size = ARRAY_LEN(data);

        /* Put in the data */
        CPPUNIT_ASSERT_EQUAL(size, ring_buffer_put(rb, data, size));
        /* Peek at the data.  Ensure we only read `size` bytes */
        CPPUNIT_ASSERT_EQUAL(size, ring_buffer_peek(rb, buff,
                                                    ARRAY_LEN(buff)));

        /* Check the sizes now.  They should be unchanged. */
        const size_t free_exp = RING_BUFF_CAP - size;
        CPPUNIT_ASSERT_EQUAL(size, ring_buffer_bytes_used(rb));
        CPPUNIT_ASSERT_EQUAL(free_exp, ring_buffer_bytes_free(rb));

        /* Check data in == data out */
        CPPUNIT_ASSERT(0 == strcmp(data, buff));
}

void RingBufferTest::testDrop()
{
        const char data[] = "BARbaz";
        const size_t size = ARRAY_LEN(data);

        /* Put in the data */
        CPPUNIT_ASSERT_EQUAL(size, ring_buffer_put(rb, data, size));
        /* Drop the data.  Ensure we only read `size` bytes */
        CPPUNIT_ASSERT_EQUAL(size, ring_buffer_get(rb, NULL,
                                                   ARRAY_LEN(data)));

        CPPUNIT_ASSERT_EQUAL((size_t) 0, ring_buffer_bytes_used(rb));
        CPPUNIT_ASSERT_EQUAL((size_t) RING_BUFF_CAP,
                             ring_buffer_bytes_free(rb));
}

void RingBufferTest::testClear()
{
        const char data[] = "baZbaR";
        const size_t size = ARRAY_LEN(data);

        /* Put in the data */
        CPPUNIT_ASSERT_EQUAL(size, ring_buffer_put(rb, data, size));

        /* Clear the data.  */
        ring_buffer_clear(rb);

        CPPUNIT_ASSERT_EQUAL((size_t) 0, ring_buffer_bytes_used(rb));
        CPPUNIT_ASSERT_EQUAL((size_t) RING_BUFF_CAP,
                             ring_buffer_bytes_free(rb));
}

void RingBufferTest::testWrite()
{
        const char data[] = "baZb";
        const size_t size = ARRAY_LEN(data);

        while(ring_buffer_bytes_free(rb) >= size)
                CPPUNIT_ASSERT_EQUAL(size, ring_buffer_write(rb, data, size));

        /* Now that there is less free space than size, ensure write works */
        const size_t avail = ring_buffer_bytes_free(rb);
        CPPUNIT_ASSERT_EQUAL(avail, ring_buffer_write(rb, data, size));
}
