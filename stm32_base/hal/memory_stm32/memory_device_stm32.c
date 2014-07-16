#include "memory_device.h"
#include "stm32f4xx_flash.h"

#define MEMORY_PAGE_SIZE       2048
#define FLASH_SUCCESS			0
#define FLASH_WRITE_ERROR		-1

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

static uint32_t selectFlashSector(const void *address){
	uint32_t addr = *((uint32_t*)address);
	switch (addr){
		case ADDR_FLASH_SECTOR_0:
			return ADDR_FLASH_SECTOR_0;
		case ADDR_FLASH_SECTOR_1:
			return ADDR_FLASH_SECTOR_1;
		case ADDR_FLASH_SECTOR_2:
			return ADDR_FLASH_SECTOR_2;
		case ADDR_FLASH_SECTOR_3:
			return ADDR_FLASH_SECTOR_3;
		case ADDR_FLASH_SECTOR_4:
			return ADDR_FLASH_SECTOR_4;
		default:
			return 0;
	}
}

int memory_device_flash_region(const void *address, const void *data, unsigned int length) {

	int rc = FLASH_SUCCESS;
	//erase the entire page before you can write. this filters the incoming addresses to available flash pages for the STM32F4
	uint32_t flashSector = selectFlashSector(address);

	if (flashSector){
		FLASH_Unlock();
		FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);
		FLASH_EraseSector(flashSector, VoltageRange_3);

		uint32_t addrTarget = *(uint32_t*)address;
		uint8_t *dataTarget = (uint8_t*)data;

		for (unsigned int i = 0; i < length; i++) {
			if (FLASH_ProgramByte(addrTarget + i, *(dataTarget + i)) != FLASH_COMPLETE){
				rc = FLASH_WRITE_ERROR;
				break;
			}
		}
		FLASH_Lock();
	}
	else{
		rc = FLASH_WRITE_ERROR;
	}
	return rc;
}
