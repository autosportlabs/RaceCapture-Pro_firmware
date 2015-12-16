/*
 * Race Capture Pro Firmware
 *
 * Copyright (C) 2015 Autosport Labs
 *
 * This file is part of the Race Capture Pro fimrware suite
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

#ifndef _STDUTIL_H_
#define _STDUTIL_H_

#define MAX(a,b) \
        ({ typeof (a) _a = (a);                 \
                typeof (b) _b = (b);            \
                _a > _b ? _a : _b; })           \

#define MIN(a,b) \
        ({ typeof (a) _a = (a);                 \
                typeof (b) _b = (b);            \
                _a < _b ? _a : _b; })           \


#endif /* _STDUTIL_H_ */
