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

#include "JsmnTest.hh"
#include "macros.h"
#include "mock_serial.h"
#include "serial.h"
#include <cppunit/extensions/HelperMacros.h>

extern "C" {
#include "jsmn/jsmn.c"
}

CPPUNIT_TEST_SUITE_REGISTRATION( JsmnTest );

using std::string;

void JsmnTest::decodeStringTest()
{
	const char encoded_json[] = "foo\\\"bar\\\\\\/baz\\t\\u1234\\r\\n";
	const char expected_str[] = "foo\"bar\\/baz\t?\r\n";
	char actual_str[ARRAY_LEN(expected_str)];

	jsmn_decode_string(actual_str, encoded_json, ARRAY_LEN(actual_str));
	CPPUNIT_ASSERT_EQUAL(string(expected_str), string(actual_str));
}

void JsmnTest::encodeWriteStringTest()
{
	const char str[] = "foo\"bar\\/baz\t?\r\n";
	const char expected_json[] = "foo\\\"bar\\\\/baz\\t?\\r\\n";

	setupMockSerial();
	Serial* serial = getMockSerial();

	jsmn_encode_write_string(serial, str);
	CPPUNIT_ASSERT_EQUAL(string(expected_json),
			     string(mock_getTxBuffer()));

}
