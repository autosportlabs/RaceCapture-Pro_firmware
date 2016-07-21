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

#include "convert.h"

float convert_kph_mph(const float kph)
{
        return kph * 0.621371192;
}

float convert_mph_kph(const float mph)
{
        return mph * 1.60934400061;
}

float convert_km_mi(const float km)
{
	return km * 0.621371;
}

float convert_mi_km(const float mi)
{
	return mi * 1.609344;
}

float convert_ft_m(const float ft)
{
	return ft * 0.3048;
}

float convert_m_ft(const float m)
{
	return m * 3.28084;
}
