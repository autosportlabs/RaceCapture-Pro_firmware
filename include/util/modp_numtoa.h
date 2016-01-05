/* -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4 -*- */
/* vi: set expandtab shiftwidth=4 tabstop=4: */

/**
 * \file
 *
 * <pre>
 * Copyright &copy; 2007, Nick Galbreath -- nickg [at] modp [dot] com
 * All rights reserved.
 * http://code.google.com/p/stringencoders/
 * Released under the bsd license.
 * </pre>
 *
 * This defines signed/unsigned integer, and 'double' to char buffer
 * converters.  The standard way of doing this is with "sprintf", however
 * these functions are
 *   * guarenteed maximum size output
 *   * 5-20x faster!
 *   * core-dump safe
 *
 *
 */

#ifndef COM_MODP_STRINGENCODERS_NUMTOA_H
#define COM_MODP_STRINGENCODERS_NUMTOA_H

#include "cpp_guard.h"

#include <stdint.h>

CPP_GUARD_BEGIN

/** \brief convert an signed integer to char buffer
 *
 * \param[in] value
 * \param[out] buf the output buffer.  Should be 16 chars or more.
 */
void modp_itoa10(int32_t value, char* buf);

/** \brief convert an unsigned integer to char buffer
 *
 * \param[in] value
 * \param[out] buf The output buffer, should be 16 chars or more.
 */
void modp_uitoa10(uint32_t value, char* buf);

/** \brief convert a long to char buffer
 *
 * \param[in] value
 * \param[out] buf The output buffer, should be 21 chars or more.
 */
void modp_ltoa10(int64_t value, char* str);

/** \brief convert an unsigned long to char buffer
 *
 * \param[in] value
 * \param[out] buf The output buffer, should be 16 chars or more.
 */
void modp_ultoa10(uint64_t value, char* buf);

/** \brief convert a floating point number to char buffer with fixed-precision format
 *
 * If the input value is greater than 1<<31, then the output format
 * will be switched exponential format.
 *
 * \param[in] value
 * \param[out] buf  The allocated output buffer.  Should be 32 chars or more.
 * \param[in] precision  Number of digits to the right of the decimal point.
 *    Can only be 0-9.
 */
void modp_ftoa(float value, char* buf, int precision);

void modp_dtoa(double value, char* buf, int precision);

/** \brief convert an integer to string with selectable base
 *  \param[in] value
 *  \param[out] buf  The allocated output buffer.  Should be 32 chars or more.
 *  \param[in] base - base of the resulting conversion
 */
char* modp_itoaX(int value, char* result, int base);

/**
 * Trims leading zeros on a number.  Works with strrev by replacing zeros
 * with '/0' characters
 * @param ptr The array to trim the leading zeros on.
 * @return The new start of the string pointer to use.
 */
char* trimLeadingZeros(char *ptr);


CPP_GUARD_END

#endif
