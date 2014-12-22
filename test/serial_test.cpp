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
 * Authors: jstoezel
 */

#include <stdint.h>
#include "serial.h"
#include "serial_test.h"


CPPUNIT_TEST_SUITE_REGISTRATION( SerialTest );

char      SerialTest::m_TestBuffer[512];
uint32_t  SerialTest::m_BufferIndex = 0;
  
void SerialTest::setUp() {}

void SerialTest::tearDown() {}

void SerialTest::testPrintf() {

   char STRAIGHT_STRING[] = "STRAIGHT_STRING\r\n";
   char buffer[256];
   
   Serial serialIf = {
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   SerialTest::putc,
   NULL,
   NULL
   };
   
   // String
   ResetTestBuffer();
   serial_printf(&serialIf, STRAIGHT_STRING);
   CPPUNIT_ASSERT_EQUAL(m_BufferIndex, strlen(STRAIGHT_STRING));
   CPPUNIT_ASSERT_EQUAL(static_cast<int>(0), strcmp(m_TestBuffer, STRAIGHT_STRING));
   ResetTestBuffer();
   serial_printf(&serialIf, "%s", STRAIGHT_STRING);
   CPPUNIT_ASSERT_EQUAL(m_BufferIndex, strlen(STRAIGHT_STRING));
   CPPUNIT_ASSERT_EQUAL(static_cast<int>(0), strcmp(m_TestBuffer, STRAIGHT_STRING));
   
   // chars
   for(char i = ' '; i < '~'; i++)
   {
      ResetTestBuffer();
      if('%' != i)
      {
         serial_printf(&serialIf, "%c", i);
         sprintf(buffer, "%c", i);
         CPPUNIT_ASSERT_EQUAL(static_cast<int>(0), strcmp(m_TestBuffer, buffer));
      }
   } // for
   
   #define SERIAL_TEST_UINT_STEP 901
   // Unsigned int
   for(uint32_t i = 0; i < (4294967295 - SERIAL_TEST_UINT_STEP); i += SERIAL_TEST_UINT_STEP)
   {
      ResetTestBuffer();
      serial_printf(&serialIf, "%u", i);
      sprintf(buffer, "%u", i);
      CPPUNIT_ASSERT_EQUAL(static_cast<int>(0), strcmp(m_TestBuffer, buffer));
   } // for
   
   #define SERIAL_TEST_INT_STEP 901
   // int
   for(int32_t i = (-2147483648 + SERIAL_TEST_INT_STEP); i < (2147483647 - SERIAL_TEST_INT_STEP); i += SERIAL_TEST_INT_STEP)
   {
      ResetTestBuffer();
      serial_printf(&serialIf, "%d", i);
      sprintf(buffer, "%d", i);
      CPPUNIT_ASSERT_EQUAL(static_cast<int>(0), strcmp(m_TestBuffer, buffer));
   } // for
   
   // Special characters
   ResetTestBuffer();
   serial_printf(&serialIf, "%%");
   sprintf(buffer, "%%");
   CPPUNIT_ASSERT_EQUAL(static_cast<int>(0), strcmp(m_TestBuffer, buffer));
   CPPUNIT_ASSERT_EQUAL(static_cast<char>('%'), m_TestBuffer[0]);
   ResetTestBuffer();
   serial_printf(&serialIf, "%%%%");
   sprintf(buffer, "%%%%");
   CPPUNIT_ASSERT_EQUAL(static_cast<char>('%'), m_TestBuffer[0]);
   CPPUNIT_ASSERT_EQUAL(static_cast<char>('%'), m_TestBuffer[1]);
} /* SerialTest::testPrintf */

