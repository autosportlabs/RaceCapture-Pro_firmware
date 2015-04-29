#ifndef MEMORY_H_
#define MEMORY_H_

#define FLASH_SUCCESS		0
#define FLASH_WRITE_ERROR	-1

int memory_flash_region(const void *vAddress, const void *vData, unsigned int length);

#endif /* MEMORY_H_ */
