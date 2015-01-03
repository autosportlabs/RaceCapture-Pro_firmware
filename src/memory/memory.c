#include <string.h>
#include "memory.h"
#include "memory_device.h"


int memory_flash_region(const void *address, const void *data, unsigned int length){
	return memory_device_flash_region(address, data, length);
}

void memory_read_region(const void *address, void * const data, unsigned int length){
	memcpy(data, address, length);
}
