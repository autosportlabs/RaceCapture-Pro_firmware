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

#ifndef GPIO_H_
#define GPIO_H_

#include "cpp_guard.h"
#include "loggerConfig.h"

CPP_GUARD_BEGIN

int GPIO_init(LoggerConfig *loggerConfig);

int GPIO_get(int port);
void GPIO_set(int port, unsigned int state);
int GPIO_is_SD_card_present(void);
int GPIO_is_SD_card_writable(void);
int GPIO_is_button_pressed(void);

CPP_GUARD_END

#endif /* GPIO_H_ */
