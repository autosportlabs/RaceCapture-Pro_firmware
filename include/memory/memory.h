#ifndef MEMORY_H_
#define MEMORY_H_
#include "memory_device_page_size.h"

#define FLASH_MEMORY_PAGE_SIZE MEMORY_PAGE_SIZE

int memory_flash_region(void *vAddress, void *vData, unsigned int length);

#endif /* MEMORY_H_ */
