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

#include "RxBuffTest.hh"
#include "serial.h"
#include "mock_serial.h"

/* Inclue the code to test here */
extern "C" {
#include "serial/rx_buff.c"
}

#define RX_BUFF_CAPACITY 32

using std::string;

struct rx_buff *rxbuff;

CPPUNIT_TEST_SUITE_REGISTRATION( RxBuffTest );

void RxBuffTest::setUp()
{
	rxbuff = rx_buff_create(RX_BUFF_CAPACITY);
	CPPUNIT_ASSERT(rxbuff);

	setupMockSerial();
}

void RxBuffTest::tearDown()
{
	rx_buff_destroy(rxbuff);
	rxbuff = NULL;
}

void RxBuffTest::clearBuffTest()
{
	Serial* serial = getMockSerial();
	mock_appendRxBuffer("foo");
	const bool ready = rx_buff_read(rxbuff, serial, false);

	CPPUNIT_ASSERT_EQUAL(false, ready);
	CPPUNIT_ASSERT_EQUAL(false, rxbuff->msg_ready);
	CPPUNIT_ASSERT_EQUAL((size_t) 3, rxbuff->idx);

	rx_buff_clear(rxbuff);

	CPPUNIT_ASSERT_EQUAL(false, ready);
	CPPUNIT_ASSERT_EQUAL(false, rxbuff->msg_ready);
	CPPUNIT_ASSERT_EQUAL((size_t) 0, rxbuff->idx);
}

void RxBuffTest::createRxBuffTest()
{
	/* Already allocated by time we get here */
	CPPUNIT_ASSERT_EQUAL((size_t) RX_BUFF_CAPACITY, rxbuff->cap);
	CPPUNIT_ASSERT_EQUAL((size_t) 0, rxbuff->idx);
	CPPUNIT_ASSERT(rxbuff->buff);
	CPPUNIT_ASSERT_EQUAL(false, rxbuff->msg_ready);
	CPPUNIT_ASSERT_EQUAL(false, rxbuff->echo);
}

void RxBuffTest::msgReadyTest()
{
	Serial* serial = getMockSerial();
	mock_appendRxBuffer("bar\r");
	const bool ready = rx_buff_read(rxbuff, serial, false);

	CPPUNIT_ASSERT_EQUAL(true, ready);
	CPPUNIT_ASSERT_EQUAL(false, rx_buff_is_overflow(rxbuff));
	CPPUNIT_ASSERT_EQUAL(RX_BUFF_STATUS_READY,
			     rx_buff_get_status(rxbuff));
	CPPUNIT_ASSERT_EQUAL(string("bar"),
			     string(rx_buff_get_msg(rxbuff)));
}

void RxBuffTest::msgPartialTest()
{
	Serial* serial = getMockSerial();
	mock_appendRxBuffer("bar");
	const bool ready = rx_buff_read(rxbuff, serial, false);

	CPPUNIT_ASSERT_EQUAL(false, ready);
	CPPUNIT_ASSERT_EQUAL(false, rx_buff_is_overflow(rxbuff));
	CPPUNIT_ASSERT_EQUAL(RX_BUFF_STATUS_PARTIAL,
			     rx_buff_get_status(rxbuff));
	CPPUNIT_ASSERT(!rx_buff_get_msg(rxbuff));
}

void RxBuffTest::msgOverflowTest()
{
	Serial* serial = getMockSerial();
	for (int i = 0; i < RX_BUFF_CAPACITY + 1; ++i)
		mock_appendRxBuffer("b");

	const bool ready = rx_buff_read(rxbuff, serial, false);

	CPPUNIT_ASSERT_EQUAL(true, ready);
	CPPUNIT_ASSERT_EQUAL(true, rx_buff_is_overflow(rxbuff));
	CPPUNIT_ASSERT_EQUAL(RX_BUFF_STATUS_OVERFLOW,
			     rx_buff_get_status(rxbuff));
	CPPUNIT_ASSERT(!rx_buff_get_msg(rxbuff));
}
