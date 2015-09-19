/**
 * Race Capture Firmware
 *
 * Copyright (C) 2014 Autosport Labs
 *
 * This file is part of the Race Capture firmware suite
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

#ifndef _TEST_H_
#define _TEST_H_

/**
 * This helpful definition causes static definitions to disappear when we are
 * testing our code but re-appear when we are compiling for firmware.  This
 * better isolates the scope of the code in the firmware and may help the
 * compiler do a better job of optimizing the code.
 */

#ifndef RCP_TESTING
#define TESTABLE_STATIC static
#else
#define TESTABLE_STATIC
#endif /* RCP_TESTING */

#endif /* _TEST_H_ */
