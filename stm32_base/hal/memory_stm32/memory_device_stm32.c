#include "memory_device.h"
#include "stm32f4xx_flash.h"

#define MEMORY_PAGE_SIZE	2048

/* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_0 ((uint32_t)0x08000000)
/* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1 ((uint32_t)0x08004000)
/* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2 ((uint32_t)0x08008000)
/* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3 ((uint32_t)0x0800C000)
/* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_4 ((uint32_t)0x08010000)

static uint32_t selectFlashSector(const void *address)
{
    uint32_t addr = (uint32_t) address;
    //we don't support writing flash sector 0 since that's where the bootloader lives.
    switch (addr) {
    case ADDR_FLASH_SECTOR_1:
        return FLASH_Sector_1;
    case ADDR_FLASH_SECTOR_2:
        return FLASH_Sector_2;
    case ADDR_FLASH_SECTOR_3:
        return FLASH_Sector_3;
    case ADDR_FLASH_SECTOR_4:
        return FLASH_Sector_4;
    default:
        return 0;
    }
}

enum memory_flash_result_t memory_device_flash_region(const void *address, const void *data,
        unsigned int length)
{

    enum memory_flash_result_t rc = MEMORY_FLASH_SUCCESS;
    /* Erase the entire page before you can write.  This filters
     * the incoming addresses to available flash pages for the STM32F4 */
    uint32_t flashSector = selectFlashSector(address);
    if (flashSector) {
        FLASH_Unlock();
        FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR |
                        FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR |
                        FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
        FLASH_EraseSector(flashSector, VoltageRange_3);

        uint32_t addrTarget = (uint32_t) address;
        uint8_t *dataTarget = (uint8_t *) data;

        for (unsigned int i = 0; i < length; i++) {
            if (FLASH_ProgramByte(addrTarget + i, *(dataTarget + i)) != FLASH_COMPLETE) {
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
