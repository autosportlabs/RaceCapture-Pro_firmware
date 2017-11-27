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


#include <stdint.h>
#include "byteswap.h"
#include "panic.h"

uint16_t swap_uint16( uint16_t val )
{
    return (val << 8) | (val >> 8 );
}

int16_t swap_int16( int16_t val )
{
    return (val << 8) | ((val >> 8) & 0xFF);
}

uint32_t swap_uint24(int32_t val )
{
    return ((val & 0xFF0000) >> 16) | (val & 0x00FF00) | ((val & 0x0000FF) << 16);
}

uint32_t swap_uint32( uint32_t val )
{
    val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF );
    return (val << 16) | (val >> 16);
}

int32_t swap_int32( int32_t val )
{
    val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF );
    return (val << 16) | ((val >> 16) & 0xFFFF);
}

uint64_t swap_uint64(uint64_t val)
{
    return (val & 0x00000000000000FFUL) << 56 | (val & 0x000000000000FF00UL) << 40 |
             (val & 0x0000000000FF0000UL) << 24 | (val & 0x00000000FF000000UL) << 8 |
             (val & 0x000000FF00000000UL) >> 8 | (val & 0x0000FF0000000000UL) >> 24 |
             (val & 0x00FF000000000000UL) >> 40 | (val & 0xFF00000000000000UL) >> 56;
}

uint64_t decode_little_endian_bitmode(uint64_t val, size_t bit_length)
/*
 * Convert a variable bit length little endian encoded stream, accounting for byte-level
 * endian granularity, also accounting for remaining bits
 *
 * example conversion:
 * incoming 12 bit stream:  LLLLLLLL HHHHxxxx
 * converts to: xxxxHHHH LLLLLLLL
 *
 * @param val incoming value
 * @param bit_length number of bits to swap
 * @return byte swapped value
 */
{
        if (bit_length <= 8) return val & ((1 << bit_length) - 1);
        if (bit_length <= 16){
                const size_t partial_bit_length = bit_length - 8;
                const uint8_t partial_bit_mask = (1 << partial_bit_length) - 1;
                return ((val & partial_bit_mask) << 8) + ((val >> partial_bit_length) & 0xFF);
        }
        if (bit_length <= 24) {
                const size_t partial_bit_length = bit_length - 16;
                const uint8_t partial_bit_mask = (1 << partial_bit_length) - 1;
                return ((val & partial_bit_mask) << 16) + swap_uint16(val >> partial_bit_length);
        }
        if (bit_length <= 32) {
                const size_t partial_bit_length = bit_length - 24;
                const uint8_t partial_bit_mask = (1 << partial_bit_length) - 1;
                return ((val & partial_bit_mask) << 24) + swap_uint24(val >> partial_bit_length);
        }
        panic(PANIC_CAUSE_UNREACHABLE);
        return 0; /* to make compiler happy */
}
