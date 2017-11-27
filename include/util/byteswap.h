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

#ifndef _BYTESWAP_H_
#define _BYTESWAP_H_

#include "cpp_guard.h"
#include <stddef.h>
#include <stdint.h>

CPP_GUARD_BEGIN

/*
 * Byte swap unsigned 16 bits
 */
uint16_t swap_uint16(uint16_t val );

/*
 * Byte swap signed 16 bits
 */
int16_t swap_int16( int16_t val );

/*
* Byte swap unsigned 24 bits
*/
uint32_t swap_uint24(int32_t val );

/*
 * Byte swap signed 32 bits
 */
int32_t swap_int32( int32_t val );

/*
 * Byte swap unsigned 32 bits
 */
uint32_t swap_uint32( uint32_t val );

/*
* Byte swap unsigned 64 bits
 */
uint64_t swap_uint64( uint64_t val );

/*
 * decode a little endian encoded variable bit length stream
 */
uint64_t decode_little_endian_bitmode(uint64_t val, size_t bit_length);
CPP_GUARD_END

#endif /* _BYTESWAP_H_ */
