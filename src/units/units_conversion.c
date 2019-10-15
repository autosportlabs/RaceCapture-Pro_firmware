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

#include "units_conversion.h"

static float no_conversion(float value)
{
        return value;
}
static float c_to_f(float value)
{
        return  value * 1.8f + 32;
}

static float f_to_c(float value)
{
        return (value - 32) * 0.555555556f;
}

static float bar_to_psi(float value)
{
        return value * 14.5037738f;
}

static float psi_to_bar(float value)
{
        return value * 0.0689475729f;
}

static float kph_to_mph(float value)
{
        return value * 0.6213711922f;
}

static float mph_to_kph(float value)
{
        return value * 1.609344f;
}

static float km_to_mi(float value)
{
        return value * 0.6213711922f;
}

static float mi_to_km(float value)
{
        return value * 1.609344f;
}

static float mm_to_inch(float value)
{
        return value * 0.0393700787f;
}

static float inch_to_mm(float value)
{
        return value * 25.4f;
}

static float l_to_gal(float value)
{
        return value * 0.2641720524f;
}

static float gal_to_l(float value)
{
        return value * 3.785411784f;
}

static float kg_to_lb(float value)
{
        return value * 2.2046226218f;
}

static float lb_to_kg(float value)
{
        return value * 0.45359237f;
}

static float nm_to_lbft(float value)
{
        return value * 0.7375621493f;
}

static float lbft_to_nm(float value)
{
        return value * 1.3558179483f;
}

static float w_to_hp(float value)
{
        return value * 0.0013410221f;
}

static float hp_to_w(float value)
{
        return value * 745.69987158f;
}

static float(*units_converter[UNITS_CONVERSION_COUNT]) (float value) = {
        no_conversion,
        c_to_f,
        f_to_c,
        bar_to_psi,
        psi_to_bar,
        kph_to_mph,
        mph_to_kph,
        km_to_mi,
        mi_to_km,
        mm_to_inch,
        inch_to_mm,
        l_to_gal,
        gal_to_l,
        kg_to_lb,
        lb_to_kg,
        nm_to_lbft,
        lbft_to_nm,
        w_to_hp,
        hp_to_w
};

float convert_units(enum unit_conversions id, const float value)
{
        if (id >= UNITS_CONVERSION_COUNT )
                return value;

        return (*units_converter[id]) (value);
}
