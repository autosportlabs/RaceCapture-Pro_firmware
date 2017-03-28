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

#define UNITS_CONVERSION_COUNT 19

enum unit_conversions {
    UNIT_CONVERSION_NONE = 0,

    /* Temperature conversion */
    UNIT_CONVERSION_TEMPERATURE_C_TO_F,
    UNIT_CONVERSION_TEMPERATURE_F_TO_C,

    /* Pressure Conversion */
    UNIT_CONVERSION_PRESSURE_BAR_TO_PSI,
    UNIT_CONVERSION_PRESSURE_PSI_TO_BAR,

    /* Speed Conversion */
    UNIT_CONVERSION_SPEED_KPH_TO_MPH,
    UNIT_CONVERSION_SPEED_MPH_TO_KPH,

    /* Length Conversion */
    UNIT_CONVERSION_LENGTH_KM_TO_MI,
    UNIT_CONVERSION_LENGTH_MI_TO_KM,
    UNIT_CONVERSION_LENGTH_MM_TO_INCH,
    UNIT_CONVERSION_LENGTH_INCH_TO_MM,

    /* Volume Conversion */
    UNIT_CONVERSION_VOLUME_L_TO_GAL,
    UNIT_CONVERSION_VOLUME_GAL_TO_L,

    /* Mass Conversion */
    UNIT_CONVERSION_MASS_KG_TO_LB,
    UNIT_CONVERSION_MASS_LB_TO_KG,

    /* Torque Conversion */
    UNIT_CONVERSION_TORQUE_NM_TO_LBFT,
    UNIT_CONVERSION_TORQUE_LBFT_TO_NM,

    /* Power Conversion */
    UNIT_CONVERSION_POWER_W_TO_HP,
    UNIT_CONVERSION_POWER_HP_TO_W,
};

/**
 * Perform a units conversion for the specified units conversion id
 * @param id the units conversion id
 * @param value the value to convert
 * @return the converted value if the id is a valid conversion id; if not, the same value is returned (no conversion)
 **/
float convert_units(enum unit_conversions id, const float value);

#endif /* UNITS_CONVERSION_H_ */
