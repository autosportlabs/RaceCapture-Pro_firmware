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


#include "modp_numtoa.h"

#include <stdint.h>
#include <string.h>

// other interesting references on num to string convesion
// http://www.jb.man.ac.uk/~slowe/cpp/itoa.html
// and http://www.ddj.com/dept/cpp/184401596?pgno=6

// Version 19-Nov-2007
// Fixed round-to-even rules to match printf
//   thanks to Johannes Otepka

/**
 * Powers of 10
 * 10^0 to 10^9
 */
static const float aPow10[] = {1, 10, 100, 1000, 10000, 100000, 1000000,
                               10000000, 100000000, 1000000000
                              };

static void strreverse(char* begin, char* end)
{
    char aux;
    while (end > begin)
        aux = *end, *end-- = *begin, *begin++ = aux;
}

void modp_itoa10(int32_t value, char* str)
{
    char* wstr=str;
    int sign;
    // Take care of sign
    if ((sign=value) < 0) value = -value;
    // Conversion. Number is reversed.
    do *wstr++ = (48 + (value % 10));
    while( value /= 10);
    if (sign < 0) *wstr++ = '-';
    *wstr='\0';

    // Reverse string
    strreverse(str,wstr-1);
}

void modp_uitoa10(uint32_t value, char* str)
{
    char* wstr=str;
    // Conversion. Number is reversed.
    do *wstr++ = 48 + (value % 10);
    while (value /= 10);
    *wstr='\0';
    // Reverse string
    strreverse(str, wstr-1);
}

void modp_ltoa10(int64_t value, char* str)
{
    char* wstr=str;
    int neg = value < 0;

    // Don't know why this is needed, but following suit b/c tired/lazy.
    if (neg)
        value = -value;

    // Conversion. Number is reversed.
    do *wstr++ = 48 + (value % 10);
    while (value /= 10);

    if (neg)
        *wstr++ = '-';

    *wstr='\0';

    // Reverse string
    strreverse(str, wstr-1);
}


void modp_ultoa10(uint64_t value, char* str)
{
    char* wstr=str;
    // Conversion. Number is reversed.
    do *wstr++ = 48 + (value % 10);
    while (value /= 10);
    *wstr='\0';
    // Reverse string
    strreverse(str, wstr-1);
}

char* trimLeadingZeros(char *ptr)
{
    for(; *ptr == '0'; ++ptr) {
        switch(*(ptr + 1)) {
        case '.':
        case '\0':
            // Return here b/c we don't want to increment the ptr.
            return ptr;
        default:
            *ptr = '\0';
            break;
        }
    }

    return ptr;
}

void modp_ftoa(float value, char* str, int prec)
{
    /* if input is larger than thres_max, revert to exponential */
    const float thres_max = (float)(0x7FFFFFFF);

    float diff = 0.0;
    char* wstr = str;

    if (prec < 0) {
        prec = 0;
    } else if (prec > 9) {
        /* precision of >= 10 can lead to overflow errors */
        prec = 9;
    }


    /* we'll work in positive values and deal with the
       negative sign issue later */
    int neg = 0;
    if (value < 0) {
        neg = 1;
        value = -value;
    }


    int whole = (int) value;
    float tmp = (value - whole) * aPow10[prec];
    uint32_t frac = (uint32_t)(tmp);
    diff = tmp - frac;

    if (diff > 0.5) {
        ++frac;
        /* handle rollover, e.g.  case 0.99 with prec 1 is 1.0  */
        if (frac >= aPow10[prec]) {
            frac = 0;
            ++whole;
        }
    } else if (diff == 0.5 && ((frac == 0) || (frac & 1))) {
        /* if halfway, round up if odd, OR
           if last digit is 0.  That last part is strange */
        ++frac;
    }

    /* for very large numbers switch back to native sprintf for exponentials.
       anyone want to write code to replace this? */
    /*
       normal printf behavior is to print EVERY whole number digit
       which can be 100s of characters overflowing your buffers == bad
    */
    if (value > thres_max) {
        strcpy(str,"3735928559"); //overflow - avoid sprintf for now  //oxdeadbeef
        //sprintf(str, "%e", neg ? -value : value);
        return;
    }

    if (prec == 0) {
        diff = value - whole;
        if (diff > 0.5) {
            /* greater than 0.5, round up, e.g. 1.6 -> 2 */
            ++whole;
        } else if (diff == 0.5 && (whole & 1)) {
            /* exactly 0.5 and ODD, then round up */
            /* 1.5 -> 2, but 2.5 -> 2 */
            ++whole;
        }
    } else {
        int count = prec;
        // now do fractional part, as an unsigned number
        do {
            --count;
            *wstr++ = 48 + (frac % 10);
        } while (frac /= 10);
        // add extra 0s
        while (count-- > 0) *wstr++ = '0';
        // add decimal
        *wstr++ = '.';
    }

    // do whole part
    // Take care of sign
    // Conversion. Number is reversed.
    do *wstr++ = 48 + (whole % 10);
    while (whole /= 10);
    if (neg) {
        *wstr++ = '-';
    }
    *wstr='\0';

    if (prec > 0) trimLeadingZeros(str);
    strreverse(str, wstr-1);
}

