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


#include "filter.h"
#include "stdutil.h"

//Implements a fast Exponential Moving Average filter
#define MIN_ALPHA 0.0001
void init_filter(Filter *filter, const float alpha)
{
        filter->max_samples = (int32_t)(1 / MAX(alpha, MIN_ALPHA));
        filter->current_value = 0;
        filter->total = 0;
        filter->count = 0;
}

int32_t update_filter(Filter *filter, const int32_t value)
{
        filter->total += value;
        if (filter->count >= filter->max_samples) {
                filter->total -= filter->current_value;
        } else {
                filter->count++;
        }
        filter->current_value = filter->total / filter->count;
        return filter->current_value;
}
