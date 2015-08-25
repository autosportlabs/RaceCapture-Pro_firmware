#include "memory_device.h"
#include "stm32f30x_flash.h"
#include <stddef.h>

#define MEMORY_PAGE_SIZE	2048

/* STM32F3 is on 2K page sizes */
static uint32_t selectFlashSector(const void *address)
{
    uint32_t addr = (uint32_t) address;
    return addr % MEMORY_PAGE_SIZE == 0 ? addr : 0;
}

enum memory_flash_result_t memory_device_flash_region(const void *address, const void *data,
        unsigned int length)
{
    enum memory_flash_result_t rc = MEMORY_FLASH_SUCCESS;

    /* adjust length to word boundary */
	length += (length % sizeof(uint32_t) == 0) ? 0 : sizeof(uint32_t) - length % sizeof(uint32_t);

    uint32_t flash_page = selectFlashSector(address);
    if (flash_page) {
        FLASH_Unlock();
        FLASH_ClearFlag(FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR | FLASH_FLAG_EOP);

        /*flash all sectors in range */
        for (size_t i = flash_page; i < flash_page + length; i += MEMORY_PAGE_SIZE) {
        	FLASH_ErasePage(i);
        }

        uint32_t addrTarget = (uint32_t) address;
        uint32_t *dataTarget = (uint32_t *) data;

        size_t data_index = 0;
        for (size_t i = 0; i < length; i+=sizeof(uint32_t)) {
            if (FLASH_ProgramWord(addrTarget + i, dataTarget[data_index++]) != FLASH_COMPLETE) {
                rc = MEMORY_FLASH_WRITE_ERROR;
                break;
            }
        }
        FLASH_Lock();
    } else {
        rc = MEMORY_FLASH_WRITE_ERROR;
    }
    return rc;
}