void modp_dtoa(double value, char* str, int prec)
{
    /* if input is larger than thres_max, revert to exponential */
    const double thres_max = (double)(0x7FFFFFFFFFFFFFFF);

    double diff = 0.0;
    char* wstr = str;

    if (prec < 0) {
        prec = 0;
    } else if (prec > 9) {
        /* precision of >= 10 can lead to overflow errors */
        prec = 9;
    }


    /* we'll work in positive values and deal with the
       negative sign issue later */
    int neg = 0;
    if (value < 0) {
        neg = 1;
        value = -value;
    }


    int whole = (int) value;
    double tmp = (value - whole) * aPow10[prec];
    uint32_t frac = (uint32_t)(tmp);
    diff = tmp - frac;

    if (diff > 0.5) {
        ++frac;
        /* handle rollover, e.g.  case 0.99 with prec 1 is 1.0  */
        if (frac >= aPow10[prec]) {
            frac = 0;
            ++whole;
        }
    } else if (diff == 0.5 && ((frac == 0) || (frac & 1))) {
        /* if halfway, round up if odd, OR
           if last digit is 0.  That last part is strange */
        ++frac;
    }

    /* for very large numbers switch back to native sprintf for exponentials.
       anyone want to write code to replace this? */
    /*
       normal printf behavior is to print EVERY whole number digit
       which can be 100s of characters overflowing your buffers == bad
    */
    if (value > thres_max) {
        strcpy(str,"3735928559"); //overflow - avoid sprintf for now //0xdeadbeef
        //sprintf(str, "%e", neg ? -value : value);
        return;
    }

    if (prec == 0) {
        diff = value - whole;
        if (diff > 0.5) {
            /* greater than 0.5, round up, e.g. 1.6 -> 2 */
            ++whole;
        } else if (diff == 0.5 && (whole & 1)) {
            /* exactly 0.5 and ODD, then round up */
            /* 1.5 -> 2, but 2.5 -> 2 */
            ++whole;
        }
    } else {
        int count = prec;
        // now do fractional part, as an unsigned number
        do {
            --count;
            *wstr++ = 48 + (frac % 10);
        } while (frac /= 10);
        // add extra 0s
        while (count-- > 0) *wstr++ = '0';
        // add decimal
        *wstr++ = '.';
    }

    // do whole part
    // Take care of sign
    // Conversion. Number is reversed.
    do *wstr++ = 48 + (whole % 10);
    while (whole /= 10);
    if (neg) {
        *wstr++ = '-';
    }
    *wstr='\0';

    if (prec > 0) trimLeadingZeros(str);
    strreverse(str, wstr-1);
}

char* modp_itoaX(int value, char* result, int base)
{
    // check that the base if valid
    if (base < 2 || base > 36) {
        *result = '\0';
        return result;
    }

    char* ptr = result, *ptr1 = result, tmp_char;
    int tmp_value;

    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    } while ( value );

    // Apply negative sign
    if (tmp_value < 0) *ptr++ = '-';
    *ptr-- = '\0';
    while(ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }
    return result;
}
