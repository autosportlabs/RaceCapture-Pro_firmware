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

#ifndef COM_MODP_STRINGENCODERS_ATONUM_H
#define COM_MODP_STRINGENCODERS_ATONUM_H

#include "cpp_guard.h"

CPP_GUARD_BEGIN

int modp_atoi(const char *str);
unsigned int modp_atoui(const char *str);
float modp_atof(const char *str);
double modp_atod(const char *str);
unsigned char modp_xtoc(const char *str);

CPP_GUARD_END

#endif
