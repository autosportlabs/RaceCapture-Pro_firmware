#ifndef MEMORY_H_
#define MEMORY_H_

enum memory_flash_result_t{
    MEMORY_FLASH_SUCCESS = 0,
    MEMORY_FLASH_WRITE_ERROR = -1
};

enum memory_flash_result_t memory_flash_region(const void *vAddress, const void *vData, unsigned int length);

#endif /* MEMORY_H_ */
