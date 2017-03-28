/*
 * Race Capture Firmware
 *
 * Copyright (C) 2017 Autosport Labs
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

#ifndef UNITS_CONVERSION_H_
#define UNITS_CONVERSION_H_

/**
 * Perform a units conversion for the specified units conversion id
 * @param id the units conversion id
 * @param value the value to convert
 * @return the converted value if the id is a valid conversion id; if not, the same value is returned (no conversion)
 **/
float convert_units(int id, const float value);

#endif /* UNITS_CONVERSION_H_ */
