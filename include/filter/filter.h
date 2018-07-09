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

#ifndef _FILTER_H_
#define _FILTER_H_

#include "cpp_guard.h"
#include <stdint.h>

CPP_GUARD_BEGIN

typedef struct _Filter {
        float alpha;
        int32_t total;
        int32_t current_value;
        int32_t count;
        int32_t max_samples;
} Filter;

void init_filter(Filter *filter, float alpha);

int32_t update_filter(Filter *filter, int32_t value);

CPP_GUARD_END

#endif /* _FILTER_H_ */
