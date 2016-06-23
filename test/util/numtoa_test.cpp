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


#include "numtoa_test.h"
#include "modp_numtoa.h"
#include <string.h>

#include <string>

using std::string;

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( NumtoaTest );

void NumtoaTest::setUp() {}

void NumtoaTest::tearDown() {}

void NumtoaTest::testDoubleConversion() {
  double num;
  char expStr[10];
  char str[10];

  num = 0.123;
  strcpy(expStr, "0.123");
  modp_dtoa(num, str, 5);
  CPPUNIT_ASSERT_EQUAL(string(expStr), string(str));

  num = -1.1500;
  strcpy(expStr, "-1.15");
  modp_dtoa(num, str, 5);
  CPPUNIT_ASSERT_EQUAL(string(expStr), string(str));

  num = 1.0000;
  strcpy(expStr, "1.0");
  modp_dtoa(num, str, 5);
  CPPUNIT_ASSERT_EQUAL(string(expStr), string(str));

  // XXX: Should this be "5"?
  num = 005;
  strcpy(expStr, "5.0");
  modp_dtoa(num, str, 5);
  CPPUNIT_ASSERT_EQUAL(string(expStr), string(str));

  num = 3000;
  strcpy(expStr, "3000");
  modp_dtoa(num, str, 0);
  CPPUNIT_ASSERT_EQUAL(string(expStr), string(str));

}

void NumtoaTest::testModpFToA() {
  float num;
  char expStr[10];
  char str[10];

  num = 0.123;
  strcpy(expStr, "0.123");
  modp_ftoa(num, str, 5);
  CPPUNIT_ASSERT_EQUAL(string(expStr), string(str));

  num = -1.1500;
  strcpy(expStr, "-1.15");
  modp_ftoa(num, str, 5);
  CPPUNIT_ASSERT_EQUAL(string(expStr), string(str));

  num = 1.0000;
  strcpy(expStr, "1.0");
  modp_ftoa(num, str, 5);
  CPPUNIT_ASSERT_EQUAL(string(expStr), string(str));

  // XXX: Should this be "5"?
  num = 005;
  strcpy(expStr, "5.0");
  modp_ftoa(num, str, 5);
  CPPUNIT_ASSERT_EQUAL(string(expStr), string(str));

  num = 3000;
  strcpy(expStr, "3000");
  modp_ftoa(num, str, 0);
  CPPUNIT_ASSERT_EQUAL(string(expStr), string(str));

}
