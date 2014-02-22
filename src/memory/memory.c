#include "memory.h"
#include "memory_device.h"


int memory_flash_region(const void *address, const void *data, unsigned int length){
	if (length < MEMORY_PAGE_SIZE) length = length + (MEMORY_PAGE_SIZE - length);
	return memory_device_flash_region(address, data, length);
}
