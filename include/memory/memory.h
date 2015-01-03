#ifndef MEMORY_H_
#define MEMORY_H_

int memory_flash_region(const void *vAddress, const void *vData, unsigned int length);
void memory_read_region(const void *address, void * const data, unsigned int length);

#endif /* MEMORY_H_ */
