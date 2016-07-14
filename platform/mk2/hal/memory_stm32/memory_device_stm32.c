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

#include "macros.h"
#include "memory_device.h"
#include "printk.h"
#include "stm32f4xx_flash.h"
#include <stddef.h>

static const struct sector_info {
	void* addr;
	size_t size;
	uint32_t sector;
} flash_sectors[] = {
	{(void*) 0x08004000, 16384, FLASH_Sector_1},
	{(void*) 0x08008000, 16384, FLASH_Sector_2},
	{(void*) 0x0800C000, 16384, FLASH_Sector_3},
	{(void*) 0x08010000, 65536, FLASH_Sector_4},
};

static const struct sector_info* get_flash_sector(
	const volatile void* const addr)
{
	for (size_t i = 0; i < ARRAY_LEN(flash_sectors); ++i) {
		const struct sector_info* si = flash_sectors + i;
		if (si->addr <= addr && addr < si->addr + si->size)
			return si;
	}

	pr_warning("Failed to find flash sector\r\n");
	return NULL;
}

/**
 * Clears the entirety of a region of flash.
 * @param addr The address of any byte within the region of flash.
 */
bool memory_device_region_clear(const volatile void *addr)
{
	const struct sector_info* si = get_flash_sector(addr);
	if (!si)
		return false;

	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR |
			FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR |
			FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

	const FLASH_Status fs = FLASH_EraseSector(si->sector, VoltageRange_3);

	FLASH_Lock();

	if (FLASH_COMPLETE != fs) {
		pr_warning_int_msg("Failed to erase sector ",
				   (uint32_t) si->addr);
		pr_warning_int_msg("Error code: ", (int) fs);
		return false;
	}

	return true;
}


/**
 * Writes bits to a region in flash that was previously cleared using the
 * #memory_device_region_clear method. If that was not used then this may
 * fail to write properly.
 */
int memory_device_write_words(const volatile void* addr, const void* data,
			      const size_t len)
{
	const struct sector_info* si = get_flash_sector(addr);
	if (!si)
		return -1;

	/* Check for word alignment */
	const size_t word_size = sizeof(uint32_t);
	if ((size_t) addr % word_size ||
	    (size_t) data % word_size ||
	    len % word_size) {
		pr_warning("Dest or Data or Len value not word aligned!\r\n");
		return -2;
	}

	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR |
			FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR |
			FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

	size_t i = 0;
	const uint32_t* data_ptr = (const uint32_t*) data;
	const size_t words_len = len / word_size;
	for (; i < words_len; ++i) {
		const uint32_t dest_addr = (uint32_t) addr + word_size * i;
		const uint32_t dest_data = data_ptr[i];
		if (FLASH_COMPLETE != FLASH_ProgramWord(dest_addr, dest_data))
			/* If here, error scenario */
			pr_warning_int_msg("Failed to flash ", dest_addr);
	}

	FLASH_Lock();
	return i * word_size;
}

enum memory_flash_result_t memory_device_flash_region(const volatile void *address,
						      const void *data,
						      unsigned int length)
{
	const bool status =
		memory_device_region_clear(address) &&
		length <= memory_device_write_words(address, data, length);

	return status ? MEMORY_FLASH_SUCCESS : MEMORY_FLASH_WRITE_ERROR;
}
