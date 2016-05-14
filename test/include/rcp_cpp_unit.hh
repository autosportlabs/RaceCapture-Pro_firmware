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
 *
 * Put all useful CUnit macros here to avoid duplication.
 */

#ifndef _RCP_CPP_UNIT_H_
#define _RCP_CPP_UNIT_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <cppunit/extensions/HelperMacros.h>

/**
 * Tests if a condition is close enough.  Useful for tests where the results aren't
 * directly exact.  Try to avoid using this unless absolutely necessary.
 */
#define RCPUNIT_ASSERT_CLOSE(EXPECTED, TOLERANCE, ACTUAL)               \
        CPPUNIT_ASSERT((TOLERANCE) > 0 &&                               \
                       ((ACTUAL) - (EXPECTED)) > (-TOLERANCE) &&        \
                       ((ACTUAL) - (EXPECTED)) < (TOLERANCE))

const bool debug = getenv("DEBUG") != NULL;
#define RCPUNIT_DEBUG(MSG, ...) if (debug) printf(MSG, ##__VA_ARGS__)

/**
 * Older close_enough macro that was inflexible.  The newer #RCPUNIT_ASSERT_CLOSE is more flexible.
 * @deprecated Use #RCPUNIT_ASSERT_CLOSE instead.
 */
#define CPPUNIT_ASSERT_CLOSE_ENOUGH(ACTUAL, EXPECTED) \
        RCPUNIT_ASSERT_CLOSE(EXPECTED, 0.00001, ACTUAL)


#endif /* _RCP_CPP_UNIT_H_ */
