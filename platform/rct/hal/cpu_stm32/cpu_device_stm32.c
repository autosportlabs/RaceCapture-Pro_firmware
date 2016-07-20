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

#include "cpu_device.h"
#include "portmacro.h"
#include <app_info.h>
#include <stddef.h>
#include <stdint.h>
#include <stm32f30x_misc.h>
#include <stm32f30x_rcc.h>

#define CPU_ID_REGISTER_START 	0x1FFFF7AC
#define CPU_ID_BYTE_COUNT	12
#define SERIAL_ID_BUFFER_LEN	(CPU_ID_BYTE_COUNT * 2 + 1)

extern uint32_t _flash_start;
static char cpu_id[SERIAL_ID_BUFFER_LEN];

static char to_hex(uint8_t val)
{
        val &= 0x0F;
        return val >= 10 ? val - 10  + 'A' : val + '0';
}

static void init_cpu_id()
{
        const uint8_t* ids = (const uint8_t *) CPU_ID_REGISTER_START;

        for (size_t i = 0; i < CPU_ID_BYTE_COUNT; ++i) {
                cpu_id[2 * i] = to_hex(ids[i] >> 4);
                cpu_id[2 * i + 1] = to_hex(ids[i] & 0xF);
        }
}

int cpu_device_init(void)
{
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, (uint32_t)&_flash_start);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	init_cpu_id();
	return 1;
}

void cpu_device_reset(int bootloader)
{
	struct app_handshake_block *handshake =
		(struct app_handshake_block *)HANDSHAKE_ADDR;

	/* Clear any reset flags that might be present (i.e. watchdog) */
	RCC_ClearFlag();

	/*
	 * If bootloader mode is requested, Set the flag in the
	 * handshake area
	 */
	if (bootloader == 1) {
		handshake->loader_magic = LOADER_KEY;
	}

	NVIC_SystemReset();
}

const char *cpu_device_get_serialnumber(void)
{
    return cpu_id;
}

/**
 * Causes the cpu to busy wait the specified number of milliseconds.
 * This is by no means accurate and is really only designed to be used
 * in cases where a crude timing mechanism is all that is available
 * (like when the system panics).
 */
void cpu_device_spin(uint32_t ms)
{
	const uint32_t iterations = 5200;
	while(ms-- > 0)
		for (volatile size_t i = 0; i < iterations; ++i);
}
