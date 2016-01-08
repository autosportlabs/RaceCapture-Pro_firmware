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

#include "UnitsTest.hh"

#include "units.h"

#include <string.h>

/* The C code to test */
extern "C" {
#include "units.c"
}

CPPUNIT_TEST_SUITE_REGISTRATION( UnitsTest );

void UnitsTest::get_unit_test_success()
{
        for(size_t i = 0; i < ARRAY_LEN(units); ++i)
                CPPUNIT_ASSERT_EQUAL(units + i, units_get_unit((units + i)->name));
}

void UnitsTest::get_unit_test_fail()
{
        CPPUNIT_ASSERT_EQUAL((const struct unit*) NULL, units_get_unit("foo"));
        CPPUNIT_ASSERT_EQUAL((const struct unit*) NULL, units_get_unit("Foo"));
        CPPUNIT_ASSERT_EQUAL((const struct unit*) NULL, units_get_unit("FOO"));
}

void UnitsTest::no_dupes_test()
{
        for(size_t i = 0; i < ARRAY_LEN(units); ++i) {
                const struct unit *u = units + i;
                int count = 0;
                for(size_t j = 0; j < ARRAY_LEN(units); ++j)
                        if (0 == strcmp((units + j)->name, u->name))
                                ++count;
                CPPUNIT_ASSERT_EQUAL(1, count);
        }
}
