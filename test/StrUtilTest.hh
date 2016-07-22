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

#ifndef _STRUTILTEST_H_
#define _STRUTILTEST_H_

#include <cppunit/extensions/HelperMacros.h>

class StrUtilTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE( StrUtilTest );

	CPPUNIT_TEST( serial_msg_strlen_test );
	CPPUNIT_TEST( lstrip_inline_test );
	CPPUNIT_TEST( rstrip_inline_test );
	CPPUNIT_TEST( strip_inline_test );
	CPPUNIT_TEST( lstrip_zeros_inline_test );
	CPPUNIT_TEST( rstrip_zeros_inline_test );
	CPPUNIT_TEST( strip_zeros_inline_test );

	CPPUNIT_TEST_SUITE_END();

public:
	void serial_msg_strlen_test();
	void lstrip_inline_test();
	void rstrip_inline_test();
	void strip_inline_test();
	void lstrip_zeros_inline_test();
	void rstrip_zeros_inline_test();
	void strip_zeros_inline_test();
};

#endif /* _STRUTILTEST_H_ */
