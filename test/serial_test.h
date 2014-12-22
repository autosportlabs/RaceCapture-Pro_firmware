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

#ifndef _SERIAL_TEST_H_
#define _SERIAL_TEST_H_

#include <cppunit/extensions/HelperMacros.h>
#include <stdint.h>
#include <string.h>

class SerialTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( SerialTest );
  CPPUNIT_TEST( testPrintf );
  CPPUNIT_TEST_SUITE_END();
  
  static void ResetTestBuffer(void)
  {
     m_BufferIndex = 0;
     memset(m_TestBuffer, 0, sizeof(m_TestBuffer));
  }
  
  static void putc(char c)
  {
     if(m_BufferIndex < sizeof(m_TestBuffer))
     {
        m_TestBuffer[m_BufferIndex] = c;
        m_BufferIndex++;
     }
     else
     {}
  }

public:
  void setUp(void);
  void tearDown(void);
  void testPrintf(void);
  
  static char      m_TestBuffer[512];
  static uint32_t  m_BufferIndex;
};

#endif /* _SERIAL_TEST_H_ */
