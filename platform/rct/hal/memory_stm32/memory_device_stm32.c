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
#include "stm32f30x_flash.h"
#include <stddef.h>

#define MEMORY_PAGE_SIZE	2048

static const struct sector_info {
	void* addr;
	size_t size;
} flash_sectors[] = {
	{(void*) 0x08038000, 16384},
	{(void*) 0x0803C000, 16384},
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
	FLASH_ClearFlag(FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR | FLASH_FLAG_EOP);

	uint32_t addr_val = (uint32_t) si->addr;
	const uint32_t end = addr_val + si->size;
	bool status = true;
	for (; addr_val < end; addr_val += MEMORY_PAGE_SIZE) {
		const bool erased = FLASH_COMPLETE == FLASH_ErasePage(addr_val);
		status &= erased;
		if (!erased)
			pr_warning_int_msg("Failed to erase sector ", addr_val);
	}

	FLASH_Lock();
	return status;
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
	FLASH_ClearFlag(FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR | FLASH_FLAG_EOP);

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
