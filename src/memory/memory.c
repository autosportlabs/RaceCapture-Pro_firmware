#include "memory.h"
#include "memory_device.h"


int memory_flash_region(void *address, void *data, unsigned int length){
	return memory_device_flash_region(address, data, length);
}
