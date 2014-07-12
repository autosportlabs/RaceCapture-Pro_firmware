#include "memory_device.h"
#include "memory_device_page_size.h"
#include "flash.h"

#define MEMORY_PAGE_SIZE       2048
#define FLASH_SUCCESS			0
#define FLASH_WRITE_ERROR		-1


static uint32_t selectFlashSector(const void *address){

	switch (*address){
	case 0x08000000:

	}
}
int memory_device_flash_region(const void *address, const void *data, unsigned int length) {

	int rc = FLASH_SUCCESS;
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);


	//erase the entire page before you can write
	uint32_t flashSector = selectFlashSector(address);

	FLASH_EraseSector(flashSector, VoltageRange_3);

	for (int i = 0; i < length; i++) {
		if (FLASH_ProgramByte(address + i, data + i) != FLASH_COMPLETE){
			result = FLASH_WRITE_ERROR;
			break;
		}
	}

	FLASH_Lock();
	return rc;
}
