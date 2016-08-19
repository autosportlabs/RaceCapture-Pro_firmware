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

#include "StrUtilTest.hh"
#include "str_util.h"
#include <cppunit/extensions/HelperMacros.h>
#include <string.h>
#include <string>

CPPUNIT_TEST_SUITE_REGISTRATION( StrUtilTest );

using std::string;
/* No need to include the code here since no state to modify */

void StrUtilTest::serial_msg_strlen_test()
{
        CPPUNIT_ASSERT_EQUAL((size_t) 4, serial_msg_strlen("Foo\r"));
	CPPUNIT_ASSERT_EQUAL((size_t) 5, serial_msg_strlen("baRRR\r\n"));
	CPPUNIT_ASSERT_EQUAL((size_t) 7, serial_msg_strlen("bazzzz\t"));
	CPPUNIT_ASSERT_EQUAL((size_t) 0, serial_msg_strlen(""));
}

void StrUtilTest::lstrip_inline_test()
{
	char ts1[] = " foo  ";
	CPPUNIT_ASSERT_EQUAL(string("foo  "), string(lstrip_inline(ts1)));

	char ts2[] = "\tfoo\t";
	CPPUNIT_ASSERT_EQUAL(string("foo\t"), string(lstrip_inline(ts2)));

	char ts3[] = " \t\r\n";
	CPPUNIT_ASSERT_EQUAL(string(""), string(lstrip_inline(ts3)));

	char ts4[] = "";
	CPPUNIT_ASSERT_EQUAL(string(""), string(lstrip_inline(ts4)));
}

void StrUtilTest::rstrip_inline_test()
{
	char ts1[] = " foo";
	CPPUNIT_ASSERT_EQUAL(string(" foo"), string(rstrip_inline(ts1)));

	char ts2[] = "\tfoo\t\r\n";
	CPPUNIT_ASSERT_EQUAL(string("\tfoo"), string(rstrip_inline(ts2)));

	char ts3[] = " \t\r\n";
	CPPUNIT_ASSERT_EQUAL(string(""), string(rstrip_inline(ts3)));

	char ts4[] = "";
	CPPUNIT_ASSERT_EQUAL(string(""), string(rstrip_inline(ts4)));
}

void StrUtilTest::strip_inline_test()
{
	char ts1[] = " foo ";
	CPPUNIT_ASSERT_EQUAL(string("foo"), string(strip_inline(ts1)));

	char ts2[] = "\t foo \r\n ";
	CPPUNIT_ASSERT_EQUAL(string("foo"), string(strip_inline(ts2)));

	char ts3[] = " \t\r\n ";
	CPPUNIT_ASSERT_EQUAL(string(""), string(strip_inline(ts3)));

	char ts4[] = "";
	CPPUNIT_ASSERT_EQUAL(string(""), string(strip_inline(ts4)));
}

void StrUtilTest::lstrip_zeros_inline_test()
{
	char ts1[] = "5.000";
	CPPUNIT_ASSERT_EQUAL(string("5.000"),
			     string(str_util_lstrip_zeros_inline(ts1)));

	char ts2[] = "-05.000";
	CPPUNIT_ASSERT_EQUAL(string("-5.000"),
			     string(str_util_lstrip_zeros_inline(ts2)));

	char ts3[] = "+00005.000";
	CPPUNIT_ASSERT_EQUAL(string("+5.000"),
			     string(str_util_lstrip_zeros_inline(ts3)));

	char ts4[] = "";
	CPPUNIT_ASSERT_EQUAL(string(""),
			     string(str_util_lstrip_zeros_inline(ts4)));
}

void StrUtilTest::rstrip_zeros_inline_test()
{
	char ts1[] = "5.000";
	CPPUNIT_ASSERT_EQUAL(string("5.0"),
			     string(str_util_rstrip_zeros_inline(ts1)));

	char ts2[] = "5.001";
	CPPUNIT_ASSERT_EQUAL(string("5.001"),
			     string(str_util_rstrip_zeros_inline(ts2)));

	char ts3[] = "005.000";
	CPPUNIT_ASSERT_EQUAL(string("005.0"),
			     string(str_util_rstrip_zeros_inline(ts3)));

	char ts4[] = "0";
	CPPUNIT_ASSERT_EQUAL(string("0"),
			     string(str_util_rstrip_zeros_inline(ts4)));

	char ts5[] = "0.A00";
	CPPUNIT_ASSERT_EQUAL(string("0.A"),
			     string(str_util_rstrip_zeros_inline(ts5)));
}

void StrUtilTest::strip_zeros_inline_test()
{
	char ts1[] = "05.000";
	CPPUNIT_ASSERT_EQUAL(string("5.0"),
			     string(str_util_strip_zeros_inline(ts1)));

	char ts2[] = "5.001";
	CPPUNIT_ASSERT_EQUAL(string("5.001"),
			     string(str_util_strip_zeros_inline(ts2)));

	char ts3[] = "005.000";
	CPPUNIT_ASSERT_EQUAL(string("5.0"),
			     string(str_util_strip_zeros_inline(ts3)));

	char ts4[] = "0";
	CPPUNIT_ASSERT_EQUAL(string("0"),
			     string(str_util_strip_zeros_inline(ts4)));

	char ts5[] = "0.A00";
	CPPUNIT_ASSERT_EQUAL(string("0.A"),
			     string(str_util_strip_zeros_inline(ts5)));
}
